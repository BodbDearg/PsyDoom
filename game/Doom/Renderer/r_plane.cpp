#include "r_plane.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

// Stores the extents of a horizontal flat span
struct span_t {
    int32_t bounds[2];
};

// Index into flat span bounds
enum : uint32_t {
    SPAN_LEFT,
    SPAN_RIGHT
};

// The bounds of each flat span for the currently drawing flat
static const VmPtr<span_t[VIEW_3D_H]> gFlatSpans(0x800980D0);

// Internal plane rendering function only: forward declare here
static void R_DrawFlatSpans(leaf_t& leaf, const fixed_t planeViewZ, const texture_t& tex) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw either the floor or ceiling flat for a subsector's leaf
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsectorFlat(leaf_t& leaf, const bool bIsCeiling) noexcept {
    // Get the texture for the flat
    const sector_t& drawsec = **gpCurDrawSector;
    const int32_t flatPicNum = (bIsCeiling) ? drawsec.ceilingpic : drawsec.floorpic;
    const int32_t flatTexNum = (*gpFlatTranslation)[flatPicNum];
    texture_t& tex = (*gpFlatTextures)[flatTexNum];

    // Upload the flat texture to VRAM if required.
    // This case will be triggered for animated flats like water & slime - only one frame will be in VRAM at a time.
    // Most normal flats however will already be uploaded to VRAM on level start.
    if (tex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Decompress the lump data to the temporary buffer if required
        const void* pLumpData;
        const bool bIsUncompressedLump = (*gpbIsUncompressedLump)[tex.lumpNum];

        if (bIsUncompressedLump) {
            pLumpData = (*gpLumpCache)[tex.lumpNum].get();
        } else {
            const void* pCompressedLumpData = (*gpLumpCache)[tex.lumpNum].get();
            decode(pCompressedLumpData, gTmpBuffer.get());
            pLumpData = gTmpBuffer.get();
        }

        // Load the decompressed texture to the required part of VRAM and mark as loaded 
        const RECT vramRect = getTextureVramRect(tex);
        LIBGPU_LoadImage(vramRect, (uint16_t*) pLumpData + 4);      // TODO: figure out what 8 bytes are being skipped
        tex.uploadFrameNum = *gNumFramesDrawn;
    }

    // Setup the texture window so that repeating occurs.
    // Note: the PSX version hardcoded the flat size here to 64x64 - but I will use the actual texture size instead.
    // The behavior should be the same but this way is more flexible for potential modding.
    {
        RECT texWinRect;
        LIBGPU_setRECT(texWinRect, tex.texPageCoordX, tex.texPageCoordY, tex.width, tex.height);

        DR_TWIN* const pTexWinPrim = (DR_TWIN*) getScratchAddr(128);
        LIBGPU_SetTexWindow(*pTexWinPrim, texWinRect);
        I_AddPrim(pTexWinPrim);
    }
    
    // Get Z position of the plane (height) in viewspace
    fixed_t planeZ;

    if (bIsCeiling) {
        planeZ = drawsec.ceilingheight - *gViewZ;
    } else {
        planeZ = drawsec.floorheight - *gViewZ;
    }

    // Draw the horizontal spans of the leaf
    R_DrawFlatSpans(leaf, planeZ >> FRACBITS, tex);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the horizontal flat spans for the specified subsector's leaf
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawFlatSpans(leaf_t& leaf, const fixed_t planeViewZ, const texture_t& tex) noexcept {
    // Compute the screen x and y values for each leaf vertex and save to scratchpad memory for fast access
    uint32_t* const pVerticesScreenX = (uint32_t*) getScratchAddr(160);
    uint32_t* const pVerticesScreenY = pVerticesScreenX + (MAX_LEAF_EDGES + 1);

    {
        uint32_t* pVertScreenX = pVerticesScreenX;
        uint32_t* pVertScreenY = pVerticesScreenY;
        leafedge_t* pEdge = leaf.edges;

        for (int32_t edgeIdx = 0; edgeIdx < leaf.numEdges; ++edgeIdx, ++pEdge, ++pVertScreenX, ++pVertScreenY) {
            const vertex_t& vert = *pEdge->vertex;
            const int32_t screenY = (HALF_VIEW_3D_H - 1) - ((planeViewZ * vert.scale) >> FRACBITS);

            *pVertScreenX = vert.screenx;
            *pVertScreenY = screenY;
        }
    }

    // Determine the y bounds for the plane and for each row determine the x bounds.
    // Basically figuring out what spans we need to draw:
    int32_t planeBegY = VIEW_3D_H;
    int32_t planeEndY = 0;

    {
        // Add each edge's contribution to draw/span bounds
        const uint32_t* pVertScreenY = pVerticesScreenY;

        for (int32_t edgeIdx = 0; edgeIdx < leaf.numEdges; ++edgeIdx, ++pVertScreenY) {
            int32_t nextEdgeIdx = edgeIdx + 1;

            if (edgeIdx == leaf.numEdges - 1) {
                nextEdgeIdx = 0;
            }

            const int32_t vertScreenY = *pVertScreenY;
            const int32_t nextVertScreenY = pVerticesScreenY[nextEdgeIdx];

            if (vertScreenY != nextVertScreenY) {
                // Figure out the screen x and y value for the top and bottom point of the edge.
                // This also determines which side of the screen the flat edge is on.
                // Edges on the left side of the screen go bottom to top, and edges on the right go top to bottom.
                int32_t botEdgeIdx;
                int32_t topEdgeIdx;
                int32_t spanBound;

                if (vertScreenY < nextVertScreenY) {
                    // Flat edge is on the right side of the screen running downwards
                    botEdgeIdx = edgeIdx;
                    topEdgeIdx = nextEdgeIdx;
                    spanBound = SPAN_RIGHT;
                } else {
                    // Flat edge is on the left side of the screen running upwards
                    botEdgeIdx = nextEdgeIdx;
                    topEdgeIdx = edgeIdx;
                    spanBound = SPAN_LEFT;
                }

                const fixed_t topScreenX = pVerticesScreenX[topEdgeIdx] << FRACBITS;
                const fixed_t botScreenX = pVerticesScreenX[botEdgeIdx] << FRACBITS;

                const int32_t topScreenY = pVerticesScreenY[topEdgeIdx];
                const int32_t botScreenY = pVerticesScreenY[botEdgeIdx];

                // Figure out the size of the edge and x step per screen row
                const fixed_t screenW = topScreenX - botScreenX;
                const int32_t screenH = topScreenY - botScreenY;
                const fixed_t xStep = screenW / screenH;

                // Clamp the edge y bounds to the screen
                fixed_t x = botScreenX;
                int32_t y = botScreenY;

                if (botScreenY < 0) {
                    x -= botScreenY * xStep;
                    y = 0;
                }

                int32_t yEnd = topScreenY;

                if (topScreenY > VIEW_3D_H) {
                    yEnd = VIEW_3D_H;
                }

                if (y < planeBegY) {
                    planeBegY = y;
                }

                if (yEnd > planeEndY) {
                    planeEndY = yEnd;
                }

                // For each row in this edge, set the appropriate bound in the flat spans that the edge intersects
                span_t* pSpan = &gFlatSpans[y];
                
                while (y < yEnd) {
                    pSpan->bounds[spanBound] = x >> FRACBITS;
                    x += xStep;
                    ++y;
                    ++pSpan;
                }
            }
        }
    }

    // PC-PSX: sanity check these bounds
    #if PC_PSX_DOOM_MODS
        ASSERT((planeBegY >= planeEndY) || (planeBegY >= 0 && planeBegY <  VIEW_3D_H));
        ASSERT((planeBegY >= planeEndY) || (planeEndY >= 0 && planeEndY <= VIEW_3D_H));
    #endif

    // Fill in the parts of the draw primitive that are common to all flat spans
    POLY_FT3& polyPrim = *(POLY_FT3*) getScratchAddr(128);

    LIBGPU_SetPolyFT3(polyPrim);
    polyPrim.clut = *g3dViewPaletteClutId;
    polyPrim.tpage = tex.texPageId;

    // Draw all of the horizontal spans in the flat
    const span_t* pSpan = &gFlatSpans[planeBegY];

    for (int32_t spanY = planeBegY; spanY < planeEndY; ++spanY, ++pSpan) {
        // Get the bounds of the span and skip if zero sized
        int32_t spanL = pSpan->bounds[SPAN_LEFT];
        int32_t spanR = pSpan->bounds[SPAN_RIGHT];

        if (spanL == spanR)
            continue;

        // Swap the span bounds if they are the wrong way around
        if (spanL > spanR) {
            const int32_t oldSpanL = spanL;
            spanL = spanR;
            spanR = oldSpanL;
        }

        // Hack that extends the span bounds, presumably so they join with walls etc.
        // This does cause some visual bugs in places, particularly around steps.
        // If you look at some steps in PSX DOOM the flat sometimes extends past where it should into the 'air'...
        spanL -= 2;
        spanR += 2;

        // Figure out the distance away that the flat span is at
        const int32_t dist = (gYSlope[spanY] * planeViewZ) >> FRACBITS;

        // Compute the base texture uv offset for the span based on solely on global position and view angle
        const fixed_t viewCos = *gViewCos;
        const fixed_t viewSin = *gViewSin;
        const fixed_t viewX = *gViewX;
        const fixed_t viewY = *gViewY;        
        const fixed_t spanUOffset = dist * viewCos + viewX;
        const fixed_t spanVOffset = dist * viewSin + viewY;

        // Compute the uv stepping per each pixel in the span.
        // I'm not sure why the code is doing a 'ceil' type rounding operation here in the case negative coords...
        fixed_t uStepPerX = dist *  viewSin;
        fixed_t vStepPerX = dist * -viewCos;
        
        if (uStepPerX < 0) {
            uStepPerX += HALF_SCREEN_W - 1;     // Round up if negative, why is this?
        }

        if (vStepPerX < 0) {
            vStepPerX += HALF_SCREEN_W - 1;     // Round up if negative, why is this?
        }

        uStepPerX /= HALF_SCREEN_W;
        vStepPerX /= HALF_SCREEN_W;        

        // Compute the uv coordinates for the left and right of the span
        const int32_t spanViewL = spanL - HALF_SCREEN_W;
        const int32_t spanViewR = spanR - HALF_SCREEN_W;

        int32_t spanUL = (spanViewL * uStepPerX + spanUOffset) >> FRACBITS;
        int32_t spanUR = (spanViewR * uStepPerX + spanUOffset) >> FRACBITS;

        int32_t spanVL = (spanViewL * vStepPerX + spanVOffset) >> FRACBITS;
        int32_t spanVR = (spanViewR * vStepPerX + spanVOffset) >> FRACBITS;

        // Wrap the uv coordinates to 64x64 using the smallest u or v coordinate as the basis for wrapping.
        // Note: if we wanted to support textures > 64x64 then this code would need to change!
        constexpr int32_t WRAP_ADJUST_MASK_64 = ~63;

        if (spanUL < spanUR) {
            const int32_t uadjust = spanUL & WRAP_ADJUST_MASK_64;
            spanUL -= uadjust;
            spanUR -= uadjust;
        } else {
            const int32_t uadjust = spanUR & WRAP_ADJUST_MASK_64;
            spanUR -= uadjust;
            spanUL -= uadjust;
        }

        if (spanVL < spanVR) {
            const int32_t vadjust = spanVL & WRAP_ADJUST_MASK_64;
            spanVL -= vadjust;
            spanVR -= vadjust;
        } else {
            const int32_t vadjust = spanVR & WRAP_ADJUST_MASK_64;
            spanVR -= vadjust;
            spanVL -= vadjust;
        }

        // Determine the light color multiplier to use for the span and set on the draw primitive
        {
            int32_t r, g, b;

            if (*gbDoViewLighting) {
                int32_t lightIntensity = LIGHT_INTENSTIY_MAX - dist / 2;
            
                if (lightIntensity < LIGHT_INTENSTIY_MIN) {
                    lightIntensity = LIGHT_INTENSTIY_MIN;
                } 
                else if (lightIntensity > LIGHT_INTENSTIY_MAX) {
                    lightIntensity = LIGHT_INTENSTIY_MAX;
                }

                r = (lightIntensity * (*gCurLightValR)) >> 7;
                g = (lightIntensity * (*gCurLightValG)) >> 7;
                b = (lightIntensity * (*gCurLightValB)) >> 7;
                if (r > 255) { r = 255; }
                if (g > 255) { g = 255; }
                if (b > 255) { b = 255; }
            } else {
                r = *gCurLightValR;
                g = *gCurLightValG;
                b = *gCurLightValB;
            }

            LIBGPU_setRGB0(polyPrim, (uint8_t) r, (uint8_t) g, (uint8_t) b);
        }

        // Determine how many span segments we will have to draw, minus 1.
        //
        // Because the PlayStation's GPU only supports byte texture coordinates, we must split up spans that have a distance of 256 units or
        // greater in texture space in order to work our way around this restriction, and issue more draw calls. The texcoord range limit will
        // typically be hit for the floors or ceilings of large open areas, where the floor or ceiling texture repeats many times.
        //
        // First of all check if if we have hit the limit, and need to split up the span into multiple pieces:
        int32_t numSpanPieces = 0;
        
        if ((spanUL > TEXCOORD_MAX) || (spanUR > TEXCOORD_MAX) || (spanVL > TEXCOORD_MAX) || (spanVR > TEXCOORD_MAX)) {
            // It looks like we are going to need more than 1 span segment given the texture coordinates exceed the max possible by the hardware.
            // Determine how much of the texture we are going to display across the span.
            int32_t spanUSize = spanUR - spanUL;
            int32_t spanVSize = spanVR - spanVL;
            if (spanUSize < 0) { spanUSize = -spanUSize; }
            if (spanVSize < 0) { spanVSize = -spanVSize; }

            // Determine the number of span pieces we would need based on the u and v range pick the largest figure.
            // The code here appears to be conservative and limits the max u & v range of each span piece to 128 units, well below the maximum of 256.
            const int32_t numUPieces = spanUSize >> 7;
            const int32_t numVPieces = spanVSize >> 7;

            if (numUPieces > numVPieces) {
                numSpanPieces = numUPieces;
            } else {
                numSpanPieces = numVPieces;
            }
        }

        // Draw the flat span piece(s)
        if (numSpanPieces == 0) {
            // Easy case - we can draw the entire flat span with a single polygon primitive
            LIBGPU_setXY3(polyPrim,
                (int16_t) spanL, (int16_t) spanY,
                (int16_t) spanR, (int16_t) spanY,
                (int16_t) spanR, (int16_t) spanY + 1
            );

            LIBGPU_setUV3(polyPrim,
                (uint8_t) spanUL, (uint8_t) spanVL,
                (uint8_t) spanUR, (uint8_t) spanVR,
                (uint8_t) spanUR, (uint8_t) spanVR
            );
            
            I_AddPrim(&polyPrim);
        } else {
            // Harder case: we must split up the flat span and issue multiple primitives.
            // Note also, the piece count is minus 1 so increment here now to get the true amount:
            ++numSpanPieces;

            // Figure out the u, v and x increment per span piece
            const int32_t xStep = (spanR - spanL) / numSpanPieces;
            const int32_t uStep = (spanUR - spanUL) / numSpanPieces;
            const int32_t vStep = (spanVR - spanVL) / numSpanPieces;

            // Issue the draw primitives for all of the span pieces
            for (int32_t pieceIdx = 0; pieceIdx < numSpanPieces; ++pieceIdx) {
                // Step the right bounds to the next piece
                spanR = spanL + xStep;
                spanUR = spanUL + uStep;
                spanVR = spanVL + vStep;
                
                // Wrap the uv coordinates to 128x128 using the smallest u or v coordinate as the basis for wrapping.
                // Do this in a similar way to the initial draw preparation, except this time the wrapping is 128x128.
                //
                // Potential bug: it looks like the 'left == right' equality case is not being handled.
                // In some circumstances it might be possible for wrapping to not be done and for the span to be distorted?
                constexpr int32_t WRAP_ADJUST_MASK_128 = ~127;

                int32_t uadjust = 0;
                int32_t vadjust = 0;

                if ((spanUL < spanUR) && (spanUR > TEXCOORD_MAX)) {
                    uadjust = spanUL & WRAP_ADJUST_MASK_128;
                    spanUL -= uadjust;
                    spanUR -= uadjust;
                } else if ((spanUL > spanUR) && (spanUL > TEXCOORD_MAX)) {
                    uadjust = spanUR & WRAP_ADJUST_MASK_128;
                    spanUR -= uadjust;
                    spanUL -= uadjust;
                }

                if ((spanVL < spanVR) && (spanVR > TEXCOORD_MAX)) {
                    vadjust = spanVL & WRAP_ADJUST_MASK_128;
                    spanVL -= vadjust;
                    spanVR -= vadjust;
                } else if ((spanVL > spanVR) && (spanVL > TEXCOORD_MAX)) {
                    vadjust = spanVR & WRAP_ADJUST_MASK_128;
                    spanVR -= vadjust;
                    spanVL -= vadjust;
                }

                // Setup the rest of the drawing primitive and draw the span
                LIBGPU_setXY3(polyPrim,
                    (int16_t) spanL, (int16_t) spanY,
                    (int16_t) spanR, (int16_t) spanY,
                    (int16_t) spanR, (int16_t) spanY + 1
                );

                LIBGPU_setUV3(polyPrim,
                    (uint8_t) spanUL, (uint8_t) spanVL,
                    (uint8_t) spanUR, (uint8_t) spanVR,
                    (uint8_t) spanUR, (uint8_t) spanVR
                );
                
                I_AddPrim(&polyPrim);
                
                // Move coords onto the next span.
                // Note that the previous wrapping operation (if any) is also undone here.
                spanL = spanR;
                spanUL = spanUR + uadjust;
                spanVL = spanVR + vadjust;
            }
        }
    }
}

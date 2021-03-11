#include "r_plane.h"

#include "Asserts.h"
#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "PcPsx/Config.h"
#include "PsyQ/LIBGPU.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

#include <vector>

// Stores the extents of a horizontal flat span
struct span_t {
    int32_t bounds[2];
};

// Index into flat span bounds
enum : int32_t {
    SPAN_LEFT,
    SPAN_RIGHT
};

// The bounds of each flat span for the currently drawing flat
static span_t gFlatSpans[VIEW_3D_H];

#if PSYDOOM_MODS
    // PsyDoom: a temporary buffer used used to store screenspace x and y values (interleaved) for leaf edge vertices
    static std::vector<int32_t> gLeafScreenVerts;
#endif

// Internal plane rendering function only: forward declare here
static void R_DrawFlatSpans(leaf_t& leaf, const fixed_t planeViewZ, const texture_t& tex) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw either the floor or ceiling flat for a subsector's leaf
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsectorFlat(leaf_t& leaf, const bool bIsCeiling) noexcept {
    // Get the texture for the flat
    const sector_t& drawsec = *gpCurDrawSector;
    const int32_t flatPicNum = (bIsCeiling) ? drawsec.ceilingpic : drawsec.floorpic;
    const int32_t flatTexNum = gpFlatTranslation[flatPicNum];
    texture_t& tex = gpFlatTextures[flatTexNum];

    // Upload the flat texture to VRAM if required.
    // This case will be triggered for animated flats like water & slime - only one frame will be in VRAM at a time.
    // Most normal flats however will already be uploaded to VRAM on level start.
    if (tex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Decompress the lump data to the temporary buffer if required
        const std::byte* pLumpData;
        const bool bIsUncompressedLump = gpbIsUncompressedLump[tex.lumpNum];

        if (bIsUncompressedLump) {
            pLumpData = (const std::byte*) gpLumpCache[tex.lumpNum];
        } else {
            const void* pCompressedLumpData = gpLumpCache[tex.lumpNum];
            ASSERT(getDecodedSize(pCompressedLumpData) <= TMP_BUFFER_SIZE);
            decode(pCompressedLumpData, gTmpBuffer);
            pLumpData = gTmpBuffer;
        }

        // Load the decompressed texture to the required part of VRAM and mark as loaded
        const RECT vramRect = getTextureVramRect(tex);
        LIBGPU_LoadImage(vramRect, (uint16_t*)(pLumpData + sizeof(texlump_header_t)));
        tex.uploadFrameNum = gNumFramesDrawn;
    }

    // Setup the texture window so that repeating occurs.
    // Note: the PSX version hardcoded the flat size here to 64x64 - but I will use the actual texture size instead.
    // The behavior should be the same but this way is more flexible for potential modding.
    {
        RECT texWinRect;
        LIBGPU_setRECT(texWinRect, tex.texPageCoordX, tex.texPageCoordY, tex.width, tex.height);

        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_TWIN texWinPrim = {};
        #else
            DR_TWIN& texWinPrim = *(DR_TWIN*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetTexWindow(texWinPrim, texWinRect);
        I_AddPrim(texWinPrim);
    }
    
    // Get Z position of the plane (height) in viewspace
    fixed_t planeZ;

    if (bIsCeiling) {
        planeZ = drawsec.ceilingheight - gViewZ;
    } else {
        planeZ = drawsec.floorheight - gViewZ;
    }

    // Draw the horizontal spans of the leaf
    R_DrawFlatSpans(leaf, d_fixed_to_int(planeZ), tex);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the horizontal flat spans for the specified subsector's leaf
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawFlatSpans(leaf_t& leaf, const fixed_t planeViewZ, const texture_t& tex) noexcept {
    // PsyDoom: if the leaf exceeds the maximum number of edges allowed then skip it.
    // TODO: in limit removing work, remove this limit and make it compile time configurable.
    #if PSYDOOM_MODS
        if (leaf.numEdges > MAX_LEAF_EDGES)
            return;
    #endif

    // Compute the screen x and y values for each leaf vertex and save to scratchpad memory for fast access.
    // PsyDoom: use a vector for this rather than the scratchpad since we're removing the use of the scratchpad from the project.
    #if PSYDOOM_MODS
        ASSERT_LOG(gLeafScreenVerts.empty(), "Leaf screen verts list should be empty on entry to this function!");
        gLeafScreenVerts.reserve(256);

        {
            leafedge_t* pEdge = leaf.edges;

            for (int32_t edgeIdx = 0; edgeIdx < leaf.numEdges; ++edgeIdx, ++pEdge) {
                const vertex_t& vert = *pEdge->vertex;
                const int32_t screenY = (HALF_VIEW_3D_H - 1) - d_fixed_to_int(planeViewZ * vert.scale);

                gLeafScreenVerts.push_back(vert.screenx);
                gLeafScreenVerts.push_back(screenY);
            }
        }

        const int32_t* const pLeafScreenVerts = gLeafScreenVerts.data();
    #else
        int32_t* const pVerticesScreenX = (int32_t*) LIBETC_getScratchAddr(160);
        int32_t* const pVerticesScreenY = pVerticesScreenX + (MAX_LEAF_EDGES + 1);

        {
            int32_t* pVertScreenX = pVerticesScreenX;
            int32_t* pVertScreenY = pVerticesScreenY;
            leafedge_t* pEdge = leaf.edges;

            for (int32_t edgeIdx = 0; edgeIdx < leaf.numEdges; ++edgeIdx, ++pEdge, ++pVertScreenX, ++pVertScreenY) {
                const vertex_t& vert = *pEdge->vertex;
                const int32_t screenY = (HALF_VIEW_3D_H - 1) - d_fixed_to_int(planeViewZ * vert.scale);

                *pVertScreenX = vert.screenx;
                *pVertScreenY = screenY;
            }
        }
    #endif  // #if PSYDOOM_MODS

    // Determine the y bounds for the plane and for each row determine the x bounds.
    // Basically figuring out what spans we need to draw:
    int32_t planeBegY = VIEW_3D_H;
    int32_t planeEndY = 0;

    {
        // Add each edge's contribution to draw/span bounds
        #if !PSYDOOM_MODS
            const int32_t* pVertScreenY = pVerticesScreenY;
        #endif

        for (int32_t edgeIdx = 0; edgeIdx < leaf.numEdges; ++edgeIdx) {
            #if !PSYDOOM_MODS
                ++pVertScreenY;
            #endif

            int32_t nextEdgeIdx = edgeIdx + 1;

            if (edgeIdx == leaf.numEdges - 1) {
                nextEdgeIdx = 0;
            }

            #if PSYDOOM_MODS
                // PsyDoom: have to modify how these values are queried since they are now in a std::vector rather than the scratchpad
                const int32_t vertScreenY = pLeafScreenVerts[edgeIdx * 2 + 1];
                const int32_t nextVertScreenY = pLeafScreenVerts[nextEdgeIdx * 2 + 1];
            #else
                const int32_t vertScreenY = *pVertScreenY;
                const int32_t nextVertScreenY = pVerticesScreenY[nextEdgeIdx];
            #endif

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

                // PsyDoom: have to modify how these values are queried for the new (non-scratchpad) container used
                #if PSYDOOM_MODS
                    // PsyDoom: have to modify how these values are queried since they are now in a std::vector rather than the scratchpad
                    const fixed_t topScreenX = d_int_to_fixed((int32_t) pLeafScreenVerts[topEdgeIdx * 2]);
                    const fixed_t botScreenX = d_int_to_fixed((int32_t) pLeafScreenVerts[botEdgeIdx * 2]);

                    const int32_t topScreenY = pLeafScreenVerts[topEdgeIdx * 2 + 1];
                    const int32_t botScreenY = pLeafScreenVerts[botEdgeIdx * 2 + 1];
                #else
                    const fixed_t topScreenX = d_int_to_fixed((int32_t) pVerticesScreenX[topEdgeIdx]);
                    const fixed_t botScreenX = d_int_to_fixed((int32_t) pVerticesScreenX[botEdgeIdx]);

                    const int32_t topScreenY = pVerticesScreenY[topEdgeIdx];
                    const int32_t botScreenY = pVerticesScreenY[botEdgeIdx];
                #endif

                // Figure out the size of the edge and x step per screen row
                const fixed_t screenW = topScreenX - botScreenX;
                const int32_t screenH = topScreenY - botScreenY;
                const fixed_t xStep = screenW / screenH;

                // Clamp the edge y bounds to the screen.
                // PsyDoom: use 64-bit arithmetic here to avoid overflows.
                #if PSYDOOM_MODS
                    int64_t x = botScreenX;
                #else
                    fixed_t x = botScreenX;
                #endif
                    
                int32_t y = botScreenY;

                if (botScreenY < 0) {
                    #if PSYDOOM_MODS
                        x -= (int64_t) botScreenY * (int64_t) xStep;
                    #else
                        x -= botScreenY * xStep;
                    #endif
                    
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
                while (y < yEnd) {
                    span_t& span = gFlatSpans[y];
                    
                    #if PSYDOOM_MODS
                        span.bounds[spanBound] = (int32_t) d_rshift<FRACBITS>(x);
                    #else
                        span.bounds[spanBound] = d_fixed_to_int(x);
                    #endif
                    
                    x += xStep;
                    ++y;
                }
            }
        }
    }

    // PsyDoom: sanity check these bounds and get whether to apply the floor gap fix
    #if PSYDOOM_MODS
        ASSERT((planeBegY >= planeEndY) || (planeBegY >= 0 && planeBegY <  VIEW_3D_H));
        ASSERT((planeBegY >= planeEndY) || (planeEndY >= 0 && planeEndY <= VIEW_3D_H));

        const bool bFixFloorGaps = Config::gbFloorRenderGapFix;
    #endif

    // Fill in the parts of the draw primitive that are common to all flat spans
    #if PSYDOOM_MODS
        // PsyDoom: switched this to the new 'floor row' drawing primitive supported by PsyDoom's enhanced GPU - this performs better and with similar results.
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state.
        FLOORROW_FT drawPrim = {};
        LIBGPU_SetFloorRowFT(drawPrim);
    #else
        POLY_FT3& drawPrim = *(POLY_FT3*) LIBETC_getScratchAddr(128);
        LIBGPU_SetPolyFT3(drawPrim);
    #endif

    drawPrim.clut = g3dViewPaletteClutId;
    drawPrim.tpage = tex.texPageId;

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

        // A hack that extends the span bounds, presumably so they join with walls etc.
        // This does cause some visual bugs in places, particularly around steps.
        // If you look at some steps in PSX DOOM the flat sometimes extends past where it should into the 'air'...
        spanL -= 2;
        spanR += 2;

        // Figure out the distance away that the flat span is at
        const int32_t dist = d_fixed_to_int(gYSlope[spanY] * planeViewZ);

        // Compute the base texture uv offset for the span based on solely on global position and view angle
        const fixed_t viewCos = gViewCos;
        const fixed_t viewSin = gViewSin;
        const fixed_t viewX = gViewX;
        const fixed_t viewY = gViewY;
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

        int32_t spanUL = d_fixed_to_int(spanViewL * uStepPerX + spanUOffset);
        int32_t spanUR = d_fixed_to_int(spanViewR * uStepPerX + spanUOffset);

        int32_t spanVL = d_fixed_to_int(spanViewL * vStepPerX + spanVOffset);
        int32_t spanVR = d_fixed_to_int(spanViewR * vStepPerX + spanVOffset);

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

            if (gbDoViewLighting) {
                int32_t lightIntensity = LIGHT_INTENSTIY_MAX - d_rshift<1>(dist);
                
                if (lightIntensity < LIGHT_INTENSTIY_MIN) {
                    lightIntensity = LIGHT_INTENSTIY_MIN;
                }
                else if (lightIntensity > LIGHT_INTENSTIY_MAX) {
                    lightIntensity = LIGHT_INTENSTIY_MAX;
                }

                r = ((uint32_t) lightIntensity * gCurLightValR) >> 7;
                g = ((uint32_t) lightIntensity * gCurLightValG) >> 7;
                b = ((uint32_t) lightIntensity * gCurLightValB) >> 7;
                if (r > 255) { r = 255; }
                if (g > 255) { g = 255; }
                if (b > 255) { b = 255; }
            } else {
                r = gCurLightValR;
                g = gCurLightValG;
                b = gCurLightValB;
            }

            LIBGPU_setRGB0(drawPrim, (uint8_t) r, (uint8_t) g, (uint8_t) b);
        }

        // Determine how many span segments we will have to draw, minus 1.
        //
        // Because the PlayStation's GPU only supports byte texture coordinates, we must split up spans that have a distance of 256 units or
        // greater in texture space in order to work our way around this restriction, and issue more draw calls. The texcoord range limit will
        // typically be hit for the floors or ceilings of large open areas, where the floor or ceiling texture repeats many times.
        //
        // First of all check if if we have hit the limit, and need to split up the span into multiple pieces:
        int32_t numSpanPieces = 0;

        // PsyDoom: optimization, if we are using 16-bit UVs and using the floor render gap fix then we can always draw everything in one go.
        // Otherwise emulate the old slightly more artifact prone behavior...
        #if PSYDOOM_LIMIT_REMOVING
            const bool bNeedToCheckForSpanSplits = (!bFixFloorGaps);
        #else
            const bool bNeedToCheckForSpanSplits = true;
        #endif

        if (bNeedToCheckForSpanSplits) {
            if ((spanUL > TEXCOORD_MAX) || (spanUR > TEXCOORD_MAX) || (spanVL > TEXCOORD_MAX) || (spanVR > TEXCOORD_MAX)) {
                // It looks like we are going to need more than 1 span segment given the texture coordinates exceed the max possible by the hardware.
                // Determine how much of the texture we are going to display across the span.
                int32_t spanUSize = spanUR - spanUL;
                int32_t spanVSize = spanVR - spanVL;
                if (spanUSize < 0) { spanUSize = -spanUSize; }
                if (spanVSize < 0) { spanVSize = -spanVSize; }

                // Determine the number of span pieces we would need based on the u and v range pick the largest figure.
                // The code here appears to be conservative and limits the max u & v range of each span piece to 128 units, well below the maximum of 256.
                const uint32_t numUPieces = (uint32_t) spanUSize >> 7;
                const uint32_t numVPieces = (uint32_t) spanVSize >> 7;

                if (numUPieces > numVPieces) {
                    numSpanPieces = numUPieces;
                } else {
                    numSpanPieces = numVPieces;
                }
            }
        }

        // Draw the flat span piece(s)
        if (numSpanPieces == 0) {
            // Easy case - we can draw the entire flat span with a single polygon primitive.
            // PsyDoom: now drawing this with the new GPU 'floor row' primitive instead of a polygon for better performance.
            #if PSYDOOM_MODS
                drawPrim.x0 = (int16_t) spanL;
                drawPrim.x1 = (int16_t) spanR;
                drawPrim.y0 = (int16_t) spanY;
                drawPrim.u0 = (LibGpuUV) spanUL;
                drawPrim.v0 = (LibGpuUV) spanVL;
                drawPrim.u1 = (LibGpuUV) spanUR;
                drawPrim.v1 = (LibGpuUV) spanVR;
            #else
                LIBGPU_setXY3(drawPrim,
                    (int16_t) spanL, (int16_t) spanY,
                    (int16_t) spanR, (int16_t) spanY,
                    (int16_t) spanR, (int16_t) spanY + 1
                );

                LIBGPU_setUV3(drawPrim,
                    (uint8_t) spanUL, (uint8_t) spanVL,
                    (uint8_t) spanUR, (uint8_t) spanVR,
                    (uint8_t) spanUR, (uint8_t) spanVR
                );
            #endif
            
            I_AddPrim(drawPrim);
        } else {
            // Harder case: we must split up the flat span and issue multiple primitives.
            // Note also, the piece count is minus 1 so increment here now to get the true amount:
            ++numSpanPieces;

            // Figure out the u, v and x increment per span piece
            const int32_t xStep = (spanR - spanL) / numSpanPieces;
            const int32_t uStep = (spanUR - spanUL) / numSpanPieces;
            const int32_t vStep = (spanVR - spanVL) / numSpanPieces;

            // PsyDoom: precision fix to prevent cracks at the right side of the screen on large open maps like 'Tower Of Babel'.
            // Store the coords where the last span should end, and use those for the right side of the last span instead of
            // the somewhat truncated/imprecise stepped coords.
            #if PSYDOOM_MODS
                const int32_t origSpanR = spanR;
                const int32_t origSpanUR = spanUR;
                const int32_t origSpanVR = spanVR;
            #endif

            // Issue the draw primitives for all of the span pieces
            for (int32_t pieceIdx = 0; pieceIdx < numSpanPieces; ++pieceIdx) {
                // Step the right bounds to the next piece
                spanR = spanL + xStep;
                spanUR = spanUL + uStep;
                spanVR = spanVL + vStep;

                // PsyDoom: precision fix to prevent cracks at the right side of the screen on large open maps like 'Tower Of Babel'.
                // This is only neccessary if we are not doing limit removing and don't have 16-bit uvs.
                // If we have 16-bit uvs then we can just draw the whole thing in go all of the time.
                #if PSYDOOM_MODS && !PSYDOOM_LIMIT_REMOVING
                    if (bFixFloorGaps) {
                        if (pieceIdx + 1 >= numSpanPieces) {
                            spanR = origSpanR;
                            spanUR = origSpanUR;
                            spanVR = origSpanVR;
                        }
                    }
                #endif
                
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
                // PsyDoom: now drawing this with the new GPU 'floor row' primitive instead of a polygon for better performance.
                #if PSYDOOM_MODS
                    drawPrim.x0 = (int16_t) spanL;
                    drawPrim.x1 = (int16_t) spanR;
                    drawPrim.y0 = (int16_t) spanY;
                    drawPrim.u0 = (LibGpuUV) spanUL;
                    drawPrim.v0 = (LibGpuUV) spanVL;
                    drawPrim.u1 = (LibGpuUV) spanUR;
                    drawPrim.v1 = (LibGpuUV) spanVR;
                #else
                    LIBGPU_setXY3(drawPrim,
                        (int16_t) spanL, (int16_t) spanY,
                        (int16_t) spanR, (int16_t) spanY,
                        (int16_t) spanR, (int16_t) spanY + 1
                    );

                    LIBGPU_setUV3(drawPrim,
                        (uint8_t) spanUL, (uint8_t) spanVL,
                        (uint8_t) spanUR, (uint8_t) spanVR,
                        (uint8_t) spanUR, (uint8_t) spanVR
                    );
                #endif

                I_AddPrim(drawPrim);

                // Move coords onto the next span.
                // Note that the previous wrapping operation (if any) is also undone here.
                spanL = spanR;
                spanUL = spanUR + uadjust;
                spanVL = spanVR + vadjust;
            }
        }
    }

    // PsyDoom: need to cleanup this list before exiting
    #if PSYDOOM_MODS
        gLeafScreenVerts.clear();
    #endif
}

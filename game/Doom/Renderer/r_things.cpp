#include "r_things.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

// Describes a sprite that is to be drawn
struct vissprite_t {
    int32_t                 viewx;      // Viewspace x position
    fixed_t                 scale;      // Scale due to perspective
    VmPtr<mobj_t>           thing;      // The thing
    VmPtr<vissprite_t>      next;       // Next in the list of sprites
};

static_assert(sizeof(vissprite_t) == 16);

// This is the maximum number of vissprites that can be drawn per subsector.
// Any more than this will simply be ignored.
static constexpr int32_t MAXVISSPRITES = 64;

// The linked list of draw sprites (sorted back to front) for the current subsector and head of the draw list.
// The head is a dummy vissprite which is not actually drawn and vorks in a similar fashion to the head of the map objects list.
static const VmPtr<vissprite_t[MAXVISSPRITES]> gVisSprites(0x800A8A7C);
static const VmPtr<vissprite_t[MAXVISSPRITES]> gVisSpriteHead(0x80096D6C);

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws all of the sprites in a subsector, back to front
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsectorSprites(subsector_t& subsec) noexcept {
    // Initially the linked list of draw sprites is empty, cap it off as such
    gVisSpriteHead->next = gVisSpriteHead.get();
    
    // Run through all the things in the current sector and add things in this subsector to the draw list
    sector_t& sector = *subsec.sector;
    int32_t numDrawSprites = 0;

    {
        vissprite_t* pVisSprite = gVisSprites.get();

        for (mobj_t* pThing = sector.thinglist.get(); pThing; pThing = pThing->snext.get()) {
            // Only draw the thing if it's in the subsector we are interested in
            if (pThing->subsector != &subsec)
                continue;

            // Get the sprite's viewspace position
            VECTOR viewpos;

            {
                SVECTOR worldpos = {
                    (int16_t)((pThing->x - gViewX) >> FRACBITS),
                    0,
                    (int16_t)((pThing->y - gViewY) >> FRACBITS)
                };

                int32_t flagsOut;
                LIBGTE_RotTrans(worldpos, viewpos, flagsOut);
            }

            // If the sprite is too close the near or left/right clip planes (respectively) then ignore.
            // Note that the near plane clipping is more aggressive here than elsewhere, but left/right clipping is far more lenient
            // since it increases the view frustrum FOV to around 126 degrees for the purposes of culling sprite positions:
            if (viewpos.vz < NEAR_CLIP_DIST * 2)
                continue;

            {
                const int32_t leftRightClipX = viewpos.vz * 2;

                if (viewpos.vx < -leftRightClipX)
                    continue;

                if (viewpos.vx > leftRightClipX)
                    continue;
            }
        
            // Sprite is not offscreen! Save viewspace position, scale due to perspective, and it's thing.
            pVisSprite->viewx = viewpos.vx;
            pVisSprite->scale = (HALF_SCREEN_W * FRACUNIT) / viewpos.vz;
            pVisSprite->thing = pThing;

            // Find the vissprite in the linked list to insert the new sprite AFTER.
            // This will be first sprite for which the next sprite is bigger than the new sprite we are inserting.
            // This method basically sorts the sprites from back to front, with sprites at the back being first in the draw list:
            vissprite_t* pInsertPt = gVisSpriteHead.get();

            {
                vissprite_t* pNextSpr = pInsertPt->next.get();

                while (pNextSpr != gVisSpriteHead.get()) {
                    if (pNextSpr->scale >= pVisSprite->scale)
                        break;

                    pInsertPt = pNextSpr;
                    pNextSpr = pNextSpr->next.get();
                }
            }

            // Add the sprite into the linked list at the insertion point and move onto the next sprite slot
            pVisSprite->next = pInsertPt->next;
            pInsertPt->next = pVisSprite;
            ++numDrawSprites;
            ++pVisSprite;

            // If we have exceeded the sprite limit then do not draw any more!
            if (numDrawSprites >= MAXVISSPRITES)
                break;
        }
    }

    // If there is nothing to draw then we are done
    if (numDrawSprites == 0)
        return;

    // Clear the texture window to disable wrapping
    {
        RECT texWinRect;
        LIBGPU_setRECT(texWinRect, 0, 0, 0, 0);

        DR_TWIN* const pTexWinPrim = (DR_TWIN*) LIBETC_getScratchAddr(128);
        LIBGPU_SetTexWindow(*pTexWinPrim, texWinRect);
        I_AddPrim(pTexWinPrim);
    }
    
    // Initialize the quad primitive used to draw sprites
    POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
    LIBGPU_SetPolyFT4(polyPrim);
    polyPrim.clut = *g3dViewPaletteClutId;

    // Draw all the sprites in the draw list for the subsector
    for (const vissprite_t* pSpr = gVisSpriteHead->next.get(); pSpr != gVisSpriteHead.get(); pSpr = pSpr->next.get()) {
        // Grab the sprite frame to use
        const mobj_t& thing = *pSpr->thing;
        const spritedef_t& spriteDef = gSprites[thing.sprite];
        const spriteframe_t& frame = spriteDef.spriteframes[thing.frame & FF_FRAMEMASK];
        
        // Decide on which sprite lump to use and whether the sprite is flipped.
        // If the frame supports rotations then decide on the exact orientation to use, otherwise use the default.
        int32_t lumpNum;
        bool bFlipSpr;

        if (frame.rotate) {
            const angle_t angToThing = R_PointToAngle2(gViewX, gViewY, thing.x, thing.y);
            const uint32_t dirIdx = (angToThing - thing.angle + (ANG45 / 2) * 9) >> 29;     // Note: same calculation as PC Doom

            lumpNum = frame.lump[dirIdx];
            bFlipSpr = frame.flip[dirIdx];
        } else {
            lumpNum = frame.lump[0];
            bFlipSpr = frame.flip[0];
        }

        // Upload the sprite texture to VRAM if not already uploaded
        const int32_t sprIndex = lumpNum - *gFirstSpriteLumpNum;
        texture_t& tex = (*gpSpriteTextures)[sprIndex];
        I_CacheTex(tex);

        // Get the 3 blending flags and set whether the sprite is semi transparent
        const int32_t blendFlags = (thing.flags & MF_ALL_BLEND_FLAGS) >> 28;
        
        if (blendFlags != 0) {  // Minor logic bug? Should be testing against 'MF_BLEND_ON' instead?
            LIBGPU_SetSemiTrans(&polyPrim, true);
        } else {
            LIBGPU_SetSemiTrans(&polyPrim, false);
        }

        // Apply the desired semi transparency mode and discard the 'MF_BLEND_ON' bit since it is not defining a blend mode.
        // Note: this is encoded/packed into the texture page id.
        polyPrim.tpage = tex.texPageId | LIBGPU_getTPage(0, blendFlags >> 1, 0, 0);

        // Set draw color
        if (thing.frame & FF_FULLBRIGHT) {
            LIBGPU_setRGB0(polyPrim, LIGHT_INTENSTIY_MAX, LIGHT_INTENSTIY_MAX, LIGHT_INTENSTIY_MAX);
        } else {
            LIBGPU_setRGB0(polyPrim, (uint8_t) gCurLightValR, (uint8_t) gCurLightValG, (uint8_t) gCurLightValB);
        }

        // Figure out the y position and width + height for the sprite.
        //
        // The 4/5 scaling of sprite width here is very interesting: this may have been an attempt to perhaps account for the fact
        // that the original artwork was designed for non square pixels that were around 1.2 times taller than they were wide, so
        // on a system where pixels were square (regular TVs?) this could make the artwork seem wider than originally intended.
        // For more on this topic see 'GAME ENGINE BLACK BOOK: DOOM - 5.12.12 Sprite aspect ratio'.
        //
        // In any case the effect of the adjustment is largely nullified by the fact that the game stretches the 256 pixel wide
        // framebuffer to roughly 293 pixels, so sprites like the Cacodemon still appear mostly round rather than elliptical.
        // Oh well, points for effort at least!
        constexpr fixed_t ASPECT_CORRECT = (FRACUNIT * 4) / 5;
        
        const fixed_t scale = pSpr->scale;
        int32_t drawY = (thing.z - gViewZ) >> FRACBITS;
        drawY += tex.offsetY;
        drawY = (drawY * -scale) >> FRACBITS;   // Scale due to perspective
        drawY += HALF_VIEW_3D_H;

        int32_t drawW = (tex.width * ASPECT_CORRECT) >> FRACBITS;
        drawW = (drawW * scale) >> FRACBITS;

        const int32_t drawH = (tex.height * scale) >> FRACBITS;

        // Sprite UV coordinates for left, right, top and bottom
        const uint8_t tex_ul = tex.texPageCoordX;
        const uint8_t tex_ur = tex.texPageCoordX + (uint8_t) tex.width - 1;
        const uint8_t tex_vt = tex.texPageCoordY;
        const uint8_t tex_vb = tex.texPageCoordY + (uint8_t) tex.height - 1;

        // Set sprite UV coordinates and decide on draw x position
        const int32_t texOffsetX = (tex.offsetX * ASPECT_CORRECT) >> FRACBITS;
        int32_t drawX;

        if (!bFlipSpr) {
            LIBGPU_setUV4(polyPrim,
                tex_ul, tex_vt,
                tex_ur, tex_vt,
                tex_ul, tex_vb,
                tex_ur, tex_vb
            );

            drawX = ((pSpr->viewx - texOffsetX) * scale) >> FRACBITS;
            drawX += HALF_SCREEN_W;
        } else {
            LIBGPU_setUV4(polyPrim,
                tex_ur, tex_vt,
                tex_ul, tex_vt,
                tex_ur, tex_vb,
                tex_ul, tex_vb
            );

            drawX = ((pSpr->viewx + texOffsetX) * scale) >> FRACBITS;
            drawX += HALF_SCREEN_W - drawW;
        }

        // Finally set the vertexes position and submit the quad
        const int16_t pos_lx = (int16_t)(drawX);
        const int16_t pos_rx = (int16_t)(drawX + drawW);
        const int16_t pos_ty = (int16_t)(drawY);
        const int16_t pos_by = (int16_t)(drawY + drawH);

        LIBGPU_setXY4(polyPrim,
            pos_lx, pos_ty,
            pos_rx, pos_ty,
            pos_lx, pos_by,
            pos_rx, pos_by
        );

        I_AddPrim(&polyPrim);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the player's weapon sprites and muzzle flash
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawWeapon() noexcept {
    // Run through all of the player sprites for the view player and render
    player_t& player = *gpViewPlayer;
    pspdef_t* pSprite = player.psprites;
    
    for (int32_t pspIdx = 0; pspIdx < NUMPSPRITES; ++pspIdx, ++pSprite) {
        // Is this particular player sprite slot showing anything?
        if (!pSprite->state)
            continue;

        // Get the texture for the sprite and upload to VRAM if required
        const state_t& state = *pSprite->state;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const int32_t frameNum = state.frame & FF_FRAMEMASK;
        const spriteframe_t& frame = spriteDef.spriteframes[frameNum];
        const int32_t texNum = frame.lump[0] - *gFirstSpriteLumpNum;

        texture_t& tex = (*gpSpriteTextures)[texNum];
        I_CacheTex(tex);

        // Setup the default drawing mode to disable wrapping (remove texture window).
        // This code also sets blending options and encodes that in the texture page id.
        bool bIsTransparent;

        {
            RECT texWin = { 0, 0, 0, 0 };
            bIsTransparent = ((player.mo->flags & MF_ALL_BLEND_FLAGS) != 0);    // Minor logic bug? Should be testing against 'MF_BLEND_ON' instead?
            const uint16_t texPageId = tex.texPageId | LIBGPU_getTPage(0, (bIsTransparent) ? 1 : 0, 0, 0);

            DR_MODE& drawMode = *(DR_MODE*) LIBETC_getScratchAddr(128);
            LIBGPU_SetDrawMode(drawMode, false, false, texPageId, &texWin);
            I_AddPrim(&drawMode);
        }

        // Setup the sprite to be drawn and submit the drawing primitive.
        // Set transparency, size, UV coords, palette and color before drawing.
        SPRT& spr = *(SPRT*) LIBETC_getScratchAddr(128);
        LIBGPU_SetSprt(spr);

        if (bIsTransparent) {
            LIBGPU_SetSemiTrans(&spr, true);
        }

        LIBGPU_setXY0(spr,
            (int16_t)((pSprite->sx >> FRACBITS) + HALF_SCREEN_W - tex.offsetX),
            (int16_t)((pSprite->sy >> FRACBITS) + VIEW_3D_H - 1 - tex.offsetY)
        );

        LIBGPU_setWH(spr, tex.width, tex.height);
        LIBGPU_setUV0(spr, tex.texPageCoordX, tex.texPageCoordY);
        spr.clut = *g3dViewPaletteClutId;

        if (state.frame & FF_FULLBRIGHT) {
            // Note: these magic 5/8 multipliers correspond VERY closely to 'LIGHT_INTENSTIY_MAX / 255'.
            // The resulting values are sometimes not quite the same however.
            const light_t& light = *gpCurLight;

            LIBGPU_setRGB0(spr,
                (uint8_t)(((uint32_t) light.r * 5) / 8),
                (uint8_t)(((uint32_t) light.g * 5) / 8),
                (uint8_t)(((uint32_t) light.b * 5) / 8)
            );
        } else {
            LIBGPU_setRGB0(spr,
                (uint8_t) gCurLightValR,
                (uint8_t) gCurLightValG,
                (uint8_t) gCurLightValB
            );
        }
        
        I_AddPrim(&spr);
    }
}

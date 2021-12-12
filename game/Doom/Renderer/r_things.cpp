#include "r_things.h"

#include "Asserts.h"
#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/sprinfo.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

// Describes a sprite that is to be drawn
struct vissprite_t {
    int32_t         viewx;      // Viewspace x position
    fixed_t         scale;      // Scale due to perspective
    mobj_t*         thing;      // The thing
    vissprite_t*    next;       // Next in the list of sprites
};

// This is the maximum number of vissprites that can be drawn per subsector - any more than this will simply be ignored.
// Should be enough for even the most extreme situations; if we go much higher then sprite sorting times would become super slow!
#if PSYDOOM_LIMIT_REMOVING
    static constexpr int32_t MAXVISSPRITES = 8192;
#else
    static constexpr int32_t MAXVISSPRITES = 64;
#endif

// The linked list of draw sprites (sorted back to front) for the current subsector and head of the draw list.
// The head is a dummy vissprite which is not actually drawn and vorks in a similar fashion to the head of the map objects list.
static vissprite_t  gVisSprites[MAXVISSPRITES];
static vissprite_t  gVisSpriteHead;

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: sets up the final XY and UV coordinates for a sprite quad, after clipping against the screen edges.
// The clipping is done to avoid the sprite going too much offscreen, and thus running into PlayStation GPU limitations.
// If the X distance between vertices exceeds '1023' or Y distance exceeds '511' then the quad won't render; clipping prevents that.
//------------------------------------------------------------------------------------------------------------------------------------------
static void R_ClipSprite(
    POLY_FT4& poly,
    int16_t xl,
    int16_t xr,
    int16_t yt,
    int16_t yb,
    LibGpuUV ul,
    LibGpuUV ur,
    LibGpuUV vt,
    LibGpuUV vb
) noexcept {
    ASSERT(xl <= xr);
    ASSERT(yt <= yb);
    int32_t spriteW = xr - xl;
    int32_t spriteH = yb - yt;

    // Too much offscreen at the top?
    if ((spriteH > 500) && (yt < 0)) {
        const int32_t offscreenAmt = -yt;
        const fixed_t offscreenPercent = (offscreenAmt << FRACBITS) / spriteH;
        yt = 0;
        vt = (LibGpuUV) d_fixed_to_int(R_LerpCoord(vt << FRACBITS, vb << FRACBITS, offscreenPercent) + 0x8000);
        spriteH -= offscreenAmt;
    }

    // Too much offscreen at the bottom?
    if ((spriteH > 500) && (yb > VIEW_3D_H)) {
        const int32_t offscreenAmt = yb - VIEW_3D_H;
        const fixed_t offscreenPercent = (offscreenAmt << FRACBITS) / spriteH;
        yb = VIEW_3D_H;
        vb = (LibGpuUV) d_fixed_to_int(R_LerpCoord(vb << FRACBITS, vt << FRACBITS, offscreenPercent) + 0x8000);
    }

    // Too much offscreen to the left?
    if ((spriteW > 1000) && (xl < 0)) {
        const int32_t offscreenAmt = -xl;
        const fixed_t offscreenPercent = (offscreenAmt << FRACBITS) / spriteW;
        xl = 0;
        ul = (LibGpuUV) d_fixed_to_int(R_LerpCoord(ul << FRACBITS, ur << FRACBITS, offscreenPercent) + 0x8000);
        spriteW -= offscreenAmt;
    }

    // Too much offscreen to the right?
    if ((spriteW > 1000) && (xr > SCREEN_W)) {
        const int32_t offscreenAmt = xr - SCREEN_W;
        const fixed_t offscreenPercent = (offscreenAmt << FRACBITS) / spriteW;
        xr = SCREEN_W;
        ur = (LibGpuUV) d_fixed_to_int(R_LerpCoord(ur << FRACBITS, ul << FRACBITS, offscreenPercent) + 0x8000);
    }

    // Output the final (clipped) polygon
    poly.x0 = xl;   poly.y0 = yt;
    poly.x1 = xr;   poly.y1 = yt;
    poly.x2 = xl;   poly.y2 = yb;
    poly.x3 = xr;   poly.y3 = yb;

    poly.u0 = ul;   poly.v0 = vt;
    poly.u1 = ur;   poly.v1 = vt;
    poly.u2 = ul;   poly.v2 = vb;
    poly.u3 = ur;   poly.v3 = vb;
}
#endif  // #if PSYDOOM_LIMIT_REMOVING

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws all of the sprites in a subsector, back to front
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsectorSprites(subsector_t& subsec) noexcept {
    // Initially the linked list of draw sprites is empty, cap it off as such
    gVisSpriteHead.next = &gVisSpriteHead;

    // Run through all the things in the current sector and add things in this subsector to the draw list
    sector_t& sector = *subsec.sector;
    int32_t numDrawSprites = 0;

    {
        vissprite_t* pVisSprite = gVisSprites;

        for (mobj_t* pThing = sector.thinglist; pThing; pThing = pThing->snext) {
            // Only draw the thing if it's in the subsector we are interested in
            if (pThing->subsector != &subsec)
                continue;

            // Psydoom: ignore the thing if in state 'S_NULL' (needed to avoid rendering PsyDoom's new 'marker' things)
            #if PSYDOOM_MODS
                if (pThing->state == &gStates[S_NULL])
                    continue;
            #endif

            // PsyDoom: don't draw this player's thing (exception: allow if the external camera is active)
            #if PSYDOOM_MODS
                if (pThing->player && (pThing->player == &gPlayers[gCurPlayerIndex]) && (gExtCameraTicsLeft <= 0))
                    continue;
            #endif

            // Get the sprite's viewspace position
            VECTOR viewpos;

            {
                // PsyDoom: account for interpolation if that is enabled
                #if PSYDOOM_MODS
                    const fixed_t thingX = pThing->x.renderValue();
                    const fixed_t thingY = pThing->y.renderValue();
                #else
                    const fixed_t thingX = pThing->x;
                    const fixed_t thingY = pThing->y;
                #endif

                SVECTOR worldpos = {
                    (int16_t) d_fixed_to_int(thingX - gViewX),
                    0,
                    (int16_t) d_fixed_to_int(thingY - gViewY)
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
            vissprite_t* pInsertPt = &gVisSpriteHead;

            {
                vissprite_t* pNextSpr = pInsertPt->next;

                while (pNextSpr != &gVisSpriteHead) {
                    if (pNextSpr->scale >= pVisSprite->scale)
                        break;

                    pInsertPt = pNextSpr;
                    pNextSpr = pNextSpr->next;
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
        SRECT texWinRect;
        LIBGPU_setRECT(texWinRect, 0, 0, 0, 0);

        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_TWIN texWinPrim = {};
        #else
            DR_TWIN& texWinPrim = *(DR_TWIN*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetTexWindow(texWinPrim, texWinRect);
        I_AddPrim(texWinPrim);
    }

    // Initialize the quad primitive used to draw sprites
    #if PSYDOOM_MODS
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        POLY_FT4 polyPrim = {};
    #else
        POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
    #endif

    LIBGPU_SetPolyFT4(polyPrim);
    polyPrim.clut = g3dViewPaletteClutId;

    // Draw all the sprites in the draw list for the subsector
    for (const vissprite_t* pSpr = gVisSpriteHead.next; pSpr != &gVisSpriteHead; pSpr = pSpr->next) {
        // Grab the sprite frame to use
        mobj_t& thing = *pSpr->thing;
        const spritedef_t& spriteDef = gSprites[thing.sprite];
        const spriteframe_t& frame = spriteDef.spriteframes[thing.frame & FF_FRAMEMASK];

        // Decide on which sprite lump to use and whether the sprite is flipped.
        // If the frame supports rotations then decide on the exact orientation to use, otherwise use the default.
        int32_t lumpIdx;
        bool bFlipSpr;

        if (frame.rotate) {
            // PsyDoom: account for interpolation if that is enabled
            #if PSYDOOM_MODS
                const fixed_t thingX = thing.x.renderValue();
                const fixed_t thingY = thing.y.renderValue();
            #else
                const fixed_t thingX = thing.x;
                const fixed_t thingY = thing.y;
            #endif

            const angle_t angToThing = R_PointToAngle2(gViewX, gViewY, thingX, thingY);
            const uint32_t dirIdx = (angToThing - thing.angle + (ANG45 / 2) * 9) >> 29;     // Note: same calculation as PC Doom

            lumpIdx = frame.lump[dirIdx];
            bFlipSpr = frame.flip[dirIdx];
        } else {
            lumpIdx = frame.lump[0];
            bFlipSpr = frame.flip[0];
        }

        // Upload the sprite texture to VRAM if not already uploaded.
        // PsyDoom: updates for changes to how textures are managed (changes that help support user modding).
        #if PSYDOOM_MODS
            texture_t& tex = R_GetTexForLump(lumpIdx);
        #else
            const int32_t sprIndex = lumpIdx - gFirstSpriteLumpNum;
            texture_t& tex = gpSpriteTextures[sprIndex];
        #endif

        I_CacheTex(tex);

        // Get the 3 blending flags and set whether the sprite is semi transparent
        const uint32_t blendFlags = (thing.flags & MF_ALL_BLEND_FLAGS) >> 28;

        if (blendFlags != 0) {  // Minor logic bug? Should be testing against 'MF_BLEND_ON' instead?
            LIBGPU_SetSemiTrans(polyPrim, true);
        } else {
            LIBGPU_SetSemiTrans(polyPrim, false);
        }

        // Apply the desired semi transparency mode and discard the 'MF_BLEND_ON' bit since it is not defining a blend mode.
        // Note: this is encoded/packed into the texture page id.
        polyPrim.tpage = tex.texPageId | LIBGPU_GetTPageSemiTransBits(blendFlags >> 1);

        // Set draw color.
        // PsyDoom: adding support for uncapped framerate interpolation for the thing Z value.
        #if PSYDOOM_MODS
            const fixed_t thingZ = thing.z.renderValue();
        #else
            const fixed_t thingZ = thing.z;
        #endif

        if (thing.frame & FF_FULLBRIGHT) {
            LIBGPU_setRGB0(polyPrim, LIGHT_INTENSTIY_MAX, LIGHT_INTENSTIY_MAX, LIGHT_INTENSTIY_MAX);
        } else {
            // PsyDoom: need to account for dual colored lighting now when setting the thing color
            #if PSYDOOM_MODS
                uint8_t sectorR, sectorG, sectorB;
                R_GetSectorDrawColor(sector, thingZ, sectorR, sectorG, sectorB);
                LIBGPU_setRGB0(polyPrim, sectorR, sectorG, sectorB);
            #else
                LIBGPU_setRGB0(polyPrim, (uint8_t) gCurLightValR, (uint8_t) gCurLightValG, (uint8_t) gCurLightValB);
            #endif
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

        #if PSYDOOM_MODS
            // PsyDoom: add support for interpolation when using uncapped framerates
            int32_t drawY = d_fixed_to_int(thingZ - gViewZ);
        #else
            int32_t drawY = d_fixed_to_int(thing.z - gViewZ);
        #endif

        drawY += tex.offsetY;
        drawY = d_fixed_to_int(drawY * -scale);     // Scale due to perspective
        drawY += HALF_VIEW_3D_H;

        int32_t drawW = d_fixed_to_int(tex.width * ASPECT_CORRECT);
        drawW = d_fixed_to_int(drawW * scale);

        const int32_t drawH = d_fixed_to_int(tex.height * scale);

        // Sprite UV coordinates for left, right, top and bottom.
        // PsyDoom: corrected UV coordinates to slight pixel stretching and use 16-bit coords in case of overflow.
        #if PSYDOOM_LIMIT_REMOVING
            const LibGpuUV tex_ul = (LibGpuUV)(tex.texPageCoordX);
            const LibGpuUV tex_ur = (LibGpuUV)(tex.texPageCoordX + tex.width);
            const LibGpuUV tex_vt = (LibGpuUV)(tex.texPageCoordY);
            const LibGpuUV tex_vb = (LibGpuUV)(tex.texPageCoordY + tex.height);
        #else
            const LibGpuUV tex_ul = tex.texPageCoordX;
            const LibGpuUV tex_ur = tex.texPageCoordX + (uint8_t) tex.width - 1;
            const LibGpuUV tex_vt = tex.texPageCoordY;
            const LibGpuUV tex_vb = tex.texPageCoordY + (uint8_t) tex.height - 1;
        #endif

        // Set sprite UV coordinates and decide on draw x position
        const int32_t texOffsetX = d_fixed_to_int(tex.offsetX * ASPECT_CORRECT);
        int32_t drawX;

        if (!bFlipSpr) {
            LIBGPU_setUV4(polyPrim,
                tex_ul, tex_vt,
                tex_ur, tex_vt,
                tex_ul, tex_vb,
                tex_ur, tex_vb
            );

            drawX = d_fixed_to_int((pSpr->viewx - texOffsetX) * scale);
            drawX += HALF_SCREEN_W;
        } else {
            LIBGPU_setUV4(polyPrim,
                tex_ur, tex_vt,
                tex_ul, tex_vt,
                tex_ur, tex_vb,
                tex_ul, tex_vb
            );

            drawX = d_fixed_to_int((pSpr->viewx + texOffsetX) * scale);
            drawX += HALF_SCREEN_W - drawW;
        }

        // Finally set the vertexes position and submit the quad
        const int16_t pos_lx = (int16_t)(drawX);
        const int16_t pos_rx = (int16_t)(drawX + drawW);
        const int16_t pos_ty = (int16_t)(drawY);
        const int16_t pos_by = (int16_t)(drawY + drawH);

        #if PSYDOOM_LIMIT_REMOVING
            // PsyDoom limit removing: clip the quad so that tall sprites (e.g tech columns) which are up close do not vanish.
            // In the original PSX engine hardware limits could sometimes be exceeded and due to the distances between sprite vertexes being too great.
            // Avoid exceeding these GPU limitations via clipping...
            R_ClipSprite(polyPrim, pos_lx, pos_rx, pos_ty, pos_by, polyPrim.u0, polyPrim.u1, polyPrim.v0, polyPrim.v2);
        #else
            LIBGPU_setXY4(polyPrim,
                pos_lx, pos_ty,
                pos_rx, pos_ty,
                pos_lx, pos_by,
                pos_rx, pos_by
            );
        #endif

        I_AddPrim(polyPrim);
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
        // PsyDoom: made updates for changes to how textures are managed (changes that help support user modding).
        const state_t& state = *pSprite->state;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const int32_t frameNum = state.frame & FF_FRAMEMASK;
        const spriteframe_t& frame = spriteDef.spriteframes[frameNum];

        #if PSYDOOM_MODS
            texture_t& tex = R_GetTexForLump(frame.lump[0]);
        #else
            const int32_t texNum = frame.lump[0] - gFirstSpriteLumpNum;
            texture_t& tex = gpSpriteTextures[texNum];
        #endif

        I_CacheTex(tex);

        // Setup the default drawing mode to disable wrapping (remove texture window).
        // This code also sets blending options and encodes that in the texture page id.
        bool bIsTransparent;

        {
            SRECT texWin = { 0, 0, 0, 0 };
            bIsTransparent = ((player.mo->flags & MF_ALL_BLEND_FLAGS) != 0);    // Minor logic bug? Should be testing against 'MF_BLEND_ON' instead?
            const uint16_t texPageId = tex.texPageId | LIBGPU_GetTPageSemiTransBits((bIsTransparent) ? 1 : 0);

            // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
            #if PSYDOOM_MODS
                DR_MODE drawMode = {};
            #else
                DR_MODE& drawMode = *(DR_MODE*) LIBETC_getScratchAddr(128);
            #endif

            LIBGPU_SetDrawMode(drawMode, false, false, texPageId, &texWin);
            I_AddPrim(drawMode);
        }

        // Setup the sprite to be drawn and submit the drawing primitive.
        // Set transparency, size, UV coords, palette and color before drawing.
        #if PSYDOOM_MODS
            // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
            SPRT spr = {};
        #else
            SPRT& spr = *(SPRT*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetSprt(spr);

        if (bIsTransparent) {
            LIBGPU_SetSemiTrans(spr, true);
        }


        #if PSYDOOM_MODS
            // PsyDoom: weapon sway is now interpolated
            const fixed_t sprX = pSprite->sx.renderValue();
            const fixed_t sprY = pSprite->sy.renderValue();
        #else
            const fixed_t sprX = pSprite->sx;
            const fixed_t sprY = pSprite->sy;
        #endif

        LIBGPU_setXY0(spr,
            (int16_t)(d_fixed_to_int(sprX) + HALF_SCREEN_W - tex.offsetX),
            (int16_t)(d_fixed_to_int(sprY) + VIEW_3D_H - 1 - tex.offsetY)
        );

        LIBGPU_setWH(spr, tex.width, tex.height);
        LIBGPU_setUV0(spr, tex.texPageCoordX, tex.texPageCoordY);
        spr.clut = g3dViewPaletteClutId;

        // PsyDoom: changes to account for dual colored lighting
        #if PSYDOOM_MODS
            mobj_t& playerMobj = *player.mo;
            const sector_t& sector = *playerMobj.subsector->sector;
        #endif

        if (state.frame & FF_FULLBRIGHT) {
            // Note: these magic 5/8 multipliers correspond VERY closely to 'LIGHT_INTENSTIY_MAX / 255'.
            // The resulting values are sometimes not quite the same however.
            // PsyDoom: added changes here to account for dual colored lighting and uncapped framerate interpolation.
            #if PSYDOOM_MODS
                const light_t light = R_GetSectorLightColor(sector, playerMobj.z.renderValue());
            #else
                const light_t& light = *gpCurLight;
            #endif

            LIBGPU_setRGB0(spr,
                (uint8_t)(((uint32_t) light.r * 5) / 8),
                (uint8_t)(((uint32_t) light.g * 5) / 8),
                (uint8_t)(((uint32_t) light.b * 5) / 8)
            );
        } else {
            // PsyDoom: changes to account for dual colored lighting and uncapped framerate interpolation
            #if PSYDOOM_MODS
                uint8_t sectorR, sectorG, sectorB;
                R_GetSectorDrawColor(sector, playerMobj.z.renderValue(), sectorR, sectorG, sectorB);
                LIBGPU_setRGB0(spr, sectorR, sectorG, sectorB);
            #else
                LIBGPU_setRGB0(spr, (uint8_t) gCurLightValR, (uint8_t) gCurLightValG, (uint8_t) gCurLightValB);
            #endif
        }

        I_AddPrim(spr);
    }
}

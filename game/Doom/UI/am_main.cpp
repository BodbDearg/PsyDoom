#include "am_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_local.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/RendererVk/rv_automap.h"
#include "PsyDoom/Config.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/Utils.h"
#include "PsyDoom/Video.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

static constexpr fixed_t MOVESTEP   = FRACUNIT * 128;   // Controls how fast manual automap movement happens
static constexpr fixed_t SCALESTEP  = 2;                // How fast to scale in/out
static constexpr int32_t MAXSCALE   = 64;               // Maximum map zoom
static constexpr int32_t MINSCALE   = 8;                // Minimum map zoom

static fixed_t  gAutomapXMin;
static fixed_t  gAutomapXMax;
static fixed_t  gAutomapYMin;
static fixed_t  gAutomapYMax;

// Internal module functions
static void DrawLine(const uint32_t color, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2) noexcept;

#if PSYDOOM_MODS

// The position and rotation to use for the player this frame on the automap, and the free camera position and camera zoom
static fixed_t gAM_PlayerX;
static fixed_t gAM_PlayerY;
static angle_t gAM_PlayerAngle;
static fixed_t gAM_AutomapX;
static fixed_t gAM_AutomapY;
static fixed_t gAM_AutomapScale;

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute the position and rotation to use for the automap for the player, taking into account framerate independent movement.
// Also does the same for the 'free camera' automap position that is used when the player is manually panning over the map.
//------------------------------------------------------------------------------------------------------------------------------------------
static void AM_CalcPlayerMapTransforms() noexcept {
    const player_t& player = gPlayers[gCurPlayerIndex];

    R_CalcLerpFactors();
    const bool bUncapFramerate = Config::gbUncapFramerate;

    if (bUncapFramerate) {
        const fixed_t lerpFactor = gPlayerLerpFactor;
        gAM_PlayerX = R_LerpCoord(gOldViewX, player.mo->x, lerpFactor);
        gAM_PlayerY = R_LerpCoord(gOldViewY, player.mo->y, lerpFactor);
        gAM_PlayerAngle = R_LerpAngle(gOldViewAngle, player.mo->angle, lerpFactor);
        gAM_AutomapX = R_LerpCoord(gOldAutomapX, player.automapx, lerpFactor);
        gAM_AutomapY = R_LerpCoord(gOldAutomapY, player.automapy, lerpFactor);
        gAM_AutomapScale = R_LerpCoord(gOldAutomapScale * FRACUNIT, player.automapscale * FRACUNIT, lerpFactor);
    } else {
        gAM_PlayerX = player.mo->x;
        gAM_PlayerY = player.mo->y;
        gAM_PlayerAngle = player.mo->angle;
        gAM_AutomapX = player.automapx;
        gAM_AutomapY = player.automapy;
        gAM_AutomapScale = player.automapscale * FRACUNIT;
    }
}

#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Automap initialization logic
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Start() noexcept {
    gAutomapXMin = gBlockmapOriginX;
    gAutomapYMin = gBlockmapOriginY;
    gAutomapXMax = d_lshift<MAPBLOCKSHIFT>(gBlockmapWidth) + gBlockmapOriginX;
    gAutomapYMax = d_lshift<MAPBLOCKSHIFT>(gBlockmapHeight) + gBlockmapOriginY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the automap: handles player input & controls
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Control(player_t& player) noexcept {
    // If the game is paused we do nothing
    if (gbGamePaused)
        return;

    // PsyDoom: if the external camera is active do nothing and clear the current automap flags
    #if PSYDOOM_MODS
        if (gExtCameraTicsLeft > 0) {
            player.automapflags &= ~AF_ACTIVE;
            return;
        }
    #endif

    // Toggle the automap on and off if select has just been pressed
    #if PSYDOOM_MODS
        TickInputs& inputs = gTickInputs[gPlayerNum];
        const TickInputs& oldInputs = gOldTickInputs[gPlayerNum];

        const bool bMenuBack = (inputs.bToggleMap && (!oldInputs.bToggleMap));
        const bool bAutomapPan = inputs.bAutomapPan;
        const bool bPanFast = inputs.bRun;
        const bool bAutomapMoveLeft = inputs.bAutomapMoveLeft;
        const bool bAutomapMoveRight = inputs.bAutomapMoveRight;
        const bool bAutomapMoveUp = inputs.bAutomapMoveUp;
        const bool bAutomapMoveDown = inputs.bAutomapMoveDown;
        const bool bAutomapZoomIn = inputs.bAutomapZoomIn;
        const bool bAutomapZoomOut = inputs.bAutomapZoomOut;
    #else
        const padbuttons_t ticButtons = gTicButtons[gPlayerNum];
        const padbuttons_t oldTicButtons = gOldTicButtons[gPlayerNum];

        const bool bMenuBack = ((ticButtons & PAD_SELECT) && ((oldTicButtons & PAD_SELECT) == 0));
        const bool bAutomapPan = (ticButtons & PAD_CROSS);
        const bool bPanFast = (ticButtons & PAD_SQUARE);
        const bool bAutomapMoveLeft = (ticButtons & PAD_LEFT);
        const bool bAutomapMoveRight = (ticButtons & PAD_RIGHT);
        const bool bAutomapMoveUp = (ticButtons & PAD_UP);
        const bool bAutomapMoveDown = (ticButtons & PAD_DOWN);
        const bool bAutomapZoomIn = (ticButtons & PAD_L1);
        const bool bAutomapZoomOut = (ticButtons & PAD_R1);
    #endif

    if (bMenuBack) {
        player.automapflags ^= AF_ACTIVE;
        player.automapx = player.mo->x;
        player.automapy = player.mo->y;
    }

    // If the automap is not active or the player dead then do nothing
    if ((player.automapflags & AF_ACTIVE) == 0)
        return;

    if (player.playerstate != PST_LIVE)
        return;

    // Follow the player unless the cross button is pressed.
    // The rest of the logic is for when we are NOT following the player.
    if (!bAutomapPan) {
        player.automapflags &= ~AF_FOLLOW;
        return;
    }

    // Snap the manual automap movement position to the player location once we transition from following to not following
    if ((player.automapflags & AF_FOLLOW) == 0) {
        player.automapflags |= AF_FOLLOW;
        player.automapx = player.mo->x;
        player.automapy = player.mo->y;
    }

    // Figure out the movement amount for manual camera movement
    const fixed_t moveStep = (bPanFast) ? MOVESTEP * 2 : MOVESTEP;

    // Not sure why this check was done, it can never be true due to the logic above.
    // PsyDoom: remove this block as it is useless...
    #if !PSYDOOM_MODS
        if ((player.automapflags & AF_FOLLOW) == 0)
            return;
    #endif

    // Left/right movement
    if (bAutomapMoveRight) {
        player.automapx += moveStep;

        if (player.automapx > gAutomapXMax) {
            player.automapx = gAutomapXMax;
        }
    }
    else if (bAutomapMoveLeft) {
        player.automapx -= moveStep;

        if (player.automapx < gAutomapXMin) {
            player.automapx = gAutomapXMin;
        }
    }

    // Up/down movement
    if (bAutomapMoveUp) {
        player.automapy += moveStep;

        if (player.automapy > gAutomapYMax) {
            player.automapy = gAutomapYMax;
        }
    }
    else if (bAutomapMoveDown) {
        player.automapy -= moveStep;

        if (player.automapy < gAutomapYMin) {
            player.automapy = gAutomapYMin;
        }
    }

    // Scale up and down
    if (bAutomapZoomOut) {
        player.automapscale -= SCALESTEP;

        if (player.automapscale < MINSCALE) {
            player.automapscale = MINSCALE;
        }
    }
    else if (bAutomapZoomIn) {
        player.automapscale += SCALESTEP;

        if (player.automapscale > MAXSCALE) {
            player.automapscale = MAXSCALE;
        }
    }

    // When not in follow mode, consume these inputs so that we don't move the player in the level
    #if PSYDOOM_MODS
        inputs.bMoveForward = false;
        inputs.bMoveBackward = false;
        inputs.bAttack = false;
        inputs.bTurnLeft = false;
        inputs.bTurnRight = false;
        inputs.bStrafeLeft = false;
        inputs.bStrafeRight = false;
        inputs.analogForwardMove = 0;
        inputs.analogSideMove = 0;
        inputs.analogTurn = 0;
    #else
        gTicButtons[gPlayerNum] &= ~(PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_R1 | PAD_L1);
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does drawing for the automap
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Drawer() noexcept {
    // Finish up the previous frame.
    // PsyDoom: moved this to the end of 'P_Drawer' instead - needed for the new Vulkan renderer integration.
    #if !PSYDOOM_MODS
        I_DrawPresent();
    #endif

    // PsyDoom: if the Vulkan renderer is active then delegate automap drawing to that.
    // Otherwise compute the player map transforms to use, taking into account framerate independent movement.
    #if PSYDOOM_MODS
        #if PSYDOOM_VULKAN_RENDERER
            if (Video::isUsingVulkanRenderPath()) {
                RV_DrawAutomap();
                return;
            }
        #endif

        AM_CalcPlayerMapTransforms();
    #endif

    // Determine the scale to render the map at.
    // PsyDoom: use framerate uncapped scaling here.
    const player_t& curPlayer = gPlayers[gCurPlayerIndex];

    #if PSYDOOM_MODS
        const fixed_t scale = gAM_AutomapScale;
    #else
        const int32_t scale = curPlayer.automapscale;
    #endif

    // Determine the map camera origin depending on follow mode status.
    // PsyDoom: use framerate uncapped positions here.
    fixed_t ox, oy;

    #if PSYDOOM_MODS
        if (curPlayer.automapflags & AF_FOLLOW) {
            ox = gAM_AutomapX;
            oy = gAM_AutomapY;
        } else {
            ox = gAM_PlayerX;
            oy = gAM_PlayerY;
        }
    #else
        if (curPlayer.automapflags & AF_FOLLOW) {
            ox = curPlayer.automapx;
            oy = curPlayer.automapy;
        } else {
            ox = curPlayer.mo->x;
            oy = curPlayer.mo->y;
        }
    #endif

    // Draw all the map lines
    {
        const line_t* pLine = gpLines;

        for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx, ++pLine) {
            // See whether we should draw the automap line or not
            const bool bHiddenLine = (pLine->flags & ML_DONTDRAW);
            const bool bLineSeen = ((pLine->flags & ML_MAPPED) && (!bHiddenLine));

            #if PSYDOOM_MODS
                // PsyDoom: bug fix: if the line is marked as invisible then the allmap powerup shouldn't reveal it.
                // This change makes the behavior consistent with Linux Doom.
                const bool bLineMapped = ((curPlayer.powers[pw_allmap]) && (!bHiddenLine));
            #else
                const bool bLineMapped = (curPlayer.powers[pw_allmap]);
            #endif

            const bool bAllLinesCheatOn = (curPlayer.cheats & CF_ALLLINES);
            const bool bDraw = (bLineSeen || bLineMapped || bAllLinesCheatOn);

            if (!bDraw)
                continue;

            // Compute the line points in viewspace.
            // PsyDoom: scale is now a fixed point number due to framerate uncapped automap movement.
            #if PSYDOOM_MODS
                const int32_t x1 = d_fixed_to_int(FixedMul((pLine->vertex1->x - ox) / SCREEN_W, scale));
                const int32_t y1 = d_fixed_to_int(FixedMul((pLine->vertex1->y - oy) / SCREEN_W, scale));
                const int32_t x2 = d_fixed_to_int(FixedMul((pLine->vertex2->x - ox) / SCREEN_W, scale));
                const int32_t y2 = d_fixed_to_int(FixedMul((pLine->vertex2->y - oy) / SCREEN_W, scale));
            #else
                const int32_t x1 = d_fixed_to_int(((pLine->vertex1->x - ox) / SCREEN_W) * scale);
                const int32_t y1 = d_fixed_to_int(((pLine->vertex1->y - oy) / SCREEN_W) * scale);
                const int32_t x2 = d_fixed_to_int(((pLine->vertex2->x - ox) / SCREEN_W) * scale);
                const int32_t y2 = d_fixed_to_int(((pLine->vertex2->y - oy) / SCREEN_W) * scale);
            #endif

            // Decide on line color: start off with the normal two sided line color to begin with
            uint32_t color = AM_COLOR_BROWN;

            if (((curPlayer.cheats & CF_ALLLINES) + curPlayer.powers[pw_allmap] != 0) && ((pLine->flags & ML_MAPPED) == 0)) {
                // A known line (due to all map cheat/powerup) but unseen
                color = AM_COLOR_GREY;
            }
            else if (pLine->flags & ML_SECRET) {
                // Secret
                color = AM_COLOR_RED;
            }
            else if (pLine->special != 0) {
                // Special or activatable thing
                color = AM_COLOR_YELLOW;
            }
            else if ((pLine->flags & ML_TWOSIDED) == 0) {
                // One sided line
                color = AM_COLOR_RED;
            }

            DrawLine(color, x1, y1, x2, y2);
        }
    }

    // Show all map things cheat: display a little wireframe triangle for for all things
    if (curPlayer.cheats & CF_ALLMOBJ) {
        for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
            // Ignore the player for this particular draw
            if (pMobj == curPlayer.mo)
                continue;

            // Compute the the sine and cosines for the angles of the 3 points in the triangle
            const uint32_t fineAng1 = (pMobj->angle                ) >> ANGLETOFINESHIFT;
            const uint32_t fineAng2 = (pMobj->angle - ANG90 - ANG45) >> ANGLETOFINESHIFT;
            const uint32_t fineAng3 = (pMobj->angle + ANG90 + ANG45) >> ANGLETOFINESHIFT;

            const fixed_t cos1 = gFineCosine[fineAng1];
            const fixed_t cos2 = gFineCosine[fineAng2];
            const fixed_t cos3 = gFineCosine[fineAng3];

            const fixed_t sin1 = gFineSine[fineAng1];
            const fixed_t sin2 = gFineSine[fineAng2];
            const fixed_t sin3 = gFineSine[fineAng3];

            // Compute the line points.
            // PsyDoom: scale is now a fixed point number due to framerate uncapped automap movement. Also supporting interpolation here.
            #if PSYDOOM_MODS
                const fixed_t vx = pMobj->x.renderValue() - ox;
                const fixed_t vy = pMobj->y.renderValue() - oy;

                const int32_t x1 = d_fixed_to_int(FixedMul((vx + cos1 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
                const int32_t y1 = d_fixed_to_int(FixedMul((vy + sin1 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
                const int32_t x2 = d_fixed_to_int(FixedMul((vx + cos2 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
                const int32_t y2 = d_fixed_to_int(FixedMul((vy + sin2 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
                const int32_t x3 = d_fixed_to_int(FixedMul((vx + cos3 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
                const int32_t y3 = d_fixed_to_int(FixedMul((vy + sin3 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            #else
                const fixed_t vx = pMobj->x - ox;
                const fixed_t vy = pMobj->y - oy;

                const int32_t x1 = d_fixed_to_int(((vx + cos1 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
                const int32_t y1 = d_fixed_to_int(((vy + sin1 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
                const int32_t x2 = d_fixed_to_int(((vx + cos2 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
                const int32_t y2 = d_fixed_to_int(((vy + sin2 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
                const int32_t x3 = d_fixed_to_int(((vx + cos3 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
                const int32_t y3 = d_fixed_to_int(((vy + sin3 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            #endif

            // Draw the triangle
            DrawLine(AM_COLOR_AQUA, x1, y1, x2, y2);
            DrawLine(AM_COLOR_AQUA, x2, y2, x3, y3);
            DrawLine(AM_COLOR_AQUA, x1, y1, x3, y3);
        }
    }

    // Draw map things for players: again display a little triangle for each player
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        // In deathmatch only show this player's triangle
        if ((gNetGame != gt_coop) && (playerIdx != gCurPlayerIndex))
            continue;

        // Flash the player's triangle when alive
        const player_t& player = gPlayers[playerIdx];

        #if PSYDOOM_MODS
            const bool bIsLocalPlayer = (gCurPlayerIndex == playerIdx);
        #endif

        if ((player.playerstate == PST_LIVE) && (gGameTic & 2))
            continue;

        // Change the colors of this player in COOP to distinguish
        uint32_t color = AM_COLOR_GREEN;

        if ((gNetGame == gt_coop) && (playerIdx == gCurPlayerIndex)) {
            color = AM_COLOR_YELLOW;
        }

        // Compute the the sine and cosines for the angles of the 3 points in the triangle.
        // PsyDoom: use a (potentially) framerate uncapped rotation if it is the local player.
        mobj_t& mobj = *player.mo;

        #if PSYDOOM_MODS
            const angle_t playerAngle = (bIsLocalPlayer) ? gAM_PlayerAngle : mobj.angle;
        #else
            const angle_t playerAngle = mobj.angle;
        #endif

        const uint32_t fineAng1 = (playerAngle                ) >> ANGLETOFINESHIFT;
        const uint32_t fineAng2 = (playerAngle - ANG90 - ANG45) >> ANGLETOFINESHIFT;
        const uint32_t fineAng3 = (playerAngle + ANG90 + ANG45) >> ANGLETOFINESHIFT;

        const fixed_t cos1 = gFineCosine[fineAng1];
        const fixed_t cos2 = gFineCosine[fineAng2];
        const fixed_t cos3 = gFineCosine[fineAng3];

        const fixed_t sin1 = gFineSine[fineAng1];
        const fixed_t sin2 = gFineSine[fineAng2];
        const fixed_t sin3 = gFineSine[fineAng3];

        // Compute the line points.
        // PsyDoom: use a (potentially) framerate uncapped rotation if it is the local player.
        #if PSYDOOM_MODS
            const fixed_t playerX = (bIsLocalPlayer) ? gAM_PlayerX : mobj.x.renderValue();
            const fixed_t playerY = (bIsLocalPlayer) ? gAM_PlayerY : mobj.y.renderValue();
        #else
            const fixed_t playerX = mobj.x;
            const fixed_t playerY = mobj.y;
        #endif

        const fixed_t vx = playerX - ox;
        const fixed_t vy = playerY - oy;

        #if PSYDOOM_MODS
            // PsyDoom: scale is now a fixed point number due to framerate uncapped automap movement
            const int32_t x1 = d_fixed_to_int(FixedMul((vx + cos1 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            const int32_t y1 = d_fixed_to_int(FixedMul((vy + sin1 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            const int32_t x2 = d_fixed_to_int(FixedMul((vx + cos2 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            const int32_t y2 = d_fixed_to_int(FixedMul((vy + sin2 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            const int32_t x3 = d_fixed_to_int(FixedMul((vx + cos3 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
            const int32_t y3 = d_fixed_to_int(FixedMul((vy + sin3 * AM_THING_TRI_SIZE) / SCREEN_W, scale));
        #else
            const int32_t x1 = d_fixed_to_int(((vx + cos1 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            const int32_t y1 = d_fixed_to_int(((vy + sin1 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            const int32_t x2 = d_fixed_to_int(((vx + cos2 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            const int32_t y2 = d_fixed_to_int(((vy + sin2 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            const int32_t x3 = d_fixed_to_int(((vx + cos3 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
            const int32_t y3 = d_fixed_to_int(((vy + sin3 * AM_THING_TRI_SIZE) / SCREEN_W) * scale);
        #endif

        // Draw the triangle
        DrawLine(color, x1, y1, x2, y2);
        DrawLine(color, x2, y2, x3, y3);
        DrawLine(color, x1, y1, x3, y3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw an automap line in the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawLine(const uint32_t color, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2) noexcept {
    // Reject the line quickly using the 'Cohen-Sutherland' algorithm.
    // Note: no clipping is done since that is handled by the hardware.
    enum OutFlags : int32_t {
        INSIDE  = 0,
        LEFT    = 1,
        RIGHT   = 2,
        BOTTOM  = 4,
        TOP     = 8
    };

    uint32_t outcode1 = (x1 < -128) ? LEFT : INSIDE;

    if (x1 >  128) { outcode1 |= RIGHT;     }
    if (y1 < -100) { outcode1 |= BOTTOM;    }
    if (y1 >  100) { outcode1 |= TOP;       }

    uint32_t outcode2 = (x2 < -128) ? LEFT : INSIDE;

    if (x2 >  128) { outcode2 |= RIGHT;     }
    if (y2 < -100) { outcode2 |= BOTTOM;    }
    if (y2 >  100) { outcode2 |= TOP;       }

    if (outcode1 & outcode2)
        return;

    // Setup the map line primitive and draw it.
    //
    // Use the 1 KiB scratchpad also as temp storage space for the primitive.
    // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
    #if PSYDOOM_MODS
        LINE_F2 line = {};
    #else
        LINE_F2& line = *(LINE_F2*) LIBETC_getScratchAddr(128);
    #endif

    LIBGPU_SetLineF2(line);
    LIBGPU_setRGB0(line, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t) color);
    LIBGPU_setXY2(line, (int16_t)(x1 + 128), (int16_t)(100 - y1), (int16_t)(x2 + 128), (int16_t)(100 - y2));

    I_AddPrim(line);
}

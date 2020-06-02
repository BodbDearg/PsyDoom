#include "am_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_local.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

static constexpr fixed_t MOVESTEP       = FRACUNIT * 128;   // Controls how fast manual automap movement happens
static constexpr fixed_t SCALESTEP      = 2;                // How fast to scale in/out
static constexpr int32_t MAXSCALE       = 64;               // Maximum map zoom
static constexpr int32_t MINSCALE       = 8;                // Minimum map zoom
static constexpr int32_t THING_TRI_SIZE = 24;               // The size of the triangles for thing displays on the automap

// Map line colors
static constexpr uint32_t COLOR_RED     = 0xA40000;
static constexpr uint32_t COLOR_GREEN   = 0x00C000;
static constexpr uint32_t COLOR_BROWN   = 0x8A5C30;
static constexpr uint32_t COLOR_YELLOW  = 0xCCCC00;
static constexpr uint32_t COLOR_GREY    = 0x808080;
static constexpr uint32_t COLOR_AQUA    = 0x0080FF;

static fixed_t  gAutomapXMin;
static fixed_t  gAutomapXMax;
static fixed_t  gAutomapYMin;
static fixed_t  gAutomapYMax;

//------------------------------------------------------------------------------------------------------------------------------------------
// Automap initialization logic
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Start() noexcept {
    gAutomapXMin = *gBlockmapOriginX;
    gAutomapYMin = *gBlockmapOriginY;
    gAutomapXMax = (*gBlockmapWidth  << MAPBLOCKSHIFT) + *gBlockmapOriginX;
    gAutomapYMax = (*gBlockmapHeight << MAPBLOCKSHIFT) + *gBlockmapOriginY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the automap: handles player input & controls
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Control(player_t& player) noexcept {
    // If the game is paused we do nothing
    if (*gbGamePaused)
        return;
    
    // Toggle the automap on and off if select has just been pressed
    const padbuttons_t ticButtons = gTicButtons[*gPlayerNum];
    
    if ((ticButtons & PAD_SELECT) && ((gOldTicButtons[*gPlayerNum] & PAD_SELECT) == 0)) {
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
    if ((ticButtons & PAD_CROSS) == 0) {
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
    const fixed_t moveStep = (ticButtons & PAD_SQUARE) ? MOVESTEP * 2 : MOVESTEP;

    // Not sure why this check was done, it can never be true due to the logic above.
    // PC-PSX: remove this block as it is useless...
    #if !PC_PSX_DOOM_MODS
        if ((player.automapflags & AF_FOLLOW) == 0)
            return;
    #endif

    // Left/right movement
    if (ticButtons & PAD_RIGHT) {
        player.automapx += moveStep;

        if (player.automapx > gAutomapXMax) {
            player.automapx = gAutomapXMax;
        }
    }
    else if (ticButtons & PAD_LEFT) {
        player.automapx -= moveStep;

        if (player.automapx < gAutomapXMin) {
            player.automapx = gAutomapXMin;
        }
    }

    // Up/down movement
    if (ticButtons & PAD_UP) {
        player.automapy += moveStep;

        if (player.automapy > gAutomapYMax) {
            player.automapy = gAutomapYMax;
        }
    }
    else if (ticButtons & PAD_DOWN) {
        player.automapy -= moveStep;

        if (player.automapy < gAutomapYMin) {
            player.automapy = gAutomapYMin;
        }
    }
    
    // Scale up and down
    if (ticButtons & PAD_R1) {
        player.automapscale -= SCALESTEP;
        
        if (player.automapscale < MINSCALE) {
            player.automapscale = MINSCALE;
        }
    }
    else if (ticButtons & PAD_L1) {
        player.automapscale += SCALESTEP;

        if (player.automapscale > MAXSCALE) {
            player.automapscale = MAXSCALE;
        }
    }

    // When not in follow mode, consume these inputs so that we don't move the player in the level
    gTicButtons[*gPlayerNum] &= ~(PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_R1 | PAD_L1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does drawing for the automap
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Drawer() noexcept {
    // Finish up the previous frame
    I_DrawPresent();

    // Determine the scale to render the map at
    const player_t& curPlayer = gPlayers[*gCurPlayerIndex];
    const int32_t scale = curPlayer.automapscale;

    // Determine the map camera origin depending on follow mode status
    fixed_t ox, oy;

    if (curPlayer.automapflags & AF_FOLLOW) {
        ox = curPlayer.automapx;
        oy = curPlayer.automapy;
    } else {
        ox = curPlayer.mo->x;
        oy = curPlayer.mo->y;
    }

    // Draw all the map lines
    {
        const line_t* pLine = gpLines->get();

        for (int32_t lineIdx = 0; lineIdx < *gNumLines; ++lineIdx, ++pLine) {
            // See whether we should draw the automap line or not
            const bool bHiddenLine = (pLine->flags & ML_DONTDRAW);
            const bool bLineSeen = ((pLine->flags & ML_MAPPED) && (!bHiddenLine));

            #if PC_PSX_DOOM_MODS
                // PC-PSX: bug fix: if the line is marked as invisible then the allmap powerup shouldn't reveal it.
                // This change makes the behavior consistent with Linux Doom.
                const bool bLineMapped = ((curPlayer.powers[pw_allmap]) && (!bHiddenLine));
            #else
                const bool bLineMapped = (curPlayer.powers[pw_allmap]);
            #endif

            const bool bAllLinesCheatOn = (curPlayer.cheats & CF_ALLLINES);
            const bool bDraw = (bLineSeen || bLineMapped || bAllLinesCheatOn);

            if (!bDraw)
                continue;

            // Compute the line points in viewspace
            const int32_t x1 = (((pLine->vertex1->x - ox) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t y1 = (((pLine->vertex1->y - oy) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t x2 = (((pLine->vertex2->x - ox) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t y2 = (((pLine->vertex2->y - oy) / SCREEN_W) * scale) >> FRACBITS;

            // Decide on line color
            uint32_t color = COLOR_BROWN;   // Normal two sided line
            
            if (((curPlayer.cheats & CF_ALLLINES) + curPlayer.powers[pw_allmap] != 0) &&
                ((pLine->flags & ML_MAPPED) == 0)
            ) {
                color = COLOR_GREY;     // A known line (due to all map cheat/powerup) but unseen
            }
            else if (pLine->flags & ML_SECRET) {
                color = COLOR_RED;      // Secret
            }
            else if (pLine->special != 0) {
                color = COLOR_YELLOW;   // Special or activatable thing
            }
            else if ((pLine->flags & ML_TWOSIDED) == 0) {
                color = COLOR_RED;      // One sided line
            }
            
            DrawLine(color, x1, y1, x2, y2);
        }
    }
    
    // Show all map things cheat: display a little wireframe triangle for for all things
    if (curPlayer.cheats & CF_ALLMOBJ) {
        for (mobj_t* pMObj = gMObjHead->next.get(); pMObj != gMObjHead.get(); pMObj = pMObj->next.get()) {
            // Ignore the player for this particular draw
            if (pMObj == curPlayer.mo.get())
                continue;

            // Compute the the sine and cosines for the angles of the 3 points in the triangle
            const uint32_t fineAng1 = (pMObj->angle                ) >> ANGLETOFINESHIFT;
            const uint32_t fineAng2 = (pMObj->angle - ANG90 - ANG45) >> ANGLETOFINESHIFT;
            const uint32_t fineAng3 = (pMObj->angle + ANG90 + ANG45) >> ANGLETOFINESHIFT;

            const fixed_t cos1 = gFineCosine[fineAng1];
            const fixed_t cos2 = gFineCosine[fineAng2];
            const fixed_t cos3 = gFineCosine[fineAng3];

            const fixed_t sin1 = gFineSine[fineAng1];
            const fixed_t sin2 = gFineSine[fineAng2];
            const fixed_t sin3 = gFineSine[fineAng3];

            // Compute the line points
            const fixed_t vx = pMObj->x - ox;
            const fixed_t vy = pMObj->y - oy;

            const int32_t x1 = (((vx + cos1 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t y1 = (((vy + sin1 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t x2 = (((vx + cos2 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t y2 = (((vy + sin2 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t x3 = (((vx + cos3 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
            const int32_t y3 = (((vy + sin3 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;

            // Draw the triangle
            DrawLine(COLOR_AQUA, x1, y1, x2, y2);
            DrawLine(COLOR_AQUA, x2, y2, x3, y3);
            DrawLine(COLOR_AQUA, x1, y1, x3, y3);
        }
    }

    // Draw map things for players: again display a little triangle for each player
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        // In deathmatch only show this player's triangle
        if ((*gNetGame != gt_coop) && (playerIdx != *gCurPlayerIndex))
            continue;

        // Flash the player's triangle when alive
        const player_t& player = gPlayers[playerIdx];
        
        if ((player.playerstate == PST_LIVE) && (*gGameTic & 2))
            continue;
        
        // Change the colors of this player in COOP to distinguish
        uint32_t color = COLOR_GREEN;

        if ((*gNetGame == gt_coop) && (playerIdx == *gCurPlayerIndex)) {
            color = COLOR_YELLOW;
        }

        // Compute the the sine and cosines for the angles of the 3 points in the triangle
        const mobj_t& mobj = *player.mo;

        const uint32_t fineAng1 = (mobj.angle                ) >> ANGLETOFINESHIFT;
        const uint32_t fineAng2 = (mobj.angle - ANG90 - ANG45) >> ANGLETOFINESHIFT;
        const uint32_t fineAng3 = (mobj.angle + ANG90 + ANG45) >> ANGLETOFINESHIFT;

        const fixed_t cos1 = gFineCosine[fineAng1];
        const fixed_t cos2 = gFineCosine[fineAng2];
        const fixed_t cos3 = gFineCosine[fineAng3];

        const fixed_t sin1 = gFineSine[fineAng1];
        const fixed_t sin2 = gFineSine[fineAng2];
        const fixed_t sin3 = gFineSine[fineAng3];

        // Compute the line points
        const fixed_t vx = player.mo->x - ox;
        const fixed_t vy = player.mo->y - oy;

        const int32_t x1 = (((vx + cos1 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
        const int32_t y1 = (((vy + sin1 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
        const int32_t x2 = (((vx + cos2 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
        const int32_t y2 = (((vy + sin2 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
        const int32_t x3 = (((vx + cos3 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;
        const int32_t y3 = (((vy + sin3 * THING_TRI_SIZE) / SCREEN_W) * scale) >> FRACBITS;

        // Draw the triangle
        DrawLine(color, x1, y1, x2, y2);
        DrawLine(color, x2, y2, x3, y3);
        DrawLine(color, x1, y1, x3, y3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw an automap line in the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
void DrawLine(const uint32_t color, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2) noexcept {
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
    // Use the 1 KiB scratchpad also as temp storage space for the primitive.
    LINE_F2& line = *(LINE_F2*) LIBETC_getScratchAddr(128);
    
    LIBGPU_SetLineF2(line);
    LIBGPU_setRGB0(line, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t) color);
    LIBGPU_setXY2(line, (int16_t)(x1 + 128), (int16_t)(100 - y1), (int16_t)(x2 + 128), (int16_t)(100 - y2));
    
    I_AddPrim(&line);
}

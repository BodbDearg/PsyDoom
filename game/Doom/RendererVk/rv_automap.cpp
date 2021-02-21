//------------------------------------------------------------------------------------------------------------------------------------------
// This module is responsible for drawing the automap for the new Vulkan renderer.
// A new map drawing implementation is needed to fix precision issues with the old map drawing routines, and also to support widescreen.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "rv_automap.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Doom/Base/i_main.h"
#include "Doom/doomdef.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/UI/am_main.h"
#include "Matrix4.h"
#include "PcPsx/Config.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VRenderer.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_utils.h"

#include <cmath>

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the transform matrix for rendering the automap
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SetupAutomapTransformMatrix() noexcept {
    // Get the view translation transform
    const player_t& player = gPlayers[gCurPlayerIndex];
    float amViewX;
    float amViewY;

    if (player.automapflags & AF_FOLLOW) {
        amViewX = RV_FixedToFloat(player.automapx);
        amViewY = RV_FixedToFloat(player.automapy);
    } else {
        amViewX = RV_FixedToFloat(player.mo->x);
        amViewY = RV_FixedToFloat(player.mo->y);
    }

    const Matrix4f viewTranslate = Matrix4f::translate(-amViewX, -amViewY, 0.0f);

    // Get the view scaling transform
    const float amScale = (float) player.automapscale / (float) SCREEN_W;
    const Matrix4f viewScale = Matrix4f::scale(amScale, amScale, 1.0f);

    // This matrix inverts the y coordinate of all vertices
    const Matrix4f invertY = Matrix4f::scale(1.0f, -1.0f, 1.0f);

    // This matrix offsets all lines so that 0,0 is in the middle of the screen
    const Matrix4f centeringOffset = Matrix4f::translate(128.0f, 100.0f, 0.0f);

    // Get the UI projection & transform matrix and combine all the transforms
    const Matrix4f uiTransformMatrix = VDrawing::computeTransformMatrixForUI(Config::gbVulkanWidescreenEnabled);

    const Matrix4f combinedTransformMatrix = (
        viewTranslate *
        viewScale *
        invertY *
        centeringOffset *
        uiTransformMatrix
    );

    // Set the uniforms to contain this transform matrix
    VShaderUniforms_Draw uniforms = {};
    uniforms.mvpMatrix = combinedTransformMatrix;
    uniforms.ndcToPsxScaleX = VRenderer::gNdcToPsxScaleX;
    uniforms.ndcToPsxScaleY = VRenderer::gNdcToPsxScaleY;
    uniforms.psxNdcOffsetX = VRenderer::gPsxNdcOffsetX;
    uniforms.psxNdcOffsetY = VRenderer::gPsxNdcOffsetY;

    VDrawing::setDrawUniforms(uniforms);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add an automap line to be drawn
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_AddAutomapLine(const uint32_t color, const float x1, const float y1, const float x2, const float y2) noexcept {
    VDrawing::addUILine(x1, y1, x2, y2, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)(color));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws all visible map lines
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawMapLines() noexcept {
    const player_t& curPlayer = gPlayers[gCurPlayerIndex];
    const int32_t numLines = gNumLines;
    const line_t* const pLines = gpLines;

    for (int32_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
        // See whether we should draw the automap line or not
        const line_t& line = pLines[lineIdx];

        const bool bHiddenLine = (line.flags & ML_DONTDRAW);
        const bool bLineSeen = ((line.flags & ML_MAPPED) && (!bHiddenLine));
        const bool bLineMapped = ((curPlayer.powers[pw_allmap]) && (!bHiddenLine));
        const bool bAllLinesCheatOn = (curPlayer.cheats & CF_ALLLINES);
        const bool bDraw = (bLineSeen || bLineMapped || bAllLinesCheatOn);

        if (!bDraw)
            continue;

        // Get the line point coords in float format
        const vertex_t& v1 = *line.vertex1;
        const vertex_t& v2 = *line.vertex2;
        const float x1 = RV_FixedToFloat(v1.x);
        const float y1 = RV_FixedToFloat(v1.y);
        const float x2 = RV_FixedToFloat(v2.x);
        const float y2 = RV_FixedToFloat(v2.y);

        // Decide on line color: start off with the normal two sided line color to begin with
        uint32_t color = AM_COLOR_BROWN;

        if (((curPlayer.cheats & CF_ALLLINES) + curPlayer.powers[pw_allmap] != 0) && ((line.flags & ML_MAPPED) == 0)) {
            // A known line (due to all map cheat/powerup) but unseen
            color = AM_COLOR_GREY;
        }
        else if (line.flags & ML_SECRET) {
            // Secret
            color = AM_COLOR_RED;
        }
        else if (line.special != 0) {
            // Special or activatable thing
            color = AM_COLOR_YELLOW;
        }
        else if ((line.flags & ML_TWOSIDED) == 0) {
            // One sided line
            color = AM_COLOR_RED;
        }

        RV_AddAutomapLine(color, x1, y1, x2, y2);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws things for the show all map things cheat: displays a little wireframe triangle for for all things
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawAutomapShowAllThingsCheat() noexcept {
    const player_t& curPlayer = gPlayers[gCurPlayerIndex];
    const mobj_t& playerMObj = *curPlayer.mo;

    for (mobj_t* pMObj = gMObjHead.next; pMObj != &gMObjHead; pMObj = pMObj->next) {
        // Ignore the player for this particular draw
        if (pMObj == &playerMObj)
            continue;

        // Compute the the sine and cosines for the angles of the 3 points in the triangle
        mobj_t& mobj = *pMObj;

        const float ang1 = RV_AngleToFloat(mobj.angle);
        const float ang2 = RV_AngleToFloat(mobj.angle - ANG90 - ANG45);
        const float ang3 = RV_AngleToFloat(mobj.angle + ANG90 + ANG45);

        const float cos1 = std::cos(ang1);
        const float cos2 = std::cos(ang2);
        const float cos3 = std::cos(ang3);
        const float sin1 = std::sin(ang1);
        const float sin2 = std::sin(ang2);
        const float sin3 = std::sin(ang3);

        // Compute the line points for the triangle
        const float tx = RV_FixedToFloat(mobj.x);
        const float ty = RV_FixedToFloat(mobj.y);

        const float x1 = tx + cos1 * (float) AM_THING_TRI_SIZE;
        const float y1 = ty + sin1 * (float) AM_THING_TRI_SIZE;
        const float x2 = tx + cos2 * (float) AM_THING_TRI_SIZE;
        const float y2 = ty + sin2 * (float) AM_THING_TRI_SIZE;
        const float x3 = tx + cos3 * (float) AM_THING_TRI_SIZE;
        const float y3 = ty + sin3 * (float) AM_THING_TRI_SIZE;

        // Draw the triangle
        RV_AddAutomapLine(AM_COLOR_AQUA, x1, y1, x2, y2);
        RV_AddAutomapLine(AM_COLOR_AQUA, x2, y2, x3, y3);
        RV_AddAutomapLine(AM_COLOR_AQUA, x1, y1, x3, y3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws map things for players; displays a little triangle for each player
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawAutomapPlayers() noexcept {
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        // In deathmatch only show this player's triangle
        if ((gNetGame != gt_coop) && (playerIdx != gCurPlayerIndex))
            continue;

        // Flash the player's triangle when alive
        const player_t& player = gPlayers[playerIdx];
        
        if ((player.playerstate == PST_LIVE) && (gGameTic & 2))
            continue;
        
        // Change the colors of this player in COOP to distinguish
        uint32_t color = AM_COLOR_GREEN;

        if ((gNetGame == gt_coop) && (playerIdx == gCurPlayerIndex)) {
            color = AM_COLOR_YELLOW;
        }

        // Compute the the sine and cosines for the angles of the 3 points in the triangle
        const mobj_t& mobj = *player.mo;

        const float ang1 = RV_AngleToFloat(mobj.angle);
        const float ang2 = RV_AngleToFloat(mobj.angle - ANG90 - ANG45);
        const float ang3 = RV_AngleToFloat(mobj.angle + ANG90 + ANG45);
        
        const float cos1 = std::cos(ang1);
        const float cos2 = std::cos(ang2);
        const float cos3 = std::cos(ang3);
        const float sin1 = std::sin(ang1);
        const float sin2 = std::sin(ang2);
        const float sin3 = std::sin(ang3);

        // Compute the line points for the triangle
        const float tx = RV_FixedToFloat(mobj.x);
        const float ty = RV_FixedToFloat(mobj.y);

        const float x1 = tx + cos1 * (float) AM_THING_TRI_SIZE;
        const float y1 = ty + sin1 * (float) AM_THING_TRI_SIZE;
        const float x2 = tx + cos2 * (float) AM_THING_TRI_SIZE;
        const float y2 = ty + sin2 * (float) AM_THING_TRI_SIZE;
        const float x3 = tx + cos3 * (float) AM_THING_TRI_SIZE;
        const float y3 = ty + sin3 * (float) AM_THING_TRI_SIZE;

        // Draw the triangle
        RV_AddAutomapLine(color, x1, y1, x2, y2);
        RV_AddAutomapLine(color, x2, y2, x3, y3);
        RV_AddAutomapLine(color, x1, y1, x3, y3);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the automap
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawAutomap() noexcept {
    // Setup the draw transform matrix and switch to drawing lines
    RV_SetupAutomapTransformMatrix();
    VDrawing::setDrawPipeline(VPipelineType::Lines);

    // Draw all automap elements
    RV_DrawMapLines();

    if (gPlayers[gCurPlayerIndex].cheats & CF_ALLMOBJ) {
        RV_DrawAutomapShowAllThingsCheat();
    }

    RV_DrawAutomapPlayers();

    // Switch to the matrix for drawing UI and draw the letterbox for widescreen in the vertical area occupied by the status bar
    Utils::onBeginUIDrawing();
    RV_DrawWidescreenStatusBarLetterbox();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER

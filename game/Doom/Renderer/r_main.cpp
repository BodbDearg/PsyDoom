#include "r_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_user.h"
#include "PsyDoom/Config.h"
#include "PsyDoom/Game.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_bsp.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_things.h"

#include <algorithm>
#include <chrono>
#include <cmath>

// Incremented whenever checks are made
int32_t gValidCount = 1;

// View properties
player_t*   gpViewPlayer;
fixed_t     gViewX;
fixed_t     gViewY;
fixed_t     gViewZ;
angle_t     gViewAngle;
fixed_t     gViewCos;
fixed_t     gViewSin;
bool        gbIsSkyVisible;
MATRIX      gDrawMatrix;

// Light properties
bool gbDoViewLighting;

#if !PSYDOOM_MODS
    // PsyDoom: these are not used anymore with dual colored lighting.
    // They were only usable when sectors were guaranteed to have a single color.
    const light_t*  gpCurLight;
    uint32_t        gCurLightValR;
    uint32_t        gCurLightValG;
    uint32_t        gCurLightValB;
#endif

// PsyDoom: the number of draw subsectors is now unlimited
#if PSYDOOM_LIMIT_REMOVING
    std::vector<subsector_t*> gpDrawSubsectors;
#else
    // The list of subsectors to draw and current position in the list.
    // The draw subsector count does not appear to be used for anything however... Maybe used in debug builds for stat tracking?
    subsector_t*    gpDrawSubsectors[MAX_DRAW_SUBSECTORS];
    int32_t         gNumDrawSubsectors;
#endif

sector_t*       gpCurDrawSector;        // What sector is currently being drawn
subsector_t**   gppEndDrawSubsector;    // Used to point to the last draw subsector in the list and iterate backwards

// PsyDoom: used for interpolation for uncapped framerates 
#if PSYDOOM_MODS
    typedef std::chrono::high_resolution_clock::time_point timepoint_t;
    static timepoint_t gPrevFrameTime;

    fixed_t     gOldViewX;
    fixed_t     gOldViewY;
    fixed_t     gOldViewZ;
    angle_t     gOldViewAngle;
    fixed_t     gOldAutomapX;
    fixed_t     gOldAutomapY;
    fixed_t     gOldAutomapScale;
    bool        gbSnapViewZInterpolation;
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// One time setup for the 3D view renderer
//------------------------------------------------------------------------------------------------------------------------------------------
void R_Init() noexcept {
    // Initialize texture lists, palettes etc.
    R_InitData();

    // PsyDoom: preallocating memory for certain dynamic buffers ahead of time
    #if PSYDOOM_LIMIT_REMOVING
        R_InitDrawBuffers();
    #endif

    // Initialize the transform matrix used for drawing and upload it to the GTE
    gDrawMatrix.t[0] = 0;
    gDrawMatrix.t[1] = 0;
    gDrawMatrix.t[2] = 0;
    LIBGTE_SetTransMatrix(gDrawMatrix);

    gDrawMatrix.m[0][0] = 0;
    gDrawMatrix.m[0][1] = 0;
    gDrawMatrix.m[0][2] = 0;
    gDrawMatrix.m[1][0] = 0;
    gDrawMatrix.m[1][1] = GTE_ROTFRAC_UNIT;     // This part of the matrix never changes, so assign here
    gDrawMatrix.m[1][2] = 0;
    gDrawMatrix.m[2][0] = 0;
    gDrawMatrix.m[2][1] = 0;
    gDrawMatrix.m[2][2] = 0;
    LIBGTE_SetRotMatrix(gDrawMatrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Render the 3D view and also player weapons
//------------------------------------------------------------------------------------------------------------------------------------------
void R_RenderPlayerView() noexcept {
    // If currently in fullbright mode (no lighting) then setup the light params now.
    // PsyDoom: we don't compute these globals anymore now that dual colored lighting can be used.
    #if !PSYDOOM_MODS
        if (!gbDoViewLighting) {
            gCurLightValR = 128;
            gCurLightValG = 128;
            gCurLightValB = 128;
            gpCurLight = &gpLightsLump[0];
        }
    #endif

    // Store view parameters before drawing
    player_t& player = gPlayers[gCurPlayerIndex];
    gpViewPlayer = &player;

    // PsyDoom: use interpolation to update the actual view if doing an uncapped framerate
    #if PSYDOOM_MODS
        const bool bInterpolateFrame = Config::gbUncapFramerate;
    #else
        const bool bInterpolateFrame = false;
    #endif

    mobj_t& playerMobj = *player.mo;

    if (bInterpolateFrame) {
        // PsyDoom: this is new logic. Note that I am still truncating coords to integers (in spite of interpolation) because renderer
        // artifacts will occur if we don't. The original renderer appears to be built with the assumption that coords are truncated.
        const fixed_t newViewX = playerMobj.x;
        const fixed_t newViewY = playerMobj.y;
        const fixed_t newViewZ = player.viewz;
        const angle_t newViewAngle = playerMobj.angle;
        const fixed_t lerp = R_CalcLerpFactor();

        if (gbSnapViewZInterpolation) {
            gOldViewZ = newViewZ;
            gbSnapViewZInterpolation = false;
        }

        gViewX = R_LerpCoord(gOldViewX, newViewX, lerp) & (~FRACMASK);
        gViewY = R_LerpCoord(gOldViewY, newViewY, lerp) & (~FRACMASK);
        gViewZ = R_LerpCoord(gOldViewZ, newViewZ, lerp) & (~FRACMASK);

        // View angle is not interpolated (except in demos) since turning movements are now completely framerate uncapped
        if (gbDemoPlayback) {
            gViewAngle = R_LerpAngle(gOldViewAngle, newViewAngle, lerp);
        } else {
            // Normal gameplay: take into consideration how much turning movement we haven't committed to the player object yet here.
            // For net games, we must use the view angle we said we would use NEXT as that is the most up-to-date angle.
            if (gNetGame == gt_single) {
                gViewAngle = playerMobj.angle + gPlayerUncommittedTurning;
            } else {
                gViewAngle = gPlayerNextTickViewAngle + gPlayerUncommittedTurning;
            }
        }
    }
    else {
        // Originally this is all that happened
        gViewX = playerMobj.x & (~FRACMASK);
        gViewY = playerMobj.y & (~FRACMASK);
        gViewZ = player.viewz & (~FRACMASK);
        gViewAngle = playerMobj.angle;
    }

    // PsyDoom: if the external camera is active the override the player's camera
    #if PSYDOOM_MODS
        if (gExtCameraTicsLeft > 0) {
            gViewX = gExtCameraX & (~FRACMASK);
            gViewY = gExtCameraY & (~FRACMASK);
            gViewZ = gExtCameraZ & (~FRACMASK);
            gViewAngle = gExtCameraAngle;
        }
    #endif

    gViewCos = gFineCosine[gViewAngle >> ANGLETOFINESHIFT];
    gViewSin = gFineSine[gViewAngle >> ANGLETOFINESHIFT];

    // Set the draw matrix and upload to the GTE
    gDrawMatrix.m[0][0] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewSin);
    gDrawMatrix.m[0][2] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>(-gViewCos);
    gDrawMatrix.m[2][0] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewCos);
    gDrawMatrix.m[2][2] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewSin);
    LIBGTE_SetRotMatrix(gDrawMatrix);

    // PsyDoom: increment the marker used to determine when to update the 'draw height' for each sector
    #if PSYDOOM_MODS
        gValidCount++;
    #endif

    // Traverse the BSP tree to determine what needs to be drawn and in what order
    R_BSP();

    // Stat tracking: how many subsectors will we draw?
    // PsyDoom: if doing limit removing then we already have this count in the std::vector.
    #if !PSYDOOM_LIMIT_REMOVING
        gNumDrawSubsectors = (int32_t)(gppEndDrawSubsector - gpDrawSubsectors);
    #endif

    // Finish up the previous draw before we continue and draw the sky if currently visible.
    // PsyDoom: moved this to the end of 'P_Drawer' instead - needed for the new Vulkan renderer integration.
    #if !PSYDOOM_MODS
        I_DrawPresent();
    #endif

    if (gbIsSkyVisible) {
        R_DrawSky();
    }

    // PsyDoom: increment the marker used to determine when to update the shading params for each sector
    #if PSYDOOM_MODS
        gValidCount++;
    #endif

    // Draw all subsectors emitted during BSP traversal.
    // Draw them in back to front order.
    #if PSYDOOM_LIMIT_REMOVING
        gppEndDrawSubsector = gpDrawSubsectors.data() + gpDrawSubsectors.size();
    #endif

    while (true) {
        #if PSYDOOM_LIMIT_REMOVING
            if (gppEndDrawSubsector <= gpDrawSubsectors.data())
                break;
        #else
            if (gppEndDrawSubsector <= gpDrawSubsectors)
                break;
        #endif

        --gppEndDrawSubsector;

        // Set the current draw sector
        subsector_t& subsec = **gppEndDrawSubsector;
        sector_t& sec = *subsec.sector;
        gpCurDrawSector = &sec;

        // PsyDoom: make sure the shading params for the sector are up to date
        #if PSYDOOM_MODS
            R_UpdateShadingParams(sec);
        #endif

        // Setup the lighting values to use for the sector.
        // PsyDoom: we don't compute these globals anymore now that dual colored lighting can be used.
        #if !PSYDOOM_MODS
            if (gbDoViewLighting) {
                // Compute basic light values
                const light_t& light = gpLightsLump[sec.colorid];

                gpCurLight = &light;
                gCurLightValR = ((uint32_t) sec.lightlevel * (uint32_t) light.r) >> 8;
                gCurLightValG = ((uint32_t) sec.lightlevel * (uint32_t) light.g) >> 8;
                gCurLightValB = ((uint32_t) sec.lightlevel * (uint32_t) light.b) >> 8;

                // Contribute the player muzzle flash to the light and saturate
                if (player.extralight != 0) {
                    gCurLightValR += player.extralight;
                    gCurLightValG += player.extralight;
                    gCurLightValB += player.extralight;

                    if (gCurLightValR > 255) { gCurLightValR = 255; }
                    if (gCurLightValG > 255) { gCurLightValG = 255; }
                    if (gCurLightValB > 255) { gCurLightValB = 255; }
                }
            }
        #endif

        R_DrawSubsector(subsec);
    }

    // Draw any player sprites/weapons.
    // PsyDoom: skip if we're using the external camera.
    #if PSYDOOM_MODS
        if (gExtCameraTicsLeft <= 0) {
            R_DrawWeapon();
        }
    #else
        R_DrawWeapon();
    #endif

    // Clearing the texture window: this is probably not required?
    // Not sure what the reason is for this, but everything seems to work fine without doing this.
    // I'll leave the code as-is for now though until I have a better understanding of this, just in case something breaks.
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the index from 'gTanToAngle' to use for the given input angle.
// The angle is specified by the slope, which is decomposed into a separate numerator and demoninator.
// This function is only meaningful for angles within the first octant of the unit circle (<= 45 degrees).
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_SlopeDiv(const uint32_t num, const uint32_t den) noexcept {
    // If we're going to be dividing by 1 (or zero) then just return the max angle index (45 degrees)
    if (den < 512)
        return SLOPERANGE;

    // Otherwise figure out the lookup index from the table and clamp to the max range
    const uint32_t ans = (num << 3) / (den >> 8);
    return (ans <= SLOPERANGE) ? ans : SLOPERANGE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Calculates the angle going from one 2d point to another
//------------------------------------------------------------------------------------------------------------------------------------------
angle_t R_PointToAngle2(const fixed_t x1, const fixed_t y1, const fixed_t x2, const fixed_t y2) noexcept {
    fixed_t dx = x2 - x1;
    fixed_t dy = y2 - y1;

    if (dx == 0 && dy == 0)
        return 0;

    if (dx >= 0) {
        if (dy >= 0) {
            if (dx > dy) {
                return gTanToAngle[R_SlopeDiv(dy, dx)];                 // Octant 0
            } else {
                return ANG90 - 1 - gTanToAngle[R_SlopeDiv(dx, dy)];     // Octant 1
            }
        } else {
            dy = -dy;

            if (dx > dy) {
                return -gTanToAngle[R_SlopeDiv(dy, dx)];                // Octant 8
            } else {
                return ANG270 + gTanToAngle[R_SlopeDiv(dx, dy)];        // Octant 7
            }
        }
    } else {
        dx = -dx;

        if (dy >= 0) {
            if (dx > dy) {
                return ANG180 - 1 - gTanToAngle[R_SlopeDiv(dy, dx)];    // Octant 3
            } else {
                return ANG90 + gTanToAngle[R_SlopeDiv(dx, dy)];         // Octant 2
            }
        } else {
            dy = -dy;

            if (dx > dy) {
                return gTanToAngle[R_SlopeDiv(dy, dx)] - ANG180;        // Octant 4
            } else {
                return ANG270 - 1 - gTanToAngle[R_SlopeDiv(dx, dy)];    // Octant 5
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells what side of a line a point is on, with accuracy in terms of integer units.
// Returns '0' if the point is on the 'front' side of the line, otherwise '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept {
    // Special case shortcut for vertical lines
    if (node.line.dx == 0) {
        if (x <= node.line.x) {
            return (node.line.dy > 0);
        } else {
            return (node.line.dy < 0);
        }
    }

    // Special case shortcut for horizontal lines
    if (node.line.dy == 0) {
        if (y <= node.line.y) {
            return (node.line.dx < 0);
        } else {
            return (node.line.dx > 0);
        }
    }

    // Compute which side of the line the point is on using the cross product
    const fixed_t dx = x - node.line.x;
    const fixed_t dy = y - node.line.y;
    const int32_t lprod = d_fixed_to_int(dx) * d_fixed_to_int(node.line.dy);
    const int32_t rprod = d_fixed_to_int(dy) * d_fixed_to_int(node.line.dx);
    return (rprod >= lprod);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the subsector that a 2D point is in.
// Note: there should always be a subsector returned for a valid DOOM level.
//------------------------------------------------------------------------------------------------------------------------------------------
subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept {
    // Not sure why there would ever be '0' BSP nodes - that does not seem like a valid DOOM level to me?
    // The same logic can also be found in other versions of DOOM...
    if (gNumBspNodes == 0) {
        return gpSubsectors;
    }

    // Traverse the BSP tree starting at the root node, using the given position to decide which half-spaces to visit.
    // Once we reach a subsector stop and return it.
    int32_t nodeNum = gNumBspNodes - 1;

    while ((nodeNum & NF_SUBSECTOR) == 0) {
        node_t& node = gpBspNodes[nodeNum];
        const int32_t side = R_PointOnSide(x, y, node);
        nodeNum = node.children[side];
    }

    const int32_t actualNodeNum = nodeNum & (~NF_SUBSECTOR);
    return &gpSubsectors[actualNodeNum];
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: update the 'previous' values used in interpolation to their current (actual) values.
// Used before simulating an actual game tick, or when we want to snap immediately to the actual values - like when teleporting.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_NextInterpolation() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    mobj_t& mobj = *player.mo;

    const bool bAutomapFreeCamera = (player.automapflags & AF_FOLLOW);

    gPrevFrameTime = std::chrono::high_resolution_clock::now();
    gOldViewX = mobj.x;
    gOldViewY = mobj.y;
    gOldViewZ = player.viewz;
    gOldViewAngle = mobj.angle;
    gOldAutomapX = (bAutomapFreeCamera) ? player.automapx : mobj.x;     // Use the current player position if not following to avoid sudden jumps when switching to free camera
    gOldAutomapY = (bAutomapFreeCamera) ? player.automapy : mobj.y;
    gOldAutomapScale = player.automapscale;
    gbSnapViewZInterpolation = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Kill the current interpolation of viewz and cause it to go immediately to the actual viewz value
//------------------------------------------------------------------------------------------------------------------------------------------
void R_SnapViewZInterpolation() noexcept {
    gbSnapViewZInterpolation = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: compute the interpolation factor (0-1) by how much we are in between frames.
// When the value is '0' it means use the old frame values, when '1' use the new frame values.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t R_CalcLerpFactor() noexcept {
    // Get the elapsed time since the last frame we saved data for
    const timepoint_t now = std::chrono::high_resolution_clock::now();
    const double elapsedSeconds = std::chrono::duration<double>(now - gPrevFrameTime).count();

    // How many tics per second can the game do maximum?
    // For demo playback/recording the game is capped at 15 Hz for consistency, and the cap is 30 Hz for normal games.
    // These values are adjusted slightly for PAL mode also.
    const double normalTicsPerSec = (Game::gSettings.bUsePalTimings) ? 25.0 : 30.0;
    const double demoTicsPerSec = (Game::gSettings.bUsePalTimings) ? 16.666666 : 15.0;
    const double ticsPerSec = (Game::gSettings.bUseDemoTimings) ? demoTicsPerSec : normalTicsPerSec;

    // Compute the lerp factor in 16.16 format
    const double elapsedGameTics = std::clamp(elapsedSeconds * ticsPerSec, 0.0, 1.0);
    return (fixed_t)(elapsedGameTics * (double) FRACUNIT);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: interpolate between the given old and new value by the given amount
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t R_LerpCoord(const fixed_t oldCoord, const fixed_t newCoord, const fixed_t mix) noexcept {
    return FixedMul(oldCoord, FRACUNIT - mix) + FixedMul(newCoord, mix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: interpolate between the given old and new angle by the given amount
//------------------------------------------------------------------------------------------------------------------------------------------
angle_t R_LerpAngle(const angle_t oldAngle, const angle_t newAngle, const fixed_t mix) noexcept {
    const int32_t diff = (int32_t)(newAngle - oldAngle);
    const int32_t adjust = d_rshift<16>(diff) * mix;
    return oldAngle + (angle_t) adjust;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: check for a sector surrounding the given sector that has a sky ceiling which also happens to be higher.
// 
// This is used by the Vulkan and limit extending Classic renderer to tell when to treat the area past a sky ceiling as a void where nothing
// will render. Voids can be used to do floating platform type effects in certain situations.
//------------------------------------------------------------------------------------------------------------------------------------------
bool R_HasHigherSurroundingSkyCeiling(const sector_t& sector) noexcept {
    const int32_t numLines = sector.linecount;
    const line_t* const* const pLines = sector.lines;

    for (int32_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
        const line_t& line = *pLines[lineIdx];
        const sector_t* pNextSector = (line.frontsector == &sector) ? line.backsector : line.frontsector;

        if (pNextSector && (pNextSector->ceilingpic == -1)) {
            if (pNextSector->ceilingheight > sector.ceilingheight)
                return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: check for a sector surrounding the given sector that has a sky floor which also happens to be lower.
// Note: Uses the 'render' height of the floor, rather than the actual height.
// 
// This is used by the Vulkan and limit extending Classic renderer to tell when to treat the area past the floor of a sky sector
// as a void where nothing will render. Voids can be used to do floating platform type effects in certain situations.
//------------------------------------------------------------------------------------------------------------------------------------------
bool R_HasLowerSurroundingSkyFloor(const sector_t& sector) noexcept {
    const int32_t numLines = sector.linecount;
    const line_t* const* const pLines = sector.lines;

    for (int32_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
        const line_t& line = *pLines[lineIdx];
        const sector_t* pNextSector = (line.frontsector == &sector) ? line.backsector : line.frontsector;

        if (pNextSector && (pNextSector->floorpic == -1)) {
            if (pNextSector->floorDrawHeight < sector.floorDrawHeight)
                return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: updates the height that the sector's floor is to be rendered at, if required.
// It might be rendered at a different height to it's real (physical) height to achieve invisible platform effects.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_UpdateFloorDrawHeight(sector_t& sector) noexcept {
    const bool bIsInvisiblePlatform = (sector.flags & SF_GHOSTPLAT);

    if (!bIsInvisiblePlatform) {
        sector.floorDrawHeight = sector.floorheight;
    } else {
        // Finding the lowest floor surrounding a sector might be slightly expensive if done often, so skip it if we can:
        const int32_t validCount = gValidCount;

        if (sector.validcount != validCount) {
            sector.floorDrawHeight = P_FindLowestFloorSurrounding(sector);
            sector.validcount = validCount;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: updates sector parameters relating to 2-colored lighting.
// If the sector does not use two colored lighting then the parameters are defaulted.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_UpdateShadingParams(sector_t& sector) noexcept {
    // If the lower and upper colors are the same then just skip all this
    if (sector.colorid == sector.ceilColorid)
        return;

    // Only do it if needed: use the valid count field to avoid recomputing if we can
    const int32_t validCount = gValidCount;

    if (validCount == sector.validcount)
        return;

    // Initially the z values at which to apply the floor and ceiling colors are just the floor and ceiling heights respectively
    fixed_t lowerColorZ = sector.floorheight;
    fixed_t upperColorZ = sector.ceilingheight;

    // Adjust the floor/ceiling z values for the purposes of shading (if adjustments are specified)
    const uint32_t sflags = sector.flags;
    const fixed_t sectorH = upperColorZ - lowerColorZ;
    const uint8_t floorAdjust = (sflags & SF_GRAD_FLOOR_PLUS_1 ? 1 : 0) + (sflags & SF_GRAD_FLOOR_PLUS_2 ? 2 : 0);
    const uint8_t ceilAdjust = (sflags & SF_GRAD_CEIL_PLUS_1 ? 1 : 0) + (sflags & SF_GRAD_CEIL_PLUS_2 ? 2 : 0);

    if (sflags & SF_GRAD_CONTRACT) {
        // Contracting the sector shading height
        switch (floorAdjust) {
            case 1: lowerColorZ += sectorH >> 2;        break;  // Floor Z +25% of sector height
            case 2: lowerColorZ += sectorH >> 1;        break;  // Floor Z +50% of sector height
            case 3: lowerColorZ += (sectorH * 3) >> 4;  break;  // Floor Z +75% of sector height
        }

        switch (ceilAdjust) {
            case 1: upperColorZ -= sectorH >> 2;        break;  // Ceiling Z -25% of sector height
            case 2: upperColorZ -= sectorH >> 1;        break;  // Ceiling Z -50% of sector height
            case 3: upperColorZ -= (sectorH * 3) >> 4;  break;  // Ceiling Z -75% of sector height
        }
    }
    else
    {
        // Expanding the sector shading height
        switch (floorAdjust) {
            case 1: lowerColorZ -= sectorH >> 1;    break;  // Floor Z -50%  of sector height
            case 2: lowerColorZ -= sectorH;         break;  // Floor Z -100% of sector height
            case 3: lowerColorZ -= sectorH << 1;    break;  // Floor Z -200% of sector height
        }

        switch (ceilAdjust) {
            case 1: upperColorZ += sectorH >> 1;    break;  // Ceiling Z +50%  of sector height
            case 2: upperColorZ += sectorH;         break;  // Ceiling Z +100% of sector height
            case 3: upperColorZ += sectorH << 1;    break;  // Ceiling Z +200% of sector height
        }
    }

    sector.lowerColorZ = lowerColorZ;
    sector.upperColorZ = upperColorZ;

    // Compute the divisor for sector shading; compute approximately '1.0 / sectorH'
    const int32_t sectorShadeH = (upperColorZ - lowerColorZ) >> FRACBITS;

    if (sectorShadeH > 0) {
        sector.shadeHeightDiv = FRACUNIT / sectorShadeH;
    } else {
        sector.shadeHeightDiv = FRACUNIT;
    }

    // Don't compute this again unless we need to
    sector.validcount = validCount;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: computes the light color to use at the given z height for the specified sector.
// Assumes 'R_UpdateShadingParams' has already been called on the sector for the current frame.
//------------------------------------------------------------------------------------------------------------------------------------------
light_t R_GetSectorLightColor(const sector_t& sector, const fixed_t z) noexcept {
    // If lighting is disabled (light amp visor) then we just return the identity light color (full white, 1st color entry)
    const light_t* const pLights = gpLightsLump;

    if (!gbDoViewLighting)
        return pLights[0];

    // If the floor and ceiling color are the same then just early out and return that - no point in interpolating
    const int16_t lowerColorIdx = sector.colorid;
    const int16_t upperColorIdx = sector.ceilColorid;
    const light_t lowerColor = pLights[lowerColorIdx];

    if (lowerColorIdx == upperColorIdx)
        return lowerColor;

    // Otherwise use linear interpolation to figure out the color
    const light_t upperColor = pLights[upperColorIdx];

    const fixed_t t = std::min((std::max(z - sector.lowerColorZ, 0) >> FRACBITS) * sector.shadeHeightDiv, FRACUNIT);
    const fixed_t tInv = FRACUNIT - t;

    const uint32_t r = (lowerColor.r * tInv + upperColor.r * t) >> FRACBITS;
    const uint32_t g = (lowerColor.g * tInv + upperColor.g * t) >> FRACBITS;
    const uint32_t b = (lowerColor.b * tInv + upperColor.b * t) >> FRACBITS;

    return {
        (uint8_t) std::clamp(r, 0u, 255u),
        (uint8_t) std::clamp(g, 0u, 255u),
        (uint8_t) std::clamp(b, 0u, 255u),
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the RGB color value to apply to shade the sector at the given Z height, accounting for light color and sector brightness etc.
// A value of '128' for a component means full brightness, and values over that are over-bright.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_GetSectorDrawColor(const sector_t& sector, const fixed_t z, uint8_t& r, uint8_t& g, uint8_t& b) noexcept {
    if (gbDoViewLighting) {
        // Compute the basic light color at this z value
        const light_t lightColor = R_GetSectorLightColor(sector, z);

        // Modulate by the light level
        const uint16_t lightLevel = sector.lightlevel;
        uint32_t r32 = (lightLevel * lightColor.r) >> 8;
        uint32_t g32 = (lightLevel * lightColor.g) >> 8;
        uint32_t b32 = (lightLevel * lightColor.b) >> 8;

        // Contribute player muzzle flash to the light
        player_t& player = gPlayers[gCurPlayerIndex];
        const uint32_t extraLight = player.extralight;

        r32 += extraLight;
        g32 += extraLight;
        b32 += extraLight;

        // Return the saturated light value
        r = (uint8_t) std::min(r32, 255u);
        g = (uint8_t) std::min(g32, 255u);
        b = (uint8_t) std::min(b32, 255u);
    } else {
        // No lighting - render full bright!
        r = 128;
        g = 128;
        b = 128;
    }
}
#endif  // PSYDOOM_MODS

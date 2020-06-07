#include "r_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_bsp.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_things.h"

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
bool            gbDoViewLighting;
const light_t*  gpCurLight;
uint32_t        gCurLightValR;
uint32_t        gCurLightValG;
uint32_t        gCurLightValB;

// The list of subsectors to draw and current position in the list.
// The draw subsector count does not appear to be used for anything however... Maybe used in debug builds for stat tracking?
subsector_t*    gpDrawSubsectors[MAX_DRAW_SUBSECTORS];
subsector_t**   gppEndDrawSubsector;
int32_t         gNumDrawSubsectors;

// What sector is currently being drawn
sector_t* gpCurDrawSector;

//------------------------------------------------------------------------------------------------------------------------------------------
// One time setup for the 3D view renderer
//------------------------------------------------------------------------------------------------------------------------------------------
void R_Init() noexcept {
    // Initialize texture lists, palettes etc.
    R_InitData();

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
    // If currently in fullbright mode (no lighting) then setup the light params now
    if (!gbDoViewLighting) {
        gCurLightValR = 128;
        gCurLightValG = 128;
        gCurLightValB = 128;
        gpCurLight = &gpLightsLump[0];
    }

    // Store view parameters before drawing
    player_t& player = gPlayers[gCurPlayerIndex];

    gpViewPlayer = &player;
    gViewX = player.mo->x & (~FRACMASK);
    gViewY = player.mo->y & (~FRACMASK);
    gViewZ = player.viewz & (~FRACMASK);
    gViewAngle = player.mo->angle;
    gViewCos = gFineCosine[gViewAngle >> ANGLETOFINESHIFT];
    gViewSin = gFineSine[gViewAngle >> ANGLETOFINESHIFT];
    
    // Set the draw matrix and upload to the GTE
    gDrawMatrix.m[0][0] = (int16_t)( gViewSin >> GTE_ROTFRAC_SHIFT);
    gDrawMatrix.m[0][2] = (int16_t)(-gViewCos >> GTE_ROTFRAC_SHIFT);
    gDrawMatrix.m[2][0] = (int16_t)( gViewCos >> GTE_ROTFRAC_SHIFT);
    gDrawMatrix.m[2][2] = (int16_t)( gViewSin >> GTE_ROTFRAC_SHIFT);
    LIBGTE_SetRotMatrix(gDrawMatrix);

    // Traverse the BSP tree to determine what needs to be drawn and in what order.
    R_BSP();
    
    // Stat tracking: how many subsectors will we draw?
    gNumDrawSubsectors = (int32_t)(gppEndDrawSubsector - gpDrawSubsectors);

    // Finish up the previous draw before we continue and draw the sky if currently visible
    I_DrawPresent();

    if (gbIsSkyVisible) {
        R_DrawSky();
    }
    
    // Draw all subsectors emitted during BSP traversal.
    // Draw them in back to front order.
    while (gppEndDrawSubsector > gpDrawSubsectors) {
        --gppEndDrawSubsector;

        // Set the current draw sector
        subsector_t& subsec = **gppEndDrawSubsector;
        sector_t& sec = *subsec.sector;
        gpCurDrawSector = &sec;

        // Setup the lighting values to use for the sector
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
        
        R_DrawSubsector(subsec);
    }

    // Draw any player sprites
    R_DrawWeapon();

    // Clearing the texture window: this is probably not required?
    // Not sure what the reason is for this, but everything seems to work fine without doing this.
    // I'll leave the code as-is for now though until I have a better understanding of this, just in case something breaks.
    {
        RECT texWinRect;
        LIBGPU_setRECT(texWinRect, 0, 0, 0, 0);

        DR_TWIN* const pTexWinPrim = (DR_TWIN*) LIBETC_getScratchAddr(128);
        LIBGPU_SetTexWindow(*pTexWinPrim, texWinRect);
        I_AddPrim(pTexWinPrim);
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
    const int32_t lprod = (dx >> FRACBITS) * (node.line.dy >> FRACBITS);
    const int32_t rprod = (dy >> FRACBITS) * (node.line.dx >> FRACBITS);
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

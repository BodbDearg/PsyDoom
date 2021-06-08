#pragma once

#include "Doom/doomdef.h"

#include <vector>

// GTE rotation constants: 1.0 and the shift to go from 16.16 to 4.12.
// The GTE stores rotation matrix entries as 4.12 fixed point numbers!
static constexpr int16_t    GTE_ROTFRAC_UNIT    = 4096;
static constexpr uint16_t   GTE_ROTFRAC_SHIFT   = 4;

// Renderer constant: maximum number of subsectors that can be drawn in the original engine
#if !PSYDOOM_LIMIT_REMOVING
    static constexpr uint32_t MAX_DRAW_SUBSECTORS = 192;
#endif

// Renderer constant: clip geometry at this depth
static constexpr int32_t NEAR_CLIP_DIST = 2;

struct light_t;
struct MATRIX;
struct node_t;
struct subsector_t;

extern int32_t          gValidCount;
extern player_t*        gpViewPlayer;
extern fixed_t          gViewX;
extern fixed_t          gViewY;
extern fixed_t          gViewZ;
extern angle_t          gViewAngle;
extern fixed_t          gViewCos;
extern fixed_t          gViewSin;
extern bool             gbIsSkyVisible;
extern MATRIX           gDrawMatrix;
extern bool             gbDoViewLighting;
extern const light_t*   gpCurLight;
extern uint32_t         gCurLightValR;
extern uint32_t         gCurLightValG;
extern uint32_t         gCurLightValB;

// PsyDoom: the number of draw subsectors is now unlimited
#if PSYDOOM_LIMIT_REMOVING
    extern std::vector<subsector_t*> gpDrawSubsectors;
#else
    extern subsector_t*     gpDrawSubsectors[MAX_DRAW_SUBSECTORS];
    extern int32_t          gNumDrawSubsectors;
#endif

extern sector_t*        gpCurDrawSector;
extern subsector_t**    gppEndDrawSubsector;

#if PSYDOOM_MODS
    extern fixed_t      gOldViewX;
    extern fixed_t      gOldViewY;
    extern fixed_t      gOldViewZ;
    extern angle_t      gOldViewAngle;
    extern fixed_t      gOldAutomapX;
    extern fixed_t      gOldAutomapY;
    extern fixed_t      gOldAutomapScale;
    extern bool         gbSnapViewZInterpolation;
#endif

void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
int32_t R_SlopeDiv(const uint32_t num, const uint32_t den) noexcept;
angle_t R_PointToAngle2(const fixed_t x1, const fixed_t y1, const fixed_t x2, const fixed_t y2) noexcept;
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept;
subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept;

#if PSYDOOM_MODS
    void R_NextInterpolation() noexcept;
    void R_SnapViewZInterpolation() noexcept;
    fixed_t R_CalcLerpFactor() noexcept;
    fixed_t R_LerpCoord(const fixed_t oldCoord, const fixed_t newCoord, const fixed_t mix) noexcept;
    angle_t R_LerpAngle(const angle_t oldAngle, const angle_t newAngle, const fixed_t mix) noexcept;
    bool R_HasHigherSurroundingSkyCeiling(const sector_t& sector) noexcept;
    bool R_HasLowerSurroundingSkyFloor(const sector_t& sector) noexcept;
    void R_UpdateFloorDrawHeight(sector_t& sector) noexcept;
    void R_UpdateShadingParams(sector_t& sector) noexcept;
    light_t R_GetZColor(const sector_t& sector, const fixed_t z) noexcept;
#endif

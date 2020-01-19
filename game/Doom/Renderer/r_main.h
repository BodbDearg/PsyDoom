#pragma once

#include "Doom/doomdef.h"

// GTE rotation constants: 1.0 and the shift to go from 16.16 to 4.12.
// The GTE stores rotation matrix entries as 4.12 fixed point numbers!
static constexpr int16_t    GTE_ROTFRAC_UNIT    = 4096;
static constexpr uint16_t   GTE_ROTFRAC_SHIFT   = 4;

// Renderer general constants
static constexpr uint32_t   MAX_DRAW_SUBSECTORS = 192;
static constexpr int32_t    NEAR_CLIP_DIST      = 2;        // Clip geometry at this depth

struct light_t;
struct MATRIX;
struct node_t;
struct subsector_t;

extern const VmPtr<VmPtr<player_t>>                             gpViewPlayer;
extern const VmPtr<fixed_t>                                     gViewX;
extern const VmPtr<fixed_t>                                     gViewY;
extern const VmPtr<fixed_t>                                     gViewZ;
extern const VmPtr<angle_t>                                     gViewAngle;
extern const VmPtr<fixed_t>                                     gViewCos;
extern const VmPtr<fixed_t>                                     gViewSin;
extern const VmPtr<bool32_t>                                    gbIsSkyVisible;
extern const VmPtr<MATRIX>                                      gDrawMatrix;
extern const VmPtr<bool32_t>                                    gbDoViewLighting;
extern const VmPtr<VmPtr<const light_t>>                        gpCurLight;
extern const VmPtr<uint32_t>                                    gCurLightValR;
extern const VmPtr<uint32_t>                                    gCurLightValG;
extern const VmPtr<uint32_t>                                    gCurLightValB;
extern const VmPtr<VmPtr<subsector_t>[MAX_DRAW_SUBSECTORS]>     gpDrawSubsectors;
extern const VmPtr<VmPtr<VmPtr<subsector_t>>>                   gppEndDrawSubsector;
extern const VmPtr<VmPtr<sector_t>>                             gpCurDrawSector;

void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
void R_SlopeDiv() noexcept;
void R_PointToAngle2() noexcept;
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept;

subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept;
void _thunk_R_PointInSubsector() noexcept;

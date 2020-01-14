#pragma once

#include "Doom/doomdef.h"

struct MATRIX;
struct node_t;
struct subsector_t;

extern const VmPtr<angle_t>     gViewAngle;
extern const VmPtr<bool32_t>    gbIsSkyVisible;
extern const VmPtr<MATRIX>      gDrawMatrix;
extern const VmPtr<bool32_t>    gbDoViewLighting;
extern const VmPtr<uint32_t>    gCurLightValR;
extern const VmPtr<uint32_t>    gCurLightValG;
extern const VmPtr<uint32_t>    gCurLightValB;

void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
void R_SlopeDiv() noexcept;
void R_PointToAngle2() noexcept;
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept;

subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept;
void _thunk_R_PointInSubsector() noexcept;

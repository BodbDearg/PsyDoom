#pragma once

#include "Doom/doomdef.h"

struct node_t;
struct subsector_t;

extern const VmPtr<angle_t> gViewAngle;

void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
void R_SlopeDiv() noexcept;
void R_PointToAngle2() noexcept;
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept;

subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept;
void _thunk_R_PointInSubsector() noexcept;

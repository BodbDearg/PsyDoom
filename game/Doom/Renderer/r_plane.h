#pragma once

#include "Doom/doomdef.h"

struct leaf_t;
struct texture_t;

void R_DrawSubsectorFlat(leaf_t& leaf, const bool bIsCeiling) noexcept;
void R_DrawFlatSpans(leaf_t& leaf, const fixed_t planeViewZ, const texture_t& tex) noexcept;

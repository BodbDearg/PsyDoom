#pragma once

#include <cstdint>

struct leafedge_t;
struct texture_t;

void R_DrawWalls(leafedge_t& edge) noexcept;

void R_DrawWallPiece(
    const leafedge_t& edge,
    const texture_t& tex,
    const int32_t yt,
    const int32_t yb,
    const int32_t ut,
    const int32_t ub,
    bool bTransparent
) noexcept;

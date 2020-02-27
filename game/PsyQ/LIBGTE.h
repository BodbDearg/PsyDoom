#pragma once

#include <cstdint>

// Represents a basic transform matrix encoding a 3D rotation and translation
struct MATRIX {
    int16_t     m[3][3];    // Rotation matrix: 3x3
    int32_t     t[3];       // Translation
};

static_assert(sizeof(MATRIX) == 32);

// 3D vector with 32-bit elements
struct VECTOR {
    int32_t     vx;
    int32_t     vy;
    int32_t     vz;
    int32_t     pad;
};

static_assert(sizeof(VECTOR) == 16);

// 3D vector with 16-bit elements
struct SVECTOR {
    int16_t     vx;
    int16_t     vy;
    int16_t     vz;
    int16_t     pad;
};

static_assert(sizeof(SVECTOR) == 8);

void LIBGTE_SetRotMatrix(const MATRIX& m) noexcept;
void LIBGTE_SetTransMatrix(const MATRIX& m) noexcept;
void LIBGTE_SetGeomOffset(const int32_t x, const int32_t y) noexcept;
void LIBGTE_SetGeomScreen(const int32_t h) noexcept;
void LIBGTE_InitGeom() noexcept;
void LIBGTE_RotTrans(const SVECTOR& vecIn, VECTOR& vecOut, int32_t& flagsOut) noexcept;

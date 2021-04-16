#pragma once

#include "Asserts.h"

#include <algorithm>
#include <cmath>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a no-frills 4x4 float matrix; provides some very basic matrix operations.
// For the purposes of transforms the matrix is assumed to be column major.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
struct Matrix4 {
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Default constructor that creates an identity matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline Matrix4() noexcept {
        e[0][0] = 1;    e[0][1] = 0;    e[0][2] = 0;    e[0][3] = 0;
        e[1][0] = 0;    e[1][1] = 1;    e[1][2] = 0;    e[1][3] = 0;
        e[2][0] = 0;    e[2][1] = 0;    e[2][2] = 1;    e[2][3] = 0;
        e[3][0] = 0;    e[3][1] = 0;    e[3][2] = 0;    e[3][3] = 1;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Element by element matrix constructor
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline Matrix4(
        const T r0c0,
        const T r0c1,
        const T r0c2,
        const T r0c3,
        const T r1c0,
        const T r1c1,
        const T r1c2,
        const T r1c3,
        const T r2c0,
        const T r2c1,
        const T r2c2,
        const T r2c3,
        const T r3c0,
        const T r3c1,
        const T r3c2,
        const T r3c3
    ) noexcept {
        e[0][0] = r0c0;     e[0][1] = r0c1;     e[0][2] = r0c2;     e[0][3] = r0c3;
        e[1][0] = r1c0;     e[1][1] = r1c1;     e[1][2] = r1c2;     e[1][3] = r1c3;
        e[2][0] = r2c0;     e[2][1] = r2c1;     e[2][2] = r2c2;     e[2][3] = r2c3;
        e[3][0] = r3c0;     e[3][1] = r3c1;     e[3][2] = r3c2;     e[3][3] = r3c3;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Construct from an array of elements
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline Matrix4(const T* const m[4][4]) noexcept {
        e[0][0] = m[0][0];  e[0][1] = m[0][1];  e[0][2] = m[0][2];  e[0][3] = m[0][3];
        e[1][0] = m[1][0];  e[1][1] = m[1][1];  e[1][2] = m[1][2];  e[1][3] = m[1][3];
        e[2][0] = m[2][0];  e[2][1] = m[2][1];  e[2][2] = m[2][2];  e[2][3] = m[2][3];
        e[3][0] = m[3][0];  e[3][1] = m[3][1];  e[3][2] = m[3][2];  e[3][3] = m[3][3];
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Copy from another matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    Matrix4(const Matrix4& other) noexcept = default;
    Matrix4& operator = (const Matrix4& other) noexcept = default;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Matrix multiplication with another matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline Matrix4 operator * (const Matrix4& m) const noexcept {
        return Matrix4(
            e[0][0] * m.e[0][0] + e[0][1] * m.e[1][0] + e[0][2] * m.e[2][0] + e[0][3] * m.e[3][0],  // R0C0
            e[0][0] * m.e[0][1] + e[0][1] * m.e[1][1] + e[0][2] * m.e[2][1] + e[0][3] * m.e[3][1],  // R0C1
            e[0][0] * m.e[0][2] + e[0][1] * m.e[1][2] + e[0][2] * m.e[2][2] + e[0][3] * m.e[3][2],  // R0C2
            e[0][0] * m.e[0][3] + e[0][1] * m.e[1][3] + e[0][2] * m.e[2][3] + e[0][3] * m.e[3][3],  // R0C3

            e[1][0] * m.e[0][0] + e[1][1] * m.e[1][0] + e[1][2] * m.e[2][0] + e[1][3] * m.e[3][0],  // R1C0
            e[1][0] * m.e[0][1] + e[1][1] * m.e[1][1] + e[1][2] * m.e[2][1] + e[1][3] * m.e[3][1],  // R1C1
            e[1][0] * m.e[0][2] + e[1][1] * m.e[1][2] + e[1][2] * m.e[2][2] + e[1][3] * m.e[3][2],  // R1C2
            e[1][0] * m.e[0][3] + e[1][1] * m.e[1][3] + e[1][2] * m.e[2][3] + e[1][3] * m.e[3][3],  // R1C3

            e[2][0] * m.e[0][0] + e[2][1] * m.e[1][0] + e[2][2] * m.e[2][0] + e[2][3] * m.e[3][0],  // R2C0
            e[2][0] * m.e[0][1] + e[2][1] * m.e[1][1] + e[2][2] * m.e[2][1] + e[2][3] * m.e[3][1],  // R2C1
            e[2][0] * m.e[0][2] + e[2][1] * m.e[1][2] + e[2][2] * m.e[2][2] + e[2][3] * m.e[3][2],  // R2C2
            e[2][0] * m.e[0][3] + e[2][1] * m.e[1][3] + e[2][2] * m.e[2][3] + e[2][3] * m.e[3][3],  // R2C3

            e[3][0] * m.e[0][0] + e[3][1] * m.e[1][0] + e[3][2] * m.e[2][0] + e[3][3] * m.e[3][0],  // R3C0
            e[3][0] * m.e[0][1] + e[3][1] * m.e[1][1] + e[3][2] * m.e[2][1] + e[3][3] * m.e[3][1],  // R3C1
            e[3][0] * m.e[0][2] + e[3][1] * m.e[1][2] + e[3][2] * m.e[2][2] + e[3][3] * m.e[3][2],  // R3C2
            e[3][0] * m.e[0][3] + e[3][1] * m.e[1][3] + e[3][2] * m.e[2][3] + e[3][3] * m.e[3][3]   // R3C3
        );
    }

    inline void operator *= (const Matrix4& m) noexcept {
        *this = (*this) * m;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get/set a row of the matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void getRow(const size_t row, T dst[4]) const noexcept {
        ASSERT(row < 4);
        dst[0] = e[row][0];
        dst[1] = e[row][1];
        dst[2] = e[row][2];
        dst[3] = e[row][3];
    }

    inline void setRow(const size_t row, const T src[4]) noexcept {
        ASSERT(row < 4);
        e[row][0] = src[0];
        e[row][1] = src[1];
        e[row][2] = src[2];
        e[row][3] = src[3];
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Get/set a column of the matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void getCol(const size_t col, T dst[4]) const noexcept {
        ASSERT(col < 4);
        dst[0] = e[0][col];
        dst[1] = e[1][col];
        dst[2] = e[2][col];
        dst[3] = e[3][col];
    }

    inline void setColumn(const size_t col, const T src[4]) noexcept {
        ASSERT(col < 4);
        e[0][col] = src[0];
        e[1][col] = src[1];
        e[2][col] = src[2];
        e[3][col] = src[3];
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Swap rows with columns in the matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void transpose() noexcept {
        std::swap(e[0][1], e[1][0]);
        std::swap(e[0][2], e[2][0]);
        std::swap(e[0][3], e[3][0]);
        std::swap(e[1][2], e[2][1]);
        std::swap(e[1][3], e[3][1]);
        std::swap(e[2][3], e[3][2]);
    }

    inline Matrix4 transposed() const noexcept {
        return Matrix4(
            e[0][0], e[1][0], e[2][0], e[3][0],
            e[0][1], e[1][1], e[2][1], e[3][1],
            e[0][2], e[1][2], e[2][2], e[3][2],
            e[0][3], e[1][3], e[2][3], e[3][3]
        );
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Transforms the given 3D vector by this matrix and returns the result.
    // The vector is expanded to 4D for the calculation and the W component can be optionally specified (defaults to '1').
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void transform3d(const T src[3], T dst[3], const T srcW = T(1)) const noexcept {
        const float x = src[0] * e[0][0] + src[1] * e[1][0] + src[2] * e[2][0] + srcW * e[3][0];
        const float y = src[0] * e[0][1] + src[1] * e[1][1] + src[2] * e[2][1] + srcW * e[3][1];
        const float z = src[0] * e[0][2] + src[1] * e[1][2] + src[2] * e[2][2] + srcW * e[3][2];
        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Transforms the given 4D vector by this matrix and returns the result
    //--------------------------------------------------------------------------------------------------------------------------------------
    inline void transform4d(const T src[4], T dst[4]) const noexcept {
        const float x = src[0] * e[0][0] + src[1] * e[1][0] + src[2] * e[2][0] + src[3] * e[3][0];
        const float y = src[0] * e[0][1] + src[1] * e[1][1] + src[2] * e[2][1] + src[3] * e[3][1];
        const float z = src[0] * e[0][2] + src[1] * e[1][2] + src[2] * e[2][2] + src[3] * e[3][2];
        const float w = src[0] * e[0][3] + src[1] * e[1][3] + src[2] * e[2][3] + src[3] * e[3][3];
        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
        dst[3] = w;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Return the identity matrix
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 IDENTITY() noexcept {
        return Matrix4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Creates a translation matrix to translate in 3d by using the xyz values of the given vector
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 translate(const T x, const T y, const T z) noexcept {
        return Matrix4(
            1,  0,  0,  0,
            0,  1,  0,  0,
            0,  0,  1,  0,
            x,  y,  z,  1
        );
    }

    static inline Matrix4 translate(const T v[3]) noexcept {
        return translate(v[0], v[1], v[2]);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Create rotation matrices about either the x, y or z axes
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 rotateX(const T angleInRadians) noexcept {
        const T c = std::cos(angleInRadians);
        const T s = std::sin(angleInRadians);

        return Matrix4(
            1,  0,  0,  0,
            0,  c,  s,  0,
            0, -s,  c,  0,
            0,  0,  0,  1
        );
    }

    static inline Matrix4 rotateY(const T angleInRadians) noexcept {
        const T c = std::cos(angleInRadians);
        const T s = std::sin(angleInRadians);

        return Matrix4(
            c,  0, -s,  0,
            0,  1,  0,  0,
            s,  0,  c,  0,
            0,  0,  0,  1
        );
    }

    static inline Matrix4 rotateZ(const T angleInRadians) noexcept {
        const T c = std::cos(angleInRadians);
        const T s = std::sin(angleInRadians);

        return Matrix4(
             c,  s,  0,  0, 
            -s,  c,  0,  0,
             0,  0,  1,  0,
             0,  0,  0,  1
        );
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Creates a scaling transform in 3d.
    // Scales in xyz dimensions according to the xyz components of the given vector.
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 scale(const T x, const T y, const T z) noexcept {
        return Matrix4(
            x,  0,  0,  0,
            0,  y,  0,  0,
            0,  0,  z,  0,
            0,  0,  0,  1
        );
    }

    static inline Matrix4 scale(const T s[3]) noexcept {
        return scale(s[0], s[1], s[2]);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Creates an inverse scaling transform in 3d, i.e one that divides by the given scale rather than multiplying.
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 invScale(const T x, const T y, const T z) noexcept {
        return Matrix4(
            1 / x,  0,      0,      0,
            0,      1 / y,  0,      0,
            0,      0,      1 / z,  0,
            0,      0,      0,      1
        );
    }

    static inline Matrix4 invScale(const T s[3]) noexcept {
        return invScale(s[0], s[1], s[2]);
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Creates a right handed orthographic off-center projection matrix
    //
    // @param lx    View plane left x value
    // @param rx    View plane right x value
    // @param ty    View plane top y value
    // @param by    View plane bottom y value
    // @param zn    Near plane z value
    // @param zf    Far plane z value
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 orthographicOffCenter(
        const T lx,
        const T rx,
        const T ty,
        const T by,
        const T zn,
        const T zf
    ) noexcept {
        // Note: the calculations here are based on the DirectX 9 utility function 'D3DXMatrixOrthoOffCenterRH()'.
        // Some adjustments were made however for this Vulkan:
        //
        //  (1) Negated 'e[1][1]' to account for Vukan's new NDC and flipped y.
        //      See: https://matthewwellings.com/blog/the-new-vulkan-coordinate-system
        //  (2) Reversed top and bottom y subtraction to account for Vulkan's flipped y.
        //
        return Matrix4(
            2 / (rx - lx),          0,                      0,               0,
            0,                      2 / (by - ty),          0,               0,
            0,                      0,                      1  / (zn - zf),  0,
            (lx + rx) / (lx - rx),  (ty + by) / (ty - by),  zn / (zn - zf),  1
        );
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Creates a right handed projection matrix.
    // Uses the given dimensions of the view frustrum front plane (viewing plane) to construct the matrix.
    // 
    //
    // @param lx    View plane left x value
    // @param rx    View plane right x value
    // @param ty    View plane top y value
    // @param by    View plane bottom y value
    // @param zn    Near plane z value
    // @param zf    Far plane z value
    //--------------------------------------------------------------------------------------------------------------------------------------
    static inline Matrix4 perspectiveOffCenter(
        const T lx,
        const T rx,
        const T ty,
        const T by,
        const T zn,
        const T zf
    ) noexcept {
        // Note: the calculations here are based on the DirectX 9 utility function 'D3DXMatrixPerspectiveOffCenterRH()'.
        // Some adjustments were made however for this Vulkan:
        //
        //  (1) Negated 'e[1][1]' to account for Vukan's new NDC and flipped y.
        //      See: https://matthewwellings.com/blog/the-new-vulkan-coordinate-system
        //  (2) Negated 'e[2][2]' and 'e[2][3]' to make Z go INTO the screen rather than come out from it.
        //
        return Matrix4(
            2 * zn / (rx - lx),     0,                      0,                    0,
            0,                      -2 * zn / (ty - by),    0,                    0,
            (lx + rx) / (rx - lx),  (ty + by) / (ty - by),  -zf / (zn - zf),      1,
            0,                      0,                      zn * zf / (zn - zf),  0
        );
    }

    // The actual elements of the matrix
    T e[4][4];
};

// Different precision versions of this class
typedef Matrix4<float>      Matrix4f;
typedef Matrix4<double>     Matrix4d;

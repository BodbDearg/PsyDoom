#include "ShaderCommon.h"

//----------------------------------------------------------------------------------------------------------------------
// Declare the vertex inputs for the vertex type 'VVertex_Draw'.
// Depending on the shaders involved, some of these attributes may be ignored.
//
// Meaning:
//
//  in_pos              Vertex XYZ position.
//  in_color            RGB color for the vertex where 128.0 = fully white.
//                      Values over 128.0 are overbright.
//  in_lightDimMode     Light diminishing mode (VLightDimMode) for the 3D view shaders only. Ignored by UI shaders.
//  in_uv               UV texture coordinate (in pixels)
//  in_texWinPos        Top left XY position (in current format pixel coords) of the texture wrapping window
//  in_texWinSize       Top left XY position (in current format pixel coords) of the texture wrapping window
//  in_clutPos          XY position (in 16 bpp pixel coords) of the color lookup table for 4/8-bit textures
//  in_stmul            RGBA multiplier for when a sampled texel is marked semi-transparent (controls blending)
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_VS_INPUTS_VVERTEX_DRAW()\
    layout(location = 0) in vec3    in_pos;             \
    layout(location = 1) in vec3    in_color;           \
    layout(location = 2) in uint    in_lightDimMode;    \
    layout(location = 3) in vec2    in_uv;              \
    layout(location = 4) in uvec2   in_texWinPos;       \
    layout(location = 5) in uvec2   in_texWinSize;      \
    layout(location = 6) in uvec2   in_clutPos;         \
    layout(location = 7) in vec4    in_stmul;

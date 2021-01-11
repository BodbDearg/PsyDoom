#version 460

//----------------------------------------------------------------------------------------------------------------------
// Fragment shader for rendering UI sprites (16bpp).
// Does texture mapping (@16bpp), coloring and adjustments for blending.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;
layout(location = 2) flat in uvec2 in_texWinPos;
layout(location = 3) flat in uvec2 in_texWinSize;
layout(location = 4) flat in uvec2 in_clutPos;      // Unused for 16-bpp mode
layout(location = 5) flat in vec4 in_stmul;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = tex16bpp(vramTex, in_uv, in_texWinPos, in_texWinSize, in_stmul);
    out_color *= in_color;
}

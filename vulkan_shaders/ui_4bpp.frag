#version 460

//----------------------------------------------------------------------------------------------------------------------
// Fragment shader for rendering UI sprites (4bpp).
// Does texture mapping (@4bpp) with clamp to texture window, coloring and adjustments for blending.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Frag.h"

// Whether to shade with 16-bit precision like the original PlayStation
layout(constant_id = 1) const bool USE_PSX_16_BIT_SHADING = true;

// A texture containing the entirety of PSX VRAM
layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) flat in ivec2 in_texWinPos;
layout(location = 3) flat in ivec2 in_texWinSize;
layout(location = 4) flat in ivec2 in_clutPos;
layout(location = 5) flat in vec4 in_stmul;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = tex4bpp(vramTex, ivec2(floor(in_uv)), in_texWinPos, in_texWinSize, in_clutPos, in_stmul, false, true);
    out_color.rgb *= in_color;

    if (USE_PSX_16_BIT_SHADING) {
        out_color = psxR5G5B5BitCrush(out_color);
    }
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// View shader: fragment.
// The view shader is responsible for drawing stuff in the 3D/world view.
// Very similar to the UI shaders except it includes a 'scale' value for light diminishing effects.
// All view textures are also assumed to be 8 bits per pixel.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_uv_scale;
layout(location = 2) flat in uvec2 in_texWinPos;
layout(location = 3) flat in uvec2 in_texWinSize;
layout(location = 4) flat in uvec2 in_clutPos;
layout(location = 5) flat in vec4 in_stmul;

layout(location = 0) out vec4 out_color;

void main() {
    // TODO: light diminishing effects
    out_color = tex8bpp(vramTex, in_uv_scale.xy, in_texWinPos, in_texWinSize, in_clutPos, in_stmul);
    out_color *= in_color;
    out_color = psxR5G5B5BitCrush(out_color);
}

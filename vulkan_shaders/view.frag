#version 460

//----------------------------------------------------------------------------------------------------------------------
// View shader: fragment.
// The view shader is responsible for drawing stuff in the 3D/world view.
// Very similar to the UI shaders except it includes a 'scale' value for light diminishing effects.
// All view textures are also assumed to be 8 bits per pixel.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_uv_z;
layout(location = 2) flat in vec3 in_lightDimModeStrength;
layout(location = 3) flat in uvec2 in_texWinPos;
layout(location = 4) flat in uvec2 in_texWinSize;
layout(location = 5) flat in uvec2 in_clutPos;
layout(location = 6) flat in vec4 in_stmul;

layout(location = 0) out vec4 out_color;

void main() {
    // Grab the basic texel
    out_color = tex8bpp(vramTex, in_uv_z.xy, in_texWinPos, in_texWinSize, in_clutPos, in_stmul);

    // Compute color multiply after accounting for input color and light diminishing effects.
    // Add a little bias also to prevent switching back and forth between cases that are close, due to float inprecision...
    vec3 colorMul = trunc(in_color * getLightDiminishingMultiplier(in_uv_z.z, in_lightDimModeStrength) + 0.0001) / 128.0;

    // Apply the color multiply and bit crush the color in a manner similar to the PSX
    out_color.rgb *= colorMul;
    out_color = psxR5G5B5BitCrush(out_color);
}

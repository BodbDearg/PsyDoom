#version 460

//----------------------------------------------------------------------------------------------------------------------
// Vertex shader used in conjunction with all UI fragment shaders.
// Used for rendering UI sprites and other 2D elements.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()
DECLARE_VS_INPUTS_VVERTEX_DRAW()

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) flat out ivec2 out_texWinPos;      // Note: pre-convert these to ivec2 for the frag shader
layout(location = 3) flat out ivec2 out_texWinSize;
layout(location = 4) flat out ivec2 out_clutPos;
layout(location = 5) flat out vec4 out_stmul;

void main() {
    gl_Position = uniforms.mvpMatrix * vec4(in_pos, 1);
    out_color = in_color / 128.0;
    out_uv = in_uv;
    out_texWinPos = ivec2(in_texWinPos);
    out_texWinSize = ivec2(in_texWinSize);
    out_clutPos = ivec2(in_clutPos);
    out_stmul = in_stmul / 128.0;
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// Vertex shader used in conjunction with all UI fragment shaders.
// Used for rendering UI sprites and other 2D elements.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

DECLARE_UNIFORMS()
DECLARE_VERTEX_SHADER_INPUTS()

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) flat out uvec2 out_texWinPos;
layout(location = 3) flat out uvec2 out_texWinSize;
layout(location = 4) flat out uvec2 out_clutPos;
layout(location = 5) flat out vec4 out_stmul;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
    out_color = in_color / 128.0;
    out_uv = in_uv;
    out_texWinPos = in_texWinPos;
    out_texWinSize = in_texWinSize;
    out_clutPos = in_clutPos;
    out_stmul = in_stmul / 128.0;
}

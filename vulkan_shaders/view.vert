#version 460

//----------------------------------------------------------------------------------------------------------------------
// View shader: vertex.
// The view shader is responsible for drawing stuff in the 3D/world view.
// Very similar to the UI shaders except it includes a 'scale' value for light diminishing effects.
// All view textures are also assumed to be 8 bits per pixel.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

DECLARE_UNIFORMS()
DECLARE_VERTEX_SHADER_INPUTS()

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_uv_scale;
layout(location = 2) flat out uvec2 out_texWinPos;
layout(location = 3) flat out uvec2 out_texWinSize;
layout(location = 4) flat out uvec2 out_clutPos;
layout(location = 5) flat out vec4 out_stmul;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos_scale.xyz, 1);
    out_color = in_color / 128.0;
    out_uv_scale = vec3(in_uv, in_pos_scale.w);
    out_texWinPos = in_texWinPos;
    out_texWinSize = in_texWinSize;
    out_clutPos = in_clutPos;
    out_stmul = in_stmul / 128.0;
}

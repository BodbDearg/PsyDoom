#version 460

//----------------------------------------------------------------------------------------------------------------------
// Simple vertex shader that transforms input vertices by a model view projection matrix.
// The vertices are then output with the same input color that they receive on input.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon.h"

DECLARE_UNIFORMS()
DECLARE_VERTEX_SHADER_INPUTS()

layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
    out_color.rgb = in_color;
    out_color.a = in_stmul.a;
    out_color /= 128.0;
}

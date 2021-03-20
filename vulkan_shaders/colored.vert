#version 460

//----------------------------------------------------------------------------------------------------------------------
// Simple vertex shader that transforms input vertices by a model view projection matrix.
// The vertices are then output with the same input color that they receive on input.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()
DECLARE_VS_INPUTS_VVERTEX_DRAW()

layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = uniforms.mvpMatrix * vec4(in_pos, 1);
    out_color.rgb = vec3(in_color_lightDimMode.rgb);
    out_color.a = in_stmul.a;
    out_color /= 128.0;
}

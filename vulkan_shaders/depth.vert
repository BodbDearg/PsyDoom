#version 460

//----------------------------------------------------------------------------------------------------------------------
// Depth render shader: vertex.
// Used just to output depth to the depth buffer, for the purposes of masking sprites and geometry.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()

layout(location = 0) in vec3 in_pos;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// Occluder plane shader: vertex.
// Simply draws the linear depth of the plane to the occluder plane buffer.
// These depths are used to decide when pixels from sprites and masked walls are occluded.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()

layout(location = 0) in vec3 in_pos;
layout(location = 0) out float out_depth;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
    out_depth = gl_Position.z;
}

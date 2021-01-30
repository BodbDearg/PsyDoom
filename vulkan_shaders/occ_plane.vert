#version 460

//----------------------------------------------------------------------------------------------------------------------
// Occlusion plane shader: vertex.
// The occlusion plane shader simply encodes occlusion plane info for walls into an output render target.
// These occlusion planes are used to decide when pixels from sprites and masked walls are occluded.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()

layout(location = 0) in vec3 in_pos;
layout(location = 1) in int in_planeAngle;
layout(location = 2) in int in_planeOffset;

layout(location = 0) flat out ivec2 out_planeInfo;

void main() {
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
    out_planeInfo.x = in_planeAngle;
    out_planeInfo.y = in_planeOffset;
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// Simple vertex shader that transforms input vertices with color by a model view projection matrix.
// The vertices are then output with the same input color.
//----------------------------------------------------------------------------------------------------------------------
layout(push_constant) uniform Constants {
    mat4 mvpMatrix;
} constants;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = in_color;
    gl_Position = constants.mvpMatrix * vec4(in_position, 1);
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// Simple vertex shader that transforms input vertices with color by a model view projection matrix.
// The vertices are then output with the same input color.
//----------------------------------------------------------------------------------------------------------------------
layout(push_constant) uniform Constants {
    mat4    mvpMatrix;
    uvec2   texWinXY;
    uvec2   texWinWH;
    uvec2   clutXY;
    vec4    semiTransMul;
} constants;

layout(location = 0) in vec4 in_pos_scale;      // Note: 'scale' or 'w' member unused by this shader
layout(location = 1) in vec4 in_color;
layout(location = 2) in uvec2 in_uv;            // Note: texcoord/uv unused by this shader
layout(location = 0) out vec4 out_color;

void main() {
    out_color = in_color;
    gl_Position = constants.mvpMatrix * vec4(in_pos_scale.xyz, 1);
}

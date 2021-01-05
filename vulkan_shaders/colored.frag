#version 460

//----------------------------------------------------------------------------------------------------------------------
// Simple fragment shader that outputs input vertex colors
//----------------------------------------------------------------------------------------------------------------------
layout(location = 0) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = in_color;
}

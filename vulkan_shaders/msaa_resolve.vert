#version 460

//----------------------------------------------------------------------------------------------------------------------
// MSAA resolve shader: vertex.
// The MSAA resolve shader simply samples from a multi-sampled texture and then averages all of the samples.
//----------------------------------------------------------------------------------------------------------------------
layout(location = 0) in vec2 in_pos;

void main() {
    gl_Position = vec4(in_pos, 0, 1);   // Note: not using any transforms, just work in normalized device coords
}

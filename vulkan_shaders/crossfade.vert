#version 460

//----------------------------------------------------------------------------------------------------------------------
// Crossfade shader: vertex.
// The crossfade shader simply crossfades between two input images.
//----------------------------------------------------------------------------------------------------------------------
layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec2 out_uv;

void main() {
    gl_Position = vec4(in_pos, 0, 1);   // Note: not using any transforms, just work in normalized device coords
    out_uv = in_uv;
}

#version 460

//----------------------------------------------------------------------------------------------------------------------
// Textured normalized device coordinate shader: fragment.
// This shader simply does texture mapping and masking (with no shading) to input geometry in 2D NDC space.
// It can be used for very simple image blitting.
//----------------------------------------------------------------------------------------------------------------------
layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex, in_uv);

    if (out_color.a <= 0)
        discard;
}

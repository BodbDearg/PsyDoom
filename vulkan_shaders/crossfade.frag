#version 460

//----------------------------------------------------------------------------------------------------------------------
// Crossfade fragment shader: this fragment shader simply crossfades between two input images.
// It uses the 'ndc_textured' vertex shader to pass along the input uv.
//----------------------------------------------------------------------------------------------------------------------

// Controls how how much of each texture is mixed in
layout(push_constant) uniform Uniforms {
    float   lerpAmount;
} uniforms;

// The two textures to fade between
layout(set = 0, binding = 0) uniform sampler2D textures[2];

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main() {
    vec4 texel1 = texture(textures[0], in_uv);
    vec4 texel2 = texture(textures[1], in_uv);
    out_color = mix(texel1, texel2, uniforms.lerpAmount);
}

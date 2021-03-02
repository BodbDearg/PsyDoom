#version 460

#include "ShaderCommon_Frag.h"

//----------------------------------------------------------------------------------------------------------------------
// Crossfade fragment shader: this fragment shader simply crossfades between two input images.
// It uses the 'ndc_textured' vertex shader to pass along the input uv.
//----------------------------------------------------------------------------------------------------------------------

// Whether to shade with 16-bit precision like the original PlayStation
layout(constant_id = 0) const bool USE_PSX_16_BIT_SHADING = true;

// Controls how how much of each texture is mixed in
layout(push_constant) uniform Uniforms {
    float   lerpAmount;
} uniforms;

// The two textures to fade between
layout(set = 0, binding = 0) uniform sampler2D tex1;
layout(set = 0, binding = 1) uniform sampler2D tex2;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

void main() {
    vec4 texel1 = texture(tex1, in_uv);
    vec4 texel2 = texture(tex2, in_uv);
    out_color = mix(texel1, texel2, uniforms.lerpAmount);

    if (USE_PSX_16_BIT_SHADING) {
        out_color = psxR5G5B5BitCrush(out_color);
    }
}

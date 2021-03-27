#version 460

//----------------------------------------------------------------------------------------------------------------------
// Sky shader: fragment
// This is used for rendering the sky in parts of the ceiling or on walls that have been marked as 'sky'.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Frag.h"

DECLARE_UNIFORMS()

// Whether to shade with 16-bit precision like the original PlayStation
layout(constant_id = 1) const bool USE_PSX_16_BIT_SHADING = true;

// A texture containing the entirety of PSX VRAM
layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) flat in vec2 in_uv_offset;
layout(location = 1) flat in ivec2 in_texWinPos;
layout(location = 2) flat in ivec2 in_texWinSize;
layout(location = 3) flat in ivec2 in_clutPos;

layout(location = 0) out vec4 out_color;

//----------------------------------------------------------------------------------------------------------------------
// Get the coordinate of this fragment in terms of the original PSX framebuffer coordinate system (256x240)
//----------------------------------------------------------------------------------------------------------------------
ivec2 getPixelPsxCoords() {
    // Get a normalized device coordinate from 0-2
    vec2 fragCoord = floor(gl_FragCoord.xy) + 0.5;
    vec2 ndc = (fragCoord * uniforms.invDrawRes) * 2.0;

    // Convert to the PSX coord system
    return ivec2(floor((ndc - uniforms.psxNdcOffset) * uniforms.ndcToPsxScale));
}

//----------------------------------------------------------------------------------------------------------------------
// Shades the sky fragment
//----------------------------------------------------------------------------------------------------------------------
void main() {
    // Get the fragment's position in terms of PSX framebuffer coordinates
    ivec2 psxCoords = getPixelPsxCoords();

    // The uv coord to use for the sky is based on the PSX framebuffer coord.
    // We also add offsetting to account for view rotation and that is stored in the vertex uv attribute.
    ivec2 uv = psxCoords + ivec2(floor(in_uv_offset));

    // Don't draw the sky past where it should be drawn
    if (uv.y >= in_texWinSize.y)
        discard;

    // Just do a standard 8bpp texel lookup to draw the sky.
    // Don't allow texels to be discarded for the sky however!
    out_color = tex8bpp(vramTex, uv, in_texWinPos, in_texWinSize, in_clutPos, vec4(1, 1, 1, 1), true, false);

    if (USE_PSX_16_BIT_SHADING) {
        out_color = psxR5G5B5BitCrush(out_color);
    }
}

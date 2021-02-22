#version 460

//----------------------------------------------------------------------------------------------------------------------
// World shader: fragment.
// This shader handles all in-game walls, floors, and sprites with or without light diminishing effects.
// All textures are also assumed to be 8 bits per pixel.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Frag.h"

layout(constant_id = 0) const bool WRAP_TEXTURE = true;             // Whether to use wrap texture mode or clamp to edge; clamping is used for sprites to prevent edge artifacts
layout(constant_id = 1) const bool USE_PSX_16_BIT_SHADING = true;   // Whether to shade in 16-bit mode like the original PlayStation

// A texture containing the entirety of PSX VRAM
layout(set = 0, binding = 0) uniform usampler2D vramTex;

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_uv_z;
layout(location = 2) flat in vec3 in_lightDimModeStrength;
layout(location = 3) flat in ivec2 in_texWinPos;
layout(location = 4) flat in ivec2 in_texWinSize;
layout(location = 5) flat in ivec2 in_clutPos;
layout(location = 6) flat in vec4 in_stmul;

layout(location = 0) out vec4 out_color;

//----------------------------------------------------------------------------------------------------------------------
// Compute the light diminishing multiplier for a pixel Z depth and strength vector for the different diminish styles.
// We use the strength vector here to avoid 'if()' branching when supporting the different light diminishing modes.
//----------------------------------------------------------------------------------------------------------------------
float getLightDiminishingMultiplier(float z, vec3 lightDimModeStrength) {
    // This is the light diminishing intensity when the effect is off (no change)
    float offDimIntensity = 128.0;

    // Compute the light diminishing intensity for floors
    float floorDimIntensity = 160.0 - z * 0.5;

    // Compute the light diminishing intensity for walls
    float wallDimintensity = ((128 * 65536) / z) / 256;

    // Compute the light diminishing intensity we will use
    float intensity = (
        offDimIntensity * lightDimModeStrength.x +
        wallDimintensity * lightDimModeStrength.y + 
        floorDimIntensity * lightDimModeStrength.z
    );

    // Clamp the intensity to the min/max allowed amounts (0.5x to 1.25x in normalized coords) and add a little bias to
    // fix precision issues and flipping back and forth between values when the calculations are close:
    intensity = trunc(clamp(intensity, 64, 160) + 0.0001);

    // Scale the diminish intensity back to normalized color coords rather than 0-128
    return intensity / 128.0;
}

//----------------------------------------------------------------------------------------------------------------------
// Shader entrypoint: do shading for a world/3d-view texel, with or without light diminishing
//----------------------------------------------------------------------------------------------------------------------
void main() {
    // Sample the raw texel first
    out_color = tex8bpp(vramTex, ivec2(floor(in_uv_z.xy)), in_texWinPos, in_texWinSize, in_clutPos, in_stmul, WRAP_TEXTURE, true);

    // Compute color multiply after accounting for input color and light diminishing effects.
    // Add a little bias also to prevent switching back and forth between cases that are close, due to float inprecision...
    vec3 colorMul = trunc(in_color * getLightDiminishingMultiplier(in_uv_z.z, in_lightDimModeStrength) + 0.0001) / 128.0;

    // The PSX renderer doesn't allow the color multiply to go larger than this:
    colorMul = min(colorMul, 255.0 / 128.0);

    // Apply the color multiply and bit crush the color in a manner similar to the PSX
    out_color.rgb *= colorMul;

    if (USE_PSX_16_BIT_SHADING) {
        out_color = psxR5G5B5BitCrush(out_color);
    }
}

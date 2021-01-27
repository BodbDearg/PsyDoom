//----------------------------------------------------------------------------------------------------------------------
// Push constants used by most shaders, presently just a model/view/projection matrix
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_UNIFORMS()\
    layout(push_constant) uniform Constants {   \
        mat4    mvpMatrix;                      \
    } constants;

//----------------------------------------------------------------------------------------------------------------------
// Declare the vertex inputs for the vertex type 'VVertex_Draw'.
// Depending on the shaders involved, some of these attributes may be ignored.
//
// Meaning:
//
//  in_pos              Vertex XYZ position.
//  in_color            RGB color for the vertex where 128.0 = fully white.
//                      Values over 128.0 are overbright.
//  in_lightDimMode     Light diminishing mode (VLightDimMode) for the 3D view shaders only. Ignored by UI shaders.
//  in_uv               UV texture coordinate (in pixels)
//  in_texWinPos        Top left XY position (in pixels) of the texture wrapping window
//  in_texWinSize       Top left XY position (in pixels) of the texture wrapping window
//  in_clutPos          XY position (in pixels) of the color lookup table for 4/8-bit textures that require it
//  in_stmul            RGBA multiplier for when the pixel is marked semi-transparent (controls blending)
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_VS_INPUTS_VVERTEX_DRAW()\
    layout(location = 0) in vec3    in_pos;             \
    layout(location = 1) in vec3    in_color;           \
    layout(location = 2) in uint    in_lightDimMode;    \
    layout(location = 3) in vec2    in_uv;              \
    layout(location = 4) in uvec2   in_texWinPos;       \
    layout(location = 5) in uvec2   in_texWinSize;      \
    layout(location = 6) in uvec2   in_clutPos;         \
    layout(location = 7) in vec4    in_stmul;

//----------------------------------------------------------------------------------------------------------------------
// Transforms the given UV coordinate by the texture window and wraps it to the texture window
//----------------------------------------------------------------------------------------------------------------------
vec2 transformTexWinUV(vec2 uv, uvec2 texWinPos, uvec2 texWinSize) {
    vec2 wrappedUv = mod(fract(uv / texWinSize) + 1.0, 1.0) * texWinSize;
    return wrappedUv + texWinPos;
}

//----------------------------------------------------------------------------------------------------------------------
// Directly read a 16-bit texel from PSX VRAM at 16 bits per pixel with nearest neighbor filtering.
// Applies semi-transparency modulations to the pixel if the PSX 'semi-transparent' flag is set.
//----------------------------------------------------------------------------------------------------------------------
vec4 getVramTexel(usampler2D vram, vec2 uv, vec4 stmul) {
    // If the texel bits are all zero then the pixel is transparent
    uint texelBits = textureLod(vram, uv, 0).r;

    if (texelBits == 0)
        return vec4(0, 0, 0, 0);

    // Unpack the color
    vec4 color = vec4(
        (texelBits >> 0 ) & 0x1F,
        (texelBits >> 5 ) & 0x1F,
        (texelBits >> 10) & 0x1F,
        31.0
    ) / 31.0;
    
    // Multiply by the semi-transparency multiply if the pixel is semi-transparent
    if ((texelBits & 0x8000) != 0) {
        color *= stmul;
    }

    return color;
}

//----------------------------------------------------------------------------------------------------------------------
// Sample a texture from PSX VRAM at 16 bits per pixel with nearest neighbor filtering.
//----------------------------------------------------------------------------------------------------------------------
vec4 tex16bpp(usampler2D vram, vec2 uv, uvec2 texWinPos, uvec2 texWinSize, vec4 stmul) {
    // Wrap the UV coordinate and transform by the texture window
    uv = transformTexWinUV(uv, texWinPos, texWinSize);

    // Lookup the texel from the CLUT and return it
    return getVramTexel(vram, uv, stmul);
}

//----------------------------------------------------------------------------------------------------------------------
// Sample a texture from PSX VRAM at 8 bits per pixel with nearest neighbor filtering.
//----------------------------------------------------------------------------------------------------------------------
vec4 tex8bpp(usampler2D vram, vec2 uv, uvec2 texWinPos, uvec2 texWinSize, uvec2 clutPos, vec4 stmul) {
    // Wrap the UV coordinate and transform by the texture window
    uv = transformTexWinUV(uv, texWinPos, texWinSize);

    // Figure out which 8-bits of the 16-bit pixel we will use for the CLUT index and get the CLUT index
    uint vramPixel = textureLod(vram, uv, 0).r;
    uint byteIdx = uint(uv.x * 2) & 1;
    uint clutIdx = vramPixel >> (byteIdx * 8);
    clutIdx &= 0xFF;

    // Lookup the texel from the CLUT and return it
    return getVramTexel(vram, vec2(clutPos.x + clutIdx, clutPos.y), stmul);
}

//----------------------------------------------------------------------------------------------------------------------
// Sample a texture from PSX VRAM at 4 bits per pixel with nearest neighbor filtering.
//----------------------------------------------------------------------------------------------------------------------
vec4 tex4bpp(usampler2D vram, vec2 uv, uvec2 texWinPos, uvec2 texWinSize, uvec2 clutPos, vec4 stmul) {
    // Wrap the UV coordinate and transform by the texture window
    uv = transformTexWinUV(uv, texWinPos, texWinSize);

    // Figure out which 4-bits of the 16-bit pixel we will use for the CLUT index and get the CLUT index
    uint vramPixel = textureLod(vram, uv, 0).r;
    uint nibbleIdx = uint(uv.x * 4) & 3;
    uint clutIdx = vramPixel >> (nibbleIdx * 4);
    clutIdx &= 0xF;

    // Lookup the texel from the CLUT and return it
    return getVramTexel(vram, vec2(clutPos.x + clutIdx, clutPos.y), stmul);
}

//----------------------------------------------------------------------------------------------------------------------
// Quantizes/truncates the given color to R5G5B5 in a way that mimics how the PS1 truncates color.
// Used to help reproduce the shading of the PlayStation.
//----------------------------------------------------------------------------------------------------------------------
vec4 psxR5G5B5BitCrush(vec4 color) {
    // Note: I added a slight amount here to prevent weird rounding issues on NV hardware.
    // This small bias prevents artifacts where pixels rapidly cycle between one 5-bit color component and the next.
    return trunc(color * 31 + 0.0001) / 31;
}

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

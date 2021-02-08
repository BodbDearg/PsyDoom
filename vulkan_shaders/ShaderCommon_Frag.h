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
    // Note: I added a slight amount here to prevent weird rounding/precision issues.
    // This small bias prevents artifacts where pixels rapidly cycle between one 5-bit color component and the next.
    return trunc(color * 31 + 0.0001) / 31;
}

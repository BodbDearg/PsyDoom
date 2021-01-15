//----------------------------------------------------------------------------------------------------------------------
// Push constants used by all the shaders, presently just a model/view/projection matrix
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_UNIFORMS()\
    layout(push_constant) uniform Constants {   \
        mat4    mvpMatrix;                      \
    } constants;

//----------------------------------------------------------------------------------------------------------------------
// Declare the vertex inputs fed into every vertex shader in PsyDoom.
// Depending on the shaders involved, some of these attributes may be ignored.
//
// Meaning:
//
//  in_pos_scale    Vertex XYZ position and a 'scale' (W component) which is only used by 3D view shaders.
//                  The 'scale' value is used for the light diminishing effect.
//  in_color        RGBA color for the vertex where 128.0 = fully white.
//                  Values over 128.0 are overbright.
//  in_uv           UV texture coordinate (in pixels)
//  in_texWinPos    Top left XY position (in pixels) of the texture wrapping window
//  in_texWinSize   Top left XY position (in pixels) of the texture wrapping window
//  in_clutPos      XY position (in pixels) of the color lookup table for 4/8-bit textures that require it
//  in_stmul        RGBA multiplier for when the pixel is marked semi-transparent (controls blending)
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_VERTEX_SHADER_INPUTS()\
    layout(location = 0) in vec4    in_pos_scale;   \
    layout(location = 1) in vec4    in_color;       \
    layout(location = 2) in vec2    in_uv;          \
    layout(location = 3) in uvec2   in_texWinPos;   \
    layout(location = 4) in uvec2   in_texWinSize;  \
    layout(location = 5) in uvec2   in_clutPos;     \
    layout(location = 6) in vec4    in_stmul;

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

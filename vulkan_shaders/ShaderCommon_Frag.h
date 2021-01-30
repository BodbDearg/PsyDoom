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

//----------------------------------------------------------------------------------------------------------------------
// Do shading for a world/3d-view texel, with or without light diminishing.
// The light diminishing on/off setting should be a constant, so should hopefully not branch once optimized.
//----------------------------------------------------------------------------------------------------------------------
vec4 shadeWorldTexel(
    bool bDoLightDiminish,
    usampler2D vramTex,
    vec3 in_color,
    vec3 in_uv_z,
    vec3 in_lightDimModeStrength,
    uvec2 in_texWinPos,
    uvec2 in_texWinSize,
    uvec2 in_clutPos,
    vec4 in_stmul
) {
    // Grab the basic texel
    vec4 out_color = tex8bpp(vramTex, in_uv_z.xy, in_texWinPos, in_texWinSize, in_clutPos, in_stmul);

    // Compute color multiply after accounting for input color and light diminishing effects.
    // Add a little bias also to prevent switching back and forth between cases that are close, due to float inprecision...
    vec3 colorMul = (bDoLightDiminish) ?
        trunc(in_color * getLightDiminishingMultiplier(in_uv_z.z, in_lightDimModeStrength) + 0.0001) / 128.0 :
        in_color / 128.0;

    // The PSX renderer doesn't allow the color multiply to go larger than this:
    colorMul = min(colorMul, 255.0 / 128.0);

    // Apply the color multiply and bit crush the color in a manner similar to the PSX
    out_color.rgb *= colorMul;
    out_color = psxR5G5B5BitCrush(out_color);
    return out_color;
}

//----------------------------------------------------------------------------------------------------------------------
// Occlusion tests for sprites and transparent or masked wall geometry.
// Tests the sort point against the texel read from input occlusion plane attachment.
// Returns whether fragment should be considered occluded by the plane.
//----------------------------------------------------------------------------------------------------------------------
bool isOccludedTexel(isubpassInputMS occPlaneTex, vec2 sortPt) {
    // Get the cocclusion plane angle and offset
    ivec2 occPlaneComponents = subpassLoad(occPlaneTex, gl_SampleID).xy;
    uint planeAngle = uint(occPlaneComponents.x);
    float planeOffset = float(occPlaneComponents.y);

    // Is it an occlusion plane at this texel?
    // Clear the 'is plane' flag also once we confirm that this texel does have an occlusion plane.
    if ((planeAngle & 0x8000) == 0)
        return false;

    planeAngle &= 0x7FFF;

    // Get the radian angle (0-359.9999 degrees) of the occlusion plane and then the normal vector
    float planeAngleRad = (float(planeAngle) / 32768.0) * 6.28318530718;
    vec2 planeNorm = vec2(cos(planeAngleRad), sin(planeAngleRad));

    // Get the distance of the point to the plane and judge whether occluded from that
    float planeDist = dot(planeNorm, sortPt);
    return (planeOffset < planeDist);
}

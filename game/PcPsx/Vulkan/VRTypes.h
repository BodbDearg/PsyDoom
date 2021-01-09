#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// This header contains some basic types used by the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Graphics pipeline type the Vulkan Renderer.
// Affects the primitive types expected, shaders used, blending mode and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class VRPipelineType : uint8_t {
    Lines,              // Solid colored lines
    UI_4bpp,            // Texture mapped @ 4bpp, alpha blended
    UI_8bpp,            // Texture mapped @ 8bpp, alpha blended
    UI_16bpp,           // Texture mapped @ 16bpp, alpha blended
    View_Alpha,         // 3D View: Texture mapped @ 8bpp, light diminishing, alpha blended
    View_Additive,      // 3D View: Texture mapped @ 8bpp, light diminishing, additive blended
    View_Subtractive,   // 3D View: Texture mapped @ 8bpp, light diminishing, additive blended
    NUM_TYPES           // Convenience declaration...
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Shader uniforms for the various shaders used by the Vulkan Renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct VRShaderUniforms {
    float       mvpMatrix[4][4];    // Modelview projection matrix: used to transform vertices in the vertex shader
    uint32_t    texWinX;            // Texture wrapping window: left x in pixels
    uint32_t    texWinY;            // Texture wrapping window: top y in pixels
    uint32_t    texWinW;            // Texture wrapping window: width in pixels
    uint32_t    texWinH;            // Texture wrapping window: height in pixels
    uint32_t    clutX;              // Texture coordinate of the CLUT in VRAM: x
    uint32_t    clutY;              // Texture coordinate of the CLUT in VRAM: y

    // When a pixel is flagged as 'semi-transparent', the RGBA color to multiply that pixel by.
    // Used to control blending when semi-transparency is active.
    float stMulR, stMulG, stMulB, stMulA;
};

// Make sure shader uniforms are not bigger than the minimum push constant range required by Vulkan, which is 128 bytes.
// We pass these as push constants because the data is very small.
static_assert(sizeof(VRShaderUniforms) <= 128);

//------------------------------------------------------------------------------------------------------------------------------------------
// Vertex type for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct VRVertex {
    float       x, y, z, s;     // XYZ Position and 'Scale' for light diminishing effects in-game (unused by UI shaders)
    float       r, g, b, a;     // Color for the vertex
    uint16_t    u, v;           // 2D Texture coordinates for the vertex
};

#endif  // #if PSYDOOM_VULKAN_RENDERER

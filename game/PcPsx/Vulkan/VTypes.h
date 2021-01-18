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
enum class VPipelineType : uint8_t {
    Lines,              // Solid colored lines
    UI_4bpp,            // Texture mapped @ 4bpp, alpha blended
    UI_8bpp,            // Texture mapped @ 8bpp, alpha blended
    UI_8bpp_Add,        // Texture mapped @ 8bpp, additive blended (used for player weapon when partial invisibility is active)
    UI_16bpp,           // Texture mapped @ 16bpp, alpha blended
    View_Alpha,         // 3D View: Texture mapped @ 8bpp, light diminishing, alpha blended
    View_Additive,      // 3D View: Texture mapped @ 8bpp, light diminishing, additive blended
    View_Subtractive,   // 3D View: Texture mapped @ 8bpp, light diminishing, additive blended
    NUM_TYPES           // Convenience declaration...
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Light diminishing mode for 3D view triangles
//------------------------------------------------------------------------------------------------------------------------------------------
enum class VLightDimMode : uint8_t {
    None,       // No light diminishing (used for things/sprites)
    Walls,      // Wall light diminishing mode: a bit brighter than the floor
    Flats       // Floor diminishing mode: darkens quicker than walls
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Shader uniforms for the various shaders used by the Vulkan Renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct VShaderUniforms {
    // Modelview projection matrix: used to transform vertices in the vertex shader
    float mvpMatrix[4][4];
};

// Make sure shader uniforms are not bigger than the minimum push constant range required by Vulkan, which is 128 bytes.
// We pass these as push constants because the data is very small.
static_assert(sizeof(VShaderUniforms) <= 128);

//------------------------------------------------------------------------------------------------------------------------------------------
// Vertex type for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex {
    // XYZ Position of the vertex
    float x, y, z;
    
    // Color for the vertex: rgb where '128' is regarded as 1.0
    uint8_t r, g, b;

    // Light diminishing mode for the 3D view: unused by UI shaders
    VLightDimMode lightDimMode;

    // 2D Texture coordinates for the vertex.
    // These coordinates are in terms of 16-bit pixels in VRAM.
    float u, v;
    
    // Texture wrapping window: x and y position
    uint16_t texWinX, texWinY;
    
    // Texture wrapping window: width and height
    uint16_t texWinW, texWinH;
    
    // CLUT location for 4-bit and 8-bit color modes: x and y
    uint16_t clutX, clutY;
    
    // When a pixel is flagged as 'semi-transparent', the RGBA color to multiply that pixel by.
    // Used to control blending when semi-transparency is active; a value of '128' is regarded as 1.0.
    // The 'alpha' semi transparency multiply component effectively is the alpha for the vertex.
    uint8_t stmulR, stmulG, stmulB, stmulA;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER

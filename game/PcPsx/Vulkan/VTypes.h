#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// This header contains some basic types used by the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "Matrix4.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Graphics pipeline type the Vulkan Renderer.
// Affects the primitive types expected, shaders used, blending mode and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class VPipelineType : uint8_t {
    Lines,                  // Solid colored lines, no blending: can be in either 2D or 3D (for debug use for example)
    Colored,                // Solid colored triangles, no blending: can be in either 2D or 3D (for debug use for example)
    UI_4bpp,                // 2D/UI: texture mapped @ 4bpp, masked but no blending
    UI_8bpp,                // 2D/UI: texture mapped @ 8bpp, masked but no blending
    UI_8bpp_Add,            // 2D/UI: texture mapped @ 8bpp, masked & additive blended (used for player weapon when partial invisibility is active)
    UI_16bpp,               // 2D/UI: texture mapped @ 16bpp, masked but no blending
    World_Masked,           // 3D world/view: textured @ 8bpp and lit, masked but no blending
    World_Alpha,            // 3D world/view: textured @ 8bpp and lit, masked & alpha blended
    World_Additive,         // 3D world/view: textured @ 8bpp and lit, masked & additive blended
    World_Subtractive,      // 3D world/view: textured @ 8bpp and lit, masked & subtractive blended
    World_Sky,              // 3D world/view: Used to draw the sky, masked but no blending
    Msaa_Resolve,           // Simple shader that resolves MSAA samples
    Crossfade,              // Used for doing crossfades
    NUM_TYPES               // Convenience declaration...
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
// Shader uniforms for the various shaders in the main 'draw' subpass
//------------------------------------------------------------------------------------------------------------------------------------------
struct VShaderUniforms_Draw {
    // Modelview projection matrix: used to transform vertices in the vertex shader
    Matrix4f mvpMatrix;

    // Coordinate system conversion: scalars to convert from normalized device coords to the original PSX framebuffer coords (256x240).
    // Also where the where the original PSX framebuffer coords start in NDC space.
    float ndcToPsxScaleX, ndcToPsxScaleY;
    float psxNdcOffsetX, psxNdcOffsetY;
};

// Make sure shader uniforms are not bigger than the minimum push constant range required by the Vulkan standard, which is 128 bytes.
// We pass these as push constants because the data is very small and for a lot of GPUs push constants are more optimal.
static_assert(sizeof(VShaderUniforms_Draw) <= 128);

//------------------------------------------------------------------------------------------------------------------------------------------
// Shader uniforms for crossfading: contains just a single linear interpolation factor to control blending between the framebuffers
//------------------------------------------------------------------------------------------------------------------------------------------
struct VShaderUniforms_Crossfade {
    float lerpAmount;
};

static_assert(sizeof(VShaderUniforms_Crossfade) <= 128);    // Same restrictions apply as with 'VShaderUniforms_Draw' - see above...

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer vertex type: used for all direct drawing operations
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_Draw {
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer vertex type: used for MSAA resolve and just contains a 2D position in normalized device coords
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_MsaaResolve {
    float x, y;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer vertex type: used for crossfading.
// Contains a 2D position in normalized device coords, as well as UV coordinates to use for sampling.
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_Crossfade {
    float x, y;
    float u, v;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER

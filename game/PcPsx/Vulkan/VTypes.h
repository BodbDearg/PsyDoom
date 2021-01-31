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
    Lines,                      // Solid colored lines, alpha blended: can be in either 2D or 3D (for debug use for example)
    UI_4bpp,                    // 2D/UI: texture mapped @ 4bpp, alpha blended
    UI_8bpp,                    // 2D/UI: texture mapped @ 8bpp, alpha blended
    UI_8bpp_Add,                // 2D/UI: texture mapped @ 8bpp, additive blended (used for player weapon when partial invisibility is active)
    UI_16bpp,                   // 2D/UI: texture mapped @ 16bpp, alpha blended
    World_Depth,                // 3D world/view: draw depth for occluding planes which mask sprites and other geometry
    World_SolidGeom,            // 3D world/view: Texture mapped @ 8bpp, light diminishing, no blending - for fully solid/opaque geometry.
    World_SolidGeomXray,        // 3D world/view: Texture mapped @ 8bpp, light diminishing, no blending - for fully solid/opaque geometry (X-Ray vision cheat).
    World_AlphaGeom,            // 3D world/view: Texture mapped @ 8bpp, light diminishing, alpha blended - for masked and translucent walls.
    World_AlphaSprite,          // 3D world/view: Texture mapped @ 8bpp, light diminishing, alpha blended - for sprites.
    World_AdditiveSprite,       // 3D world/view: Texture mapped @ 8bpp, light diminishing, additive blended - for sprites.
    World_SubtractiveSprite,    // 3D world/view: Texture mapped @ 8bpp, light diminishing, subtractive blended - for sprites.
    World_Sky,                  // 3D world/view: Used to draw the sky in areas of the screen which have been carved out with alpha '0'
    Msaa_Resolve,               // Simple shader that resolves MSAA samples
    NUM_TYPES                   // Convenience declaration...
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
// Vulkan renderer vertex type: used to do a depth render pass and contains just a 3D position in world coordinates.
// The output of the depth pass is used to help clip sprites and geometry.
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_Depth {
    float x, y, z;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer vertex type: used for MSAA resolve and just contains a 2D position in normalized device coords
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_MsaaResolve {
    float x, y;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER

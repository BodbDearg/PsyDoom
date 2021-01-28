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
    Lines,              // Solid colored lines, alpha blended: can be in either 2D or 3D (for debug use for example)
    UI_4bpp,            // 2D/UI: texture mapped @ 4bpp, alpha blended
    UI_8bpp,            // 2D/UI: texture mapped @ 8bpp, alpha blended
    UI_8bpp_Add,        // 2D/UI: texture mapped @ 8bpp, additive blended (used for player weapon when partial invisibility is active)
    UI_16bpp,           // 2D/UI: texture mapped @ 16bpp, alpha blended
    World_OccPlane,     // 3D world/view: draw information for occluding planes which occlude sprites and masked/translucent walls
    World_Alpha,        // 3D world/view: Texture mapped @ 8bpp, light diminishing, alpha blended
    World_Additive,     // 3D world/view: Texture mapped @ 8bpp, light diminishing, additive blended
    World_Subtractive,  // 3D world/view: Texture mapped @ 8bpp, light diminishing, additive blended
    Msaa_Resolve,       // Simple shader that resolves MSAA samples
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
// Vulkan renderer vertex type: used to render occluding planes for walls.
// These occluding planes are used to mask sprites as well as translucent or masked walls.
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_OccPlane {
    static constexpr uint16_t ANGLE_MASK    = 0x7FFF;   // Mask to extract the actual 15 angle bits
    static constexpr uint16_t ANGLE_BITS    = 15;       // How many bits to encode the angle with
    static constexpr uint16_t IS_PLANE_BIT  = 0x8000;   // A bit which is set to mark the texel as containing an occluding plane, packed into the 'angle' field

    // XYZ Position of the vertex
    float x, y, z;

    // A 15-bit binary angle for the wall plane packed into the low 15-bits.
    // The highest bit is set if the texel actually contains an occluding plane, so we can handle the case of no visible planes.
    // Note: this value is really unsigned, but I used signed for shader packing reasons.
    int16_t planeAngle;

    // The integer offset of the occluding plane from the origin.
    // Higher than integer precision would probably be ideal, but this saves a lot of memory and is good enough for most cases.
    int16_t planeOffset;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer vertex type: used for MSAA resolve and just contains a 2D position in normalized device coords
//------------------------------------------------------------------------------------------------------------------------------------------
struct VVertex_MsaaResolve {
    float x, y;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER

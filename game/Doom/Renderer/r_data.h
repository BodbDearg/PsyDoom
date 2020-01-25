#pragma once

#include "PsxVm/VmPtr.h"

struct RECT;

// Constant for an invalid or undefined (not yet set) upload frame number for a texture
static constexpr uint32_t TEX_INVALID_UPLOAD_FRAME_NUM = UINT32_MAX;

// Stores information about a texture, including it's dimensions, lump info and texture cache info
struct texture_t {
    int16_t                     offsetX;
    int16_t                     offsetY;
    int16_t                     width;                  // TODO: is this signed or unsigned?
    int16_t                     height;                 // TODO: is this signed or unsigned?
    uint8_t                     texPageCoordX;          // TODO: COMMENT
    uint8_t                     texPageCoordY;          // TODO: COMMENT
    uint16_t                    texPageId;              // TODO: COMMENT
    uint16_t                    widthIn16Blocks;        // Width in 16 pixel increments (rounded up)
    uint16_t                    heightIn16Blocks;       // Height in 16 pixel increments (rounded up)
    uint16_t                    lumpNum;
    uint16_t                    __padding;              // TODO: is this actually used?
    VmPtr<VmPtr<texture_t>>     ppTexCacheEntry;        // The texture cache entry for this texture
    uint32_t                    unknown1;               // TODO: what is this?
    uint32_t                    uploadFrameNum;         // What frame the texture was added to the texture cache, used to detect texture cache overflows
};

static_assert(sizeof(texture_t) == 32);

// An entry in the 'LIGHTS' lump.
// Simply holds the color of a light to apply to a sector.
struct light_t {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     _pad;    // Does not appear to be used, always '0'
};

extern const VmPtr<VmPtr<texture_t>>    gpTextures;
extern const VmPtr<VmPtr<texture_t>>    gpSkyTexture;
extern const VmPtr<VmPtr<int32_t>>      gpTextureTranslation;
extern const VmPtr<VmPtr<light_t>>      gpLightsLump;
extern const VmPtr<uint16_t>            gPaletteClutId_Main;
extern const VmPtr<uint16_t>            g3dViewPaletteClutId;

void R_InitData() noexcept;
void R_InitTextures() noexcept;
void R_InitFlats() noexcept;
void R_InitSprites() noexcept;
void R_TextureNumForName() noexcept;
void R_FlatNumForName() noexcept;
void R_InitPalette() noexcept;

// PC-PSX: helper to reduce some redundancy
RECT getTextureVramRect(const texture_t& tex) noexcept;

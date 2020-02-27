#pragma once

#include "PsxVm/VmPtr.h"

struct RECT;

// Constant for an invalid or undefined (not yet set) upload frame number for a texture
static constexpr uint32_t TEX_INVALID_UPLOAD_FRAME_NUM = UINT32_MAX;

// The number of palettes available in the game
static constexpr uint32_t NUMPALETTES = 20;

// Palette indices and ranges
static constexpr uint32_t MAINPAL               = 0;    // Used for most sprites and textures in the game
static constexpr uint32_t STARTREDPALS          = 1;    // Pain palettes (red shift) start
static constexpr uint32_t NUMREDPALS            = 8;    // Number of pain palettes
static constexpr uint32_t STARTBONUSPALS        = 9;    // Bonus pickup (gold shift) palettes start
static constexpr uint32_t NUMBONUSPALS          = 4;    // Number of bonus pickup palettes
static constexpr uint32_t RADIATIONPAL          = 13;   // Radiation suit green shift
static constexpr uint32_t INVULNERABILITYPAL    = 14;   // PSX Doom: invulernability effect
static constexpr uint32_t FIRESKYPAL            = 15;   // PSX Doom: fire sky palette
static constexpr uint32_t UIPAL                 = 16;   // PSX Doom: ui elements palette
static constexpr uint32_t TITLEPAL              = 17;   // PSX Doom: title screen palette
static constexpr uint32_t IDCREDITS1PAL         = 18;   // PSX Doom: id credits screen palette
static constexpr uint32_t WCREDITS1PAL          = 19;   // PSX Doom: williams credits screen palette

// Stores information about a texture, including it's dimensions, lump info and texture cache info
struct texture_t {
    int16_t                     offsetX;
    int16_t                     offsetY;
    int16_t                     width;                  // TODO: is this signed or unsigned?
    int16_t                     height;                 // TODO: is this signed or unsigned?
    uint8_t                     texPageCoordX;          // TODO: COMMENT
    uint8_t                     texPageCoordY;          // TODO: COMMENT
    uint16_t                    texPageId;              // Hardware specific field corresonding to the texture page in VRAM where the texture is held. '0' when not resident in VRAM.
    uint16_t                    width16;                // Width of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.
    uint16_t                    height16;               // Height of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.
    uint16_t                    lumpNum;
    uint16_t                    _pad1;                  // Unused
    VmPtr<VmPtr<texture_t>>     ppTexCacheEntries;      // Points to the top left cell in the texture cache where this texture is placed.
    uint32_t                    _pad2;                  // Unused
    uint32_t                    uploadFrameNum;         // What frame the texture was added to the texture cache, used to detect texture cache overflows
};

static_assert(sizeof(texture_t) == 32);

// Stores info about the size of a texture in WAD lump
struct patch_t {
    int16_t     offsetX;
    int16_t     offsetY;
    int16_t     width;
    int16_t     height;
};

static_assert(sizeof(patch_t) == 8);

// An entry in the 'LIGHTS' lump.
// Simply holds the color of a light to apply to a sector.
struct light_t {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     _pad;    // Does not appear to be used, always '0'
};

static_assert(sizeof(light_t) == 4);

extern const VmPtr<VmPtr<texture_t>>        gpTextures;
extern const VmPtr<VmPtr<texture_t>>        gpFlatTextures;
extern const VmPtr<VmPtr<texture_t>>        gpSpriteTextures;
extern const VmPtr<VmPtr<texture_t>>        gpSkyTexture;
extern const VmPtr<VmPtr<int32_t>>          gpTextureTranslation;
extern const VmPtr<VmPtr<int32_t>>          gpFlatTranslation;
extern const VmPtr<VmPtr<light_t>>          gpLightsLump;
extern const VmPtr<uint16_t[NUMPALETTES]>   gPaletteClutIds;
extern const VmPtr<uint16_t>                g3dViewPaletteClutId;
extern const VmPtr<int32_t>                 gFirstTexLumpNum;
extern const VmPtr<int32_t>                 gLastTexLumpNum;
extern const VmPtr<int32_t>                 gNumTexLumps;
extern const VmPtr<int32_t>                 gFirstFlatLumpNum;
extern const VmPtr<int32_t>                 gLastFlatLumpNum;
extern const VmPtr<int32_t>                 gNumFlatLumps;
extern const VmPtr<int32_t>                 gFirstSpriteLumpNum;
extern const VmPtr<int32_t>                 gLastSpriteLumpNum;
extern const VmPtr<int32_t>                 gNumSpriteLumps;

void R_InitData() noexcept;
void R_InitTextures() noexcept;
void R_InitFlats() noexcept;
void R_InitSprites() noexcept;
int32_t R_TextureNumForName(const char* const name) noexcept;
int32_t R_FlatNumForName(const char* const name) noexcept;
void R_InitPalette() noexcept;

// PC-PSX: helper to reduce some redundancy
RECT getTextureVramRect(const texture_t& tex) noexcept;

#pragma once

#include <cstdint>
#include <memory>

struct SRECT;

// Constant for an invalid or undefined (not yet set) upload frame number for a texture
static constexpr uint32_t TEX_INVALID_UPLOAD_FRAME_NUM = UINT32_MAX;

// The number of palettes available in the game for Doom and Final Doom and maximum the engine supports
static constexpr uint32_t NUMPALETTES_DOOM          = 20;
static constexpr uint32_t NUMPALETTES_FINAL_DOOM    = 26;

#if PSYDOOM_MODS
    static constexpr uint32_t MAXPALETTES = 32;     // PsyDoom: allow an additional 6 user palettes (there is room in VRAM)
#else
    static constexpr uint32_t MAXPALETTES = NUMPALETTES_FINAL_DOOM;
#endif

// Palette indices and ranges
static constexpr uint32_t MAINPAL               = 0;    // Used for most sprites and textures in the game
static constexpr uint32_t STARTREDPALS          = 1;    // Pain palettes (red shift) start
static constexpr uint32_t NUMREDPALS            = 8;    // Number of pain palettes
static constexpr uint32_t STARTBONUSPALS        = 9;    // Bonus pickup (gold shift) palettes start
static constexpr uint32_t NUMBONUSPALS          = 4;    // Number of bonus pickup palettes
static constexpr uint32_t RADIATIONPAL          = 13;   // Radiation suit green shift
static constexpr uint32_t INVULNERABILITYPAL    = 14;   // PSX Doom: invulnerability effect
static constexpr uint32_t FIRESKYPAL            = 15;   // PSX Doom: fire sky palette
static constexpr uint32_t UIPAL                 = 16;   // PSX Doom: ui elements palette
static constexpr uint32_t TITLEPAL              = 17;   // PSX Doom: title screen palette
static constexpr uint32_t IDCREDITS1PAL         = 18;   // PSX Doom: ID credits screen palette
static constexpr uint32_t WCREDITS1PAL          = 19;   // PSX Doom: Williams credits screen palette
static constexpr uint32_t UIPAL2                = 20;   // PSX Final Doom: additional UI palette (used for plaques etc.)
static constexpr uint32_t SKYPAL1               = 21;   // PSX Final Doom: additional sky palette
static constexpr uint32_t SKYPAL2               = 22;   // PSX Final Doom: additional sky palette
static constexpr uint32_t SKYPAL3               = 23;   // PSX Final Doom: additional sky palette
static constexpr uint32_t SKYPAL4               = 24;   // PSX Final Doom: additional sky palette
static constexpr uint32_t SKYPAL5               = 25;   // PSX Final Doom: additional sky palette

// Stores information about a texture, including it's dimensions, lump info and texture cache info
struct texture_t {
    int16_t     offsetX;    // Used for anchoring/offsetting in some UIs
    int16_t     offsetY;    // Used for anchoring/offsetting in some UIs
    int16_t     width;      // Pixel width of texture
    int16_t     height;     // Pixel height of texture

    // PsyDoom: use 16-bit uvs if limit removing; allows for textures larger than 256x256.
    #if PSYDOOM_LIMIT_REMOVING
        uint16_t    texPageCoordX;      // Texture coordinate (X/U) inside the current texture page
        uint16_t    texPageCoordY;      // Texture coordinate (Y/V) inside the current texture page
    #else                               
        uint8_t     texPageCoordX;      // Texture coordinate (X/U) inside the current texture page
        uint8_t     texPageCoordY;      // Texture coordinate (Y/V) inside the current texture page
    #endif

    // Hardware specific field corresponding to the texture page in VRAM where the texture is located.
    // Originally this was '0' when the texture was not resident in VRAM but now 'bIsCached' is used for that purpose and '0' now means the 1st texture page.
    uint16_t texPageId;

    // PsyDoom: adding the 'evictable' flag needed by the new texture cache management code
    #if PSYDOOM_MODS
        uint8_t     bIsCached;      // If 'true' then the texture is loaded into the texture cache
        uint8_t     width16;        // Width of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.
        uint8_t     height16;       // Height of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.

        // Individual texture locking is a limit removing feature.
        // If 'false' then the texture can be evicted from the texture cache.
        #if PSYDOOM_LIMIT_REMOVING
            uint8_t bIsLocked;
        #endif
    #else
        uint16_t    width16;        // Width of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.
        uint16_t    height16;       // Height of the texture in 16 pixel units (rounded up). Base unit for a texture cache cell.
    #endif

    uint16_t lumpNum;   // Which WAD lump this texture was loaded from

    // PsyDoom: struct layout improvements
    #if PSYDOOM_MODS
        uint32_t        uploadFrameNum;         // What frame the texture was added to the texture cache, used to detect texture cache overflows
        texture_t**     ppTexCacheEntries;      // Points to the top left cell in the texture cache where this texture is placed.
    #else
        uint16_t        _pad1;                  // Unused
        texture_t**     ppTexCacheEntries;      // Points to the top left cell in the texture cache where this texture is placed.
        uint32_t        _pad2;                  // Unused
        uint32_t        uploadFrameNum;         // What frame the texture was added to the texture cache, used to detect texture cache overflows
    #endif
};

// Stores info about the size and anchor point (offsetting) for a texture in WAD lump
struct patch_t {
    int16_t     offsetX;
    int16_t     offsetY;
    int16_t     width;
    int16_t     height;
};

// An entry in the 'LIGHTS' lump.
// Simply holds the color of a light to apply to a sector.
struct light_t {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     _pad;    // Does not appear to be used, always '0'
};

extern texture_t*   gpTextures;
extern texture_t*   gpFlatTextures;
extern texture_t*   gpSpriteTextures;
extern texture_t*   gpSkyTexture;
extern int32_t*     gpTextureTranslation;
extern int32_t*     gpFlatTranslation;
extern light_t*     gpLightsLump;
extern uint16_t     gPaletteClutIds[MAXPALETTES];
extern uint16_t     g3dViewPaletteClutId;
extern int32_t      gNumTexLumps;
extern int32_t      gNumFlatLumps;
extern int32_t      gNumSpriteLumps;

// PsyDoom: texture, flat and sprite lists might no longer be a contiguous set of lumps
#if !PSYDOOM_MODS
    extern int32_t  gFirstTexLumpNum;
    extern int32_t  gLastTexLumpNum;
    extern int32_t  gFirstFlatLumpNum;
    extern int32_t  gLastFlatLumpNum;
    extern int32_t  gFirstSpriteLumpNum;
    extern int32_t  gLastSpriteLumpNum;
#endif

void R_InitData() noexcept;

#if PSYDOOM_MODS
    int32_t R_TextureNumForName(const char* const name, const bool bMustExist = false) noexcept;
    int32_t R_FlatNumForName(const char* const name, const bool bMustExist = false) noexcept;
    texture_t& R_GetTexForLump(const int32_t lumpIdx) noexcept;
#else
    int32_t R_TextureNumForName(const char* const name) noexcept;
    int32_t R_FlatNumForName(const char* const name) noexcept;
#endif

// PsyDoom: helper to reduce some redundancy
SRECT getTextureVramRect(const texture_t& tex) noexcept;

#if PSYDOOM_MODS
    void R_UpdateTexMetricsFromData(texture_t& tex, const void* const pTexData, const int32_t texDataSize) noexcept;
#endif

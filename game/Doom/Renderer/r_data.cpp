#include "r_data.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/doomdata.h"
#include "PsyQ/LIBGPU.h"

#include <cstring>

// Structure for a palette in the game: contains 256 RGBA5551 color values.
struct palette_t {
    uint16_t colors[256];
};

// Details about all of the textures in the game and the sky texture
texture_t*  gpTextures;
texture_t*  gpFlatTextures;
texture_t*  gpSpriteTextures;
texture_t*  gpSkyTexture;

// Texture translation: converts from an input texture index to the actual texture index to render for that input index.
// Used to implement animated textures whereby the translation is simply updated as the texture animates.
int32_t*    gpTextureTranslation;

// Flat translation: similar function to texture translation except for flats rather than wall textures.
int32_t*    gpFlatTranslation;

// The loaded lights lump
light_t*    gpLightsLump;

// Palette stuff
uint16_t    gPaletteClutIds[MAXPALETTES];       // CLUT ids for all of the game's palettes. These are all held in VRAM.
uint16_t    g3dViewPaletteClutId;               // Currently active in-game palette. Changes as effects are applied in the game.

// Lump number ranges
int32_t     gFirstTexLumpNum;       // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gLastTexLumpNum;        // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gNumTexLumps;
int32_t     gFirstFlatLumpNum;      // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gLastFlatLumpNum;       // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gNumFlatLumps;
int32_t     gFirstSpriteLumpNum;    // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gLastSpriteLumpNum;     // TODO: remove first and last lump number once texture lists from multiple WADs are supported
int32_t     gNumSpriteLumps;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the palette and asset management for various draw assets
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitData() noexcept {
    R_InitPalette();
    R_InitTextures();
    R_InitFlats();
    R_InitSprites();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global wall textures list and load texture size metadata.
// Also initialize the texture translation table for animated wall textures.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitTextures() noexcept {
    // Determine basic texture list stats.
    // TODO: update this to support texture lists in multiple WAD files.
    gFirstTexLumpNum = W_GetNumForName("T_START") + 1;
    gLastTexLumpNum = W_GetNumForName("T_END") - 1;
    gNumTexLumps = gLastTexLumpNum - gFirstTexLumpNum + 1;

    // Alloc the list of textures and the texture translation table
    {
        std::byte* const pAlloc = (std::byte*) Z_Malloc(
            *gpMainMemZone,
            gNumTexLumps * (sizeof(texture_t) + sizeof(int32_t)),
            PU_STATIC,
            nullptr
        );

        gpTextures = (texture_t*) pAlloc;
        gpTextureTranslation = (int32_t*)(pAlloc + gNumTexLumps * sizeof(texture_t));
    }

    // Load the texture metadata lump and process each associated texture entry with the loaded size info.
    // PsyDoom: added updates here to work with the new WAD management code.
    {
        #if PSYDOOM_MODS
            const WadLump& texture1Lump = W_CacheLumpName("TEXTURE1", PU_CACHE, true);
            maptexture_t* const pMapTextures = (maptexture_t*) texture1Lump.pCachedData;
        #else
            maptexture_t* const pMapTextures = (maptexture_t*) W_CacheLumpName("TEXTURE1", PU_CACHE, true);
        #endif

        maptexture_t* pMapTex = pMapTextures;
        texture_t* pTex = gpTextures;

        for (int32_t lumpNum = gFirstTexLumpNum; lumpNum <= gLastTexLumpNum; ++lumpNum, ++pMapTex, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;

            #if PSYDOOM_MODS
                pTex->bIsCached = false;

                #if PSYDOOM_LIMIT_REMOVING
                    pTex->bIsLocked = false;
                #endif
            #endif

            pTex->texPageId = 0;
            pTex->width = Endian::littleToHost(pMapTex->width);
            pTex->height = Endian::littleToHost(pMapTex->height);

            if (pTex->width + 15 >= 0) {
                pTex->width16 = (uint8_t)((pTex->width + 15) / 16);
            } else {
                pTex->width16 = (uint8_t)((pTex->width + 30) / 16);     // This case is never hit. Not sure why texture sizes would be negative? What does that mean?
            }

            if (pTex->height + 15 >= 0) {
                pTex->height16 = (uint8_t)((pTex->height + 15) / 16);
            } else {
                pTex->height16 = (uint8_t)((pTex->height + 30) / 16);   // This case is never hit. Not sure why texture sizes would be negative? What does that mean?
            }
        }

        // Cleanup this after we are done
        Z_Free2(*gpMainMemZone, pMapTextures);
    }

    // Init the texture translation table: initially all textures translate to themselves
    int32_t* const pTexTranslation = gpTextureTranslation;

    for (int32_t texIdx = 0; texIdx < gNumTexLumps; ++texIdx) {
        pTexTranslation[texIdx] = texIdx;
    }

    // Clear out any blocks marked as 'cache' which can be evicted.
    // This call is here probably to try and avoid spikes in memory load.
    Z_FreeTags(*gpMainMemZone, PU_CACHE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global flat textures list.
// Also initialize the flat texture translation table used for animation.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitFlats() noexcept {
    // Determine basic flat texture list stats.
    // TODO: update this to support texture lists in multiple WAD files.
    gFirstFlatLumpNum = W_GetNumForName("F_START") + 1;
    gLastFlatLumpNum = W_GetNumForName("F_END") - 1;
    gNumFlatLumps = gLastFlatLumpNum - gFirstFlatLumpNum + 1;

    // Alloc the list of flat textures and the flat texture translation table
    {
        std::byte* const pAlloc = (std::byte*) Z_Malloc(
            *gpMainMemZone,
            gNumFlatLumps * (sizeof(texture_t) + sizeof(int32_t)),
            PU_STATIC,
            nullptr
        );

        gpFlatTextures = (texture_t*) pAlloc;
        gpFlatTranslation = (int32_t*)(pAlloc + gNumFlatLumps * sizeof(texture_t));
    }

    // The hardcoded assumed/size for flats.
    // Note: PsyDoom now allows flats to be differently sized to this, it checks the actual texture size upon loading.
    constexpr uint32_t FLAT_TEX_SIZE = 64;

    // Setup the texture structs for each flat
    {
        texture_t* pTex = gpFlatTextures;

        for (int32_t lumpNum = gFirstFlatLumpNum; lumpNum <= gLastFlatLumpNum; ++lumpNum, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;
            pTex->texPageId = 0;

            #if PSYDOOM_MODS
                pTex->bIsCached = false;

                #if PSYDOOM_LIMIT_REMOVING
                    pTex->bIsLocked = false;
                #endif
            #endif

            pTex->width = FLAT_TEX_SIZE;
            pTex->height = FLAT_TEX_SIZE;
            pTex->width16 = FLAT_TEX_SIZE / 16;
            pTex->height16 = FLAT_TEX_SIZE / 16;
        }
    }

    // Init the flat translation table: initially all flats translate to themselves
    int32_t* const pFlatTranslation = gpFlatTranslation;

    for (int32_t texIdx = 0; texIdx < gNumFlatLumps; ++texIdx) {
        pFlatTranslation[texIdx] = texIdx;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global sprite textures list and load sprite size and offset metadata
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitSprites() noexcept {
    // Determine basic sprite texture list stats
    gFirstSpriteLumpNum = W_GetNumForName("S_START") + 1;
    gLastSpriteLumpNum = W_GetNumForName("S_END") - 1;
    gNumSpriteLumps = gLastSpriteLumpNum - gFirstSpriteLumpNum + 1;

    // Alloc the list of sprite textures
    gpSpriteTextures = (texture_t*) Z_Malloc(*gpMainMemZone, gNumSpriteLumps * sizeof(texture_t), PU_STATIC, nullptr);

    // Load the sprite metadata lump and process each associated sprite texture entry.
    // The metadata gives us the size and offsetting for each sprite.
    // PsyDoom: added updates here to work with the new WAD management code.
    #if PSYDOOM_MODS
        const WadLump& sprite1Lump = W_CacheLumpName("SPRITE1", PU_CACHE, true);
        maptexture_t* const pMapSpriteTextures = (maptexture_t*) sprite1Lump.pCachedData;
    #else
        maptexture_t* const pMapSpriteTextures = (maptexture_t*) W_CacheLumpName("SPRITE1", PU_CACHE, true);
    #endif

    maptexture_t* pMapTex = pMapSpriteTextures;
    texture_t* pTex = gpSpriteTextures;

    for (int32_t lumpNum = gFirstSpriteLumpNum; lumpNum <= gLastSpriteLumpNum; ++lumpNum, ++pTex, ++pMapTex) {
        pTex->lumpNum = (uint16_t) lumpNum;
        pTex->texPageId = 0;
        pTex->offsetX = Endian::littleToHost(pMapTex->offsetX);
        pTex->offsetY = Endian::littleToHost(pMapTex->offsetY);
        pTex->width = Endian::littleToHost(pMapTex->width);
        pTex->height = Endian::littleToHost(pMapTex->height);

        if (pTex->width + 15 >= 0) {
            pTex->width16 = (uint8_t)((pTex->width + 15) / 16);
        } else {
            pTex->width16 = (uint8_t)((pTex->width + 30) / 16);     // This case is never hit. Not sure why sprite sizes would be negative? What does that mean?
        }

        if (pTex->height + 15 >= 0) {
            pTex->height16 = (uint8_t)((pTex->height + 15) / 16);
        } else {
            pTex->height16 = (uint8_t)((pTex->height + 30) / 16);   // This case is never hit. Not sure why sprite sizes would be negative? What does that mean?
        }
    }

    // Cleanup this after we are done and clear out any blocks marked as 'cache' - those can be evicted.
    // This call is here probably to try and avoid spikes in memory load.
    Z_Free2(*gpMainMemZone, pMapSpriteTextures);
    Z_FreeTags(*gpMainMemZone, PU_CACHE);
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Given a lump name (case insensitive) for a wall texture, returns the texture index among wall texture lumps.
// Returns '-1' if the name was not found.
// PsyDoom: this function has been re-written, see the 'Old' code folder for the original version.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_TextureNumForName(const char* const name) noexcept {
    const WadLumpName searchLumpName = WadUtils::makeUppercaseLumpName(name);
    
    const texture_t* const pTextures = gpTextures;
    const int32_t numTextures = gNumTexLumps;

    for (int32_t texIdx = 0; texIdx < numTextures; ++texIdx) {
        const int32_t texLumpIdx = pTextures[texIdx].lumpNum;
        const WadLumpName texLumpName = W_GetLumpName(texLumpIdx);

        if ((texLumpName.word & WAD_LUMPNAME_MASK) == searchLumpName.word)
            return texIdx;
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Given a lump name (case insensitive) for a flat texture, returns the texture index among flat texture lumps.
// PsyDoom: this function has been re-written, see the 'Old' code folder for the original version.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_FlatNumForName(const char* const name) noexcept {
    const WadLumpName searchLumpName = WadUtils::makeUppercaseLumpName(name);

    const texture_t* const pFlatTextures = gpFlatTextures;
    const int32_t numFlatTextures = gNumFlatLumps;

    for (int32_t flatIdx = 0; flatIdx < numFlatTextures; ++flatIdx) {
        const int32_t flatLumpIdx = pFlatTextures[flatIdx].lumpNum;
        const WadLumpName flatLumpName = W_GetLumpName(flatLumpIdx);

        if ((flatLumpName.word & WAD_LUMPNAME_MASK) == searchLumpName.word)
            return flatIdx;
    }

    return -1;
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads all of the game's palettes into VRAM.
// Also loads the 'LIGHTS' lump which gives the color multipliers for various sector colors.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitPalette() noexcept {
    // Load the colored light multipliers lump and force the first entry to be fullbright.
    // PsyDoom: added updates here to work with the new WAD management code.
    #if PSYDOOM_MODS
        const WadLump& lightsWadLump = W_CacheLumpName("LIGHTS", PU_STATIC, true);
        gpLightsLump = (light_t*) lightsWadLump.pCachedData;
    #else
        gpLightsLump = (light_t*) W_CacheLumpName("LIGHTS", PU_STATIC, true);
    #endif

    light_t& firstLight = *gpLightsLump;
    firstLight.r = 255;
    firstLight.g = 255;
    firstLight.b = 255;

    // Load the palettes lump and sanity check its size; accept either the Doom or Final Doom palette count.
    // PsyDoom: added updates here to work with the new WAD management code.
    const int32_t playpalLumpNum = W_GetNumForName("PLAYPAL");

    #if PSYDOOM_MODS
        const WadLump& playPalWadLump = W_CacheLumpNum(playpalLumpNum, PU_CACHE, true);
        const palette_t* const pGamePalettes = (const palette_t*) playPalWadLump.pCachedData;
    #else
        const palette_t* const pGamePalettes = (const palette_t*) W_CacheLumpNum(playpalLumpNum, PU_CACHE, true);
    #endif

    const int32_t numPalettes = W_LumpLength(playpalLumpNum) / sizeof(palette_t);

    if ((numPalettes != NUMPALETTES_DOOM) && (numPalettes != NUMPALETTES_FINAL_DOOM)) {
        I_Error("R_InitPalettes: palette foulup\n");
    }

    // PsyDoom: zero init the palettes list so nothing is undefined if we don't load some (have less palettes for Doom)
    #if PSYDOOM_MODS
        std::memset(gPaletteClutIds, 0, sizeof(gPaletteClutIds));
    #endif

    // Upload all the palettes into VRAM
    {
        SRECT dstVramRect = {};
        dstVramRect.w = 256;
        dstVramRect.h = 1;

        const palette_t* pPalette = pGamePalettes;
        uint16_t* pClutId = gPaletteClutIds;

        for (int32_t palIdx = 0; palIdx < numPalettes; ++palIdx, ++pPalette, ++pClutId) {
            // How many palettes we can squeeze onto a VRAM texture page that also has a framebuffer.
            // The palettes for the game are packed into some of the unused rows of the framebuffer.
            constexpr int32_t PAL_ROWS_PER_TPAGE = 256 - SCREEN_H;

            // Set the destination location in VRAM for the palette.
            //
            // Note: i'm not sure why this check is done - the palette index cannot be negative?
            // Perhaps something that was compiled out for the PAL version of the game maybe?
            const int32_t palRow = (palIdx >= 0) ? palIdx : palIdx + 15;
            const int32_t palTPage = palRow / PAL_ROWS_PER_TPAGE;

            dstVramRect.x = (int16_t)(palTPage * 256);
            dstVramRect.y = (int16_t)(palRow - palTPage * PAL_ROWS_PER_TPAGE + SCREEN_H);

            // Upload the palette to VRAM and save the CLUT id for this location
            LIBGPU_LoadImage(dstVramRect, (const uint16_t*) pPalette->colors);
            *pClutId = LIBGPU_GetClut(dstVramRect.x, dstVramRect.y);
        }
    }

    // Set the initial palette to use for the 3d view: use the regular palette
    g3dViewPaletteClutId = gPaletteClutIds[MAINPAL];

    // Clear out the palette data we just loaded as it's now in VRAM and doesn't need a backing RAM store.
    // This will also clear out anything else that is non essential.
    Z_FreeTags(*gpMainMemZone, PU_CACHE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: helper that consolidates some commonly used graphics logic in one function.
//
// Given the specified texture object which is assumed to be 8-bits per pixel (i.e color indexed), returns the 'SRECT' in VRAM where the
// texture would be placed. Useful for getting the texture's VRAM position prior to uploading to the GPU.
//
// I decided to make this a helper rather than duplicate the same code everywhere.
// Produces the same result as the original inline logic, but cleaner.
//------------------------------------------------------------------------------------------------------------------------------------------
SRECT getTextureVramRect(const texture_t& tex) noexcept {
    const int16_t texPageCoordX = (int16_t)(uint16_t) tex.texPageCoordX;    // N.B: make sure to zero extend!
    const int16_t texPageCoordY = (int16_t)(uint16_t) tex.texPageCoordY;
    const uint16_t texPageId = tex.texPageId;

    // Note: all x dimension coordinates get divided by 2 because 'SRECT' coords are for 16-bit mode.
    // This function assumes all textures are 8 bits per pixel.
    SRECT rect = {};
    rect.x = texPageCoordX / 2 + ((texPageId >> 4) & 0x7Fu) * 64u;
    rect.y = texPageCoordY + ((texPageId >> 11) & 0x1Fu) * 256u;
    rect.w = tex.width / 2;
    rect.h = tex.height;

    return rect;
}

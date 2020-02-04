#include "r_data.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/doomdata.h"
#include "PcPsx/Endian.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"

// Details about all of the textures in the game and the sky texture
const VmPtr<VmPtr<texture_t>>   gpTextures(0x80078128);
const VmPtr<VmPtr<texture_t>>   gpFlatTextures(0x80078124);
const VmPtr<VmPtr<texture_t>>   gpSpriteTextures(0x80077EC4);
const VmPtr<VmPtr<texture_t>>   gpSkyTexture(0x80078050);

// Texture translation: converts from an input texture index to the actual texture index to render for that input index.
// Used to implement animated textures whereby the translation is simply updated as the texture animates.
const VmPtr<VmPtr<int32_t>>     gpTextureTranslation(0x80077F6C);

// Flat translation: similar function to texture translation except for flats rather than wall textures.
const VmPtr<VmPtr<int32_t>>     gpFlatTranslation(0x80077F60);

// The loaded lights lump
const VmPtr<VmPtr<light_t>>     gpLightsLump(0x80078068);

// Palette stuff
const VmPtr<uint16_t>   gPaletteClutId_Main(0x800A9084);        // The regular in-game palette
const VmPtr<uint16_t>   g3dViewPaletteClutId(0x80077F7C);       // Currently active in-game palette. Changes as effects are applied in the game.

// Lump number ranges
const VmPtr<int32_t>    gFirstTexLumpNum(0x800782E0);
const VmPtr<int32_t>    gLastTexLumpNum(0x8007819C);
const VmPtr<int32_t>    gNumTexLumps(0x800781D4);
const VmPtr<int32_t>    gFirstFlatLumpNum(0x800782B8);
const VmPtr<int32_t>    gLastFlatLumpNum(0x80078170);
const VmPtr<int32_t>    gNumFlatLumps(0x800781C0);
const VmPtr<int32_t>    gFirstSpriteLumpNum(0x80078014);
const VmPtr<int32_t>    gLastSpriteLumpNum(0x80077F38);
const VmPtr<int32_t>    gNumSpriteLumps(0x80077F5C);

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
    // Determine basic texture list stats
    *gFirstTexLumpNum = W_GetNumForName("T_START") + 1;
    *gLastTexLumpNum = W_GetNumForName("T_END") - 1;
    *gNumTexLumps = *gLastTexLumpNum - *gFirstTexLumpNum + 1;

    // Alloc the list of textures and the texture translation table
    {
        std::byte* const pAlloc = (std::byte*) Z_Malloc(
            **gpMainMemZone,
            (*gNumTexLumps) * (sizeof(texture_t) + sizeof(int32_t)),
            PU_STATIC,
            nullptr
        );

        *gpTextures = (texture_t*) pAlloc;
        *gpTextureTranslation = (int32_t*)(pAlloc + (*gNumTexLumps) * sizeof(texture_t));
    }

    // Load the texture metadata lump and process each associated texture entry with the loaded size info
    {
        maptexture_t* const pMapTextures = (maptexture_t*) W_CacheLumpName("TEXTURE1", PU_CACHE, true);
        maptexture_t* pMapTex = pMapTextures;

        texture_t* pTex = gpTextures->get();
    
        for (int32_t lumpNum = *gFirstTexLumpNum; lumpNum < *gLastTexLumpNum; ++lumpNum, ++pMapTex, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;
            pTex->texPageId = 0;
            pTex->width = Endian::littleToHost(pMapTex->width);
            pTex->height = Endian::littleToHost(pMapTex->height);

            if (pTex->width + 15 >= 0) {
                pTex->width16 = (pTex->width + 15) / 16;
            } else {
                pTex->width16 = (pTex->width + 30) / 16;    // This case is never hit. Not sure why texture sizes would be negative? What does that mean?
            }
            
            if (pTex->height + 15 >= 0) {
                pTex->height16 = (pTex->height + 15) / 16;
            } else {
                pTex->height16 = (pTex->height + 30) / 16;  // This case is never hit. Not sure why texture sizes would be negative? What does that mean?
            }
        }

        // Cleanup this after we are done
        Z_Free2(**gpMainMemZone, pMapTextures);
    }
    
    // Init the texture translation table: initially all textures translate to themselves
    int32_t* const pTexTranslation = gpTextureTranslation->get();

    for (int32_t texIdx = 0; texIdx < *gNumTexLumps; ++texIdx) {
        pTexTranslation[texIdx] = texIdx;
    }
    
    // Clear out any blocks marked as 'cache' which can be evicted.
    // This call is here probably to try and avoid spikes in memory load.
    Z_FreeTags(**gpMainMemZone, PU_CACHE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global flat textures list.
// Also initialize the flat texture translation table used for animation.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitFlats() noexcept {
    // Determine basic flat texture list stats
    *gFirstFlatLumpNum = W_GetNumForName("F_START") + 1;
    *gLastFlatLumpNum = W_GetNumForName("F_END") - 1;
    *gNumFlatLumps = *gLastFlatLumpNum - *gFirstFlatLumpNum + 1;

    // Alloc the list of flat textures and the flat texture translation table
    {
        std::byte* const pAlloc = (std::byte*) Z_Malloc(
            **gpMainMemZone,
            (*gNumFlatLumps) * (sizeof(texture_t) + sizeof(int32_t)),
            PU_STATIC,
            nullptr
        );

        *gpFlatTextures = (texture_t*) pAlloc;
        *gpFlatTranslation = (int32_t*)(pAlloc + (*gNumFlatLumps) * sizeof(texture_t));
    }

    // The hardcoded assumed/size for flats.
    // If you wanted to support variable sized flat textures then the code which follows would need to change.
    constexpr uint32_t FLAT_TEX_SIZE = 64;

    // Setup the texture structs for each flat
    {
        texture_t* pTex = gpFlatTextures->get();

        for (int32_t lumpNum = *gFirstFlatLumpNum; lumpNum < *gLastFlatLumpNum; ++lumpNum, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;
            pTex->texPageId = 0;
            pTex->width = FLAT_TEX_SIZE;
            pTex->height = FLAT_TEX_SIZE;
            pTex->width16 = FLAT_TEX_SIZE / 16;
            pTex->height16 = FLAT_TEX_SIZE / 16;
        }
    }

    // Init the flat translation table: initially all flats translate to themselves
    int32_t* const pFlatTranslation = gpFlatTranslation->get();

    for (int32_t texIdx = 0; texIdx < *gNumFlatLumps; ++texIdx) {
        pFlatTranslation[texIdx] = texIdx;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global sprite textures list and load sprite size and offset metadata
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitSprites() noexcept {
    // Determine basic sprite texture list stats
    *gFirstSpriteLumpNum = W_GetNumForName("S_START") + 1;
    *gLastSpriteLumpNum = W_GetNumForName("S_END") - 1;
    *gNumSpriteLumps = *gLastSpriteLumpNum - *gFirstSpriteLumpNum + 1;

    // Alloc the list of sprite textures
    *gpSpriteTextures = (texture_t*) Z_Malloc(**gpMainMemZone, (*gNumSpriteLumps) * sizeof(texture_t), PU_STATIC, nullptr);

    // Load the sprite metadata lump and process each associated sprite texture entry.
    // The metadata gives us the size and offsetting for each sprite.
    maptexture_t* const pMapSpriteTextures = (maptexture_t*) W_CacheLumpName("SPRITE1", PU_CACHE, true);
    maptexture_t* pMapTex = pMapSpriteTextures;

    texture_t* pTex = gpSpriteTextures->get();
    
    for (int32_t lumpNum = *gFirstSpriteLumpNum; lumpNum <= *gLastSpriteLumpNum; ++lumpNum, ++pTex, ++pMapTex) {
        pTex->lumpNum = (uint16_t) lumpNum;
        pTex->texPageId = 0;
        pTex->offsetX = Endian::littleToHost(pMapTex->offsetX);
        pTex->offsetY = Endian::littleToHost(pMapTex->offsetY);
        pTex->width = Endian::littleToHost(pMapTex->width);
        pTex->height = Endian::littleToHost(pMapTex->height);

        if (pTex->width + 15 >= 0) {
            pTex->width16 = (pTex->width + 15) / 16;
        } else {
            pTex->width16 = (pTex->width + 30) / 16;        // This case is never hit. Not sure why sprite sizes would be negative? What does that mean?
        }

        if (pTex->height + 15 >= 0) {
            pTex->height16 = (pTex->height + 15) / 16;
        } else {
            pTex->height16 = (pTex->height + 30) / 16;      // This case is never hit. Not sure why sprite sizes would be negative? What does that mean?
        }
    }

    // Cleanup this after we are done and clear out any blocks marked as 'cache' - those can be evicted.
    // This call is here probably to try and avoid spikes in memory load.
    Z_Free2(**gpMainMemZone, pMapSpriteTextures);
    Z_FreeTags(**gpMainMemZone, PU_CACHE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Given a lump name (case insensitive) for a texture, returns the texture index among texture lumps.
// Returns '-1' if the name was not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_TextureNumForName(const char* const name) noexcept {
    // Chunk up the name for faster comparisons and also make case insensitive (uppercase).
    // Note: using a union here to try and avoid strict aliasing violations.
    lumpname_t nameUpper = {};
    
    for (int32_t i = 0; (i < 8) && name[i]; ++i) {
        char c = name[i];

        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }

        nameUpper.chars[i] = c;
    }

    // Search for the specified lump name and return the index if found
    const lumpinfo_t* pLumpInfo = &(*gpLumpInfo)[*gFirstTexLumpNum];

    for (int32_t lumpIdx = 0; lumpIdx < *gNumTexLumps; ++lumpIdx, ++pLumpInfo) {
        if (pLumpInfo->name.words[1] == nameUpper.words[1]) {
            if ((pLumpInfo->name.words[0] & NAME_WORD_MASK) == nameUpper.words[0]) {
                // Found the requested lump name!
                return lumpIdx;
            }
        }
    }

    return -1;
}

void _thunk_R_TextureNumForName() noexcept {
    v0 = R_TextureNumForName(vmAddrToPtr<const char>(a0));
}

void R_FlatNumForName() noexcept {
loc_8002BE68:
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_8002BE7C:
    v0 = lbu(a0);
    v1 = v0;
    if (v0 == 0) goto loc_8002BEB4;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_8002BEA0;
    v1 -= 0x20;
loc_8002BEA0:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_8002BE7C;
loc_8002BEB4:
    v0 = *gFirstFlatLumpNum;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E3C);                               // Load from: gpLumpInfo (800781C4)
    a3 = lw(sp);
    v0 <<= 4;
    v0 += v1;
    v1 = *gNumFlatLumps;
    a2 = lw(sp + 0x4);
    a0 = 0;
    if (i32(v1) <= 0) goto loc_8002BF1C;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_8002BEE8:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_8002BF0C;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_8002BF20;
    }
loc_8002BF0C:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_8002BEE8;
loc_8002BF1C:
    v0 = 0;                                             // Result = 00000000
loc_8002BF20:
    sp += 0x10;
    return;
}

void R_InitPalette() noexcept {
loc_8002BF2C:
    sp -= 0x28;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7BA4;                                       // Result = STR_LumpName_LIGHTS[0] (80077BA4)
    a1 = 1;                                             // Result = 00000001
    a2 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    _thunk_W_CacheLumpName();
    v1 = 0xFF;                                          // Result = 000000FF
    *gpLightsLump = v0;
    sb(v1, v0);
    v0 = *gpLightsLump;
    sb(v1, v0 + 0x1);
    v0 = *gpLightsLump;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7BAC;                                       // Result = STR_LumpName_PLAYPAL[0] (80077BAC)
    sb(v1, v0 + 0x2);
    v0 = W_GetNumForName(vmAddrToPtr<const char>(a0));
    s0 = v0;
    a0 = s0;
    a1 = 0x20;                                          // Result = 00000020
    a2 = 1;                                             // Result = 00000001
    _thunk_W_CacheLumpNum();
    s1 = v0;
    v0 = W_LumpLength((int32_t) s0);
    v0 >>= 9;
    v1 = 0x14;
    s0 = 0;
    if (v0 == v1) goto loc_8002BFCC;
    I_Error("R_InitPalettes: palette foulup\n");
loc_8002BFCC:
    s2 = gPaletteClutId_Main;
    v0 = 0x100;                                         // Result = 00000100
    sh(v0, sp + 0x14);
    v0 = 1;                                             // Result = 00000001
    sh(v0, sp + 0x16);
loc_8002BFE4:
    v0 = s0;
    if (i32(s0) >= 0) goto loc_8002BFF0;
    v0 = s0 + 0xF;
loc_8002BFF0:
    a0 = sp + 0x10;
    a1 = s1;
    v0 = u32(i32(v0) >> 4);
    v1 = v0 << 8;
    v0 <<= 4;
    v0 = s0 - v0;
    v0 += 0xF0;
    sh(v1, sp + 0x10);
    sh(v0, sp + 0x12);
    _thunk_LIBGPU_LoadImage();
    s1 += 0x200;
    a0 = lh(sp + 0x10);
    a1 = lh(sp + 0x12);
    s0++;
    LIBGPU_GetClut();
    sh(v0, s2);
    v0 = (i32(s0) < 0x14);
    s2 += 2;
    if (v0 != 0) goto loc_8002BFE4;
    *g3dViewPaletteClutId = *gPaletteClutId_Main;
    
    Z_FreeTags(**gpMainMemZone, PU_CACHE);

    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PC-PSX addition: helper that consolidates some commonly used graphics logic in one function.
//
// Given the specified texture object which is assumed to be 8-bits per pixel (i.e color indexed), returns the 'RECT' in VRAM where the
// texture would be placed. Useful for getting the texture's VRAM position prior to uploading to the GPU.
// 
// I decided to make this a helper rather than duplicate the same code everywhere.
// Produces the same result as the original inline logic, but cleaner.
//------------------------------------------------------------------------------------------------------------------------------------------
RECT getTextureVramRect(const texture_t& tex) noexcept {
    const int16_t texPageCoordX = (int16_t)(uint16_t) tex.texPageCoordX;    // N.B: make sure to zero extend!
    const int16_t texPageCoordY = (int16_t)(uint16_t) tex.texPageCoordY;
    const int16_t texPageId = (int16_t) tex.texPageId;
    
    // Note: all x dimension coordinates get divided by 2 because 'RECT' coords are for 16-bit mode.
    // This function assumes all textures are 8 bits per pixel.
    RECT rect;
    rect.x = texPageCoordX / 2 + (texPageId & 0xF) * 64;
    rect.y = (texPageId & 0x10) * 16 + texPageCoordY;
    rect.w = tex.width / 2;
    rect.h = tex.height;

    return rect;
}

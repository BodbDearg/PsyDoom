//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows patching original game textures to fix a few select issues
//------------------------------------------------------------------------------------------------------------------------------------------
#include "TexturePatcher.h"

#include "Config/Config.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Renderer/r_data.h"

#include <md5.h>

BEGIN_NAMESPACE(TexturePatcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Verifies that the data for the given texture has the expected MD5 hash
//------------------------------------------------------------------------------------------------------------------------------------------
static bool md5HashMatches(const texdata_t texData, const uint64_t expectedMd5Word1, const uint64_t expectedMd5Word2) noexcept {
    // Get the hash for the texture data
    uint8_t md5[16] = {};

    MD5 md5Hasher;
    md5Hasher.add(texData.pBytes, texData.size);
    md5Hasher.getHash(md5);

    // Convert the hash to two 64-bit words
    const uint64_t md5Word1 = (
        ((uint64_t) md5[0 ] << 56) | ((uint64_t) md5[1 ] << 48) | ((uint64_t) md5[2 ] << 40) | ((uint64_t) md5[3 ] << 32) |
        ((uint64_t) md5[4 ] << 24) | ((uint64_t) md5[5 ] << 16) | ((uint64_t) md5[6 ] <<  8) | ((uint64_t) md5[7 ] <<  0)
    );

    const uint64_t md5Word2 = (
        ((uint64_t) md5[8 ] << 56) | ((uint64_t) md5[9 ] << 48) | ((uint64_t) md5[10] << 40) | ((uint64_t) md5[11] << 32) |
        ((uint64_t) md5[12] << 24) | ((uint64_t) md5[13] << 16) | ((uint64_t) md5[14] <<  8) | ((uint64_t) md5[15] <<  0)
    );

    // Return whether the hash matches the expected hash
    return ((md5Word1 == expectedMd5Word1) && (md5Word2 == expectedMd5Word2));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Doom: patches the 'GRATE' floor/ceiling texture to fix unintended holes in it
//------------------------------------------------------------------------------------------------------------------------------------------
void patchTex_GRATE(const texture_t& tex, const texdata_t texData) noexcept {
    uint8_t* const pTexPixels = (uint8_t*)(texData.pBytes + sizeof(texlump_header_t));
    const int32_t numTexPixels = tex.width * tex.height;

    for (int32_t i = 0; i < numTexPixels; ++i) {
        // Replace transparent pixels with black pixels
        if (pTexPixels[i] == 0) {
            pTexPixels[i] = 8;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Final Doom: patches the 'STATUS' texture to fix unintended pixels on the keycards
//------------------------------------------------------------------------------------------------------------------------------------------
void patchTex_FinalDoom_STATUS([[maybe_unused]] const texture_t& tex, const texdata_t texData) noexcept {
    uint8_t* const pTexPixels = (uint8_t*)(texData.pBytes + sizeof(texlump_header_t));
    pTexPixels[49031] = 0;
    pTexPixels[49032] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies patches to the specified texture, if appropriate
//------------------------------------------------------------------------------------------------------------------------------------------
void applyTexturePatches(const texture_t& tex, const texdata_t texData) noexcept {
    // Only apply texture patches if visual map patches are enabled
    if (!Config::gbEnableMapPatches_Visual)
        return;

    // Get the lump name and remove the compressed flag from the first character
    WadLumpName lumpName = W_GetLumpName(tex.lumpNum);
    lumpName.chars[0] &= 0x7F;

    // Patch this texture?
    if (lumpName == "GRATE") {
        if (md5HashMatches(texData, 0x57EC4B1EF4567271, 0xA097852AE0E5C725)) {
            patchTex_GRATE(tex, texData);
        }
    } 
    else if (lumpName == "STATUS") {
        if (md5HashMatches(texData, 0xD35CCD07913A9366, 0xC1B0D65C90757FAA)) {
            patchTex_FinalDoom_STATUS(tex, texData);
        }
    }
}

END_NAMESPACE(TexturePatcher)

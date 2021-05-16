#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the global wall textures list and load texture size metadata.
// Also initialize the texture translation table for animated wall textures.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitTextures() noexcept {
    // Determine basic texture list stats
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

    // Load the texture metadata lump and process each associated texture entry with the loaded size info
    {
        maptexture_t* const pMapTextures = (maptexture_t*) W_CacheLumpName("TEXTURE1", PU_CACHE, true);
        maptexture_t* pMapTex = pMapTextures;
        texture_t* pTex = gpTextures;

        for (int32_t lumpNum = gFirstTexLumpNum; lumpNum <= gLastTexLumpNum; ++lumpNum, ++pMapTex, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;
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
    // Determine basic flat texture list stats
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

    // The hardcoded assumed/size for flats
    constexpr uint32_t FLAT_TEX_SIZE = 64;

    // Setup the texture structs for each flat
    {
        texture_t* pTex = gpFlatTextures;

        for (int32_t lumpNum = gFirstFlatLumpNum; lumpNum <= gLastFlatLumpNum; ++lumpNum, ++pTex) {
            pTex->lumpNum = (uint16_t) lumpNum;
            pTex->texPageId = 0;
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
    maptexture_t* const pMapSpriteTextures = (maptexture_t*) W_CacheLumpName("SPRITE1", PU_CACHE, true);
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Given a lump name (case insensitive) for a wall texture, returns the texture index among wall texture lumps.
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
    const lumpinfo_t* pLumpInfo = &gpLumpInfo[gFirstTexLumpNum];

    for (int32_t lumpIdx = 0; lumpIdx < gNumTexLumps; ++lumpIdx, ++pLumpInfo) {
        if (pLumpInfo->name.words[1] == nameUpper.words[1]) {
            if ((pLumpInfo->name.words[0] & NAME_WORD_MASK) == nameUpper.words[0]) {
                // Found the requested lump name!
                return lumpIdx;
            }
        }
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Given a lump name (case insensitive) for a flat texture, returns the texture index among flat texture lumps
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_FlatNumForName(const char* const name) noexcept {
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
    const lumpinfo_t* pLumpInfo = &gpLumpInfo[gFirstFlatLumpNum];

    for (int32_t lumpIdx = 0; lumpIdx < gNumTexLumps; ++lumpIdx, ++pLumpInfo) {
        if (pLumpInfo->name.words[1] == nameUpper.words[1]) {
            if ((pLumpInfo->name.words[0] & NAME_WORD_MASK) == nameUpper.words[0]) {
                // Found the requested lump name!
                return lumpIdx;
            }
        }
    }

    return 0;   // This was probably a bug, should be '-1' instead?
}

#endif  // #if !PSYDOOM_MODS

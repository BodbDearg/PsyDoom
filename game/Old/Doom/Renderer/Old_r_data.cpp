#if !PSYDOOM_MODS

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
// Given a lump name (case insensitive) for a flat texture, returns the texture index among flat texture lumps.
// PsyDoom: Returns '-1' if the name was not found. Originally returned '0'.
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

    // PsyDoom: Returning '0' means we return a valid index even if not found.
    // Fix this and change the return to '-1' if not found.
    #if PSYDOOM_MODS
        return -1;
    #else
        return 0;
    #endif
}

#endif  // #if !PSYDOOM_MODS

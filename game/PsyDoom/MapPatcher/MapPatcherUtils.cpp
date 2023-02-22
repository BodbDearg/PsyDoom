#include "MapPatcherUtils.h"

#include "Doom/Base/sounds.h"
#include "Doom/Game/p_telept.h"

BEGIN_NAMESPACE(MapPatcherUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears mysterious sector flags (of unknown purpose) other than 'SF_NO_REVERB' found in original 'Doom' and 'Final Doom' maps.
// 
// The 'SF_NO_REVERB' flag was the only one originally used by the retail version of the game but for some reason various sectors in the
// game can use any of the 16 available bits as sector flags. This would not be a problem except that PsyDoom now assigns meaning to the
// most of the flag bits (for 2-colored lighting and other features) and this can cause issues for original game maps.
// Hence we must clear all sector flag bits (other than the 1st) for original game maps.
//------------------------------------------------------------------------------------------------------------------------------------------
void clearMysterySectorFlags() noexcept {
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        // Note: flags is now only interpreted as 8-bits in the level data, the other 8-bits are used for 'ceilColorid'
        pSectors[i].flags &= (~SF_GHOSTPLAT);

        // Originally this was the high 8-bits of the 'flags' field.
        // Original maps never use 2 colored lighting so just set it to the floor color.
        pSectors[i].ceilColorid = pSectors[i].colorid;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies patches common to ALL original 'Doom' and 'Final Doom' maps
//------------------------------------------------------------------------------------------------------------------------------------------
void applyOriginalMapCommonPatches() noexcept {
    // Note: always apply this patch regardless of map patch settings.
    // This one is CRITICAL to being able to play original game maps in PsyDoom!
    clearMysterySectorFlags();
}

END_NAMESPACE(MapPatches)

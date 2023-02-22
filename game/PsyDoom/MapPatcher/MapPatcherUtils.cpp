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

//------------------------------------------------------------------------------------------------------------------------------------------
// Move a single map object matching the specified type and x/y position to another location.
// The type or x/y position can be ommitted to waive matching those fields, if required.
// Can be used to fix placement bugs for various map objects.
// 
// Returns 'true' if the move was actually performed, 'false' otherwise.
// 
// Notes:
//  (1) All x/y coordinates are specified as 16-bit integers (whole numbers).
//      This works for fixing map bugs on startup because the map format stores positions as 16-bit integers anyway.
//  (2) Does not do any form of collision testing while performing the relocation.
//      The map object can be moved anywhere.
//------------------------------------------------------------------------------------------------------------------------------------------
bool moveMobj(
    const std::optional<int32_t> srcSectorIdx,
    const std::optional<mobjtype_t> srcType,
    const std::optional<int32_t> srcX,
    const std::optional<int32_t> srcY,
    const std::optional<int32_t> dstX,
    const std::optional<int32_t> dstY,
    const std::optional<angle_t> dstAngle
) noexcept {
    // Helper that tries to move the specified map object, provided that it matches the required conditions.
    // Returns 'true' if the move was actually performed.
    const auto checkMobjAndMove = [&](mobj_t& mobj) noexcept -> bool {
        const mobjtype_t mobjType = mobj.type;
        const int32_t mobjX = d_fixed_to_int(mobj.x);
        const int32_t mobjY = d_fixed_to_int(mobj.y);

        const bool bMatchSrcType = (srcType.value_or(mobjType) == mobjType);
        const bool bMatchSrcX = (srcX.value_or(mobjX) == mobjX);
        const bool bMatchSrcY = (srcY.value_or(mobjY) == mobjY);
        const bool bMatch = (bMatchSrcType && bMatchSrcX && bMatchSrcY);

        if (!bMatch)
            return false;

        // Note: use noclip to always allow the move
        const uint32_t oldMobjFlags = mobj.flags;
        mobj.flags = oldMobjFlags | MF_NOCLIP;
            
        EV_TeleportTo(
            mobj,
            dstX.has_value() ? d_int_to_fixed(dstX.value()) : mobj.x,
            dstY.has_value() ? d_int_to_fixed(dstY.value()) : mobj.y,
            dstAngle.has_value() ? dstAngle.value() : mobj.angle,
            false,
            false,
            (mobjtype_t) 0,
            sfx_None
        );

        mobj.flags = oldMobjFlags;
        return true;
    };

    // Is the source sector specified?
    if (srcSectorIdx.has_value()) {
        // Source sector specified: this makes the search much more specific
        ASSERT((srcSectorIdx >= 0) && (srcSectorIdx < gNumSectors));

        for (mobj_t* pMobj = gpSectors[srcSectorIdx.value()].thinglist; pMobj != nullptr; pMobj = pMobj->snext) {
            if (checkMobjAndMove(*pMobj))
                return true;
        }
    }
    else {
        // No source sector specified: check map objects in all sectors
        for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
            if (checkMobjAndMove(*pMobj))
                return true;
        }
    }

    return false;
}

END_NAMESPACE(MapPatches)

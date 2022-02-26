#include "p_telept.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "info.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "p_setup.h"
#include "p_tick.h"

#include <cstdlib>

//------------------------------------------------------------------------------------------------------------------------------------------
// Telefrags map objects (that can be shot) around the given object when placed at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Telefrag(
    // PsyDoom: allow self-telefragging to be disabled (required for the 'Icon Of Sin' spawner boxes)
    #if PSYDOOM_MODS
        mobj_t& mobj, const fixed_t x, const fixed_t y, const bool bCanSelfTelefrag = true
    #else
        mobj_t& mobj, const fixed_t x, const fixed_t y
    #endif
) noexcept {
    for (mobj_t* pTarget = gMobjHead.next; pTarget != &gMobjHead; pTarget = pTarget->next) {
        // Can't telefrag the object if it's not shootable
        if ((pTarget->flags & MF_SHOOTABLE) == 0)
            continue;

        // PsyDoom: allow self-telefragging to be disabled (required for the 'Icon Of Sin' spawner boxes)
        #if PSYDOOM_MODS
            if ((pTarget == &mobj) && (!bCanSelfTelefrag))
                continue;
        #endif

        // Telefrag this target if it's closer than the minimum separation distance
        const fixed_t dx = std::abs(pTarget->x - x);
        const fixed_t dy = std::abs(pTarget->y - y);
        const fixed_t minDist = pTarget->radius + mobj.radius + 4 * FRACUNIT;

        if ((dx <= minDist) && (dy <= minDist)) {
            P_DamageMobj(*pTarget, &mobj, &mobj, 10000);    // Damage by a very large amount to kill
            pTarget->flags &= ~(MF_SOLID | MF_SHOOTABLE);   // Now dead (hopefully?!)
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Teleport the given map object to a sector with a tag matching the given line and with a valid destination marker.
// Returns 'true' if the teleportation was done successfully.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_Teleport(line_t& line, mobj_t& mobj) noexcept {
    // Don't teleport if coming from the back side of the line and now on the front.
    // This is so you can get out of the teleporter.
    if (P_PointOnLineSide(mobj.x, mobj.y, line) == 0)
        return false;

    // Missiles cannot teleport (wouldn't that have been fun though?!)
    if (mobj.flags & MF_MISSILE)
        return false;

    // Search for a teleport destination marker in a sector with a tag matching the given line
    sector_t* const pSectors = gpSectors;

    for (int32_t sectorIdx = 0; sectorIdx < gNumSectors; ++sectorIdx) {
        // Ignore this sector if it doesn't have the right tag
        sector_t& sector = pSectors[sectorIdx];

        if (sector.tag != line.tag)
            continue;

        // Try to find a teleport destination that is in the target sector
        for (mobj_t* pDstMarker = gMobjHead.next; pDstMarker != &gMobjHead; pDstMarker = pDstMarker->next) {
            // Ignore if the marker is not a teleport marker or not in this sector
            if (pDstMarker->type != MT_TELEPORTMAN)
                continue;

            const int32_t destSectorIdx = (int32_t)(pDstMarker->subsector->sector - pSectors);

            if (destSectorIdx != sectorIdx)
                continue;

            // Reset the number of lines to check for being crossed (to trigger specials) and remember the pre-teleport position for FX
            #if PSYDOOM_MODS
                gpCrossCheckLines.clear();
                gpCrossCheckLines.reserve(32);
            #else
                gNumCrossCheckLines = 0;
            #endif

            const fixed_t oldX = mobj.x;
            const fixed_t oldY = mobj.y;
            const fixed_t oldZ = mobj.z;

            // Mark the object as currently teleporting and telefrag if a player
            mobj.flags |= MF_TELEPORT;

            if (mobj.player) {
                // PsyDoom: disabling all self telefragging
                #if PSYDOOM_MODS
                    P_Telefrag(mobj, pDstMarker->x, pDstMarker->y, false);
                #else
                    P_Telefrag(mobj, pDstMarker->x, pDstMarker->y);
                #endif
            }

            // See if the teleport move can be made and abort if not
            const bool bCanMove = P_TryMove(mobj, pDstMarker->x, pDstMarker->y);
            mobj.flags &= ~MF_TELEPORT;

            if (!bCanMove)
                return false;

            // Ground the thing being teleported
            mobj.z = mobj.floorz;

            // Spawn teleport fog at both the source and destination
            {
                mobj_t& srcFog = *P_SpawnMobj(oldX, oldY, oldZ, MT_TFOG);
                S_StartSound(&srcFog, sfx_telept);

                // Note: spawn the destination fog a little in front of the thing being teleported
                const uint32_t fineAng = pDstMarker->angle >> ANGLETOFINESHIFT;
                const fixed_t dstFogX = pDstMarker->x + gFineCosine[fineAng] * 20;
                const fixed_t dstFogY = pDstMarker->y + gFineSine[fineAng] * 20;

                mobj_t& dstFog = *P_SpawnMobj(dstFogX, dstFogY, mobj.z, MT_TFOG);
                S_StartSound(&dstFog, sfx_telept);
            }

            // Force player to not move for a little bit
            if (mobj.player) {
                mobj.reactiontime = 9;
            }

            // Kill player momentum and set exit angle to that of the destination marker
            mobj.momz = 0;
            mobj.momy = 0;
            mobj.momx = 0;
            mobj.angle = pDstMarker->angle;

            // PsyDoom: if we just teleported this player then kill any interpolations.
            // Also kill any interpolations for the map object itself.
            #if PSYDOOM_MODS
                if (mobj.player == &gPlayers[gCurPlayerIndex]) {
                    R_NextPlayerInterpolation();
                    R_NextWorldInterpolation();
                }

                R_SnapMobjInterpolation(mobj);
            #endif

            // Teleportation was a success!
            return true;
        }
    }

    return false;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// A teleport function meant to be invoked via scripting on non-player objects or for silent player teleporting.
// Allows for greater customization of the teleporting, 
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_TeleportTo(
    mobj_t& mobj,
    const fixed_t dstX,
    const fixed_t dstY,
    const angle_t dstAngle,
    const bool bTelefrag,
    const bool bPreserveMomentum,
    const mobjtype_t fogMobjType,
    const sfxenum_t fogSoundId
) noexcept {
    // Reset the number of lines to check for being crossed (to trigger specials) and remember the pre-teleport position for FX
    gpCrossCheckLines.clear();
    gpCrossCheckLines.reserve(32);

    const fixed_t oldX = mobj.x;
    const fixed_t oldY = mobj.y;
    const fixed_t oldZ = mobj.z;

    // Mark the object as currently teleporting and telefrag if specified
    mobj.flags |= MF_TELEPORT;

    if (bTelefrag) {
        P_Telefrag(mobj, dstX, dstY, false);
    }

    // See if the teleport move can be made and abort if not
    const bool bCanMove = P_TryMove(mobj, dstX, dstY);
    mobj.flags &= ~MF_TELEPORT;

    if (!bCanMove)
        return false;

    // Ground the thing being teleported
    mobj.z = mobj.floorz;

    // Spawn teleport fog at both the source and destination, if a valid fog type is defined.
    // Also play the teleport fog sound if defined.
    const bool bFogTypeValid = ((fogMobjType != 0) && ((uint32_t) fogMobjType < (uint32_t) gNumMobjInfo));

    if (bFogTypeValid) {
        mobj_t& srcFog = *P_SpawnMobj(oldX, oldY, oldZ, fogMobjType);

        if (fogSoundId != sfx_None) {
            S_StartSound(&srcFog, fogSoundId);
        }

        mobj_t& dstFog = *P_SpawnMobj(dstX, dstY, mobj.z, MT_TFOG);

        if (fogSoundId != sfx_None) {
            S_StartSound(&dstFog, fogSoundId);
        }
    } else {
        if (fogSoundId != sfx_None) {
            degenmobj_t oldMobjPos = {};
            oldMobjPos.x = oldX;
            oldMobjPos.y = oldY;
            oldMobjPos.z = oldZ;
            oldMobjPos.subsector = R_PointInSubsector(oldX, oldY);

            S_StartSound((mobj_t*) &oldMobjPos, fogSoundId);
            S_StartSound(&mobj, fogSoundId);
        }
    }

    // Kill momentum (if specified) and set exit angle
    if (!bPreserveMomentum) {
        mobj.momx = 0;
        mobj.momy = 0;
        mobj.momz = 0;
    }

    mobj.angle = dstAngle;

    // If it's player and we're preserving momentum adjust the current xy interpolation to account for the teleportation.
    // This is done so we can do a smooth silent teleport to another location.
    if (bPreserveMomentum && (mobj.player == &gPlayers[gCurPlayerIndex])) {
        // Snap the z component if it's changed by the teleportation
        if (oldZ != mobj.z) {
            R_SnapViewZInterpolation();
        }

        const fixed_t dx = oldX - gOldViewX;
        const fixed_t dy = oldY - gOldViewY;
        gOldViewX = mobj.x - dx;
        gOldViewY = mobj.y - dy;
        gOldViewAngle = mobj.angle;
    } 
    else {
        // Otherwise snap all movements
        R_SnapMobjInterpolation(mobj);
    }

    // Teleportation was a success!
    return true;
}
#endif  // #if PSYDOOM_MODS

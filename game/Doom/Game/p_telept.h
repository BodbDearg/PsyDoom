#pragma once

#include "Doom/doomdef.h"

struct line_t;
enum sfxenum_t : int32_t;

// PsyDoom: allow self-telefragging to be disabled (required for the 'Icon Of Sin' spawner boxes)
#if PSYDOOM_MODS
    void P_Telefrag(mobj_t& mobj, const fixed_t x, const fixed_t y, const bool bCanSelfTelefrag) noexcept;
#else
    void P_Telefrag(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
#endif

bool EV_Teleport(line_t& line, mobj_t& mobj) noexcept;

// PsyDoom: try teleport to a specific point with additional options
#if PSYDOOM_MODS
    bool EV_TeleportTo(
        mobj_t& mobj,
        const fixed_t dstX,
        const fixed_t dstY,
        const angle_t dstAngle,
        const bool bTelefrag,
        const bool bPreserveMomentum,
        const mobjtype_t fogMobjType,
        const sfxenum_t fogSoundId
    ) noexcept;
#endif

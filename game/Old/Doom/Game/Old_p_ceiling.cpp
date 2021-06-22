#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpauses ceiling movers which have the same tag as the given line and which are in stasis (paused)
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ActivateInStasisCeiling(line_t& line) noexcept {
    const int32_t numCeilings = MAXCEILINGS;

    for (int32_t i = 0; i < numCeilings; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction == 0)) {    // Direction 0 = in stasis
            pCeiling->direction = pCeiling->olddirection;
            pCeiling->thinker.function = (think_t) &T_MoveCeiling;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses active ceiling movers (crushers) with the same sector tag as the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_CeilingCrushStop(line_t& line) noexcept {
    const int32_t numCeilings = MAXCEILINGS;

    bool bPausedACrusher = false;

    for (int32_t i = 0; i < numCeilings; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction != 0)) {
            pCeiling->olddirection = pCeiling->direction;       // Remember which direction it was moving in for unpause
            pCeiling->direction = 0;                            // Now in stasis
            pCeiling->thinker.function = nullptr;               // Remove the thinker function until unpaused
            bPausedACrusher = true;
        }
    }

    return bPausedACrusher;
}

#endif  // #if !PSYDOOM_MODS

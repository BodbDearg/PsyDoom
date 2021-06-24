#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Reactivates moving platforms that were paused which match the given sector tag
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ActivateInStasis(const int32_t tag) noexcept {
    const int32_t numPlats = MAXPLATS;

    for (int32_t i = 0; i < numPlats; ++i) {
        plat_t* pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->tag == tag) && (pPlat->status == in_stasis)) {
            pPlat->status = pPlat->oldstatus;
            pPlat->thinker.function = (think_t) &T_PlatRaise;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops moving platforms that are moving that have a sector tag matching the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_StopPlat(line_t& line) noexcept {
    const int32_t numPlats = MAXPLATS;

    for (int32_t i = 0; i < numPlats; ++i) {
        plat_t* const pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->status != in_stasis) && (pPlat->tag == line.tag)) {
            // Stop this moving platform: remember the status before stopping and put into stasis
            pPlat->oldstatus = pPlat->status;
            pPlat->status = in_stasis;
            pPlat->thinker.function = nullptr;
        }
    }
}

#endif  // #if !PSYDOOM_MODS

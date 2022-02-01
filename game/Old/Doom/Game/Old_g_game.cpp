#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Run the actual in-game (3d) portions of the game.
// Also do intermission and finale screens following a level.
// This is used to run the game for non-demo gameplay.
// 
// PsyDoom note: this function still contains some modifications necessary to support both Doom and Final Doom in the same binary.
// This is not the TRUE original version but very close. The original version differs between Doom and Final Doom.
//------------------------------------------------------------------------------------------------------------------------------------------
void G_RunGame() noexcept {
    while (true) {
        // Load the level and run the game
        G_DoLoadLevel();
        MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

        // Assume we are not restarting the current level at first
        gbIsLevelBeingRestarted = false;

        // End demo recording actions
        if (gGameAction == ga_recorddemo) {
            G_EndDemoRecording();
        }

        if (gGameAction == ga_warped)
            continue;

        // Can restart the level if died or explicitly restarting
        if ((gGameAction == ga_died) || (gGameAction == ga_restart)) {
            gbIsLevelBeingRestarted = true;
            continue;
        }

        // Cleanup after the level is done.
        // Texture cache: unlock everything except UI assets and other reserved areas of VRAM.
        gLockedTexPagesMask &= 1;
        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);

        if (gGameAction == ga_exitdemo)
            break;

        // Do the intermission
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

        // Should we do the Ultimate DOOM style (text only, no cast sequence) finale?
        //
        // Notes:
        //  (1) For Final Doom this finale type will show when finishing the first 2 out of 3 episodes.
        //  (2) Showing finales is restricted to the case where we are not warping to a secret level, since the
        //      Ultimate Doom secret levels will not be within the map number range for the Ultimate Doom episode.
        //      Any warp to a level other than the next one is considered a secret level warp.
        //  (3) PsyDoom: I'm restricting endings to the Doom and Final Doom games specifically.
        //      If some other game type is playing then we simply won't do them.
        //
        const bool bDoFinales = ((Game::gGameType == GameType::Doom) || (Game::gGameType == GameType::FinalDoom));
        const bool bGoingToSecretLevel = (gNextMap != gGameMap + 1);
        const int32_t curEpisodeNum = Game::getMapEpisode(gGameMap);
        const int32_t nextEpisodeNum = Game::getMapEpisode(gNextMap);

        if ((gNetGame == gt_single) && (!bGoingToSecretLevel) && (curEpisodeNum != nextEpisodeNum)) {
            if (bDoFinales) {
                MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);
            } else {
                gGameAction = ga_nothing;
                break;
            }

            if (gGameAction == ga_warped || gGameAction == ga_restart)
                continue;

            if (gGameAction == ga_exitdemo)
                break;

            gStartMapOrEpisode = -nextEpisodeNum;   // The '-' instructs the main menu to select this episode automatically
            break;
        }

        // If there is a next map go onto it, otherwise show the DOOM II style finale (text, followed by cast)
        if (gNextMap <= Game::getNumMaps()) {
            gGameMap = gNextMap;
            continue;
        }

        if (bDoFinales) {
            MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);
        } else {
            gGameAction = ga_nothing;
            break;
        }

        if ((gGameAction != ga_warped) && (gGameAction != ga_restart))
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays back the current demo in the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t G_PlayDemoPtr() noexcept {
    // Read the demo skill and map number
    gpDemo_p = gpDemoBuffer;
    const skill_t skill = Demo_Read<skill_t>();
    const int32_t mapNum = Demo_Read<int32_t>();

    // Read the control bindings for the demo and save the previous ones before that.
    //
    // Note: for original PSX Doom there are 8 bindings, for Final Doom there are 10.
    // Need to adjust demo reading accordingly depending on which game version we are dealing with.
    padbuttons_t prevCtrlBindings[NUM_BINDABLE_BTNS];
    D_memcpy(prevCtrlBindings, gCtrlBindings, sizeof(prevCtrlBindings));

    if (Game::isFinalDoom()) {
        Demo_Read(gCtrlBindings, NUM_BINDABLE_BTNS);
    } else {
        // Note: original Doom did not have the move forward/backward bindings (due to no mouse support) - hence they are zeroed here:
        Demo_Read(gCtrlBindings, 8);
        gCtrlBindings[8] = 0;
        gCtrlBindings[9] = 0;
    }

    static_assert(sizeof(prevCtrlBindings) == 40);

    // For Final Doom read the mouse sensitivity and save the old value to restore later
    const int32_t oldPsxMouseSensitivity = gPsxMouseSensitivity;

    if (Game::isFinalDoom()) {
        gPsxMouseSensitivity = Demo_Read<int32_t>();
    }

    // Do basic game initialization
    G_InitNew(skill, mapNum, gt_single);

    // Load the map used by the demo
    G_DoLoadLevel();

    // Run the demo
    gbDemoPlayback = true;
    const gameaction_t exitAction = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    gbDemoPlayback = false;

    // Restore the previous control bindings, mouse sensitivity and cleanup
    D_memcpy(gCtrlBindings, prevCtrlBindings, sizeof(prevCtrlBindings));
    gPsxMouseSensitivity = oldPsxMouseSensitivity;

    // Texture cache: unlock everything except UI assets and other reserved areas of VRAM
    gLockedTexPagesMask &= 1;
    Z_FreeTags(*gpMainMemZone, PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE);

    return exitAction;
}

#endif  // #if !PSYDOOM_MODS

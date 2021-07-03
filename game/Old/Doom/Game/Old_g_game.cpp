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

#endif  // #if !PSYDOOM_MODS

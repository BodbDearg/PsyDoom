#pragma once

#include "SmallString.h"

enum class GameType : int32_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Constants for the current game type.
// These are NOT configurable by the user, just a convenient and centralized way to express differences between the game versions.
//------------------------------------------------------------------------------------------------------------------------------------------
struct GameConstants {
    String16    mainWads[4];                        // The list of main resource WADS to load for this game (terminated by an empty string, earlier WADs take precedence).
    uint32_t    netGameId;                          // ID to send for the game type in networked games. Must match the game id for other players.
    uint32_t    baseNumAnims;                       // How many animated floor and wall textures are built into the game by default? (ignoring any loaded user mods)
    uint8_t     texPalette_BUTTONS;                 // Which palette to use for the 'BUTTONS' lump? (Note: this lump is non longer used by PsyDoom, here just for historical reference)
    uint8_t     numPalettesRequired;                // How many palettes this game requires (at a minimum) to be available
    bool        bUseFinalDoomClassicDemoFormat;     // Whether to interpret classic demos as being in the Final Doom format or not
    bool        bUseFinalDoomSkyPalettes;           // Use Final Doom style sky palettes for certain skies?
    bool        bUseFinalDoomPasswordStorage;       // PsyDoom: store the last password in the 'Final Doom' password field instead of 'Doom'?

    void populate(const GameType gameType) noexcept;
};

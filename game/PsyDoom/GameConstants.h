#pragma once

#include "PsyDoom/WadFile.h"

enum class GameType : int32_t;
struct palette_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a demo in the classic PSX Doom or Final Doom format
//------------------------------------------------------------------------------------------------------------------------------------------
struct ClassicDemoDef {
    // Definition for a function that can override the map number contained in the demo file.
    // Needed if the map numbers for the current game are remapped, like with the '[GEC] Master Edition (Beta 3)' disc.
    typedef int32_t (*MapNumOverrideFn)(const int32_t) noexcept;

    String16            filename;               // Name of the demo file on the disc (just the filename, not the full path)
    bool                bFinalDoomDemo;         // Is the demo file in the Final Doom format, and with Final Doom game rules?
    bool                bPalDemo;               // Is the demo for the PAL version of the game? (if not then it's NTSC)
    bool                bShowCreditsAfter;      // Show the credits screen after this demo finishes?
    MapNumOverrideFn    mapNumOverrider;        // A function that remaps the map number referenced by the demo file (if required)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Constants for the current game type.
// These are NOT configurable by the user, just a convenient and centralized way to express differences between the game versions.
//------------------------------------------------------------------------------------------------------------------------------------------
struct GameConstants {
    String16                mainWads[4];                        // The list of main resource WADS to load for this game (terminated by an empty string, earlier WADs take precedence).
    RemapWadLumpNameFn      mainWadLumpRemappers[4];            // Optional remappers that change/remap the names of the lumps in each main wad
    String32                introMovies[3];                     // Intro movies to play for the game (up to 3, blank string terminated list)
    ClassicDemoDef          demos[8];                           // Demos to play for this game (up to 8, the list is terminated by an empty demo filename)
    const palette_t*        pExtraPalettes;                     // Extra palettes to load for this game on top of the 'PLAYPAL' lump
    uint8_t                 numExtraPalettes;                   // How many extra palettes there are in 'pExtraPalettes'
    uint8_t                 numPalettesRequired;                // How many palettes this game requires (at a minimum) to be available
    uint32_t                netGameId;                          // ID to send for the game type in networked games. Must match the game id for other players.
    uint32_t                baseNumAnims;                       // How many animated floor and wall textures are built into the game by default? (ignoring any loaded user mods)
    uint8_t                 texPalette_BUTTONS;                 // Which palette to use for the 'BUTTONS' lump? (Note: this lump is non longer used by PsyDoom, here just for historical reference)
    bool                    bUseFinalDoomSkyPalettes;           // Use Final Doom style sky palettes for certain skies?
    bool                    bUseFinalDoomPasswordStorage;       // PsyDoom: store the last password in the 'Final Doom' password field instead of 'Doom'?
    int16_t                 doomLogoYPos;                       // Y position to render the 'DOOM' logo at in menus

    void populate(const GameType gameType, const bool bIsDemoVersion) noexcept;
};

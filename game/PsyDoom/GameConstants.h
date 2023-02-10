#pragma once

#include "PsyDoom/WadFile.h"

enum class GameType : int32_t;
struct palette_t;

namespace PlayerPrefs {
    struct Password;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a built-in demo for the current game type which is present on the game disc.
// The demo will normally be in the classic PSX Doom or Final Doom format, but can also be in the 'GEC Master Edition' format.
//------------------------------------------------------------------------------------------------------------------------------------------
struct BuiltInDemoDef {
    // Definition for a function that can override the map number contained in the demo file.
    // Needed if the map numbers for the current game are remapped, like with the '[GEC] Master Edition (Beta 3)' disc.
    typedef int32_t (*MapNumOverrideFn)(const int32_t) noexcept;

    String16            filename;               // Name of the demo file on the disc (just the filename, not the full path)
    bool                bFinalDoomDemo;         // Is the demo file in the Final Doom format, and with Final Doom game rules? (note: this field is ignored for the GEC demo format)
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
    BuiltInDemoDef          demos[8];                           // Demos to play for this game (up to 8, the list is terminated by an empty demo filename)
    const char*             saveFilePrefix;                     // Prefix added to all save files for this game type (like 'Doom_')
    PlayerPrefs::Password*  pLastPasswordField;                 // Which player prefs field this game stores the last map password in
    const palette_t*        pExtraPalettes;                     // Extra palettes to load for this game on top of the 'PLAYPAL' lump
    uint8_t                 numExtraPalettes;                   // How many extra palettes there are in 'pExtraPalettes'
    uint8_t                 numPalettesRequired;                // How many palettes this game requires (at a minimum) to be available
    uint32_t                netGameId;                          // ID to send for the game type in networked games. Must match the game id for other players.
    uint32_t                baseNumAnims;                       // How many animated floor and wall textures are built into the game by default? (ignoring any loaded user mods)
    uint8_t                 texPalette_BUTTONS;                 // Which palette to use for the 'BUTTONS' lump? (Note: this lump is non longer used by PsyDoom, here just for historical reference)
    bool                    bUseFinalDoomSkyPalettes;           // Use Final Doom style sky palettes for certain skies?

    // GEC ME Beta 4 and later: activate a hack to choose the right 'LOADING' plaque graphic?
    // In GEC ME Beta 4 the 'LOADING' graphic has a progress bar baked into it, but we don't want that for PsyDoom.
    // The lump immediately after it is also called 'LOADING' but does not have the progress bar.
    // Enabling this hack makes PsyDoom choose the version of the progress bar without the loading bar, if detected next to the other version.
    bool bRemove_LOADING_progressBar;

    void populate(const GameType gameType, const bool bIsDemoVersion) noexcept;

    void SetNumDemos_GecMe_Beta4OrLater(const int32_t numDemos) noexcept;
};

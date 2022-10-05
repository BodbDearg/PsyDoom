#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

enum class SaveFileSlot : uint8_t;

// What mode the load/save menu is in
enum class LoadSaveMenuMode : int32_t {
    Load,
    Save
};

// The context from which a save operation was triggered
enum class SaveGameContext : int32_t {
    Menu,           // Saving was triggered via the save menu
    Autosave,       // Saving was triggered via autosave on level start
    Quicksave       // Saving was triggered by the 'quicksave' key
};

// The context from which a load operation was triggered
enum class LoadGameContext : int32_t {
    Menu,           // The load was triggered by the load menu
    Quickload       // The load was triggered by the 'quickload' key
};

void LoadSave_SetMode(const LoadSaveMenuMode mode) noexcept;
void LoadSave_Init() noexcept;
void LoadSave_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t LoadSave_Update() noexcept;
void LoadSave_Draw() noexcept;
gameaction_t SaveGameForSlot(const SaveFileSlot slot, const SaveGameContext saveContext) noexcept;
gameaction_t LoadGameForSlot(const SaveFileSlot slot, const LoadGameContext loadContext) noexcept;
bool ShouldLoadSaveOnLevelStart() noexcept;
void ClearLoadSaveOnLevelStartFlag() noexcept;
void DisplaySavedHudMessage(const SaveGameContext saveContext, const bool bSuccess) noexcept;
void DisplayLoadedHudMessage(const LoadGameContext loadContext, const bool bSuccess) noexcept;
void DoQuicksave() noexcept;
[[nodiscard]] gameaction_t DoQuickload() noexcept;

#endif  // #if PSYDOOM_MODS

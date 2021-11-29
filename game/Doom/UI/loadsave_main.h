#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

enum class SaveFileSlot : uint8_t;

enum class LoadSaveMenuMode : int32_t {
    Load,
    Save
};

void LoadSave_SetMode(const LoadSaveMenuMode mode) noexcept;
void LoadSave_Init() noexcept;
void LoadSave_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t LoadSave_Update() noexcept;
void LoadSave_Draw() noexcept;
gameaction_t SaveGameForSlot(const SaveFileSlot slot) noexcept;
gameaction_t LoadGameForSlot(const SaveFileSlot slot) noexcept;
bool ShouldLoadSaveOnLevelStart() noexcept;
void ClearLoadSaveOnLevelStartFlag() noexcept;
void DisplaySavedHudMessage(const SaveFileSlot slot, const bool bSuccess) noexcept;
void DisplayLoadedHudMessage(const SaveFileSlot slot, const bool bSuccess) noexcept;
void DoQuicksave() noexcept;
[[nodiscard]] gameaction_t DoQuickload() noexcept;

#endif  // #if PSYDOOM_MODS

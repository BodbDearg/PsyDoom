#pragma once

#if !PSYDOOM_MODS

#include <cstdint>

extern padbuttons_t*    gpPlayerCtrlBindings[MAXPLAYERS];

void I_NetSendRecv() noexcept;
uint32_t I_LocalButtonsToNet(const padbuttons_t pCtrlBindings[NUM_CTRL_BINDS]) noexcept;
padbuttons_t* I_NetButtonsToLocal(const uint32_t encodedBindings) noexcept;

#endif  // !PSYDOOM_MODS

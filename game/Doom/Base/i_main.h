#pragma once

#include "Doom/doomdef.h"

struct texture_t;

// Size of the temporary buffer that is used for WAD loading and other stuff - 64 KiB
static constexpr uint32_t TMP_BUFFER_SIZE = 0x10000;

// Gamepad button binding index: these are the actions which are configurable to different buttons.
// These also must be synchronized in a network game.
enum padbinding_t {
    PAD_BINDING_ATTACK,
    PAD_BINDING_USE,
    PAD_BINDING_STRAFE,
    PAD_BINDING_SPEED,
    PAD_BINDING_STRAFE_LEFT,
    PAD_BINDING_STRAFE_RIGHT,
    PAD_BINDING_WEAPON_BACK,
    PAD_BINDING_WEAPON_FORWARD,
    NUM_PAD_BINDINGS
};

// Type for a pressed button mask.
// Certain bits correspond to certain buttons on the PSX digital controller.
//
// TODO: create constants for available button bits. 
typedef uint32_t padbuttons_t;

extern const VmPtr<std::byte[TMP_BUFFER_SIZE]>                          gTmpBuffer;
extern const VmPtr<uint32_t>                                            gTotalVBlanks;
extern const VmPtr<uint32_t>                                            gLastTotalVBlanks;
extern const VmPtr<uint32_t>                                            gElapsedVBlanks;
extern const VmPtr<uint32_t>                                            gNumFramesDrawn;
extern const VmPtr<uint32_t>                                            gCurPlayerIndex;
extern const VmPtr<uint32_t>                                            gLockedTexPagesMask;
extern const VmPtr<uint32_t>                                            gLockedTexPagesMask;
extern const VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>                      gBtnBindings;
extern const VmPtr<VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>[MAXPLAYERS]>   gpPlayerBtnBindings;

void I_Main() noexcept;
void I_PSXInit() noexcept;
[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept;
void I_ReadGamepad() noexcept;
void I_CacheTexForLumpName() noexcept;
void I_CacheAndDrawSprite() noexcept;
void I_DrawSprite() noexcept;
void I_DrawPlaque() noexcept;
void I_IncDrawnFrameCount() noexcept;
void I_DrawPresent() noexcept;
void I_VsyncCallback() noexcept;
void I_Init() noexcept;

void I_CacheTex(texture_t& tex) noexcept;
void _thunk_I_CacheTex() noexcept;

void I_RemoveTexCacheEntry() noexcept;
void I_PurgeTexCache() noexcept;
void I_VramViewerDraw() noexcept;
void I_NetSetup() noexcept;
void I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_NetSendRecv() noexcept;
void I_SubmitGpuCmds() noexcept;
void I_LocalButtonsToNet() noexcept;
void I_NetButtonsToLocal() noexcept;

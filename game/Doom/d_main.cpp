#include "d_main.h"

#include "Base/d_vsprintf.h"
#include "Base/i_file.h"
#include "Base/i_main.h"
#include "Base/i_misc.h"
#include "Base/s_sound.h"
#include "Base/w_wad.h"
#include "Base/z_zone.h"
#include "cdmaptbl.h"
#include "doomdef.h"
#include "Game/g_game.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "Renderer/r_main.h"
#include "UI/cr_main.h"
#include "UI/le_main.h"
#include "UI/m_main.h"
#include "UI/st_main.h"
#include "UI/ti_main.h"
#include "Wess/psxsnd.h"

// The current number of 60 Hz ticks
const VmPtr<int32_t> gTicCon(0x8007814C);

// Pointer to a buffer holding the demo
const VmPtr<VmPtr<uint32_t>> gpDemoBuffer(0x800775E8);

// Game start parameters
const VmPtr<skill_t>        gStartSkill(0x800775FC);
const VmPtr<int32_t>        gStartMapOrEpisode(0x80077600);
const VmPtr<gametype_t>     gStartGameType(0x80077604);

//------------------------------------------------------------------------------------------------------------------------------------------
// Main DOOM entry point.
// Bootstraps the engine and platform specific code and runs the game loops.
//------------------------------------------------------------------------------------------------------------------------------------------
void D_DoomMain() noexcept {
    // PlayStation specific setup
    I_PSXInit();

    a0 = (lw(0x800775F0) * 127) / 100;      // Load from: gOptionsSndVol (800775F0)
    a1 = (lw(0x800775F4) * 127) / 100;      // Load from: gOptionsMusVol (800775F4)    
    a2 = gTmpBuffer;
    PsxSoundInit();

    // Initializing standard DOOM subsystems, zone memory management, WAD, platform stuff, renderer etc.
    Z_Init();
    I_Init();
    W_Init();
    R_Init();
    ST_Init();

    // Clearing some global tick counters and inputs
    *gPrevGameTic = 0;
    *gGameTic = 0;
    *gLastTgtGameTicCount = 0;
    *gTicCon = 0;
    sw(0, 0x80077F44);          // Store to: gPlayerPadButtons[0] (80077F44)
    sw(0, 0x80077F48);          // Store to: gPlayerPadButtons[1] (80077F48)
    sw(0, 0x80078214);          // Store to: gPlayerOldPadButtons[0] (80078214)
    sw(0, 0x80078218);          // Store to: gPlayerOldPadButtons[1] (80078218)

    // TODO: PC-PSX: allow this loop to exit if the application is quit
    //
    // The main intro and demo scenes flow.
    // Continue looping until there is input and then execute the main menu until it times out.    
    while (true) {
        if (RunTitle() != ga_exit) {
            if (RunDemo(CdMapTbl_File::DEMO1_LMP) != ga_exit) {
                if (RunCredits() != ga_exit) {
                    if (RunDemo(CdMapTbl_File::DEMO2_LMP) != ga_exit)
                        continue;
                }
            }
        }
        
        do {
            RunMenu();
        } while (v0 != ga_timeout);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs a screen with scrolling legals text.
// This function is never called in the retail game!
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunLegals() noexcept {
    return MiniLoop(START_Legals, STOP_Legals, TIC_Legals, DRAW_Legals);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the title screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunTitle() noexcept {
    return MiniLoop(START_Title, STOP_Title, TIC_Title, DRAW_Title);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load and run the specified demo file.
// The maximum allowed demo size to be handled by this function is 16 KiB.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunDemo(const CdMapTbl_File file) noexcept {
    // Read the demo file contents (up to 16 KiB)
    *gpDemoBuffer = (uint32_t*) Z_EndMalloc(**gpMainMemZone, 0x4000, PU_STATIC, nullptr);
    const uint32_t openFileIdx = OpenFile(file);
    ReadFile(openFileIdx, gpDemoBuffer->get(), 0x4000);
    CloseFile(openFileIdx);
    
    // Play the demo
    G_PlayDemoPtr();
    const gameaction_t exitAction = (gameaction_t) v0;
    
    // Free the demo buffer and return the exit action
    Z_Free2(**gpMainMemZone, gpDemoBuffer->get());
    return exitAction;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunCredits() noexcept {
    return MiniLoop(START_Credits, STOP_Credits, TIC_Credits, DRAW_Credits);
}

void I_SetDebugDrawStringPos() noexcept {
    sw(a0, gp + 0xA50);                                 // Store to: gDebugDrawStringXPos (80078030)
    sw(a1, gp + 0xA5C);                                 // Store to: gDebugDrawStringYPos (8007803C)
}

void I_DebugDrawString(const char* const fmtMsg, ...) noexcept {
    sp -= 0x120;
    sw(s0, sp + 0x118);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    sw(a0, sp + 0x120);
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lhu(a3 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x11C);
    sw(0, sp + 0x10);
    LIBGPU_SetDrawMode();
    s0 += 4;                                            // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80012578:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_800125E0;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_800126A4;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_800125E0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80012694;
    if (v1 == a0) goto loc_80012578;
loc_80012604:
    v0 = lw(gp + 0x4);                                  // Load from: GPU_REG_GP1 (800775E4)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80012578;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t3;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_80012670;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80012654:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp);                                        // Load from: GPU_REG_GP0 (800775E0)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80012654;
loc_80012670:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80012578;
    goto loc_80012604;
loc_80012694:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800126A4:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80012704;
    v1 = -1;                                            // Result = FFFFFFFF
loc_800126EC:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_800126EC;
loc_80012704:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_800127B8;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80012730:
    v0 = lw(gp + 0x4);                                  // Load from: GPU_REG_GP1 (800775E4)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_800127B8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_8001279C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80012780:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp);                                        // Load from: GPU_REG_GP0 (800775E0)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80012780;
loc_8001279C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80012730;
loc_800127B8:
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    LIBGPU_SetSprt();
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    LIBGPU_SetSemiTrans();
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    LIBGPU_SetShadeTex();
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F7C);                              // Load from: gPaletteClutId_Main (800A9084)
    v0 = 0x80;                                          // Result = 00000080
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    
    {
        va_list args;
        va_start(args, fmtMsg);        
        v0 = D_vsprintf(vmAddrToPtr<char>(sp + 0x18), fmtMsg, args);
        va_end(args);
    }

    a0 = lw(gp + 0xA50);                                // Load from: gDebugDrawStringXPos (80078030)
    a1 = lw(gp + 0xA5C);                                // Load from: gDebugDrawStringYPos (8007803C)
    a2 = sp + 0x18;
    I_DrawStringSmall();
    v0 = lw(gp + 0xA5C);                                // Load from: gDebugDrawStringYPos (8007803C)
    v0 += 8;
    sw(v0, gp + 0xA5C);                                 // Store to: gDebugDrawStringYPos (8007803C)
    ra = lw(sp + 0x11C);
    s0 = lw(sp + 0x118);
    sp += 0x120;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set a region of memory to a specified byte value.
// Bulk writes in 32-byte chunks where possible.
//------------------------------------------------------------------------------------------------------------------------------------------
void D_memset(void* const pDst, const std::byte fillByte, const uint32_t count) noexcept {
    // Fill up until the next aligned 32-bit address
    uint32_t bytesLeft = count;
    std::byte* pDstByte = (std::byte*) pDst;

    while (((uintptr_t) pDstByte & 3) != 0) {
        if (bytesLeft == 0)
            return;
        
        *pDstByte = fillByte;
        ++pDstByte;
        --bytesLeft;
    }

    // Fill 32 bytes at a time (with 8 writes)
    {
        const uint32_t fb32 = (uint32_t) fillByte;
        const uint32_t fillWord = (fb32 << 24) | (fb32 << 16) | (fb32 << 8) | fb32;
    
        while (bytesLeft >= 32) {
            uint32_t* const pDstWords = (uint32_t*) pDstByte;

            pDstWords[0] = fillWord;
            pDstWords[1] = fillWord;
            pDstWords[2] = fillWord;
            pDstWords[3] = fillWord;
            pDstWords[4] = fillWord;
            pDstWords[5] = fillWord;
            pDstWords[6] = fillWord;
            pDstWords[7] = fillWord;
            
            pDstByte += 32;
            bytesLeft -= 32;
        }
    }

    // Fill the remaining bytes
    while (bytesLeft != 0) {
        *pDstByte = fillByte;
        bytesLeft--;
        pDstByte++;
    }
}

void _thunk_D_memset() noexcept {
    VmPtr<std::byte> pDst(a0);
    D_memset(pDst.get(), (std::byte) a1, a2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy a number of bytes from source to destination
//------------------------------------------------------------------------------------------------------------------------------------------
void D_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept {
    uint32_t bytesLeft = numBytes;
    std::byte* pDstByte = (std::byte*) pDst;
    const std::byte* pSrcByte = (const std::byte*) pSrc;

    while (bytesLeft != 0) {
        bytesLeft--;
        *pDstByte = *pSrcByte;
        ++pSrcByte;
        ++pDstByte;
    }
}

void _thunk_D_memcpy() noexcept {
    VmPtr<std::byte> pDst = a0;
    VmPtr<std::byte> pSrc = a1;
    D_memcpy(pDst.get(), pSrc.get(), a2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copy up to 'maxChars' from 'src' to 'dst'
//------------------------------------------------------------------------------------------------------------------------------------------
void D_strncpy(char* dst, const char* src, uint32_t maxChars) noexcept {
    while (maxChars != 0) {
        const char c = *dst = *src;
        --maxChars;
        ++src;
        ++dst;

        if (c == 0)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compare two strings, up to 'maxCount' characters.
// Return '0' if equal or '1' if not equal.
// Confusingly, unlike the equivalent standard C function, this comparison is *NOT* case insensitive.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t D_strncasecmp(const char* str1, const char* str2, int32_t maxCount) noexcept {
    while (*str1 && *str2) {
        if (*str1 != *str2)
            return 1;
        
        ++str1;
        ++str2;
        --maxCount;
        
        // Bug fix: if the function is called with 'maxCount' as '0' for some reason then prevent a near infinite loop
        // due to wrapping around to '-1'. I don't think this happened in practice but just guard against it here anyway
        // in case future mods happen to trigger this issue...
        #if PC_PSX_DOOM_MODS
            if (maxCount <= 0)
                return 0;
        #else
            if (maxCount == 0)
                return 0;
        #endif
    }

    return (*str1 == *str2);
}

void _thunk_D_strncasecmp() noexcept {
    v0 = D_strncasecmp(vmAddrToPtr<const char>(a0), vmAddrToPtr<const char>(a1), (int32_t) a2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the given ASCII string uppercase
//------------------------------------------------------------------------------------------------------------------------------------------
void strupr(char* str) noexcept {
    for (char c = *str; c != 0; c = *str) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }

        *str = c;
        ++str;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the game loop for a menu screen or for the level gameplay.
// Calls startup/shutdown functions and drawer/ticker functions.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t MiniLoop(
    void (*const pStart)(),
    void (*const pStop)(),
    void (*const pTicker)(),
    void (*const pDrawer)()
) noexcept {
    // Network initialization
    if (*gNetGame != gt_single) {
        I_NetHandshake();
    }

    // Init timers and exit action
    *gGameAction = ga_nothing;
    *gPrevGameTic = 0;
    *gGameTic = 0;
    *gTicCon = 0;
    *gLastTgtGameTicCount = 0;

    // Run startup logic for this game loop beginning
    pStart();

    // Update the video refresh timers
    *gLastTotalVBlanks = LIBETC_VSync(-1);
    *gElapsedVBlanks = 0;

    // Continue running the game loop until something causes us to exit
    gameaction_t exitAction = ga_nothing;

    while (true) {
        v0 = lw(0x80077618);        // Load from: gCurPlayerIndex (80077618)
        at = 0x80077FBC;            // Result = gPlayersElapsedVBlanks[0] (80077FBC)
        at += v0 * 4;
        sw(*gElapsedVBlanks, at);

        v0 = lw(0x80077F44);        // Load from: gPlayerPadButtons[0] (80077F44)
        v1 = lw(0x80077F48);        // Load from: gPlayerPadButtons[1] (80077F48)
        sw(v0, 0x80078214);         // Store to: gPlayerOldPadButtons[0] (80078214)
        sw(v1, 0x80078218);         // Store to: gPlayerOldPadButtons[1] (80078218)
        
        I_ReadGamepad();

        a0 = v0;
        v1 = lw(0x80077618);        // Load from: gCurPlayerIndex (80077618)
        v1 <<= 2;

        uint32_t x4 = 0x80077F44;   // Result = gPlayerPadButtons[0] (80077F44)
        a1 = v1 + x4;
        sw(a0, a1);

        if (*gNetGame != gt_single) {
            // Updates for when we are in a networked game.
            // Abort from the game also if there is a problem.
            I_NetUpdate();

            if (v0 != 0) {
                *gGameAction = ga_warped;
                exitAction = ga_warped;
                break;
            }
        }
        else if (*gbDemoRecording || *gbDemoPlayback) {
            // Demo recording or playback.
            // Need to either read inputs from or save them to a buffer.
            v0 = a0 & 0xF9FF;

            if (*gbDemoPlayback) {
                exitAction = ga_exit;

                if (v0 != 0)
                    break;

                v1 = lw(0x800775EC);        // Load from: gpDemo_p (800775EC)
                v0 = v1 + 4;
                sw(v0, 0x800775EC);         // Store to: gpDemo_p (800775EC)
                a0 = lw(v1);
                sw(a0, a1);
            }

            v0 = a0 & 0x800;

            if (*gbDemoRecording) {
                v1 = lw(0x800775EC);        // Load from: gpDemo_p (800775EC)
                v0 = v1 + 4;
                sw(v0, 0x800775EC);         // Store to: gpDemo_p (800775EC)
                sw(a0, v1);
                v0 = a0 & 0x800;
            }

            exitAction = ga_exitdemo;

            if (v0 != 0)
                break;
            
            // Is the demo recording too big or are we at the end of the largest possible demo size?
            // If so then stop right now...
            v0 = lw(0x800775EC);    // Load from: gpDemo_p (800775EC)
            v1 = lw(0x800775E8);    // Load from: gpDemoBuffer (800775E8)

            const int32_t demoTicksElapsed = (v0 - v1) / sizeof(uint32_t);

            if (demoTicksElapsed >= MAX_DEMO_TICKS)
                break;
        }

        // Advance the number of 60 Hz ticks passed
        v0 = *gTicCon;
        v1 = lw(0x80077FBC);        // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
        v0 += v1;
        *gTicCon = v0;
        
        // Advance to the next game tick if it is time.
        // Video refreshes at 60 Hz but the game ticks at 15 Hz:
        const int32_t tgtGameTicCount = i32(v0) / 4;
        
        if (*gLastTgtGameTicCount < tgtGameTicCount) {
            *gLastTgtGameTicCount = tgtGameTicCount;
            *gGameTic += 1;
        }
        
        // Call the ticker function to do updates for the frame
        pTicker();
        exitAction = (gameaction_t) v0;

        if (exitAction != ga_nothing)
            break;

        // Call the drawer function to do drawing for the frame
        pDrawer();
        
        // Do we need to update sound? (sound updates at 15 Hz)
        if (*gPrevGameTic < *gGameTic) {
            S_UpdateSounds();
        }

        *gPrevGameTic = *gGameTic;
    }
    
    // Run cleanup logic for this game loop ending
    a0 = exitAction;
    pStop();

    // Current pad buttons become the old ones
    v1 = lw(0x80077F44);        // Load from: gPlayerPadButtons[0] (80077F44)
    sw(v1, 0x80078214);         // Store to: gPlayerOldPadButtons[0] (80078214)

    a0 = lw(0x80077F48);        // Load from: gPlayerPadButtons[1] (80077F48)
    sw(a0, 0x80078218);         // Store to: gPlayerOldPadButtons[1] (80078218)

    // Return the exit game action
    return exitAction;
}

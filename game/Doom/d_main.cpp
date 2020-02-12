#include "d_main.h"

#include "Base/d_vsprintf.h"
#include "Base/i_drawcmds.h"
#include "Base/i_file.h"
#include "Base/i_main.h"
#include "Base/i_misc.h"
#include "Base/s_sound.h"
#include "Base/w_wad.h"
#include "Base/z_zone.h"
#include "cdmaptbl.h"
#include "doomdef.h"
#include "Game/g_game.h"
#include "Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "Renderer/r_data.h"
#include "Renderer/r_main.h"
#include "UI/cr_main.h"
#include "UI/le_main.h"
#include "UI/m_main.h"
#include "UI/o_main.h"
#include "UI/st_main.h"
#include "UI/ti_main.h"
#include "Wess/psxsnd.h"
#include <cstdio>

// The current number of 60Hz ticks
const VmPtr<int32_t> gTicCon(0x8007814C);

// The number of elapsed vblanks (60Hz) for all players
const VmPtr<int32_t[MAXPLAYERS]> gPlayersElapsedVBlanks(0x80077FBC);

// Pointer to a buffer holding the demo and the current pointer within the buffer for playback/recording
const VmPtr<VmPtr<uint32_t>> gpDemoBuffer(0x800775E8);
const VmPtr<VmPtr<uint32_t>> gpDemo_p(0x800775EC);

// Game start parameters
const VmPtr<skill_t>        gStartSkill(0x800775FC);
const VmPtr<int32_t>        gStartMapOrEpisode(0x80077600);
const VmPtr<gametype_t>     gStartGameType(0x80077604);

// Net games: set if the connection attempt was aborted
const VmPtr<bool32_t>   gbDidAbortGame(0x80077C0C);

// Debug draw string position
const VmPtr<int32_t>    gDebugDrawStringXPos(0x80078030);
const VmPtr<int32_t>    gDebugDrawStringYPos(0x8007803C);

//------------------------------------------------------------------------------------------------------------------------------------------
// Main DOOM entry point.
// Bootstraps the engine and platform specific code and runs the game loops.
//------------------------------------------------------------------------------------------------------------------------------------------
void D_DoomMain() noexcept {
    // PlayStation specific setup
    I_PSXInit();

    a0 = ((*gOptionsSndVol) * 127) / 100;
    a1 = ((*gOptionsMusVol) * 127) / 100;
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

    for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gTicButtons[playerIdx] = 0;
        gOldTicButtons[playerIdx] = 0;
    }

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
        
        while (true) {
            if (RunMenu() == ga_timeout)
                break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs a screen with scrolling legals text.
// This function is never called in the retail game, but was used for the PSX DOOM demo build.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunLegals() noexcept {
    return MiniLoop(START_Legals, _thunk_STOP_Legals, _thunk_TIC_Legals, DRAW_Legals);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the title screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunTitle() noexcept {
    return MiniLoop(START_Title, _thunk_STOP_Title, _thunk_TIC_Title, DRAW_Title);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load and run the specified demo file.
// The maximum allowed demo size to be handled by this function is 16 KiB.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunDemo(const CdMapTbl_File file) noexcept {
    // Read the demo file contents (up to 16 KiB)
    *gpDemoBuffer = (uint32_t*) Z_EndMalloc(**gpMainMemZone, 16 * 1024, PU_STATIC, nullptr);
    const uint32_t openFileIdx = OpenFile(file);
    ReadFile(openFileIdx, gpDemoBuffer->get(), 16 * 1024);
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
    return MiniLoop(START_Credits, _thunk_STOP_Credits, _thunk_TIC_Credits, DRAW_Credits);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the text position for the debug draw string
//------------------------------------------------------------------------------------------------------------------------------------------
void I_SetDebugDrawStringPos(const int32_t x, const int32_t y) noexcept {
    *gDebugDrawStringXPos = x;
    *gDebugDrawStringYPos = y;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the debug draw string.
// The string also scrolls down the screen with repeated calls.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DebugDrawString(const char* const fmtMsg, ...) noexcept {
    // Setup the drawing mode
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);

        // PC-PSX: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that
        #if PC_PSX_DOOM_MODS
            RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);
        #endif
    }

    // Setting up some sprite primitive stuff for the 'draw string' call that follows
    {
        SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

        LIBGPU_SetSprt(spritePrim);
        LIBGPU_SetSemiTrans(&spritePrim, false);
        LIBGPU_SetShadeTex(&spritePrim, false);
        LIBGPU_setRGB0(spritePrim, 128, 128, 128);    
        spritePrim.clut = gPaletteClutIds[MAINPAL];
    }
    
    // Format the message and print
    char msgBuffer[256];

    {
        va_list args;
        va_start(args, fmtMsg);

        // PC-PSX: Use 'vsnprint' as it's safer!
        #if PC_PSX_DOOM_MODS
            std::vsnprintf(msgBuffer, C_ARRAY_SIZE(msgBuffer), fmtMsg, args);
        #else
            D_vsprintf(msgBuffer, fmtMsg, args);
        #endif

        va_end(args);
    }

    I_DrawStringSmall(*gDebugDrawStringXPos, *gDebugDrawStringYPos, msgBuffer);

    // The message scrolls down the screen as it is drawn more
    *gDebugDrawStringYPos += 8;
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
        // Update timing and buttons
        gPlayersElapsedVBlanks[*gCurPlayerIndex] = *gElapsedVBlanks;

        for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            gOldTicButtons[playerIdx] = gTicButtons[playerIdx];
        }
        
        // Read pad inputs and save as the current pad buttons (overwritten if a demo)
        I_ReadGamepad();
        uint32_t padBtns = v0;
        gTicButtons[*gCurPlayerIndex] = padBtns;

        if (*gNetGame != gt_single) {
            // Updates for when we are in a networked game.
            // Abort from the game also if there is a problem.
            I_NetUpdate();
            bool exitGame = (v0 != 0);

            if (exitGame) {
                *gGameAction = ga_warped;
                exitAction = ga_warped;
                break;
            }
        }
        else if (*gbDemoRecording || *gbDemoPlayback) {
            // Demo recording or playback.
            // Need to either read inputs from or save them to a buffer.
            if (*gbDemoPlayback) {
                // Demo playback: any button pressed on the gamepad will abort
                exitAction = ga_exit;

                if (padBtns & PAD_ANY)
                    break;

                // Read inputs from the demo buffer and advance the demo.
                // N.B: Demo inputs override everything else from here on in.
                padBtns = (*gpDemo_p)[0];
                gTicButtons[*gCurPlayerIndex] = padBtns;
                *gpDemo_p += 1;
            }
            else {
                // Demo recording: record pad inputs to the buffer
                (*gpDemo_p)[0] = padBtns;
                *gpDemo_p += 1;
            }

            // Abort demo recording?
            exitAction = ga_exitdemo;

            if (padBtns & PAD_START)
                break;
            
            // Is the demo recording too big or are we at the end of the largest possible demo size?
            // If so then stop right now...
            const int32_t demoTicksElapsed = (int32_t)(gpDemo_p->get() - gpDemoBuffer->get());
            
            if (demoTicksElapsed >= MAX_DEMO_TICKS)
                break;
        }

        // Advance the number of 60 Hz ticks passed
        *gTicCon += gPlayersElapsedVBlanks[0];
        
        // Advance to the next game tick if it is time.
        // Video refreshes at 60 Hz but the game ticks at 15 Hz:
        const int32_t tgtGameTicCount = *gTicCon / 4;
        
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
    for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gOldTicButtons[playerIdx] = gTicButtons[playerIdx];
    }

    // Return the exit game action
    return exitAction;
}

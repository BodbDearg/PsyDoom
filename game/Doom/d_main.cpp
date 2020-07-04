#include "d_main.h"

#include "Base/d_vsprintf.h"
#include "Base/i_file.h"
#include "Base/i_main.h"
#include "Base/i_misc.h"
#include "Base/s_sound.h"
#include "Base/w_wad.h"
#include "Base/z_zone.h"
#include "cdmaptbl.h"
#include "Game/g_game.h"
#include "Game/p_tick.h"
#include "PcPsx/FatalErrors.h"
#include "PcPsx/FileUtils.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxPadButtons.h"
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

#include <cstdio>
#include <memory>

// The current number of 60Hz ticks
int32_t gTicCon;

// The number of elapsed vblanks (60Hz) for all players
int32_t gPlayersElapsedVBlanks[MAXPLAYERS];

// PC-PSX: networking - what amount of elapsed vblanks we told the other player we will simulate next
#if PC_PSX_DOOM_MODS
    int32_t gNextPlayerElapsedVBlanks;
#endif

// Pointer to a buffer holding the demo and the current pointer within the buffer for playback/recording
uint32_t* gpDemoBuffer;
uint32_t* gpDemo_p;

// Game start parameters
skill_t     gStartSkill         = sk_medium;
int32_t     gStartMapOrEpisode  = 1;
gametype_t  gStartGameType      = gt_single;

// Net games: set if a network game being started was aborted
bool gbDidAbortGame = false;

#if PC_PSX_DOOM_MODS
    bool        gbIsFirstTick;          // Set to 'true' for the very first tick only, 'false' thereafter
    bool        gbKeepInputEvents;      // Ticker request: if true then don't consume input events after invoking the current ticker in 'MiniLoop'
    uint32_t*   gpDemoBufferEnd;        // PC-PSX: save the end pointer for the buffer, so we know when to end the demo; do this instead of hardcoding the end
#endif

// Debug draw string position
static int32_t gDebugDrawStringXPos;
static int32_t gDebugDrawStringYPos;

//------------------------------------------------------------------------------------------------------------------------------------------
// Main DOOM entry point.
// Bootstraps the engine and platform specific code and runs the game loops.
//------------------------------------------------------------------------------------------------------------------------------------------
void D_DoomMain() noexcept {
    // PlayStation specific setup
    I_PSXInit();

    // PC-PSX: Sound init: allocate a buffer big enough to hold the WMD file (as it is on disk) temporarily.
    // The original PSX Doom used the 64 KiB static 'temp' buffer for this purpose; Final Doom did a temp 'Z_EndMalloc' of 122,880 bytes
    // because it's WMD file was much bigger. This method is more flexible and will allow for practically any sized WMD.
    #if PC_PSX_DOOM_MODS
    {
        const int32_t wmdFileSize = psxcd_get_file_size(CdFileId::DOOMSND_WMD);
        std::unique_ptr<std::byte[]> wmdFileBuffer(new std::byte[wmdFileSize]);
        PsxSoundInit(doomToWessVol(gOptionsSndVol), doomToWessVol(gOptionsMusVol), wmdFileBuffer.get());
    }
    #else
        PsxSoundInit(doomToWessVol(gOptionsSndVol), doomToWessVol(gOptionsMusVol), gTmpBuffer);
    #endif

    // Initializing standard DOOM subsystems, zone memory management, WAD, platform stuff, renderer etc.
    Z_Init();
    I_Init();
    W_Init();
    R_Init();
    ST_Init();

    // Clearing some global tick counters and inputs
    gPrevGameTic = 0;
    gGameTic = 0;
    gLastTgtGameTicCount = 0;
    gTicCon = 0;

    #if PC_PSX_DOOM_MODS
        for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            gTickInputs[playerIdx] = {};
            gOldTickInputs[playerIdx] = {};
        }

        gNextTickInputs = {};
        gTicButtons = 0;
        gOldTicButtons = 0;
    #else
        for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            gTicButtons[playerIdx] = 0;
            gOldTicButtons[playerIdx] = 0;
        }
    #endif

    // PC-PSX: play a single demo file and exit if commanded.
    // Also, if in headless mode then don't run the main game - only single demo playback is allowed.
    #if PC_PSX_DOOM_MODS
        if (ProgArgs::gPlayDemoFilePath[0]) {
            RunDemoAtPath(ProgArgs::gPlayDemoFilePath);
            return;
        }

        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    // The main intro and demo scenes flow.
    // Continue looping until there is input and then execute the main menu until it times out.
    while (true) {
        if (RunTitle() != ga_exit) {
            if (RunDemo(CdFileId::DEMO1_LMP) != ga_exit) {
                if (RunCredits() != ga_exit) {
                    if (RunDemo(CdFileId::DEMO2_LMP) != ga_exit)
                        continue;
                }
            }
        }
        
        while (true) {
            // Go back to the title screen if timing out
            const gameaction_t result = RunMenu();

            if (result == ga_timeout)
                break;

            // PC-PSX: quit the application entirely if requested
            #if PC_PSX_DOOM_MODS
                if (result == ga_quitapp)
                    return;
            #endif
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs a screen with scrolling legals text.
// This function is never called in the retail game, but was used for the PSX DOOM demo build.
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
gameaction_t RunDemo(const CdFileId file) noexcept {
    // PC-PSX: ensure this required graphic is loaded before starting the demo
    #if PC_PSX_DOOM_MODS
        if (gTex_LOADING.texPageId == 0) {
            I_LoadAndCacheTexLump(gTex_LOADING, "LOADING", 0);
        }
    #endif

    // Open the demo file
    const uint32_t openFileIdx = OpenFile(file);

    // PC-PSX: determine the file size to read and only read the actual size of the demo rather than assuming it's 16 KiB.
    // Also allocate the demo buffer on the native host heap, so as to allow very large demos without affecting zone memory.
    #if PC_PSX_DOOM_MODS
        const int32_t demoFileSize = SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::END);

        std::unique_ptr<uint8_t[]> pDemoBuffer(new uint8_t[demoFileSize]);
        gpDemoBuffer = (uint32_t*) pDemoBuffer.get();
        gpDemoBufferEnd = (uint32_t*)(pDemoBuffer.get() + demoFileSize);

        SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::SET);
        ReadFile(openFileIdx, gpDemoBuffer, demoFileSize);
    #else
        // Read the demo file contents (up to 16 KiB)
        constexpr uint32_t DEMO_BUFFER_SIZE = 16 * 1024;
        gpDemoBuffer = (uint32_t*) Z_EndMalloc(*gpMainMemZone, DEMO_BUFFER_SIZE, PU_STATIC, nullptr);
        ReadFile(openFileIdx, gpDemoBuffer->get(), 16 * 1024);
    #endif
    
    CloseFile(openFileIdx);
    
    // Play the demo, free the demo buffer and return the exit action
    const gameaction_t exitAction = G_PlayDemoPtr();

    // PC-PSX: the demo buffer is no longer allocated through the zone memory system; std::unique_ptr will also cleanup after itself
    #if !PC_PSX_DOOM_MODS
        Z_Free2(*gpMainMemZone, gpDemoBuffer);
    #endif

    return exitAction;
}

#if PC_PSX_DOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PC-PSX: load and run the specified demo file at the specified path on the host machine
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunDemoAtPath(const char* const filePath) noexcept {
    // Ensure this required graphic is loaded before starting the demo
    if (gTex_LOADING.texPageId == 0) {
        I_LoadAndCacheTexLump(gTex_LOADING, "LOADING", 0);
    }

    // Read the demo file into memory
    const FileData fileData = FileUtils::getContentsOfFile(filePath);

    if (!fileData.bytes) {
        FatalErrors::raiseF("Unable to read demo file '%s'! Is the file path valid?", filePath);
    }

    // Setup the demo buffers and play the demo file
    gpDemoBuffer = (uint32_t*) fileData.bytes.get();
    gpDemoBufferEnd = (uint32_t*)(fileData.bytes.get() + fileData.size);

    const gameaction_t exitAction = G_PlayDemoPtr();

    // Cleanup after we are done and return the exit action
    gpDemoBuffer = nullptr;
    gpDemoBufferEnd = nullptr;

    return exitAction;
}
#endif  // #if PC_PSX_DOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunCredits() noexcept {
    return MiniLoop(START_Credits, STOP_Credits, TIC_Credits, DRAW_Credits);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the text position for the debug draw string
//------------------------------------------------------------------------------------------------------------------------------------------
void I_SetDebugDrawStringPos(const int32_t x, const int32_t y) noexcept {
    gDebugDrawStringXPos = x;
    gDebugDrawStringYPos = y;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the debug draw string.
// The string also scrolls down the screen with repeated calls.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DebugDrawString(const char* const fmtMsg, ...) noexcept {
    // Some UI elements are handled differently for Final Doom
    const bool bIsFinalDoom = (Game::gGameType == GameType::FinalDoom);

    // Setup the drawing mode
    {
        DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);

        // PC-PSX: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that
        #if PC_PSX_DOOM_MODS
            RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, nullptr);
        #endif
    }

    // Setting up some sprite primitive stuff for the 'draw string' call that follows
    {
        SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

        LIBGPU_SetSprt(spritePrim);
        LIBGPU_SetSemiTrans(&spritePrim, false);
        LIBGPU_SetShadeTex(&spritePrim, false);
        LIBGPU_setRGB0(spritePrim, 128, 128, 128);
        spritePrim.clut = gPaletteClutIds[(bIsFinalDoom) ? UIPAL : MAINPAL];
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

    I_DrawStringSmall(gDebugDrawStringXPos, gDebugDrawStringYPos, msgBuffer);

    // The message scrolls down the screen as it is drawn more
    gDebugDrawStringYPos += 8;
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the given ASCII string uppercase
//------------------------------------------------------------------------------------------------------------------------------------------
void D_strupr(char* str) noexcept {
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
    void (*const pStop)(const gameaction_t exitAction),
    gameaction_t (*const pTicker)(),
    void (*const pDrawer)()
) noexcept {
    // Network initialization
    if (gNetGame != gt_single) {
        I_NetHandshake();
    }

    // Init timers and exit action
    gGameAction = ga_nothing;
    gPrevGameTic = 0;
    gGameTic = 0;
    gTicCon = 0;
    gLastTgtGameTicCount = 0;

    #if PC_PSX_DOOM_MODS
        gbIsFirstTick = true;
        Input::consumeEvents();     // Clear any input events leftover
    #endif

    // Run startup logic for this game loop beginning
    pStart();

    // Update the video refresh timers.
    // PC-PSX: use 'I_GetTotalVBlanks' because it can adjust time in networked games.
    #if PC_PSX_DOOM_MODS
        gLastTotalVBlanks = I_GetTotalVBlanks();
    #else
        gLastTotalVBlanks = LIBETC_VSync(-1);
    #endif

    gElapsedVBlanks = 0;

    // Continue running the game loop until something causes us to exit
    gameaction_t exitAction = ga_nothing;

    while (true) {
        // PC-PSX: initially assume no elasped vblanks for all players until found otherwise.
        // For net games we should get some elapsed vblanks from the other player in their packet, if it's time to read a new packet.
        // It will be time to read a new packet if we update inputs and timing.
        #if PC_PSX_DOOM_MODS
            for (int32_t i = 0; i < MAXPLAYERS; ++i) {
                gPlayersElapsedVBlanks[i] = 0;
            }
        #endif

        // Update timing and buttons.
        // PC-PSX: only do if enough time has elapsed or if it's the first frame, due to potentially uncapped framerate.
        gPlayersElapsedVBlanks[gCurPlayerIndex] = gElapsedVBlanks;

        #if PC_PSX_DOOM_MODS
            const bool bUpdateInputsAndTiming = ((gElapsedVBlanks > 0) || gbIsFirstTick);
        #else
            const bool bUpdateInputsAndTiming = true;
        #endif

        if (bUpdateInputsAndTiming) {
            // Read pad inputs and save as the current pad buttons (note: overwritten if a demo); also save old inputs for button just pressed detection.
            // PC-PSX: read tick inputs in addition to raw gamepad inputs, this is now the primary input source.
            #if PC_PSX_DOOM_MODS
                for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
                    gOldTickInputs[playerIdx] = gTickInputs[playerIdx];
                }

                gOldTicButtons = gTicButtons;

                TickInputs& tickInputs = gTickInputs[gCurPlayerIndex];
                P_GatherTickInputs(tickInputs);
                gTicButtons = I_ReadGamepad();
            #else
                for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
                    gOldTicButtons[playerIdx] = gTicButtons[playerIdx];
                }

                uint32_t padBtns = I_ReadGamepad();
                gTicButtons[gCurPlayerIndex] = padBtns;
            #endif

            if (gNetGame != gt_single) {
                // Updates for when we are in a networked game: abort from the game also if there is a problem
                const bool bNetError = I_NetUpdate();

                if (bNetError) {
                    // PC-PSX: if a network error occurs don't try to restart the level, the connection is most likely still gone.
                    // Exit to the main menu instead.
                    #if PC_PSX_DOOM_MODS
                        gGameAction = ga_exitdemo;
                        exitAction = ga_exitdemo;
                    #else
                        gGameAction = ga_warped;
                        exitAction = ga_warped;
                    #endif

                    break;
                }
            }
            else if (gbDemoRecording || gbDemoPlayback) {
                // Had to move the demo pointer increment and end of demo check to here to work with uncapped framerates.
                // The demo pointer is now also only incremented whenever actual 'vblanks' are registered as elapsed, which
                // occurs when the required interval (15 Hz tick for demos, 30 Hz tick for normal gameplay) has elapsed.
                #if PC_PSX_DOOM_MODS
                    if (gElapsedVBlanks > 0) {
                        gpDemo_p++;

                        // Note: use a pointer to the end of the demo buffer to tell if the demo has ended for PC-PSX.
                        // Don't assume the demo buffer is a fixed size, this allows us to work with demos of any size.
                        // Also, the last tick of the demo does not get executed, hence +1...
                        if (gpDemo_p + 1 >= gpDemoBufferEnd)
                            break;
                    }
                #endif

                // Demo recording or playback.
                // Need to either read inputs from or save them to a buffer.
                if (gbDemoPlayback) {
                    // Demo playback: any button pressed on the gamepad will abort.
                    // PC-PSX: just use the menu action buttons to abort.
                    exitAction = ga_exit;

                    #if PC_PSX_DOOM_MODS
                        if (tickInputs.bMenuOk || tickInputs.bMenuBack || tickInputs.bMenuStart)
                            break;
                    #else
                        if (padBtns & PAD_ANY_BTNS)
                            break;
                    #endif

                    // Read inputs from the demo buffer and advance the demo.
                    // N.B: Demo inputs override everything else from here on in.
                    #if PC_PSX_DOOM_MODS
                        const uint32_t padBtns = *gpDemo_p;
                        gTicButtons = padBtns;
                        P_PsxButtonsToTickInputs(padBtns, gCtrlBindings, gTickInputs[gCurPlayerIndex]);
                    #else
                        padBtns = *gpDemo_p;
                        gTicButtons[gCurPlayerIndex] = padBtns;
                    #endif
                }
                else {
                    // Demo recording: record pad inputs to the buffer.
                    // FIXME: need to implement a new demo format to support analog movements etc. - this won't work.
                    #if PC_PSX_DOOM_MODS
                        *gpDemo_p = gTicButtons;
                    #else
                        *gpDemo_p = padBtns;
                    #endif
                }

                // PC-PSX: moving the demo pointer increment to above to work with uncapped framerates
                #if !PC_PSX_DOOM_MODS
                    gpDemo_p++;
                #endif

                // Abort demo recording?
                exitAction = ga_exitdemo;

                #if PC_PSX_DOOM_MODS
                    if (gTickInputs[gCurPlayerIndex].bTogglePause)
                        break;
                #else
                    if (padBtns & PAD_START)
                        break;
                #endif
            
                // PC-PSX: moving the end of demo check to above in order to work with uncapped framerates
                #if !PC_PSX_DOOM_MODS
                    // Is the demo recording too big or are we at the end of the largest possible demo size? If so then stop right now...
                    const int32_t demoTicksElapsed = (int32_t)(gpDemo_p - gpDemoBuffer);

                    if (demoTicksElapsed >= MAX_DEMO_TICKS)
                        break;
                #endif
            }

            // Advance the number of 60 Hz ticks passed.
            // N.B: the tick count used here is ALWAYS for player 1, this is how time is kept in sync for a network game.
            gTicCon += gPlayersElapsedVBlanks[0];
        
            // Advance to the next game tick if it is time.
            // Video refreshes at 60 Hz but the game ticks at 15 Hz:
            const int32_t tgtGameTicCount = gTicCon >> VBLANK_TO_TIC_SHIFT;
        
            if (gLastTgtGameTicCount < tgtGameTicCount) {
                gLastTgtGameTicCount = tgtGameTicCount;
                gGameTic++;
            }
        }
        
        // Call the ticker function to do updates for the frame.
        // Note that I am calling this in all situations, even if the framerate is capped and if we haven't passed enough time for a game tick.
        // That allows for possible update logic which runs > 30 Hz in future, like framerate uncapped turning movement.
        exitAction = pTicker();

        if (exitAction != ga_nothing)
            break;

        // PC-PSX: done with input events after the ticker has been called.
        // Unless the ticker has requested that we hold onto them.
        #if PC_PSX_DOOM_MODS
            if (!gbKeepInputEvents) {
                Input::consumeEvents();
            } else {
                gbKeepInputEvents = false;  // Temporary request only!
            }
        #endif

        // Call the drawer function to do drawing for the frame
        pDrawer();
        
        // Do we need to update sound? (sound updates at 15 Hz)
        if (gGameTic > gPrevGameTic) {
            S_UpdateSounds();
        }

        gPrevGameTic = gGameTic;
        gbIsFirstTick = false;
    }
    
    // Run cleanup logic for this game loop ending
    pStop(exitAction);

    // Current inputs become the old ones
    #if PC_PSX_DOOM_MODS
        for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            gOldTickInputs[playerIdx] = gTickInputs[playerIdx];
        }

        gOldTicButtons = gTicButtons;
    #else
        for (uint32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            gOldTicButtons[playerIdx] = gTicButtons[playerIdx];
        }
    #endif

    // Return the exit game action
    return exitAction;
}

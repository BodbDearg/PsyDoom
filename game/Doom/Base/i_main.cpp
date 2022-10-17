#include "i_main.h"

#include "Asserts.h"
#include "d_vsprintf.h"
#include "Doom/d_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/errormenu_main.h"
#include "FatalErrors.h"
#include "i_drawcmds.h"
#include "i_texcache.h"
#include "PsyDoom/Controls.h"
#include "PsyDoom/DemoPlayer.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapHash.h"
#include "PsyDoom/Network.h"
#include "PsyDoom/PlayerPrefs.h"
#include "PsyDoom/ProgArgs.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Utils.h"
#include "PsyDoom/Video.h"
#include "PsyDoom/Vulkan/VDrawing.h"
#include "PsyDoom/Vulkan/VPlaqueDrawer.h"
#include "PsyDoom/Vulkan/VRenderer.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "PsyQ/LIBSN.h"
#include "w_wad.h"
#include "z_zone.h"

#include <algorithm>

// PsyDoom: the time at which the app started.
// Used for measuring the total vblanks elapsed since app start
#if PSYDOOM_MODS
    static std::chrono::steady_clock::time_point gAppStartTime;
#endif

// Video vblank timers: track the total amount, last total and current elapsed amount
uint32_t gTotalVBlanks;
uint32_t gLastTotalVBlanks;
uint32_t gElapsedVBlanks;

// The index of the currently displaying framebuffer, 0 or 1
uint32_t gCurDispBufferIdx;

// The draw and display environments for framebuffers 0 and 1
DISPENV gDispEnvs[2];
DRAWENV gDrawEnvs[2];

// Used to tell when the texture cache overflows.
// Each texture when added to the cache is assigned the current value of this number.
// When the time comes to evict a texture to make room for another, we check to make sure that the texture wasn't loaded in the current frame.
// If the evicted texture WAS loaded in the current frame, then it means we've run out of texture memory and can't draw all of the textures in the frame.
uint32_t gNumFramesDrawn;

// The index of the user's player in the array of players: whether you are player 1 or 2 in other words
int32_t gCurPlayerIndex;

// Original PSX Doom: this player's control bindings.
// This is only used for playback of original PSX Doom demos now, to translate PSX buttons into a 'TickInputs' structure.
// PsyDoom has it's own more flexible configuration system that supercedes this mechanism. 
padbuttons_t gCtrlBindings[NUM_CTRL_BINDS] = {
    PAD_TRIANGLE | PSX_MOUSE_LEFT,
    PAD_CIRCLE,
    PAD_CROSS | PSX_MOUSE_RIGHT,
    PAD_SQUARE,
    PAD_L1,
    PAD_R1,
    PAD_L2,
    PAD_R2,
    0,          // Move forward is default unbound
    0           // Move backward is default unbound
};

// Original PSX Doom: mouse sensitivity.
// This is only used for playback of original PSX Doom demos now.
int32_t gPsxMouseSensitivity = 50;

// Bit masks for each of the bindable buttons
const padbuttons_t gBtnMasks[NUM_BINDABLE_BTNS] = {
    PAD_TRIANGLE,       // bindablebtn_triangle
    PAD_CIRCLE,         // bindablebtn_circle
    PAD_CROSS,          // bindablebtn_cross
    PAD_SQUARE,         // bindablebtn_square
    PAD_L1,             // bindablebtn_l1
    PAD_R1,             // bindablebtn_r1
    PAD_L2,             // bindablebtn_l2
    PAD_R2,             // bindablebtn_r2
    PSX_MOUSE_LEFT,     // bindablebtn_mouse_left
    PSX_MOUSE_RIGHT     // bindablebtn_mouse_right
};

// The main UI texture atlas for the game.
// This is loaded into the 1st available texture page and kept loaded at all times after that.
texture_t gTex_STATUS;

// Loading, connecting, error etc. plaques
texture_t gTex_PAUSE;
texture_t gTex_LOADING;
texture_t gTex_NETERR;
texture_t gTex_CONNECT;

// PsyDoom: the temporary buffer can now be resized to any size.
// Start off with a plentiful 1 MiB however...
#if PSYDOOM_LIMIT_REMOVING
    ResizableBuffer gTmpBuffer(1024 * 1024);
#else
    // A 64-KiB buffer used for WAD loading and other stuff
    std::byte gTmpBuffer[TMP_BUFFER_SIZE];
#endif

#if PSYDOOM_MODS
    // Max tolerated packet delay in milliseconds: after which we start adjusting clocks
    static constexpr int32_t MAX_PACKET_DELAY_MS = 15;

    // The current network protocol version.
    // Should be incremented whenever the data format being transmitted changes, or when updates might cause differences in game behavior.
    static constexpr int32_t NET_PROTOCOL_VERSION = 29;

    // Previous game error checking value when we last sent to the other player.
    // Have to store this because we always send 1 packet ahead for the next frame.
    static uint32_t gNetPrevErrorCheck;

    // Flag set to true upon connection and cleared the first call to 'I_NetUpdate'.
    // Lets us know when we are sending the first update packet of the session.
    static bool gbNetIsFirstNetUpdate;

    // How delayed the last packet we received from the other player was.
    // This is relayed to the other peer on the next packet, so that the other player's time can be adjusted forward (if required).
    static int32_t gLastInputPacketDelayMs;

    // How much to adjust time in a networked game.
    // Used to tweak the clock to try and get both peers locked into the same time.
    int32_t gNetTimeAdjustMs;

    // Whether the game is being recorded as a demo by one of the players.
    // Affects whether pause can be used or whether it ends the recording session.
    bool gbNetIsGameBeingRecorded;
#endif

// Buffers used originally by the PsyQ SDK to store gamepad input.
// In PsyDoom they are just here for historical reference.
static uint8_t gPadInputBuffer_1[34];
static uint8_t gPadInputBuffer_2[34];

//------------------------------------------------------------------------------------------------------------------------------------------
// User/client entrypoint for PlayStation DOOM.
// This was probably the actual 'main()' function in the real source code.
// I'm just calling 'I_Main()' so as not to confuse it with this port's 'main()'...
//------------------------------------------------------------------------------------------------------------------------------------------
void I_Main() noexcept {
    #if PSYDOOM_MODS
        gAppStartTime = std::chrono::steady_clock::now();   // For measuring vblanks since app start
    #endif

    LIBSN__main();
    D_DoomMain();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does PlayStation specific hardware initialization, except for audio (which is handled by the WESS library)
//------------------------------------------------------------------------------------------------------------------------------------------
void I_PSXInit() noexcept {
    // Initialize system callbacks and interrupt handlers 
    LIBETC_ResetCallback();

    // Initialize the GPU
    LIBGPU_ResetGraph(0);
    LIBGPU_SetGraphDebug(0);

    // Initialize the gamepad
    LIBAPI_InitPAD(gPadInputBuffer_1, gPadInputBuffer_2, 34, 34);
    LIBAPI_StartPAD();
    LIBAPI_ChangeClearPAD(0);

    // Initialize the Geometry Transformation Engine (GTE) coprocessor
    LIBGTE_InitGeom();

    // These calls don't really matter for PSX DOOM since we don't do perspective projection using the GTE
    LIBGTE_SetGeomScreen(128);
    LIBGTE_SetGeomOffset(128, 100);

    // Setup the descriptors (draw and display environments) for the two framebuffers.
    // These structs specify where in VRAM the framebuffers live, clearing and dithering options and so on.
    LIBGPU_SetDefDrawEnv(gDrawEnvs[0], 0, 0, 256, 240);
    gDrawEnvs[0].isbg = true;
    gDrawEnvs[0].dtd = false;

    LIBGPU_SetDefDrawEnv(gDrawEnvs[1], 256, 0, 256, 240);
    gDrawEnvs[1].isbg = true;
    gDrawEnvs[1].dtd = false;

    LIBGPU_SetDefDispEnv(gDispEnvs[0], 256, 0, 256, 240);
    LIBGPU_SetDefDispEnv(gDispEnvs[1], 0, 0, 256, 240);

    // Start off presenting this framebuffer
    gCurDispBufferIdx = 0;

    // Not sure why interrupts were being disabled and then immediately re-enabled, perhaps to flush out some state?
    LIBAPI_EnterCriticalSection();
    LIBAPI_ExitCriticalSection();

    // PsyDoom: no longer need to setup serial I/O stuff.
    // We setup networking on demand when the user goes to start a networked game.
    #if !PSYDOOM_MODS
        // Setup serial (PlayStation link cable) communications:
        //  (1) Install the serial I/O driver.
        //  (2) Setup events for read and write done, so we can detect when these operations complete.
        //  (3) Open the serial I/O files that we use for input and output
        //  (4) Set data transmission rate (bits per second)
        //
        LIBCOMB_AddCOMB();

        gSioReadDoneEvent = LIBAPI_OpenEvent(HwSIO, EvSpIOER, EvMdNOINTR, nullptr);
        LIBAPI_EnableEvent(gSioReadDoneEvent);

        gSioWriteDoneEvent = LIBAPI_OpenEvent(HwSIO, EvSpIOEW, EvMdNOINTR, nullptr);
        LIBAPI_EnableEvent(gSioWriteDoneEvent);

        gNetOutputFd = LIBAPI_open("sio:", O_WRONLY);
        gNetInputFd = LIBAPI_open("sio:", O_RDONLY | O_NOWAIT);

        LIBCOMB_CombSetBPS(38400);
    #endif

    // Clear both draw buffers by swapping twice (clears on swap)
    I_DrawPresent();
    I_DrawPresent();

    // Enable display output
    LIBGPU_SetDispMask(1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Prints a fatal non-recoverable error to the screen using a debug font.
// The error message is formatted printf style.
//------------------------------------------------------------------------------------------------------------------------------------------
[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept {
    // Compose the message
    #if PSYDOOM_MODS
        char msgBuffer[2048];
    #else
        char msgBuffer[256];
    #endif

    {
        va_list args;
        va_start(args, fmtMsg);
        D_vsprintf(msgBuffer, fmtMsg, args);
        va_end(args);
    }

    // PsyDoom: if running in headless mode then terminate at this point rather than spinning forever
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode) {
            std::printf("I_Error: %s", msgBuffer);
            std::abort();
        }
    #endif

    // PsyDoom: I_Error now uses a simple system error dialog by default, which is more user friendly and easier to exit from.
    // The old way of hanging the app with an error message can still be built-in however, if required.
    #if PSYDOOM_USE_NEW_I_ERROR
        FatalErrors::raise(msgBuffer);
    #else
        // Finish up any current drawing
        I_DrawPresent();

        // Load the debug font to VRAM
        LIBGPU_FntLoad(960, 256);

        // Open a print stream for debug printing and make it current
        const int32_t printStreamId = LIBGPU_FntOpen(0, 0, 256, 200, false, 256);
        LIBGPU_SetDumpFnt(printStreamId);

        // Print the required string, then flush to display
        LIBGPU_FntPrint(printStreamId, "\n\n\n %s", msgBuffer);
        LIBGPU_FntFlush(printStreamId);

        // Present the resulting message
        I_DrawPresent();

        // Deliberate infinite loop...
        // The user can kill the app however by closing the window.
        while (true) {
            #if PSYDOOM_MODS
                Utils::doPlatformUpdates();

                if (Input::isQuitRequested()) {
                    // Note: shutting down PsxVm kills the app window immediately
                    PsxVm::shutdown();
                    std::abort();
                }
            #endif
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current controller buttons pressed for gamepad 1 and 2.
// The buttons for pad 1 are in the lower 16-bits, the buttons for pad 2 are in the high 16 bits.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t I_ReadGamepad() noexcept {
    return LIBETC_PadRead(0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the WAD data for the specified texture lump and populate the 'texture_t' object info.
// Also put the texture/sprite into VRAM.
// The lump can either be specified by name or by number.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_LoadAndCacheTexLump(texture_t& tex, const char* const name, int32_t lumpNum) noexcept {
    // Do a name lookup?
    if (name) {
        lumpNum = W_GetNumForName(name);
    }

    // Ensure the lump data is loaded and decompress if required.
    // PsyDoom: updates to work with the new WAD management code.
    #if PSYDOOM_MODS
        const WadLump& lump = W_CacheLumpNum(lumpNum, PU_CACHE, false);
        const void* pLumpData = lump.pCachedData;
        const bool bIsCompressed = (!lump.bIsUncompressed);
    #else
        const void* pLumpData = W_CacheLumpNum(lumpNum, PU_CACHE, false);
        const bool bIsCompressed = (!gpbIsUncompressedLump[lumpNum]);
    #endif

    if (bIsCompressed) {
        #if PSYDOOM_LIMIT_REMOVING
            // PsyDoom limit removing: temporary buffer can be any size now
            gTmpBuffer.ensureSize(lump.uncompressedSize);
            decode(pLumpData, gTmpBuffer.bytes());
            pLumpData = gTmpBuffer.bytes();
        #else
            // PsyDoom: check for buffer overflows and issue an error if we exceed the limits
            #if PSYDOOM_MODS
                if (getDecodedSize(pLumpData) > TMP_BUFFER_SIZE) {
                    I_Error("I_LoadAndCacheTexLump: lump %d size > 64 KiB!", lumpNum);
                }
            #endif

            decode(pLumpData, gTmpBuffer);
            pLumpData = gTmpBuffer;
        #endif
    }

    // Populate the basic info for the texture
    const patch_t& patch = *(const patch_t*) pLumpData;
    tex.offsetX = Endian::littleToHost(patch.offsetX);
    tex.offsetY = Endian::littleToHost(patch.offsetY);

    const int16_t patchW = Endian::littleToHost(patch.width);
    const int16_t patchH = Endian::littleToHost(patch.height);
    tex.width = patchW;
    tex.height = patchH;

    if (patchW + 15 >= 0) {
        tex.width16 = (uint8_t)((patchW + 15) / 16);
    } else {
        tex.width16 = (uint8_t)((patchW + 30) / 16);    // Weird code added by the original compiler - to handle a negative case that would never happen?
    }

    if (patchH + 15 >= 0) {
        tex.height16 = (uint8_t)((patchH + 15) / 16);
    } else {
        tex.height16 = (uint8_t)((patchH + 30) / 16);   // Weird code added by the original compiler - to handle a negative case that would never happen?
    }

    // PsyDoom: don't wipe this field, if it's already in the cache leave it there...
    #if !PSYDOOM_MODS
        tex.texPageId = 0;
    #endif

    tex.lumpNum = (uint16_t) lumpNum;
    I_CacheTex(tex);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to 'I_DrawSprite' except the image being drawn is added to VRAM first before drawing.
// Because a texture object is specified also, less parameters are required.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CacheAndDrawSprite(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept {
    I_CacheTex(tex);
    I_DrawSprite(tex.texPageId, clutId, xpos, ypos, tex.texPageCoordX, tex.texPageCoordY, tex.width, tex.height);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a sprite to the screen with the specified texture coords, position and palette.
// The sprite is drawn without any scaling or rotation, just a very basic 1:1 blit is done.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawSprite(
    const uint16_t texPageId,
    const int16_t clutId,
    const int16_t xpos,
    const int16_t ypos,
    const LibGpuUV texU,
    const LibGpuUV texV,
    const uint16_t texW,
    const uint16_t texH
) noexcept {
    // Set the drawing mode
    {
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_MODE drawModePrim = {};
        #else
            DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetDrawMode(drawModePrim, false, false, texPageId, nullptr);
        I_AddPrim(drawModePrim);
    }

    // Setup the sprite primitive and submit.
    // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state.
    #if PSYDOOM_MODS
        SPRT spritePrim = {};
    #else
        SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);
    #endif

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_setRGB0(spritePrim, 128, 128, 128);
    LIBGPU_setXY0(spritePrim, xpos, ypos);
    LIBGPU_setUV0(spritePrim, texU, texV);
    LIBGPU_setWH(spritePrim, texW, texH);
    spritePrim.clut = clutId;

    I_AddPrim(spritePrim);
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom extension/helper: same as 'I_DrawSprite' but with the ability to change the sprite's color and semi-transparency option
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawColoredSprite(
    const uint16_t texPageId,
    const int16_t clutId,
    const int16_t xpos,
    const int16_t ypos,
    const LibGpuUV texU,
    const LibGpuUV texV,
    const uint16_t texW,
    const uint16_t texH,
    const uint8_t r,
    const uint8_t g,
    const uint8_t b,
    const bool bSemiTransparent
) noexcept {
    // Set the drawing mode
    DR_MODE drawModePrim = {};
    LIBGPU_SetDrawMode(drawModePrim, false, false, texPageId, nullptr);

    I_AddPrim(drawModePrim);

    // Setup the sprite primitive and submit
    SPRT spritePrim = {};
    LIBGPU_SetSprt(spritePrim);
    LIBGPU_setRGB0(spritePrim, r, g, b);
    LIBGPU_setXY0(spritePrim, xpos, ypos);
    LIBGPU_setUV0(spritePrim, texU, texV);
    LIBGPU_setWH(spritePrim, texW, texH);
    LIBGPU_SetSemiTrans(spritePrim, bSemiTransparent);
    spritePrim.clut = clutId;

    I_AddPrim(spritePrim);
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the front buffer to the back buffer, draws a loading plaque over it and presents it to the screen.
// Useful for drawing a loading message before doing a long running load or connect operation.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawLoadingPlaque(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept {
    // PsyDoom: ignore in headless mdoe
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    #if PSYDOOM_MODS && PSYDOOM_VULKAN_RENDERER
        // PsyDoom: If we are using the Vulkan render path then delegate to that instead
        if (Video::isUsingVulkanRenderPath()) {
            VPlaqueDrawer::drawPlaque(tex, xpos, ypos, clutId);
            return;
        }
    #endif

    // Make sure the GPU is idle and copy the front buffer to the back buffer, will draw over that
    LIBGPU_DrawSync(0);
    const DISPENV& frontBuffer = gDispEnvs[gCurDispBufferIdx];
    const DISPENV& backBuffer = gDispEnvs[gCurDispBufferIdx ^ 1];
    LIBGPU_MoveImage(frontBuffer.disp, backBuffer.disp.x, backBuffer.disp.y);

    // Ensure the plaque is loaded
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    I_CacheTex(tex);

    // Draw and present the plaque
    I_DrawSprite(tex.texPageId, clutId, xpos, ypos, tex.texPageCoordX, tex.texPageCoordY, tex.width, tex.height);
    I_SubmitGpuCmds();
    I_DrawPresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Increments the 'drawn frame' counter used to track texture cache overflows
//------------------------------------------------------------------------------------------------------------------------------------------
void I_IncDrawnFrameCount() noexcept {
    gNumFramesDrawn++;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Swaps the draw and display framebuffers, causing the current frame being rendered to be displayed onscreen.
// Also does framerate limiting to 30 Hz and updates the elapsed vblank count, which feeds the game's timing system.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawPresent() noexcept {
    // Finish up all in-flight drawing commands
    LIBGPU_DrawSync(0);

    // Wait until a VBlank occurs to swap the framebuffers.
    // PsyDoom: Note: this call now does nothing as that would inferfere with frame pacing - here just for historical reference!
    LIBETC_VSync(0);

    // Swap the framebuffers
    gCurDispBufferIdx ^= 1;
    LIBGPU_PutDrawEnv(gDrawEnvs[gCurDispBufferIdx]);
    LIBGPU_PutDispEnv(gDispEnvs[gCurDispBufferIdx]);

    // PsyDoom: copy the PSX framebuffer to the display
    #if PSYDOOM_MODS
        Video::displayFramebuffer();
    #endif

    // How many vblanks there are in a demo tick
    #if PSYDOOM_MODS
        const int32_t demoTickVBlanks = (Game::gSettings.bUsePalTimings) ? 3 : VBLANKS_PER_TIC;
    #else
        const int32_t demoTickVBlanks = VBLANKS_PER_TIC;
    #endif

    // Continously poll and wait until the required number of vblanks have elapsed before continuing
    while (true) {
        // PsyDoom: use 'I_GetTotalVBlanks' because it can adjust time in networked games
        #if PSYDOOM_MODS
            const int32_t curVBlanks = I_GetTotalVBlanks();
        #else
            const int32_t curVBlanks = LIBETC_VSync(-1);
        #endif

        const int32_t elapsedVBlanks = curVBlanks - gLastTotalVBlanks;

        // PsyDoom: don't wait and spin here if doing an uncapped framerate - exit immediately. However, don't register any
        // elapsed vblanks if we haven't passed the required interval (a 30 Hz tick for normal gameplay, a 15 Hz tick for demos).
        // This prevents the game from ticking if it hasn't met the time requirements.
        #if PSYDOOM_MODS
            if (PlayerPrefs::gbUncapFramerate) {
                const int32_t minTickVBlanks = (Game::gSettings.bUseDemoTimings) ? demoTickVBlanks : 2;

                if (elapsedVBlanks < minTickVBlanks) {
                    gElapsedVBlanks = 0;
                    return;
                }
            }
        #endif

        gTotalVBlanks = curVBlanks;

        // PsyDoom: don't allow elapsed vblanks to ever exceed '4' (15 Hz frame-rate).
        // I don't think this should actually affect any logic, but I'm doing this for the benefit of the new demo recorder.
        // The new demo recorder encodes the elapsed vblanks count using just 3 bits (0-7).
        #if PSYDOOM_MODS
            gElapsedVBlanks = std::min(elapsedVBlanks, 4);
        #else
            gElapsedVBlanks = elapsedVBlanks;
        #endif

        // The original code spun in this loop to limit the framerate to 30 FPS (2 vblanks)
        if (gElapsedVBlanks >= 2)
            break;

        // PsyDoom: do platform updates (sound, window etc.) and yield some cpu since we are waiting for a bit
        #if PSYDOOM_MODS
            Utils::doPlatformUpdates();
            Utils::threadYield();
        #endif
    }

    // Further framerate limiting for demos:
    // Demo playback or recording is forced to run at 15 Hz all of the time (the game simulation rate).
    // Probably done so the simulation remains consistent!
    if (Game::gSettings.bUseDemoTimings) {
        while (gElapsedVBlanks < (uint32_t) demoTickVBlanks) {
            // PsyDoom: do platform updates (sound, window etc.) and yield some cpu since we are waiting for a bit
            #if PSYDOOM_MODS
                Utils::doPlatformUpdates();
                Utils::threadYield();
            #endif

            // PsyDoom: use 'I_GetTotalVBlanks' because it can adjust time in networked games
            #if PSYDOOM_MODS
                gTotalVBlanks = I_GetTotalVBlanks();
            #else
                gTotalVBlanks = LIBETC_VSync(-1);
            #endif

            gElapsedVBlanks = gTotalVBlanks - gLastTotalVBlanks;
        }

        gElapsedVBlanks = demoTickVBlanks;
    }

    // So we can compute the elapsed vblank amount next time round
    gLastTotalVBlanks = gTotalVBlanks;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Callback for when a vblank occurs.
// This function appears to be unused in the retail game, probably more simple to use polling instead and not deal with interrupts?
//------------------------------------------------------------------------------------------------------------------------------------------
void I_VsyncCallback() noexcept {
    gTotalVBlanks++;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// DOOM engine platform specific initialization.
// For the PSX this will just setup the texture cache.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_Init() noexcept {
    // Alloc and init the texture cache, and do a 'purge' to initialize tracking/management state.
    // PsyDoom: the purge isn't strictly necessary here anymore with the new texture cache code, keeping it mainly for historical reasons.
    #if PSYDOOM_MODS
        I_InitTexCache();
    #else
        gpTexCache = (tcache_t*) Z_Malloc(*gpMainMemZone, sizeof(tcache_t), PU_STATIC, nullptr);
        D_memset(gpTexCache, std::byte(0), sizeof(tcache_t));
    #endif

    I_PurgeTexCache();
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Does the setup for a network game: synchronizes between players, then sends the game details and control bindings.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetSetup() noexcept {
    // Establish a connection over TCP for the server and client of the game.
    // If it fails or aborted then abort the game start attempt.
    const bool bHaveNetConn = (ProgArgs::gbIsNetServer) ? Network::initForServer() : Network::initForClient();

    if (!bHaveNetConn) {
        if (!Network::gbWasInitAborted) {
            RunNetErrorMenu_FailedToConnect();
        }

        gbDidAbortGame = true;
        return;
    }

    // Player number determination: use the client/server setting
    const bool bIsPlayer1 = ProgArgs::gbIsNetServer;
    gCurPlayerIndex = (bIsPlayer1) ? 0 : 1;

    // Clear these value initially and set whether the game is being demo recorded based on the settings of this peer
    gNetPrevErrorCheck = {};
    gNetTimeAdjustMs = 0;
    gLastInputPacketDelayMs = 0;
    gbNetIsGameBeingRecorded = ProgArgs::gbRecordDemos;

    // Fill in the connect output packet; note that player 1 decides the game params, so theese are zeroed for player 2:
    NetPacket_Connect outPkt = {};
    outPkt.protocolVersion = NET_PROTOCOL_VERSION;
    outPkt.gameId = Game::gConstants.netGameId;
    outPkt.bIsDemoRecording = ProgArgs::gbRecordDemos;

    if (gCurPlayerIndex == 0) {
        outPkt.startGameType = gStartGameType;
        outPkt.startGameSkill = gStartSkill;
        outPkt.startMap = (int16_t) gStartMapOrEpisode;
    } else {
        outPkt.startGameType = {};
        outPkt.startGameSkill = {};
        outPkt.startMap = {};
    }

    // Endian correct the output packet and send
    outPkt.endianCorrect();
    Network::sendBytes(&outPkt, sizeof(outPkt));

    // If the player is the server then determine the game settings, endian correct and send to the client
    if (gCurPlayerIndex == 0) {
        // Determine the game settings to use based on config and command line arguments
        Game::getUserGameSettings(Game::gSettings);

        // Copy and endian correct the settings before sending to the client player
        GameSettings settings = Game::gSettings;
        settings.endianCorrect();
        Network::sendBytes(&settings, sizeof(settings));
    }

    // Read the input connect packet from the other player and endian correct
    NetPacket_Connect inPkt = {};
    Network::recvBytes(&inPkt, sizeof(inPkt));
    inPkt.endianCorrect();

    // Verify the network protocol version and game ids are OK - abort if not
    if ((inPkt.protocolVersion != NET_PROTOCOL_VERSION) || (inPkt.gameId != Game::gConstants.netGameId)) {
        gbDidAbortGame = true;
        RunNetErrorMenu_GameTypeOrVersionMismatch();
        return;
    }

    // If the other player is demo recording then mark the game as being recorded
    gbNetIsGameBeingRecorded = (gbNetIsGameBeingRecorded || inPkt.bIsDemoRecording);

    // Player 2 gets the details of the game from player 1 and must setup the game accordingly.
    // This includes the game settings packet sent from the server.
    if (gCurPlayerIndex != 0) {
        gStartGameType = inPkt.startGameType;
        gStartSkill = inPkt.startGameSkill;
        gStartMapOrEpisode = inPkt.startMap;

        // Read the game settings from the server, endian correct and save
        GameSettings settings = {};

        if (!Network::recvBytes(&settings, sizeof(settings))) {
            RunNetErrorMenu_FailedToConnect();
            gbDidAbortGame = true;
            return;
        }

        settings.endianCorrect();
        Game::gSettings = settings;
    }

    // One last check to see if the network connection was killed.
    // This will happen if an error occurred, and if this is the case then we should abort the connection attempt:
    if (!Network::isConnected()) {
        RunNetErrorMenu_FailedToConnect();
        gbDidAbortGame = true;
        return;
    }

    // Starting the game and the next call to I_NetUpdate will be the first:
    gbDidAbortGame = false;
    gbNetIsFirstNetUpdate = true;

    // Start requesting tick packets
    Network::requestTickPackets();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sends the packet for the current frame in a networked game and receives the packet from the other player.
// Also does error checking, to make sure that the connection is still OK.
// Returns 'true' if a network error has occurred.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
bool I_NetUpdate() noexcept {
    // Easy case: if doing demo playback of a networked game then just read the inputs from the demo.
    // Also check if the key is pressed to toggle which player is viewed.
    if (gbDemoPlayback) {
        if (Controls::isJustReleased(Controls::Binding::Toggle_ViewPlayer)) {
            gCurPlayerIndex ^= 1;
            gPlayers[gCurPlayerIndex].message = (gCurPlayerIndex == 0) ? "Viewing player 1" : "Viewing player 2";
        }

        return (!DemoPlayer::readTickInputs());
    }

    // Compute the value used for error checking.
    // Only do this while we are in the level however...
    const bool bInGame = gbIsLevelDataCached;
    uint32_t errorCheck = 0;

    if (bInGame) {
        for (int32_t i = 0; i < MAXPLAYERS; ++i) {
            mobj_t& mobj = *gPlayers[i].mo;
            errorCheck ^= mobj.x;
            errorCheck ^= mobj.y;
            errorCheck ^= mobj.z;
            errorCheck ^= mobj.angle;
            errorCheck ^= (uint32_t) MapHash::gWord1;   // Check both players are playing the same map
        }
    }

    // If it's the very first network update for this session send a dummy packet with no inputs to the other player.
    // This kick starts the sequence of always sending one packet ahead on both ends and helps to reduce lag.
    if (gbNetIsFirstNetUpdate) {
        // Populate and send the output packet
        NetPacket_Tick outPkt = {};
        outPkt.errorCheck = Endian::hostToLittle(errorCheck);
        outPkt.inputs.reset();
        Network::sendTickPacket(outPkt);

        // Make sure these are set correctly for what we expect
        gNextTickInputs.reset();
        gNextPlayerElapsedVBlanks = 0;
        gNetPrevErrorCheck = errorCheck;
    }

    // No longer the first network update
    gbNetIsFirstNetUpdate = false;

    // For networked games the current inputs become the ones we send to the other player as our NEXT move.
    // The next inputs we said we'd previously use become the CURRENT move. 
    // Inputs are sent 1 frame ahead of when they are used to help combat network latency, and we also do the same for elapsed vblanks.
    // 
    // HOWEVER, in spite of all this (using the previous instead of the current move) we preserve the current effective view angle because
    // you are now allowed to turn at any time outside of the regular 30 Hz update loop for the player.
    {
        // Note: this code assumes uncommitted turning has been rolled into this 'TickInput' prior to this function being called.
        // The only situation where this should not be the case is if the game is paused and some of the uncommitted turning is being held onto for the next tick.
        ASSERT((gPlayerUncommittedTurning == 0) || (gbGamePaused));
        const angle_t playerAngle = (bInGame) ? gPlayers[gCurPlayerIndex].mo->angle : 0;
        gPlayerNextTickViewAngle = playerAngle + gTickInputs[gCurPlayerIndex].getAnalogTurn() + gNextTickInputs.getAnalogTurn();
    }

    std::swap(gTickInputs[gCurPlayerIndex], gNextTickInputs);
    std::swap(gPlayersElapsedVBlanks[gCurPlayerIndex], gNextPlayerElapsedVBlanks);

    // Makeup the output packet including error detection bits
    NetPacket_Tick outPkt = {};
    outPkt.errorCheck = errorCheck;
    outPkt.elapsedVBlanks = gNextPlayerElapsedVBlanks;
    outPkt.inputs = gNextTickInputs;
    outPkt.lastPacketDelayMs = gLastInputPacketDelayMs;

    // Endian correct the output packet and send it
    outPkt.errorCheck = Endian::hostToLittle(outPkt.errorCheck);
    outPkt.elapsedVBlanks = Endian::hostToLittle(outPkt.elapsedVBlanks);
    outPkt.inputs.endianCorrect();
    outPkt.lastPacketDelayMs = Endian::hostToLittle(outPkt.lastPacketDelayMs);

    Network::sendTickPacket(outPkt);

    // Receive the same packet from the opposite end and see how much it was delayed.
    // Ideally it should be sitting in the packet queue ready to go, a full frame before when we need it.
    // We'll tolerate a certain amount of delay, after which the other peer must start moving it's clock forward.
    typedef std::chrono::system_clock::time_point time_point_t;

    NetPacket_Tick inPkt;
    time_point_t inPktRecvTime;
    Network::recvTickPacket(inPkt, inPktRecvTime);

    const time_point_t now = std::chrono::system_clock::now();
    const int64_t packetAgeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - inPktRecvTime).count();
    gLastInputPacketDelayMs = std::max(MAX_PACKET_DELAY_MS - (int32_t) packetAgeMs, 0);

    // Endian correct the input packet before we use it
    inPkt.errorCheck = Endian::littleToHost(inPkt.errorCheck);
    inPkt.elapsedVBlanks = Endian::littleToHost(inPkt.elapsedVBlanks);
    inPkt.inputs.endianCorrect();
    inPkt.lastPacketDelayMs = Endian::littleToHost(inPkt.lastPacketDelayMs);

    // See if the packet we received from the other player is what we expect; if it isn't then show a 'network error' message.
    // Note: we only check the 'errorCheck' field while in game.
    const bool bNetworkError = (
        (!Network::isConnected()) ||
        (gbIsLevelDataCached && (gNetPrevErrorCheck != inPkt.errorCheck))
    );

    if (bNetworkError) {
        // Uses the current image as the basis for the next frame; copy the presented framebuffer to the drawing framebuffer:
        LIBGPU_DrawSync(0);
        LIBGPU_MoveImage(
            gDispEnvs[gCurDispBufferIdx].disp,
            gDispEnvs[gCurDispBufferIdx ^ 1].disp.x,
            gDispEnvs[gCurDispBufferIdx ^ 1].disp.y
        );

        // Show the 'Network error' plaque
        #if PSYDOOM_MODS
            Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
        #endif

        I_IncDrawnFrameCount();
        I_CacheTex(gTex_NETERR);
        I_DrawSprite(
            gTex_NETERR.texPageId,
            Game::getTexPalette_NETERR(),
            84,
            109,
            gTex_NETERR.texPageCoordX,
            gTex_NETERR.texPageCoordY,
            gTex_NETERR.width,
            gTex_NETERR.height
        );

        I_SubmitGpuCmds();
        I_DrawPresent();

        // Try and do a sync handshake between the players
        I_NetHandshake();

        // Clear all inputs
        for (int32_t i = 1; i < MAXPLAYERS; ++i) {
            gTickInputs[i] = {};
            gOldTickInputs[i] = {};
        }

        gNextTickInputs = {};
        gNextPlayerElapsedVBlanks = 0;

        // PsyDoom: wait for 2 seconds so the network error can be displayed.
        // When done clear the screen so the 'loading' message displays clearly and not overlapped with the 'network error' message:
        Utils::waitForSeconds(2.0f);
        I_DrawPresent();

        // There was a network error!
        return true;
    }

    // Read and save the inputs for the other player and their elapsed vblank count.
    // Note that the vblank count for player 1 is what determines the speed of the game for both players.
    if (gCurPlayerIndex == 0) {
        gTickInputs[1] = inPkt.inputs;
        gPlayersElapsedVBlanks[1] = inPkt.elapsedVBlanks;
    } else {
        gTickInputs[0] = inPkt.inputs;
        gPlayersElapsedVBlanks[0] = inPkt.elapsedVBlanks;
    }

    // Do time adjustment: do a 1/4 adjustment each time to more gradually correct
    {
        const int32_t adjustMs = inPkt.lastPacketDelayMs / 4;
        gNetTimeAdjustMs += adjustMs;
    }

    // No network error occured, save the error checking value for verification of the other player's game state next time around.
    // Also request more tick packets to be ready for next time we want them.
    gNetPrevErrorCheck = errorCheck;
    Network::requestTickPackets();
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used to do a synchronization handshake between the two players over the serial cable.
// Now does nothing since the underlying transport protocol (TCP) guarantees reliability and packet ordering.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetHandshake() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submits any pending draw primitives in the gpu commands buffer to the GPU.
// PsyDoom: this no longer needs to do anything, since we submit all primitves to the GPU directly and execute immediately.
// For the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_SubmitGpuCmds() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: helper chrono typedefs for the code below
//------------------------------------------------------------------------------------------------------------------------------------------
typedef std::chrono::steady_clock                           appclock_t;
typedef appclock_t::time_point                              time_point_t;
typedef appclock_t::duration                                duration_t;
typedef std::chrono::duration<int64_t, std::ratio<1,  50>>  tick_50hz_t;    // A tick that fires 50 times a second (50Hz/PAL)
typedef std::chrono::duration<int64_t, std::ratio<1,  60>>  tick_60hz_t;    // A tick that fires 60 times a second (60Hz/NTSC)

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: get the total number of vblanks elapsed.
// Also takes into account the current time adjustment in networked games.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t I_GetTotalVBlanks() noexcept {
    const time_point_t now = appclock_t::now();
    const duration_t timeSinceAppStart = now - gAppStartTime;
    const duration_t netTimeAdjust = std::chrono::milliseconds((gNetGame != gt_single) ? gNetTimeAdjustMs : 0);
    const duration_t adjustedDuration = timeSinceAppStart + netTimeAdjust;

    if (Game::gSettings.bUsePalTimings) {
        return (int32_t) std::chrono::duration_cast<tick_50hz_t>(adjustedDuration).count();
    } else {
        return (int32_t) std::chrono::duration_cast<tick_60hz_t>(adjustedDuration).count();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: gets the absolute time that the specified vblank occurred at
//------------------------------------------------------------------------------------------------------------------------------------------
time_point_t I_GetVBlankTimepoint(const int32_t vblankIdx) noexcept {
    constexpr double NANOSECS_PER_SECOND = 1000 * 1000 * 1000;

    if (Game::gSettings.bUsePalTimings) {
        constexpr double nanosPerVblank = NANOSECS_PER_SECOND / 50.0;
        return gAppStartTime + std::chrono::nanoseconds((int64_t)((double) vblankIdx * nanosPerVblank));
    } else {
        constexpr double nanosPerVblank = NANOSECS_PER_SECOND / 60.0;
        return gAppStartTime + std::chrono::nanoseconds((int64_t)((double) vblankIdx * nanosPerVblank));
    }
}
#endif  // #if PSYDOOM_MODS

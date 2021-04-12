#include "i_main.h"

#include "Asserts.h"
#include "d_vsprintf.h"
#include "Doom/d_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/neterror_main.h"
#include "FatalErrors.h"
#include "i_drawcmds.h"
#include "PcPsx/Config.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/Network.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/PsxVm.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Video.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VPlaqueDrawer.h"
#include "PcPsx/Vulkan/VRenderer.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "PsyQ/LIBSN.h"
#include "w_wad.h"
#include "z_zone.h"

#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// Texture cache related stuff.
// VRAM for PSX Doom is managed as follows:
//
//  (1) There is 1 MiB in total of VRAM split into 16 texture 'pages'.
//  (2) Each texture page is 256x256 when interpreted as 8 bits per pixel data and 64 KiB in size.
//  (3) The first 4 texture pages are reserved for the front and back framebuffer.
//      Each framebuffer is 256x256 pixels (note: some vertical space is unused) but since the data is
//      16 bits per pixel rather than 8 bits, each screen occupies 2 texture pages rather than 1.
//  (4) Following that, 11 pages are free to be used in any way for map textures and sprites.
//      This is what is referred to as the 'texture cache'.
//  (5) 1 texture page at the end appears to be reserved/unused during gameplay.
//      Perhaps it was held off limits in case memory got tight and a quick fix was needed, or kept reserved for debug stuff?
//  (6) Each of the 11 256x256 pages are broken up into a grid of 16x16 cells, where each cell is 16 pixels wide and tall.
//      Each cell stores a pointer to the 'texture_t' occupying the cell, if any.
//  (7) When adding a sprite or texture to the cache, the code will search through the cells for a free space
//      large enough to accomodate the given resource size, rounded up to 16 pixel increments. The search starts after
//      where the last texture was placed and proceeds across and downwards (in that order) to fill the current texture page.
//      If the current texture page has been filled then the algorithm moves around to the next page, potentially wrapping
//      around back to the first.
//  (8) If a request to add a texture to the cache comes across occupied VRAM cells with textures uploaded in the current frame,
//      then a 'Texture Cache Overflow' fatal error is declared. This means that the algorithm has looped around once already and
//      filled the cache entirely in the same frame. Uploading any more textures would overwrite textures needed for current drawing
//      so that this point the renderer gives up and dies. Perhaps a more robust fix for this problem might have been to simply ignore
//      the request, or flush all pending drawing ops and clear the cache - but perhaps those solutions were deemed unacceptable...
//  (9) One other small detail also is that certain texture pages can be marked as 'locked'.
//      These 'locked' pages are left alone when clearing or adding textures to the cache.
//      Map floor and wall textures are placed in 'locked' pages so that they are never unloaded during level gameplay.
//  (10) Lastly it is worth mentioning that since textures in unlocked pages can be evicted at any time, these textures must also be
//       backed up and retained in main RAM. So essentially the renderer needs to keep a copy of all sprite data in main RAM also.
//------------------------------------------------------------------------------------------------------------------------------------------
struct tcachepage_t {
    texture_t* cells[TCACHE_CELLS_Y][TCACHE_CELLS_X];
};

struct tcache_t {
    tcachepage_t pages[NUM_TCACHE_PAGES];
};

// The texture page index in VRAM that the texture cache starts at.
// The first 4 pages are reserved for the framebuffer!
static constexpr uint32_t TCACHE_BASE_PAGE = 4;

// The texture coordinates of each texture cache page in VRAM.
// Notes:
//  (1) Since the PsyQ SDK expects these coordinates to be in terms of a 16-bit texture, X is halved here because Doom's textures are actually 8 bit.
//  (2) The coordinates ignore the first 4 pages in VRAM, since that is used for the framebuffer.
//  (3) The extra 'reserved' or unused texture page that is not used in the cache is also here too, hence 1 extra coordinate.
constexpr uint16_t TEX_PAGE_VRAM_TEXCOORDS[NUM_TCACHE_PAGES + 1][2] = {
    { 512,  0,  },
    { 640,  0,  },
    { 768,  0,  },
    { 896,  0,  },
    { 0,    256 },
    { 128,  256 },
    { 256,  256 },
    { 384,  256 },
    { 512,  256 },
    { 640,  256 },
    { 768,  256 },
    { 896,  256 }
};

// Texture cache variables.
// The texture cache data structure, where we are filling in the cache next and the current fill row height (in cells).
static tcache_t* gpTexCache;

uint32_t    gTCacheFillPage;
uint32_t    gTCacheFillCellX;
uint32_t    gTCacheFillCellY;
uint32_t    gTCacheFillRowCellH;

// Texture cache: a bit mask of which texture pages (past the initial 4 which are reserved for the framebuffer) are 'locked' during gameplay.
// Locked texture pages are not allowed to be unloaded and are used for UI sprites, wall and floor textures.
// Sprites on the other hand are placed in 'unlocked' texture pages and can be evicted at any time.
uint32_t gLockedTexPagesMask;

// A 64-KB buffer used for WAD loading and other stuff
std::byte gTmpBuffer[TMP_BUFFER_SIZE];

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

#if PSYDOOM_MODS
    // Max tolerated packet delay in milliseconds: after which we start adjusting clocks
    static constexpr int32_t MAX_PACKET_DELAY_MS = 15;

    // The current network protocol version.
    // Should be incremented whenever the data format being transmitted changes.
    static constexpr int32_t NET_PROTOCOL_VERSION = 6;

    // Game ids for networking
    static constexpr int32_t NET_GAMEID_DOOM        = 0xAA11AA22;
    static constexpr int32_t NET_GAMEID_FINAL_DOOM  = 0xAB11AB22;

    // Previous game error checking value when we last sent to the other player.
    // Have to store this because we always send 1 packet ahead for the next frame.
    static uint32_t gNetPrevErrorCheck;

    // Flag set to true upon connection and cleared the first call to 'I_NetUpdate'.
    // Lets us know when we are sending the first update packet of the session.
    static bool gbNetIsFirstNetUpdate;

    // How much to adjust time in a networked game.
    // Used to tweak the clock to try and get both peers locked into the same time.
    static int32_t gNetTimeAdjustMs;

    // How delayed the last packet we received from the other player was.
    // This is relayed to the other peer on the next packet, so that the other player's time can be adjusted forward (if required).
    static int32_t gLastInputPacketDelayMs;
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
            std::exit(1);
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
                    std::exit(0);
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

    // Ensure the lump data is loaded and decompress if required
    const void* pLumpData = W_CacheLumpNum(lumpNum, PU_CACHE, false);
    const bool bIsCompressed = (!gpbIsUncompressedLump[lumpNum]);

    if (bIsCompressed) {
        ASSERT(getDecodedSize(pLumpData) <= TMP_BUFFER_SIZE);
        decode(pLumpData, gTmpBuffer);
        pLumpData = gTmpBuffer;
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
        tex.width16 = (patchW + 15) / 16;
    } else {
        tex.width16 = (patchW + 30) / 16;   // Not sure why texture sizes would be negative? What does that mean?
    }
    
    if (patchH + 15 >= 0) {
        tex.height16 = (patchH + 15) / 16;
    } else {
        tex.height16 = (patchH + 30) / 16;  // Not sure why texture sizes would be negative? What does that mean?
    }
    
    tex.texPageId = 0;
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
    const uint8_t texU,
    const uint8_t texV,
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
    const uint8_t texU,
    const uint8_t texV,
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

#endif

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
            if (Config::gbUncapFramerate) {
                const int32_t minTickVBlanks = (Game::gSettings.bUseDemoTimings) ? demoTickVBlanks : 2;

                if (elapsedVBlanks < minTickVBlanks) {
                    gElapsedVBlanks = 0;
                    return;
                }
            }
        #endif

        gTotalVBlanks = curVBlanks;
        gElapsedVBlanks = elapsedVBlanks;
        
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
    // Alloc the texture cache, zero initialize and do a 'purge' to initialize tracking/management state
    gpTexCache = (tcache_t*) Z_Malloc(*gpMainMemZone, sizeof(tcache_t), PU_STATIC, nullptr);
    D_memset(gpTexCache, std::byte(0), sizeof(tcache_t));
    I_PurgeTexCache();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the specified texture into VRAM if it's not already resident.
// If there's no more room for textures in VRAM for this frame then the game will die with an error.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CacheTex(texture_t& tex) noexcept {
    // First update the frame the texture was added to the cache, for tracking overflows
    tex.uploadFrameNum = gNumFramesDrawn;

    // If the texture is already in the cache then there is nothing else to do
    if (tex.texPageId != 0)
        return;

    const uint32_t startTCacheFillPage = gTCacheFillPage;
    texture_t** pTexStartCacheCell = nullptr;

    {
    find_free_tcache_location:
        // Move onto another row in the texture cache if this row can't accomodate the texture
        if (gTCacheFillCellX + tex.width16 > TCACHE_CELLS_X) {
            gTCacheFillCellY += gTCacheFillRowCellH;
            gTCacheFillCellX = 0;
            gTCacheFillRowCellH = 0;
        }
        
        // Move onto another page in the texture cache if this page can't accomodate the texture.
        // Find one that is not locked and which is available for modification.
        if (gTCacheFillCellY + tex.height16 > TCACHE_CELLS_Y) {
            const uint32_t lockedTPages = gLockedTexPagesMask;

            // PsyDoom: if all the pages are locked this code will loop forever.
            // If this situation arises go straight to the overflow error so we can at least report the problem.
            #if PSYDOOM_MODS
                if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
            #endif
                    // Continue moving to the next texture page and wraparound if required until we find one that is not locked
                    do {
                        gTCacheFillPage++;
                        gTCacheFillPage -= (gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
                    } while ((lockedTPages >> gTCacheFillPage) & 1);
            #if PSYDOOM_MODS
                }
            #endif

            // If we wound up back where we started then there's nowhere in the cache to fit this texture.
            // This is where the imfamous overflow error kicks in...
            if (gTCacheFillPage == startTCacheFillPage) {
                I_Error("Texture Cache Overflow\n");
            }

            gTCacheFillCellX = 0;
            gTCacheFillCellY = 0;
            gTCacheFillRowCellH = 0;
        }

        // At the current fill location search all of the cells in the texture cache that this texture would occupy.
        // Make sure all of the cells are free and available for use before we can proceed.
        // If cells are not free then evict whatever is in the cache if allowed, otherwise skip past it.
        tcache_t& tcache = *gpTexCache;
        tcachepage_t& tcachepage = tcache.pages[gTCacheFillPage];
        pTexStartCacheCell = &tcachepage.cells[gTCacheFillCellY][gTCacheFillCellX];

        {
            // Iterate through all the cells this texture would occupy
            texture_t** pCacheEntry = pTexStartCacheCell;
            
            for (int32_t y = 0; y < tex.height16; ++y) {
                for (int32_t x = 0; x < tex.width16; ++x) {
                    // Check to see if this cell is empty and move past it.
                    // If it's already empty then we don't need to do anything:
                    texture_t* const pCellTex = *pCacheEntry;
                    ++pCacheEntry;

                    if (!pCellTex)
                        continue;

                    // Cell is not empty! If the texture in the cell is in use for this frame then we can't evict it.
                    // In this case skip past the texture and try again:
                    if (pCellTex->uploadFrameNum == gNumFramesDrawn) {
                        gTCacheFillCellX += pCellTex->width16;

                        // We may need to skip onto the next row on retry also, make sure we have the right row height recorded.
                        // The row height is the max of all the texture heights on the row basically:
                        if (gTCacheFillRowCellH < pCellTex->height16) {
                            gTCacheFillRowCellH = pCellTex->height16;
                        }

                        goto find_free_tcache_location;
                    }

                    // The cell is not empty but we can evict the texture, do that here now
                    I_RemoveTexCacheEntry(*pCellTex);
                }

                pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Move onto the next row of cells
            }
        }
    }

    // Fill all of the cells in the cache occupied by this texture with references to it
    {
        texture_t** pCacheEntry = pTexStartCacheCell;

        for (int32_t y = 0; y < tex.height16; ++y) {
            for (int32_t x = 0; x < tex.width16; ++x) {
                *pCacheEntry = &tex;
                ++pCacheEntry;
            }

            pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Move to the next row of cells
        }
    }

    // Record on the texture where it is located in the cache (top left corner)
    tex.ppTexCacheEntries = pTexStartCacheCell;

    // Make sure the texture's lump is loaded and decompress if required
    const std::byte* pTexData = (const std::byte*) W_CacheLumpNum(tex.lumpNum, PU_CACHE, false);
    const bool bIsTexCompressed = (!gpbIsUncompressedLump[tex.lumpNum]);

    if (bIsTexCompressed) {
        ASSERT(getDecodedSize(pTexData) <= TMP_BUFFER_SIZE);
        decode(pTexData, gTmpBuffer);
        pTexData = gTmpBuffer;
    }

    // Upload the texture to VRAM at the current fill location
    {
        const uint16_t tpageU = TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][0];
        const uint16_t tpageV = TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][1];
        
        RECT dstVramRect;
        LIBGPU_setRECT(
            dstVramRect,
            (int16_t)(tpageU + gTCacheFillCellX * TCACHE_CELL_SIZE / 2),
            (int16_t)(tpageV + gTCacheFillCellY * TCACHE_CELL_SIZE),
            tex.width / 2,
            tex.height
        );

        LIBGPU_LoadImage(dstVramRect, (uint16_t*)(pTexData + sizeof(texlump_header_t)));
    }

    // Save the textures page coordinate
    tex.texPageCoordX = (uint8_t)(gTCacheFillCellX * TCACHE_CELL_SIZE);
    tex.texPageCoordY = (uint8_t)(gTCacheFillCellY * TCACHE_CELL_SIZE);
    
    // Get and save the texture page id.
    tex.texPageId = LIBGPU_GetTPage(
        1,                                                                  // Format 1 = 8 bpp (indexed)
        0,                                                                  // Note: transparency bits are added later during rendering
        (gTCacheFillPage + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),      // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
        TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][1]
    );

    // Advance the fill position in the texture cache.
    // Also expand the fill row height if this texture is taller than the current height.
    gTCacheFillCellX += tex.width16;

    if (gTCacheFillRowCellH < tex.height16) {
        gTCacheFillRowCellH = tex.height16;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the given texture from the cache and frees any cells that it is using.
// Note this should only be called if the texture is actually in the cache!
//------------------------------------------------------------------------------------------------------------------------------------------
void I_RemoveTexCacheEntry(texture_t& tex) noexcept {
    // PsyDoom: added sanity check: this could trash textures in the cache if the texture is already freed!
    #if PSYDOOM_MODS
        ASSERT(tex.texPageId != 0);
    #endif

    // Wipe the texture page id used and clear any cells the texture occupies
    tex.texPageId = 0;
    texture_t** pCacheEntry = tex.ppTexCacheEntries;
    
    for (int32_t y = 0; y < tex.height16; ++y) {
        for (int32_t x = 0; x < tex.width16; ++x) {
            *pCacheEntry = nullptr;
            ++pCacheEntry;
        }

        pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Next row
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evicts all textures in non-locked pages in the texture cache.
// Also resets the next position that we will populate the cache at.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_PurgeTexCache() noexcept {
    tcache_t& tcache = *gpTexCache;

    for (int32_t texPageIdx = 0; texPageIdx < NUM_TCACHE_PAGES; ++texPageIdx) {
        // Leave this texture page alone and skip past it if it is locked
        const uint32_t lockedTCachePages = gLockedTexPagesMask;

        // PsyDoom: this could potentially skip past the last texture cache page if it is locked.
        // It doesn't happen in practice because the last pages are always used for sprites and unlocked, but
        // change the method used here to skip just in case we do get undefined behavior...
        #if PSYDOOM_MODS
            if ((lockedTCachePages >> texPageIdx) & 1)
                continue;
        #else
            while ((lockedTPages >> texPageIdx) & 1) {
                ++texPageIdx;
            }
        #endif

        // Run though all of the cells in the texture cache page.
        // For each cell where we find an occupying texture, clear any cells that the found texture occupies.
        {
            texture_t** pPageCacheEntry = &tcache.pages[texPageIdx].cells[0][0];

            for (uint32_t cellIdx = 0; cellIdx < NUM_TCACHE_PAGE_CELLS; ++cellIdx, ++pPageCacheEntry) {
                // Ignore the texture cache cell if not occupied by a texture
                if (!*pPageCacheEntry)
                    continue;

                // Get the texture occupying this cell and clear it's texture page.
                texture_t& tex = **pPageCacheEntry;
                tex.texPageId = 0;

                // Clear all cells occupied by the texture
                texture_t** pTexCacheEntry = tex.ppTexCacheEntries;

                for (int32_t y = 0; y < tex.height16; ++y) {
                    for (int32_t x = 0; x < tex.width16; ++x) {
                        *pTexCacheEntry = nullptr;
                        ++pTexCacheEntry;
                    }

                    pTexCacheEntry += TCACHE_CELLS_X - tex.width16;     // Move to the next row of cells
                }
            }
        }
    }

    // Decide on the initial texture cache fill page: find the first page that is not locked.
    //
    // PsyDoom: I also added an additional safety check here.
    // If all the pages are locked for some reason this code would loop forever, check for that situation first.
    const uint32_t lockedTPages = gLockedTexPagesMask;
    gTCacheFillPage = 0;
    
    #if PSYDOOM_MODS
        if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
    #endif
            // Move onto the next texture cache page and wraparound, until we find an unlocked page to settle on
            while ((lockedTPages >> gTCacheFillPage) & 1) {
                gTCacheFillPage++;
                gTCacheFillPage -= (gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
            }
    #if PSYDOOM_MODS
        }
    #endif

    // Reset the other fill parameters
    gTCacheFillCellX = 0;
    gTCacheFillCellY = 0;
    gTCacheFillRowCellH = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the 'vram viewer' screen that was only enabled in development builds of PSX DOOM.
// This code was still compiled into the retail binary however, but was inaccessible outside of memory hacks.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_VramViewerDraw(const int32_t texPageNum) noexcept {
    // Draw a sprite covering the screen which covers the entire vram page
    {
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            POLY_FT4 polyPrim = {};
        #else
            POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setRGB0(polyPrim, 128, 128, 128);

        // PsyDoom: fix these right and bottom UVs now that we can do 16-bit coordinates.
        // This fixes slight pixel stretching issues in the VRAM viewer.
        #if PSYDOOM_LIMIT_REMOVING
            constexpr LibGpuUV BUV = 256;
            constexpr LibGpuUV RUV = 256;
        #else
            constexpr LibGpuUV BUV = 255;
            constexpr LibGpuUV RUV = 255;
        #endif

        LIBGPU_setUV4(polyPrim,
            0,      0,
            RUV,    0,
            0,      BUV,
            RUV,    BUV
        );

        // PsyDoom: adjust the bottom edge of the VRAM view to account for overscan of 8-pixels
        #if PSYDOOM_MODS
            const int16_t BY = 232;
        #else
            const int16_t BY = 240;
        #endif

        LIBGPU_setXY4(polyPrim,
            0,      0,
            256,    0,
            0,      BY,
            256,    BY
        );

        polyPrim.clut = gPaletteClutIds[MAINPAL];

        polyPrim.tpage = LIBGPU_GetTPage(
            1,                                                          // Format 1 = 8 bpp (indexed)
            0,                                                          // Don't care about transparency
            (texPageNum + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),   // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
            TEX_PAGE_VRAM_TEXCOORDS[texPageNum][1]
        );

        I_AddPrim(polyPrim);
    }

    // Draw red lines around all entries on this texture cache page
    const tcachepage_t& cachePage = gpTexCache->pages[texPageNum];
    const texture_t* const* pCacheEntry = &cachePage.cells[0][0];

    for (int32_t entryIdx = 0; entryIdx < TCACHE_CELLS_X * TCACHE_CELLS_Y; ++entryIdx, ++pCacheEntry) {
        // Skip past this texture cache cell if there's no texture occupying it
        const texture_t* pTex = *pCacheEntry;
        
        if (!pTex)
            continue;

        // Setup some stuff for all lines drawn.
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state.
        #if PSYDOOM_MODS
            LINE_F2 linePrim = {};
        #else
            LINE_F2& linePrim = *(LINE_F2*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetLineF2(linePrim);
        LIBGPU_setRGB0(linePrim, 255, 0, 0);
        
        // Figure out the y position and height for the texture cache entry.
        // Have to rescale in using a 24.8 fixed point format because the screen is 240 pixels high, and the texture cache is 256 pixels high.
        // I'm not sure why there is negative checks here however, texcoords are never negative?
        int16_t y = (int16_t)(((int32_t) d_lshift<8>(pTex->texPageCoordY)) * SCREEN_H / TCACHE_PAGE_SIZE);
        int16_t h = (int16_t)(((int32_t) d_lshift<8>(pTex->height))        * SCREEN_H / TCACHE_PAGE_SIZE);

        if (y < 0) { y += 255; }
        if (h < 0) { h += 255; }

        y = d_rshift<8>(y);
        h = d_rshift<8>(h);

        const int16_t x = pTex->texPageCoordX;
        const int16_t w = pTex->width;

        // Draw all the box lines for the cache entry
        LIBGPU_setXY2(linePrim, x, y, x + w, y);                // Top
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x + w, y, x + w, y + h);        // Right
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x + w, y + h, x, y + h);        // Bottom
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x, y + h, x, y);                // Left
        I_AddPrim(linePrim);
    }
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

    // Clear these value initially
    gNetPrevErrorCheck = {};
    gNetTimeAdjustMs = 0;
    gLastInputPacketDelayMs = 0;

    // Fill in the connect output packet; note that player 1 decides the game params, so theese are zeroed for player 2:
    const uint32_t netGameId = (Game::isFinalDoom()) ? NET_GAMEID_FINAL_DOOM : NET_GAMEID_DOOM;

    NetPacket_Connect outPkt = {};
    outPkt.protocolVersion = NET_PROTOCOL_VERSION;
    outPkt.gameId = netGameId;

    if (gCurPlayerIndex == 0) {
        outPkt.startGameType = gStartGameType;
        outPkt.startGameSkill = gStartSkill;
        outPkt.startMap = gStartMapOrEpisode;
    } else {
        outPkt.startGameType = {};
        outPkt.startGameSkill = {};
        outPkt.startMap = {};
    }

    // Endian correct the output packet and send
    outPkt.protocolVersion = Endian::hostToLittle(outPkt.protocolVersion);
    outPkt.gameId = Endian::hostToLittle(outPkt.gameId);
    outPkt.startGameType = Endian::hostToLittle(outPkt.startGameType);
    outPkt.startGameSkill = Endian::hostToLittle(outPkt.startGameSkill);
    outPkt.startMap = Endian::hostToLittle(outPkt.startMap);

    Network::sendBytes(&outPkt, sizeof(outPkt));

    // If the player is the server then determine the game settings, endian correct and send to the client
    if (gCurPlayerIndex == 0) {
        // Determine the game settings to use based on config and command line arguments
        Game::getUserGameSettings(Game::gSettings);

        // Copy and endian correct the settings before sending to the client player
        GameSettings settings = Game::gSettings;
        settings.lostSoulSpawnLimit = Endian::hostToLittle(settings.lostSoulSpawnLimit);

        Network::sendBytes(&settings, sizeof(settings));
    }

    // Read the input connect packet from the other player and endian correct
    NetPacket_Connect inPkt = {};
    Network::recvBytes(&inPkt, sizeof(inPkt));

    inPkt.protocolVersion = Endian::littleToHost(inPkt.protocolVersion);
    inPkt.gameId = Endian::littleToHost(inPkt.gameId);
    inPkt.startGameType = Endian::littleToHost(inPkt.startGameType);
    inPkt.startGameSkill = Endian::littleToHost(inPkt.startGameSkill);
    inPkt.startMap = Endian::littleToHost(inPkt.startMap);

    // Verify the network protocol version and game ids are OK - abort if not
    if ((inPkt.protocolVersion != NET_PROTOCOL_VERSION) || (inPkt.gameId != netGameId)) {
        gbDidAbortGame = true;
        RunNetErrorMenu_GameTypeOrVersionMismatch();
        return;
    }

    // Player 2 gets the details of the game from player 1 and must setup the game accordingly.
    // This includes the game settings packet sent from the server.
    if (gCurPlayerIndex != 0) {
        gStartGameType = inPkt.startGameType;
        gStartSkill = inPkt.startGameSkill;
        gStartMapOrEpisode = inPkt.startMap;

        // Read the game settings from the server and endian correct
        GameSettings settings = {};

        if (!Network::recvBytes(&settings, sizeof(settings))) {
            RunNetErrorMenu_FailedToConnect();
            gbDidAbortGame = true;
            return;
        }

        settings.lostSoulSpawnLimit = Endian::littleToHost(settings.lostSoulSpawnLimit);

        // Save the game settings to use
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
        }
    }

    // If it's the very first network update for this session send a dummy packet with no inputs to the other player.
    // This kick starts the sequence of always sending one packet ahead on both ends and helps to reduce lag.
    if (gbNetIsFirstNetUpdate) {
        // Populate and send the output packet
        NetPacket_Tick outPkt = {};
        outPkt.errorCheck = Endian::hostToLittle(errorCheck);

        Network::sendTickPacket(outPkt);

        // Make sure these are set correctly for what we expect
        gNextTickInputs = {};
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
        gPlayerNextTickViewAngle = playerAngle + gTickInputs[gCurPlayerIndex].analogTurn + gNextTickInputs.analogTurn;
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
    outPkt.inputs.analogForwardMove = Endian::hostToLittle(outPkt.inputs.analogForwardMove);
    outPkt.inputs.analogSideMove = Endian::hostToLittle(outPkt.inputs.analogSideMove);
    outPkt.inputs.analogTurn = Endian::hostToLittle(outPkt.inputs.analogTurn);
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
    inPkt.inputs.analogForwardMove = Endian::littleToHost(inPkt.inputs.analogForwardMove);
    inPkt.inputs.analogSideMove = Endian::littleToHost(inPkt.inputs.analogSideMove);
    inPkt.inputs.analogTurn = Endian::littleToHost(inPkt.inputs.analogTurn);
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
// PsyDoom: get the total number of vblanks elapsed, and use the current time adjustment in networked games
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t I_GetTotalVBlanks() noexcept {
    typedef std::chrono::steady_clock::time_point               time_point_t;
    typedef std::chrono::steady_clock::duration                 duration_t;
    typedef std::chrono::duration<int64_t, std::ratio<1, 50>>   tick_50hz_t;
    typedef std::chrono::duration<int64_t, std::ratio<1, 60>>   tick_60hz_t;
    
    const time_point_t now = std::chrono::steady_clock::now();
    const duration_t timeSinceAppStart = now - gAppStartTime;
    const duration_t timeAdjustMs = std::chrono::milliseconds((gNetGame != gt_single) ? gNetTimeAdjustMs : 0);
    const duration_t adjustedDuration = timeSinceAppStart + timeAdjustMs;

    if (Game::gSettings.bUsePalTimings) {
        return (int32_t) std::chrono::duration_cast<tick_50hz_t>(adjustedDuration).count();
    } else {
        return (int32_t) std::chrono::duration_cast<tick_60hz_t>(adjustedDuration).count();
    }
}

#endif

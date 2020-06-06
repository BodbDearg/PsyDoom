#include "i_main.h"

#include "d_vsprintf.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "i_drawcmds.h"
#include "PcPsx/Network.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Video.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBCOMB.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "PsyQ/LIBSN.h"
#include "w_wad.h"
#include "z_zone.h"

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
//      TODO: find out more about why this is.
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
    VmPtr<texture_t> cells[TCACHE_CELLS_Y][TCACHE_CELLS_X];
};

static_assert(sizeof(tcachepage_t) == 1024);

struct tcache_t {
    tcachepage_t pages[NUM_TCACHE_PAGES];
};

static_assert(sizeof(tcache_t) == 1024 * 11);

// The texture page index in VRAM that the texture cache starts at.
// The first 4 pages are reserved for the framebuffer!
static constexpr uint32_t TCACHE_BASE_PAGE = 4;

// The texture coordinates of each texture cache page in VRAM.
// Notes:
//  (1) Since the PsyQ SDK expects these coordinates to be in terms of a 16-bit texture, X is halved here
//      because Doom's textures are actually 8 bit.
//  (2) The coordinates ignore the first 4 pages in VRAM, since that is used for the framebuffer.
//  (3) The extra 'reserved' texture page that is not used in the cache is also here too, hence 1 extra coordinate.
//      TODO: find out more about this.
//
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

constexpr int32_t   NET_PACKET_SIZE     = 8;        // The size of a packet and the packet buffers in a networked game
constexpr uint8_t   NET_PACKET_HEADER   = 0xAA;     // The 1st byte in every network packet: used for error detection purposes

// Texture cache variables.
// The texture cache data structure, where we are filling in the cache next and the current fill row height (in cells).
static const VmPtr<VmPtr<tcache_t>> gpTexCache(0x80077F74);

const VmPtr<uint32_t>   gTCacheFillPage(0x80078028);
const VmPtr<uint32_t>   gTCacheFillCellX(0x800782E4);
const VmPtr<uint32_t>   gTCacheFillCellY(0x800782E8);
const VmPtr<uint32_t>   gTCacheFillRowCellH(0x80078278);

// Texture cache: a bit mask of which texture pages (past the initial 4 which are reserved for the framebuffer) are 'locked' during gameplay.
// Locked texture pages are not allowed to be unloaded and are used for UI sprites, wall and floor textures.
// Sprites on the other hand are placed in 'unlocked' texture pages and can be evicted at any time.
const VmPtr<uint32_t> gLockedTexPagesMask(0x80077C08);

// A 64-KB buffer used for WAD loading and other stuff
const VmPtr<std::byte[TMP_BUFFER_SIZE]> gTmpBuffer(0x80098748);

// Video vblank timers: track the total amount, last total and current elapsed amount
const VmPtr<uint32_t> gTotalVBlanks(0x80077E98);
const VmPtr<uint32_t> gLastTotalVBlanks(0x80078114);
const VmPtr<uint32_t> gElapsedVBlanks(0x800781BC);

// The index of the currently displaying framebuffer, 0 or 1
const VmPtr<uint32_t> gCurDispBufferIdx(0x800780F8);

// The draw and display environments for framebuffers 0 and 1
const VmPtr<DISPENV[2]> gDispEnvs(0x800A9164);
const VmPtr<DRAWENV[2]> gDrawEnvs(0x800A90AC);

// Used to tell when the texture cache overflows.
// Each texture when added to the cache is assigned the current value of this number.
// When the time comes to evict a texture to make room for another, we check to make sure that the texture wasn't loaded in the current frame.
// If the evicted texture WAS loaded in the current frame, then it means we've run out of texture memory and can't draw all of the textures in the frame.
const VmPtr<uint32_t> gNumFramesDrawn(0x80077C10);

// The index of the user's player in the array of players: whether you are player 1 or 2 in other words
const VmPtr<int32_t> gCurPlayerIndex(0x80077618);

// Control related stuff
const VmPtr<padbuttons_t[NUM_CTRL_BINDS]>                       gCtrlBindings(0x80073E0C);              // This players control bindings
const VmPtr<padbuttons_t[NUM_CTRL_BINDS]>                       gOtherPlayerCtrlBindings(0x800782FC);   // Control bindings for the remote player
const VmPtr<VmPtr<padbuttons_t[NUM_CTRL_BINDS]>[MAXPLAYERS]>    gpPlayerCtrlBindings(0x80077FC8);       // Pointer to control bindings for player 1 and 2

const padbuttons_t gBtnMasks[NUM_BINDABLE_BTNS] = {
    PAD_TRIANGLE,   // bindablebtn_triangle
    PAD_CIRCLE,     // bindablebtn_circle
    PAD_CROSS,      // bindablebtn_cross
    PAD_SQUARE,     // bindablebtn_square
    PAD_L1,         // bindablebtn_l1
    PAD_R1,         // bindablebtn_r1
    PAD_L2,         // bindablebtn_l2
    PAD_R2          // bindablebtn_r2
};

// The main UI texture atlas for the game.
// This is loaded into the 1st available texture page and kept loaded at all times after that.
const VmPtr<texture_t> gTex_STATUS(0x800A94E8);

// Loading, connecting, error etc. plaques
const VmPtr<texture_t>  gTex_PAUSE(0x80097A70);
const VmPtr<texture_t>  gTex_LOADING(0x80097A90);
const VmPtr<texture_t>  gTex_NETERR(0x80097AF0);
const VmPtr<texture_t>  gTex_CONNECT(0x80097B10);

// PSX Kernel events that fire when reads and writes complete for Serial I/O in a multiplayer game
static const VmPtr<uint32_t>    gSioReadDoneEvent(0x80077F24);
static const VmPtr<uint32_t>    gSioWriteDoneEvent(0x80078040);

// File descriptors for the input/output streams used for multiplayer games.
// These are opened against the Serial I/O device (PlayStation Link Cable).
static const VmPtr<int32_t>     gNetInputFd(0x80078234);
static const VmPtr<int32_t>     gNetOutputFd(0x80077F14);

// The packet buffers for sending and receiving in a multiplayer game.
// The link cable allows 8 bytes to be sent and received at a time.
static const VmPtr<uint8_t[NET_PACKET_SIZE]>    gNetInputPacket(0x80077EA8);
static const VmPtr<uint8_t[NET_PACKET_SIZE]>    gNetOutputPacket(0x80077FB0);

// Buffers used originally by the PsyQ SDK to store gamepad input.
// In PsyDoom they are just here for historical reference.
static const VmPtr<uint8_t[34]>     gPadInputBuffer_1(0x80097788);
static const VmPtr<uint8_t[34]>     gPadInputBuffer_2(0x800978EC);

//------------------------------------------------------------------------------------------------------------------------------------------
// User/client entrypoint for PlayStation DOOM.
// This was probably the actual 'main()' function in the real source code.
// I'm just calling 'I_Main()' so as not to confuse it with this port's 'main()'...
//------------------------------------------------------------------------------------------------------------------------------------------
void I_Main() noexcept {
    // PsyQ SDK initialization stuff
    LIBSN__main();

    #if PC_PSX_DOOM_MODS
        Video::initVideo();
    #endif

    D_DoomMain();

    #if PC_PSX_DOOM_MODS
        Video::shutdownVideo();
    #endif
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
    LIBAPI_InitPAD(gPadInputBuffer_1.get(), gPadInputBuffer_2.get(), 34, 34);
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
    *gCurDispBufferIdx = 0;
    
    // Not sure why interrupts are being disabled and then immediately re-enabled, perhaps to flush out some state?
    LIBAPI_EnterCriticalSection();
    LIBAPI_ExitCriticalSection();

    // Setup serial (PlayStation link cable) communications:
    //  (1) Install the serial I/O driver.
    //  (2) Setup events for read and write done, so we can detect when these operations complete.
    //  (3) Open the serial I/O files that we use for input and output
    //  (4) Set data transmission rate (bits per second)
    //
    LIBCOMB_AddCOMB();
    
    *gSioReadDoneEvent = LIBAPI_OpenEvent(HwSIO, EvSpIOER, EvMdNOINTR, nullptr);
    LIBAPI_EnableEvent(*gSioReadDoneEvent);
    
    *gSioWriteDoneEvent = LIBAPI_OpenEvent(HwSIO, EvSpIOEW, EvMdNOINTR, nullptr);
    LIBAPI_EnableEvent(*gSioWriteDoneEvent);

    *gNetOutputFd = LIBAPI_open("sio:", O_WRONLY);
    *gNetInputFd = LIBAPI_open("sio:", O_RDONLY | O_NOWAIT);

    LIBCOMB_CombSetBPS(38400);
        
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
    char msgBuffer[256];

    {
        va_list args;
        va_start(args, fmtMsg);
        D_vsprintf(msgBuffer, fmtMsg, args);
        va_end(args);
    }

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
    // TODO: check for window close and then terminate forcefully here.
    while (true) {
        #if PC_PSX_DOOM_MODS
            Utils::doPlatformUpdates();
        #endif
    }
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
        decode(pLumpData, gTmpBuffer.get());
        pLumpData = gTmpBuffer.get();
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
        DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);
        LIBGPU_SetDrawMode(drawModePrim, false, false, texPageId, nullptr);
        I_AddPrim(&drawModePrim);
    }

    // Setup the sprite primitive and submit
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_setRGB0(spritePrim, 128, 128, 128);
    LIBGPU_setXY0(spritePrim, xpos, ypos);
    LIBGPU_setUV0(spritePrim, texU, texV);
    LIBGPU_setWH(spritePrim, texW, texH);
    spritePrim.clut = clutId;

    I_AddPrim(LIBETC_getScratchAddr(128));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the front buffer to the back buffer, draws a loading plaque over it and presents it to the screen.
// Useful for drawing a loading message before doing a long running load or connect operation.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawLoadingPlaque(texture_t& tex, const int16_t xpos, const int16_t ypos, const int16_t clutId) noexcept {
    // PC-PSX: ignore in headless mdoe
    #if PC_PSX_DOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    // Make sure the GPU is idle and copy the front buffer to the back buffer, will draw over that
    LIBGPU_DrawSync(0);

    const DISPENV& frontBuffer = gDispEnvs[*gCurDispBufferIdx];
    const DISPENV& backBuffer = gDispEnvs[*gCurDispBufferIdx ^ 1];
    LIBGPU_MoveImage(frontBuffer.disp, backBuffer.disp.x, backBuffer.disp.y);

    // Ensure the plaque is loaded
    I_IncDrawnFrameCount();
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
    *gNumFramesDrawn += 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Swaps the draw and display framebuffers, causing the current frame being rendered to be displayed onscreen.
// Also does framerate limiting to 30 Hz and updates the elapsed vblank count, which feeds the game's timing system.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawPresent() noexcept {
    // Finish up all in-flight drawing commands
    LIBGPU_DrawSync(0);

    // Wait until a VBlank occurs to swap the framebuffers.
    // PC-PSX: Note: this call now does nothing as that would inferfere with frame pacing - here just for historical reference!
    LIBETC_VSync(0);

    // Swap the framebuffers
    *gCurDispBufferIdx ^= 1;
    LIBGPU_PutDrawEnv(gDrawEnvs[*gCurDispBufferIdx]);
    LIBGPU_PutDispEnv(gDispEnvs[*gCurDispBufferIdx]);

    // PC-PSX: copy the PSX framebuffer to the display
    #if PC_PSX_DOOM_MODS
        Video::displayFramebuffer();
    #endif
        
    // In PSX Doom the rendering rate was originally limited to 30 Hz
    uint32_t minElapsedVBlanks = 2;

    #if PC_PSX_DOOM_MODS
        // FIXME: remove the high fps hack eventually
        if (ProgArgs::gbUseHighFpsHack) {
            minElapsedVBlanks = 1;
        }
    #endif

    // Continously poll and wait until the required number of vblanks have elapsed before continuing
    while (true) {
        *gTotalVBlanks = LIBETC_VSync(-1);
        *gElapsedVBlanks = *gTotalVBlanks - *gLastTotalVBlanks;

        // Has the required time passed?
        if (*gElapsedVBlanks >= minElapsedVBlanks)
            break;

        // PC-PSX: do platform updates (sound, window etc.) and yield some cpu since we are waiting for a bit
        #if PC_PSX_DOOM_MODS
            Utils::doPlatformUpdates();
            Utils::threadYield();
        #endif
    }

    // Further framerate limiting for demos:
    // Demo playback or recording is forced to run at 15 Hz all of the time (the game simulation rate).
    // Probably done so the simulation remains consistent!
    if (*gbDemoPlayback || *gbDemoRecording) {
        while (*gElapsedVBlanks < 4) {
            // PC-PSX: do platform updates (sound, window etc.) and yield some cpu since we are waiting for a bit
            #if PC_PSX_DOOM_MODS
                Utils::doPlatformUpdates();
                Utils::threadYield();
            #endif

            *gTotalVBlanks = LIBETC_VSync(-1);
            *gElapsedVBlanks = *gTotalVBlanks - *gLastTotalVBlanks;
        }

        *gElapsedVBlanks = 4;
    }

    // So we can compute the elapsed vblank amount next time round
    *gLastTotalVBlanks = *gTotalVBlanks;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Callback for when a vblank occurs.
// This function appears to be unused in the retail game, probably more simple to use polling instead and not deal with interrupts?
//------------------------------------------------------------------------------------------------------------------------------------------
void I_VsyncCallback() noexcept {
    *gTotalVBlanks += 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// DOOM engine platform specific initialization.
// For the PSX this will just setup the texture cache.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_Init() noexcept {
    // Alloc the texture cache, zero initialize and do a 'purge' to initialize tracking/management state
    *gpTexCache = (tcache_t*) Z_Malloc(*gpMainMemZone, sizeof(tcache_t), PU_STATIC, nullptr);
    D_memset(gpTexCache->get(), std::byte(0), sizeof(tcache_t));
    I_PurgeTexCache();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the specified texture into VRAM if it's not already resident.
// If there's no more room for textures in VRAM for this frame then the game will die with an error.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CacheTex(texture_t& tex) noexcept {
    // First update the frame the texture was added to the cache, for tracking overflows
    tex.uploadFrameNum = *gNumFramesDrawn;

    // If the texture is already in the cache then there is nothing else to do
    if (tex.texPageId != 0)
        return;

    const uint32_t startTCacheFillPage = *gTCacheFillPage;
    VmPtr<texture_t>* pTexStartCacheCell = nullptr;

    {
    find_free_tcache_location:
        // Move onto another row in the texture cache if this row can't accomodate the texture
        if (*gTCacheFillCellX + tex.width16 > TCACHE_CELLS_X) {
            *gTCacheFillCellY = *gTCacheFillCellY + *gTCacheFillRowCellH;
            *gTCacheFillCellX = 0;
            *gTCacheFillRowCellH = 0;
        }
        
        // Move onto another page in the texture cache if this page can't accomodate the texture.
        // Find one that is not locked and which is available for modification.
        if (*gTCacheFillCellY + tex.height16 > TCACHE_CELLS_Y) {
            const uint32_t lockedTPages = *gLockedTexPagesMask;

            // PC-PSX: if all the pages are locked this code will loop forever.
            // If this situation arises go straight to the overflow error so we can at least report the problem.
            #if PC_PSX_DOOM_MODS
                if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
            #endif
                    // Continue moving to the next texture page and wraparound if required until we find one that is not locked
                    do {
                        *gTCacheFillPage += 1;
                        *gTCacheFillPage -= (*gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
                    } while ((lockedTPages >> *gTCacheFillPage) & 1);
            #if PC_PSX_DOOM_MODS
                }
            #endif

            // If we wound up back where we started then there's nowhere in the cache to fit this texture.
            // This is where the imfamous overflow error kicks in...
            if (*gTCacheFillPage == startTCacheFillPage) {
                I_Error("Texture Cache Overflow\n");
            }

            *gTCacheFillCellX = 0;
            *gTCacheFillCellY = 0;
            *gTCacheFillRowCellH = 0;
        }

        // At the current fill location search all of the cells in the texture cache that this texture would occupy.
        // Make sure all of the cells are free and available for use before we can proceed.
        // If cells are not free then evict whatever is in the cache if allowed, otherwise skip past it.
        tcache_t& tcache = **gpTexCache;
        tcachepage_t& tcachepage = tcache.pages[*gTCacheFillPage];
        pTexStartCacheCell = &tcachepage.cells[*gTCacheFillCellY][*gTCacheFillCellX];

        {
            // Iterate through all the cells this texture would occupy
            VmPtr<texture_t>* pCacheEntry = pTexStartCacheCell;
            
            for (int32_t y = 0; y < tex.height16; ++y) {
                for (int32_t x = 0; x < tex.width16; ++x) {
                    // Check to see if this cell is empty and move past it.
                    // If it's already empty then we don't need to do anything:
                    texture_t* const pCellTex = pCacheEntry->get();
                    ++pCacheEntry;

                    if (!pCellTex)
                        continue;

                    // Cell is not empty! If the texture in the cell is in use for this frame then we can't evict it.
                    // In this case skip past the texture and try again:
                    if (pCellTex->uploadFrameNum == *gNumFramesDrawn) {
                        *gTCacheFillCellX += pCellTex->width16;

                        // We may need to skip onto the next row on retry also, make sure we have the right row height recorded.
                        // The row height is the max of all the texture heights on the row basically:
                        if (*gTCacheFillRowCellH < pCellTex->height16) {
                            *gTCacheFillRowCellH = pCellTex->height16;
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
        VmPtr<texture_t>* pCacheEntry = pTexStartCacheCell;

        for (int32_t y = 0; y < tex.height16; ++y) {
            for (int32_t x = 0; x < tex.width16; ++x) {
                *pCacheEntry = &tex;
                ++pCacheEntry;
            }

            pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Move to the next row of cells
        }
    }

    // Record on the texture where it is located in the cache (top left corner)
    tex.ppTexCacheEntries = ptrToVmAddr(pTexStartCacheCell);

    // Make sure the texture's lump is loaded and decompress if required
    const void* pTexData = W_CacheLumpNum(tex.lumpNum, PU_CACHE, false);
    const bool bIsTexCompressed = (!gpbIsUncompressedLump[tex.lumpNum]);

    if (bIsTexCompressed) {
        decode(pTexData, gTmpBuffer.get());
        pTexData = gTmpBuffer.get();
    }

    // Upload the texture to VRAM at the current fill location
    {
        const uint16_t tpageU = TEX_PAGE_VRAM_TEXCOORDS[*gTCacheFillPage][0];
        const uint16_t tpageV = TEX_PAGE_VRAM_TEXCOORDS[*gTCacheFillPage][1];
        
        RECT dstVramRect;
        LIBGPU_setRECT(
            dstVramRect,
            (int16_t)(tpageU + (*gTCacheFillCellX) * (TCACHE_CELL_SIZE / 2)),
            (int16_t)(tpageV + (*gTCacheFillCellY) * TCACHE_CELL_SIZE),
            tex.width / 2,
            tex.height
        );

        LIBGPU_LoadImage(dstVramRect, (uint16_t*) pTexData + 4);    // TODO: figure out what 8 bytes is being skipped
    }

    // Save the textures page coordinate
    tex.texPageCoordX = (uint8_t)((*gTCacheFillCellX) * TCACHE_CELL_SIZE);
    tex.texPageCoordY = (uint8_t)((*gTCacheFillCellY) * TCACHE_CELL_SIZE);
    
    // Get and save the texture page id.
    tex.texPageId = LIBGPU_GetTPage(
        1,                                                                  // Format 1 = 8 bpp (indexed)
        0,                                                                  // Note: transparency bits are added later during rendering
        (*gTCacheFillPage + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),     // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
        TEX_PAGE_VRAM_TEXCOORDS[*gTCacheFillPage][1]
    );

    // Advance the fill position in the texture cache.
    // Also expand the fill row height if this texture is taller than the current height.
    *gTCacheFillCellX += tex.width16;

    if (*gTCacheFillRowCellH < tex.height16) {
        *gTCacheFillRowCellH = tex.height16;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the given texture from the cache and frees any cells that it is using.
// Note this should only be called if the texture is actually in the cache!
//------------------------------------------------------------------------------------------------------------------------------------------
void I_RemoveTexCacheEntry(texture_t& tex) noexcept {
    // PC-PSX: added sanity check: this could trash textures in the cache if the texture is already freed!
    #if PC_PSX_DOOM_MODS
        ASSERT(tex.texPageId != 0);
    #endif

    // Wipe the texture page id used and clear any cells the texture occupies
    tex.texPageId = 0;
    VmPtr<texture_t>* pCacheEntry = tex.ppTexCacheEntries.get();
    
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
    tcache_t& tcache = **gpTexCache;

    for (int32_t texPageIdx = 0; texPageIdx < NUM_TCACHE_PAGES; ++texPageIdx) {
        // Leave this texture page alone and skip past it if it is locked
        const uint32_t lockedTCachePages = *gLockedTexPagesMask;

        // PC-PSX: this could potentially skip past the last texture cache page if it is locked.
        // It doesn't happen in practice because the last pages are always used for sprites and unlocked, but
        // change the method used here to skip just in case we do get undefined behavior...
        #if PC_PSX_DOOM_MODS
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
            VmPtr<texture_t>* pPageCacheEntry = &tcache.pages[texPageIdx].cells[0][0];

            for (uint32_t cellIdx = 0; cellIdx < NUM_TCACHE_PAGE_CELLS; ++cellIdx, ++pPageCacheEntry) {
                // Ignore the texture cache cell if not occupied by a texture
                if (!pPageCacheEntry->get())
                    continue;

                // Get the texture occupying this cell and clear it's texture page.
                texture_t& tex = **pPageCacheEntry;
                tex.texPageId = 0;

                // Clear all cells occupied by the texture
                VmPtr<texture_t>* pTexCacheEntry = tex.ppTexCacheEntries.get();

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
    // PC-PSX: I also added an additional safety check here.
    // If all the pages are locked for some reason this code would loop forever, check for that situation first.
    const uint32_t lockedTPages = *gLockedTexPagesMask;
    *gTCacheFillPage = 0;
    
    #if PC_PSX_DOOM_MODS
        if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
    #endif
            // Move onto the next texture cache page and wraparound, until we find an unlocked page to settle on
            while ((lockedTPages >> *gTCacheFillPage) & 1) {
                *gTCacheFillPage += 1;
                *gTCacheFillPage -= (*gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
            }
    #if PC_PSX_DOOM_MODS
        }
    #endif

    // Reset the other fill parameters
    *gTCacheFillCellX = 0;
    *gTCacheFillCellY = 0;
    *gTCacheFillRowCellH = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the 'vram viewer' screen that was only enabled in development builds of PSX DOOM.
// This code was still compiled into the retail binary however, but was inaccessible outside of memory hacks.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_VramViewerDraw(const int32_t texPageNum) noexcept {
    // Draw a sprite covering the screen which covers the entire vram page
    {
        POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);

        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setRGB0(polyPrim, 128, 128, 128);

        LIBGPU_setUV4(polyPrim,
            0,      0,
            255,    0,
            0,      255,
            255,    255
        );

        LIBGPU_setXY4(polyPrim,
            0,      0,
            256,    0,
            0,      240,
            256,    240
        );

        polyPrim.clut = gPaletteClutIds[MAINPAL];

        polyPrim.tpage = LIBGPU_GetTPage(
            1,                                                          // Format 1 = 8 bpp (indexed)
            0,                                                          // Don't care about transparency
            (texPageNum + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),   // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
            TEX_PAGE_VRAM_TEXCOORDS[texPageNum][1]
        );

        I_AddPrim(&polyPrim);
    }

    // Draw red lines around all entries on this texture cache page
    const tcachepage_t& cachePage = (*gpTexCache)->pages[texPageNum];
    const VmPtr<texture_t>* pCacheEntry = &cachePage.cells[0][0];

    for (int32_t entryIdx = 0; entryIdx < TCACHE_CELLS_X * TCACHE_CELLS_Y; ++entryIdx, ++pCacheEntry) {
        // Skip past this texture cache cell if there's no texture occupying it
        const texture_t* pTex = pCacheEntry->get();
        
        if (!pTex)
            continue;

        // Setup some stuff for all lines drawn
        LINE_F2& linePrim = *(LINE_F2*) LIBETC_getScratchAddr(128);
        LIBGPU_SetLineF2(linePrim);
        LIBGPU_setRGB0(linePrim, 255, 0, 0);
        
        // Figure out the y position and height for the texture cache entry.
        // Have to rescale in using a 24.8 fixed point format because the screen is 240 pixels high, and the texture cache is 256 pixels high.
        // I'm not sure why there is negative checks here however, texcoords are never negative?
        int16_t y = (int16_t)(((int32_t) pTex->texPageCoordY << 8) * SCREEN_H / TCACHE_PAGE_SIZE);
        int16_t h = (int16_t)(((int32_t) pTex->height        << 8) * SCREEN_H / TCACHE_PAGE_SIZE);

        if (y < 0) { y += 255; }
        if (h < 0) { h += 255; }

        y >>= 8;
        h >>= 8;

        const int16_t x = pTex->texPageCoordX;
        const int16_t w = pTex->width;

        // Draw all the box lines for the cache entry
        LIBGPU_setXY2(linePrim, x, y, x + w, y);                // Top
        I_AddPrim(&linePrim);

        LIBGPU_setXY2(linePrim, x + w, y, x + w, y + h);        // Right
        I_AddPrim(LIBETC_getScratchAddr(128));

        LIBGPU_setXY2(linePrim, x + w, y + h, x, y + h);        // Bottom
        I_AddPrim(LIBETC_getScratchAddr(128));

        LIBGPU_setXY2(linePrim, x, y + h, x, y);                // Left
        I_AddPrim(LIBETC_getScratchAddr(128));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the setup for a network game: synchronizes between players, then sends the game details and control bindings
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetSetup() noexcept {
    // Set the output packet header
    gNetOutputPacket[0] = NET_PACKET_HEADER;

    // PsyDoom: logic to establish a connection over TCP for the server and client of the game.
    // If it fails or aborted then abort the game start attempt.
    #if PC_PSX_DOOM_MODS
        const bool bHaveNetConn = (ProgArgs::gbIsNetServer) ? Network::initForServer() : Network::initForClient();

        if (!bHaveNetConn) {
            gbDidAbortGame = true;
            return;
        }
    #endif

    // Player number determination: if the current PlayStation is first in the game (NOT cleared to send) then it becomes Player 1.
    // PC-PSX: use the client/server setting to determine this instead.
    #if PC_PSX_DOOM_MODS
        const bool bIsPlayer1 = ProgArgs::gbIsNetServer;
    #else
        const bool bIsPlayer1 = (!LIBCOMB_CombCTS());
    #endif

    // Read and write a dummy 8 bytes between the two players.
    // Allow the player to abort with the select button also, if a network game is no longer desired.
    if (bIsPlayer1) {
        // Player 1 waits to read from Player 2 firstly
        *gCurPlayerIndex = 0;
        LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);

        // PC-PSX: don't need do any of this anymore, read blocks until complete and clear to send no longer applies!
        #if !PC_PSX_DOOM_MODS
            // Wait until the read is done before continuing, or abort if 'select' is pressed
            do {
                if (LIBETC_PadRead(0) & PAD_SELECT) {
                    gbDidAbortGame = true;
                    LIBCOMB_CombCancelRead();
                    return;
                }
            } while (!LIBAPI_TestEvent(*gSioReadDoneEvent));

            // Wait until we are cleared to send to the receiver
            while (!LIBCOMB_CombCTS()) {}
        #endif

        // Send the dummy packet to the client
        LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);
    } else {
        // Player 2 writes a packet to Player 1 firstly
        *gCurPlayerIndex = true;
        LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);
        LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);

        // PC-PSX: don't need do any of this anymore, read blocks until complete...
        #if !PC_PSX_DOOM_MODS
            // Wait until the read is done before continuing, or abort if 'select' is pressed
            do {
                if (LIBETC_PadRead(0) & PAD_SELECT) {
                    gbDidAbortGame = true;
                    LIBCOMB_CombCancelRead();
                    return;
                }
            } while (!LIBAPI_TestEvent(*gSioReadDoneEvent));
        #endif
    }
    
    // Do a synchronization handshake between the players
    I_NetHandshake();

    // Send the game details if player 1, if player 2 then receive them:
    if (*gCurPlayerIndex == 0) {
        // Fill in the packet details with game type, skill, map and this player's control bindings
        gNetOutputPacket[1] = (uint8_t) gStartGameType;
        gNetOutputPacket[2] = (uint8_t) gStartSkill;
        gNetOutputPacket[3] = (uint8_t) gStartMapOrEpisode;

        const uint32_t thisPlayerBtns = I_LocalButtonsToNet(gCtrlBindings.get());
        gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
        gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
        gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
        gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);

        // Wait until cleared to send then send the packet.
        // PC-PSX: clear to send wait no longer required.
        #if !PC_PSX_DOOM_MODS
            while (!LIBCOMB_CombCTS()) {}
        #endif

        LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);

        // Read the control bindings for the other player and wait until it is read.
        // PC-PSX: read already blocks, no need to wait.
        LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);

        #if !PC_PSX_DOOM_MODS
            while (!LIBAPI_TestEvent(*gSioReadDoneEvent)) {}
        #endif

        const uint32_t otherPlayerBtns = (
            ((uint32_t) gNetInputPacket[4] << 0) |
            ((uint32_t) gNetInputPacket[5] << 8) |
            ((uint32_t) gNetInputPacket[6] << 16) |
            ((uint32_t) gNetInputPacket[7] << 24)
        );

        // Save the control bindings for both players
        gpPlayerCtrlBindings[0] = ptrToVmAddr(gCtrlBindings.get());
        gpPlayerCtrlBindings[1] = ptrToVmAddr(I_NetButtonsToLocal(otherPlayerBtns));
    }
    else {
        // Read the game details and control bindings for the other player and wait until it is read.
        // PC-PSX: read already blocks, no need to wait.
        LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);

        #if !PC_PSX_DOOM_MODS
            while (!LIBAPI_TestEvent(*gSioReadDoneEvent)) {}
        #endif
        
        // Save the game details and the control bindings
        const uint32_t otherPlayerBtns = (
            ((uint32_t) gNetInputPacket[4] << 0) |
            ((uint32_t) gNetInputPacket[5] << 8) |
            ((uint32_t) gNetInputPacket[6] << 16) |
            ((uint32_t) gNetInputPacket[7] << 24)
        );

        gStartGameType = (gametype_t) gNetInputPacket[1];
        gStartSkill = (skill_t) gNetInputPacket[2];
        gStartMapOrEpisode = gNetInputPacket[3];
        gpPlayerCtrlBindings[0] = ptrToVmAddr(I_NetButtonsToLocal(otherPlayerBtns));
        gpPlayerCtrlBindings[1] = ptrToVmAddr(gCtrlBindings.get());

        // For the output packet send the control bindings of this player to the other player
        const uint32_t thisPlayerBtns = I_LocalButtonsToNet(gCtrlBindings.get());
        gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
        gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
        gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
        gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);
        
        // Wait until we are cleared to send to the receiver and send the output packet.
        // PC-PSX: clear to send wait no longer required.
        #if !PC_PSX_DOOM_MODS
            while (!LIBCOMB_CombCTS()) {}
        #endif
        
        LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);
    }

    // PC-PSX: one last check to see if the network connection was killed.
    // This will happen if an error occurred, and if this is the case then we should abort the connection attempt:
    #if PC_PSX_DOOM_MODS
        if (!Network::isConnected()) {
            gbDidAbortGame = true;
            return;
        }
    #endif

    gbDidAbortGame = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sends the packet for the current frame in a networked game and receives the packet from the other player.
// Also does error checking, to make sure that the connection is still OK.
// Returns 'true' if a network error has occurred.
//------------------------------------------------------------------------------------------------------------------------------------------
bool I_NetUpdate() noexcept {
    // The 1st two bytes of the network packet are for error detection, header and position sanity check:
    mobj_t& player1Mobj = *gPlayers[0].mo;
    mobj_t& player2Mobj = *gPlayers[1].mo;

    gNetOutputPacket[0] = NET_PACKET_HEADER;
    gNetOutputPacket[1] = (uint8_t)(player1Mobj.x ^ player1Mobj.y ^ player2Mobj.x ^ player2Mobj.y);

    // Send the elapsed tics for this player and the buttons pressed
    gNetOutputPacket[2] = (uint8_t)(gPlayersElapsedVBlanks[*gCurPlayerIndex]);

    const uint32_t thisPlayerBtns = gTicButtons[*gCurPlayerIndex];
    gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
    gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
    gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
    gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);

    I_NetSendRecv();

    // See if the packet we received from the other player is what we expect.
    // If it isn't then show a 'network error' message:
    const bool bNetworkError = (
        (gNetInputPacket[0] != NET_PACKET_HEADER) ||
        (gNetInputPacket[1] != gNetOutputPacket[1]) ||
        // PC-PSX: connection loss is also a network error
        #if PC_PSX_DOOM_MODS
            (!Network::isConnected())
        #endif
    );

    if (bNetworkError) {
        // Uses the current image as the basis for the next frame; copy the presented framebuffer to the drawing framebuffer:
        LIBGPU_DrawSync(0);
        LIBGPU_MoveImage(
            gDispEnvs[*gCurDispBufferIdx].disp,
            gDispEnvs[*gCurDispBufferIdx ^ 1].disp.x,
            gDispEnvs[*gCurDispBufferIdx ^ 1].disp.y
        );

        // Show the 'Network error' plaque
        I_IncDrawnFrameCount();
        I_CacheTex(*gTex_NETERR);
        I_DrawSprite(
            gTex_NETERR->texPageId,
            gPaletteClutIds[UIPAL],
            84,
            109,
            gTex_NETERR->texPageCoordX,
            gTex_NETERR->texPageCoordY,
            gTex_NETERR->width,
            gTex_NETERR->height
        );

        I_SubmitGpuCmds();
        I_DrawPresent();

        // Try and do a sync handshake between the players
        I_NetHandshake();

        // Clear the other player's buttons, and this player's previous buttons (not sure why that would matter)
        gTicButtons[1] = 0;
        gOldTicButtons[0] = 0;

        // PC-PSX: wait for 2 seconds so the network error can be displayed.
        // When done clear the screen so the 'loading' message displays clearly and not overlapped with the 'network error' message:
        #if PC_PSX_DOOM_MODS
            Utils::waitForSeconds(2.0f);
            I_DrawPresent();
        #endif

        // There was a network error!
        return true;
    }

    // Read and save the buttons for the other player and their elapsed vblank count.
    // Note that the vblank count for player 1 is what determines the speed of the game for both players.
    const uint32_t otherPlayerBtns = (
        ((uint32_t) gNetInputPacket[4] << 0) |
        ((uint32_t) gNetInputPacket[5] << 8) |
        ((uint32_t) gNetInputPacket[6] << 16) |
        ((uint32_t) gNetInputPacket[7] << 24)
    );

    if (*gCurPlayerIndex == 0) {
        gTicButtons[1] = otherPlayerBtns;
        gPlayersElapsedVBlanks[1] = gNetInputPacket[2];
    } else {
        gTicButtons[0] = otherPlayerBtns;
        gPlayersElapsedVBlanks[0] = gNetInputPacket[2];
    }

    // No network error occured
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs a synchronization handshake between the two PlayStations involved in a networked game over Serial Cable.
// Sends a sequence of expected 8 bytes, and expects to receive the same 8 bytes back.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetHandshake() noexcept {
    // PC-PSX: this is no longer neccessary and is in fact wasteful since the underlying protocol (TCP)
    // now guarantees correct ordering and reliability for packets - disable:
    #if !PC_PSX_DOOM_MODS
        // Send the values 0-7 and verify we get the same values back
        uint8_t syncByte = 0;

        while (syncByte < 8) {
            // Send the sync byte and get the other one back
            gNetOutputPacket[0] = syncByte;
            I_NetSendRecv();
        
            // Is it what we expected? If it isn't then start over, otherwise move onto the next sync byte:
            if (gNetInputPacket[0] == gNetOutputPacket[0]) {
                syncByte++;
            } else {
                syncByte = 0;
            }
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sends and receives a single packet of data between the two players in a networked game (over the serial link cable)
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetSendRecv() noexcept {
    while (true) {
        // Which of the two players are we doing comms for?
        if (*gCurPlayerIndex == 0) {
            // Player 1: start by waiting until we are clear to send.
            // PC-PSX: No wait for clear to send here.
            #if !PC_PSX_DOOM_MODS
                while (!LIBCOMB_CombCTS()) {}
            #endif
            
            // Write the output packet
            LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);

            // Read the input packet and wait until it's done.
            // Timeout after 5 seconds and retry the entire send/receive procedure.
            LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);

            // PC-PSX: don't need to wait here - read now blocks itself:
            #if PC_PSX_DOOM_MODS
                break;
            #else
                const int32_t startVBlanks = LIBETC_VSync(-1);
            
                while (true) {
                    // If the read is done then we can finish up this round of sending/receiving
                    if (LIBAPI_TestEvent(*gSioReadDoneEvent))
                        return;

                    // Timeout after 5 seconds if the read still isn't done...
                    if (LIBETC_VSync(-1) - startVBlanks >= VBLANKS_PER_SEC * 5)
                        break;
                }
            #endif
        } else {
            // Player 2: start by reading the input packet
            LIBAPI_read(*gNetInputFd, gNetInputPacket.get(), NET_PACKET_SIZE);
            
            // Wait until the input packet is read.
            // Timeout after 5 seconds and retry the entire send/receive procedure.
            // PC-PSX: no need to wait since the read operation already blocks, go straight to sending.
            #if PC_PSX_DOOM_MODS
                LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);
                break;
            #else
                const int32_t startVBlanks = LIBETC_VSync(-1);

                while (true) {
                    // Is the input packet done yet?
                    if (LIBAPI_TestEvent(*gSioReadDoneEvent)) {
                        // Yes we read it! Write the output packet and finish up once we are clear to send.
                        while (!LIBCOMB_CombCTS()) {}
                        LIBAPI_write(*gNetOutputFd, gNetOutputPacket.get(), NET_PACKET_SIZE);
                        return;
                    }

                    // Timeout after 5 seconds if the read still isn't done...
                    if (LIBETC_VSync(-1) - startVBlanks >= VBLANKS_PER_SEC * 5)
                        break;
                }
            #endif
        }

        // Clear the error bits before we retry the send again
        LIBCOMB_CombResetError();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submits any pending draw primitives in the gpu commands buffer to the GPU
//------------------------------------------------------------------------------------------------------------------------------------------
void I_SubmitGpuCmds() noexcept {
    // Submit the primitives list to the GPU if it's not empty
    if (gpGpuPrimsBeg != gpGpuPrimsEnd) {
        // Note: this marks the end of the primitive list, by setting the 'tag' field of an invalid primitive to 0xFFFFFF.
        // This is similar to LIBGPU_TermPrim, except we don't bother using a valid primitive struct.
        ((uint32_t*) gpGpuPrimsEnd)[0] = 0x00FFFFFF;
        LIBGPU_DrawOTag(gpGpuPrimsBeg);
    }

    // Clear the primitives list
    gpGpuPrimsBeg = gGpuCmdsBuffer;
    gpGpuPrimsEnd = gGpuCmdsBuffer;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Encodes the given control bindings into a 32-bit integer suitable for network/link-cable transmission
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t I_LocalButtonsToNet(const padbuttons_t pCtrlBindings[NUM_CTRL_BINDS]) noexcept {
    // Encode each control binding using 4-bits.
    // Technically it could be done in 3 bits since there are only 8 possible buttons, but 8 x 4-bits will still fit in the 32-bits also.
    uint32_t encodedBindings = 0;

    for (int32_t bindingIdx = 0; bindingIdx < NUM_CTRL_BINDS; ++bindingIdx) {
        // Find out which button this action is bound to
        int32_t buttonIdx = 0;

        for (; buttonIdx < NUM_BINDABLE_BTNS; ++buttonIdx) {
            // Is this the button used for this action?
            if (gBtnMasks[buttonIdx] == pCtrlBindings[bindingIdx])
                break;
        }
        
        // Encode the button using a 4 bit slot in the 32-bit integer
        encodedBindings |= buttonIdx << (bindingIdx * 4);
    }

    return encodedBindings;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// The reverse operation to 'I_LocalButtonsToNet'.
// Decodes the given 32-bit integer containing control bindings for the other player in the networked game.
// The bindings are saved to the 'gOtherPlayerCtrlBindings' global array and this is also what is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
padbuttons_t* I_NetButtonsToLocal(const uint32_t encodedBindings) noexcept {
    for (int32_t bindingIdx = 0; bindingIdx < NUM_CTRL_BINDS; ++bindingIdx) {
        // PC-PSX: use '7' as the mask here, because there are only 7 bindable buttons.
        // Also assert the assumption here that there are only 8 buttons which can be bound.
        static_assert(NUM_BINDABLE_BTNS == 8);

        #if PC_PSX_DOOM_MODS
            const uint32_t buttonIdx = (encodedBindings >> (bindingIdx * 4)) & 7;
        #else
            const uint32_t buttonIdx = (encodedBindings >> (bindingIdx * 4)) & 0xF;
        #endif

        gOtherPlayerCtrlBindings[bindingIdx] = gBtnMasks[buttonIdx];
    }

    return gOtherPlayerCtrlBindings.get();
}

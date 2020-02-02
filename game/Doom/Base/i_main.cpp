#include "i_main.h"

#include "d_vsprintf.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Renderer/r_data.h"
#include "PcPsx/Finally.h"
#include "PcPsx/Video.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBCOMB.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "PsyQ/LIBSN.h"
#include "w_wad.h"
#include "z_zone.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Texture cache related constants and data structures.
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
constexpr uint32_t NUM_TCACHE_PAGES         = 11;
constexpr uint32_t TCACHE_CELL_SIZE         = 16;
constexpr uint32_t TCACHE_CELLS_X           = 256 / TCACHE_CELL_SIZE;
constexpr uint32_t TCACHE_CELLS_Y           = 256 / TCACHE_CELL_SIZE;
constexpr uint32_t NUM_TCACHE_PAGE_CELLS    = TCACHE_CELLS_X * TCACHE_CELLS_Y;
constexpr uint32_t ALL_TPAGES_MASK          = (UINT32_MAX >> (32 - NUM_TCACHE_PAGES));

struct tcachepage_t {
    VmPtr<texture_t> cells[TCACHE_CELLS_Y][TCACHE_CELLS_X];
};

static_assert(sizeof(tcachepage_t) == 1024);

struct tcache_t {
    tcachepage_t pages[NUM_TCACHE_PAGES];
};

static_assert(sizeof(tcache_t) == 1024 * 11);

// The texture coordinates of each texture cache page in VRAM. 
// Notes:
//  (1) Since the PsyQ SDK expects these coordinates to be in terms of a 16-bit texture, X is halved here
//      because Doom's textures are actually 8 bit.
//  (2) The coordinates ignore the first 4 pages in VRAM, since that is used for the framebuffer.
//  (3) The extra 'reserved' texture page that is not used in the cache is also here too, hence 1 extra coordinate.
//      TODO: find out more about this.
//
constexpr uint16_t TEX_PAGE_VRAM_TEXCOORDS[NUM_TCACHE_PAGES + 1][2] = {
    512,    0,
    640,    0,
    768,    0,
    896,    0,
    0,      256,
    128,    256,
    256,    256,
    384,    256,
    512,    256,
    640,    256,
    768,    256,
    896,    256
};

// Texture cache variables.
// The texture cache data structure, where we are filling in the cache next and the current fill row height.
static const VmPtr<VmPtr<tcache_t>>     gpTexCache(0x80077F74);
static const VmPtr<uint32_t>            gTCacheFillPage(0x80078028);
static const VmPtr<uint32_t>            gTCacheFillCellX(0x800782E4);
static const VmPtr<uint32_t>            gTCacheFillCellY(0x800782E8);
static const VmPtr<uint32_t>            gTCacheFillRowCellH(0x80078278);

// A 64-KB buffer used for WAD loading and other stuff
const VmPtr<std::byte[TMP_BUFFER_SIZE]> gTmpBuffer(0x80098748);

// Video vblank timers: track the total amount, last total and current elapsed amount
const VmPtr<uint32_t> gTotalVBlanks(0x80077E98);
const VmPtr<uint32_t> gLastTotalVBlanks(0x80078114);
const VmPtr<uint32_t> gElapsedVBlanks(0x800781BC);

// Used to tell when the texture cache overflows.
// Each texture when added to the cache is assigned the current value of this number.
// When the time comes to evict a texture to make room for another, we check to make sure that the texture wasn't loaded in the current frame.
// If the evicted texture WAS loaded in the current frame, then it means we've run out of texture memory and can't draw all of the textures in the frame.
const VmPtr<uint32_t> gNumFramesDrawn(0x80077C10);

// A bit mask of which texture pages (past the initial 4 which are reserved for the framebuffer) are 'locked' during gameplay.
// Locked texture pages are not allowed to be unloaded and are used for UI sprites, wall and floor textures.
// Sprites on the other hand are placed in 'unlocked' texture pages and can be evicted at any time.
const VmPtr<uint32_t> gLockedTexPagesMask(0x80077C08);

// The index of the user's player in the array of players: whether you are player 1 or 2 in other words
const VmPtr<uint32_t> gCurPlayerIndex(0x80077618);

// Control related stuff
const VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>                         gBtnBindings(0x80073E0C);
const VmPtr<VmPtr<padbuttons_t[NUM_PAD_BINDINGS]>[MAXPLAYERS]>      gpPlayerBtnBindings(0x80077FC8);

//------------------------------------------------------------------------------------------------------------------------------------------
// User PlayStation entrypoint for DOOM.
// This was probably the actual 'main()' function in the real source code.
// I'm just calling 'I_Main()' so as not to confuse it with our this port's 'main()'... 
//------------------------------------------------------------------------------------------------------------------------------------------
void I_Main() noexcept {
    // PsyQ SDK initialization stuff
    LIBSN__main();

    #if PC_PSX_DOOM_MODS
        PcPsx::initVideo();
    #endif

    D_DoomMain();

    #if PC_PSX_DOOM_MODS
        PcPsx::shutdownVideo();
    #endif
}

void I_PSXInit() noexcept {
loc_80032934:
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    LIBETC_ResetCallback();
    a0 = 0;                                             // Result = 00000000
    LIBGPU_ResetGraph();
    a0 = 0;                                             // Result = 00000000
    LIBGPU_SetGraphDebug();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7788;                                       // Result = gPadInputBuffer_1[0] (80097788)
    a1 = 0x22;                                          // Result = 00000022
    a2 = 0x80090000;                                    // Result = 80090000
    a2 += 0x78EC;                                       // Result = gPadInputBuffer_2[0] (800978EC)
    a3 = 0x22;                                          // Result = 00000022
    LIBAPI_InitPAD();
    s0 = 0xF0;                                          // Result = 000000F0
    LIBAPI_StartPAD();
    a0 = 0;                                             // Result = 00000000
    LIBAPI_ChangeClearPAD();
    s2 = 1;                                             // Result = 00000001
    LIBGTE_InitGeom();
    a0 = 0x80;                                          // Result = 00000080
    LIBGTE_SetGeomScreen();
    a0 = 0x80;                                          // Result = 00000080
    a1 = 0x64;                                          // Result = 00000064
    LIBGTE_SetGeomOffset();
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x6F54;                                       // Result = gDrawEnv1[0] (800A90AC)
    a0 = s1;                                            // Result = gDrawEnv1[0] (800A90AC)
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0x100;                                         // Result = 00000100
    sw(s0, sp + 0x10);
    LIBGPU_SetDefDrawEnv();
    a0 = s1 + 0x5C;                                     // Result = gDrawEnv2[0] (800A9108)
    a1 = 0x100;                                         // Result = 00000100
    a2 = 0;                                             // Result = 00000000
    a3 = 0x100;                                         // Result = 00000100
    at = 0x800B0000;                                    // Result = 800B0000
    sb(s2, at - 0x6F3C);                                // Store to: gDrawEnv1[C] (800A90C4)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(0, at - 0x6F3E);                                 // Store to: gDrawEnv1[B] (800A90C2)
    sw(s0, sp + 0x10);
    LIBGPU_SetDefDrawEnv();
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    a0 = s1;                                            // Result = gDispEnv1[0] (800A9164)
    a1 = 0x100;                                         // Result = 00000100
    a2 = 0;                                             // Result = 00000000
    a3 = 0x100;                                         // Result = 00000100
    at = 0x800B0000;                                    // Result = 800B0000
    sb(s2, at - 0x6EE0);                                // Store to: gDrawEnv2[C] (800A9120)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(0, at - 0x6EE2);                                 // Store to: gDrawEnv2[B] (800A911E)
    sw(s0, sp + 0x10);
    LIBGPU_SetDefDispEnv();
    a0 = s1 + 0x14;                                     // Result = gDispEnv2[0] (800A9178)
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0x100;                                         // Result = 00000100
    sw(s0, sp + 0x10);
    LIBGPU_SetDefDispEnv();
    sw(0, gp + 0xB18);                                  // Store to: gCurDrawDispBufferIdx (800780F8)
    LIBAPI_EnterCriticalSection();
    LIBAPI_ExitCriticalSection();
    LIBCOMB_AddCOMB();
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 0xB;                                          // Result = F000000B
    a1 = 0x400;                                         // Result = 00000400
    a2 = 0x2000;                                        // Result = 00002000
    a3 = 0;                                             // Result = 00000000
    LIBAPI_OpenEvent();
    a0 = v0;
    sw(a0, gp + 0x944);                                 // Store to: gSioErrorEvent (80077F24)
    LIBAPI_EnableEvent();
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 0xB;                                          // Result = F000000B
    a1 = 0x800;                                         // Result = 00000800
    a2 = 0x2000;                                        // Result = 00002000
    a3 = 0;                                             // Result = 00000000
    LIBAPI_OpenEvent();
    a0 = v0;
    sw(a0, gp + 0xA60);                                 // Store to: gSioWriteDoneEvent (80078040)
    LIBAPI_EnableEvent();
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x7C1C;                                       // Result = STR_sio_3[0] (80077C1C)
    a0 = s0;                                            // Result = STR_sio_3[0] (80077C1C)
    a1 = 2;                                             // Result = 00000002
    LIBAPI_open();
    a0 = s0;                                            // Result = STR_sio_3[0] (80077C1C)
    sw(v0, gp + 0x934);                                 // Store to: gNetOutputFd (80077F14)
    a1 = 0x8001;                                        // Result = 00008001
    LIBAPI_open();
    a0 = 1;                                             // Result = 00000001
    a1 = 3;                                             // Result = 00000003
    sw(v0, gp + 0xC54);                                 // Store to: gNetInputFd (80078234)
    a2 = 0x9600;                                        // Result = 00009600
    LIBCOMB__comb_control();
    I_DrawPresent();
    I_DrawPresent();
    a0 = 1;                                             // Result = 00000001
    LIBGPU_SetDispMask();
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept {
    sp -= 0x120;
    sw(s0, sp + 0x118);
    sw(a0, sp + 0x120);

    {
        va_list args;
        va_start(args, fmtMsg);
        v0 = D_vsprintf(vmAddrToPtr<char>(sp + 0x18), fmtMsg, args);
        va_end(args);
    }

    I_DrawPresent();

    a0 = 0x3C0;
    a1 = 0x100;
    LIBGPU_FntLoad();

    sw(0, sp + 0x10);
    sw(0x100, sp + 0x14);

    a0 = 0;
    a1 = 0;
    a2 = 0x100;
    a3 = 0xC8;
    LIBGPU_FntOpen();
    s0 = v0;

    a0 = s0;
    LIBGPU_SetDumpFnt();

    a0 = 0x80077C24;    // Result = STR_I_Error_PrintToScreenFmtStr[0] (80077C24)
    a1 = sp + 0x18;
    LIBGPU_FntPrint();

    a0 = s0;
    LIBGPU_FntFlush();

    I_DrawPresent();

    // TODO: PC-PSX: allow the PC client to exit the app from here
    while (true) {
        // Deliberate infinite loop...
    }

    s0 = lw(sp + 0x118);
    sp += 0x120;
}

void I_ReadGamepad() noexcept {
loc_80032BB8:
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7788);                               // Load from: gPadInputBuffer_1[0] (80097788)
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
    a0 = v1 ^ v0;
    v1 = a0 & 0xF0FF;
    v0 = 0x4000;                                        // Result = 00004000
    {
        const bool bJump = (v1 != v0);
        v1 = a0 >> 24;
        if (bJump) goto loc_80032BE8;
    }
    v0 = a0 >> 8;
    v0 &= 0xFF00;
    a0 = v0 | v1;
    goto loc_80032BEC;
loc_80032BE8:
    a0 = 0;                                             // Result = 00000000
loc_80032BEC:
    v0 = a0;
    return;
}

void I_CacheTexForLumpName() noexcept {
loc_80032BF4:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a0 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    if (a0 == 0) goto loc_80032C24;
    v0 = W_GetNumForName(vmAddrToPtr<const char>(a0));
    s2 = v0;
loc_80032C24:
    a0 = s2;
    a1 = 0x20;                                          // Result = 00000020
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpNum();
    v1 = *gpbIsUncompressedLump;
    v1 += s2;
    v1 = lbu(v1);
    a0 = v0;
    if (v1 != 0) goto loc_80032C68;
    s0 = gTmpBuffer;
    a1 = gTmpBuffer;
    _thunk_decode();
    a0 = gTmpBuffer;
loc_80032C68:
    v0 = lhu(a0);
    sh(v0, s1);
    v0 = lhu(a0 + 0x2);
    sh(v0, s1 + 0x2);
    v0 = lhu(a0 + 0x4);
    sh(v0, s1 + 0x4);
    v0 = lhu(a0 + 0x6);
    sh(v0, s1 + 0x6);
    v1 = lh(a0 + 0x4);
    v0 = v1 + 0xF;
    {
        const bool bJump = (i32(v0) >= 0);
        v0 = u32(i32(v0) >> 4);
        if (bJump) goto loc_80032CB4;
    }
    v0 = v1 + 0x1E;
    v0 = u32(i32(v0) >> 4);
loc_80032CB4:
    sh(v0, s1 + 0xC);
    a0 = lh(a0 + 0x6);
    v0 = a0 + 0xF;
    if (i32(v0) >= 0) goto loc_80032CD0;
    v0 = a0 + 0x1E;
loc_80032CD0:
    a0 = s1;
    v0 = u32(i32(v0) >> 4);
    sh(v0, a0 + 0xE);
    sh(0, a0 + 0xA);
    sh(s2, a0 + 0x10);
    _thunk_I_CacheTex();
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void I_CacheAndDrawSprite() noexcept {
loc_80032D04:
    sp -= 0x38;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s2, sp + 0x28);
    s2 = a1;
    sw(s3, sp + 0x2C);
    s3 = a2;
    sw(s1, sp + 0x24);
    sw(ra, sp + 0x30);
    s1 = a3;
    _thunk_I_CacheTex();
    v0 = lbu(s0 + 0x8);
    a0 = lhu(s0 + 0xA);
    sw(v0, sp + 0x10);
    v0 = lbu(s0 + 0x9);
    a1 = s1;
    sw(v0, sp + 0x14);
    v0 = lh(s0 + 0x4);
    a2 = s2;
    sw(v0, sp + 0x18);
    v0 = lh(s0 + 0x6);
    a3 = s3;
    sw(v0, sp + 0x1C);
    I_DrawSprite();
    ra = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void I_DrawSprite() noexcept {
loc_80032D84:
    sp -= 0x40;
    v0 = a0;
    sw(s3, sp + 0x24);
    s3 = a1;
    sw(s1, sp + 0x1C);
    s1 = a2;
    sw(s2, sp + 0x20);
    s2 = a3;
    sw(s0, sp + 0x18);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    sw(s4, sp + 0x28);
    s4 = lw(sp + 0x50);
    a0 = s0;                                            // Result = 1F800200
    sw(s5, sp + 0x2C);
    s5 = lw(sp + 0x54);
    a1 = 0;                                             // Result = 00000000
    sw(s6, sp + 0x30);
    s6 = lw(sp + 0x58);
    a2 = 0;                                             // Result = 00000000
    sw(s7, sp + 0x34);
    s7 = lw(sp + 0x5C);
    a3 = v0;
    sw(ra, sp + 0x38);
    sw(0, sp + 0x10);
    _thunk_LIBGPU_SetDrawMode();
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
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80032E24:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80032E78;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80032F24;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(t7, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80032E78:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80032F18;
    if (v1 == a0) goto loc_80032E24;
loc_80032E98:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80032E24;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t3;
    v0 |= t5;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_80032EFC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80032EE0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80032EE0;
loc_80032EFC:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80032E24;
    goto loc_80032E98;
loc_80032F18:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80032F24:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80032F7C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80032F64:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80032F64;
loc_80032F7C:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_80033018;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80032FA0:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_80033018;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_80033004;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80032FE8:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80032FE8;
loc_80033004:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80032FA0;
loc_80033018:
    t0 = 4;                                             // Result = 00000004
    t2 = 0x1F800000;                                    // Result = 1F800000
    t2 += 0x204;                                        // Result = 1F800204
    t3 = 0x10;                                          // Result = 00000010
    t4 = 0x14;                                          // Result = 00000014
    t1 = 0xFF0000;                                      // Result = 00FF0000
    t1 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = s0 & t1;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    t6 = 0x80000000;                                    // Result = 80000000
    t5 = -1;                                            // Result = FFFFFFFF
    a3 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x64;                                          // Result = 00000064
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0x80;                                          // Result = 00000080
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s1, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s2, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sb(s4, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(s5, at + 0x20D);                                 // Store to: 1F80020D
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s6, at + 0x210);                                 // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s7, at + 0x212);                                 // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s3, at + 0x20E);                                 // Store to: 1F80020E
loc_800330BC:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t3 + a0;
        if (bJump) goto loc_80033110;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t4 + a0;
        if (bJump) goto loc_800331BC;
    }
    v0 = lw(a3);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80033110:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t3 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800331B0;
    if (v1 == a0) goto loc_800330BC;
loc_80033130:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_800330BC;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t1;
    v0 |= t6;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_80033194;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80033178:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80033178;
loc_80033194:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800330BC;
    goto loc_80033130;
loc_800331B0:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t4;
loc_800331BC:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;                                               // Result = 00000003
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_80033214;
    v1 = -1;                                            // Result = FFFFFFFF
loc_800331FC:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_800331FC;
loc_80033214:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_800332B0;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80033238:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_800332B0;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_8003329C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80033280:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80033280;
loc_8003329C:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80033238;
loc_800332B0:
    ra = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}

void I_DrawPlaque() noexcept {
loc_800332E0:
    sp -= 0x40;
    sw(s0, sp + 0x28);
    s0 = a0;
    sw(s2, sp + 0x30);
    s2 = a1;
    sw(s3, sp + 0x34);
    s3 = a2;
    sw(s1, sp + 0x2C);
    s1 = a3;
    sw(ra, sp + 0x38);
    a0 = 0;                                             // Result = 00000000
    LIBGPU_DrawSync();
    a3 = lw(gp + 0xB18);                                // Load from: gCurDrawDispBufferIdx (800780F8)
    v1 = a3 ^ 1;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    a0 = a3 << 2;
    a0 += a3;
    a0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    at += v0;
    a1 = lh(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6E9A;                                       // Result = gDispEnv1[1] (800A9166)
    at += v0;
    a2 = lh(at);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    a0 += v0;
    LIBGPU_MoveImage();
    I_IncDrawnFrameCount();
    a0 = s0;
    _thunk_I_CacheTex();
    v0 = lbu(s0 + 0x8);
    a0 = lhu(s0 + 0xA);
    sw(v0, sp + 0x10);
    v0 = lbu(s0 + 0x9);
    a1 = s1;
    sw(v0, sp + 0x14);
    v0 = lh(s0 + 0x4);
    a2 = s2;
    sw(v0, sp + 0x18);
    v0 = lh(s0 + 0x6);
    a3 = s3;
    sw(v0, sp + 0x1C);
    I_DrawSprite();
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Increments the 'drawn frame' counter used to track texture cache overflows
//------------------------------------------------------------------------------------------------------------------------------------------
void I_IncDrawnFrameCount() noexcept {
    ++*gNumFramesDrawn;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Swaps the draw and display framebuffers, causing the current frame being rendered to be displayed onscreen.
// Also does framerate limiting to 30 Hz and updates the elapsed vblank count, which feeds the game's timing system.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawPresent() noexcept {
    // Finish up all in-flight drawing commands
    a0 = 0;
    LIBGPU_DrawSync();

    // PC-PSX: this interferes with frame pacing in the new host environment - disabling
    #if !PC_PSX_DOOM_MODS
        // Wait for a vblank to occur
        LIBETC_VSync(0);
    #endif

    // Swap the framebuffers
    v1 = lw(0x800780F8);            // Load from: gCurDrawDispBufferIdx (800780F8)    
    v1 ^= 1;
    sw(v1, 0x800780F8);             // Store to: gCurDrawDispBufferIdx (800780F8)    
    a0 = 0x800A90AC + v1 * 92;      // gDrawEnv1[0] (800A90AC)
    LIBGPU_PutDrawEnv();

    v0 = lw(0x800780F8);            // Load from: gCurDrawDispBufferIdx (800780F8)
    a0 = 0x800A9164 + v0 * 20;      // gDispEnv1[0] (800A9164)
    LIBGPU_PutDispEnv();

    // Frame rate limiting to 30 Hz.
    // Continously poll and wait until at least 2 vblanks have elapsed before continuing.
    do {
        // PC-PSX: wait one 30 Hz tick
        #if PC_PSX_DOOM_MODS
            PcPsx::doFrameRateLimiting();
        #endif

        *gTotalVBlanks = LIBETC_VSync(-1);
        *gElapsedVBlanks = *gTotalVBlanks - *gLastTotalVBlanks;
    } while (*gElapsedVBlanks < 2);

    // Further framerate limiting for demos:
    // Demo playback or recording is forced to run at 15 Hz all of the time (the game simulation rate).
    // Probably done so the simulation remains consistent!
    if (*gbDemoPlayback || *gbDemoRecording) {
        while (*gElapsedVBlanks < 4) {
            // PC-PSX: wait one 30 Hz tick
            #if PC_PSX_DOOM_MODS
                PcPsx::doFrameRateLimiting();
            #endif

            *gTotalVBlanks = LIBETC_VSync(-1);
            *gElapsedVBlanks = *gTotalVBlanks - *gLastTotalVBlanks;
        }

        *gElapsedVBlanks = 4;
    }

    // So we can compute the elapsed vblank amount next time round
    *gLastTotalVBlanks = *gTotalVBlanks;

    // PC-PSX: copy the PSX framebuffer to the display
    #if PC_PSX_DOOM_MODS
        PcPsx::displayFramebuffer();
    #endif
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
    *gpTexCache = (tcache_t*) Z_Malloc(**gpMainMemZone, sizeof(tcache_t), PU_STATIC, nullptr);
    D_memset(gpTexCache->get(), (std::byte) 0, sizeof(tcache_t));
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
        if (*gTCacheFillCellX + tex.widthIn16Blocks > TCACHE_CELLS_X) {
            *gTCacheFillCellY = *gTCacheFillCellY + *gTCacheFillRowCellH;
            *gTCacheFillCellX = 0;
            *gTCacheFillRowCellH = 0;
        }
        
        // Move onto another page in the texture cache if this page can't accomodate the texture.
        // Find one that is not locked and which is available for modification.
        if (*gTCacheFillCellY + tex.heightIn16Blocks > TCACHE_CELLS_Y) {            
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
            
            for (int32_t y = 0; y < tex.heightIn16Blocks; ++y) {
                for (int32_t x = 0; x < tex.widthIn16Blocks; ++x) {
                    // Check to see if this cell is empty and move past it.
                    // If it's already empty then we don't need to do anything:
                    texture_t* const pCellTex = pCacheEntry->get();
                    ++pCacheEntry;

                    if (!pCellTex)
                        continue;

                    // Cell is not empty! If the texture in the cell is in use for this frame then we can't evict it.
                    // In this case skip past the texture and try again:
                    if (pCellTex->uploadFrameNum == *gNumFramesDrawn) {
                        *gTCacheFillCellX += pCellTex->widthIn16Blocks;

                        // We may need to skip onto the next row on retry also, make sure we have the right row height recorded.
                        // The row height is the max of all the texture heights on the row basically:
                        if (*gTCacheFillRowCellH < pCellTex->heightIn16Blocks) {
                            *gTCacheFillRowCellH = pCellTex->heightIn16Blocks;
                        }

                        goto find_free_tcache_location;
                    }

                    // The cell is not empty but we can evict the texture, do that here now
                    a0 = ptrToVmAddr(pCellTex);
                    I_RemoveTexCacheEntry();
                }

                pCacheEntry += TCACHE_CELLS_X - tex.widthIn16Blocks;    // Move onto the next row of cells
            }
        }
    }

    // Fill all of the cells in the cache occupied by this texture with references to it
    {
        VmPtr<texture_t>* pCacheEntry = pTexStartCacheCell;

        for (int32_t y = 0; y < tex.heightIn16Blocks; ++y) {
            for (int32_t x = 0; x < tex.widthIn16Blocks; ++x) {
                *pCacheEntry = &tex;
                ++pCacheEntry;
            }

            pCacheEntry += TCACHE_CELLS_X - tex.widthIn16Blocks;    // Move to the next row of cells
        }
    }

    // Record on the texture where it is located in the cache (top left corner)
    tex.ppTexCacheEntries = ptrToVmAddr(pTexStartCacheCell);

    // Make sure the texture's lump is loaded and decompress if required
    const void* pTexData = W_CacheLumpNum(tex.lumpNum, PU_CACHE, false);
    const bool bIsTexCompressed = (!(*gpbIsUncompressedLump)[tex.lumpNum]);

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

        LIBGPU_LoadImage(dstVramRect, (uint32_t*) pTexData + 2);    // TODO: figure out what 8 bytes is being skipped
    }

    // Save the textures page coordinate
    tex.texPageCoordX = (uint8_t)((*gTCacheFillCellX) * TCACHE_CELL_SIZE);
    tex.texPageCoordY = (uint8_t)((*gTCacheFillCellY) * TCACHE_CELL_SIZE);

    // Get and save the texture page id
    a0 = 1;
    a1 = 0;
    a2 = (*gTCacheFillPage + 4) * 128;
    a3 = TEX_PAGE_VRAM_TEXCOORDS[*gTCacheFillPage][1];
    LIBGPU_GetTPage();
    tex.texPageId = (uint16_t) v0;

    // Advance the fill position in the texture cache.
    // Also expand the fill row height if this texture is taller than the current height.
    *gTCacheFillCellX += tex.widthIn16Blocks;

    if (*gTCacheFillRowCellH < tex.heightIn16Blocks) {
        *gTCacheFillRowCellH = tex.heightIn16Blocks;
    }
}

void _thunk_I_CacheTex() noexcept {
    I_CacheTex(*vmAddrToPtr<texture_t>(a0));
}

void I_RemoveTexCacheEntry() noexcept {
loc_8003390C:
    sp -= 0x10;
    a1 = lw(a0 + 0x14);
    v0 = lh(a0 + 0xE);
    a2 = 0;                                             // Result = 00000000
    sh(0, a0 + 0xA);
    if (i32(v0) <= 0) goto loc_80033970;
    a3 = 0x10;                                          // Result = 00000010
loc_80033928:
    v0 = lh(a0 + 0xC);
    v1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80033950;
loc_80033938:
    sw(0, a1);
    v0 = lh(a0 + 0xC);
    v1++;
    v0 = (i32(v1) < i32(v0));
    a1 += 4;
    if (v0 != 0) goto loc_80033938;
loc_80033950:
    v0 = a3 - v1;
    v0 <<= 2;
    a1 += v0;
    v0 = lh(a0 + 0xE);
    a2++;
    v0 = (i32(a2) < i32(v0));
    if (v0 != 0) goto loc_80033928;
loc_80033970:
    sp += 0x10;
    return;
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

                for (int32_t y = 0; y < tex.heightIn16Blocks; ++y) {
                    for (int32_t x = 0; x < tex.widthIn16Blocks; ++x) {
                        *pTexCacheEntry = nullptr;
                        ++pTexCacheEntry;
                    }

                    pTexCacheEntry += TCACHE_CELLS_X - tex.widthIn16Blocks;     // Move to the next row of cells
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

void I_VramViewerDraw() noexcept {
loc_80033AC4:
    sp -= 0x30;
    sw(s2, sp + 0x18);
    s2 = a0;
    v0 = 9;                                             // Result = 00000009
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x2C;                                          // Result = 0000002C
    v1 = 0x100;                                         // Result = 00000100
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0xF0;                                          // Result = 000000F0
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x21A);                                 // Store to: 1F80021A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x222);                                 // Store to: 1F800222
    v0 = 0xFF;                                          // Result = 000000FF
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x214);                                 // Store to: 1F800214
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x21D);                                 // Store to: 1F80021D
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x224);                                 // Store to: 1F800224
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x225);                                 // Store to: 1F800225
    v0 = 0x80;                                          // Result = 00000080
    a2 = s2 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
    v0 = s2 << 3;
    a0 = 1;                                             // Result = 00000001
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x210);                                 // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x220);                                 // Store to: 1F800220
    v1 = *gPaletteClutId_Main;
    a1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x2C);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x208);                                  // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20A);                                  // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x212);                                  // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x218);                                  // Store to: 1F800218
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x20C);                                  // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x20D);                                  // Store to: 1F80020D
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x215);                                  // Store to: 1F800215
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x21C);                                  // Store to: 1F80021C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3C80;                                       // Result = TexPageVramTexCoords[1] (80073C80)
    at += v0;
    a3 = lw(at);
    a2 <<= 7;
    LIBGPU_GetTPage();
    t4 = 0x1F800000;                                    // Result = 1F800000
    t4 += 0x204;                                        // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s0 & t3;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    t6 = 0x80000000;                                    // Result = 80000000
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t5 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x216);                                 // Store to: 1F800216
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80033C28:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80033C7C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80033D28;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80033C7C:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80033D1C;
    if (v1 == a0) goto loc_80033C28;
loc_80033C9C:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_80033C28;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t3;
    v0 |= t6;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_80033D00;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80033CE4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80033CE4;
loc_80033D00:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80033C28;
    goto loc_80033C9C;
loc_80033D1C:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80033D28:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80033D80;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80033D68:
    v0 = lw(t4);
    t4 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80033D68;
loc_80033D80:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    s5 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80033E20;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80033DA8:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t2;
    s5 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80033E20;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_80033E0C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80033DF0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80033DF0;
loc_80033E0C:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    s5 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_80033DA8;
loc_80033E20:
    s6 = 0x1F800000;                                    // Result = 1F800000
    s6 += 0x200;                                        // Result = 1F800200
    t4 = 0xFF0000;                                      // Result = 00FF0000
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    s3 = 0x80080000;                                    // Result = 80080000
    s3 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s3 & t4;                                       // Result = 00086550
    t9 = 0x4000000;                                     // Result = 04000000
    t8 = 0x80000000;                                    // Result = 80000000
    t5 = -1;                                            // Result = FFFFFFFF
    v1 = lw(gp + 0x994);                                // Load from: gpTexCache (80077F74)
    v0 = s2 << 10;
    s4 = v0 + v1;
loc_80033E54:
    a1 = lw(s4);
    v0 = 3;                                             // Result = 00000003
    if (a1 == 0) goto loc_800346F0;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x40;                                          // Result = 00000040
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0xFF;                                          // Result = 000000FF
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x205);                                  // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x206);                                  // Store to: 1F800206
    v1 = lbu(a1 + 0x9);
    t7 = lbu(a1 + 0x8);
    v0 = v1 << 4;
    v0 -= v1;
    a0 = v0 << 4;
    if (i32(a0) >= 0) goto loc_80033EB4;
    a0 += 0xFF;
loc_80033EB4:
    v1 = lh(a1 + 0x6);
    s0 = lh(a1 + 0x4);
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 4;
    t6 = u32(i32(a0) >> 8);
    if (i32(v0) >= 0) goto loc_80033ED4;
    v0 += 0xFF;
loc_80033ED4:
    s2 = u32(i32(v0) >> 8);
    t0 = 3;                                             // Result = 00000003
    t1 = s6 + 4;                                        // Result = 1F800204
    t2 = 0xC;                                           // Result = 0000000C
    t3 = 0x10;                                          // Result = 00000010
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = t7 + s0;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t6, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t6, at + 0x20E);                                 // Store to: 1F80020E
loc_80033F10:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80033F64;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t3 + a0;
        if (bJump) goto loc_80034010;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s3, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80033F64:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t2 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80034004;
    if (v1 == a0) goto loc_80033F10;
loc_80033F84:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_80033F10;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_80033FE8;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80033FCC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80033FCC;
loc_80033FE8:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80033F10;
    goto loc_80033F84;
loc_80034004:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t3;
loc_80034010:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;                                               // Result = 00000002
    a2 += 4;
    if (t0 == t5) goto loc_800340C8;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80034044:
    v0 = lw(t1);
    t1 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80034044;
    goto loc_800340C8;
loc_80034064:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v0 == 0) goto loc_800340DC;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800340C8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800340AC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800340AC;
loc_800340C8:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v1 != v0) goto loc_80034064;
loc_800340DC:
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = t7 + s0;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = t6 + s2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t6, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20E);                                 // Store to: 1F80020E
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80034118:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003416C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80034218;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s3, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_8003416C:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003420C;
    if (v1 == a0) goto loc_80034118;
loc_8003418C:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_80034118;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800341F0;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800341D4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800341D4;
loc_800341F0:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80034118;
    goto loc_8003418C;
loc_8003420C:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80034218:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t5) goto loc_800342D0;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003424C:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8003424C;
    goto loc_800342D0;
loc_8003426C:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v0 == 0) goto loc_800342E4;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800342D0;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800342B4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800342B4;
loc_800342D0:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v1 != v0) goto loc_8003426C;
loc_800342E4:
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = t7 + s0;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = t6 + s2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20E);                                 // Store to: 1F80020E
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80034320:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80034374;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80034420;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s3, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80034374:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80034414;
    if (v1 == a0) goto loc_80034320;
loc_80034394:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_80034320;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800343F8;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800343DC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800343DC;
loc_800343F8:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80034320;
    goto loc_80034394;
loc_80034414:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80034420:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t5) goto loc_800344D8;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80034454:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80034454;
    goto loc_800344D8;
loc_80034474:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v0 == 0) goto loc_800344EC;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800344D8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800344BC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800344BC;
loc_800344D8:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    t3 = s6 + 4;                                        // Result = 1F800204
    if (v1 != v0) goto loc_80034474;
loc_800344EC:
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = t6 + s2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t6, at + 0x20E);                                 // Store to: 1F80020E
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80034524:
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80034578;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80034624;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    sw(s3, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    a0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
loc_80034578:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80034618;
    if (v1 == a0) goto loc_80034524;
loc_80034598:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_80034524;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800345FC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800345E0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800345E0;
loc_800345FC:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80034524;
    goto loc_80034598;
loc_80034618:
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80034624:
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t5) goto loc_800346DC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80034658:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80034658;
    goto loc_800346DC;
loc_80034678:
    v0 = lw(gp + 0x624);                                // Load from: GPU_REG_GP1 (80077C04)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_800346F0;
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800346DC;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800346C0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x620);                                // Load from: GPU_REG_GP0 (80077C00)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800346C0;
loc_800346DC:
    v1 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v0 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80034678;
loc_800346F0:
    s5++;
    v0 = (i32(s5) < 0x100);
    s4 += 4;
    if (v0 != 0) goto loc_80033E54;
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}

void I_NetSetup() noexcept {
loc_8003472C:
    sp -= 0x20;
    v0 = 0xAA;                                          // Result = 000000AA
    a0 = 3;                                             // Result = 00000003
    a1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    sb(v0, gp + 0x9D0);                                 // Store to: gNetOutputPacket[0] (80077FB0)
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    a2 = 8;                                             // Result = 00000008
    if (v0 == 0) goto loc_800347EC;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    s1 = 0xFFFF0000;                                    // Result = FFFF0000
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    *gCurPlayerIndex = 1;
    s0 = 0x4000;                                        // Result = 00004000
    LIBAPI_write();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_read();
loc_80034794:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7788);                               // Load from: gPadInputBuffer_1[0] (80097788)
    v1 = v0 ^ s1;
    v0 = v1 & 0xF0FF;
    {
        const bool bJump = (v0 != s0);
        v0 = v1 >> 8;
        if (bJump) goto loc_800347C0;
    }
    v0 &= 0xFF00;
    v1 >>= 24;
    v1 |= v0;
    goto loc_800347C4;
loc_800347C0:
    v1 = 0;                                             // Result = 00000000
loc_800347C4:
    v0 = v1 & 0x100;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003496C;
    }
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    if (v0 == 0) goto loc_80034794;
    goto loc_80034884;
loc_800347EC:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    s1 = 0xFFFF0000;                                    // Result = FFFF0000
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    *gCurPlayerIndex = 0;
    s0 = 0x4000;                                        // Result = 00004000
    LIBAPI_read();
loc_8003480C:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7788);                               // Load from: gPadInputBuffer_1[0] (80097788)
    v1 = v0 ^ s1;
    v0 = v1 & 0xF0FF;
    {
        const bool bJump = (v0 != s0);
        v0 = v1 >> 8;
        if (bJump) goto loc_80034838;
    }
    v0 &= 0xFF00;
    v1 >>= 24;
    v1 |= v0;
    goto loc_8003483C;
loc_80034838:
    v1 = 0;                                             // Result = 00000000
loc_8003483C:
    v0 = v1 & 0x100;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003496C;
    }
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    a0 = 3;                                             // Result = 00000003
    if (v0 == 0) goto loc_8003480C;
loc_8003485C:
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    a0 = 3;                                             // Result = 00000003
    if (v0 == 0) goto loc_8003485C;
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_write();
loc_80034884:
    I_NetHandshake();
    v0 = *gCurPlayerIndex;
    if (v0 != 0) goto loc_80034988;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7604);                              // Load from: gStartGameType (80077604)
    v1 = *gStartSkill;
    a1 = (uint8_t) *gStartMapOrEpisode;
    a0 = gBtnBindings;
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x7FB1);                                // Store to: gNetOutputPacket[1] (80077FB1)
    at = 0x80070000;                                    // Result = 80070000
    sb(v1, at + 0x7FB2);                                // Store to: gNetOutputPacket[2] (80077FB2)
    at = 0x80070000;                                    // Result = 80070000
    sb(a1, at + 0x7FB3);                                // Store to: gNetOutputPacket[3] (80077FB3)
    I_LocalButtonsToNet();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FB4);                                // Store to: gNetOutputPacket[4] (80077FB4)
    a0 = 3;                                             // Result = 00000003
loc_800348EC:
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    a0 = 3;                                             // Result = 00000003
    if (v0 == 0) goto loc_800348EC;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_write();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_read();
loc_80034928:
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    if (v0 == 0) goto loc_80034928;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EAC);                               // Load from: gNetInputPacket[4] (80077EAC)
    v0 = gBtnBindings;
    at = 0x80070000;                                    // Result = 80070000
    gpPlayerBtnBindings[0] = v0;
    I_NetButtonsToLocal();
    at = 0x80070000;                                    // Result = 80070000
    gpPlayerBtnBindings[1] = v0;
    goto loc_80034A44;
loc_8003496C:
    sw(v0, gp + 0x62C);                                 // Store to: gbDidAbortGame (80077C0C)
    a0 = 2;
    a1 = 3;
    a2 = 0;
    LIBCOMB__comb_control();
    goto loc_80034A48;
loc_80034988:
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_read();
loc_8003499C:
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    if (v0 == 0) goto loc_8003499C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EAC);                               // Load from: gNetInputPacket[4] (80077EAC)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7EA9);                              // Load from: gNetInputPacket[1] (80077EA9)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x7EAA);                              // Load from: gNetInputPacket[2] (80077EAA)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lbu(a1 + 0x7EAB);                              // Load from: gNetInputPacket[3] (80077EAB)
    s0 = gBtnBindings;
    at = 0x80070000;                                    // Result = 80070000
    gpPlayerBtnBindings[1] = s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7604);                                // Store to: gStartGameType (80077604)
    *gStartSkill = (skill_t) v1;
    *gStartMapOrEpisode = a1;
    I_NetButtonsToLocal();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FC8);                                // Store to: MAYBE_gpButtonBindings_Player1 (80077FC8)
    a0 = s0;
    I_LocalButtonsToNet();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FB4);                                // Store to: gNetOutputPacket[4] (80077FB4)
    a0 = 3;                                             // Result = 00000003
loc_80034A1C:
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    a0 = 3;                                             // Result = 00000003
    if (v0 == 0) goto loc_80034A1C;
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_write();
loc_80034A44:
    sw(0, gp + 0x62C);                                  // Store to: gbDidAbortGame (80077C0C)
loc_80034A48:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void I_NetUpdate() noexcept {
loc_80034A60:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7814);                               // Load from: gPlayer1[0] (800A87EC)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x76E8);                               // Load from: gPlayer2[0] (800A8918)
    sp -= 0x48;
    sw(ra, sp + 0x40);
    sw(s1, sp + 0x3C);
    sw(s0, sp + 0x38);
    v1 = lbu(v0);
    a1 = lbu(v0 + 0x4);
    a2 = lbu(a0);
    a0 = lbu(a0 + 0x4);
    v0 = 0xAA;                                          // Result = 000000AA
    sb(v0, gp + 0x9D0);                                 // Store to: gNetOutputPacket[0] (80077FB0)
    v1 ^= a1;
    v1 ^= a2;
    v1 ^= a0;
    v0 = v1 >> 8;
    v0 ^= v1;
    v1 >>= 16;
    a0 = *gCurPlayerIndex;
    v0 ^= v1;
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x7FB1);                                // Store to: gNetOutputPacket[1] (80077FB1)
    a0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += a0;
    v0 = lbu(at);
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x7FB2);                                // Store to: gNetOutputPacket[2] (80077FB2)
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += a0;
    v0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FB4);                                // Store to: gNetOutputPacket[4] (80077FB4)
    I_NetSendRecv();
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    v1 = lbu(gp + 0x8C8);                               // Load from: gNetInputPacket[0] (80077EA8)
    v0 = 0xAA;                                          // Result = 000000AA
    if (v1 != v0) goto loc_80034B44;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x7EA9);                              // Load from: gNetInputPacket[1] (80077EA9)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7FB1);                              // Load from: gNetOutputPacket[1] (80077FB1)
    if (v1 == v0) goto loc_80034C48;
loc_80034B44:
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 = lh(s0 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a0 = 0;                                             // Result = 00000000
    LIBGPU_DrawSync();
    a3 = lw(gp + 0xB18);                                // Load from: gCurDrawDispBufferIdx (800780F8)
    v1 = a3 ^ 1;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    a0 = a3 << 2;
    a0 += a3;
    a0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    at += v0;
    a1 = lh(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6E9A;                                       // Result = gDispEnv1[1] (800A9166)
    at += v0;
    a2 = lh(at);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    a0 += v0;
    LIBGPU_MoveImage();
    I_IncDrawnFrameCount();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AF0;                                       // Result = gTexInfo_NETERR[0] (80097AF0)
    _thunk_I_CacheTex();
    a1 = s0;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lhu(a0 + 0x7AFA);                              // Load from: gTexInfo_NETERR[2] (80097AFA)
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lbu(v0 + 0x7AF8);                              // Load from: gTexInfo_NETERR[2] (80097AF8)
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lbu(v1 + 0x7AF9);                              // Load from: gTexInfo_NETERR[2] (80097AF9)
    a3 = 0x80090000;                                    // Result = 80090000
    a3 = lh(a3 + 0x7AF4);                               // Load from: gTexInfo_NETERR[1] (80097AF4)
    t0 = 0x80090000;                                    // Result = 80090000
    t0 = lh(t0 + 0x7AF6);                               // Load from: gTexInfo_NETERR[1] (80097AF6)
    a2 = 0x54;                                          // Result = 00000054
    sw(a3, sp + 0x18);
    a3 = 0x6D;                                          // Result = 0000006D
    sw(v0, sp + 0x10);
    sw(v1, sp + 0x14);
    sw(t0, sp + 0x1C);
    I_DrawSprite();
    I_SubmitGpuCmds();
    I_DrawPresent();
    I_NetHandshake();    
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F48);                                 // Store to: gTicButtons[1] (80077F48)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7DEC);                                 // Store to: gOldTicButtons[0] (80078214)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F48);                                 // Store to: gTicButtons[1] (80077F48)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7DEC);                                 // Store to: gOldTicButtons[0] (80078214)
    v0 = 1;                                             // Result = 00000001
    goto loc_80034CA0;
loc_80034C48:
    v0 = *gCurPlayerIndex;
    v1 = s0;                                            // Result = gTicButtons[0] (80077F44)
    if (v0 != 0) goto loc_80034C64;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x7F48;                                       // Result = gTicButtons[1] (80077F48)
loc_80034C64:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EAC);                               // Load from: gNetInputPacket[4] (80077EAC)
    sw(v0, v1);
    v0 = *gCurPlayerIndex;
    a0 = s1;                                            // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    if (v0 != 0) goto loc_80034C90;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7FC0;                                       // Result = gPlayersElapsedVBlanks[1] (80077FC0)
loc_80034C90:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x7EAA);                              // Load from: gNetInputPacket[2] (80077EAA)
    v0 = 0;                                             // Result = 00000000
    sw(v1, a0);
loc_80034CA0:
    ra = lw(sp + 0x40);
    s1 = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x48;
    return;
}

void I_NetHandshake() noexcept {
loc_80034CB8:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = 0;                                             // Result = 00000000
loc_80034CC8:
    sb(s0, gp + 0x9D0);                                 // Store to: gNetOutputPacket[0] (80077FB0)
    I_NetSendRecv();
    v1 = lbu(gp + 0x8C8);                               // Load from: gNetInputPacket[0] (80077EA8)
    v0 = lbu(gp + 0x9D0);                               // Load from: gNetOutputPacket[0] (80077FB0)
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(s0) < 8);                             // Result = 00000001
        if (bJump) goto loc_80034CF8;
    }
    s0++;                                               // Result = 00000001
    v0 = (i32(s0) < 8);                                 // Result = 00000001
    if (v0 != 0) goto loc_80034CC8;
loc_80034CF8:
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80034CC8;
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void I_NetSendRecv() noexcept {
loc_80034D14:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
loc_80034D20:
    v0 = *gCurPlayerIndex;
    if (v0 == 0) goto loc_80034DB8;
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_read();
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    s0 = v0;
loc_80034D54:
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    a0 = 3;                                             // Result = 00000003
    if (v0 != 0) goto loc_80034D88;
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v0 -= s0;
    v0 = (i32(v0) < 0x12C);
    a0 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_80034D54;
    goto loc_80034E30;
loc_80034D88:
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    a0 = 3;                                             // Result = 00000003
    if (v0 == 0) goto loc_80034D88;
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_write();
    goto loc_80034E44;
loc_80034DB8:
    a0 = 3;                                             // Result = 00000003
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    if (v0 == 0) goto loc_80034DB8;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7FB0;                                       // Result = gNetOutputPacket[0] (80077FB0)
    a0 = lw(gp + 0x934);                                // Load from: gNetOutputFd (80077F14)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_write();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7EA8;                                       // Result = gNetInputPacket[0] (80077EA8)
    a0 = lw(gp + 0xC54);                                // Load from: gNetInputFd (80078234)
    a2 = 8;                                             // Result = 00000008
    LIBAPI_read();
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    s0 = v0;
loc_80034E04:
    a0 = lw(gp + 0x944);                                // Load from: gSioErrorEvent (80077F24)
    LIBAPI_TestEvent();
    if (v0 != 0) goto loc_80034E44;
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v0 -= s0;
    v0 = (i32(v0) < 0x12C);
    a0 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_80034E04;
loc_80034E30:
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    LIBCOMB__comb_control();
    goto loc_80034D20;
loc_80034E44:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void I_SubmitGpuCmds() noexcept {
loc_80034E58:
    v0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    v1 = lw(gp + 0x638);                                // Load from: gpGpuPrimsEnd (80077C18)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == v1) goto loc_80034E84;
    v0 = 0xFF0000;                                      // Result = 00FF0000
    v0 |= 0xFFFF;                                       // Result = 00FFFFFF
    sw(v0, v1);
    a0 = lw(gp + 0x634);                                // Load from: gpGpuPrimsBeg (80077C14)
    LIBGPU_DrawOTag();
loc_80034E84:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    sw(v0, gp + 0x634);                                 // Store to: gpGpuPrimsBeg (80077C14)
    sw(v0, gp + 0x638);                                 // Store to: gpGpuPrimsEnd (80077C18)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void I_LocalButtonsToNet() noexcept {
loc_80034EA4:
    t1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    t2 = 0x80070000;                                    // Result = 80070000
    t2 += 0x3DEC;                                       // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
loc_80034EB4:
    a1 = 0;                                             // Result = 00000000
    t0 = a2 << 2;
    a3 = lw(a0);
    v1 = t2;                                            // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
loc_80034EC4:
    v0 = lw(v1);
    {
        const bool bJump = (a3 == v0);
        v0 = a1 << t0;
        if (bJump) goto loc_80034EE8;
    }
    a1++;                                               // Result = 00000001
    v0 = (i32(a1) < 8);                                 // Result = 00000001
    v1 += 4;                                            // Result = gBtnSprite_Circle_ButtonMask (80073DF0)
    if (v0 != 0) goto loc_80034EC4;
    v0 = a1 << t0;
loc_80034EE8:
    t1 |= v0;
    a2++;
    v0 = (i32(a2) < 8);
    a0 += 4;
    if (v0 != 0) goto loc_80034EB4;
    v0 = t1;
    return;
}

void I_NetButtonsToLocal() noexcept {
loc_80034F04:
    v1 = 0;                                             // Result = 00000000
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x3DEC;                                       // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7D04;                                       // Result = 800782FC
loc_80034F18:
    v0 = v1 << 2;
    v0 = i32(a0) >> v0;
    v0 &= 0xF;
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v1++;
    sw(v0, a1);
    v0 = (i32(v1) < 8);
    a1 += 4;
    if (v0 != 0) goto loc_80034F18;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x7D04;                                       // Result = 800782FC
    return;
}

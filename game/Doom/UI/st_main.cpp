#include "st_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "in_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBC2.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include <cstdio>

// Positions for each of the micronumbers on the status bar
static constexpr int32_t NUMMICROS = 8;

static const int16_t gMicronumsX[NUMMICROS] = { 199, 212, 225, 238, 199, 212, 225, 238 };
static const int16_t gMicronumsY[NUMMICROS] = { 204, 204, 204, 204, 216, 216, 216, 216 };

// Keycard y positions on the status bar
static const int16_t gCardY[NUMCARDS] = { 204, 212, 220, 204, 212, 220 };

// Which slot (by index) on the weapon micronumbers display each weapon maps to
static const int32_t gWeaponMicroIndexes[NUMWEAPONS] = { 0, 1, 2, 3, 4, 5, 6, 7, 0 };

// The definitions for each face sprite
const facesprite_t gFaceSprites[NUMFACES] = {
    { 118, 202,   0,  41, 19, 29 },     // STFST01  - 0
    { 118, 202,  20,  41, 19, 29 },     // STFST02  - 1
    { 118, 202, 234, 137, 19, 29 },     // STFST00  - 2
    { 118, 202,  40,  41, 21, 31 },     // STFTL00  - 3
    { 118, 202,  62,  41, 21, 31 },     // STFTR00  - 4
    { 118, 202,  84,  41, 19, 31 },     // STFOUCH0 - 5
    { 118, 202, 104,  41, 19, 31 },     // STFEVL0  - 6 (EVILFACE)
    { 118, 202, 124,  41, 19, 31 },     // STFKILL0 - 7
    { 118, 202, 144,  41, 19, 31 },     // STFST11  - 8
    { 118, 202, 164,  41, 19, 31 },     // STFST10  - 9
    { 118, 202, 184,  41, 19, 31 },     // STFST12  - 10
    { 118, 202, 204,  41, 20, 31 },     // STFTL10  - 11
    { 118, 202, 226,  41, 21, 31 },     // STFTR10  - 12
    { 118, 202,   0,  73, 19, 31 },     // STFOUCH1 - 13
    { 118, 202,  20,  73, 19, 31 },     // STFEVL1  - 14
    { 118, 202,  40,  73, 19, 31 },     // STFKILL1 - 15
    { 118, 202,  60,  73, 19, 31 },     // STFST21  - 16
    { 118, 202,  80,  73, 19, 31 },     // STFST20  - 17
    { 118, 202, 100,  73, 19, 31 },     // STFST22  - 18
    { 118, 202, 120,  73, 22, 31 },     // STFTL20  - 19
    { 118, 202, 142,  73, 22, 31 },     // STFTR20  - 20
    { 118, 202, 166,  73, 19, 31 },     // STFOUCH2 - 21
    { 118, 202, 186,  73, 19, 31 },     // STFEVL2  - 22
    { 118, 202, 206,  73, 19, 31 },     // STFKILL2 - 23
    { 118, 202, 226,  73, 19, 31 },     // STFST31  - 24
    { 118, 202,   0, 105, 19, 31 },     // STFST30  - 25
    { 118, 202,  20, 105, 19, 31 },     // STFST32  - 26
    { 118, 202,  40, 105, 23, 31 },     // STFTL30  - 27
    { 118, 202,  64, 105, 23, 31 },     // STFTR30  - 28
    { 118, 202,  88, 105, 19, 31 },     // STFOUCH3 - 29
    { 118, 202, 108, 105, 19, 31 },     // STFEVL3  - 30
    { 118, 202, 128, 105, 19, 31 },     // STFKILL3 - 31
    { 118, 202, 148, 105, 19, 31 },     // STFST41  - 32
    { 118, 202, 168, 105, 19, 31 },     // STFST40  - 33
    { 118, 202, 188, 105, 19, 31 },     // STFST42  - 34
    { 118, 202, 208, 105, 24, 31 },     // STFTL40  - 35
    { 118, 202, 232, 105, 23, 31 },     // STFTR40  - 36
    { 118, 202,   0, 137, 18, 31 },     // STFOUCH4 - 37
    { 118, 202,  20, 137, 19, 31 },     // STFEVL4  - 38
    { 118, 202,  40, 137, 19, 31 },     // STFKILL4 - 39
    { 118, 202,  60, 137, 19, 31 },     // STFGOD0  - 40 (GODFACE)
    { 118, 202,  80, 137, 19, 31 },     // STFDEAD0 - 41 (DEADFACE)
    { 118, 202, 100, 137, 19, 30 },     // STSPLAT0 - 42 (FIRSTSPLAT)
    { 114, 201, 120, 137, 27, 30 },     // STSPLAT1 - 43
    { 114, 204, 148, 137, 28, 30 },     // STSPLAT2 - 44
    { 114, 204, 176, 137, 28, 30 },     // STSPLAT3 - 45
    { 114, 204, 204, 137, 28, 30 }      // STSPLAT4 - 46
};

// State relating to flashing keycards on the status bar
struct sbflash_t {
    int16_t     active;     // Is the flash currently active?
    int16_t     doDraw;     // Are we currently drawing the keycard as part of the flash?
    int16_t     delay;      // Ticks until next draw/no-draw change
    int16_t     times;      // How many flashes are left
};

static const VmPtr<sbflash_t[NUMCARDS]> gFlashCards(0x800A94B4);

// Status bar message
const VmPtr<VmPtr<const char>>  gpStatusBarMsgStr(0x80098740);
const VmPtr<int32_t>            gStatusBarMsgTicsLeft(0x80098744);

// Face related state
static const VmPtr<bool32_t>                gbDrawSBFace(0x80078130);
static const VmPtr<VmPtr<facesprite_t>>     gpCurSBFaceSprite(0x80078230);

void ST_Init() noexcept {
loc_80038558:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FD8);                               // Load from: gTexCacheFillPage (80078028)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003857C;
    I_Error("ST_Init: initial texture cache foulup\n");
loc_8003857C:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6B18;                                       // Result = gTex_STATUS[0] (800A94E8)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7CE4;                                       // Result = STR_LumpName_STATUS[0] (80077CE4)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FD8);                               // Load from: gTexCacheFillPage (80078028)
    if (v0 == 0) goto loc_800385B8;
    I_Error("ST_Init: final texture cache foulup\n");
loc_800385B8:
    v1 = *gLockedTexPagesMask;
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    v1 |= 1;
    *gLockedTexPagesMask = v1;
    
    Z_FreeTags(**gpMainMemZone, PU_CACHE);

    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void ST_Start() noexcept {
loc_80038610:
    a1 = 0;                                             // Result = 00000000
    a0 = 0;                                             // Result = 00000000
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78CC;                                       // Result = gStatusBar[7] (80098734)
    v1 = v0 - 0x18;                                     // Result = gStatusBar[1] (8009871C)
    sw(0, v0);                                          // Store to: gStatusBar[7] (80098734)
    v0 = 1;                                             // Result = 00000001
    *gbDrawSBFace = v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E68;                                       // Result = StatusBarFaceSpriteInfo[0] (80073E68)
    sw(0, gp + 0xA78);                                  // Store to: gbStatusBarPlayerGotGibbed (80078058)
    sw(0, gp + 0x8EC);                                  // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78E8);                                 // Store to: gStatusBar[0] (80098718)
    *gStatusBarMsgTicsLeft = 0;
    sw(0, gp + 0xB54);                                  // Store to: gFaceTics (80078134)
    *gpCurSBFaceSprite = v0;
loc_80038658:
    sw(0, v1);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gFlashCards[0] (800A94B4)
    at += a0;
    sh(0, at);
    a0 += 8;
    a1++;
    v0 = (i32(a1) < 6);
    v1 += 4;
    if (v0 != 0) goto loc_80038658;
    return;
}

void ST_Ticker() noexcept {
loc_80038688:
    a0 = lw(gp + 0xB54);                                // Load from: gFaceTics (80078134)
    v1 = *gCurPlayerIndex;
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a0--;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    sw(a0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    s4 = v1 + v0;
    if (i32(a0) > 0) goto loc_80038710;
    _thunk_M_Random();
    v0 &= 0xF;
    sw(v0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    _thunk_M_Random();
    v0 &= 3;
    v1 = 3;                                             // Result = 00000003
    sw(v0, gp + 0xA44);                                 // Store to: gStatusBarFaceFrameNum (80078024)
    {
        const bool bJump = (v0 != v1);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003870C;
    }
    sw(v0, gp + 0xA44);                                 // Store to: gStatusBarFaceFrameNum (80078024)
loc_8003870C:
    sw(0, gp + 0x8EC);                                  // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
loc_80038710:
    v1 = 0x800A0000;                                    // Result = 800A0000
    v1 -= 0x78E8;                                       // Result = gStatusBar[0] (80098718)
    v0 = lw(v1);                                        // Load from: gStatusBar[0] (80098718)
    if (v0 == 0) goto loc_80038740;
    sw(v0, gp + 0x928);                                 // Store to: gStatusBarCurSpecialFace (80077F08)
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    v0 = 1;                                             // Result = 00000001
    sw(0, v1);                                          // Store to: gStatusBar[0] (80098718)
    sw(v0, gp + 0x8EC);                                 // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
loc_80038740:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78CC);                               // Load from: gStatusBar[7] (80098734)
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80038774;
    }
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v0 = 1;                                             // Result = 00000001
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78C8);                                 // Store to: gStatusBarGibAnimFrame (80098738)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78CC);                                 // Store to: gStatusBar[7] (80098734)
    sw(v0, gp + 0xA78);                                 // Store to: gbStatusBarPlayerGotGibbed (80078058)
loc_80038774:
    v0 = lw(gp + 0xA78);                                // Load from: gbStatusBarPlayerGotGibbed (80078058)
    if (v0 == 0) goto loc_800387D4;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C4);                               // Load from: gStatusBarGibAnimTicsLeft (8009873C)
    v0--;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v1 = 2;                                             // Result = 00000002
    if (i32(v0) > 0) goto loc_800387D4;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C8);                               // Load from: gStatusBarGibAnimFrame (80098738)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v1, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v0++;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C8);                                // Store to: gStatusBarGibAnimFrame (80098738)
    v0 = (i32(v0) < 5);
    if (v0 != 0) goto loc_800387D4;
    sw(0, gp + 0xA78);                                  // Store to: gbStatusBarPlayerGotGibbed (80078058)
    *gbDrawSBFace = false;
loc_800387D4:
    v0 = lw(s4 + 0xD4);
    if (v0 == 0) goto loc_8003882C;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7740;                                       // Result = gPlayer1[35] (800A88C0)
    at += v0;
    v1 = lw(at);
    v0 = 0x4B;
    *gStatusBarMsgTicsLeft = v0;
    *gpStatusBarMsgStr = v1;
    sw(0, s4 + 0xD4);
loc_8003882C:
    v0 = *gStatusBarMsgTicsLeft;
    s2 = 0;
    if (v0 == 0) goto loc_8003884C;
    v0--;
    *gStatusBarMsgTicsLeft = v0;
loc_8003884C:
    s3 = 4;                                             // Result = 00000004
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x6B4C;                                       // Result = gFlashCards[0] (800A94B4)
    s0 = 0;                                             // Result = 00000000
loc_8003885C:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78E4;                                       // Result = gStatusBar[1] (8009871C)
    v1 = s2 << 2;
    v1 += v0;
    v0 = lw(v1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800388C0;
    }
    sw(0, v1);
    sh(v0, s1);
    v0 = 7;                                             // Result = 00000007
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gFlashCards[2] (800A94B8)
    at += s0;
    sh(s3, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B46;                                       // Result = gFlashCards[3] (800A94BA)
    at += s0;
    sh(v0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gFlashCards[1] (800A94B6)
    at += s0;
    sh(0, at);
    s1 += 8;
    goto loc_800389AC;
loc_800388C0:
    v0 = lh(s1);
    if (v0 == 0) goto loc_800389A8;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6B4C;                                       // Result = gFlashCards[0] (800A94B4)
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gFlashCards[2] (800A94B8)
    at += s0;
    v0 = lhu(at);
    a0 = s0 + v1;
    v0--;
    sh(v0, a0 + 0x4);
    v0 <<= 16;
    if (v0 != 0) goto loc_800389A8;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gFlashCards[1] (800A94B6)
    at += s0;
    v0 = lhu(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B46;                                       // Result = gFlashCards[3] (800A94BA)
    at += s0;
    v1 = lhu(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gFlashCards[2] (800A94B8)
    at += s0;
    sh(s3, at);
    v0 ^= 1;
    v1--;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gFlashCards[1] (800A94B6)
    at += s0;
    sh(v0, at);
    sh(v1, a0 + 0x6);
    v1 <<= 16;
    if (v1 != 0) goto loc_80038968;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gFlashCards[0] (800A94B4)
    at += s0;
    sh(0, at);
loc_80038968:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gFlashCards[1] (800A94B6)
    at += s0;
    v0 = lh(at);
    if (v0 == 0) goto loc_800389A8;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gFlashCards[0] (800A94B4)
    at += s0;
    v0 = lh(at);
    a0 = 0;
    if (v0 == 0) goto loc_800389A8;
    a1 = sfx_itemup;
    S_StartSound();
loc_800389A8:
    s1 += 8;
loc_800389AC:
    s2++;
    v0 = (i32(s2) < 6);
    s0 += 8;
    if (v0 != 0) goto loc_8003885C;
    v0 = lw(s4 + 0xC0);
    v0 &= 2;
    v1 = 0x28;                                          // Result = 00000028
    if (v0 != 0) goto loc_800389E0;
    v0 = lw(s4 + 0x30);
    if (v0 == 0) goto loc_800389F0;
loc_800389E0:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v1, at - 0x78EC);                                // Store to: gStatusBarFaceAnimNum (80098714)
    goto loc_80038AB8;
loc_800389F0:
    v0 = lw(gp + 0xA78);                                // Load from: gbStatusBarPlayerGotGibbed (80078058)
    if (v0 == 0) goto loc_80038A10;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C8);                               // Load from: gStatusBarGibAnimFrame (80098738)
    v0 += 0x2A;
    goto loc_80038AB0;
loc_80038A10:
    v1 = lw(s4 + 0x24);
    v0 = 0x29;                                          // Result = 00000029
    if (v1 == 0) goto loc_80038AB0;
    v0 = lw(gp + 0x8EC);                                // Load from: gbStatusBarIsShowingSpecialFace (80077ECC)
    {
        const bool bJump = (v0 == 0);
        v0 = 0x66660000;                                // Result = 66660000
        if (bJump) goto loc_80038A70;
    }
    v0 |= 0x6667;                                       // Result = 66666667
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 3);
    v1 = v0 - v1;
    v0 = (i32(v1) < 4);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80038A5C;
    }
    v1 = 0;                                             // Result = 00000000
    goto loc_80038A64;
loc_80038A5C:
    v0 -= v1;
    v1 = v0 << 3;
loc_80038A64:
    v0 = lw(gp + 0x928);                                // Load from: gStatusBarCurSpecialFace (80077F08)
    v0 += v1;
    goto loc_80038AB0;
loc_80038A70:
    v0 |= 0x6667;                                       // Result = 66666667
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 3);
    v1 = v0 - v1;
    v0 = (i32(v1) < 4);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80038A9C;
    }
    v1 = 0;                                             // Result = 00000000
    goto loc_80038AA4;
loc_80038A9C:
    v0 -= v1;
    v1 = v0 << 3;
loc_80038AA4:
    v0 = lw(gp + 0xA44);                                // Load from: gStatusBarFaceFrameNum (80078024)
    v0 += v1;
loc_80038AB0:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78EC);                                // Store to: gStatusBarFaceAnimNum (80098714)
loc_80038AB8:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78EC);                               // Load from: gStatusBarFaceAnimNum (80098714)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E68;                                       // Result = StatusBarFaceSpriteInfo[0] (80073E68)
    v1 += v0;
    *gpCurSBFaceSprite = v1;
    I_UpdatePalette();
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do drawing for the HUD status bar
//------------------------------------------------------------------------------------------------------------------------------------------
void ST_Drawer() noexcept {
    // Setup the current texture page and texture window.
    // PC-PSX: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that.
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);

        #if PC_PSX_DOOM_MODS
            RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);
        #endif

        I_AddPrim(&drawModePrim);
    }

    // Setup some sprite primitive state that is used for all the draw calls that follow
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_SetShadeTex(&spritePrim, true);
    spritePrim.clut = gPaletteClutIds[UIPAL];
    
    // Draw the current status bar message, or the map name (if in the automap)
    player_t& player = gPlayers[*gCurPlayerIndex];

    if (*gStatusBarMsgTicsLeft > 0) {
        I_DrawStringSmall(7, 193, gpStatusBarMsgStr->get());
    } else {
        if (player.automapflags & AF_ACTIVE) {
            constexpr const char* const MAP_TITLE_FMT = "LEVEL %d:%s";
            char mapTitle[64];

            // PC-PSX: use 'snprintf' just to be safe here
            #if PC_PSX_DOOM_MODS
                std::snprintf(mapTitle, C_ARRAY_SIZE(mapTitle), MAP_TITLE_FMT, *gGameMap, gMapNames[*gGameMap - 1]);
            #else
                std::sprintf(mapTitle, MAP_TITLE_FMT, *gGameMap, gMapNames[*gGameMap - 1]);
            #endif

            I_DrawStringSmall(7, 193, mapTitle);
        }
    }

    // Draw the background for the status bar
    LIBGPU_setXY0(spritePrim, 0, 200);
    LIBGPU_setUV0(spritePrim, 0, 0);
    LIBGPU_setWH(spritePrim, 256, 40);

    I_AddPrim(&spritePrim);

    // Figure out what weapon to display ammo for and what to show for the ammo amount
    const weapontype_t weapon = (player.pendingweapon == wp_nochange) ?
        player.readyweapon :
        player.pendingweapon;

    const weaponinfo_t& weaponInfo = gWeaponInfo[weapon];
    const ammotype_t ammoType = weaponInfo.ammo;
    const int32_t ammo = (ammoType != am_noammo) ? player.ammo[ammoType] : 0;

    // Draw ammo, health and armor amounts
    I_DrawNumber(28, 204, ammo);
    I_DrawNumber(71, 204, player.health);
    I_DrawNumber(168, 204, player.armorpoints);

    // Draw keycards and skull keys
    {
        LIBGPU_setWH(spritePrim, 11, 8);
        spritePrim.x0 = 100;
        spritePrim.tv0 = 184;

        uint8_t texU = 114;

        for (int32_t cardIdx = 0; cardIdx < NUMCARDS; ++cardIdx) {
            const bool bHaveCard = player.cards[cardIdx];

            // Draw the card if we have it or if it's currently flashing
            if (bHaveCard || (gFlashCards[cardIdx].active && gFlashCards[cardIdx].doDraw)) {
                spritePrim.tu0 = texU;
                spritePrim.y0 = gCardY[cardIdx];

                I_AddPrim(&spritePrim);
            }

            texU += 11;
        }
    }

    // Draw weapon selector or frags (if deathmatch)
    if (*gNetGame != gt_deathmatch) {
        // Draw the weapon number box/container
        LIBGPU_setXY0(spritePrim, 200, 205);
        LIBGPU_setUV0(spritePrim, 180, 184);
        LIBGPU_setWH(spritePrim, 51, 23);

        I_AddPrim(&spritePrim);

        // Draw the micro numbers for each weapon.
        // Note that numbers '1' and '2' are already baked into the status bar graphic, so we start at the shotgun.
        {
            LIBGPU_setWH(spritePrim, 4, 6);
            spritePrim.tv0 = 184;

            uint8_t texU = 232;

            for (int32_t weaponIdx = wp_shotgun; weaponIdx < NUMMICROS; ++weaponIdx) {
                if (player.weaponowned[weaponIdx]) {
                    LIBGPU_setXY0(spritePrim, gMicronumsX[weaponIdx] + 5, gMicronumsY[weaponIdx] + 3);
                    spritePrim.tu0 = texU;

                    I_AddPrim(&spritePrim);
                }

                texU += 4;
            }
        }

        // Draw the white box or highlight for the currently selected weapon
        const int32_t microNumIdx = gWeaponMicroIndexes[weapon];

        LIBGPU_setXY0(spritePrim, gMicronumsX[microNumIdx], gMicronumsY[microNumIdx]);
        LIBGPU_setUV0(spritePrim, 164, 192);
        LIBGPU_setWH(spritePrim, 12, 12);
        
        I_AddPrim(&spritePrim);
    } else {
        // Draw the frags container box
        LIBGPU_setXY0(spritePrim, 209, 221);
        LIBGPU_setUV0(spritePrim, 208, 243);
        LIBGPU_setWH(spritePrim, 33, 8);

        I_AddPrim(&spritePrim);

        // Draw the number of frags
        I_DrawNumber(225, 204, player.frags);
    }

    // Draw the doomguy face if enabled
    if (*gbDrawSBFace) {
        const facesprite_t& sprite = **gpCurSBFaceSprite;

        LIBGPU_setXY0(spritePrim, sprite.xPos, sprite.yPos);
        LIBGPU_setUV0(spritePrim, sprite.texU, sprite.texV);
        LIBGPU_setWH(spritePrim, sprite.w, sprite.h);

        I_AddPrim(&spritePrim);
    }

    // Draw the paused overlay, level warp and vram viewer
    if (*gbGamePaused) {
        I_DrawPausedOverlay();
    }
}

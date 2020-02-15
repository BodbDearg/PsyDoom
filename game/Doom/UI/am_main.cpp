#include "am_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_local.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

static constexpr fixed_t MOVESTEP   = FRACUNIT * 128;   // Controls how fast manual automap movement happens
static constexpr fixed_t SCALESTEP  = 2;                // How fast to scale in/out
static constexpr int32_t MAXSCALE   = 64;               // Maximum map zoom
static constexpr int32_t MINSCALE   = 8;                // Minimum map zoom

static const VmPtr<fixed_t> gAutomapXMin(0x80078280);
static const VmPtr<fixed_t> gAutomapXMax(0x80078290);
static const VmPtr<fixed_t> gAutomapYMin(0x8007828C);
static const VmPtr<fixed_t> gAutomapYMax(0x80078298);

//------------------------------------------------------------------------------------------------------------------------------------------
// Automap initialization logic
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Start() noexcept {
    *gAutomapXMin = *gBlockmapOriginX;
    *gAutomapYMin = *gBlockmapOriginY;
    *gAutomapXMax = (*gBlockmapWidth  << MAPBLOCKSHIFT) + *gBlockmapOriginX;    
    *gAutomapYMax = (*gBlockmapHeight << MAPBLOCKSHIFT) + *gBlockmapOriginY;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the automap: handles player input & controls
//------------------------------------------------------------------------------------------------------------------------------------------
void AM_Control(player_t& player) noexcept {
    // If the game is paused we do nothing
    if (*gbGamePaused)
        return;
    
    // Toggle the automap on and off if select has just been pressed
    const padbuttons_t ticButtons = gTicButtons[*gPlayerNum];
    
    if ((ticButtons & PAD_SELECT) && ((gOldTicButtons[*gPlayerNum] & PAD_SELECT) == 0)) {
        player.automapflags ^= AF_ACTIVE;
        player.automapx = player.mo->x;
        player.automapy = player.mo->y;
    }

    // If the automap is not active or the player dead then do nothing
    if ((player.automapflags & AF_ACTIVE) == 0)
        return;

    if (player.playerstate != PST_LIVE)
        return;

    // Follow the player unless the cross button is pressed.
    // The rest of the logic is for when we are NOT following the player.
    if ((ticButtons & PAD_CROSS) == 0) {
        player.automapflags &= ~AF_FOLLOW;
        return;
    }
    
    // Snap the manual automap movement position to the player location once we transition from following to not following
    if ((player.automapflags & AF_FOLLOW) == 0) {
        player.automapflags |= AF_FOLLOW;
        player.automapx = player.mo->x;
        player.automapy = player.mo->y;
    }
    
    // Figure out the movement amount for manual camera movement    
    const fixed_t moveStep = (ticButtons & PAD_SQUARE) ? MOVESTEP * 2 : MOVESTEP;

    // Not sure why this check was done, it can never be true due to the logic above.
    // PC-PSX: remove this block as it is useless...
    #if !PC_PSX_DOOM_MODS
        if ((player.automapflags & AF_FOLLOW) == 0)
            return;
    #endif

    // Left/right movement
    if (ticButtons & PAD_RIGHT) {
        player.automapx += moveStep;

        if (player.automapx > *gAutomapXMax) {
            player.automapx = *gAutomapXMax;
        }
    }
    else if (ticButtons & PAD_LEFT) {
        player.automapx -= moveStep;

        if (player.automapx < *gAutomapXMin) {
            player.automapx = *gAutomapXMin;
        }
    }

    // Up/down movement
    if (ticButtons & PAD_UP) {
        player.automapy += moveStep;

        if (player.automapy > *gAutomapYMax) {
            player.automapy = *gAutomapYMax;
        }
    }
    else if (ticButtons & PAD_DOWN) {
        player.automapy -= moveStep;

        if (player.automapy < *gAutomapYMin) {
            player.automapy = *gAutomapYMin;
        }
    }
    
    // Scale up and down
    if (ticButtons & PAD_R1) {
        player.automapscale -= SCALESTEP;
        
        if (player.automapscale < MINSCALE) {
            player.automapscale = MINSCALE;
        }
    } 
    else if (ticButtons & PAD_L1) {
        player.automapscale += SCALESTEP;

        if (player.automapscale > MAXSCALE) {
            player.automapscale = MAXSCALE;
        }
    }

    // When not in follow mode, consume these inputs so that we don't move the player in the level
    gTicButtons[*gPlayerNum] &= ~(PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_R1 | PAD_L1);
}

void AM_Drawer() noexcept {
loc_8003BD34:
    sp -= 0x60;
    sw(ra, sp + 0x5C);
    sw(fp, sp + 0x58);
    sw(s7, sp + 0x54);
    sw(s6, sp + 0x50);
    sw(s5, sp + 0x4C);
    sw(s4, sp + 0x48);
    sw(s3, sp + 0x44);
    sw(s2, sp + 0x40);
    sw(s1, sp + 0x3C);
    sw(s0, sp + 0x38);
    I_DrawPresent();
    v1 = *gCurPlayerIndex;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v1 += v0;
    sw(v1, sp + 0x28);
    v0 = lw(v1 + 0x124);
    s7 = lw(v1 + 0x120);
    v0 &= 2;
    if (v0 == 0) goto loc_8003BDC0;
    t4 = lw(v1 + 0x118);
    sw(t4, sp + 0x18);
    t4 = lw(v1 + 0x11C);
    sw(t4, sp + 0x20);
    goto loc_8003BDE8;
loc_8003BDC0:
    t4 = lw(sp + 0x28);
    v0 = lw(t4);
    t4 = lw(v0);
    sw(t4, sp + 0x18);
    v0 = lw(v0 + 0x4);
    sw(v0, sp + 0x20);
loc_8003BDE8:
    v0 = *gNumLines;
    s1 = *gpLines;
    fp = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8003BF6C;
    s0 = s1 + 0x14;
loc_8003BE04:
    v0 = lw(s0 - 0x4);
    v1 = 0x100;                                         // Result = 00000100
    v0 &= 0x180;
    s6 = 0x8A0000;                                      // Result = 008A0000
    if (v0 == v1) goto loc_8003BE44;
    t4 = lw(sp + 0x28);
    v0 = lw(t4 + 0x40);
    if (v0 != 0) goto loc_8003BE44;
    v0 = lw(t4 + 0xC0);
    v0 &= 4;
    if (v0 == 0) goto loc_8003BF4C;
loc_8003BE44:
    v1 = lw(s1);
    t4 = lw(sp + 0x18);
    v0 = lw(v1);
    v0 -= t4;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v0 = lw(v1 + 0x4);
    t4 = lw(sp + 0x20);
    a1 = lo;
    v0 -= t4;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 = lw(s0 - 0x10);
    t4 = lw(sp + 0x18);
    v0 = lw(v1);
    a0 = lo;
    v0 -= t4;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    t4 = lw(sp + 0x20);
    v0 = lw(v1 + 0x4);
    v1 = lo;
    v0 -= t4;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    s6 |= 0x5C30;                                       // Result = 008A5C30
    t2 = u32(i32(a0) >> 16);
    t4 = lw(sp + 0x28);
    a3 = u32(i32(v1) >> 16);
    v0 = lo;
    a0 = u32(i32(v0) >> 16);
    v0 = lw(t4 + 0xC0);
    v1 = lw(t4 + 0x40);
    v0 &= 4;
    v0 += v1;
    t3 = u32(i32(a1) >> 16);
    if (v0 == 0) goto loc_8003BEFC;
    v0 = lw(s0 - 0x4);
    v0 &= 0x100;
    if (v0 != 0) goto loc_8003BEFC;
    s6 = 0x800000;                                      // Result = 00800000
    s6 |= 0x8080;                                       // Result = 00808080
    goto loc_8003BF38;
loc_8003BEFC:
    v1 = lw(s0 - 0x4);
    v0 = v1 & 0x20;
    if (v0 != 0) goto loc_8003BF34;
    v0 = lw(s0);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 4;
        if (bJump) goto loc_8003BF2C;
    }
    s6 = 0xCC0000;                                      // Result = 00CC0000
    s6 |= 0xCC00;                                       // Result = 00CCCC00
    goto loc_8003BF38;
loc_8003BF2C:
    if (v0 != 0) goto loc_8003BF38;
loc_8003BF34:
    s6 = 0xA40000;                                      // Result = 00A40000
loc_8003BF38:
    sw(a0, sp + 0x10);
    a0 = s6;
    a1 = t3;
    a2 = t2;
    _thunk_DrawLine();
loc_8003BF4C:
    fp++;
    s0 += 0x4C;
    v0 = *gNumLines;
    v0 = (i32(fp) < i32(v0));
    s1 += 0x4C;
    if (v0 != 0) goto loc_8003BE04;
loc_8003BF6C:
    t4 = lw(sp + 0x28);
    v0 = lw(t4 + 0xC0);
    v0 &= 8;
    if (v0 == 0) goto loc_8003C168;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    a3 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    if (a3 == v0) goto loc_8003C168;
    fp = 0x80060000;                                    // Result = 80060000
    fp += 0x7958;                                       // Result = FineSine[0] (80067958)
loc_8003BFA8:
    t4 = lw(sp + 0x28);
    v0 = lw(t4);
    s6 = lw(a3 + 0x14);
    if (a3 == v0) goto loc_8003C154;
    t0 = lw(a3 + 0x24);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(a3);
    t4 = lw(sp + 0x18);
    a1 = t0 >> 19;
    a1 <<= 2;
    v0 = a1 + a2;
    a0 = lw(v0);
    t3 = v1 - t4;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    a1 += fp;
    t4 = lw(sp + 0x20);
    v0 = lw(a3 + 0x4);
    v1 = lw(a1);
    t2 = v0 - t4;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    a1 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 = 0xA0000000;                                    // Result = A0000000
    v1 += t0;
    v1 >>= 19;
    v1 <<= 2;
    v0 = v1 + a2;
    a0 = lw(v0);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    t1 = lo;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 += fp;
    v1 = lw(v1);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    a3 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 = 0x60000000;                                    // Result = 60000000
    v1 += t0;
    v1 >>= 19;
    v1 <<= 2;
    a2 += v1;
    a0 = lw(a2);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    t0 = lo;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 += fp;
    v1 = lw(v1);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v1 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    s5 = u32(i32(a1) >> 16);
    a1 = s5;
    s4 = u32(i32(t1) >> 16);
    a2 = s4;
    a0 = 0x80FF;                                        // Result = 000080FF
    s1 = u32(i32(a3) >> 16);
    a3 = s1;
    s0 = u32(i32(t0) >> 16);
    sw(s0, sp + 0x10);
    s3 = u32(i32(v1) >> 16);
    v0 = lo;
    s2 = u32(i32(v0) >> 16);
    _thunk_DrawLine();
    a0 = 0x80FF;                                        // Result = 000080FF
    a1 = s1;
    a2 = s0;
    a3 = s3;
    sw(s2, sp + 0x10);
    _thunk_DrawLine();
    a0 = 0x80FF;                                        // Result = 000080FF
    a1 = s5;
    a2 = s4;
    a3 = s3;
    sw(s2, sp + 0x10);
    _thunk_DrawLine();
loc_8003C154:
    a3 = s6;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    if (a3 != v0) goto loc_8003BFA8;
loc_8003C168:
    t4 = 0x800B0000;                                    // Result = 800B0000
    t4 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    sw(t4, sp + 0x28);
    fp = 0;                                             // Result = 00000000
loc_8003C178:
    v1 = *gNetGame;
    t4 = 1;                                             // Result = 00000001
    if (v1 == t4) goto loc_8003C1A0;
    v0 = *gCurPlayerIndex;
    if (fp != v0) goto loc_8003C3A4;
loc_8003C1A0:
    t4 = lw(sp + 0x28);
    v0 = lw(t4 + 0x4);
    s6 = 0xC000;                                        // Result = 0000C000
    if (v0 != 0) goto loc_8003C1D0;
    v0 = *gGameTic;
    v0 &= 2;
    if (v0 != 0) goto loc_8003C3A4;
loc_8003C1D0:
    t4 = 1;                                             // Result = 00000001
    if (v1 != t4) goto loc_8003C1F8;
    v0 = *gCurPlayerIndex;
    if (fp != v0) goto loc_8003C1F8;
    s6 = 0xCC0000;                                      // Result = 00CC0000
    s6 |= 0xCC00;                                       // Result = 00CCCC00
loc_8003C1F8:
    t4 = lw(sp + 0x28);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a2 = lw(t4);
    t4 = lw(sp + 0x18);
    t0 = lw(a2 + 0x24);
    v1 = lw(a2);
    a1 = t0 >> 19;
    a1 <<= 2;
    v0 = a1 + a3;
    a0 = lw(v0);
    t3 = v1 - t4;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    t4 = 0x80060000;                                    // Result = 80060000
    t4 += 0x7958;                                       // Result = FineSine[0] (80067958)
    a1 += t4;
    t4 = lw(sp + 0x20);
    v0 = lw(a2 + 0x4);
    v1 = lw(a1);
    t2 = v0 - t4;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    a1 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 = 0xA0000000;                                    // Result = A0000000
    v1 += t0;
    v1 >>= 19;
    v1 <<= 2;
    v0 = v1 + a3;
    a0 = lw(v0);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    a2 = lo;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    t4 = 0x80060000;                                    // Result = 80060000
    t4 += 0x7958;                                       // Result = FineSine[0] (80067958)
    v1 += t4;
    v1 = lw(v1);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    t1 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 = 0x60000000;                                    // Result = 60000000
    v1 += t0;
    v1 >>= 19;
    v1 <<= 2;
    a3 += v1;
    a0 = lw(a3);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    t0 = lo;
    v0 += t3;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    v1 += t4;
    v1 = lw(v1);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v1 = lo;
    v0 += t2;
    v0 = u32(i32(v0) >> 8);
    mult(s7, v0);
    s5 = u32(i32(a1) >> 16);
    a1 = s5;
    s4 = u32(i32(a2) >> 16);
    a2 = s4;
    a0 = s6;
    s1 = u32(i32(t1) >> 16);
    a3 = s1;
    s0 = u32(i32(t0) >> 16);
    sw(s0, sp + 0x10);
    s3 = u32(i32(v1) >> 16);
    v0 = lo;
    s2 = u32(i32(v0) >> 16);
    _thunk_DrawLine();
    a0 = s6;
    a1 = s1;
    a2 = s0;
    a3 = s3;
    sw(s2, sp + 0x10);
    _thunk_DrawLine();
    a0 = s6;
    a1 = s5;
    a2 = s4;
    a3 = s3;
    sw(s2, sp + 0x10);
    _thunk_DrawLine();
loc_8003C3A4:
    fp++;
    t4 = lw(sp + 0x28);
    v0 = (i32(fp) < 2);
    t4 += 0x12C;
    sw(t4, sp + 0x28);
    if (v0 != 0) goto loc_8003C178;
    ra = lw(sp + 0x5C);
    fp = lw(sp + 0x58);
    s7 = lw(sp + 0x54);
    s6 = lw(sp + 0x50);
    s5 = lw(sp + 0x4C);
    s4 = lw(sp + 0x48);
    s3 = lw(sp + 0x44);
    s2 = lw(sp + 0x40);
    s1 = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x60;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw an automap line in the specified color
//------------------------------------------------------------------------------------------------------------------------------------------
void DrawLine(const uint32_t color, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2) noexcept {    
    // Reject the line quickly using the 'Cohen-Sutherland' algorithm.
    // Note: no clipping is done since that is handled by the hardware.
    enum OutFlags : uint32_t {
        INSIDE  = 0,
        LEFT    = 1,
        RIGHT   = 2,
        BOTTOM  = 4,
        TOP     = 8
    };
    
    uint32_t outcode1 = (x1 < -128) ? LEFT : INSIDE;
    if (x1 >  128) { outcode1 |= RIGHT;     }
    if (y1 < -100) { outcode1 |= BOTTOM;    }
    if (y1 >  100) { outcode1 |= TOP;       }

    uint32_t outcode2 = (x2 < -128) ? LEFT : INSIDE;    
    if (x2 >  128) { outcode2 |= RIGHT;     }
    if (y2 < -100) { outcode2 |= BOTTOM;    }
    if (y2 >  100) { outcode2 |= TOP;       }
    
    if (outcode1 & outcode2) 
        return;

    // Setup the map line primitive and draw it.
    // Use the 1 KiB scratchpad also as temp storage space for the primitive.
    LINE_F2& line = *(LINE_F2*) getScratchAddr(128);
    
    LIBGPU_SetLineF2(line);
    LIBGPU_setRGB0(line, (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t) color);
    LIBGPU_setXY2(line, (int16_t)(x1 + 128), (int16_t)(100 - y1), (int16_t)(x2 + 128), (int16_t)(100 - y2));
    
    I_AddPrim(&line);
}

void _thunk_DrawLine() noexcept {
    DrawLine(a0, a1, a2, a3, lw(sp + 0x10));
}

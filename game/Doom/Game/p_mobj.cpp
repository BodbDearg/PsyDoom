#include "p_mobj.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_password.h"
#include "p_pspr.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

void P_RemoveMObj() noexcept {
loc_8001C724:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = 0x20000;                                       // Result = 00020000
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x64);
    v0 |= 1;                                            // Result = 00020001
    v1 &= v0;
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 0x2E;                                      // Result = 0000002E
        if (bJump) goto loc_8001C7F4;
    }
    v1 = lw(s0 + 0x54);
    {
        const bool bJump = (v1 == v0);
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_8001C7F4;
    }
    if (v1 == v0) goto loc_8001C7F4;
    a1 = lw(gp + 0xB58);                                // Load from: gItemRespawnQueueHead (80078138)
    v1 = *gTicCon;
    a0 = a1 & 0x3F;
    v0 = a0 << 2;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7910;                                       // Result = gItemRespawnTime[0] (80097910)
    at += v0;
    sw(v1, at);
    v0 += a0;
    v1 = lhu(s0 + 0x88);
    v0 <<= 1;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x612C;                                       // Result = gItemRespawnQueue[0] (8008612C)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8A);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x612E;                                       // Result = gItemRespawnQueue[1] (8008612E)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8C);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6132;                                       // Result = gItemRespawnQueue[3] (80086132)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8E);
    a1++;
    sw(a1, gp + 0xB58);                                 // Store to: gItemRespawnQueueHead (80078138)
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6130;                                       // Result = gItemRespawnQueue[2] (80086130)
    at += v0;
    sh(v1, at);
loc_8001C7F4:
    a0 = s0;
    _thunk_P_UnsetThingPosition();
    v1 = lw(s0 + 0x14);
    v0 = lw(s0 + 0x10);
    a0 = *gpMainMemZone;
    sw(v0, v1 + 0x10);
    v1 = lw(s0 + 0x10);
    v0 = lw(s0 + 0x14);
    a1 = s0;
    sw(v0, v1 + 0x14);
    _thunk_Z_Free2();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_RespawnSpecials() noexcept {
loc_8001C838:
    sp -= 0x20;
    v1 = *gNetGame;
    v0 = 2;                                             // Result = 00000002
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v1 != v0) goto loc_8001C9FC;
    v1 = lw(gp + 0xB58);                                // Load from: gItemRespawnQueueHead (80078138)
    v0 = lw(gp + 0xBA0);                                // Load from: gItemRespawnQueueTail (80078180)
    {
        const bool bJump = (v1 == v0);
        v0 = v1 - v0;
        if (bJump) goto loc_8001C9FC;
    }
    v0 = (i32(v0) < 0x41);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 - 0x40;
        if (bJump) goto loc_8001C880;
    }
    sw(v0, gp + 0xBA0);                                 // Store to: gItemRespawnQueueTail (80078180)
loc_8001C880:
    v0 = lw(gp + 0xBA0);                                // Load from: gItemRespawnQueueTail (80078180)
    a1 = v0 & 0x3F;
    a0 = a1 << 2;
    v0 = *gTicCon;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7910;                                       // Result = gItemRespawnTime[0] (80097910)
    at += a0;
    v1 = lw(at);
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 + a1;
        if (bJump) goto loc_8001C9FC;
    }
    v0 <<= 1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x612C;                                       // Result = gItemRespawnQueue[0] (8008612C)
    s0 = v0 + v1;
    v0 = lh(s0);
    s2 = v0 << 16;
    v0 = lh(s0 + 0x2);
    a0 = s2;
    s1 = v0 << 16;
    a1 = s1;
    _thunk_R_PointInSubsector();
    a0 = s2;
    v0 = lw(v0);
    a1 = s1;
    a2 = lw(v0);
    a3 = 0x1E;                                          // Result = 0000001E
    P_SpawnMObj();
    a0 = v0;
    a1 = 3;                                             // Result = 00000003
    S_StartSound();
    a3 = 0;                                             // Result = 00000000
    v1 = 0;                                             // Result = 00000000
    a0 = lh(s0 + 0x6);
loc_8001C91C:
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    at += v1;
    v0 = lw(at);
    a2 = 0x80000000;                                    // Result = 80000000
    if (a0 == v0) goto loc_8001C948;
    a3++;
    v0 = (i32(a3) < 0x7F);
    v1 += 0x58;
    if (v0 != 0) goto loc_8001C91C;
loc_8001C948:
    v0 = a3 << 1;
    v0 += a3;
    v0 <<= 2;
    v0 -= a3;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F70;                                       // Result = MObjInfo_MT_PLAYER[15] (8005E090)
    at += v0;
    v0 = lw(at);
    v0 &= 0x100;
    a0 = s2;
    if (v0 == 0) goto loc_8001C984;
    a2 = 0x7FFF0000;                                    // Result = 7FFF0000
    a2 |= 0xFFFF;                                       // Result = 7FFFFFFF
loc_8001C984:
    a1 = s1;
    P_SpawnMObj();
    a1 = 0xB60B0000;                                    // Result = B60B0000
    v1 = lhu(s0 + 0x4);
    a1 |= 0x60B7;                                       // Result = B60B60B7
    v1 <<= 16;
    a0 = u32(i32(v1) >> 16);
    mult(a0, a1);
    a1 = v0;
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 += a0;
    v0 = u32(i32(v0) >> 5);
    v0 -= v1;
    v0 <<= 29;
    sw(v0, a1 + 0x24);
    v0 = lhu(s0);
    sh(v0, a1 + 0x88);
    v0 = lhu(s0 + 0x2);
    sh(v0, a1 + 0x8A);
    v0 = lhu(s0 + 0x6);
    sh(v0, a1 + 0x8C);
    v0 = lw(gp + 0xBA0);                                // Load from: gItemRespawnQueueTail (80078180)
    v1 = lhu(s0 + 0x4);
    v0++;
    sw(v0, gp + 0xBA0);                                 // Store to: gItemRespawnQueueTail (80078180)
    sh(v1, a1 + 0x8E);
loc_8001C9FC:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_SetMObjState() noexcept {
loc_8001CA18:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    if (a1 != 0) goto loc_8001CB28;
    v0 = 0x20000;                                       // Result = 00020000
    v1 = lw(s0 + 0x64);
    v0 |= 1;                                            // Result = 00020001
    v1 &= v0;
    v0 = 1;                                             // Result = 00000001
    sw(0, s0 + 0x60);
    if (v1 != v0) goto loc_8001CAF0;
    v1 = lw(s0 + 0x54);
    v0 = 0x2E;                                          // Result = 0000002E
    if (v1 == v0) goto loc_8001CAF0;
    v0 = 0x30;                                          // Result = 00000030
    if (v1 == v0) goto loc_8001CAF0;
    a1 = lw(gp + 0xB58);                                // Load from: gItemRespawnQueueHead (80078138)
    v1 = *gTicCon;
    a0 = a1 & 0x3F;
    v0 = a0 << 2;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7910;                                       // Result = gItemRespawnTime[0] (80097910)
    at += v0;
    sw(v1, at);
    v0 += a0;
    v1 = lhu(s0 + 0x88);
    v0 <<= 1;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x612C;                                       // Result = gItemRespawnQueue[0] (8008612C)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8A);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x612E;                                       // Result = gItemRespawnQueue[1] (8008612E)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8C);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6132;                                       // Result = gItemRespawnQueue[3] (80086132)
    at += v0;
    sh(v1, at);
    v1 = lhu(s0 + 0x8E);
    a1++;
    sw(a1, gp + 0xB58);                                 // Store to: gItemRespawnQueueHead (80078138)
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6130;                                       // Result = gItemRespawnQueue[2] (80086130)
    at += v0;
    sh(v1, at);
loc_8001CAF0:
    a0 = s0;
    _thunk_P_UnsetThingPosition();
    v1 = lw(s0 + 0x14);
    v0 = lw(s0 + 0x10);
    a0 = *gpMainMemZone;
    sw(v0, v1 + 0x10);
    v1 = lw(s0 + 0x10);
    v0 = lw(s0 + 0x14);
    a1 = s0;
    sw(v0, v1 + 0x14);
    _thunk_Z_Free2();
    v0 = 0;                                             // Result = 00000000
    goto loc_8001CB88;
loc_8001CB28:
    v0 = a1 << 3;
    v0 -= a1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v1 = lw(v0 + 0x4);
    sw(v1, s0 + 0x2C);
    v0 = lw(v0 + 0xC);
    if (v0 == 0) goto loc_8001CB80;
    a0 = s0;
    ptr_call(v0);
loc_8001CB80:
    sw(0, s0 + 0x18);
    v0 = 1;                                             // Result = 00000001
loc_8001CB88:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_ExplodeMissile() noexcept {
loc_8001CB9C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x54);
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    a1 = lw(at);
    P_SetMObjState();
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001CC0C;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001CC0C:
    a0 = 0xFFFE0000;                                    // Result = FFFE0000
    a0 |= 0xFFFF;                                       // Result = FFFEFFFF
    v0 = lw(s0 + 0x64);
    v1 = lw(s0 + 0x58);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    v0 = lw(v1 + 0x38);
    if (v0 == 0) goto loc_8001CC54;
    a0 = lw(s0 + 0x74);
    S_StopSound();
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x38);
    a0 = s0;
    S_StartSound();
loc_8001CC54:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_SpawnMObj() noexcept {
loc_8001CC68:
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s4, sp + 0x20);
    s4 = a2;
    sw(s0, sp + 0x10);
    s0 = a3;
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s3, sp + 0x1C);
    _thunk_Z_Malloc();
    s3 = v0;
    a0 = s3;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = s0 << 1;
    v0 += s0;
    v0 <<= 2;
    v0 -= s0;
    v0 <<= 3;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    v0 += v1;
    sw(s0, s3 + 0x54);
    sw(v0, s3 + 0x58);
    sw(s1, s3);
    sw(s2, s3 + 0x4);
    v1 = lw(v0 + 0x40);
    sw(v1, s3 + 0x40);
    v1 = lw(v0 + 0x44);
    sw(v1, s3 + 0x44);
    v1 = lw(v0 + 0x54);
    sw(v1, s3 + 0x64);
    v1 = lw(v0 + 0x8);
    sw(v1, s3 + 0x68);
    v1 = lw(v0 + 0x14);
    sw(v1, s3 + 0x78);
    v1 = lw(v0 + 0x4);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s3 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s3 + 0x5C);
    v1 = lw(v0);
    sw(v1, s3 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s3;
    sw(v0, s3 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s3 + 0xC);
    v0 = lw(v0);
    v1 = lw(s3 + 0xC);
    v0 = lw(v0);
    sw(v0, s3 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s3 + 0x3C);
    if (s4 != v0) goto loc_8001CDB8;
    v0 = lw(s3 + 0x38);
    sw(v0, s3 + 0x8);
    goto loc_8001CDE8;
loc_8001CDB8:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s4 != v0) goto loc_8001CDE4;
    v0 = lw(s3 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s3 + 0x8);
    goto loc_8001CDE8;
loc_8001CDE4:
    sw(s4, s3 + 0x8);
loc_8001CDE8:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s3, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s3 + 0x14);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    v0 = s3;
    sw(v1, v0 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_SpawnPlayer() noexcept {
loc_8001CE40:
    sp -= 0x38;
    sw(s4, sp + 0x28);
    s4 = a0;
    sw(ra, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lh(s4 + 0x6);
    v1 = a0 << 2;
    at = *gpSectors;
    at += v1;
    v0 = lw(at);
    v1 += a0;
    if (v0 != 0) goto loc_8001CE94;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001D15C;
loc_8001CE94:
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7940;                                       // Result = gTmpWadLumpBuffer[3FDE] (800A86C0)
    s3 = v0 + v1;
    v0 = lw(s3 + 0x4);
    s5 = 2;                                             // Result = 00000002
    a1 = 0x94;                                          // Result = 00000094
    if (v0 != s5) goto loc_8001CEC8;
    a0--;
    G_PlayerReborn();
    a1 = 0x94;                                          // Result = 00000094
loc_8001CEC8:
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    s1 = lh(s4);
    s2 = lh(s4 + 0x2);
    s1 <<= 16;
    s2 <<= 16;
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    sw(0, s0 + 0x54);
    sw(v0, s0 + 0x58);
    sw(s1, s0);
    sw(s2, s0 + 0x4);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F84);                               // Load from: MObjInfo_MT_PLAYER[10] (8005E07C)
    sw(v0, s0 + 0x40);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F80);                               // Load from: MObjInfo_MT_PLAYER[11] (8005E080)
    sw(v0, s0 + 0x44);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1F70);                               // Load from: MObjInfo_MT_PLAYER[15] (8005E090)
    sw(v0, s0 + 0x64);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1FBC);                               // Load from: MObjInfo_MT_PLAYER[2] (8005E044)
    sw(v0, s0 + 0x68);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1FB0);                               // Load from: MObjInfo_MT_PLAYER[5] (8005E050)
    sw(v0, s0 + 0x78);
    v1 = 0x80060000;                                    // Result = 80060000
    v1 = lw(v1 - 0x1FC0);                               // Load from: MObjInfo_MT_PLAYER[1] (8005E040)
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s0;
    sw(v0, s0 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v1 = lw(s0 + 0xC);
    v0 = lw(v0);
    sw(v0, s0 + 0x38);
    v0 = lw(v1);
    v1 = lw(s0 + 0x38);
    v0 = lw(v0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x3C);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s0, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    a1 = 0xB60B0000;                                    // Result = B60B0000
    sw(v0, s0 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    v1 = lhu(s4 + 0x4);
    a1 |= 0x60B7;                                       // Result = B60B60B7
    v1 <<= 16;
    a0 = u32(i32(v1) >> 16);
    mult(a0, a1);
    sw(s3, s0 + 0x80);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 += a0;
    v0 = u32(i32(v0) >> 5);
    v0 -= v1;
    v0 <<= 29;
    sw(v0, s0 + 0x24);
    v0 = lw(s3 + 0x24);
    v1 = 0x290000;                                      // Result = 00290000
    sw(v0, s0 + 0x68);
    v0 = 0x24;                                          // Result = 00000024
    sw(s0, s3);
    sw(0, s3 + 0x4);
    sw(0, s3 + 0xC4);
    sw(0, s3 + 0xD4);
    sw(0, s3 + 0xD8);
    sw(0, s3 + 0xDC);
    sw(0, s3 + 0xE4);
    sw(0, s3 + 0xE8);
    sw(v1, s3 + 0x18);
    sw(v0, s3 + 0x120);
    v0 = lw(s0 + 0x8);
    v0 += v1;
    sw(v0, s3 + 0x14);
    a0 = lh(s4 + 0x6);
    a0--;
    P_SetupPsprites();
    v0 = *gNetGame;
    if (v0 != s5) goto loc_8001D0DC;
    a0 = 1;                                             // Result = 00000001
    v1 = 5;                                             // Result = 00000005
    v0 = s3 + 0x14;
loc_8001D0C4:
    sw(a0, v0 + 0x48);
    v1--;
    v0 -= 4;
    if (i32(v1) >= 0) goto loc_8001D0C4;
    v0 = *gNetGame;
loc_8001D0DC:
    if (v0 != 0) goto loc_8001D130;
    v0 = lh(s4 + 0x6);
    v1 = *gCurPlayerIndex;
    v0--;
    {
        const bool bJump = (v0 != v1);
        v0 = s0;
        if (bJump) goto loc_8001D15C;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C3C);                               // Load from: gbUsingAPassword (80077C3C)
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_8001D130;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    a2 = a1;
    a3 = s3;
    P_ProcessPassword();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7C3C);                                 // Store to: gbUsingAPassword (80077C3C)
loc_8001D130:
    v0 = lh(s4 + 0x6);
    v1 = *gCurPlayerIndex;
    v0--;
    {
        const bool bJump = (v0 != v1);
        v0 = s0;
        if (bJump) goto loc_8001D15C;
    }
    ST_Start();
    I_UpdatePalette();
    v0 = s0;
loc_8001D15C:
    ra = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

void P_SpawnMapThing() noexcept {
loc_8001D184:
    sp -= 0x30;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x2C);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lh(s2 + 0x6);
    v0 = (i32(v1) < 3);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001D23C;
    }
    v0 += v1;
    v0 <<= 1;
    v1 = lwl(v1, s2 + 0x3);
    v1 = lwr(v1, s2);
    a0 = lwl(a0, s2 + 0x7);
    a0 = lwr(a0, s2 + 0x4);
    a1 = lh(s2 + 0x8);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x718B;                                       // Result = gPlayer0MapThing[1] (800A8E75)
    at += v0;
    swl(v1, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x718E;                                       // Result = gPlayer0MapThing[0] (800A8E72)
    at += v0;
    swr(v1, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7187;                                       // Result = gPlayer0MapThing[3] (800A8E79)
    at += v0;
    swl(a0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x718A;                                       // Result = gPlayer0MapThing[2] (800A8E76)
    at += v0;
    swr(a0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7186;                                       // Result = gPlayer0MapThing[4] (800A8E7A)
    at += v0;
    sh(a1, at);
    goto loc_8001D6D8;
loc_8001D23C:
    v0 = 0xB;                                           // Result = 0000000B
    a0 = 2;                                             // Result = 00000002
    if (v1 != v0) goto loc_8001D28C;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FA0);                               // Load from: gpDeathmatchP (80078060)
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x7F30;                                       // Result = 800980D0
    v0 = (a0 < v0);
    a1 = s2;
    if (v0 == 0) goto loc_8001D6D8;
    a2 = 0xA;                                           // Result = 0000000A
    _thunk_D_memcpy();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FA0);                               // Load from: gpDeathmatchP (80078060)
    v0 += 0xA;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FA0);                                // Store to: gpDeathmatchP (80078060)
    goto loc_8001D6D8;
loc_8001D28C:
    v0 = *gNetGame;
    if (v0 == a0) goto loc_8001D2B4;
    v0 = lhu(s2 + 0x8);
    v0 &= 0x10;
    if (v0 != 0) goto loc_8001D6D8;
loc_8001D2B4:
    v1 = *gGameSkill;
    v0 = (v1 < 2);
    if (v0 == 0) goto loc_8001D2D4;
    a1 = 1;                                             // Result = 00000001
    goto loc_8001D2F4;
loc_8001D2D4:
    v0 = v1 - 3;
    if (v1 != a0) goto loc_8001D2E4;
    a1 = 2;                                             // Result = 00000002
    goto loc_8001D2F4;
loc_8001D2E4:
    v0 = (v0 < 2);
    if (v0 == 0) goto loc_8001D2F4;
    a1 = 4;                                             // Result = 00000004
loc_8001D2F4:
    v0 = lh(s2 + 0x8);
    v0 &= a1;
    s1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001D6D8;
    a1 = lh(s2 + 0x6);
    v1 = 0;                                             // Result = 00000000
loc_8001D310:
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    at += v1;
    v0 = lw(at);
    {
        const bool bJump = (a1 == v0);
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_8001D340;
    }
    s1++;
    v0 = (i32(s1) < 0x7F);
    v1 += 0x58;
    if (v0 != 0) goto loc_8001D310;
    v0 = 0x7F;                                          // Result = 0000007F
loc_8001D340:
    if (s1 != v0) goto loc_8001D360;
    I_Error("P_SpawnMapThing: Unknown doomednum %d at (%d, %d)", (int32_t) a1, (int32_t) a2, (int32_t) a3);
loc_8001D360:
    v1 = *gNetGame;
    v0 = 2;
    s3 = 0x80000000;
    if (v1 != v0) goto loc_8001D3A8;
    v0 = s1 << 1;
    v0 += s1;
    v0 <<= 2;
    v0 -= s1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F70;                                       // Result = MObjInfo_MT_PLAYER[15] (8005E090)
    at += v0;
    v0 = lw(at);
    v1 = 0x2400000;                                     // Result = 02400000
    v0 &= v1;
    if (v0 != 0) goto loc_8001D6D8;
loc_8001D3A8:
    v0 = lh(s2);
    v1 = lh(s2 + 0x2);
    s6 = v0 << 16;
    v0 = s1 << 1;
    v0 += s1;
    v0 <<= 2;
    v0 -= s1;
    s4 = v0 << 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F70;                                       // Result = MObjInfo_MT_PLAYER[15] (8005E090)
    at += s4;
    v0 = lw(at);
    v0 &= 0x100;
    s5 = v1 << 16;
    if (v0 == 0) goto loc_8001D3F0;
    s3 = 0x7FFF0000;                                    // Result = 7FFF0000
    s3 |= 0xFFFF;                                       // Result = 7FFFFFFF
loc_8001D3F0:
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    v0 += s4;                                           // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    sw(s1, s0 + 0x54);
    sw(v0, s0 + 0x58);
    sw(s6, s0);
    sw(s5, s0 + 0x4);
    v1 = lw(v0 + 0x40);                                 // Load from: MObjInfo_MT_PLAYER[10] (8005E07C)
    sw(v1, s0 + 0x40);
    v1 = lw(v0 + 0x44);                                 // Load from: MObjInfo_MT_PLAYER[11] (8005E080)
    sw(v1, s0 + 0x44);
    v1 = lw(v0 + 0x54);                                 // Load from: MObjInfo_MT_PLAYER[15] (8005E090)
    sw(v1, s0 + 0x64);
    v1 = lw(v0 + 0x8);                                  // Load from: MObjInfo_MT_PLAYER[2] (8005E044)
    sw(v1, s0 + 0x68);
    v1 = lw(v0 + 0x14);                                 // Load from: MObjInfo_MT_PLAYER[5] (8005E050)
    sw(v1, s0 + 0x78);
    v1 = lw(v0 + 0x4);                                  // Load from: MObjInfo_MT_PLAYER[1] (8005E040)
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s0;
    sw(v0, s0 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v1 = lw(s0 + 0xC);
    v0 = lw(v0);
    sw(v0, s0 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s0 + 0x3C);
    if (s3 != v0) goto loc_8001D500;
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
    goto loc_8001D530;
loc_8001D500:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s3 != v0) goto loc_8001D52C;
    v0 = lw(s0 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s0 + 0x8);
    goto loc_8001D530;
loc_8001D52C:
    sw(s3, s0 + 0x8);
loc_8001D530:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s0, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(v0, s0 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    v0 = lw(s0 + 0x5C);
    if (i32(v0) <= 0) goto loc_8001D5BC;
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    div(v0, v1);
    if (v1 != 0) goto loc_8001D594;
    _break(0x1C00);
loc_8001D594:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001D5AC;
    }
    if (v0 != at) goto loc_8001D5AC;
    tge(zero, zero, 0x5D);
loc_8001D5AC:
    v1 = hi;
    v1++;
    sw(v1, s0 + 0x5C);
loc_8001D5BC:
    v0 = lw(s0 + 0x64);
    v1 = 0x400000;                                      // Result = 00400000
    v0 &= v1;
    v1 = 0x800000;                                      // Result = 00800000
    if (v0 == 0) goto loc_8001D5E8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F20);                               // Load from: gTotalKills (80077F20)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F20);                                // Store to: gTotalKills (80077F20)
loc_8001D5E8:
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xB60B0000;                                // Result = B60B0000
        if (bJump) goto loc_8001D618;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F2C);                               // Load from: gTotalItems (80077F2C)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F2C);                                // Store to: gTotalItems (80077F2C)
    v0 = 0xB60B0000;                                    // Result = B60B0000
loc_8001D618:
    v1 = lhu(s2 + 0x4);
    v0 |= 0x60B7;                                       // Result = B60B60B7
    v1 <<= 16;
    a0 = u32(i32(v1) >> 16);
    mult(a0, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 += a0;
    v0 = u32(i32(v0) >> 5);
    v0 -= v1;
    v0 <<= 29;
    sw(v0, s0 + 0x24);
    v0 = lhu(s2);
    sh(v0, s0 + 0x88);
    v0 = lhu(s2 + 0x2);
    sh(v0, s0 + 0x8A);
    v0 = lhu(s2 + 0x6);
    sh(v0, s0 + 0x8C);
    v0 = lhu(s2 + 0x4);
    sh(v0, s0 + 0x8E);
    v0 = lhu(s2 + 0x8);
    v0 &= 8;
    if (v0 == 0) goto loc_8001D69C;
    v0 = lw(s0 + 0x64);
    v0 |= 0x20;
    sw(v0, s0 + 0x64);
loc_8001D69C:
    v0 = lhu(s2 + 0x8);
    v1 = lw(s0 + 0x64);
    v0 &= 0xE0;
    v0 <<= 23;
    v0 |= v1;
    v1 = 0x70000000;                                    // Result = 70000000
    sw(v0, s0 + 0x64);
    v0 &= v1;
    v1 = 0x50000000;                                    // Result = 50000000
    if (v0 != v1) goto loc_8001D6D8;
    v0 = lw(s0 + 0x68);
    v0 <<= 1;
    sw(v0, s0 + 0x68);
loc_8001D6D8:
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

void P_SpawnPuff() noexcept {
loc_8001D704:
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x10);
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 10;
    s3 += s0;
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = 0x1B;                                          // Result = 0000001B
    sw(v0, s0 + 0x54);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x167C;                                       // Result = MObjInfo_MT_PUFF[0] (8005E984)
    sw(v0, s0 + 0x58);
    sw(s1, s0);
    sw(s2, s0 + 0x4);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x163C);                               // Load from: MObjInfo_MT_PUFF[10] (8005E9C4)
    sw(v0, s0 + 0x40);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1638);                               // Load from: MObjInfo_MT_PUFF[11] (8005E9C8)
    sw(v0, s0 + 0x44);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1628);                               // Load from: MObjInfo_MT_PUFF[15] (8005E9D8)
    sw(v0, s0 + 0x64);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1674);                               // Load from: MObjInfo_MT_PUFF[2] (8005E98C)
    sw(v0, s0 + 0x68);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1668);                               // Load from: MObjInfo_MT_PUFF[5] (8005E998)
    sw(v0, s0 + 0x78);
    v1 = 0x80060000;                                    // Result = 80060000
    v1 = lw(v1 - 0x1678);                               // Load from: MObjInfo_MT_PUFF[1] (8005E988)
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s0;
    sw(v0, s0 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v1 = lw(s0 + 0xC);
    v0 = lw(v0);
    sw(v0, s0 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s0 + 0x3C);
    if (s3 != v0) goto loc_8001D868;
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
    goto loc_8001D898;
loc_8001D868:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s3 != v0) goto loc_8001D894;
    v0 = lw(s0 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s0 + 0x8);
    goto loc_8001D898;
loc_8001D894:
    sw(s3, s0 + 0x8);
loc_8001D898:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s0, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(v0, s0 + 0x10);
    v0 = 0x10000;                                       // Result = 00010000
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    sw(v0, s0 + 0x50);
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001D8F4;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001D8F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    v0 = 0x460000;                                      // Result = 00460000
    a0 = s0;
    if (v1 != v0) goto loc_8001D910;
    a1 = 0x5F;                                          // Result = 0000005F
    P_SetMObjState();
loc_8001D910:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_SpawnBlood() noexcept {
loc_8001D930:
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s4, sp + 0x20);
    s4 = a3;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x10);
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 10;
    s3 += s0;
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = 0x1C;                                          // Result = 0000001C
    sw(v0, s0 + 0x54);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x1624;                                       // Result = MObjInfo_MT_BLOOD[0] (8005E9DC)
    sw(v0, s0 + 0x58);
    sw(s1, s0);
    sw(s2, s0 + 0x4);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x15E4);                               // Load from: MObjInfo_MT_BLOOD[10] (8005EA1C)
    sw(v0, s0 + 0x40);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x15E0);                               // Load from: MObjInfo_MT_BLOOD[11] (8005EA20)
    sw(v0, s0 + 0x44);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x15D0);                               // Load from: MObjInfo_MT_BLOOD[15] (8005EA30)
    sw(v0, s0 + 0x64);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x161C);                               // Load from: MObjInfo_MT_BLOOD[2] (8005E9E4)
    sw(v0, s0 + 0x68);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 = lw(v0 - 0x1610);                               // Load from: MObjInfo_MT_BLOOD[5] (8005E9F0)
    sw(v0, s0 + 0x78);
    v1 = 0x80060000;                                    // Result = 80060000
    v1 = lw(v1 - 0x1620);                               // Load from: MObjInfo_MT_BLOOD[1] (8005E9E0)
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s0;
    sw(v0, s0 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v1 = lw(s0 + 0xC);
    v0 = lw(v0);
    sw(v0, s0 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s0 + 0x3C);
    if (s3 != v0) goto loc_8001DA9C;
    v0 = lw(s0 + 0x38);
    sw(v0, s0 + 0x8);
    goto loc_8001DACC;
loc_8001DA9C:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s3 != v0) goto loc_8001DAC8;
    v0 = lw(s0 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s0 + 0x8);
    goto loc_8001DACC;
loc_8001DAC8:
    sw(s3, s0 + 0x8);
loc_8001DACC:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s0, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(v0, s0 + 0x10);
    v0 = 0x20000;                                       // Result = 00020000
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    sw(v0, s0 + 0x50);
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001DB28;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001DB28:
    v0 = s4 - 9;
    v0 = (v0 < 4);
    a0 = s0;
    if (v0 == 0) goto loc_8001DB40;
    a1 = 0x5B;                                          // Result = 0000005B
    goto loc_8001DB4C;
loc_8001DB40:
    v0 = (i32(s4) < 9);
    a1 = 0x5C;                                          // Result = 0000005C
    if (v0 == 0) goto loc_8001DB54;
loc_8001DB4C:
    P_SetMObjState();
loc_8001DB54:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_CheckMissileSpawn() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x48);
    v1 = lw(s0);
    a2 = lw(s0 + 0x4);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s0 + 0x4C);
    a1 = lw(s0);
    v1 = lw(s0 + 0x50);
    v0 = u32(i32(v0) >> 1);
    v0 += a2;
    v1 = u32(i32(v1) >> 1);
    sw(v0, s0 + 0x4);
    v0 = lw(s0 + 0x8);
    a2 = lw(s0 + 0x4);
    v1 += v0;
    sw(v1, s0 + 0x8);
    P_TryMove();
    if (v0 != 0) goto loc_8001DC80;
    v1 = lw(s0 + 0x54);
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    a1 = lw(at);
    a0 = s0;
    P_SetMObjState();
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001DC38;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001DC38:
    a0 = 0xFFFE0000;                                    // Result = FFFE0000
    a0 |= 0xFFFF;                                       // Result = FFFEFFFF
    v0 = lw(s0 + 0x64);
    v1 = lw(s0 + 0x58);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    v0 = lw(v1 + 0x38);
    if (v0 == 0) goto loc_8001DC80;
    a0 = lw(s0 + 0x74);
    S_StopSound();
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x38);
    a0 = s0;
    S_StartSound();
loc_8001DC80:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_SpawnMissile() noexcept {
loc_8001DC94:
    sp -= 0x30;
    sw(s4, sp + 0x20);
    s4 = a0;
    sw(s6, sp + 0x28);
    s6 = a1;
    sw(s0, sp + 0x10);
    s0 = a2;
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    v0 = 0x200000;                                      // Result = 00200000
    sw(ra, sp + 0x2C);
    sw(s5, sp + 0x24);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s4 + 0x8);
    s1 = lw(s4);
    s2 = lw(s4 + 0x4);
    s5 = v1 + v0;
    _thunk_Z_Malloc();
    s3 = v0;
    a0 = s3;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = s0 << 1;
    v0 += s0;
    v0 <<= 2;
    v0 -= s0;
    v0 <<= 3;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    v0 += v1;
    sw(s0, s3 + 0x54);
    sw(v0, s3 + 0x58);
    sw(s1, s3);
    sw(s2, s3 + 0x4);
    v1 = lw(v0 + 0x40);
    sw(v1, s3 + 0x40);
    v1 = lw(v0 + 0x44);
    sw(v1, s3 + 0x44);
    v1 = lw(v0 + 0x54);
    sw(v1, s3 + 0x64);
    v1 = lw(v0 + 0x8);
    sw(v1, s3 + 0x68);
    v1 = lw(v0 + 0x14);
    sw(v1, s3 + 0x78);
    v1 = lw(v0 + 0x4);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s3 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s3 + 0x5C);
    v1 = lw(v0);
    sw(v1, s3 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s3;
    sw(v0, s3 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s3 + 0xC);
    v0 = lw(v0);
    v1 = lw(s3 + 0xC);
    v0 = lw(v0);
    sw(v0, s3 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s3 + 0x3C);
    if (s5 != v0) goto loc_8001DDFC;
    v0 = lw(s3 + 0x38);
    sw(v0, s3 + 0x8);
    goto loc_8001DE2C;
loc_8001DDFC:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s5 != v0) goto loc_8001DE28;
    v0 = lw(s3 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s3 + 0x8);
    goto loc_8001DE2C;
loc_8001DE28:
    sw(s5, s3 + 0x8);
loc_8001DE2C:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s3, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s3 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    s1 = s3;
    sw(v0, s3 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s3, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x10);
    if (a1 == 0) goto loc_8001DE80;
    a0 = s4;
    S_StartSound();
loc_8001DE80:
    sw(s4, s1 + 0x74);
    a0 = lw(s4);
    a1 = lw(s4 + 0x4);
    a2 = lw(s6);
    a3 = lw(s6 + 0x4);
    R_PointToAngle2();
    s2 = v0;
    v0 = lw(s6 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_8001DED0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s2 += s0;
loc_8001DED0:
    sw(s2, s1 + 0x24);
    s2 >>= 19;
    a0 = s2 << 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(s1 + 0x58);
    v0 += a0;
    v1 = lh(v1 + 0x3E);
    v0 = lw(v0);
    mult(v1, v0);
    v0 = lo;
    sw(v0, s1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a0;
    v0 = lw(at);
    mult(v1, v0);
    v0 = lo;
    sw(v0, s1 + 0x4C);
    v1 = lw(s6);
    a0 = lw(s4);
    v0 = lw(s6 + 0x4);
    a1 = lw(s4 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    P_AproxDistance();
    v1 = lw(s1 + 0x58);
    v1 = lw(v1 + 0x3C);
    div(v0, v1);
    if (v1 != 0) goto loc_8001DF60;
    _break(0x1C00);
loc_8001DF60:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001DF78;
    }
    if (v0 != at) goto loc_8001DF78;
    tge(zero, zero, 0x5D);
loc_8001DF78:
    v1 = lo;
    a0 = s1;
    if (i32(v1) > 0) goto loc_8001DF8C;
    v1 = 1;                                             // Result = 00000001
loc_8001DF8C:
    a3 = lw(s6 + 0x8);
    v0 = lw(s4 + 0x8);
    a3 -= v0;
    div(a3, v1);
    if (v1 != 0) goto loc_8001DFAC;
    _break(0x1C00);
loc_8001DFAC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001DFC4;
    }
    if (a3 != at) goto loc_8001DFC4;
    tge(zero, zero, 0x5D);
loc_8001DFC4:
    a3 = lo;
    v0 = lw(s1 + 0x48);
    v1 = lw(s1);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s1);
    a1 = lw(s1);
    v0 = lw(s1 + 0x4C);
    v1 = lw(s1 + 0x4);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s1 + 0x4);
    a2 = lw(s1 + 0x4);
    sw(a3, s1 + 0x50);
    v0 = lw(s1 + 0x50);
    v1 = lw(s1 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s1 + 0x8);
    P_TryMove();
    {
        const bool bJump = (v0 != 0);
        v0 = s1;
        if (bJump) goto loc_8001E0C8;
    }
    v1 = lw(s1 + 0x54);
    sw(0, s1 + 0x50);
    sw(0, s1 + 0x4C);
    sw(0, s1 + 0x48);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    a1 = lw(at);
    a0 = s1;
    P_SetMObjState();
    _thunk_P_Random();
    v1 = lw(s1 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s1 + 0x5C);
    if (i32(v1) > 0) goto loc_8001E07C;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x5C);
loc_8001E07C:
    a0 = 0xFFFE0000;                                    // Result = FFFE0000
    a0 |= 0xFFFF;                                       // Result = FFFEFFFF
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x58);
    v0 &= a0;
    sw(v0, s1 + 0x64);
    v0 = lw(v1 + 0x38);
    {
        const bool bJump = (v0 == 0);
        v0 = s1;
        if (bJump) goto loc_8001E0C8;
    }
    a0 = lw(s1 + 0x74);
    S_StopSound();
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0x38);
    a0 = s1;
    S_StartSound();
    v0 = s1;
loc_8001E0C8:
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

void P_SpawnPlayerMissile() noexcept {
loc_8001E0F4:
    sp -= 0x38;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s5, sp + 0x24);
    s5 = a1;
    sw(ra, sp + 0x30);
    sw(s7, sp + 0x2C);
    sw(s6, sp + 0x28);
    sw(s4, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s4 = lw(s3 + 0x24);
    a2 = 0x4000000;                                     // Result = 04000000
    a1 = s4;
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    s7 = v0;
    if (v1 != 0) goto loc_8001E1A8;
    v0 = 0x4000000;                                     // Result = 04000000
    s4 += v0;
    a0 = s3;
    a1 = s4;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    s7 = v0;
    if (v1 != 0) goto loc_8001E1A8;
    v0 = 0xF8000000;                                    // Result = F8000000
    s4 += v0;
    a0 = s3;
    a1 = s4;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    s7 = v0;
    if (v1 != 0) goto loc_8001E1A8;
    s4 = lw(s3 + 0x24);
    s7 = 0;                                             // Result = 00000000
loc_8001E1A8:
    a1 = 0x94;                                          // Result = 00000094
    a2 = 2;                                             // Result = 00000002
    v0 = 0x200000;                                      // Result = 00200000
    a3 = 0;                                             // Result = 00000000
    a0 = *gpMainMemZone;
    v1 = lw(s3 + 0x8);
    s0 = lw(s3);
    s1 = lw(s3 + 0x4);
    s6 = v1 + v0;
    _thunk_Z_Malloc();
    s2 = v0;
    a0 = s2;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v0 = s5 << 1;
    v0 += s5;
    v0 <<= 2;
    v0 -= s5;
    v0 <<= 3;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x1FC4;                                       // Result = MObjInfo_MT_PLAYER[0] (8005E03C)
    v0 += v1;
    sw(s5, s2 + 0x54);
    sw(v0, s2 + 0x58);
    sw(s0, s2);
    sw(s1, s2 + 0x4);
    v1 = lw(v0 + 0x40);
    sw(v1, s2 + 0x40);
    v1 = lw(v0 + 0x44);
    sw(v1, s2 + 0x44);
    v1 = lw(v0 + 0x54);
    sw(v1, s2 + 0x64);
    v1 = lw(v0 + 0x8);
    sw(v1, s2 + 0x68);
    v1 = lw(v0 + 0x14);
    sw(v1, s2 + 0x78);
    v1 = lw(v0 + 0x4);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s2 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s2 + 0x5C);
    v1 = lw(v0);
    sw(v1, s2 + 0x28);
    v0 = lw(v0 + 0x4);
    a0 = s2;
    sw(v0, s2 + 0x2C);
    _thunk_P_SetThingPosition();
    v0 = lw(s2 + 0xC);
    v0 = lw(v0);
    v1 = lw(s2 + 0xC);
    v0 = lw(v0);
    sw(v0, s2 + 0x38);
    v0 = lw(v1);
    v1 = lw(v0 + 0x4);
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v1, s2 + 0x3C);
    if (s6 != v0) goto loc_8001E2E0;
    v0 = lw(s2 + 0x38);
    sw(v0, s2 + 0x8);
    goto loc_8001E310;
loc_8001E2E0:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    if (s6 != v0) goto loc_8001E30C;
    v0 = lw(s2 + 0x58);
    v0 = lw(v0 + 0x44);
    v0 = v1 - v0;
    sw(v0, s2 + 0x8);
    goto loc_8001E310;
loc_8001E30C:
    sw(s6, s2 + 0x8);
loc_8001E310:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(s2, v0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    sw(v0, s2 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x7160);                               // Load from: gMObjHead[4] (800A8EA0)
    sw(v0, s2 + 0x10);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s2, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    v0 = lw(s2 + 0x58);
    a1 = lw(v0 + 0x10);
    v1 = s4 >> 19;
    if (a1 == 0) goto loc_8001E368;
    a0 = s3;
    S_StartSound();
    v1 = s4 >> 19;
loc_8001E368:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 = lw(s2 + 0x58);
    v1 <<= 2;
    sw(s3, s2 + 0x74);
    sw(s4, s2 + 0x24);
    v0 += v1;
    a0 = lh(a0 + 0x3E);
    v0 = lw(v0);
    mult(a0, v0);
    v0 = lo;
    sw(v0, s2 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v1 = lw(s2);
    v0 = lw(s2 + 0x48);
    a2 = lo;
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    mult(a0, s7);
    sw(v0, s2);
    a1 = lw(s2);
    v1 = lw(s2 + 0x4);
    sw(a2, s2 + 0x4C);
    v0 = lw(s2 + 0x4C);
    a0 = s2;
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s2 + 0x4);
    a2 = lw(s2 + 0x4);
    v0 = lo;
    sw(v0, s2 + 0x50);
    v0 = lw(s2 + 0x50);
    v1 = lw(s2 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    sw(v0, s2 + 0x8);
    P_TryMove();
    if (v0 != 0) goto loc_8001E4C4;
    v1 = lw(s2 + 0x54);
    sw(0, s2 + 0x50);
    sw(0, s2 + 0x4C);
    sw(0, s2 + 0x48);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    a1 = lw(at);
    a0 = s2;
    P_SetMObjState();
    _thunk_P_Random();
    v1 = lw(s2 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s2 + 0x5C);
    if (i32(v1) > 0) goto loc_8001E47C;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s2 + 0x5C);
loc_8001E47C:
    a0 = 0xFFFE0000;                                    // Result = FFFE0000
    a0 |= 0xFFFF;                                       // Result = FFFEFFFF
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x58);
    v0 &= a0;
    sw(v0, s2 + 0x64);
    v0 = lw(v1 + 0x38);
    if (v0 == 0) goto loc_8001E4C4;
    a0 = lw(s2 + 0x74);
    S_StopSound();
    v0 = lw(s2 + 0x58);
    a1 = lw(v0 + 0x38);
    a0 = s2;
    S_StartSound();
loc_8001E4C4:
    ra = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

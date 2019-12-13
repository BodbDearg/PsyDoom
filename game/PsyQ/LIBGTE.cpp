#include "LIBGTE.h"

#include "LIBAPI.h"
#include "PsxVm/PsxVm.h"

void LIBGTE_MulMatrix() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 0);
    ctc2(t1, 1);
    ctc2(t2, 2);
    ctc2(t3, 3);
    ctc2(t4, 4);
    t0 = lhu(a1);
    t1 = lw(a1 + 0x4);
    t2 = lw(a1 + 0xC);
    at = 0xFFFF0000;                                    // Result = FFFF0000
    t1 &= at;
    t0 |= t1;
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t0 = lhu(a1 + 0x2);
    t1 = lw(a1 + 0x8);
    t2 = lh(a1 + 0xE);
    t1 <<= 16;
    t0 |= t1;
    t3 = mfc2(9);
    t4 = mfc2(10);
    t5 = mfc2(11);
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t0 = lhu(a1 + 0x4);
    t1 = lw(a1 + 0x8);
    t2 = lw(a1 + 0x10);
    at = 0xFFFF0000;                                    // Result = FFFF0000
    t1 &= at;
    t0 |= t1;
    t6 = mfc2(9);
    t7 = mfc2(10);
    t8 = mfc2(11);
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t3 &= 0xFFFF;
    t6 <<= 16;
    t6 |= t3;
    sw(t6, a0);
    t5 &= 0xFFFF;
    t8 <<= 16;
    t8 |= t5;
    sw(t8, a0 + 0xC);
    t0 = mfc2(9);
    t1 = mfc2(10);
    t0 &= 0xFFFF;
    t4 <<= 16;
    t0 |= t4;
    sw(t0, a0 + 0x4);
    t7 &= 0xFFFF;
    t1 <<= 16;
    t1 |= t7;
    sw(t1, a0 + 0x8);
    swc2(11, a0 + 0x10);
    v0 = a0;
    return;
}

void LIBGTE_MulMatrix2() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 0);
    ctc2(t1, 1);
    ctc2(t2, 2);
    ctc2(t3, 3);
    ctc2(t4, 4);
    t0 = lhu(a1);
    t1 = lw(a1 + 0x4);
    t2 = lw(a1 + 0xC);
    at = 0xFFFF0000;                                    // Result = FFFF0000
    t1 &= at;
    t0 |= t1;
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t0 = lhu(a1 + 0x2);
    t1 = lw(a1 + 0x8);
    t2 = lh(a1 + 0xE);
    t1 <<= 16;
    t0 |= t1;
    t3 = mfc2(9);
    t4 = mfc2(10);
    t5 = mfc2(11);
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t0 = lhu(a1 + 0x4);
    t1 = lw(a1 + 0x8);
    t2 = lw(a1 + 0x10);
    at = 0xFFFF0000;                                    // Result = FFFF0000
    t1 &= at;
    t0 |= t1;
    t6 = mfc2(9);
    t7 = mfc2(10);
    t8 = mfc2(11);
    mtc2(t0, 0);
    mtc2(t2, 1);
    cop2(0x486012);
    t3 &= 0xFFFF;
    t6 <<= 16;
    t6 |= t3;
    sw(t6, a1);
    t5 &= 0xFFFF;
    t8 <<= 16;
    t8 |= t5;
    sw(t8, a1 + 0xC);
    t0 = mfc2(9);
    t1 = mfc2(10);
    t0 &= 0xFFFF;
    t4 <<= 16;
    t0 |= t4;
    sw(t0, a1 + 0x4);
    t7 &= 0xFFFF;
    t1 <<= 16;
    t1 |= t7;
    sw(t1, a1 + 0x8);
    swc2(11, a1 + 0x10);
    v0 = a1;
    return;
}

void LIBGTE_ApplyMatrix() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 0);
    ctc2(t1, 1);
    ctc2(t2, 2);
    ctc2(t3, 3);
    ctc2(t4, 4);
    lwc2(0, a1);
    lwc2(1, a1 + 0x4);
    cop2(0x486012);
    swc2(25, a2);
    swc2(26, a2 + 0x4);
    swc2(27, a2 + 0x8);
    v0 = a2;
    return;
}

void LIBGTE_ApplyMatrixSV() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 0);
    ctc2(t1, 1);
    ctc2(t2, 2);
    ctc2(t3, 3);
    ctc2(t4, 4);
    lwc2(0, a1);
    lwc2(1, a1 + 0x4);
    cop2(0x486012);
    t0 = mfc2(9);
    t1 = mfc2(10);
    t2 = mfc2(11);
    sh(t0, a2);
    sh(t1, a2 + 0x2);
    sh(t2, a2 + 0x4);
    v0 = a2;
    return;
}

void LIBGTE_TransMatrix() noexcept {
    t0 = lw(a1);
    t1 = lw(a1 + 0x4);
    t2 = lw(a1 + 0x8);
    sw(t0, a0 + 0x14);
    sw(t1, a0 + 0x18);
    sw(t2, a0 + 0x1C);
    v0 = a0;
    return;
}

void LIBGTE_ScaleMatrix() noexcept {
    t0 = lw(a0);
    t3 = lw(a1);
    t1 = t0 & 0xFFFF;
    t1 <<= 16;
    t1 = u32(i32(t1) >> 16);
    multu(t1, t3);
    t4 = lw(a1 + 0x4);
    t2 = u32(i32(t0) >> 16);
    t5 = lw(a1 + 0x8);
    t0 = lw(a0 + 0x4);
    v0 = a0;
    t1 = lo;
    t1 = u32(i32(t1) >> 12);
    t1 &= 0xFFFF;
    multu(t2, t4);
    t2 = lo;
    t2 = u32(i32(t2) >> 12);
    t2 <<= 16;
    t1 |= t2;
    sw(t1, a0);
    t1 = t0 & 0xFFFF;
    t1 <<= 16;
    t1 = u32(i32(t1) >> 16);
    multu(t1, t5);
    t2 = u32(i32(t0) >> 16);
    t0 = lw(a0 + 0x8);
    t1 = lo;
    t1 = u32(i32(t1) >> 12);
    t1 &= 0xFFFF;
    multu(t2, t3);
    t2 = lo;
    t2 = u32(i32(t2) >> 12);
    t2 <<= 16;
    t1 |= t2;
    sw(t1, a0 + 0x4);
    t1 = t0 & 0xFFFF;
    t1 <<= 16;
    t1 = u32(i32(t1) >> 16);
    multu(t1, t4);
    t2 = u32(i32(t0) >> 16);
    t0 = lw(a0 + 0xC);
    t1 = lo;
    t1 = u32(i32(t1) >> 12);
    t1 &= 0xFFFF;
    multu(t2, t5);
    t2 = lo;
    t2 = u32(i32(t2) >> 12);
    t2 <<= 16;
    t1 |= t2;
    sw(t1, a0 + 0x8);
    t1 = t0 & 0xFFFF;
    t1 <<= 16;
    t1 = u32(i32(t1) >> 16);
    multu(t1, t3);
    t2 = u32(i32(t0) >> 16);
    t0 = lw(a0 + 0x10);
    t1 = lo;
    t1 = u32(i32(t1) >> 12);
    t1 &= 0xFFFF;
    multu(t2, t4);
    t2 = lo;
    t2 = u32(i32(t2) >> 12);
    t2 <<= 16;
    t1 |= t2;
    sw(t1, a0 + 0xC);
    t1 = t0 & 0xFFFF;
    t1 <<= 16;
    t1 = u32(i32(t1) >> 16);
    multu(t1, t5);
    t1 = lo;
    t1 = u32(i32(t1) >> 12);
    sw(t1, a0 + 0x10);
    return;
}

void LIBGTE_SetRotMatrix() noexcept {
loc_80050100:
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 0);
    ctc2(t1, 1);
    ctc2(t2, 2);
    ctc2(t3, 3);
    ctc2(t4, 4);
    return;
}

void LIBGTE_SetLightMatrix() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 8);
    ctc2(t1, 9);
    ctc2(t2, 10);
    ctc2(t3, 11);
    ctc2(t4, 12);
    return;
}

void LIBGTE_SetColorMatrix() noexcept {
    t0 = lw(a0);
    t1 = lw(a0 + 0x4);
    t2 = lw(a0 + 0x8);
    t3 = lw(a0 + 0xC);
    t4 = lw(a0 + 0x10);
    ctc2(t0, 16);
    ctc2(t1, 17);
    ctc2(t2, 18);
    ctc2(t3, 19);
    ctc2(t4, 20);
    return;
}

void LIBGTE_SetTransMatrix() noexcept {
loc_80050190:
    t0 = lw(a0 + 0x14);
    t1 = lw(a0 + 0x18);
    t2 = lw(a0 + 0x1C);
    ctc2(t0, 5);
    ctc2(t1, 6);
    ctc2(t2, 7);
    return;
}

void LIBGTE_SetVertex0() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    return;
}

void LIBGTE_SetVertex1() noexcept {
    lwc2(2, a0);
    lwc2(3, a0 + 0x4);
    return;
}

void LIBGTE_SetVertex2() noexcept {
    lwc2(4, a0);
    lwc2(5, a0 + 0x4);
    return;
}

void LIBGTE_SetVertexTri() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(2, a1);
    lwc2(3, a1 + 0x4);
    lwc2(4, a2);
    lwc2(5, a2 + 0x4);
    return;
}

void LIBGTE_SetRGBfifo() noexcept {
    lwc2(20, a0);
    lwc2(21, a1);
    lwc2(22, a2);
    return;
}

void LIBGTE_SetIR123() noexcept {
    mtc2(a0, 9);
    mtc2(a1, 10);
    mtc2(a2, 11);
    return;
}

void LIBGTE_SetIR0() noexcept {
    mtc2(a0, 8);
    return;
}

void LIBGTE_SetBackColor() noexcept {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    ctc2(a0, 13);
    ctc2(a1, 14);
    ctc2(a2, 15);
    return;
}

void LIBGTE_SetFarColor() noexcept {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    ctc2(a0, 21);
    ctc2(a1, 22);
    ctc2(a2, 23);
    return;
}

void LIBGTE_SetSZfifo3() noexcept {
    mtc2(a0, 17);
    mtc2(a1, 18);
    mtc2(a2, 19);
    return;
}

void LIBGTE_SetSZfifo4() noexcept {
    mtc2(a0, 16);
    mtc2(a1, 17);
    mtc2(a2, 18);
    mtc2(a3, 19);
    return;
}

void LIBGTE_SetSXSYfifo() noexcept {
    mtc2(a0, 12);
    mtc2(a1, 13);
    mtc2(a2, 14);
    return;
}

void LIBGTE_SetRii() noexcept {
    ctc2(a0, 0);
    ctc2(a1, 2);
    ctc2(a2, 4);
    return;
}

void LIBGTE_SetMAC123() noexcept {
    mtc2(a0, 25);
    mtc2(a1, 26);
    mtc2(a2, 27);
    return;
}

void LIBGTE_SetData32() noexcept {
    mtc2(a0, 30);
    return;
}

void LIBGTE_SetGeomOffset() noexcept {
loc_800502EC:
    a0 <<= 16;
    a1 <<= 16;
    ctc2(a0, 24);
    ctc2(a1, 25);
    return;
}

void LIBGTE_SetGeomScreen() noexcept {
loc_80050304:
    ctc2(a0, 26);
    return;
}

void LIBGTE_SetDQA() noexcept {
    ctc2(a0, 27);
    return;
}

void LIBGTE_SetDQB() noexcept {
    ctc2(a0, 28);
    return;
}

void LIBGTE_InitGeom() noexcept {
loc_80050334:
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x6868);                                // Store to: gLIBGTE_InitGeom_miniStack[0] (80076868)
    LIBGTE__patch_gte();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x6868);                               // Load from: gLIBGTE_InitGeom_miniStack[0] (80076868)
    v0 = mfc0(12);
    v1 = 0x40000000;                                    // Result = 40000000
    v0 |= v1;
    mtc0(v0, 12);
    t0 = 0x155;                                         // Result = 00000155
    ctc2(t0, 29);
    t0 = 0x100;                                         // Result = 00000100
    ctc2(t0, 30);
    t0 = 0x3E8;                                         // Result = 000003E8
    ctc2(t0, 26);
    t0 = -0x1062;                                       // Result = FFFFEF9E
    ctc2(t0, 27);
    t0 = 0x1400000;                                     // Result = 01400000
    ctc2(t0, 28);
    ctc2(zero, 24);
    ctc2(zero, 25);
    return;
}

void LIBGTE__patch_gte() noexcept {
// TODO: RUN NATIVELY
#if 1
    emu_call(0x800503B4);
#else
loc_800503B4:
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x7E48);                                // Store to: gLIBGTE__patch_gte_miniStack[0] (80077E48)
    LIBAPI_EnterCriticalSection();
    t2 = 0xB0;                                          // Result = 000000B0
    t1 = 0x56;                                          // Result = 00000056
    ptr_call(t2);
    k0 = 0x80050000;                                    // Result = 80050000
    k1 = 0x80050000;                                    // Result = 80050000
    v0 = lw(v0 + 0x18);
    k0 += 0x41C;                                        // Result = LIBGTE__patch_gte + 68 (8005041C) (8005041C)
    k1 += 0x454;                                        // Result = LIBAPI_FlushCache (80050454)
loc_800503E4:
    v1 = lw(k0);
    k0 += 4;
    v0 += 4;
    sw(v1, v0 - 0x4);
    if (k0 != k1) goto loc_800503E4;
    LIBAPI_FlushCache();
    LIBAPI_ExitCriticalSection();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x7E48);                               // Load from: gLIBGTE__patch_gte_miniStack[0] (80077E48)
    return;
    k0 = 0x100;                                         // Result = 00000100
    k0 = lw(k0 + 0x8);                                  // Load from: 00000108
    k0 = lw(k0);
    k0 = addi(k0, 0x8);
    sw(at, k0 + 0x4);
    sw(v0, k0 + 0x8);
    sw(v1, k0 + 0xC);
    sw(ra, k0 + 0x7C);
    v0 = mfc0(13);
#endif
}

void LIBGTE_RotTransPers() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    cop2(0x180001);
    swc2(14, a1);
    swc2(8, a2);
    v1 = cfc2(31);
    v0 = mfc2(19);
    sw(v1, a3);
    v0 = u32(i32(v0) >> 2);
    return;
}

void LIBGTE_RotTransPers3() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(2, a1);
    lwc2(3, a1 + 0x4);
    lwc2(4, a2);
    lwc2(5, a2 + 0x4);
    cop2(0x280030);
    t0 = lw(sp + 0x10);
    t1 = lw(sp + 0x14);
    t2 = lw(sp + 0x18);
    t3 = lw(sp + 0x1C);
    swc2(12, a3);
    swc2(13, t0);
    swc2(14, t1);
    swc2(8, t2);
    v1 = cfc2(31);
    v0 = mfc2(19);
    sw(v1, t3);
    v0 = u32(i32(v0) >> 2);
    return;
}

void LIBGTE_RotTrans() noexcept {
loc_800504E4:
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    cop2(0x480012);
    swc2(25, a1);
    swc2(26, a1 + 0x4);
    swc2(27, a1 + 0x8);
    v0 = cfc2(31);
    sw(v0, a2);
    return;
}

void LIBGTE_LocalLight() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    cop2(0x4A6412);
    swc2(9, a1);
    swc2(10, a1 + 0x4);
    swc2(11, a1 + 0x8);
    return;
}

void LIBGTE_DpqColor() noexcept {
    lwc2(6, a0);
    mtc2(a1, 8);
    cop2(0x780010);
    swc2(22, a2);
    return;
}

void LIBGTE_NormalColor() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    cop2(0xC8041E);
    swc2(22, a1);
    return;
}

void LIBGTE_NormalColor3() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(2, a1);
    lwc2(3, a1 + 0x4);
    lwc2(4, a2);
    lwc2(5, a2 + 0x4);
    cop2(0xD80420);
    t0 = lw(sp + 0x10);
    t1 = lw(sp + 0x14);
    swc2(20, a3);
    swc2(21, t0);
    swc2(22, t1);
    return;
}

void LIBGTE_NormalColorDpq() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(6, a1);
    mtc2(a2, 8);
    cop2(0xE80413);
    swc2(22, a3);
    return;
}

void LIBGTE_NormalColorDpq3() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(2, a1);
    lwc2(3, a1 + 0x4);
    lwc2(4, a2);
    lwc2(5, a2 + 0x4);
    lwc2(6, a3);
    lwc2(8, sp + 0x10);
    cop2(0xF80416);
    t0 = lw(sp + 0x14);
    t1 = lw(sp + 0x18);
    t2 = lw(sp + 0x1C);
    swc2(20, t0);
    swc2(21, t1);
    swc2(22, t2);
    return;
}

void LIBGTE_NormalColorCol() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(6, a1);
    cop2(0x108041B);
    swc2(22, a2);
    return;
}

void LIBGTE_NormalColorCol3() noexcept {
    lwc2(0, a0);
    lwc2(1, a0 + 0x4);
    lwc2(2, a1);
    lwc2(3, a1 + 0x4);
    lwc2(4, a2);
    lwc2(5, a2 + 0x4);
    lwc2(6, a3);
    cop2(0x118043F);
    t0 = lw(sp + 0x10);
    t1 = lw(sp + 0x14);
    t2 = lw(sp + 0x18);
    swc2(20, t0);
    swc2(21, t1);
    swc2(22, t2);
    return;
}

void LIBGTE_ColorDpq() noexcept {
    lwc2(9, a0);
    lwc2(10, a0 + 0x4);
    lwc2(11, a0 + 0x8);
    lwc2(6, a1);
    mtc2(a2, 8);
    cop2(0x1280414);
    swc2(22, a3);
    return;
}

void LIBGTE_ColorCol() noexcept {
    lwc2(9, a0);
    lwc2(10, a0 + 0x4);
    lwc2(11, a0 + 0x8);
    lwc2(6, a1);
    cop2(0x138041C);
    swc2(22, a2);
    return;
}

void LIBGTE_NormalClip() noexcept {
    mtc2(a0, 12);
    mtc2(a2, 14);
    mtc2(a1, 13);
    cop2(0x1400006);
    v0 = mfc2(24);
    return;
}

void LIBGTE_NormalClipS() noexcept {
    cop2(0x1400006);
    v0 = mfc2(24);
    return;
}

void LIBGTE_AverageSZ3() noexcept {
    cop2(0x158002D);
    v0 = mfc2(7);
    return;
}

void LIBGTE_AverageSZ4() noexcept {
    cop2(0x168002E);
    v0 = mfc2(7);
    return;
}

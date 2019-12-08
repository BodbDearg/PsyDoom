#include "p_setup.h"

#include "Doom/Base/i_file.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_bbox.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_firesky.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_switch.h"
#include "PsxVm/PsxVm.h"

void P_LoadVertexes() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80021B00;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x8FC;                                        // Result = STR_P_LoadVertexes_LumpTooBig_Err[0] (800108FC)
    I_Error();
loc_80021B00:
    a0 = s0;
    W_MapLumpLength();
    v0 >>= 3;
    a1 = v0 << 3;
    a1 -= v0;
    a1 <<= 2;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    sw(v0, gp + 0xA38);                                 // Store to: gNumVertexes (80078018)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s0;
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0xC04);                                 // Store to: gpVertexes (800781E4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    a2 = lw(gp + 0xA38);                                // Load from: gNumVertexes (80078018)
    a0 = lw(gp + 0xC04);                                // Load from: gpVertexes (800781E4)
    a1 = 0;                                             // Result = 00000000
    if (i32(a2) <= 0) goto loc_80021B8C;
    v1 = a0 + 0x18;
loc_80021B60:
    v0 = lw(s0);
    a1++;
    sw(v0, a0);
    a0 += 0x1C;
    v0 = lw(s0 + 0x4);
    s0 += 8;
    sw(0, v1);
    sw(v0, v1 - 0x14);
    v0 = (i32(a1) < i32(a2));
    v1 += 0x1C;
    if (v0 != 0) goto loc_80021B60;
loc_80021B8C:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void P_LoadSegs() noexcept {
loc_80021BA0:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80021BD4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x918;                                        // Result = STR_P_LoadSegs_LumpTooBig_Err[0] (80010918)
    I_Error();
loc_80021BD4:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0xAAAA0000;                                    // Result = AAAA0000
    v1 |= 0xAAAB;                                       // Result = AAAAAAAB
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = hi;
    v0 >>= 3;
    a1 = v0 << 2;
    a1 += v0;
    sw(v0, gp + 0xAC4);                                 // Store to: gNumSegs (800780A4)
    a1 <<= 3;
    Z_Malloc2();
    a0 = v0;
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0xC58);                                 // Store to: gpSegs (80078238)
    a2 = v0 << 2;
    a2 += v0;
    a2 <<= 3;
    D_memset();
    a0 = s0;
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    t1 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
    t2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80021DC4;
    t0 = s0 + 8;                                        // Result = gTmpWadLumpBuffer[2] (80098750)
    a1 = t1 + 0xC;
    t4 = lw(gp + 0xC04);                                // Load from: gpVertexes (800781E4)
    t3 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
loc_80021C6C:
    v1 = lh(s0);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v0 += t4;
    sw(v0, t1);
    v1 = lh(t0 - 0x6);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v0 += t4;
    sw(v0, a1 - 0x8);
    v0 = lh(t0 - 0x4);
    v0 <<= 16;
    sw(v0, a1);
    v0 = lh(t0 + 0x2);
    v0 <<= 16;
    sw(v0, a1 - 0x4);
    v1 = lh(t0 - 0x2);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = lw(gp + 0x8D0);                                // Load from: gpLines (80077EB0)
    v0 <<= 2;
    a2 = v0 + v1;
    sw(a2, a1 + 0x8);
    a3 = lh(t0);
    a0 = a3 << 2;
    a0 += a2;
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    sw(v0, a1 + 0x4);
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    v0 = lw(v0 + 0x14);
    sw(v0, a1 + 0xC);
    v0 = lw(a2 + 0x10);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0);
        v0 = a3 ^ 1;
        if (bJump) goto loc_80021D7C;
    }
    v0 <<= 2;
    v0 += a2;
    v1 = lw(v0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += t3;
    v0 = lw(v0 + 0x14);
    sw(v0, a1 + 0x10);
    goto loc_80021D80;
loc_80021D7C:
    sw(0, a1 + 0x10);
loc_80021D80:
    v1 = lw(a2);
    v0 = lw(t1);
    t2++;
    if (v1 != v0) goto loc_80021DA4;
    v0 = lw(a1);
    v0 >>= 19;
    sw(v0, a2 + 0x48);
loc_80021DA4:
    a1 += 0x28;
    t1 += 0x28;
    t0 += 0xC;
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    v0 = (i32(t2) < i32(v0));
    s0 += 0xC;
    if (v0 != 0) goto loc_80021C6C;
loc_80021DC4:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void P_LoadSubSectors() noexcept {
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80021E10;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x930;                                        // Result = STR_P_LoadSubSectors_LumpTooBig_Err[0] (80010930)
    I_Error();
loc_80021E10:
    a0 = s1;
    W_MapLumpLength();
    v0 >>= 2;
    a1 = v0 << 4;
    a2 = 2;                                             // Result = 00000002
    v1 = 0x800A0000;                                    // Result = 800A0000
    v1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    s0 = v1;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    sw(v0, gp + 0xC44);                                 // Store to: gNumSubsectors (80078224)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s1;
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0x960);                                 // Store to: gpSubsectors (80077F40)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    a1 = 0;                                             // Result = 00000000
    a2 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    a0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    a2 <<= 4;
    D_memset();
    a1 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    v0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    a0 = 0;                                             // Result = 00000000
    if (i32(a1) <= 0) goto loc_80021EAC;
    v1 = v0 + 0xA;
loc_80021E80:
    v0 = lhu(s0);
    a0++;
    sh(v0, v1 - 0x6);
    v0 = lhu(s0 + 0x2);
    s0 += 4;
    sh(0, v1 - 0x2);
    sh(0, v1);
    sh(v0, v1 - 0x4);
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80021E80;
loc_80021EAC:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_LoadSectors() noexcept {
loc_80021EC4:
    sp -= 0x40;
    sw(s0, sp + 0x20);
    s0 = a0;
    v0 = 0x53;                                          // Result = 00000053
    sb(v0, sp + 0x10);
    v0 = 0x4B;                                          // Result = 0000004B
    sb(v0, sp + 0x11);
    v0 = 0x59;                                          // Result = 00000059
    sw(ra, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sb(v0, sp + 0x12);
    sb(0, sp + 0x13);
    sb(0, sp + 0x14);
    sb(0, sp + 0x15);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    s5 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80021F30;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x950;                                        // Result = STR_P_LoadSectors_LumpTooBig_Err[0] (80010950)
    I_Error();
loc_80021F30:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x24920000;                                    // Result = 24920000
    v1 |= 0x4925;                                       // Result = 24924925
    v0 >>= 2;
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    s3 = v0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a3 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = hi;
    a1 = v0 << 1;
    a1 += v0;
    a1 <<= 3;
    a1 -= v0;
    sw(v0, gp + 0x974);                                 // Store to: gNumSectors (80077F54)
    a1 <<= 2;
    Z_Malloc2();
    a0 = v0;
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0xAC8);                                 // Store to: gpSectors (800780A8)
    a2 = v0 << 1;
    a2 += v0;
    a2 <<= 3;
    a2 -= v0;
    a2 <<= 2;
    D_memset();
    a0 = s0;
    a1 = s3;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    s4 = lw(gp + 0xAC8);                                // Load from: gpSectors (800780A8)
    if (i32(v0) <= 0) goto loc_800220A8;
    s1 = s3 + 0x12;                                     // Result = gTmpWadLumpBuffer[4] (8009875A)
    s0 = s4 + 0xC;
loc_80021FD4:
    v0 = lh(s3);
    v0 <<= 16;
    sw(v0, s4);
    v0 = lh(s1 - 0x10);
    v0 <<= 16;
    sw(v0, s0 - 0x8);
    v0 = lhu(s1 + 0x2);
    v0 >>= 8;
    sh(v0, s0 + 0x4);
    v0 = lbu(s1 + 0x2);
    sh(v0, s0 + 0x6);
    v0 = lh(s1 + 0x4);
    sw(v0, s0 + 0x8);
    v0 = lh(s1 + 0x6);
    sw(0, s0 + 0x40);
    sw(v0, s0 + 0xC);
    v0 = lh(s1 + 0x8);
    a0 = s3 + 4;
    sw(v0, s0 + 0x18);
    R_FlatNumForName();
    s2 = s3 + 0xC;
    a0 = s2;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7B04;                                       // Result = STR_LumpName_F_SKY[0] (80077B04)
    a2 = 5;                                             // Result = 00000005
    sw(v0, s0 - 0x4);
    D_strncasecmp();
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80022078;
    }
    sw(v0, s0);
    v0 = lbu(s1 - 0x1);
    sb(v0, sp + 0x13);
    v0 = lbu(s1);
    sb(v0, sp + 0x14);
    goto loc_80022084;
loc_80022078:
    a0 = s2;
    R_FlatNumForName();
    sw(v0, s0);
loc_80022084:
    s5++;
    s0 += 0x5C;
    s4 += 0x5C;
    s1 += 0x1C;
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    v0 = (i32(s5) < i32(v0));
    s3 += 0x1C;
    if (v0 != 0) goto loc_80021FD4;
loc_800220A8:
    v0 = lbu(sp + 0x13);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7FB0);                                 // Store to: gpSkyTexture (80078050)
    if (v0 == 0) goto loc_800220DC;
    a0 = sp + 0x10;
    R_TextureNumForName();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v0 <<= 5;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FB0);                                // Store to: gpSkyTexture (80078050)
loc_800220DC:
    ra = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x40;
    return;
}

void P_LoadNodes() noexcept {
loc_80022104:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80022138;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x96C;                                        // Result = STR_P_LoadNodes_LumpTooBig_Err[0] (8001096C)
    I_Error();
loc_80022138:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x24920000;                                    // Result = 24920000
    v1 |= 0x4925;                                       // Result = 24924925
    v0 >>= 2;
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    a3 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = hi;
    a1 = v0 << 3;
    a1 -= v0;
    sw(v0, gp + 0xBD8);                                 // Store to: gNumBspNodes (800781B8)
    a1 <<= 3;
    Z_Malloc2();
    a0 = s0;
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0x8C4);                                 // Store to: gpBspNodes (80077EA4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    v0 = lw(gp + 0xBD8);                                // Load from: gNumBspNodes (800781B8)
    t3 = lw(gp + 0x8C4);                                // Load from: gpBspNodes (80077EA4)
    t6 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80022264;
    t5 = s0 + 6;                                        // Result = gTmpWadLumpBuffer[1] (8009874E)
    t4 = t3 + 0xC;
loc_800221AC:
    t2 = 0;                                             // Result = 00000000
    v0 = lh(s0);
    t1 = s0;
    v0 <<= 16;
    sw(v0, t3);
    v0 = lh(t5 - 0x4);
    t0 = t3;
    v0 <<= 16;
    sw(v0, t4 - 0x8);
    v0 = lh(t5 - 0x2);
    a3 = s0;
    v0 <<= 16;
    sw(v0, t4 - 0x4);
    v0 = lh(t5);
    a2 = t3;
    v0 <<= 16;
    sw(v0, t4);
loc_800221F0:
    a1 = 0;                                             // Result = 00000000
    a0 = t0 + 0x10;
    v0 = lhu(a3 + 0x18);
    v1 = t1 + 8;
    sw(v0, a2 + 0x30);
loc_80022204:
    v0 = lh(v1);
    v1 += 2;
    a1++;
    v0 <<= 16;
    sw(v0, a0);
    v0 = (i32(a1) < 4);
    a0 += 4;
    if (v0 != 0) goto loc_80022204;
    t1 += 8;                                            // Result = gTmpWadLumpBuffer[2] (80098750)
    t0 += 0x10;
    a3 += 2;                                            // Result = gTmpWadLumpBuffer[0] (8009874A)
    t2++;                                               // Result = 00000001
    v0 = (i32(t2) < 2);                                 // Result = 00000001
    a2 += 4;
    if (v0 != 0) goto loc_800221F0;
    t6++;                                               // Result = 00000001
    t4 += 0x38;
    t3 += 0x38;
    t5 += 0x1C;                                         // Result = gTmpWadLumpBuffer[8] (8009876A)
    v0 = lw(gp + 0xBD8);                                // Load from: gNumBspNodes (800781B8)
    v0 = (i32(t6) < i32(v0));
    s0 += 0x1C;                                         // Result = gTmpWadLumpBuffer[7] (80098764)
    if (v0 != 0) goto loc_800221AC;
loc_80022264:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void P_LoadThings() noexcept {
    sp -= 0x30;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    s2 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_800222B8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x984;                                        // Result = STR_P_LoadThings_LumpTooBig_Err[0] (80010984)
    I_Error();
loc_800222B8:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0xCCCC0000;                                    // Result = CCCC0000
    v1 |= 0xCCCD;                                       // Result = CCCCCCCD
    multu(v0, v1);
    a0 = s0;
    a1 = 0x800A0000;                                    // Result = 800A0000
    a1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    s1 = a1;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    v0 = hi;
    s3 = v0 >> 3;
    W_ReadMapLump();
    s0 = s1 + 6;                                        // Result = gTmpWadLumpBuffer[1] (8009874E)
    if (s3 == 0) goto loc_8002235C;
loc_800222F4:
    v0 = lhu(s1);
    sh(v0, s1);
    v0 = lhu(s0 - 0x4);
    v1 = lhu(s0 - 0x2);
    a1 = lhu(s0);
    a2 = lhu(s0 + 0x2);
    a0 = s1;
    sh(v0, s0 - 0x4);
    sh(v1, s0 - 0x2);
    sh(a1, s0);
    sh(a2, s0 + 0x2);
    P_SpawnMapThing();
    a1 = lh(s0);
    v0 = (i32(a1) < 0x1000);
    s2++;
    if (v0 != 0) goto loc_8002234C;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x9A0;                                        // Result = STR_P_LoadThings_BadDoomEdNum_Err[0] (800109A0)
    I_Error();
loc_8002234C:
    s0 += 0xA;
    v0 = (i32(s2) < i32(s3));
    s1 += 0xA;
    if (v0 != 0) goto loc_800222F4;
loc_8002235C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void P_LoadLineDefs() noexcept {
loc_8002237C:
    sp -= 0x38;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    s6 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_800223C8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x9C4;                                        // Result = STR_P_LoadLineDefs_LumpTooBig_Err[0] (800109C4)
    I_Error();
loc_800223C8:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x92490000;                                    // Result = 92490000
    v1 |= 0x2493;                                       // Result = 92492493
    v0 >>= 1;
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    s5 = v0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a3 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = hi;
    v0 >>= 2;
    a1 = v0 << 2;
    a1 += v0;
    a1 <<= 2;
    a1 -= v0;
    sw(v0, gp + 0xBE8);                                 // Store to: gNumLines (800781C8)
    a1 <<= 2;
    Z_Malloc2();
    a0 = v0;
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0x8D0);                                 // Store to: gpLines (80077EB0)
    a2 = v0 << 2;
    a2 += v0;
    a2 <<= 2;
    a2 -= v0;
    a2 <<= 2;
    D_memset();
    a0 = s0;
    a1 = s5;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    s4 = lw(gp + 0x8D0);                                // Load from: gpLines (80077EB0)
    s0 = s4 + 0x3C;
    if (i32(v0) <= 0) goto loc_80022624;
    s3 = s5 + 0xC;                                      // Result = gTmpWadLumpBuffer[3] (80098754)
loc_8002246C:
    v0 = lh(s3 - 0x8);
    sw(v0, s0 - 0x2C);
    v0 = lh(s3 - 0x6);
    sw(v0, s0 - 0x28);
    v0 = lh(s3 - 0x4);
    sw(v0, s0 - 0x24);
    v0 = lh(s5);
    a1 = lw(gp + 0xC04);                                // Load from: gpVertexes (800781E4)
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v1 += a1;
    sw(v1, s4);
    a0 = lh(s3 - 0xA);
    s2 = v1;
    v0 = a0 << 3;
    v0 -= a0;
    v0 <<= 2;
    v0 += a1;
    s1 = v0;
    sw(v0, s0 - 0x38);
    v0 = lw(s1);
    v1 = lw(s2);
    v0 -= v1;
    sw(v0, s0 - 0x34);
    v1 = lw(s1 + 0x4);
    v0 = lw(s2 + 0x4);
    a1 = lw(s0 - 0x34);
    a0 = v1 - v0;
    sw(a0, s0 - 0x30);
    if (a1 != 0) goto loc_80022500;
    v0 = 1;                                             // Result = 00000001
    goto loc_80022524;
loc_80022500:
    if (a0 != 0) goto loc_80022510;
    sw(0, s0 - 0x8);
    goto loc_80022528;
loc_80022510:
    FixedDiv();
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80022524;
    }
    v0 = 3;                                             // Result = 00000003
loc_80022524:
    sw(v0, s0 - 0x8);
loc_80022528:
    a0 = lw(s2);
    v1 = lw(s1);
    v0 = (i32(a0) < i32(v1));
    if (v0 == 0) goto loc_80022550;
    sw(a0, s0 - 0x10);
    v0 = lw(s1);
    sw(v0, s0 - 0xC);
    goto loc_80022560;
loc_80022550:
    sw(v1, s0 - 0x10);
    v0 = lw(s2);
    sw(v0, s0 - 0xC);
loc_80022560:
    a0 = lw(s2 + 0x4);
    v1 = lw(s1 + 0x4);
    v0 = (i32(a0) < i32(v1));
    if (v0 == 0) goto loc_80022588;
    sw(a0, s0 - 0x14);
    v0 = lw(s1 + 0x4);
    sw(v0, s0 - 0x18);
    goto loc_80022598;
loc_80022588:
    sw(v1, s0 - 0x14);
    v0 = lw(s2 + 0x4);
    sw(v0, s0 - 0x18);
loc_80022598:
    v1 = lh(s3 - 0x2);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v1, s0 - 0x20);
    if (v1 == v0) goto loc_800225C8;
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    sw(v0, s0 - 0x4);
    goto loc_800225CC;
loc_800225C8:
    sw(0, s0 - 0x4);
loc_800225CC:
    v1 = lh(s3);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v1, s0 - 0x1C);
    if (v1 == v0) goto loc_800225FC;
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    sw(v0, s0);
    goto loc_80022600;
loc_800225FC:
    sw(0, s0);
loc_80022600:
    s6++;
    s3 += 0xE;
    s5 += 0xE;
    s0 += 0x4C;
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    v0 = (i32(s6) < i32(v0));
    s4 += 0x4C;
    if (v0 != 0) goto loc_8002246C;
loc_80022624:
    ra = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

void P_LoadSideDefs() noexcept {
loc_80022650:
    sp -= 0x30;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    s4 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80022694;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x9E0;                                        // Result = STR_P_LoadSideDefs_LumpTooBig_Err[0] (800109E0)
    I_Error();
loc_80022694:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x88880000;                                    // Result = 88880000
    v1 |= 0x8889;                                       // Result = 88888889
    multu(v0, v1);
    a2 = 2;                                             // Result = 00000002
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    s3 = v0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a3 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = hi;
    v0 >>= 4;
    a1 = v0 << 1;
    a1 += v0;
    sw(v0, gp + 0xBD4);                                 // Store to: gNumSides (800781B4)
    a1 <<= 3;
    Z_Malloc2();
    a0 = v0;
    v0 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0x8C0);                                 // Store to: gpSides (80077EA0)
    a2 = v0 << 1;
    a2 += v0;
    a2 <<= 3;
    D_memset();
    a0 = s0;
    a1 = s3;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    v0 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    s2 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
    s0 = s2 + 0xC;
    if (i32(v0) <= 0) goto loc_800227A8;
    s1 = s3 + 0xC;                                      // Result = gTmpWadLumpBuffer[3] (80098754)
loc_80022724:
    v0 = lh(s3);
    a0 = s3 + 4;
    v0 <<= 16;
    sw(v0, s2);
    v0 = lh(s1 - 0xA);
    s4++;
    v0 <<= 16;
    sw(v0, s0 - 0x8);
    v1 = lh(s1 + 0x10);
    s2 += 0x18;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 -= v1;
    v1 = lw(gp + 0xAC8);                                // Load from: gpSectors (800780A8)
    v0 <<= 2;
    v0 += v1;
    sw(v0, s0 + 0x8);
    R_TextureNumForName();
    a0 = s3 + 0x14;
    sw(v0, s0 - 0x4);
    R_TextureNumForName();
    a0 = s1;
    sw(v0, s0 + 0x4);
    R_TextureNumForName();
    s1 += 0x1E;
    s3 += 0x1E;
    sw(v0, s0);
    v0 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    v0 = (i32(s4) < i32(v0));
    s0 += 0x18;
    if (v0 != 0) goto loc_80022724;
loc_800227A8:
    ra = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void P_LoadBlockMap() noexcept {
loc_800227CC:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    sw(ra, sp + 0x1C);
    s0 = a0;
    W_MapLumpLength();
    a1 = v0;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s0;
    a1 = v0;
    sw(a1, gp + 0xAE4);                                 // Store to: gpBlockmapLump (800780C4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    a0 = s0;
    W_MapLumpLength();
    v1 = v0 >> 31;
    v1 += v0;
    s0 = u32(i32(v1) >> 1);
    v1 = lw(gp + 0xAE4);                                // Load from: gpBlockmapLump (800780C4)
    v0 = v1 + 8;
    sw(v0, gp + 0xB60);                                 // Store to: gpBlockmap (80078140)
    a0 = 0;                                             // Result = 00000000
    if (i32(s0) <= 0) goto loc_80022850;
loc_80022838:
    v0 = lhu(v1);
    a0++;
    sh(v0, v1);
    v0 = (i32(a0) < i32(s0));
    v1 += 2;
    if (v0 != 0) goto loc_80022838;
loc_80022850:
    v1 = lw(gp + 0xAE4);                                // Load from: gpBlockmapLump (800780C4)
    t0 = lh(v1 + 0x6);
    a1 = lh(v1 + 0x4);
    v0 = t0 << 2;
    mult(v0, a1);
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = lh(v1);
    v1 = lh(v1 + 0x2);
    a3 = 0;                                             // Result = 00000000
    sw(a1, gp + 0xCA4);                                 // Store to: gBlockmapWidth (80078284)
    sw(t0, gp + 0x8D8);                                 // Store to: gBlockmapHeight (80077EB8)
    v0 <<= 16;
    v1 <<= 16;
    sw(v0, gp + 0xBAC);                                 // Store to: gBlockmapOriginX (8007818C)
    sw(v1, gp + 0xBB4);                                 // Store to: gBlockmapOriginY (80078194)
    s0 = lo;
    a1 = s0;
    Z_Malloc2();
    a0 = v0;
    a1 = 0;                                             // Result = 00000000
    sw(a0, gp + 0x8FC);                                 // Store to: gppBlockLinks (80077EDC)
    a2 = s0;
    D_memset();
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void P_LoadMapLump() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    W_MapLumpLength();
    a1 = v0;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s0;
    a1 = v0;
    sw(a1, gp + 0xB04);                                 // Store to: gpRejectMatrix (800780E4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_LoadLeafs() noexcept {
loc_80022920:
    sp -= 0x48;
    sw(s2, sp + 0x28);
    s2 = a0;
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    fp = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80022974;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x9FC;                                        // Result = STR_P_LoadLeafs_LumpTooBig_Err[0] (800109FC)
    I_Error();
loc_80022974:
    a0 = s2;
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    s1 = 0;                                             // Result = 00000000
    s4 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
loc_80022994:
    a0 = s2;
    W_MapLumpLength();
    v0 += s0;
    v0 = (s4 < v0);
    if (v0 == 0) goto loc_800229C8;
    v0 = lh(s4);
    fp++;
    s1 += v0;
    v0 <<= 2;
    v0 += 2;
    s4 += v0;
    goto loc_80022994;
loc_800229C8:
    v0 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    s4 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    if (fp == v0) goto loc_800229E8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xA14;                                        // Result = STR_P_LoadLeafs_Inconsistency_Err[0] (80010A14)
    I_Error();
loc_800229E8:
    a1 = s1 << 3;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    s6 = v0;
    v0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    sw(s6, gp + 0xB2C);                                 // Store to: gpLeafEdges (8007810C)
    sw(0, gp + 0x984);                                  // Store to: gTotalNumLeafEdges (80077F64)
    s7 = 0;                                             // Result = 00000000
    if (i32(fp) <= 0) goto loc_80022B24;
    s2 = v0 + 8;
loc_80022A1C:
    v0 = lhu(s4);
    sh(v0, s2);
    v0 = lhu(gp + 0x984);                               // Load from: gTotalNumLeafEdges (80077F64)
    v1 = lh(s2);
    s5 = 0;                                             // Result = 00000000
    sh(v0, s2 + 0x2);
    if (i32(v1) <= 0) goto loc_80022AF8;
    s3 = s6 + 4;
    s1 = s4;
loc_80022A44:
    s0 = lh(s1 + 0x2);
    v0 = lw(gp + 0xA38);                                // Load from: gNumVertexes (80078018)
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 3;
        if (bJump) goto loc_80022A70;
    }
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xA40;                                        // Result = STR_P_LoadLeafs_BadVertex_Err[0] (80010A40)
    I_Error();
    v0 = s0 << 3;
loc_80022A70:
    v0 -= s0;
    v1 = lw(gp + 0xC04);                                // Load from: gpVertexes (800781E4)
    v0 <<= 2;
    v0 += v1;
    sw(v0, s6);
    s0 = lh(s1 + 0x4);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 != v0) goto loc_80022A9C;
    sw(0, s3);
    goto loc_80022AD8;
loc_80022A9C:
    v0 = lw(gp + 0xAC4);                                // Load from: gNumSegs (800780A4)
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80022AC4;
    }
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xA64;                                        // Result = STR_P_LoadLeafs_BadSeg_Err[0] (80010A64)
    I_Error();
    v0 = s0 << 2;
loc_80022AC4:
    v0 += s0;
    v1 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
    v0 <<= 3;
    v0 += v1;
    sw(v0, s3);
loc_80022AD8:
    s1 += 4;
    s5++;
    s3 += 8;
    v0 = lh(s2);
    v0 = (i32(s5) < i32(v0));
    s6 += 8;
    if (v0 != 0) goto loc_80022A44;
loc_80022AF8:
    s7++;
    v0 = lh(s2);
    v1 = lw(gp + 0x984);                                // Load from: gTotalNumLeafEdges (80077F64)
    v0 <<= 2;
    v0 += 2;
    s4 += v0;
    v1 += s5;
    v0 = (i32(s7) < i32(fp));
    sw(v1, gp + 0x984);                                 // Store to: gTotalNumLeafEdges (80077F64)
    s2 += 0x10;
    if (v0 != 0) goto loc_80022A1C;
loc_80022B24:
    ra = lw(sp + 0x44);
    fp = lw(sp + 0x40);
    s7 = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x48;
    return;
}

void P_GroupLines() noexcept {
loc_80022B58:
    a1 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    a0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    sp -= 0x60;
    sw(s4, sp + 0x50);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x5C);
    sw(s6, sp + 0x58);
    sw(s5, sp + 0x54);
    sw(s3, sp + 0x4C);
    sw(s2, sp + 0x48);
    sw(s1, sp + 0x44);
    sw(s0, sp + 0x40);
    if (i32(a1) <= 0) goto loc_80022BC8;
    a2 = lw(gp + 0xC58);                                // Load from: gpSegs (80078238)
loc_80022B90:
    v1 = lh(a0 + 0x6);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 3;
    v0 += a2;
    v0 = lw(v0 + 0x10);
    v0 = lw(v0 + 0x14);
    s4++;
    sw(v0, a0);
    v0 = (i32(s4) < i32(a1));
    a0 += 0x10;
    if (v0 != 0) goto loc_80022B90;
loc_80022BC8:
    a1 = 0;                                             // Result = 00000000
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    s1 = lw(gp + 0x8D0);                                // Load from: gpLines (80077EB0)
    s4 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80022C3C;
    a2 = v0;
    a0 = s1 + 0x38;
loc_80022BE4:
    v1 = lw(a0);
    v0 = lw(v1 + 0x54);
    v0++;
    sw(v0, v1 + 0x54);
    v1 = lw(a0 + 0x4);
    a1++;
    if (v1 == 0) goto loc_80022C2C;
    v0 = lw(a0);
    if (v1 == v0) goto loc_80022C2C;
    v0 = lw(v1 + 0x54);
    a1++;
    v0++;
    sw(v0, v1 + 0x54);
loc_80022C2C:
    s4++;
    v0 = (i32(s4) < i32(a2));
    a0 += 0x4C;
    if (v0 != 0) goto loc_80022BE4;
loc_80022C3C:
    a1 <<= 2;
    a2 = 2;                                             // Result = 00000002
    s4 = 0;                                             // Result = 00000000
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    s5 = v0;
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    s6 = lw(gp + 0xAC8);                                // Load from: gpSectors (800780A8)
    s3 = s6 + 0x30;
    if (i32(v0) <= 0) goto loc_80022E3C;
loc_80022C6C:
    a0 = sp + 0x10;
    M_ClearBox();
    s1 = lw(gp + 0x8D0);                                // Load from: gpLines (80077EB0)
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    s2 = 0;                                             // Result = 00000000
    sw(s5, s3 + 0x28);
    if (i32(v0) <= 0) goto loc_80022CFC;
    s0 = s1 + 4;
loc_80022C8C:
    v0 = lw(s0 + 0x34);
    if (v0 == s6) goto loc_80022CAC;
    v0 = lw(s0 + 0x38);
    if (v0 != s6) goto loc_80022CE0;
loc_80022CAC:
    sw(s1, s5);
    v0 = lw(s1);
    a1 = lw(v0);
    a2 = lw(v0 + 0x4);
    a0 = sp + 0x10;
    M_AddToBox();
    v0 = lw(s0);
    s5 += 4;
    a1 = lw(v0);
    a2 = lw(v0 + 0x4);
    a0 = sp + 0x10;
    M_AddToBox();
loc_80022CE0:
    s2++;
    s0 += 0x4C;
    v0 = lw(gp + 0xBE8);                                // Load from: gNumLines (800781C8)
    v0 = (i32(s2) < i32(v0));
    s1 += 0x4C;
    if (v0 != 0) goto loc_80022C8C;
loc_80022CFC:
    v0 = lw(s3 + 0x28);
    v1 = lw(s3 + 0x24);
    v0 = s5 - v0;
    v0 = u32(i32(v0) >> 2);
    if (v0 == v1) goto loc_80022D24;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xA84;                                        // Result = STR_P_GroupLines_Miscounted_Err[0] (80010A84)
    I_Error();
loc_80022D24:
    v0 = lw(sp + 0x1C);
    v1 = lw(sp + 0x18);
    v0 += v1;
    v1 = v0 >> 31;
    v0 += v1;
    v0 = u32(i32(v0) >> 1);
    sw(v0, s3 + 0x8);
    a1 = lw(sp + 0x10);
    v0 = lw(sp + 0x14);
    a0 = lw(s3 + 0x8);
    a1 += v0;
    v0 = a1 >> 31;
    a1 += v0;
    a1 = u32(i32(a1) >> 1);
    sw(a1, s3 + 0xC);
    R_PointInSubsector();
    sw(v0, s3 + 0x14);
    v1 = lw(sp + 0x10);
    v0 = lw(gp + 0xBB4);                                // Load from: gBlockmapOriginY (80078194)
    a0 = lw(gp + 0x8D8);                                // Load from: gBlockmapHeight (80077EB8)
    v1 -= v0;
    v0 = 0x200000;                                      // Result = 00200000
    v1 += v0;
    v1 = u32(i32(v1) >> 23);
    v0 = (i32(v1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80022D98;
    }
    v0 = a0 - 1;
loc_80022D98:
    sw(v0, s3 - 0x8);
    v0 = lw(sp + 0x14);
    v1 = lw(gp + 0xBB4);                                // Load from: gBlockmapOriginY (80078194)
    v0 -= v1;
    v1 = 0xFFE00000;                                    // Result = FFE00000
    v0 += v1;
    v1 = u32(i32(v0) >> 23);
    if (i32(v1) >= 0) goto loc_80022DC4;
    v1 = 0;                                             // Result = 00000000
loc_80022DC4:
    sw(v1, s3 - 0x4);
    v1 = lw(sp + 0x1C);
    v0 = lw(gp + 0xBAC);                                // Load from: gBlockmapOriginX (8007818C)
    a0 = lw(gp + 0xCA4);                                // Load from: gBlockmapWidth (80078284)
    v1 -= v0;
    v0 = 0x200000;                                      // Result = 00200000
    v1 += v0;
    v1 = u32(i32(v1) >> 23);
    v0 = (i32(v1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80022DF4;
    }
    v0 = a0 - 1;
loc_80022DF4:
    sw(v0, s3 + 0x4);
    v0 = lw(sp + 0x18);
    v1 = lw(gp + 0xBAC);                                // Load from: gBlockmapOriginX (8007818C)
    v0 -= v1;
    v1 = 0xFFE00000;                                    // Result = FFE00000
    v0 += v1;
    v1 = u32(i32(v0) >> 23);
    s4++;
    if (i32(v1) >= 0) goto loc_80022E20;
    v1 = 0;                                             // Result = 00000000
loc_80022E20:
    sw(v1, s3);
    s3 += 0x5C;
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    v0 = (i32(s4) < i32(v0));
    s6 += 0x5C;
    if (v0 != 0) goto loc_80022C6C;
loc_80022E3C:
    ra = lw(sp + 0x5C);
    s6 = lw(sp + 0x58);
    s5 = lw(sp + 0x54);
    s4 = lw(sp + 0x50);
    s3 = lw(sp + 0x4C);
    s2 = lw(sp + 0x48);
    s1 = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x60;
    return;
}

void P_InitMapTextures() noexcept {
loc_80022E68:
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    v1 = lw(gp + 0xAC8);                                // Load from: gpSectors (800780A8)
    sp -= 0x30;
    sw(s0, sp + 0x20);
    s0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    if (i32(v0) <= 0) goto loc_80022F0C;
    s2 = -1;                                            // Result = FFFFFFFF
    s1 = v1 + 8;
loc_80022E94:
    v1 = lw(s1 + 0x4);
    {
        const bool bJump = (v1 == s2);
        v1 <<= 5;
        if (bJump) goto loc_80022ECC;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EDC);                               // Load from: gpFlatTextures (80078124)
    a0 = v1 + v0;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80022ECC;
    I_CacheTex();
loc_80022ECC:
    v0 = lw(s1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EDC);                               // Load from: gpFlatTextures (80078124)
    v0 <<= 5;
    a0 = v0 + v1;
    v0 = lhu(a0 + 0xA);
    s0++;
    if (v0 != 0) goto loc_80022EF8;
    I_CacheTex();
loc_80022EF8:
    v0 = lw(gp + 0x974);                                // Load from: gNumSectors (80077F54)
    v0 = (i32(s0) < i32(v0));
    s1 += 0x5C;
    if (v0 != 0) goto loc_80022E94;
loc_80022F0C:
    v0 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F7C);                              // Load from: gPaletteClutId_Main (800A9084)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7B34);                                 // Store to: gpUpdateFireSkyFunc (80077B34)
    v0 |= 2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0x7D34);                                // Store to: gPaletteClutId_CurMapSky (800782CC)
    if (a0 == 0) goto loc_80022FE8;
    a0 = lh(a0 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E3C);                               // Load from: gpLumpInfo (800781C4)
    v0 = a0 << 4;
    v0 += v1;
    v1 = lbu(v0 + 0xC);
    v0 = 0x39;                                          // Result = 00000039
    a1 = 8;                                             // Result = 00000008
    if (v1 != v0) goto loc_80022FD8;
    a2 = 1;                                             // Result = 00000001
    W_CacheLumpNum();
    s0 = 0;                                             // Result = 00000000
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F5E);                              // Load from: gPaletteClutId_Sky (800A90A2)
    v0 = 0x80020000;                                    // Result = 80020000
    v0 += 0x7CB0;                                       // Result = P_UpdateFireSky (80027CB0)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7B34);                                // Store to: gpUpdateFireSkyFunc (80077B34)
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0x7D34);                                // Store to: gPaletteClutId_CurMapSky (800782CC)
loc_80022FBC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    s0++;
    P_UpdateFireSky();
    v0 = (i32(s0) < 0x40);
    if (v0 != 0) goto loc_80022FBC;
loc_80022FD8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    I_CacheTex();
loc_80022FE8:
    a0 = 0x10;                                          // Result = 00000010
    P_CacheMapTexturesWithWidth();
    a0 = 0x40;                                          // Result = 00000040
    P_CacheMapTexturesWithWidth();
    s0 = 0;                                             // Result = 00000000
    P_InitSwitchList();
    a0 = 0x80;                                          // Result = 00000080
    P_CacheMapTexturesWithWidth();
    v1 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    v0 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
    a0 = -1;                                            // Result = FFFFFFFF
    if (i32(v1) <= 0) goto loc_80023068;
    a1 = v1;
    v1 = v0 + 0xC;
loc_80023020:
    v0 = lw(v1 - 0x4);
    if (v0 != a0) goto loc_80023034;
    sw(0, v1 - 0x4);
loc_80023034:
    v0 = lw(v1 + 0x4);
    if (v0 != a0) goto loc_80023048;
    sw(0, v1 + 0x4);
loc_80023048:
    v0 = lw(v1);
    s0++;
    if (v0 != a0) goto loc_8002305C;
    sw(0, v1);
loc_8002305C:
    v0 = (i32(s0) < i32(a1));
    v1 += 0x18;
    if (v0 != 0) goto loc_80023020;
loc_80023068:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    v0 = 5;                                             // Result = 00000005
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    v1 |= 0x1C;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    a1 = 0x20;                                          // Result = 00000020
    Z_FreeTags();
    P_InitPicAnims();
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void P_SetupLevel() noexcept {
loc_800230D4:
    sp -= 0x98;
    sw(s4, sp + 0x88);
    s4 = a0;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = 0x26;                                          // Result = 00000026
    sw(ra, sp + 0x90);
    sw(s5, sp + 0x8C);
    sw(s3, sp + 0x84);
    sw(s2, sp + 0x80);
    sw(s1, sp + 0x7C);
    sw(s0, sp + 0x78);
    Z_FreeTags();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FF4);                               // Load from: gbIsLevelBeingRestarted (80077FF4)
    if (v0 != 0) goto loc_80023140;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 &= 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    a1 = 8;                                             // Result = 00000008
    Z_FreeTags();
loc_80023140:
    s1 = 0;                                             // Result = 00000000
    I_ResetTexCache();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    Z_CheckHeap();
    M_ClearRandom();
    v1 = 0;                                             // Result = 00000000
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F20);                                 // Store to: gTotalKills (80077F20)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F2C);                                 // Store to: gTotalItems (80077F2C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7FEC);                                 // Store to: gTotalSecret (80077FEC)
loc_8002317C:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x774C;                                       // Result = gPlayer1[32] (800A88B4)
    at += v1;
    sw(0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7744;                                       // Result = gPlayer1[34] (800A88BC)
    at += v1;
    sw(0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7748;                                       // Result = gPlayer1[33] (800A88B8)
    at += v1;
    sw(0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x77B0;                                       // Result = gPlayer1[19] (800A8850)
    at += v1;
    sw(0, at);
    s1++;
    v0 = (i32(s1) < 2);
    v1 += 0x12C;
    if (v0 != 0) goto loc_8002317C;
    a1 = s4 - 1;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    sw(v0, v0);                                         // Store to: gThinkerCap[0] (80096550)
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x6554);                                // Store to: gThinkerCap[1] (80096554)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    v1 = v0 - 0x14;                                     // Result = gMObjHead[0] (800A8E90)
    sw(v1, v0);                                         // Store to: gMObjHead[5] (800A8EA4)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v1, at - 0x7160);                                // Store to: gMObjHead[4] (800A8EA0)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7EC8);                                 // Store to: gItemRespawnQueueHead (80078138)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7E80);                                 // Store to: gItemRespawnQueueTail (80078180)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7FF0);                                 // Store to: gNumMObjKilled (80078010)
    a0 = a1;
    if (i32(a1) >= 0) goto loc_80023220;
    a0 = s4 + 6;
loc_80023220:
    v0 = u32(i32(a0) >> 3);
    a0 = v0 << 1;
    a0 += v0;
    a0 <<= 3;
    v0 <<= 3;
    v0 = a1 - v0;
    a0 += v0;
    a0 += 8;
    W_OpenMapWad();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(s4, v1);
    a0 = sp + 0x10;
    v1 = 0x4D;                                          // Result = 0000004D
    sb(v1, sp + 0x10);
    v1 = 0x41;                                          // Result = 00000041
    sb(v1, sp + 0x11);
    v1 = 0x50;                                          // Result = 00000050
    sb(v1, sp + 0x12);
    v1 = u32(i32(s4) >> 31);
    s5 = v0;
    sb(0, sp + 0x15);
    a1 = hi;
    a1 = u32(i32(a1) >> 2);
    a1 -= v1;
    v1 = a1 + 0x30;
    sb(v1, sp + 0x13);
    v1 = a1 << 2;
    v1 += a1;
    v1 <<= 1;
    v1 = s4 - v1;
    v1 += 0x30;
    sb(v1, sp + 0x14);
    W_MapGetNumForName();
    s2 = v0;
    v0 = -1;                                            // Result = FFFFFFFF
    s0 = s2 + 4;
    if (s2 != v0) goto loc_800232C8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xAA0;                                        // Result = STR_P_SetupLevel_NotFound_Err[0] (80010AA0)
    a1 = sp + 0x10;
    I_Error();
loc_800232C8:
    a0 = s2 + 0xA;
    P_LoadBlockMap();
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_800232F8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x8FC;                                        // Result = STR_P_LoadVertexes_LumpTooBig_Err[0] (800108FC)
    I_Error();
loc_800232F8:
    a0 = s0;
    W_MapLumpLength();
    v0 >>= 3;
    a1 = v0 << 3;
    a1 -= v0;
    a1 <<= 2;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    sw(v0, gp + 0xA38);                                 // Store to: gNumVertexes (80078018)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s0;
    a1 = 0x800A0000;                                    // Result = 800A0000
    a1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0xC04);                                 // Store to: gpVertexes (800781E4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    a1 = 0x800A0000;                                    // Result = 800A0000
    a1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a3 = lw(gp + 0xA38);                                // Load from: gNumVertexes (80078018)
    a0 = lw(gp + 0xC04);                                // Load from: gpVertexes (800781E4)
    a2 = 0;                                             // Result = 00000000
    if (i32(a3) <= 0) goto loc_80023388;
    v1 = a0 + 0x18;
loc_8002335C:
    v0 = lw(a1);
    a2++;
    sw(v0, a0);
    a0 += 0x1C;
    v0 = lw(a1 + 0x4);
    a1 += 8;
    sw(0, v1);
    sw(v0, v1 - 0x14);
    v0 = (i32(a2) < i32(a3));
    v1 += 0x1C;
    if (v0 != 0) goto loc_8002335C;
loc_80023388:
    a0 = s2 + 8;
    P_LoadSectors();
    a0 = s2 + 3;
    P_LoadSideDefs();
    a0 = s2 + 2;
    P_LoadLineDefs();
    s1 = s2 + 6;
    a0 = s1;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_800233CC;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x930;                                        // Result = STR_P_LoadSubSectors_LumpTooBig_Err[0] (80010930)
    I_Error();
loc_800233CC:
    a0 = s1;
    W_MapLumpLength();
    v0 >>= 2;
    a1 = v0 << 4;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0xC44);                                 // Store to: gNumSubsectors (80078224)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s1;
    a1 = 0x800A0000;                                    // Result = 800A0000
    a1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    sw(v0, gp + 0x960);                                 // Store to: gpSubsectors (80077F40)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    a1 = 0;                                             // Result = 00000000
    a2 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    a0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    a2 <<= 4;
    D_memset();
    a1 = lw(gp + 0xC44);                                // Load from: gNumSubsectors (80078224)
    v0 = lw(gp + 0x960);                                // Load from: gpSubsectors (80077F40)
    a0 = 0;                                             // Result = 00000000
    if (i32(a1) <= 0) goto loc_80023468;
    v1 = v0 + 0xA;
loc_8002343C:
    v0 = lhu(s0);
    a0++;
    sh(v0, v1 - 0x6);
    v0 = lhu(s0 + 0x2);
    s0 += 4;
    sh(0, v1 - 0x2);
    sh(0, v1);
    sh(v0, v1 - 0x4);
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_8002343C;
loc_80023468:
    a0 = s2 + 7;
    P_LoadNodes();
    a0 = s2 + 5;
    P_LoadSegs();
    a0 = s2 + 0xB;
    P_LoadLeafs();
    s0 = s2 + 9;
    a0 = s0;
    W_MapLumpLength();
    a1 = v0;
    a2 = 2;                                             // Result = 00000002
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    a0 = s0;
    a1 = v0;
    sw(a1, gp + 0xB04);                                 // Store to: gpRejectMatrix (800780E4)
    a2 = 1;                                             // Result = 00000001
    W_ReadMapLump();
    s0 = s2 + 1;
    P_GroupLines();
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x7F94;                                       // Result = gDeathmatchStarts[0] (8009806C)
    sw(v0, gp + 0xA80);                                 // Store to: gpDeathmatchP (80078060)
    a0 = s0;
    W_MapLumpLength();
    v1 = 0x10000;                                       // Result = 00010000
    v1 = (i32(v1) < i32(v0));
    s2 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_800234F4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x984;                                        // Result = STR_P_LoadThings_LumpTooBig_Err[0] (80010984)
    I_Error();
loc_800234F4:
    a0 = s0;
    W_MapLumpLength();
    v1 = 0xCCCC0000;                                    // Result = CCCC0000
    v1 |= 0xCCCD;                                       // Result = CCCCCCCD
    multu(v0, v1);
    a0 = s0;
    a1 = 0x800A0000;                                    // Result = 800A0000
    a1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a2 = 1;                                             // Result = 00000001
    s1 = 0x800A0000;                                    // Result = 800A0000
    s1 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    v0 = hi;
    s3 = v0 >> 3;
    W_ReadMapLump();
    if (s3 == 0) goto loc_800235A0;
    s0 = s1 + 6;                                        // Result = gTmpWadLumpBuffer[1] (8009874E)
loc_80023538:
    v0 = lhu(s1);
    sh(v0, s1);
    v0 = lhu(s0 - 0x4);
    v1 = lhu(s0 - 0x2);
    a1 = lhu(s0);
    a2 = lhu(s0 + 0x2);
    a0 = s1;
    sh(v0, s0 - 0x4);
    sh(v1, s0 - 0x2);
    sh(a1, s0);
    sh(a2, s0 + 0x2);
    P_SpawnMapThing();
    a1 = lh(s0);
    v0 = (i32(a1) < 0x1000);
    s2++;
    if (v0 != 0) goto loc_80023590;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x9A0;                                        // Result = STR_P_LoadThings_BadDoomEdNum_Err[0] (800109A0)
    I_Error();
loc_80023590:
    s0 += 0xA;
    v0 = (i32(s2) < i32(s3));
    s1 += 0xA;
    if (v0 != 0) goto loc_80023538;
loc_800235A0:
    P_SpawnSpecials();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = s5;
    Z_Free2();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FF4);                               // Load from: gbIsLevelBeingRestarted (80077FF4)
    s0 = s4 - 1;
    if (v0 != 0) goto loc_8002360C;
    v0 = s0;
    if (i32(s0) >= 0) goto loc_800235D8;
    v0 = s4 + 6;
loc_800235D8:
    v0 = u32(i32(v0) >> 3);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 <<= 3;
    v0 = s0 - v0;
    s1 = v1 + v0;
    a0 = s1 + 0x18;
    P_LoadBlocks();
    P_InitMapTextures();
    a0 = s1 + 0x10;
    P_LoadBlocks();
loc_8002360C:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    Z_FreeMemory();
    s0 = v0;
    v0 = 0xBFFF;                                        // Result = 0000BFFF
    v0 = (i32(v0) < i32(s0));
    if (v0 != 0) goto loc_80023648;
    Z_DumpHeap();
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xABC;                                        // Result = STR_P_SetupLevel_OutOfMem_Err[0] (80010ABC)
    a1 = s0;
    I_Error();
loc_80023648:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    if (v0 == 0) goto loc_800236C8;
    s1 = 0;                                             // Result = 00000000
    I_NetHandshake();
    s3 = 0x800B0000;                                    // Result = 800B0000
    s3 -= 0x7184;                                       // Result = gPlayer1MapThing[0] (800A8E7C)
    s2 = 0;                                             // Result = 00000000
loc_80023670:
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    a0 = lh(s3);                                        // Load from: gPlayer1MapThing[0] (800A8E7C)
    a1 = lh(s3 + 0x2);                                  // Load from: gPlayer1MapThing[1] (800A8E7E)
    a0 <<= 16;
    a1 <<= 16;
    P_SpawnMObj();
    s0 = v0;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += s2;
    sw(s0, at);
    a0 = s1;
    G_DoReborn();
    a0 = s0;
    P_RemoveMObj();
    s1++;
    v0 = (i32(s1) < 2);
    s2 += 0x12C;
    if (v0 != 0) goto loc_80023670;
    goto loc_800236D8;
loc_800236C8:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x7184;                                       // Result = gPlayer1MapThing[0] (800A8E7C)
    P_SpawnPlayer();
loc_800236D8:
    ra = lw(sp + 0x90);
    s5 = lw(sp + 0x8C);
    s4 = lw(sp + 0x88);
    s3 = lw(sp + 0x84);
    s2 = lw(sp + 0x80);
    s1 = lw(sp + 0x7C);
    s0 = lw(sp + 0x78);
    sp += 0x98;
    return;
}

void P_LoadBlocks() noexcept {
loc_80023700:
    sp -= 0x58;
    sw(s7, sp + 0x4C);
    s7 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s6, sp + 0x48);
    sw(s5, sp + 0x44);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    sw(a0, sp + 0x28);
    v0 = s7;                                            // Result = 00000000
loc_80023738:
    v0 = (i32(v0) < 4);
    s7++;
    if (v0 != 0) goto loc_80023754;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xAE4;                                        // Result = STR_P_LoadBlocks_DataFailure_Err[0] (80010AE4)
    I_Error();
loc_80023754:
    a0 = lw(sp + 0x28);
    s5 = 0;                                             // Result = 00000000
    OpenFile();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 2;                                             // Result = 00000002
    SeekAndTellFile();
    s4 = v0;
    s3 = s4;
    a1 = s4 - 0x18;
    a2 = 1;                                             // Result = 00000001
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    s1 = v0 - 0x18;
    s2 = s1;
    v1 = lw(v0 - 0x18);
    a0 = lw(v0 - 0x14);
    a1 = lw(v0 - 0x10);
    a2 = lw(v0 - 0xC);
    sw(v1, sp + 0x10);
    sw(a0, sp + 0x14);
    sw(a1, sp + 0x18);
    sw(a2, sp + 0x1C);
    v1 = lw(v0 - 0x8);
    a0 = lw(v0 - 0x4);
    sw(v1, sp + 0x20);
    sw(a0, sp + 0x24);
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    fp = lw(v0 - 0x4);
    s6 = lw(v0 - 0x8);
    a2 = 0;                                             // Result = 00000000
    SeekAndTellFile();
    a0 = s0;
    a1 = s1;
    a2 = s4;
    ReadFile();
    a0 = s0;
    CloseFile();
loc_800237FC:
    v1 = lh(s2 + 0xA);
    v0 = 0x1D4A;                                        // Result = 00001D4A
    if (v1 != v0) goto loc_800238DC;
    v0 = lh(s2 + 0xC);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E14);                               // Load from: gNumLumps (800781EC)
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_800238DC;
    v1 = lhu(s2 + 0xE);
    v0 = (v1 < 2);
    if (v0 == 0) goto loc_800238DC;
    if (v1 != 0) goto loc_80023870;
    a0 = s2 + 0x18;
    getDecodedSize();
    v1 = lh(s2 + 0xC);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E3C);                               // Load from: gpLumpInfo (800781C4)
    v1 <<= 4;
    v1 += a0;
    v1 = lw(v1 + 0x4);
    if (v0 != v1) goto loc_800238DC;
loc_80023870:
    v0 = lw(s2);
    s3 -= v0;
    if (i32(s3) < 0) goto loc_800238DC;
    s2 += v0;
    if (s3 != 0) goto loc_800237FC;
loc_8002388C:
    if (s5 == 0) goto loc_800238E4;
    v0 = lw(sp + 0x10);
    v1 = lw(sp + 0x14);
    a0 = lw(sp + 0x18);
    a1 = lw(sp + 0x1C);
    sw(v0, s1);
    sw(v1, s1 + 0x4);
    sw(a0, s1 + 0x8);
    sw(a1, s1 + 0xC);
    v0 = lw(sp + 0x20);
    v1 = lw(sp + 0x24);
    sw(v0, s1 + 0x10);
    sw(v1, s1 + 0x14);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = s1 + 0x18;
    Z_Free2();
    v0 = s7;
    goto loc_80023738;
loc_800238DC:
    s5 = 1;                                             // Result = 00000001
    goto loc_8002388C;
loc_800238E4:
    sw(fp, s1 + 0x14);
loc_800238E8:
    v0 = lh(s1 + 0xC);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    a0 = v0 + a1;
    v0 = lw(a0);
    {
        const bool bJump = (v0 == 0);
        v0 = s1 + 0x18;
        if (bJump) goto loc_8002391C;
    }
    sw(0, s1 + 0x4);
    sh(0, s1 + 0x8);
    sh(0, s1 + 0xA);
    goto loc_80023948;
loc_8002391C:
    v1 = lh(s1 + 0xC);
    sw(a0, s1 + 0x4);
    v1 <<= 2;
    v1 += a1;
    sw(v0, v1);
    a0 = lh(s1 + 0xC);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D10);                               // Load from: gpbIsMainWadLump (800782F0)
    v1 = lbu(s1 + 0xE);
    v0 += a0;
    sb(v1, v0);
loc_80023948:
    v0 = lw(s1);
    s4 -= v0;
    v0 += s1;
    if (s4 == 0) goto loc_80023964;
    sw(v0, s1 + 0x10);
    goto loc_80023974;
loc_80023964:
    v0 = s6 - s1;
    if (s6 == 0) goto loc_80023970;
    sw(v0, s1);
loc_80023970:
    sw(s6, s1 + 0x10);
loc_80023974:
    v0 = lw(s1 + 0x10);
    if (v0 == 0) goto loc_80023988;
    sw(s1, v0 + 0x14);
loc_80023988:
    s1 = lw(s1 + 0x10);
    if (s4 != 0) goto loc_800238E8;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    Z_CheckHeap();
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}

void P_CacheSprite() noexcept {
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(s4, sp + 0x28);
    sw(ra, sp + 0x30);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s5);
    s3 = lw(s5 + 0x4);
    s4 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80023AA0;
loc_80023A0C:
    s2 = 0;                                             // Result = 00000000
    s1 = s3;
loc_80023A14:
    s0 = lw(s1 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FEC);                               // Load from: gFirstSpriteLumpNum (80078014)
    v0 = (i32(s0) < i32(v0));
    if (v0 != 0) goto loc_80023A48;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F38);                               // Load from: gLastSpriteLumpNum (80077F38)
    v0 = (i32(v0) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_80023A5C;
loc_80023A48:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0xB00;                                        // Result = STR_CacheSprite_BadLump_Err[0] (80010B00)
    a1 = s0;
    I_Error();
    a0 = s0;
loc_80023A5C:
    a1 = 8;                                             // Result = 00000008
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpNum();
    v0 = lw(s3);
    if (v0 == 0) goto loc_80023A88;
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 8);                                 // Result = 00000001
    s1 += 4;
    if (v0 != 0) goto loc_80023A14;
loc_80023A88:
    s4++;
    v0 = lw(s5);
    v0 = (i32(s4) < i32(v0));
    s3 += 0x2C;
    if (v0 != 0) goto loc_80023A0C;
loc_80023AA0:
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

void P_CacheMapTexturesWithWidth() noexcept {
loc_80023AC8:
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D1C);                               // Load from: gTexCacheFillBlockX (800782E4)
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v1 = v0 - 1;
    if (i32(s1) >= 0) goto loc_80023AF8;
    a0 = s1 + 0xF;
loc_80023AF8:
    v0 = u32(i32(a0) >> 4);
    v1 += v0;
    v0 = -v0;
    v1 &= v0;
    v0 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7D1C);                                // Store to: gTexCacheFillBlockX (800782E4)
    v1 = lw(gp + 0x8C0);                                // Load from: gpSides (80077EA0)
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80023C14;
    s3 = -1;                                            // Result = FFFFFFFF
    s0 = v1 + 0xC;
loc_80023B28:
    v1 = lw(s0 - 0x4);
    {
        const bool bJump = (v1 == s3);
        v1 <<= 5;
        if (bJump) goto loc_80023B70;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023B70;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023B70;
    I_CacheTex();
loc_80023B70:
    v1 = lw(s0 + 0x4);
    {
        const bool bJump = (v1 == s3);
        v1 <<= 5;
        if (bJump) goto loc_80023BB8;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023BB8;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023BB8;
    I_CacheTex();
loc_80023BB8:
    v1 = lw(s0);
    s2++;
    if (v1 == s3) goto loc_80023C00;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v1 <<= 5;
    a0 = v1 + v0;
    v0 = lh(a0 + 0x4);
    if (v0 != s1) goto loc_80023C00;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80023C00;
    I_CacheTex();
loc_80023C00:
    v0 = lw(gp + 0xBD4);                                // Load from: gNumSides (800781B4)
    v0 = (i32(s2) < i32(v0));
    s0 += 0x18;
    if (v0 != 0) goto loc_80023B28;
loc_80023C14:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

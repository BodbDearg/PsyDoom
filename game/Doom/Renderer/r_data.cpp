#include "r_data.h"

#include "PsxVm/PsxVm.h"

void R_InitData() noexcept {
loc_8002B9A8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    R_InitPalette();
    R_InitTextures();
    R_InitFlats();
    R_InitSprites();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void R_InitTextures() noexcept {
loc_8002B9E0:
    sp -= 0x20;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B6C;                                       // Result = STR_LumpName_T_START[0] (80077B6C)
    sw(ra, sp + 0x18);
    W_GetNumForName();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B74;                                       // Result = STR_LumpName_T_END[0] (80077B74)
    v0++;
    sw(v0, gp + 0xD00);                                 // Store to: gFirstTexLumpNum (800782E0)
    W_GetNumForName();
    a2 = 1;                                             // Result = 00000001
    v0--;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v1 = lw(gp + 0xD00);                                // Load from: gFirstTexLumpNum (800782E0)
    a3 = 0;                                             // Result = 00000000
    sw(v0, gp + 0xBBC);                                 // Store to: gLastTexLumpNum (8007819C)
    v0 -= v1;
    v0++;
    v1 = v0 << 5;
    a1 = v0 << 2;
    sw(v0, gp + 0xBF4);                                 // Store to: gNumTexLumps (800781D4)
    a1 += v1;
    Z_Malloc2();
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x115C;                                       // Result = STR_LumpName_TEXTURE1[0] (8001115C)
    a1 = 0x20;                                          // Result = 00000020
    v1 = lw(gp + 0xBF4);                                // Load from: gNumTexLumps (800781D4)
    sw(v0, gp + 0xB48);                                 // Store to: gpTextures (80078128)
    v1 <<= 5;
    v1 += v0;
    sw(v1, gp + 0x98C);                                 // Store to: gpTextureTranslation (80077F6C)
    a2 = 1;                                             // Result = 00000001
    W_CacheLumpName();
    a1 = v0;
    a2 = lw(gp + 0xD00);                                // Load from: gFirstTexLumpNum (800782E0)
    v1 = lw(gp + 0xBBC);                                // Load from: gLastTexLumpNum (8007819C)
    a0 = lw(gp + 0xB48);                                // Load from: gpTextures (80078128)
    v0 = (i32(v1) < i32(a2));
    t1 = v1;
    if (v0 != 0) goto loc_8002BAF4;
    t0 = a1 + 6;
    a0 += 0x10;
loc_8002BA94:
    v0 = lhu(t0 - 0x2);
    sh(v0, a0 - 0xC);
    v0 = lhu(t0);
    a3 = lh(a0 - 0xC);
    sh(0, a0 - 0x6);
    v1 = a3 + 0xF;
    sh(v0, a0 - 0xA);
    if (i32(v1) >= 0) goto loc_8002BABC;
    v1 = a3 + 0x1E;
loc_8002BABC:
    a3 = lh(a0 - 0xA);
    v0 = u32(i32(v1) >> 4);
    sh(v0, a0 - 0x4);
    v0 = a3 + 0xF;
    sh(a2, a0);
    if (i32(v0) >= 0) goto loc_8002BAD8;
    v0 = a3 + 0x1E;
loc_8002BAD8:
    a2++;
    v0 = u32(i32(v0) >> 4);
    sh(v0, a0 - 0x2);
    a0 += 0x20;
    v0 = (i32(t1) < i32(a2));
    t0 += 8;
    if (v0 == 0) goto loc_8002BA94;
loc_8002BAF4:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    Z_Free2();
    a0 = lw(gp + 0xBF4);                                // Load from: gNumTexLumps (800781D4)
    a2 = 0;                                             // Result = 00000000
    if (i32(a0) <= 0) goto loc_8002BB30;
    v1 = lw(gp + 0x98C);                                // Load from: gpTextureTranslation (80077F6C)
loc_8002BB18:
    sw(a2, v1);
    a2++;
    v0 = (i32(a2) < i32(a0));
    v1 += 4;
    if (v0 != 0) goto loc_8002BB18;
loc_8002BB30:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = 0x20;                                          // Result = 00000020
    Z_FreeTags();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void R_InitFlats() noexcept {
loc_8002BB50:
    sp -= 0x20;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B7C;                                       // Result = STR_LumpName_F_START[0] (80077B7C)
    sw(ra, sp + 0x18);
    W_GetNumForName();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B84;                                       // Result = STR_LumpName_F_END[0] (80077B84)
    v0++;
    sw(v0, gp + 0xCD8);                                 // Store to: gFirstFlatLumpNum (800782B8)
    W_GetNumForName();
    v0--;
    a2 = 1;                                             // Result = 00000001
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v1 = lw(gp + 0xCD8);                                // Load from: gFirstFlatLumpNum (800782B8)
    a3 = 0;                                             // Result = 00000000
    sw(v0, gp + 0xB90);                                 // Store to: gLastFlatLumpNum (80078170)
    v0 -= v1;
    v0++;
    a1 = v0 << 5;
    sw(v0, gp + 0xBE0);                                 // Store to: gNumFlatLumps (800781C0)
    v0 <<= 2;
    a1 += v0;
    Z_Malloc2();
    a0 = lw(gp + 0xCD8);                                // Load from: gFirstFlatLumpNum (800782B8)
    v1 = lw(gp + 0xBE0);                                // Load from: gNumFlatLumps (800781C0)
    a1 = lw(gp + 0xB90);                                // Load from: gLastFlatLumpNum (80078170)
    sw(v0, gp + 0xB44);                                 // Store to: gpFlatTextures (80078124)
    v1 <<= 5;
    v1 += v0;
    sw(v1, gp + 0x980);                                 // Store to: gpFlatTranslation (80077F60)
    v1 = v0;
    v0 = (i32(a1) < i32(a0));
    if (v0 != 0) goto loc_8002BC18;
    a3 = 0x40;                                          // Result = 00000040
    a2 = 4;                                             // Result = 00000004
    v1 += 0x10;
loc_8002BBF0:
    sh(a0, v1);
    a0++;
    sh(a3, v1 - 0xC);
    sh(a3, v1 - 0xA);
    sh(0, v1 - 0x6);
    sh(a2, v1 - 0x4);
    sh(a2, v1 - 0x2);
    v0 = (i32(a1) < i32(a0));
    v1 += 0x20;
    if (v0 == 0) goto loc_8002BBF0;
loc_8002BC18:
    a1 = lw(gp + 0xBE0);                                // Load from: gNumFlatLumps (800781C0)
    a0 = 0;                                             // Result = 00000000
    if (i32(a1) <= 0) goto loc_8002BC44;
    v1 = lw(gp + 0x980);                                // Load from: gpFlatTranslation (80077F60)
loc_8002BC2C:
    sw(a0, v1);
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 4;
    if (v0 != 0) goto loc_8002BC2C;
loc_8002BC44:
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void R_InitSprites() noexcept {
loc_8002BC54:
    sp -= 0x18;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B8C;                                       // Result = STR_LumpName_S_START[0] (80077B8C)
    sw(ra, sp + 0x10);
    W_GetNumForName();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B94;                                       // Result = STR_LumpName_S_END[0] (80077B94)
    v0++;
    sw(v0, gp + 0xA34);                                 // Store to: gFirstSpriteLumpNum (80078014)
    W_GetNumForName();
    a2 = 1;                                             // Result = 00000001
    v0--;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v1 = lw(gp + 0xA34);                                // Load from: gFirstSpriteLumpNum (80078014)
    a3 = 0;                                             // Result = 00000000
    sw(v0, gp + 0x958);                                 // Store to: gLastSpriteLumpNum (80077F38)
    v0 -= v1;
    v0++;
    sw(v0, gp + 0x97C);                                 // Store to: gNumSpriteLumps (80077F5C)
    a1 = v0 << 5;
    Z_Malloc2();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7B9C;                                       // Result = STR_LumpName_SPRITE1[0] (80077B9C)
    a1 = 0x20;                                          // Result = 00000020
    sw(v0, gp + 0x8E4);                                 // Store to: gpSprites (80077EC4)
    a2 = 1;                                             // Result = 00000001
    W_CacheLumpName();
    a1 = v0;
    t0 = lw(gp + 0xA34);                                // Load from: gFirstSpriteLumpNum (80078014)
    v1 = lw(gp + 0x958);                                // Load from: gLastSpriteLumpNum (80077F38)
    t1 = lw(gp + 0x8E4);                                // Load from: gpSprites (80077EC4)
    v0 = (i32(v1) < i32(t0));
    t2 = a1;
    if (v0 != 0) goto loc_8002BD74;
    t3 = v1;
    a3 = a1 + 6;
    a0 = t1 + 0x10;
loc_8002BCF4:
    v0 = lhu(t2);
    sh(v0, t1);
    v0 = lhu(a3 - 0x4);
    sh(v0, a0 - 0xE);
    v0 = lhu(a3 - 0x2);
    sh(v0, a0 - 0xC);
    v0 = lhu(a3);
    a2 = lh(a0 - 0xC);
    sh(0, a0 - 0x6);
    v1 = a2 + 0xF;
    sh(v0, a0 - 0xA);
    if (i32(v1) >= 0) goto loc_8002BD34;
    v1 = a2 + 0x1E;
loc_8002BD34:
    a2 = lh(a0 - 0xA);
    v0 = u32(i32(v1) >> 4);
    sh(v0, a0 - 0x4);
    v0 = a2 + 0xF;
    sh(t0, a0);
    if (i32(v0) >= 0) goto loc_8002BD50;
    v0 = a2 + 0x1E;
loc_8002BD50:
    t0++;
    v0 = u32(i32(v0) >> 4);
    sh(v0, a0 - 0x2);
    a0 += 0x20;
    t1 += 0x20;
    a3 += 8;
    v0 = (i32(t3) < i32(t0));
    t2 += 8;
    if (v0 == 0) goto loc_8002BCF4;
loc_8002BD74:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    Z_Free2();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = 0x20;                                          // Result = 00000020
    Z_FreeTags();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void R_TextureNumForName() noexcept {
loc_8002BDA4:
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_8002BDB8:
    v0 = lbu(a0);
    v1 = v0;
    if (v0 == 0) goto loc_8002BDF0;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_8002BDDC;
    v1 -= 0x20;
loc_8002BDDC:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_8002BDB8;
loc_8002BDF0:
    v0 = lw(gp + 0xD00);                                // Load from: gFirstTexLumpNum (800782E0)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E3C);                               // Load from: gpLumpInfo (800781C4)
    a3 = lw(sp);
    v0 <<= 4;
    v0 += v1;
    v1 = lw(gp + 0xBF4);                                // Load from: gNumTexLumps (800781D4)
    a2 = lw(sp + 0x4);
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_8002BE58;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_8002BE24:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_8002BE48;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_8002BE5C;
    }
loc_8002BE48:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_8002BE24;
loc_8002BE58:
    v0 = -1;                                            // Result = FFFFFFFF
loc_8002BE5C:
    sp += 0x10;
    return;
}

void R_FlatNumForName() noexcept {
loc_8002BE68:
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_8002BE7C:
    v0 = lbu(a0);
    v1 = v0;
    if (v0 == 0) goto loc_8002BEB4;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_8002BEA0;
    v1 -= 0x20;
loc_8002BEA0:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_8002BE7C;
loc_8002BEB4:
    v0 = lw(gp + 0xCD8);                                // Load from: gFirstFlatLumpNum (800782B8)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E3C);                               // Load from: gpLumpInfo (800781C4)
    a3 = lw(sp);
    v0 <<= 4;
    v0 += v1;
    v1 = lw(gp + 0xBE0);                                // Load from: gNumFlatLumps (800781C0)
    a2 = lw(sp + 0x4);
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_8002BF1C;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_8002BEE8:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_8002BF0C;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_8002BF20;
    }
loc_8002BF0C:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_8002BEE8;
loc_8002BF1C:
    v0 = 0;                                             // Result = 00000000
loc_8002BF20:
    sp += 0x10;
    return;
}

void R_InitPalette() noexcept {
loc_8002BF2C:
    sp -= 0x28;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7BA4;                                       // Result = STR_LumpName_LIGHTS[0] (80077BA4)
    a1 = 1;                                             // Result = 00000001
    a2 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    W_CacheLumpName();
    v1 = 0xFF;                                          // Result = 000000FF
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7F98);                                // Store to: gpLightsLump (80078068)
    sb(v1, v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F98);                               // Load from: gpLightsLump (80078068)
    sb(v1, v0 + 0x1);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F98);                               // Load from: gpLightsLump (80078068)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7BAC;                                       // Result = STR_LumpName_PLAYPAL[0] (80077BAC)
    sb(v1, v0 + 0x2);
    W_GetNumForName();
    s0 = v0;
    a0 = s0;
    a1 = 0x20;                                          // Result = 00000020
    a2 = 1;                                             // Result = 00000001
    W_CacheLumpNum();
    a0 = s0;
    s1 = v0;
    W_LumpLength();
    v0 >>= 9;
    v1 = 0x14;                                          // Result = 00000014
    s0 = 0;                                             // Result = 00000000
    if (v0 == v1) goto loc_8002BFCC;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1168;                                       // Result = STR_R_InitPalette_PalFoulup_Err[0] (80011168)
    I_Error();
loc_8002BFCC:
    s2 = 0x800B0000;                                    // Result = 800B0000
    s2 -= 0x6F7C;                                       // Result = gPaletteClutId_Main (800A9084)
    v0 = 0x100;                                         // Result = 00000100
    sh(v0, sp + 0x14);
    v0 = 1;                                             // Result = 00000001
    sh(v0, sp + 0x16);
loc_8002BFE4:
    v0 = s0;
    if (i32(s0) >= 0) goto loc_8002BFF0;
    v0 = s0 + 0xF;
loc_8002BFF0:
    a0 = sp + 0x10;
    a1 = s1;
    v0 = u32(i32(v0) >> 4);
    v1 = v0 << 8;
    v0 <<= 4;
    v0 = s0 - v0;
    v0 += 0xF0;
    sh(v1, sp + 0x10);
    sh(v0, sp + 0x12);
    LIBGPU_LoadImage();
    s1 += 0x200;
    a0 = lh(sp + 0x10);
    a1 = lh(sp + 0x12);
    s0++;
    LIBGPU_GetClut();
    sh(v0, s2);
    v0 = (i32(s0) < 0x14);
    s2 += 2;
    if (v0 != 0) goto loc_8002BFE4;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lhu(v0 - 0x6F7C);                              // Load from: gPaletteClutId_Main (800A9084)
    at = 0x80070000;                                    // Result = 80070000
    sh(v0, at + 0x7F7C);                                // Store to: g3dViewPaletteClutId (80077F7C)
    a1 = 0x20;                                          // Result = 00000020
    Z_FreeTags();
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

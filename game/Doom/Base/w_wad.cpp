#include "w_wad.h"

#include "Doom/d_main.h"
#include "i_file.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "z_zone.h"

void W_Init() noexcept {
loc_80031394:
    sp -= 0x38;
    sw(ra, sp + 0x30);
    InitOpenFileSlots();
    a0 = 7;                                             // Result = 00000007
    OpenFile();
    a0 = v0;
    a1 = sp + 0x10;
    sw(v0, gp + 0xC74);                                 // Store to: gMainWadFileIdx (80078254)
    a2 = 0xC;                                           // Result = 0000000C
    ReadFile();
    a0 = sp + 0x10;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7BE8;                                       // Result = STR_IWAD[0] (80077BE8)
    a2 = 4;                                             // Result = 00000004
    D_strncasecmp();
    if (v0 == 0) goto loc_800313EC;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1264;                                       // Result = STR_W_Init_InvalidIWADId_Err[0] (80011264)
    I_Error();
loc_800313EC:
    a2 = 1;                                             // Result = 00000001
    a3 = 0;                                             // Result = 00000000
    v0 = lw(sp + 0x14);
    a0 = *gpMainMemZone;
    sw(v0, gp + 0xC0C);                                 // Store to: gNumLumps (800781EC)
    a1 = v0 << 4;
    _thunk_Z_Malloc2();
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a1 = lw(sp + 0x18);
    sw(v0, gp + 0xBE4);                                 // Store to: gpLumpInfo (800781C4)
    a2 = 0;                                             // Result = 00000000
    SeekAndTellFile();
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a2 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    a1 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a2 <<= 4;
    ReadFile();
    a2 = 1;                                             // Result = 00000001
    a3 = 0;                                             // Result = 00000000
    a1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    a0 = *gpMainMemZone;
    a1 <<= 2;
    _thunk_Z_Malloc2();
    a2 = 1;                                             // Result = 00000001
    a0 = *gpMainMemZone;
    a1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    sw(v0, gp + 0xC5C);                                 // Store to: gpLumpCache (8007823C)
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc2();
    a0 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    a2 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    a1 = 0;                                             // Result = 00000000
    sw(v0, gp + 0xD10);                                 // Store to: gpbIsMainWadLump (800782F0)
    a2 <<= 2;
    _thunk_D_memset();
    a0 = lw(gp + 0xD10);                                // Load from: gpbIsMainWadLump (800782F0)
    a2 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    a1 = 0;                                             // Result = 00000000
    _thunk_D_memset();
    ra = lw(sp + 0x30);
    sp += 0x38;
    return;
}

void W_CheckNumForName() noexcept {
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_800314B8:
    v0 = lbu(a0);
    v1 = v0;
    if (v0 == 0) goto loc_800314F0;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_800314DC;
    v1 -= 0x20;
loc_800314DC:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_800314B8;
loc_800314F0:
    a3 = lw(sp);
    a2 = lw(sp + 0x4);
    v1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    v0 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80031548;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_80031514:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_80031538;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_8003154C;
    }
loc_80031538:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80031514;
loc_80031548:
    v0 = -1;                                            // Result = FFFFFFFF
loc_8003154C:
    sp += 0x10;
    return;
}

void W_GetNumForName() noexcept {
loc_80031558:
    sp -= 0x28;
    a1 = a0;
    a0 = sp + 0x10;
    a2 = a1;
    a3 = sp + 0x18;
    sw(ra, sp + 0x20);
    sw(0, sp + 0x10);
    sw(0, sp + 0x14);
loc_80031578:
    v0 = lbu(a2);
    v1 = v0;
    if (v0 == 0) goto loc_800315B0;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a2++;
    if (v0 == 0) goto loc_8003159C;
    v1 -= 0x20;
loc_8003159C:
    sb(v1, a0);
    a0++;
    v0 = (i32(a0) < i32(a3));
    if (v0 != 0) goto loc_80031578;
loc_800315B0:
    t0 = lw(sp + 0x10);
    a3 = lw(sp + 0x14);
    v1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    v0 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80031608;
    t1 = -0x81;                                         // Result = FFFFFF7F
    a2 = v1;
    v1 = v0 + 8;
loc_800315D4:
    v0 = lw(v1 + 0x4);
    if (v0 != a3) goto loc_800315F8;
    v0 = lw(v1);
    v0 &= t1;
    if (v0 == t0) goto loc_80031620;
loc_800315F8:
    a0++;
    v0 = (i32(a0) < i32(a2));
    v1 += 0x10;
    if (v0 != 0) goto loc_800315D4;
loc_80031608:
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003160C:
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 == v0);
        v0 = v1;
        if (bJump) goto loc_80031628;
    }
    goto loc_80031638;
loc_80031620:
    v1 = a0;
    goto loc_8003160C;
loc_80031628:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1284;                                       // Result = STR_W_GetNumForName_NotFound_Err[0] (80011284)
    I_Error();
loc_80031638:
    ra = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void W_LumpLength() noexcept {
loc_80031648:
    v0 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (i32(s0) < i32(v0));
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_80031674;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x12A4;                                       // Result = STR_W_LumpLength_OutOfBounds_Err[0] (800112A4)
    a1 = s0;
    I_Error();
loc_80031674:
    v1 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    v0 = s0 << 4;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void W_ReadLump() noexcept {
    v0 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    sp -= 0x38;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s3, sp + 0x2C);
    s3 = a1;
    sw(s2, sp + 0x28);
    s2 = a2;
    sw(ra, sp + 0x30);
    v0 = (i32(s0) < i32(v0));
    sw(s1, sp + 0x24);
    if (v0 != 0) goto loc_800316D8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x12C4;                                       // Result = STR_W_ReadLump_InvalidLumpIdx_Err[0] (800112C4)
    a1 = s0;
    I_Error();
loc_800316D8:
    v1 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    v0 = s0 << 4;
    s1 = v0 + v1;
    if (s2 == 0) goto loc_80031764;
    v0 = lbu(s1 + 0x8);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80031764;
    a2 = 1;                                             // Result = 00000001
    a0 = *gpMainMemZone;
    a1 = lw(s1 + 0x4);
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc2_b();
    a2 = 0;                                             // Result = 00000000
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a1 = lw(s1);
    s0 = v0;
    SeekAndTellFile();
    a1 = s0;
    v0 = lw(s1 + 0x10);
    a2 = lw(s1);
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a2 = v0 - a2;
    ReadFile();
    a0 = s0;
    a1 = s3;
    decode();
    a0 = *gpMainMemZone;
    a1 = s0;
    _thunk_Z_Free2();
    goto loc_8003178C;
loc_80031764:
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a1 = lw(s1);
    a2 = 0;                                             // Result = 00000000
    SeekAndTellFile();
    a1 = s3;
    v0 = lw(s1 + 0x10);
    a2 = lw(s1);
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a2 = v0 - a2;
    ReadFile();
loc_8003178C:
    ra = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void W_CacheLumpNum() noexcept {
loc_800317AC:
    v0 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    sp -= 0x48;
    sw(s2, sp + 0x38);
    s2 = a0;
    sw(s1, sp + 0x34);
    s1 = a1;
    sw(s4, sp + 0x40);
    s4 = a2;
    sw(ra, sp + 0x44);
    sw(s3, sp + 0x3C);
    v0 = (s2 < v0);
    sw(s0, sp + 0x30);
    if (v0 != 0) goto loc_800317F0;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x12E0;                                       // Result = STR_W_CacheLumpNum_InvalidLumpIdx_Err[0] (800112E0)
    a1 = s2;
    I_Error();
loc_800317F0:
    v0 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    s0 = s2 << 2;
    v0 += s0;
    v0 = lw(v0);
    if (v0 != 0) goto loc_800319B0;
    v0 = lw(gp + 0x604);                                // Load from: gbIsLevelDataCached (80077BE4)
    if (v0 == 0) goto loc_8003182C;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1300;                                       // Result = STR_CacheMissOnLump_Msg[0] (80011300)
    a1 = s2;
    I_Error();
loc_8003182C:
    a2 = s1;
    if (s4 == 0) goto loc_80031858;
    v1 = s2 << 4;
    a0 = *gpMainMemZone;
    v0 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a3 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    v1 += v0;
    a1 = lw(v1 + 0x4);
    a3 += s0;
    goto loc_80031880;
loc_80031858:
    v0 = s2 << 4;
    a0 = *gpMainMemZone;
    v1 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a3 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    v0 += v1;
    v1 = lw(v0 + 0x10);
    a1 = lw(v0);
    a3 += s0;
    a1 = v1 - a1;
loc_80031880:
    _thunk_Z_Malloc2();
    v0 = s2 << 2;
    a0 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    v1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    v0 += a0;
    v1 = (i32(s2) < i32(v1));
    s3 = lw(v0);
    if (v1 != 0) goto loc_800318B8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x12C4;                                       // Result = STR_W_ReadLump_InvalidLumpIdx_Err[0] (800112C4)
    a1 = s2;
    I_Error();
loc_800318B8:
    v1 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    v0 = s2 << 4;
    s1 = v0 + v1;
    if (s4 == 0) goto loc_80031944;
    v0 = lbu(s1 + 0x8);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80031944;
    a2 = 1;                                             // Result = 00000001
    a0 = *gpMainMemZone;
    a1 = lw(s1 + 0x4);
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc2_b();
    a2 = 0;                                             // Result = 00000000
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a1 = lw(s1);
    s0 = v0;
    SeekAndTellFile();
    a1 = s0;
    v0 = lw(s1 + 0x10);
    a2 = lw(s1);
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a2 = v0 - a2;
    ReadFile();
    a0 = s0;
    a1 = s3;
    decode();
    a0 = *gpMainMemZone;
    a1 = s0;
    _thunk_Z_Free2();
    goto loc_8003196C;
loc_80031944:
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a1 = lw(s1);
    a2 = 0;                                             // Result = 00000000
    SeekAndTellFile();
    a1 = s3;
    v0 = lw(s1 + 0x10);
    a2 = lw(s1);
    a0 = lw(gp + 0xC74);                                // Load from: gMainWadFileIdx (80078254)
    a2 = v0 - a2;
    ReadFile();
loc_8003196C:
    v0 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    v1 = s2 << 4;
    v1 += v0;
    v0 = lbu(v1 + 0x8);
    v0 &= 0x80;
    v1 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800319A0;
    v0 = lw(gp + 0xD10);                                // Load from: gpbIsMainWadLump (800782F0)
    v0 += s2;
    sb(s4, v0);
    goto loc_800319B0;
loc_800319A0:
    v0 = lw(gp + 0xD10);                                // Load from: gpbIsMainWadLump (800782F0)
    v0 += s2;
    sb(v1, v0);
loc_800319B0:
    v1 = lw(gp + 0xC5C);                                // Load from: gpLumpCache (8007823C)
    v0 = s2 << 2;
    v0 += v1;
    v0 = lw(v0);
    ra = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x48;
    return;
}

void W_CacheLumpName() noexcept {
loc_800319E4:
    sp -= 0x30;
    t1 = a0;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(s2, sp + 0x28);
    s2 = a2;
    a0 = sp + 0x10;
    a1 = t1;
    a2 = sp + 0x18;
    sw(ra, sp + 0x2C);
    sw(s0, sp + 0x20);
    sw(0, sp + 0x10);
    sw(0, sp + 0x14);
loc_80031A18:
    v0 = lbu(a1);
    v1 = v0;
    if (v0 == 0) goto loc_80031A50;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a1++;
    if (v0 == 0) goto loc_80031A3C;
    v1 -= 0x20;
loc_80031A3C:
    sb(v1, a0);
    a0++;
    v0 = (i32(a0) < i32(a2));
    if (v0 != 0) goto loc_80031A18;
loc_80031A50:
    a3 = lw(sp + 0x10);
    a2 = lw(sp + 0x14);
    v1 = lw(gp + 0xC0C);                                // Load from: gNumLumps (800781EC)
    v0 = lw(gp + 0xBE4);                                // Load from: gpLumpInfo (800781C4)
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80031AA8;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_80031A74:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_80031A98;
    v0 = lw(v1);
    v0 &= t0;
    if (v0 == a3) goto loc_80031AC0;
loc_80031A98:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80031A74;
loc_80031AA8:
    v1 = -1;                                            // Result = FFFFFFFF
loc_80031AAC:
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_80031AC8;
    s0 = v1;
    goto loc_80031AD8;
loc_80031AC0:
    v1 = a0;
    goto loc_80031AAC;
loc_80031AC8:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1284;                                       // Result = STR_W_GetNumForName_NotFound_Err[0] (80011284)
    a1 = t1;
    I_Error();
loc_80031AD8:
    a0 = s0;
    a1 = s1;
    a2 = s2;
    W_CacheLumpNum();
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void W_OpenMapWad() noexcept {
loc_80031B04:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    OpenFile();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 2;                                             // Result = 00000002
    SeekAndTellFile();
    s1 = v0;
    a1 = s1;
    a2 = 1;                                             // Result = 00000001
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc2_b();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    sw(v0, gp + 0x938);                                 // Store to: gpMapWadFileData (80077F18)
    a2 = 0;                                             // Result = 00000000
    SeekAndTellFile();
    a0 = s0;
    a1 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a2 = s1;
    ReadFile();
    a0 = s0;
    CloseFile();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7BE8;                                       // Result = STR_IWAD[0] (80077BE8)
    a0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a2 = 4;                                             // Result = 00000004
    D_strncasecmp();
    if (v0 == 0) goto loc_80031BA0;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1318;                                       // Result = STR_W_OpenMapWad_InvalidIWADId_Err[0] (80011318)
    I_Error();
loc_80031BA0:
    v0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    v1 = lw(v0 + 0x4);
    a0 = lw(v0 + 0x8);
    sw(v1, gp + 0xB3C);                                 // Store to: gNumMapWadLumps (8007811C)
    v1 = v0 + a0;
    sw(v1, gp + 0xAE8);                                 // Store to: gpMapWadDirectory (800780C8)
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void W_MapLumpLength() noexcept {
loc_80031BD4:
    v0 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (i32(s0) < i32(v0));
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_80031C00;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x133C;                                       // Result = STR_W_MapLumpLength_OutOfRange_Err[0] (8001133C)
    a1 = s0;
    I_Error();
loc_80031C00:
    v1 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    v0 = s0 << 4;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void W_MapGetNumForName() noexcept {
loc_80031C24:
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_80031C38:
    v0 = lbu(a0);
    if (v0 == 0) goto loc_80031C78;
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_80031C64;
    v1 -= 0x20;
loc_80031C64:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_80031C38;
loc_80031C78:
    a3 = lw(sp);
    a2 = lw(sp + 0x4);
    v1 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    v0 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80031CD0;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_80031C9C:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_80031CC0;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_80031CD4;
    }
loc_80031CC0:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80031C9C;
loc_80031CD0:
    v0 = -1;                                            // Result = FFFFFFFF
loc_80031CD4:
    sp += 0x10;
    return;
}

void W_ReadMapLump() noexcept {
loc_80031CE0:
    v0 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    sp -= 0x30;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s2, sp + 0x28);
    s2 = a1;
    sw(s1, sp + 0x24);
    s1 = a2;
    v0 = (i32(s0) < i32(v0));
    sw(ra, sp + 0x2C);
    if (v0 != 0) goto loc_80031D1C;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1360;                                       // Result = STR_W_ReadMapLump_OutOfRange_Err[0] (80011360)
    a1 = s0;
    I_Error();
loc_80031D1C:
    v1 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    v0 = s0 << 4;
    v1 += v0;
    if (s1 == 0) goto loc_80031D58;
    v0 = lbu(v1 + 0x8);
    v0 &= 0x80;
    a1 = s2;
    if (v0 == 0) goto loc_80031D58;
    v0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a0 = lw(v1);
    a0 += v0;
    decode();
    goto loc_80031D74;
loc_80031D58:
    a0 = s2;
    a1 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    v0 = lw(v1);
    a2 = lw(v1 + 0x10);
    a1 += v0;
    a2 -= v0;
    _thunk_D_memcpy();
loc_80031D74:
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void decode() noexcept {
loc_80031D90:
    sp -= 8;
    a2 = a0;
    t0 = 0;                                             // Result = 00000000
    t1 = 0;                                             // Result = 00000000
    t2 = 1;                                             // Result = 00000001
loc_80031DA4:
    v0 = t1 + 1;
    if (t1 != 0) goto loc_80031DB4;
    t0 = lbu(a2);
    a2++;
loc_80031DB4:
    t1 = v0 & 7;
    v0 = t0 & 1;
    if (v0 == 0) goto loc_80031E24;
    v0 = lbu(a2);
    a2++;
    v1 = lbu(a2);
    a0 = lbu(a2);
    a2++;
    v0 <<= 4;
    v1 >>= 4;
    v0 |= v1;
    v0 = a1 - v0;
    a0 &= 0xF;
    a0++;
    a3 = v0 - 1;
    if (a0 == t2) goto loc_80031E3C;
    v1 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_80031E34;
loc_80031E00:
    v0 = lbu(a3);
    a3++;
    v1++;
    sb(v0, a1);
    v0 = (i32(v1) < i32(a0));
    a1++;
    if (v0 != 0) goto loc_80031E00;
    t0 = u32(i32(t0) >> 1);                             // Result = 00000000
    goto loc_80031DA4;
loc_80031E24:
    v0 = lbu(a2);
    a2++;
    sb(v0, a1);
    a1++;
loc_80031E34:
    t0 = u32(i32(t0) >> 1);                             // Result = 00000000
    goto loc_80031DA4;
loc_80031E3C:
    sp += 8;
    return;
}

void getDecodedSize() noexcept {
loc_80031E48:
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    v1 = 0;                                             // Result = 00000000
    a3 = 1;                                             // Result = 00000001
loc_80031E58:
    v0 = a2 + 1;
    if (a2 != 0) goto loc_80031E68;
    a1 = lbu(a0);
    a0++;
loc_80031E68:
    a2 = v0 & 7;
    v0 = a1 & 1;
    if (v0 == 0) goto loc_80031E9C;
    a0++;
    v0 = lbu(a0);
    v0 &= 0xF;
    v0++;
    a0++;
    if (v0 == a3) goto loc_80031EAC;
    v1 += v0;
    goto loc_80031EA4;
loc_80031E9C:
    v1++;
    a0++;
loc_80031EA4:
    a1 = u32(i32(a1) >> 1);                             // Result = 00000000
    goto loc_80031E58;
loc_80031EAC:
    v0 = v1;
    return;
}

#include "p_lights.h"

#include "Doom/Base/m_random.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

// Definition and state for a fire flicker light
struct fireflicker_t {
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    int32_t             count;
    int32_t             maxlight;
    int32_t             minlight;
};

static_assert(sizeof(fireflicker_t) == 28);

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a light that flickers like fire
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_FireFlicker(fireflicker_t& flicker) noexcept {
    // Time to flicker yet?
    if (--flicker.count > 0)
        return;

    // Do the flicker and reset the countdown till the next flicker
    sector_t& sector = *flicker.sector;
    const int32_t variation = (P_Random() & 3) * 16;

    if (sector.lightlevel - variation < flicker.minlight) {
        sector.lightlevel = (int16_t)(flicker.minlight);
    } else {
        sector.lightlevel = (int16_t)(flicker.maxlight - variation);
    }

    flicker.count = 3;
}

// TODO: REMOVE eventually
void _thunk_T_FireFlicker() noexcept {
    T_FireFlicker(*vmAddrToPtr<fireflicker_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a fire flicker light effect on the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnFireFlicker(sector_t& sector) noexcept {
    // Clear the current sector special (no hurt for example) and spawn the thinker
    sector.special = 0;
    fireflicker_t& flicker = *(fireflicker_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(fireflicker_t), PU_LEVSPEC, nullptr);
    P_AddThinker(flicker.thinker);

    // Setup flicker parameters
    flicker.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_FireFlicker);
    flicker.sector = &sector;
    flicker.maxlight = sector.lightlevel;
    flicker.minlight = P_FindMinSurroundingLight(sector, sector.lightlevel) + 16;
    flicker.count = 3;
}

void T_LightFlash() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x10);
    v0--;
    sw(v0, s0 + 0x10);
    if (v0 != 0) goto loc_8001AF00;
    a0 = lw(s0 + 0xC);
    v0 = lw(s0 + 0x14);
    v1 = lh(a0 + 0x12);
    if (v1 != v0) goto loc_8001AEE0;
    v0 = lhu(s0 + 0x18);
    sh(v0, a0 + 0x12);
    _thunk_P_Random();
    v1 = lw(s0 + 0x20);
    v0 &= v1;
    goto loc_8001AEF8;
loc_8001AEE0:
    v0 = lhu(s0 + 0x14);
    sh(v0, a0 + 0x12);
    _thunk_P_Random();
    v1 = lw(s0 + 0x1C);
    v0 &= v1;
loc_8001AEF8:
    v0++;
    sw(v0, s0 + 0x10);
loc_8001AF00:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_SpawnLightFlash() noexcept {
loc_8001AF14:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a1 = 0x24;                                          // Result = 00000024
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    sw(0, s1 + 0x14);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x5174;                                       // Result = T_LightFlash (8001AE8C)
    sw(v0, s0 + 0x8);
    sw(s1, s0 + 0xC);
    v0 = lh(s1 + 0x12);
    sw(v0, s0 + 0x14);
    a1 = lh(s1 + 0x12);
    a0 = s1;
    v0 = P_FindMinSurroundingLight(*vmAddrToPtr<sector_t>(a0), a1);
    sw(v0, s0 + 0x18);
    v0 = 0x40;                                          // Result = 00000040
    sw(v0, s0 + 0x1C);
    v0 = 7;                                             // Result = 00000007
    sw(v0, s0 + 0x20);
    _thunk_P_Random();
    v1 = lw(s0 + 0x1C);
    v0 &= v1;
    v0++;
    sw(v0, s0 + 0x10);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void T_StrobeFlash() noexcept {
    v0 = lw(a0 + 0x10);
    v0--;
    sw(v0, a0 + 0x10);
    if (v0 != 0) goto loc_8001B018;
    a1 = lw(a0 + 0xC);
    v0 = lw(a0 + 0x14);
    v1 = lh(a1 + 0x12);
    if (v1 != v0) goto loc_8001B000;
    v0 = lhu(a0 + 0x18);
    sh(v0, a1 + 0x12);
    v0 = lw(a0 + 0x20);
    sw(v0, a0 + 0x10);
    goto loc_8001B018;
loc_8001B000:
    v0 = lhu(a0 + 0x14);
    sh(v0, a1 + 0x12);
    v0 = lw(a0 + 0x1C);
    sw(v0, a0 + 0x10);
loc_8001B018:
    return;
}

void P_SpawnStrobeFlash() noexcept {
loc_8001B020:
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    a1 = 0x24;                                          // Result = 00000024
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x14);
    _thunk_Z_Malloc();
    s1 = v0;
    a0 = s1;
    _thunk_P_AddThinker();
    v0 = 3;                                             // Result = 00000003
    sw(s2, s1 + 0xC);
    sw(s0, s1 + 0x1C);
    sw(v0, s1 + 0x20);
    a1 = lh(s2 + 0x12);
    a0 = s2;
    v0 = P_FindMinSurroundingLight(*vmAddrToPtr<sector_t>(a0), a1);
    sw(v0, s1 + 0x14);
    v0 = lh(s2 + 0x12);
    a0 = lw(s1 + 0x14);
    sw(v0, s1 + 0x18);
    v1 = lw(s1 + 0x18);
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x5044;                                       // Result = T_StrobeFlash (8001AFBC)
    sw(v0, s1 + 0x8);
    if (a0 != v1) goto loc_8001B0AC;
    sw(0, s1 + 0x14);
loc_8001B0AC:
    if (s3 != 0) goto loc_8001B0C8;
    _thunk_P_Random();
    v0 &= 7;
    v0++;
    goto loc_8001B0CC;
loc_8001B0C8:
    v0 = 1;                                             // Result = 00000001
loc_8001B0CC:
    sw(v0, s1 + 0x10);
    sw(0, s2 + 0x14);
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_SpawnRapidStrobeFlash() noexcept {
loc_8001B0F4:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a1 = 0x24;                                          // Result = 00000024
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v1 = 1;                                             // Result = 00000001
    v0 = 0xA;                                           // Result = 0000000A
    sw(s1, s0 + 0xC);
    sw(v1, s0 + 0x1C);
    sw(v1, s0 + 0x20);
    sw(v0, s0 + 0x14);
    v0 = lh(s1 + 0x12);
    a1 = lw(s0 + 0x14);
    sw(v1, s0 + 0x10);
    sw(v0, s0 + 0x18);
    a0 = lw(s0 + 0x18);
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x5044;                                       // Result = T_StrobeFlash (8001AFBC)
    sw(v0, s0 + 0x8);
    if (a1 != a0) goto loc_8001B16C;
    sw(0, s0 + 0x14);
loc_8001B16C:
    sw(0, s1 + 0x14);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void EV_StartLightStrobing() noexcept {
loc_8001B188:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s2, sp + 0x18);
    s2 = -1;                                            // Result = FFFFFFFF
    sw(s4, sp + 0x20);
    s4 = 0xF;                                           // Result = 0000000F
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
loc_8001B1B0:
    a0 = s3;
    a1 = s2;
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
    s2 = v0;
    v0 = s2 << 1;
    if (i32(s2) < 0) goto loc_8001B274;
    v0 += s2;
    v0 <<= 3;
    v0 -= s2;
    v1 = *gpSectors;
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x24;                                          // Result = 00000024
    if (v0 != 0) goto loc_8001B1B0;
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 3;                                             // Result = 00000003
    sw(s1, s0 + 0xC);
    sw(s4, s0 + 0x1C);
    sw(v0, s0 + 0x20);
    a1 = lh(s1 + 0x12);
    a0 = s1;
    v0 = P_FindMinSurroundingLight(*vmAddrToPtr<sector_t>(a0), a1);
    sw(v0, s0 + 0x14);
    v0 = lh(s1 + 0x12);
    a0 = lw(s0 + 0x14);
    sw(v0, s0 + 0x18);
    v1 = lw(s0 + 0x18);
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x5044;                                       // Result = T_StrobeFlash (8001AFBC)
    sw(v0, s0 + 0x8);
    if (a0 != v1) goto loc_8001B258;
    sw(0, s0 + 0x14);
loc_8001B258:
    _thunk_P_Random();
    v0 &= 7;
    v0++;
    sw(v0, s0 + 0x10);
    sw(0, s1 + 0x14);
    goto loc_8001B1B0;
loc_8001B274:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void EV_TurnTagLightsOff() noexcept {
loc_8001B298:
    v0 = *gNumSectors;
    sp -= 0x40;
    sw(s3, sp + 0x2C);
    s3 = *gpSectors;
    sw(s5, sp + 0x34);
    s5 = a0;
    sw(s4, sp + 0x30);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x38);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    if (i32(v0) <= 0) goto loc_8001B36C;
    s1 = s3 + 0x12;
loc_8001B2D8:
    v1 = lw(s1 + 0x6);
    v0 = lw(s5 + 0x18);
    if (v1 != v0) goto loc_8001B34C;
    v0 = lw(s1 + 0x42);
    s2 = lh(s1);
    s0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8001B348;
loc_8001B2FC:
    v1 = lw(s1 + 0x46);
    v0 = s0 << 2;
    v0 += v1;
    a0 = lw(v0);
    a1 = s3;
    v0 = ptrToVmAddr(getNextSector(*vmAddrToPtr<line_t>(a0), *vmAddrToPtr<sector_t>(a1)));
    s0++;
    if (v0 == 0) goto loc_8001B334;
    v1 = lh(v0 + 0x12);
    v0 = (i32(v1) < i32(s2));
    if (v0 == 0) goto loc_8001B334;
    s2 = v1;
loc_8001B334:
    v0 = lw(s1 + 0x42);
    v0 = (i32(s0) < i32(v0));
    if (v0 != 0) goto loc_8001B2FC;
loc_8001B348:
    sh(s2, s1);
loc_8001B34C:
    s4++;
    s1 += 0x5C;
    v0 = *gNumSectors;
    v0 = (i32(s4) < i32(v0));
    s3 += 0x5C;
    if (v0 != 0) goto loc_8001B2D8;
loc_8001B36C:
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

void EV_LightTurnOn() noexcept {
loc_8001B394:
    v0 = *gNumSectors;
    sp -= 0x38;
    sw(s3, sp + 0x24);
    s3 = *gpSectors;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(s2, sp + 0x20);
    s2 = a1;
    sw(s4, sp + 0x28);
    s4 = 0;
    sw(ra, sp + 0x30);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (i32(v0) <= 0) goto loc_8001B478;
    s1 = s3 + 0x12;
loc_8001B3D8:
    v1 = lw(s1 + 0x6);
    v0 = lw(s5 + 0x18);
    if (v1 != v0) goto loc_8001B458;
    if (s2 != 0) goto loc_8001B454;
    v0 = lw(s1 + 0x42);
    v0 = (i32(s2) < i32(v0));
    s0 = 0;
    if (v0 == 0) goto loc_8001B454;
loc_8001B408:
    v1 = lw(s1 + 0x46);
    v0 = s0 << 2;
    v0 += v1;
    a0 = lw(v0);
    a1 = s3;
    v0 = ptrToVmAddr(getNextSector(*vmAddrToPtr<line_t>(a0), *vmAddrToPtr<sector_t>(a1)));
    s0++;
    if (v0 == 0) goto loc_8001B440;
    v1 = lh(v0 + 0x12);
    v0 = (i32(s2) < i32(v1));
    if (v0 == 0) goto loc_8001B440;
    s2 = v1;
loc_8001B440:
    v0 = lw(s1 + 0x42);
    v0 = (i32(s0) < i32(v0));
    if (v0 != 0) goto loc_8001B408;
loc_8001B454:
    sh(s2, s1);
loc_8001B458:
    s4++;
    s1 += 0x5C;
    v0 = *gNumSectors;
    v0 = (i32(s4) < i32(v0));
    s3 += 0x5C;
    if (v0 != 0) goto loc_8001B3D8;
loc_8001B478:
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

void T_Glow() noexcept {
    v1 = lw(a0 + 0x18);
    a2 = -1;                                            // Result = FFFFFFFF
    v0 = 1;                                             // Result = 00000001
    if (v1 == a2) goto loc_8001B4C0;
    if (v1 == v0) goto loc_8001B50C;
    goto loc_8001B550;
loc_8001B4C0:
    v1 = lw(a0 + 0xC);
    v0 = lhu(v1 + 0x12);
    v0 -= 3;
    sh(v0, v1 + 0x12);
    a1 = lw(a0 + 0xC);
    v1 = lw(a0 + 0x10);
    v0 = lh(a1 + 0x12);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001B550;
    v0 = lhu(a0 + 0x10);
    sh(v0, a1 + 0x12);
    v0 = 1;                                             // Result = 00000001
    sw(v0, a0 + 0x18);
    goto loc_8001B550;
loc_8001B50C:
    v1 = lw(a0 + 0xC);
    v0 = lhu(v1 + 0x12);
    v0 += 3;
    sh(v0, v1 + 0x12);
    a1 = lw(a0 + 0xC);
    v0 = lw(a0 + 0x14);
    v1 = lh(a1 + 0x12);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001B550;
    v0 = lhu(a0 + 0x14);
    sh(v0, a1 + 0x12);
    sw(a2, a0 + 0x18);
loc_8001B550:
    return;
}

void P_SpawnGlowingLight() noexcept {
loc_8001B558:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    a1 = 0x1C;                                          // Result = 0000001C
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x4B60;                                       // Result = T_Glow (8001B4A0)
    a0 = 1;                                             // Result = 00000001
    sw(s1, s0 + 0xC);
    sw(v0, s0 + 0x8);
    if (s2 == a0) goto loc_8001B5F4;
    v0 = (i32(s2) < 2);
    if (v0 == 0) goto loc_8001B5CC;
    if (s2 == 0) goto loc_8001B5E0;
    sw(0, s1 + 0x14);
    goto loc_8001B624;
loc_8001B5CC:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (s2 == v0);
        v0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_8001B610;
    }
    sw(0, s1 + 0x14);
    goto loc_8001B624;
loc_8001B5E0:
    a1 = lh(s1 + 0x12);
    a0 = s1;
    v0 = P_FindMinSurroundingLight(*vmAddrToPtr<sector_t>(a0), a1);
    sw(v0, s0 + 0x10);
    goto loc_8001B5FC;
loc_8001B5F4:
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s0 + 0x10);
loc_8001B5FC:
    v1 = lh(s1 + 0x12);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x18);
    sw(v1, s0 + 0x14);
    goto loc_8001B620;
loc_8001B610:
    v1 = lh(s1 + 0x12);
    sw(v0, s0 + 0x14);
    sw(a0, s0 + 0x18);
    sw(v1, s0 + 0x10);
loc_8001B620:
    sw(0, s1 + 0x14);
loc_8001B624:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

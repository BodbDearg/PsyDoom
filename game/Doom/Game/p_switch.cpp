#include "p_switch.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_data.h"
#include "p_ceiling.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_lights.h"
#include "p_plats.h"
#include "p_setup.h"
#include "p_spec.h"
#include "PsxVm/PsxVm.h"

// Defines the two textures used by a switch
struct switchlist_t {
    char    name1[9];
    char    name2[9];
};

// All of the switch textures in the game
static const switchlist_t gAlphSwitchList[] = {
    { "SW1BMET",  "SW2BMET"  },
    { "SW1BRICK", "SW2BRICK" },
    { "SW1BRNZ",  "SW2BRNZ"  },
    { "SW1BROWN", "SW2BROWN" },
    { "SW1MARB",  "SW2MARB"  },
    { "SW1MET",   "SW2MET"   },
    { "SW1METAL", "SW2METAL" },
    { "SW1NEW02", "SW2NEW02" },
    { "SW1NEW03", "SW2NEW03" },
    { "SW1NEW04", "SW2NEW04" },
    { "SW1NEW05", "SW2NEW05" },
    { "SW1NEW06", "SW2NEW06" },
    { "SW1NEW10", "SW2NEW10" },
    { "SW1NEW11", "SW2NEW11" },
    { "SW1NEW13", "SW2NEW13" },
    { "SW1NEW14", "SW2NEW14" },
    { "SW1NEW20", "SW2NEW20" },
    { "SW1NEW25", "SW2NEW25" },
    { "SW1NEW26", "SW2NEW26" },
    { "SW1NEW28", "SW2NEW28" },
    { "SW1NEW29", "SW2NEW29" },
    { "SW1NEW30", "SW2NEW30" },
    { "SW1NEW31", "SW2NEW31" },
    { "SW1NEW32", "SW2NEW32" },
    { "SW1NEW33", "SW2NEW33" },
    { "SW1NEW34", "SW2NEW34" },
    { "SW1NEW35", "SW2NEW35" },
    { "SW1NEW36", "SW2NEW36" },
    { "SW1NEW39", "SW2NEW39" },
    { "SW1NEW40", "SW2NEW40" },
    { "SW1NEW41", "SW2NEW41" },
    { "SW1NEW42", "SW2NEW42" },
    { "SW1NEW45", "SW2NEW45" },
    { "SW1NEW46", "SW2NEW46" },
    { "SW1NEW47", "SW2NEW47" },
    { "SW1NEW51", "SW2NEW51" },
    { "SW1NEW57", "SW2NEW57" },
    { "SW1NEW60", "SW2NEW60" },
    { "SW1NEW63", "SW2NEW63" },
    { "SW1NEW65", "SW2NEW65" },
    { "SW1NEW66", "SW2NEW66" },
    { "SW1NEW68", "SW2NEW68" },
    { "SW1NEW69", "SW2NEW69" },
    { "SW1NEW70", "SW2NEW70" },
    { "SW1RED",   "SW2RED"   },
    { "SW1RUST",  "SW2RUST"  },
    { "SW1SKULL", "SW2SKULL" },
    { "SW1STAR",  "SW2STAR"  },
    { "SW1STEEL", "SW2STEEL" },
};

static constexpr int32_t NUM_SWITCH_TYPES = C_ARRAY_SIZE(gAlphSwitchList);

// The 2 lumps for each switch texture in the game
static const VmPtr<int32_t[NUM_SWITCH_TYPES * 2]> gSwitchList(0x800975FC);

//------------------------------------------------------------------------------------------------------------------------------------------
// Caches textures for all active switches in the level.
// Must be done after 64 pixel wide wall textures have been cached in order to work.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitSwitchList() noexcept {
    int32_t* pSwitchLump = gSwitchList.get();

    for (int32_t switchIdx = 0; switchIdx < NUM_SWITCH_TYPES; ++switchIdx) {
        // Get both textures for the switch
        const int32_t tex1Lump = R_TextureNumForName(gAlphSwitchList[switchIdx].name1);
        const int32_t tex2Lump = R_TextureNumForName(gAlphSwitchList[switchIdx].name2);
        
        texture_t& tex1 = (*gpTextures)[tex1Lump];
        texture_t& tex2 = (*gpTextures)[tex2Lump];

        // Cache the other switch texture if one of the switch textures is loaded
        if ((tex1.texPageId) != 0 && (tex2.texPageId == 0)) {
            I_CacheTex(tex2);
        }

        if ((tex2.texPageId != 0) && (tex1.texPageId == 0)) {
            I_CacheTex(tex1);
        }
        
        // Save what lumps the switch uses
        pSwitchLump[0] = tex1Lump;
        pSwitchLump[1] = tex2Lump;
        pSwitchLump += 2;
    }
}

void P_StartButton() noexcept {
    t1 = 0;                                             // Result = 00000000
    t0 = 0x80090000;                                    // Result = 80090000
    t0 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80027EB8:
    v0 = lw(t0);
    if (v0 != 0) goto loc_80027F20;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(a0, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(a1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(a3, t0);
    v0 = lw(a0 + 0x38);
    v0 += 0x38;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77BC;                                       // Result = gButtonList_1[4] (800977BC)
    at += v1;
    sw(v0, at);
    goto loc_80027F34;
loc_80027F20:
    t0 += 0x14;
    t1++;
    v0 = (i32(t1) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80027EB8;
loc_80027F34:
    return;
}

void P_ChangeSwitchTexture() noexcept {
loc_80027F3C:
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    if (s3 != 0) goto loc_80027F68;
    sw(0, s1 + 0x14);
loc_80027F68:
    v1 = lw(s1 + 0x1C);
    a0 = 0xB;
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    t0 = lw(v0 + 0x8);
    a3 = lw(v0 + 0x10);
    v1 = lw(s1 + 0x14);
    a2 = lw(v0 + 0xC);
    a1 = sfx_swtchn;
    if (v1 != a0) goto loc_80027FA4;
    a1 = sfx_swtchx;
loc_80027FA4:
    s0 = 0;                                             // Result = 00000000
    s2 = 0x80090000;                                    // Result = 80090000
    s2 += 0x75FC;                                       // Result = gSwitchList[0] (800975FC)
    s4 = s2;                                            // Result = gSwitchList[0] (800975FC)
loc_80027FB4:
    v0 = lw(s2);
    if (v0 != t0) goto loc_80028080;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x1C);
    a0 = *gpSides;
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0x8);
    if (s3 == 0) goto loc_80028208;
    a2 = lw(s2);
    a3 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80028020:
    v0 = lw(a1);
    a0++;
    if (v0 != 0) goto loc_80028068;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(s1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(0, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(a3, a1);
    goto loc_800281BC;
loc_80028068:
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80028020;
    goto loc_80028208;
loc_80028080:
    if (v0 != a3) goto loc_80028110;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x1C);
    a0 = *gpSides;
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0x10);
    if (s3 == 0) goto loc_80028208;
    a3 = 1;                                             // Result = 00000001
    a2 = lw(s2);
    t0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_800280E8:
    v0 = lw(a1);
    a0++;
    if (v0 == 0) goto loc_80028188;
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_800280E8;
    goto loc_80028208;
loc_80028110:
    if (v0 != a2) goto loc_800281F8;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x1C);
    a0 = *gpSides;
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0xC);
    if (s3 == 0) goto loc_80028208;
    a3 = 2;                                             // Result = 00000002
    a2 = lw(s2);
    t0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80028178:
    v0 = lw(a1);
    a0++;
    if (v0 != 0) goto loc_800281E0;
loc_80028188:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(s1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(a3, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(t0, a1);
loc_800281BC:
    v0 = lw(s1 + 0x38);
    v0 += 0x38;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77BC;                                       // Result = gButtonList_1[4] (800977BC)
    at += v1;
    sw(v0, at);
    goto loc_80028208;
loc_800281E0:
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80028178;
    goto loc_80028208;
loc_800281F8:
    s0++;
    v0 = (s0 < 0x62);
    s2 += 4;
    if (v0 != 0) goto loc_80027FB4;
loc_80028208:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_UseSpecialLine() noexcept {
loc_8002822C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x80);
    s0 = a1;
    if (v0 != 0) goto loc_80028274;
    v0 = lw(s0 + 0x10);
    v0 &= 0x20;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028808;
    }
    v1 = lw(s0 + 0x14);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028808;
    }
loc_80028274:
    v0 = lw(s0 + 0x14);
    v1 = v0 - 1;
    v0 = (v1 < 0x8B);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80028804;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0xE34;                                        // Result = JumpTable_P_UseSpecialLine[0] (80010E34)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x800282BC: goto loc_800282BC;
        case 0x80028804: goto loc_80028804;
        case 0x800285C4: goto loc_800285C4;
        case 0x800285E0: goto loc_800285E0;
        case 0x800285F8: goto loc_800285F8;
        case 0x8002860C: goto loc_8002860C;
        case 0x8002862C: goto loc_8002862C;
        case 0x8002864C: goto loc_8002864C;
        case 0x80028668: goto loc_80028668;
        case 0x80028688: goto loc_80028688;
        case 0x800286A8: goto loc_800286A8;
        case 0x800282A8: goto loc_800282A8;
        case 0x800286C4: goto loc_800286C4;
        case 0x800286E0: goto loc_800286E0;
        case 0x8002842C: goto loc_8002842C;
        case 0x80028448: goto loc_80028448;
        case 0x80028464: goto loc_80028464;
        case 0x80028718: goto loc_80028718;
        case 0x80028734: goto loc_80028734;
        case 0x80028750: goto loc_80028750;
        case 0x80028768: goto loc_80028768;
        case 0x80028480: goto loc_80028480;
        case 0x8002849C: goto loc_8002849C;
        case 0x800284B8: goto loc_800284B8;
        case 0x800284D8: goto loc_800284D8;
        case 0x800284F4: goto loc_800284F4;
        case 0x80028550: goto loc_80028550;
        case 0x80028510: goto loc_80028510;
        case 0x80028530: goto loc_80028530;
        case 0x8002856C: goto loc_8002856C;
        case 0x8002858C: goto loc_8002858C;
        case 0x800285A8: goto loc_800285A8;
        case 0x800286FC: goto loc_800286FC;
        case 0x800282D0: goto loc_800282D0;
        case 0x80028784: goto loc_80028784;
        case 0x800287A0: goto loc_800287A0;
        case 0x800287BC: goto loc_800287BC;
        case 0x80028328: goto loc_80028328;
        case 0x80028344: goto loc_80028344;
        case 0x80028360: goto loc_80028360;
        case 0x8002837C: goto loc_8002837C;
        case 0x80028398: goto loc_80028398;
        case 0x800283B4: goto loc_800283B4;
        case 0x800283D0: goto loc_800283D0;
        case 0x800283F0: goto loc_800283F0;
        case 0x80028410: goto loc_80028410;
        case 0x800282FC: goto loc_800282FC;
        case 0x800287D8: goto loc_800287D8;
        case 0x800287E4: goto loc_800287E4;
        default: jump_table_err(); break;
    }
loc_800282A8:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
loc_800282BC:
    a0 = s0;
    a1 = s1;
    EV_VerticalDoor();
    v0 = 1;                                             // Result = 00000001
    goto loc_80028808;
loc_800282D0:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800282FC:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028328:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028344:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028360:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002837C:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028398:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800283B4:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800283D0:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800283F0:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028410:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_BuildStairs();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002842C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028448:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoCeiling();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028464:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028480:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002849C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284B8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    a2 = 1;                                             // Result = 00000001
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284D8:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284F4:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028510:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x18;                                          // Result = 00000018
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028530:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x20;                                          // Result = 00000020
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028550:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002856C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002858C:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800285A8:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800285C4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_BuildStairs();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800285E0:
    a0 = s0;
    EV_DoDonut();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800285F8:
    G_ExitLevel();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002860C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x20;                                          // Result = 00000020
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002862C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x18;                                          // Result = 00000018
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002864C:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028668:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028688:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286A8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286C4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286E0:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoCeiling();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286FC:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028718:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoCeiling();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028734:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028750:
    a0 = lw(s0 + 0x18);
    G_SecretExitLevel();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028768:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028784:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287A0:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287BC:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287D8:
    a0 = s0;
    a1 = 0xFF;                                          // Result = 000000FF
    goto loc_800287EC;
loc_800287E4:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
loc_800287EC:
    EV_LightTurnOn();
    a0 = s0;
loc_800287F8:
    a1 = 1;                                             // Result = 00000001
loc_800287FC:
    P_ChangeSwitchTexture();
loc_80028804:
    v0 = 1;                                             // Result = 00000001
loc_80028808:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

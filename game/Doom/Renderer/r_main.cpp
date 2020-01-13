#include "r_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_bsp.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_things.h"

// View properties
const VmPtr<angle_t>    gViewAngle(0x80078294);
const VmPtr<bool32_t>   gbIsSkyVisible(0x800781F4);

// Light properties
const VmPtr<bool32_t>   gbDoViewLighting(0x80078264);
const VmPtr<uint32_t>   gCurLightValR(0x80077E8C);
const VmPtr<uint32_t>   gCurLightValG(0x80078034);
const VmPtr<uint32_t>   gCurLightValB(0x80077F70);

void R_Init() noexcept {
loc_800305B0:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    R_InitData();
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6544;                                       // Result = 80086544
    s1 = s0 - 0x14;                                     // Result = gViewSinDiv16 (80086530)
    sw(0, s0);                                          // Store to: 80086544
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6548);                                 // Store to: 80086548
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x654C);                                 // Store to: 8008654C
    a0 = s1;                                            // Result = gViewSinDiv16 (80086530)
    LIBGTE_SetTransMatrix();
    a0 = s1;                                            // Result = gViewSinDiv16 (80086530)
    v0 = 0x1000;                                        // Result = 00001000
    sh(0, s0 - 0x14);                                   // Store to: gViewSinDiv16 (80086530)
    sh(0, s0 - 0x12);                                   // Store to: gViewSinDiv16 + 2 (80086532) (80086532)
    sh(0, s0 - 0x10);                                   // Store to: gMinusViewCosDiv16 (80086534)
    sh(0, s0 - 0xE);                                    // Store to: gMinusViewCosDiv16 + 2 (80086536) (80086536)
    sh(v0, s0 - 0xC);                                   // Store to: 80086538
    sh(0, s0 - 0xA);                                    // Store to: 8008653A
    sh(0, s0 - 0x8);                                    // Store to: gViewCosDiv16 (8008653C)
    sh(0, s0 - 0x6);                                    // Store to: gViewCosDiv16 + 2 (8008653E) (8008653E)
    sh(0, s0 - 0x4);                                    // Store to: gViewSinDiv16_2 (80086540)
    LIBGTE_SetRotMatrix();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void R_RenderPlayerView() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x18);

    // If currently in fullbright mode (no lighting) then setup the light params now
    if (!*gbDoViewLighting) {
        *gCurLightValR = 128;
        *gCurLightValG = 128;
        *gCurLightValB = 128;
        
        v1 = lw(gp + 0xA88);                            // Load from: gpLightsLump (80078068)
        sw(v1, gp + 0xA74);                             // Store to: gpCurLightsLumpEntry (80078054)
    }

    // Setup trig related stuff before drawing
    v1 = *gCurPlayerIndex;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v1 += v0;
    v0 = lw(v1 + 0x14);
    a1 = lw(v1);
    a3 = 0xFFFF0000;                                    // Result = FFFF0000
    sw(v1, gp + 0x954);                                 // Store to: gpViewPlayer (80077F34)
    t0 = lw(a1 + 0x24);
    v0 &= a3;
    sw(v0, gp + 0x90C);                                 // Store to: gViewZ (80077EEC)
    v0 = lw(v1);
    a1 = t0 >> 19;
    a1 <<= 2;
    a2 += a1;
    v0 = lw(v0);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    a1 = lw(at);
    v0 &= a3;
    sw(v0, gp + 0x900);                                 // Store to: gViewX (80077EE0)
    v0 = lw(v1);
    v1 = lw(a2);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x6530;                                       // Result = gViewSinDiv16 (80086530)
    sw(t0, gp + 0xCB4);                                 // Store to: gViewAngle (80078294)
    sw(a1, gp + 0xAD8);                                 // Store to: gViewSin (800780B8)
    v0 = lw(v0 + 0x4);
    a1 = u32(i32(a1) >> 4);
    sw(v1, gp + 0xABC);                                 // Store to: gViewCos (8007809C)
    sh(a1, a0);                                         // Store to: gViewSinDiv16 (80086530)
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at + 0x6540);                                // Store to: gViewSinDiv16_2 (80086540)
    v0 &= a3;
    sw(v0, gp + 0x904);                                 // Store to: gViewY (80077EE4)
    v0 = -v1;
    v0 = u32(i32(v0) >> 4);
    v1 = u32(i32(v1) >> 4);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x6534);                                // Store to: gMinusViewCosDiv16 (80086534)
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at + 0x653C);                                // Store to: gViewCosDiv16 (8008653C)
    LIBGTE_SetRotMatrix();

    // Traverse the BSP tree to determine what needs to be drawn and in what order
    R_BSP();
    v0 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6E4C;                                       // Result = gpDrawSubsectors[0] (800A91B4)
    v0 -= s0;
    v0 = u32(i32(v0) >> 2);
    sw(v0, gp + 0xB0C);                                 // Store to: gNumDrawSubsectors (800780EC)

    // Finish up the previous draw before we continue
    I_DrawPresent();

    // Draw the sky if currently visible
    if (*gbIsSkyVisible) {
        R_DrawSky();
    }
    
    // Draw all subsectors emitted during BSP traversal.
    // Draw them in back to front order.
    v1 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    v0 = s0 - 4;                                        // Result = gCheatSequenceBtns[6] (800A91B0)
    v1 -= 4;
    v0 = (v0 < v1);
    sw(v1, gp + 0xA84);                                 // Store to: gppEndDrawSubsector (80078064)
    s0 = 0xFF;                                          // Result = 000000FF

    while (v0 != 0) {
        v0 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
        t0 = lw(v0);
        a1 = lw(t0);
        sw(a1, gp + 0xA2C);                                 // Store to: gpCurSector (8007800C)

        if (*gbDoViewLighting) {
            v1 = lh(a1 + 0x10);
            v0 = lw(gp + 0xA88);                            // Load from: gpLightsLump (80078068)
            a0 = lh(a1 + 0x12);
            v1 <<= 2;
            v1 += v0;
            sw(v1, gp + 0xA74);                             // Store to: gpCurLightsLumpEntry (80078054)
            v0 = lbu(v1);
            mult(a0, v0);
            a0 = lh(a1 + 0x12);
            v0 = lo;
            a3 = u32(i32(v0) >> 8);
            *gCurLightValR = a3;
            v0 = lbu(v1 + 0x1);
            mult(a0, v0);
            a0 = lh(a1 + 0x12);
            v0 = lo;
            a2 = u32(i32(v0) >> 8);
            *gCurLightValG = a2;
            v0 = lbu(v1 + 0x2);
            mult(a0, v0);
            a0 = lw(gp + 0x954);                            // Load from: gpViewPlayer (80077F34)
            v1 = lw(a0 + 0xE4);
            v0 = lo;
            a1 = u32(i32(v0) >> 8);
            *gCurLightValB = a1;
            v0 = a3 + v1;

            if (v1 != 0) {
                *gCurLightValR = v0;
                v1 = lw(a0 + 0xE4);
                a0 = lw(a0 + 0xE4);
                v1 += a2;
                a0 += a1;
                *gCurLightValG = v1;
                *gCurLightValB = a0;

                if (i32(v0) > 255) {
                    *gCurLightValR = 255;
                }

                if (i32(v1) > 255) {
                    *gCurLightValG = 255;
                }
                
                if (i32(a0) > 255) {
                    *gCurLightValB = 255;
                }
            }
        }
        
        a0 = t0;
        R_DrawSubsector();
        v1 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
        v0 = 0x800B0000;                                    // Result = 800B0000
        v0 -= 0x6E50;                                       // Result = gCheatSequenceBtns[6] (800A91B0)
        v1 -= 4;
        v0 = (v0 < v1);
        sw(v1, gp + 0xA84);                                 // Store to: gppEndDrawSubsector (80078064)
    }

    R_DrawWeapon();

    // Clearing the texture window: this is probably not required?
    // Not sure what the reason is for this, but everything seems to work fine without doing this.
    // I'll leave the code as-is for now though until I have a better understanding of this, just in case something breaks.
    a0 = 0x1F800200;
    a1 = sp + 0x10;
    sh(0, sp + 0x10);
    sh(0, sp + 0x12);
    sh(0, sp + 0x14);
    sh(0, sp + 0x16);
    LIBGPU_SetTexWindow();

    I_AddPrim(getScratchAddr(128));

loc_80030B44:
    s0 = lw(sp + 0x18);
    sp += 0x20;
}

void R_SlopeDiv() noexcept {
    v0 = (a1 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030B98;
    }
    v1 = a0 << 3;
    v0 = a1 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030B7C;
    _break(0x1C00);
loc_80030B7C:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80030B98;
    }
    v1 = 0x800;                                         // Result = 00000800
    v0 = v1;                                            // Result = 00000800
loc_80030B98:
    return;
}

void R_PointToAngle2() noexcept {
loc_80030BA0:
    a2 -= a0;
    a3 -= a1;
    if (a2 != 0) goto loc_80030BB4;
    v0 = 0;                                             // Result = 00000000
    if (a3 == 0) goto loc_80030EAC;
loc_80030BB4:
    if (i32(a2) < 0) goto loc_80030D30;
    v0 = (i32(a3) < i32(a2));
    if (i32(a3) < 0) goto loc_80030C68;
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030C24;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030C08;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030BEC;
    _break(0x1C00);
loc_80030BEC:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80030C08;
    }
    v1 = 0x800;                                         // Result = 00000800
    v0 = v1;                                            // Result = 00000800
loc_80030C08:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v0 = lw(at);
    goto loc_80030EAC;
loc_80030C24:
    v0 = (a3 < 0x200);
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030C60;
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030C48;
    _break(0x1C00);
loc_80030C48:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x3FFF0000;                                // Result = 3FFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030C60:
    v0 = 0x3FFF0000;                                    // Result = 3FFF0000
    goto loc_80030E90;
loc_80030C68:
    a3 = -a3;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030CD0;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030CB4;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030C98;
    _break(0x1C00);
loc_80030C98:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030CB0;
    v1 = 0x800;                                         // Result = 00000800
loc_80030CB0:
    v0 = v1;
loc_80030CB4:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v0 = lw(at);
    v0 = -v0;
    goto loc_80030EAC;
loc_80030CD0:
    v0 = (a3 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030D10;
    }
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030CF4;
    _break(0x1C00);
loc_80030CF4:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030D0C;
    v1 = 0x800;                                         // Result = 00000800
loc_80030D0C:
    v0 = v1;
loc_80030D10:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0xC0000000;                                    // Result = C0000000
    v0 += v1;
    goto loc_80030EAC;
loc_80030D30:
    a2 = -a2;
    if (i32(a3) < 0) goto loc_80030DE4;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030D84;
    }
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030D7C;
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030D64;
    _break(0x1C00);
loc_80030D64:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7FFF0000;                                // Result = 7FFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030D7C:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    goto loc_80030E90;
loc_80030D84:
    v0 = (a3 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030DC4;
    }
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030DA8;
    _break(0x1C00);
loc_80030DA8:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030DC0;
    v1 = 0x800;                                         // Result = 00000800
loc_80030DC0:
    v0 = v1;
loc_80030DC4:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0x40000000;                                    // Result = 40000000
    v0 += v1;
    goto loc_80030EAC;
loc_80030DE4:
    a3 = -a3;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030E50;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030E30;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030E14;
    _break(0x1C00);
loc_80030E14:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030E2C;
    v1 = 0x800;                                         // Result = 00000800
loc_80030E2C:
    v0 = v1;
loc_80030E30:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0x80000000;                                    // Result = 80000000
    v0 = v1 - v0;
    goto loc_80030EAC;
loc_80030E50:
    v0 = (a3 < 0x200);
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030E8C;
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030E74;
    _break(0x1C00);
loc_80030E74:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xBFFF0000;                                // Result = BFFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030E8C:
    v0 = 0xBFFF0000;                                    // Result = BFFF0000
loc_80030E90:
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v1;
    v1 = lw(at);
    v0 |= 0xFFFF;
    v0 -= v1;
loc_80030EAC:
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells what side of a line a point is on, with accuracy in terms of integer units.
// Returns '0' if the point is on the 'front' side of the line, otherwise '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_PointOnSide(const fixed_t x, const fixed_t y, const node_t& node) noexcept {
    // Special case shortcut for vertical lines
    if (node.dx == 0) {
        if (x <= node.x) {
            return (node.dy > 0);
        } else {
            return (node.dy < 0);
        }
    }

    // Special case shortcut for horizontal lines
    if (node.dy == 0) {
        if (y <= node.y) {
            return (node.dx < 0);
        } else {
            return (node.dx > 0);
        }
    }

    // Compute which side of the line the point is on using the cross product
    const fixed_t dx = x - node.x;
    const fixed_t dy = y - node.y;
    const int32_t lprod = (node.dy >> FRACBITS) * (dx >> FRACBITS);
    const int32_t rprod = (node.dx >> FRACBITS) * (dy >> FRACBITS);
    return (rprod >= lprod);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the subsector that a 2D point is in.
// Note: there should always be a subsector returned for a valid DOOM level.
//------------------------------------------------------------------------------------------------------------------------------------------
subsector_t* R_PointInSubsector(const fixed_t x, const fixed_t y) noexcept {
    // Not sure why there would ever be '0' BSP nodes - that does not seem like a valid DOOM level to me?
    // The same logic can also be found in other versions of DOOM...
    if (*gNumBspNodes == 0) {
        return gpSubsectors->get();
    }
    
    // Traverse the BSP tree starting at the root node, using the given position to decide which half-spaces to visit.
    // Once we reach a subsector stop and return it.
    int32_t nodeNum = *gNumBspNodes - 1;
    
    while ((nodeNum & NF_SUBSECTOR) == 0) {
        node_t& node = (*gpBspNodes)[nodeNum];
        const int32_t side = R_PointOnSide(x, y, node);
        nodeNum = node.children[side];
    }

    const int32_t actualNodeNum = nodeNum & (~NF_SUBSECTOR);
    return &(*gpSubsectors)[actualNodeNum];
}

void _thunk_R_PointInSubsector() noexcept {
    v0 = ptrToVmAddr(R_PointInSubsector(a0, a1));
}

#include "r_things.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/info.h"
#include "PcPsx/Finally.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

void R_DrawSubsectorSprites() noexcept {
    sp -= 0x58;
    sw(s5, sp + 0x4C);
    sw(s6, sp + 0x50);
    sw(s4, sp + 0x48);
    sw(s3, sp + 0x44);
    sw(s2, sp + 0x40);
    sw(s1, sp + 0x3C);
    sw(s0, sp + 0x38);

    auto stackCleanup = finally([]() {
        s6 = lw(sp + 0x50);
        s5 = lw(sp + 0x4C);
        s4 = lw(sp + 0x48);
        s3 = lw(sp + 0x44);
        s2 = lw(sp + 0x40);
        s1 = lw(sp + 0x3C);
        s0 = lw(sp + 0x38);
        sp += 0x58;        
    });

    s5 = a0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6D78;                                       // Result = 80096D78
    v1 = v0 - 0xC;                                      // Result = 80096D6C
    
    sw(v1, v0);                                         // Store to: 80096D78
    v0 = lw(s5);
    s1 = lw(v0 + 0x4C);
    s3 = 0x800B0000;                                    // Result = 800B0000
    s3 -= 0x7584;                                       // Result = 800A8A7C
    s4 = 0;                                             // Result = 00000000
    s0 = s3 + 0xC;                                      // Result = 800A8A88

    for (s1 = lw(v0 + 0x4C); s1 != 0; s1 = lw(s1 + 0x1C)) {
        v0 = lw(s1 + 0xC);
        a2 = sp + 0x30;

        if (v0 != s5)
            continue;

        a0 = sp + 0x10;
        v0 = lw(s1);
        v1 = *gViewX;
        a1 = sp + 0x18;
        sh(0, sp + 0x12);
        v0 -= v1;
        v0 = u32(i32(v0) >> 16);
        sh(v0, sp + 0x10);
        v0 = lw(s1 + 0x4);
        v1 = *gViewY;
        v0 -= v1;
        v0 = u32(i32(v0) >> 16);
        sh(v0, sp + 0x14);
        _thunk_LIBGTE_RotTrans();
        v1 = lw(sp + 0x20);

        if (i32(v1) < 4)
            continue;

        a0 = v1 << 1;
        v1 = lw(sp + 0x18);
        
        if (i32(v1) < i32(-a0))
            continue;

        if (i32(a0) < i32(v1))
            continue;

        v0 = 0x800000;                                      // Result = 00800000
        sw(v1, s3);                                         // Store to: 800A8A7C
        v1 = lw(sp + 0x20);
        div(v0, v1);
        v0 = lo;
        sw(s1, s0 - 0x4);                                   // Store to: 800A8A84
        sw(v0, s0 - 0x8);                                   // Store to: 800A8A80
        v0 = 0x80090000;                                    // Result = 80090000
        v0 = lw(v0 + 0x6D78);                               // Load from: 80096D78
        s2 = 0x80090000;                                    // Result = 80090000
        s2 += 0x6D6C;                                       // Result = 80096D6C
        a0 = lw(s0 - 0x8);                                  // Load from: 800A8A80
        a1 = s2;                                            // Result = 80096D6C

        while (v0 != 0x80096D6C) {
            v1 = lw(s2 + 0xC);
            v0 = lw(v1 + 0x4);

            if (i32(v0) >= i32(a0))
                break;

            v0 = lw(v1 + 0xC);
            s2 = v1;
        }

        v0 = lw(s2 + 0xC);
        s4++;                                               // Result = 00000001
        sw(v0, s0);                                         // Store to: 800A8A88
        s0 += 0x10;                                         // Result = 800A8A98
        sw(s3, s2 + 0xC);
        v0 = (i32(s4) < 0x40);                              // Result = 00000001
        s3 += 0x10;                                         // Result = 800A8A8C

        if (v0 == 0)
            break;
    }

    a1 = sp + 0x28;

    if (s4 == 0)
        return;

    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    sh(0, sp + 0x28);
    sh(0, sp + 0x2A);
    sh(0, sp + 0x2C);
    sh(0, sp + 0x2E);
    _thunk_LIBGPU_SetTexWindow();
    s0 += 4;                                            // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;

    I_AddPrim(getScratchAddr(128));
	
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6D78;                                       // Result = 80096D78
    s2 = lw(v1);                                        // Load from: 80096D78
    
    v0 = 9;                                             // Result = 00000009
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    
    v0 = 0x2C;                                          // Result = 0000002C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207

    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lhu(a0 + 0x7F7C);                              // Load from: g3dViewPaletteClutId (80077F7C)
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a0, at + 0x20E);                                 // Store to: 1F80020E

    s4 = 0xFF0000;                                      // Result = 00FF0000
    s6 = 0x1F800000;                                    // Result = 1F800000
    s6 += 0x200;                                        // Result = 1F800200
    s5 = 0x80080000;                                    // Result = 80080000
    s5 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s4 |= 0xFFFF;                                       // Result = 00FFFFFF

    while (s2 != 0x80096D6C) {
        s1 = lw(s2 + 0x8);
        a0 = lw(s1 + 0x28);
        v1 = lw(s1 + 0x2C);
        a0 <<= 3;
        v1 &= 0x7FFF;
        v0 = v1 << 1;
        v0 += v1;
        v0 <<= 2;
        v0 -= v1;
        at = 0x80060000;                                    // Result = 80060000
        at += 0x6BFC;                                       // Result = 80066BFC
        at += a0;
        v1 = lw(at);
        v0 <<= 2;
        s0 = v0 + v1;
        v0 = lw(s0);

        if (v0 != 0) {
            a0 = *gViewX;
            a1 = *gViewY;
            a2 = lw(s1);
            a3 = lw(s1 + 0x4);
            R_PointToAngle2();
            v1 = lw(s1 + 0x24);
            v0 -= v1;
            v1 = 0x90000000;                                    // Result = 90000000
            v0 += v1;
            v0 >>= 29;
            v1 = v0 << 2;
            v1 += s0;
            v0 += s0;
            v1 = lw(v1 + 0x4);
            s3 = lbu(v0 + 0x24);
        } else {
            v1 = lw(s0 + 0x4);
            s3 = lbu(s0 + 0x24);
        }

        v0 = *gFirstSpriteLumpNum;
        v0 = v1 - v0;
        v1 = *gpSpriteTextures;
        v0 <<= 5;
        s0 = v0 + v1;
        a0 = s0;
        I_CacheTex();
        v0 = lw(s1 + 0x64);
        v0 >>= 28;
        v1 = v0 & 7;

        if (v1 != 0) {
            v0 = 0x1F800000;                                    // Result = 1F800000
            v0 = lbu(v0 + 0x207);                               // Load from: 1F800207
            v0 |= 2;
        } else {
            v0 = 0x1F800000;                                    // Result = 1F800000
            v0 = lbu(v0 + 0x207);                               // Load from: 1F800207
            v0 &= 0xFD;
        }

        at = 0x1F800000;                                    // Result = 1F800000
        sb(v0, at + 0x207);                                 // Store to: 1F800207
        v0 = u32(i32(v1) >> 1);
        v1 = lhu(s0 + 0xA);
        v0 <<= 5;
        v1 |= v0;
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v1, at + 0x216);                                 // Store to: 1F800216
        v0 = 0xA0;

        if ((lw(s1 + 0x2C) & 0x8000) != 0) {
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x204);                                 // Store to: 1F800204
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x205);                                 // Store to: 1F800205
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x206);                                 // Store to: 1F800206
        } else {
            v0 = (uint8_t) *gCurLightValR;
            v1 = (uint8_t) *gCurLightValG;
            a0 = (uint8_t) *gCurLightValB;
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x204);                                 // Store to: 1F800204
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v1, at + 0x205);                                 // Store to: 1F800205
            at = 0x1F800000;                                    // Result = 1F800000
            sb(a0, at + 0x206);                                 // Store to: 1F800206
        }

        a1 = lw(s2 + 0x4);
        v0 = lw(s1 + 0x8);
        v1 = *gViewZ;
        a0 = -a1;
        v0 -= v1;
        v1 = lh(s0 + 0x2);
        v0 = u32(i32(v0) >> 16);
        v0 += v1;
        mult(a0, v0);
        v1 = lh(s0 + 0x4);
        v0 = v1 << 1;
        v0 += v1;
        v1 = v0 << 4;
        v0 += v1;
        v1 = v0 << 8;
        v0 += v1;
        v1 = lo;
        v0 <<= 2;
        v0 = u32(i32(v0) >> 16);
        mult(v0, a1);
        a0 = lo;
        v0 = lh(s0 + 0x6);
        mult(v0, a1);
        v1 = u32(i32(v1) >> 16);
        a2 = v1 + 0x64;
        a3 = u32(i32(a0) >> 16);
        v0 = lo;
        t1 = u32(i32(v0) >> 16);

        if (s3 == 0) {
            v0 = lbu(s0 + 0x8);
            v1 = lh(s0);
            a0 = lw(s2);
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x20C);                                 // Store to: 1F80020C
            v0 = v1 << 1;
            v0 += v1;
            v1 = v0 << 4;
            v0 += v1;
            v1 = v0 << 8;
            v0 += v1;
            v0 <<= 2;
            v0 = u32(i32(v0) >> 16);
            a0 -= v0;
            v0 = lbu(s0 + 0x8);
            v1 = lbu(s0 + 0x4);
            mult(a0, a1);
            v0 += v1;
            v0--;
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x214);                                 // Store to: 1F800214
            v0 = lbu(s0 + 0x8);
            v1 = lbu(s0 + 0x4);
            v0 += v1;
            v0--;
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x224);                                 // Store to: 1F800224
            v0 = lbu(s0 + 0x8);
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x21C);                                 // Store to: 1F80021C
            v0 = lo;
            v0 = u32(i32(v0) >> 16);
            v1 = v0 + 0x80;
        } else {
            v1 = lh(s0);
            a0 = lbu(s0 + 0x4);
            v0 = v1 << 1;
            v0 += v1;
            v1 = v0 << 4;
            v0 += v1;
            v1 = v0 << 8;
            v0 += v1;
            v0 <<= 2;
            v1 = lw(s2);
            v0 = u32(i32(v0) >> 16);
            v0 += v1;
            v1 = lbu(s0 + 0x8);
            mult(v0, a1);
            v1 += a0;
            v1--;
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v1, at + 0x20C);                                 // Store to: 1F80020C
            v0 = lbu(s0 + 0x8);
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x214);                                 // Store to: 1F800214
            v0 = lbu(s0 + 0x8);
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x224);                                 // Store to: 1F800224
            v0 = lbu(s0 + 0x8);
            v1 = lbu(s0 + 0x4);
            v0 += v1;
            v0--;
            v1 = a3 - 0x80;
            at = 0x1F800000;                                    // Result = 1F800000
            sb(v0, at + 0x21C);                                 // Store to: 1F80021C
            v0 = lo;
            v0 = u32(i32(v0) >> 16);
            v1 = v0 - v1;
        }

        t2 = s6 + 4;                                        // Result = 1F800204
        t4 = s5 & s4;                                       // Result = 00086550
        t7 = 0x4000000;                                     // Result = 04000000
        t0 = 0x1F800000;                                    // Result = 1F800000
        t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
        v0 = v1 + a3;
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v0, at + 0x210);                                 // Store to: 1F800210
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v0, at + 0x220);                                 // Store to: 1F800220
        v0 = a2 + t1;
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v1, at + 0x208);                                 // Store to: 1F800208
        at = 0x1F800000;                                    // Result = 1F800000
        sh(a2, at + 0x20A);                                 // Store to: 1F80020A
        at = 0x1F800000;                                    // Result = 1F800000
        sh(a2, at + 0x212);                                 // Store to: 1F800212
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v0, at + 0x222);                                 // Store to: 1F800222
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v1, at + 0x218);                                 // Store to: 1F800218
        at = 0x1F800000;                                    // Result = 1F800000
        sh(v0, at + 0x21A);                                 // Store to: 1F80021A
        v0 = lbu(s0 + 0x9);
        a2 = 0x80070000;                                    // Result = 80070000
        a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
        t6 = 0x80000000;                                    // Result = 80000000
        at = 0x1F800000;                                    // Result = 1F800000
        sb(v0, at + 0x20D);                                 // Store to: 1F80020D
        v0 = lbu(s0 + 0x9);
        t5 = -1;                                            // Result = FFFFFFFF
        at = 0x1F800000;                                    // Result = 1F800000
        sb(v0, at + 0x215);                                 // Store to: 1F800215
        v0 = lbu(s0 + 0x9);
        v1 = lbu(s0 + 0x6);
        t1 = t0 << 2;
        v0 += v1;
        v0--;
        at = 0x1F800000;                                    // Result = 1F800000
        sb(v0, at + 0x225);                                 // Store to: 1F800225
        v0 = lbu(s0 + 0x9);
        v1 = lbu(s0 + 0x6);
        t3 = t1 + 4;
        v0 += v1;
        v0--;
        at = 0x1F800000;                                    // Result = 1F800000
        sb(v0, at + 0x21D);                                 // Store to: 1F80021D
	
        I_AddPrim(getScratchAddr(128));	
        s2 = lw(s2 + 0xC);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the player's weapon sprites and muzzle flash
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawWeapon() noexcept {
    // Run through all of the player sprites for the view player and render
    player_t& player = **gpViewPlayer;
    pspdef_t* pSprite = player.psprites;
    
    for (int32_t pspIdx = 0; pspIdx < NUMPSPRITES; ++pspIdx, ++pSprite) {
        // Is this particular player sprite slot showing anything?
        if (!pSprite->state)
            continue;

        // Get the texture for the sprite
        const state_t& state = *pSprite->state;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const int32_t frameNum = state.frame & FF_FRAMEMASK;
        const spriteframe_t& frame = spriteDef.spriteframes[frameNum];
        const int32_t texNum = frame.lump[0] - *gFirstSpriteLumpNum;
        texture_t& tex = (*gpSpriteTextures)[texNum];

        // Load the texture to VRAM if required
        a0 = ptrToVmAddr(&tex);
        I_CacheTex();

        // Setup the default drawing mode to disable wrapping (remove texture window).
        // This code also sets blending options and encodes that in the texture page id.
        uint16_t transparencyFlags = 0;

        {
            RECT texWin = { 0, 0, 0, 0 };
            const bool bIsTransparent = ((player.mo->flags & MF_ALL_BLEND_MASKS) != 0);

            if (bIsTransparent) {
                transparencyFlags |= 0x20;  // TODO: make constants for PSYQ transparency flags
            }

            const uint16_t texPageId = tex.texPageId | transparencyFlags;

            DR_MODE& drawMode = *(DR_MODE*) getScratchAddr(128);
            LIBGPU_SetDrawMode(drawMode, false, false, texPageId, &texWin);
            I_AddPrim(&drawMode);
        }

        // Setup the sprite to be drawn and submit the drawing primitive.
        // Set transparency, size, UV coords, palette and color before drawing.
        SPRT& spr = *(SPRT*) getScratchAddr(128);
        LIBGPU_SetSprt(spr);

        if (transparencyFlags != 0) {
            LIBGPU_SetSemiTrans(&spr, true);
        }

        LIBGPU_setXY0(spr,
            (int16_t)((pSprite->sx >> FRACBITS) + HALF_SCREEN_W - tex.offsetX),
            (int16_t)((pSprite->sy >> FRACBITS) + VIEW_3D_H - 1 - tex.offsetY)
        );

        LIBGPU_setWH(spr, tex.width, tex.height);
        LIBGPU_setUV0(spr, tex.texPageCoordX, tex.texPageCoordY);
        spr.clut = *g3dViewPaletteClutId;

        if (state.frame & FF_FULLBRIGHT) {
            // Note: these magic 5/8 multipliers correspond VERY closely to 'LIGHT_INTENSTIY_MAX / 255'.
            // The resulting values are sometimes not quite the same however.
            const light_t& light = **gpCurLight;            
            LIBGPU_setRGB0(spr,
                (uint8_t)(((uint32_t) light.r * 5) / 8),
                (uint8_t)(((uint32_t) light.g * 5) / 8),
                (uint8_t)(((uint32_t) light.b * 5) / 8)
            );
        } else {
            LIBGPU_setRGB0(spr,
                (uint8_t) *gCurLightValR,
                (uint8_t) *gCurLightValG,
                (uint8_t) *gCurLightValB
            );
        }
	    
        I_AddPrim(&spr);
    }
}

#include "p_password.h"

#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "g_game.h"
#include "p_inter.h"
#include "PsxVm/PsxVm.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// PC-PSX helper to make the encoding logic byte bit cleaner and remove some redundancy.
// Divide 'byte' by 'b' and do byte 'ceil' operation on the potentially non integer result.
// Returns the answer as an unsigned 8-bit integer.
//------------------------------------------------------------------------------------------------------------------------------------------
static inline uint8_t ceil8Div(const int32_t num, const int32_t den) noexcept {
    const int32_t result = num / den + ((num % den != 0) ? 1 : 0);
    return (uint8_t) result;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Compute a password for the current player and save in the given output buffer.
// Each byte in the output buffer contains a 5-bit number from 0-31, which corresponds to one of the allowed password chars/digits.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ComputePassword(uint8_t pOutput[10]) noexcept {
    // Get the player to encode the password for and zero init the unencrypted password
    player_t& player = gPlayers[*gCurPlayerIndex];

    uint8_t unencrypted[8];
    D_memset(unencrypted, std::byte(0), 8);

    // Encode byte: current map and skill
    unencrypted[0] = (uint8_t)((*gNextMap & 63) << 2);
    unencrypted[0] |= (uint8_t)(*gGameSkill & 3);

    // Encode byte: owned weapons (from shotgun onwards) and whether the backpack is owned
    for (int32_t i = wp_shotgun; i < NUMWEAPONS; ++i) {
        if (player.weaponowned[i]) {
            unencrypted[1] |= (uint8_t)(1 << (i - wp_shotgun));
        }
    }

    unencrypted[1] |= (player.backpack) ? 0x80 : 0;

    // Determine the maximum ammo amount for the calculations below
    const uint8_t maxAmmoShift = (player.backpack) ? 1 : 0;

    const int32_t maxClips = gMaxAmmo[am_clip] << maxAmmoShift;
    const int32_t maxShells = gMaxAmmo[am_shell] << maxAmmoShift;
    const int32_t maxCells = gMaxAmmo[am_cell] << maxAmmoShift;
    const int32_t maxMissiles = gMaxAmmo[am_misl] << maxAmmoShift;

    // Encode byte: number of bullets and shells (in 1/8 of the maximum increments, rounded up)
    const uint8_t clipsEnc = ceil8Div(player.ammo[am_clip] << 3, maxClips);
    const uint8_t shellsEnc = ceil8Div(player.ammo[am_shell] << 3, maxShells);
    unencrypted[2] = (clipsEnc << 4) | shellsEnc;

    // Encode byte: number of cells and missiles (in 1/8 of the maximum increments, rounded up)
    const uint8_t cellsEnc = ceil8Div(player.ammo[am_cell] << 3, maxCells);
    const uint8_t missilesEnc = ceil8Div(player.ammo[am_misl] << 3, maxMissiles);
    unencrypted[3] = (cellsEnc << 4) | missilesEnc;

    // Encode byte: health and armor points (in 1/8 of the maximum increments (25 HP), rounded up)
    const uint8_t healthPointsEnc = ceil8Div(player.health << 3, 200);
    const uint8_t armorPointsEnc = ceil8Div(player.armorpoints << 3, 200);
    unencrypted[4] = (healthPointsEnc << 4) | armorPointsEnc;

    // Encode byte: armor type
    unencrypted[5] = (uint8_t)(player.armortype << 3);

    // Convert the the regular 8-bit bytes that we just encoded to 5-bit bytes which can encode 32 values.
    // This is so we can use ASCII characters and numbers to encode the data.
    // This expands the size of the password from 6 bytes to 9 bytes, as we need to encode 45 bits.
    constexpr int32_t BITS_TO_ENCODE = 45;

    for (int32_t srcBitIdx = 0; srcBitIdx < BITS_TO_ENCODE;) {
        // Encode 5 source bits and save the 5-bit byte
        uint8_t dstByte = 0;

        for (int32_t bitInDstByte = 4; bitInDstByte >= 0; --bitInDstByte, ++srcBitIdx) {
            const uint8_t srcByte = unencrypted[srcBitIdx / 8];
            const uint8_t srcBitMask = (uint8_t)(0x80u >> (srcBitIdx & 7));
            const uint8_t dstBitMask = (uint8_t)(1u << bitInDstByte);

            if (srcByte & srcBitMask) {
                dstByte |= dstBitMask;      // Encode the source bit when set
            }
        }

        const int32_t dstByteIdx = (srcBitIdx - 1) / 5;     // -1 because we are now on the next dest byte
        pOutput[dstByteIdx] = dstByte;
    }

    // Simple encryption: build an XOR bitmask to apply to the 9 data bytes, from XOR-ing all 9 data bytes.
    // This 5-bit XOR pattern is stored in the last output byte of the password.
    pOutput[9] = 0;

    for (int32_t i = 0; i < 9; ++i) {
        pOutput[9] ^= pOutput[i];
    }

    // Now apply the XOR pattern to encrypt the 9 data bytes
    for (int32_t i = 0; i < 9; ++i) {
        pOutput[i] ^= pOutput[9];
    }
}

void P_ProcessPassword() noexcept {
loc_800381B0:
    sp -= 0x40;
    v0 = a0;
    sw(s2, sp + 0x30);
    s2 = a1;
    sw(s3, sp + 0x34);
    s3 = a2;
    sw(s1, sp + 0x2C);
    s1 = a3;
    sw(s0, sp + 0x28);
    s0 = sp + 0x18;
    a0 = s0;
    a1 = v0;
    sw(ra, sp + 0x38);
    a2 = 0xA;                                           // Result = 0000000A
    _thunk_D_memcpy();
    a0 = sp + 0x21;
loc_800381F0:
    v0 = lbu(s0);
    v1 = lbu(sp + 0x21);
    v0 ^= v1;
    sb(v0, s0);
    s0++;
    v0 = (i32(s0) < i32(a0));
    v1 = sp + 0x18;
    if (v0 != 0) goto loc_800381F0;
    a0 = 0;                                             // Result = 00000000
    a1 = sp + 0x21;
loc_8003821C:
    v0 = lbu(v1);
    v1++;
    a0 ^= v0;
    v0 = (i32(v1) < i32(a1));
    if (v0 != 0) goto loc_8003821C;
    v0 = lbu(sp + 0x21);
    {
        const bool bJump = (a0 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    a1 = 0;                                             // Result = 00000000
    t2 = 0x66660000;                                    // Result = 66660000
    t2 |= 0x6667;                                       // Result = 66666667
    t1 = sp + 0x18;
    t3 = 0x10;                                          // Result = 00000010
loc_80038258:
    t0 = 0;                                             // Result = 00000000
    a3 = 0x80;                                          // Result = 00000080
    a2 = 7;                                             // Result = 00000007
loc_80038264:
    mult(a1, t2);
    v0 = u32(i32(a1) >> 31);
    v1 = hi;
    v1 = u32(i32(v1) >> 1);
    v1 -= v0;
    v0 = t1 + v1;
    a0 = lbu(v0);
    v0 = v1 << 2;
    v0 += v1;
    v0 = a1 - v0;
    v0 = i32(t3) >> v0;
    a0 &= v0;
    a1++;
    if (a0 == 0) goto loc_800382A0;
    t0 |= a3;
loc_800382A0:
    a2--;
    a3 = u32(i32(a3) >> 1);
    if (i32(a2) >= 0) goto loc_80038264;
    v0 = a1 - 1;
    v1 = sp + 0x10;
    if (i32(v0) >= 0) goto loc_800382BC;
    v0 = a1 + 6;
loc_800382BC:
    v0 = u32(i32(v0) >> 3);
    v1 += v0;
    v0 = (i32(a1) < 0x30);
    sb(t0, v1);
    if (v0 != 0) goto loc_80038258;
    v0 = lbu(sp + 0x10);
    v0 >>= 2;
    sw(v0, s2);
    if (v0 == 0) goto loc_800383B0;
    v0 = (i32(v0) < 0x3C);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x10);
    v0 &= 3;
    sw(v0, s3);
    v0 = lbu(sp + 0x12);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x12);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x14);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x14);
    v1 = v0 >> 4;
    v0 = (v1 < 9);
    if (v0 == 0) goto loc_800383B0;
    v0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80038538;
    v0 = lbu(sp + 0x15);
    v0 >>= 3;
    v0 = (v0 < 3);
    if (v0 != 0) goto loc_800383B8;
loc_800383B0:
    v0 = 0;                                             // Result = 00000000
    goto loc_80038538;
loc_800383B8:
    a1 = 0;                                             // Result = 00000000
    if (s1 != 0) goto loc_800383C8;
    v0 = 1;                                             // Result = 00000001
    goto loc_80038538;
loc_800383C8:
    a0 = 1;                                             // Result = 00000001
    v1 = s1;
loc_800383D0:
    v0 = lbu(sp + 0x11);
    v0 = i32(v0) >> a1;
    v0 &= 1;
    a1++;
    if (v0 == 0) goto loc_800383EC;
    sw(a0, v1 + 0x7C);
loc_800383EC:
    v0 = (i32(a1) < 7);
    v1 += 4;
    if (v0 != 0) goto loc_800383D0;
    v0 = lbu(sp + 0x11);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80038444;
    v0 = lw(s1 + 0x60);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80038444;
    }
    sw(v0, s1 + 0x60);
    a1 = 0;                                             // Result = 00000000
    v1 = s1;
loc_80038428:
    v0 = lw(v1 + 0xA8);
    a1++;
    v0 <<= 1;
    sw(v0, v1 + 0xA8);
    v0 = (i32(a1) < 4);
    v1 += 4;
    if (v0 != 0) goto loc_80038428;
loc_80038444:
    v0 = lbu(sp + 0x12);
    v1 = lw(s1 + 0xA8);
    v0 >>= 4;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80038464;
    v0 += 7;
loc_80038464:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0x98);
    v0 = lbu(sp + 0x12);
    v1 = lw(s1 + 0xAC);
    v0 &= 0xF;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8003848C;
    v0 += 7;
loc_8003848C:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0x9C);
    v0 = lbu(sp + 0x13);
    v1 = lw(s1 + 0xB0);
    v0 >>= 4;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_800384B4;
    v0 += 7;
loc_800384B4:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0xA0);
    v0 = lbu(sp + 0x13);
    v1 = lw(s1 + 0xB4);
    v0 &= 0xF;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_800384DC;
    v0 += 7;
loc_800384DC:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0xA4);
    v1 = lbu(sp + 0x14);
    a1 = lw(s1);
    v1 >>= 4;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += v1;
    sw(v0, s1 + 0x24);
    a0 = lbu(sp + 0x14);
    v0 = 1;                                             // Result = 00000001
    a0 &= 0xF;
    v1 = a0 << 1;
    v1 += a0;
    v1 <<= 3;
    v1 += a0;
    sw(v1, s1 + 0x28);
    v1 = lbu(sp + 0x15);
    a0 = lw(s1 + 0x24);
    v1 >>= 3;
    sw(v1, s1 + 0x2C);
    sw(a0, a1 + 0x68);
loc_80038538:
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}

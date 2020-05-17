#include "p_password.h"

#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "g_game.h"
#include "p_inter.h"

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

    uint8_t pwdata[8];
    D_memset(pwdata, std::byte(0), 8);

    // Encode byte: current map and skill
    pwdata[0] = (uint8_t)((*gNextMap & 63) << 2);
    pwdata[0] |= (uint8_t)(*gGameSkill & 3);

    // Encode byte: owned weapons (from shotgun onwards) and whether the backpack is owned
    for (int32_t i = wp_shotgun; i < NUMWEAPONS; ++i) {
        if (player.weaponowned[i]) {
            pwdata[1] |= (uint8_t)(1 << (i - wp_shotgun));
        }
    }

    pwdata[1] |= (player.backpack) ? 0x80 : 0;

    // Determine the maximum ammo amount for the calculations below
    const uint8_t maxAmmoShift = (player.backpack) ? 1 : 0;

    const int32_t maxClips = gMaxAmmo[am_clip] << maxAmmoShift;
    const int32_t maxShells = gMaxAmmo[am_shell] << maxAmmoShift;
    const int32_t maxCells = gMaxAmmo[am_cell] << maxAmmoShift;
    const int32_t maxMissiles = gMaxAmmo[am_misl] << maxAmmoShift;

    // Encode byte: number of bullets and shells (in 1/8 of the maximum increments, rounded up)
    const uint8_t clipsEnc = ceil8Div(player.ammo[am_clip] << 3, maxClips);
    const uint8_t shellsEnc = ceil8Div(player.ammo[am_shell] << 3, maxShells);
    pwdata[2] = (clipsEnc << 4) | shellsEnc;

    // Encode byte: number of cells and missiles (in 1/8 of the maximum increments, rounded up)
    const uint8_t cellsEnc = ceil8Div(player.ammo[am_cell] << 3, maxCells);
    const uint8_t missilesEnc = ceil8Div(player.ammo[am_misl] << 3, maxMissiles);
    pwdata[3] = (cellsEnc << 4) | missilesEnc;

    // Encode byte: health and armor points (in 1/8 of the maximum increments (25 HP), rounded up)
    const uint8_t healthPointsEnc = ceil8Div(player.health << 3, 200);
    const uint8_t armorPointsEnc = ceil8Div(player.armorpoints << 3, 200);
    pwdata[4] = (healthPointsEnc << 4) | armorPointsEnc;

    // Encode byte: armor type
    pwdata[5] = (uint8_t)(player.armortype << 3);

    // PC-PSX: encode if the game is operating in nightmare mode in the top bit of the last unencrypted byte.
    // This change is compatible with a similar change in 'PSXDOOM-RE' so passwords should be compatible beween both projects.
    //
    // Note: only the top 5 bits of the last unencrypted byte are encoded to a password. 2 bits are used by armortype, and now
    // 1 extra bit is used by nightmare mode. Therefore there are still 2 bits left for over purposes, perhaps extended level support?
    #if PC_PSX_DOOM_MODS
        if (*gGameSkill == sk_nightmare) {
            pwdata[5] |= 0x80;
        }
    #endif

    // Convert the the regular 8-bit bytes that we just encoded to 5-bit bytes which can encode 32 values.
    // This is so we can use ASCII characters and numbers to encode the data.
    // This expands the size of the password from 6 bytes to 9 bytes, as we need to encode 45 bits.
    constexpr int32_t BITS_TO_ENCODE = 45;

    for (int32_t srcBitIdx = 0; srcBitIdx < BITS_TO_ENCODE;) {
        // Encode 5 source bits and save the 5-bit byte
        uint8_t dstByte = 0;

        for (int32_t bitInDstByte = 4; bitInDstByte >= 0; --bitInDstByte, ++srcBitIdx) {
            const uint8_t srcByte = pwdata[srcBitIdx / 8];
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Processes the given password and returns 'true' if the password is valid and should be accepted.
// The map number and skill to use are saved in the given output variables.
// If a player object is specified then the stats (weapons, ammo etc.) in the password are applied to the player.
//
// Using a password from the main menu is a 2 step process in PSX DOOM:
//  (1) We first use the password to warp to a particular map on a certain skill level, not applying it to a player because there is none yet.
//  (2) We then apply the same password again, to the player object, to get weapons, ammo and health etc. from the password.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ProcessPassword(const uint8_t pPasswordIn[10], int32_t& mapNumOut, skill_t& skillOut, player_t* const pPlayer) noexcept {
    // Copy the password to a local buffer for decryption
    uint8_t encrypted[10];
    D_memcpy(encrypted, pPasswordIn, 10);

    // Undo the XOR encryption of the first 9 password (5-bit) bytes by the last password (5-bit) byte
    for (int32_t i = 0; i < 9; ++i) {
        encrypted[i] ^= encrypted[9];
    }

    // Figure out what the expected XOR mask would be based on the encoded password data.
    // Use this to verify password validity:
    uint8_t expectedXorMask = 0;

    for (int32_t i = 0; i < 9; ++i) {
        expectedXorMask ^= encrypted[i];
    }

    if (expectedXorMask != encrypted[9])
        return false;
    
    // Convert the 9 (5-bit) bytes back out to 6 8-bit bytes.
    constexpr int32_t NUM_OUTPUT_BITS = 48;
    uint8_t pwdata[8];
    
    for (int32_t dstBitIdx = 0; dstBitIdx < NUM_OUTPUT_BITS;) {
        uint8_t dstByte = 0;

        // Harmless bug: eagle eyed readers may notice that this happens to decrypt at the end (accidentally) 3 bits from 'encrypted[9]' due
        // to decoding in groups of 8 bits. This byte is not actual valid payload data, as it contains the XOR mask used for encryption.
        // These 3 invalid bits are not used however, so it doesn't matter in practice.
        for (int32_t bitInDstByte = 7; bitInDstByte >= 0; --bitInDstByte, ++dstBitIdx) {
            const uint8_t srcByte = encrypted[dstBitIdx / 5];
            const uint8_t srcBitMask = (uint8_t)(0x10u >> dstBitIdx % 5u);
            const uint8_t dstBitMask = (uint8_t)(1u << bitInDstByte);

            if (srcByte & srcBitMask) {
                dstByte |= dstBitMask;      // Decode the source bit when set
            }
        }

        const int32_t dstByteIdx = (dstBitIdx - 1) / 8;     // -1 because we are now on the next dest byte
        pwdata[dstByteIdx] = dstByte;
    }
    
    // Decode byte: current map and skill
    const int32_t mapNum = pwdata[0] >> 2;
    mapNumOut = mapNum;

    if ((mapNum == 0) || (mapNum > 59))
        return false;

    skillOut = (skill_t)(pwdata[0] & 3);

    #if PC_PSX_DOOM_MODS
        // PC-PSX: support the nightmare skill level in passwords!
        if (pwdata[5] & 0x80) {
            skillOut = sk_nightmare;
        }
    #endif

    // Decode and verify byte: number of bullets and shells (in 1/8 of the maximum increments)
    const int32_t clipsEnc = pwdata[2] >> 4;
    const int32_t shellsEnc = pwdata[2] & 0xF;

    if ((clipsEnc > 8) || (shellsEnc > 8))
        return false;

    // Decode and verify byte: number of cells and missiles (in 1/8 of the maximum increments)
    const int32_t cellsEnc = pwdata[3] >> 4;
    const int32_t missilesEnc = pwdata[3] & 0xF;

    if ((cellsEnc > 8) || (missilesEnc > 8))
        return false;
    
    // Decode and verify byte: health and armor points (in 1/8 of the maximum increments (25 HP))
    const int32_t healthPointsEnc = pwdata[4] >> 4;
    const int32_t armorPointsEnc = pwdata[4] & 0xF;

    if ((healthPointsEnc > 8) || (healthPointsEnc == 0) || (armorPointsEnc > 8))    // Note: '0' health (dead) is not a valid password!
        return false;
    
    // Decode byte: armor type
    #if PC_PSX_DOOM_MODS
        const int32_t armorType = (pwdata[5] >> 3) & 0x3;  // PC-PSX: added a mask operation here on account of the 'nightmare' flag now being in the top bit
    #else
        const int32_t armorType = pwdata[5] >> 3;
    #endif

    if (armorType > 2)
        return false;

    // At this point we have confirmed the password is valid.
    // If we don't have a player to apply the password to yet, then we can stop.
    if (!pPlayer)
        return true;

    // Apply: owned weapons
    for (int32_t i = 0; i < 7; ++i) {
        if ((pwdata[1] >> i) & 1) {
            pPlayer->weaponowned[wp_shotgun + i] = true;
        }
    }

    // Apply: backpack (if in password and not already owned)
    if ((pwdata[1] & 0x80) && (!pPlayer->backpack)) {
        pPlayer->backpack = true;

        for (int32_t i = 0; i < NUMAMMO; ++i) {
            pPlayer->maxammo[i] <<= 1;
        }
    }
    
    // Apply: ammo, health and armor
    pPlayer->ammo[am_clip] = (clipsEnc * pPlayer->maxammo[am_clip]) / 8;
    pPlayer->ammo[am_shell] = (shellsEnc * pPlayer->maxammo[am_shell]) / 8;
    pPlayer->ammo[am_cell] = (cellsEnc * pPlayer->maxammo[am_cell]) / 8;
    pPlayer->ammo[am_misl] = (missilesEnc * pPlayer->maxammo[am_misl]) / 8;
    pPlayer->health = (healthPointsEnc * 200) / 8;
    pPlayer->armorpoints = (armorPointsEnc * 200) / 8;
    pPlayer->armortype = armorType;
    pPlayer->mo->health = pPlayer->health;
    return true;
}

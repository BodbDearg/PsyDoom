#include "doomdef.h"

#include "Endian.h"

#include <algorithm>

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: decodes an 8-bit analog movement field to a 32-bit fixed point number in the range of -1.0 to +1.0.
// The top bit is the sign bit and the lower 7-bits are a 0-127 movement amount representing 0 to 100% movement.
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t decodeAnalogMoveAmount(const uint8_t amt8) noexcept {
    const bool bNegative = ((amt8 & 0x80) != 0);
    const fixed_t absMoveAmt = ((fixed_t)(amt8 & 0x7F) << 16) / 127;
    return (bNegative) ? -absMoveAmt : absMoveAmt;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: encodes a 32-bit fixed point movement amount (from -1.0 to +1.0) to an 8-bit analog movement field.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t encodeAnalogMoveAmount(const fixed_t amt) noexcept {
    const bool bNegative = (amt < 0);
    const fixed_t absClampedAmount = std::min(std::abs(amt), 0x10000);      // Make positive and clamp to 0-1
    const uint8_t amt7 = (uint8_t)((absClampedAmount * 127) >> 16);         // Convert from 0-127
    return (bNegative) ? amt7 | 0x80u : amt7;                               // Add the sign bit
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get and set analog movement and rotation fields
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t TickInputs::getAnalogForwardMove() const noexcept {
    return decodeAnalogMoveAmount(_analogForwardMove);
}

fixed_t TickInputs::getAnalogSideMove() const noexcept {
    return decodeAnalogMoveAmount(_analogSideMove);
}

void TickInputs::setAnalogForwardMove(const fixed_t amt) noexcept {
    _analogForwardMove = encodeAnalogMoveAmount(amt);
}

void TickInputs::setAnalogSideMove(const fixed_t amt) noexcept {
    _analogSideMove = encodeAnalogMoveAmount(amt);
}

angle_t TickInputs::getAnalogTurn() const noexcept {
    return (angle_t) _analogTurn << 16;
}

void TickInputs::setAnalogTurn(const angle_t amt) noexcept {
    _analogTurn = (uint16_t)(amt >> 16);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reset tick inputs to their defaults
//------------------------------------------------------------------------------------------------------------------------------------------
void TickInputs::reset() noexcept {
    *this = {};
    directSwitchToWeapon = wp_nochange;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the 'TickInputs' structure
//------------------------------------------------------------------------------------------------------------------------------------------
void TickInputs::byteSwap() noexcept {
    Endian::byteSwapInPlace(_analogForwardMove);
    Endian::byteSwapInPlace(_analogSideMove);
    Endian::byteSwapInPlace(_analogTurn);
    Endian::byteSwapInPlace(directSwitchToWeapon);
    Endian::byteSwapInPlace(_flags1.bits);
    Endian::byteSwapInPlace(_flags2.bits);
    Endian::byteSwapInPlace(_flags3.bits);
    Endian::byteSwapInPlace(_flags4.bits);
    Endian::byteSwapInPlace(_flags5.bits);
    Endian::byteSwapInPlace(psxMouseDx);
    Endian::byteSwapInPlace(psxMouseDy);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bytes swaps from little to big endian or big endian to little if the host architecture is big-endian
//------------------------------------------------------------------------------------------------------------------------------------------
void TickInputs::endianCorrect() noexcept {
    if constexpr (Endian::isBig()) {
        byteSwap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the 'NetPacket_Connect' structure
//------------------------------------------------------------------------------------------------------------------------------------------
void NetPacket_Connect::byteSwap() noexcept {
    Endian::byteSwapInPlace(protocolVersion);
    Endian::byteSwapInPlace(gameId);
    Endian::byteSwapEnumInPlace(startGameType);
    Endian::byteSwapEnumInPlace(startGameSkill);
    Endian::byteSwapInPlace(startMap);
    Endian::byteSwapInPlace(bIsDemoRecording);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bytes swaps from little to big endian or big endian to little if the host architecture is big-endian
//------------------------------------------------------------------------------------------------------------------------------------------
void NetPacket_Connect::endianCorrect() noexcept {
    if constexpr (Endian::isBig()) {
        byteSwap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the 'NetPacket_Tick' structure
//------------------------------------------------------------------------------------------------------------------------------------------
void NetPacket_Tick::byteSwap() noexcept {
    Endian::byteSwapInPlace(errorCheck);
    Endian::byteSwapInPlace(elapsedVBlanks);
    Endian::byteSwapInPlace(lastPacketDelayMs);
    inputs.byteSwap();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bytes swaps from little to big endian or big endian to little if the host architecture is big-endian
//------------------------------------------------------------------------------------------------------------------------------------------
void NetPacket_Tick::endianCorrect() noexcept {
    if constexpr (Endian::isBig()) {
        byteSwap();
    }
}
#endif  // #if PSYDOOM_MODS

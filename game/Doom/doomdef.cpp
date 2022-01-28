#include "doomdef.h"

#include "Endian.h"

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the 'TickInputs' structure
//------------------------------------------------------------------------------------------------------------------------------------------
void TickInputs::byteSwap() noexcept {
    Endian::byteSwapInPlace(analogForwardMove);
    Endian::byteSwapInPlace(analogSideMove);
    Endian::byteSwapInPlace(analogTurn);
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

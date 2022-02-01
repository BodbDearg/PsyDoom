#include "DemoCommon.h"

#include "Doom/doomdef.h"
#include "Endian.h"

#include <cstring>

BEGIN_NAMESPACE(DemoCommon)

//--------------------------------------------------------------------------------------------------------------------------------------
// Saves the data from a full-fat 'TickInputs' data structure to 'DemoTickInputs'
//--------------------------------------------------------------------------------------------------------------------------------------
void DemoTickInputs::serializeFrom(const TickInputs& tickInputs) noexcept {
    analogForwardMove = tickInputs._analogForwardMove;
    analogSideMove = tickInputs._analogSideMove;
    analogTurn = tickInputs._analogTurn;
    directSwitchToWeapon = tickInputs.directSwitchToWeapon;
    flags1 = tickInputs._flags1.bits;
    flags2 = tickInputs._flags2.bits;
    flags3 = tickInputs._flags3.bits;
}

//--------------------------------------------------------------------------------------------------------------------------------------
// Saves the data in a 'DemoTickInputs' to the given 'TickInputs'.
// Fields not saved by 'DemoTickInputs' are zeroed.
//--------------------------------------------------------------------------------------------------------------------------------------
void DemoTickInputs::deserializeTo(TickInputs& tickInputs) const noexcept {
    tickInputs = {};
    tickInputs._analogForwardMove = analogForwardMove;
    tickInputs._analogSideMove = analogSideMove;
    tickInputs._analogTurn = analogTurn;
    tickInputs.directSwitchToWeapon = directSwitchToWeapon;
    tickInputs._flags1.bits = flags1;
    tickInputs._flags2.bits = flags2;
    tickInputs._flags3.bits = flags3;
}

//--------------------------------------------------------------------------------------------------------------------------------------
// Reverses the byte order of this data structure
//--------------------------------------------------------------------------------------------------------------------------------------
void DemoTickInputs::byteSwap() noexcept {
    Endian::byteSwapInPlace(analogForwardMove);
    Endian::byteSwapInPlace(analogSideMove);
    Endian::byteSwapInPlace(analogTurn);
    Endian::byteSwapInPlace(directSwitchToWeapon);
    Endian::byteSwapInPlace(flags1);
    Endian::byteSwapInPlace(flags2);
    Endian::byteSwapInPlace(flags3);
}

//--------------------------------------------------------------------------------------------------------------------------------------
// Checks to see if the other tick inputs structure matches this one
//--------------------------------------------------------------------------------------------------------------------------------------
bool DemoTickInputs::equals(const DemoTickInputs& other) const noexcept {
    return (std::memcmp(this, &other, sizeof(*this)) == 0);
}

END_NAMESPACE(DemoCommon)

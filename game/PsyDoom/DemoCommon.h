#pragma once

#include "Endian.h"

struct TickInputs;

BEGIN_NAMESPACE(DemoCommon)

//------------------------------------------------------------------------------------------------------------------------------------------
// The current demo file format version.
// This should be incremented whenever the contents of or expected behavior of the demo file changes.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t DEMO_FILE_VERSION = 12;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: calls 'byteSwap()' on the specified type if the host architecture is big endian.
// Used to convert from host to little endian and back again.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void endianCorrect([[maybe_unused]] T& obj) noexcept {
    if constexpr (Endian::isBig()) {
        obj.byteSwap();
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------
// A condensed version of 'TickInputs' used to record to demo files.
// Strips out stuff that we don't need for demo recording to save on the overall bitrate.
// For more info on what each field means, see 'TickInputs'.
//--------------------------------------------------------------------------------------------------------------------------------------
struct DemoTickInputs {
    uint8_t     analogForwardMove;
    uint8_t     analogSideMove;
    uint16_t    analogTurn;
    uint8_t     directSwitchToWeapon;
    uint8_t     flags1;
    uint8_t     flags2;
    uint8_t     flags3;

    void serializeFrom(const TickInputs& tickInputs) noexcept;
    void deserializeTo(TickInputs& tickInputs) const noexcept;

    void byteSwap() noexcept;
    bool equals(const DemoTickInputs& other) const noexcept;
};

static_assert(sizeof(DemoTickInputs) == 8);

END_NAMESPACE(DemoCommon)

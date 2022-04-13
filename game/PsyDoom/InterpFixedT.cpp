#include "InterpFixedT.h"

#include "Doom/Game/g_game.h"
#include "Doom/Renderer/r_main.h"

fixed_t InterpFixedT::operator = (const fixed_t& newValue) noexcept {
    const int32_t curTic = gGameTic;

    if (curTic != oldGameTic) {
        oldValue = value;
        oldGameTic = curTic;
    }

    value = newValue;
    return newValue;
}

fixed_t InterpFixedT::renderValue() noexcept {
    // If the value is the same don't bother interpolating (common case)
    if (value == oldValue)
        return value;
    
    // See if the current tic is to be interpolated.
    // If not then snap the interpolation to speed up future queries (saves looking up the 'gGameTic' global).
    if (gGameTic != oldGameTic) {
        snap();
        return value;
    }

    // Need to interpolate, do it!
    const fixed_t lerpFactor = gWorldLerpFactor;
    return R_LerpCoord(oldValue, value, lerpFactor);
}

fixed_t InterpFixedT::renderValue(const bool bInterpolate) noexcept {
    return (bInterpolate) ? renderValue() : value;
}

#pragma once

#include <cstdint>

typedef int32_t fixed_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// A 'fixed_t' value that supports being interpolated (according to the world/mobj tic rate) for uncapped framerates
//------------------------------------------------------------------------------------------------------------------------------------------
struct InterpFixedT {
    fixed_t     value;          // The current value of this field
    fixed_t     oldValue;       // The previous value of this field, prior to the current value being set
    int32_t     oldGameTic;     // The game tick in which the old value was last assigned, this is the tick that should be interpolated

    // Assign a new value to the interpolated field and remember the replaced value if from a previous frame
    fixed_t operator = (const fixed_t& newValue) noexcept;

    inline fixed_t operator += (const fixed_t& amt) noexcept { return operator = (value + amt); }
    inline fixed_t operator -= (const fixed_t& amt) noexcept { return operator = (value - amt); }
    inline fixed_t operator *= (const fixed_t& amt) noexcept { return operator = (value * amt); }
    inline fixed_t operator /= (const fixed_t& amt) noexcept { return operator = (value / amt); }

    // Returns the un-interpolated value of the field.
    // Most of the time this is what is needed (for game logic).
    inline operator fixed_t () const noexcept {
        return value;
    }

    // Ends the current interpolation
    inline void snap() noexcept {
        oldValue = value;
        oldGameTic = 0;
    }

    // Returns the interpolated value used for rendering (if uncapped framerates are enabled) or just the value itself.
    // An optional overload allows the interpolation to be predicated on a condition.
    // May modify the old value to speed up future render value queries.
    // The interpolation is done using the world/mobj frame rate of 15 Hz for NTSC.
    fixed_t renderValue() noexcept;
    fixed_t renderValue(const bool bInterpolate) noexcept;
};

#include "CpuGpr.h"
#include <cassert>

//------------------------------------------------------------------------------------------------------------------------------------------
// Constant evaluator register state:
//      Structure containing current register state for the constant instruction evaluator.
//      Stores whether the state of each register is known as a constant, just by inspecting the instructions and the
//      exact value of each register if it is a known constant.
//------------------------------------------------------------------------------------------------------------------------------------------
struct ConstEvalRegState {
    bool bGprValueKnown[CpuGpr::NUM_GPRS];      // Whether the value of each general purpose register is known as a constant
    bool bHiRegValueKnown;                      // Whether the value of the '$hi' register is known as a constant
    bool bLoRegValueKnown;                      // Whether the value of the '$lo' register is known as a constant

    uint32_t gprValue[CpuGpr::NUM_GPRS];        // The value of each general purpose register if known as a constant (undefined if not)
    uint32_t hiRegValue;                        // The value of the '$hi' register if known as a constant (undefined if not)
    uint32_t loRegValue;                        // The value of the '$lo' register if known as a constant (undefined if not)

    //------------------------------------------------------------------------------------------------------------------
    // Clear the state of the given GPR and mark it as undefined.
    // One exception, $zero is always set to '0' and marked as known since it is always '0'!
    //------------------------------------------------------------------------------------------------------------------
    inline void clearGpr(const uint8_t gprIdx) noexcept {
        assert(gprIdx < CpuGpr::NUM_GPRS);

        if (gprIdx > 0) {
            bGprValueKnown[gprIdx] = false;
            gprValue[gprIdx] = 0xDEADBEEFu;
        } else {
            bGprValueKnown[0] = true;
            gprValue[0] = 0;
        }
    }

    inline void clearHi() noexcept {
        bHiRegValueKnown = false;
        hiRegValue = 0xDEADBEEFu;
    }

    inline void clearLo() noexcept {
        bLoRegValueKnown = false;
        loRegValue = 0xDEADBEEFu;
    }

    inline void clearHiAndLo() noexcept {
        clearHi();
        clearLo();
    }

    //------------------------------------------------------------------------------------------------------------------
    // Clear the known constant state of all registers.
    // Everything except the '$zero' register is marked unknown and undefined.
    //------------------------------------------------------------------------------------------------------------------
    inline void clear() noexcept {
        for (uint8_t i = 0; i < CpuGpr::NUM_GPRS; ++i) {
            clearGpr(i);
        }

        clearHiAndLo();
    }

    //------------------------------------------------------------------------------------------------------------------
    // Clears registers that are NOT preserved by function calls
    //------------------------------------------------------------------------------------------------------------------
    inline void clearFuncCallTransientRegisters() noexcept {
        clearGpr(CpuGpr::ZERO);     // Note: this just ensures it's still zero!
        clearGpr(CpuGpr::AT);
        clearGpr(CpuGpr::V0);
        clearGpr(CpuGpr::V1);
        clearGpr(CpuGpr::A0);
        clearGpr(CpuGpr::A1);
        clearGpr(CpuGpr::A2);
        clearGpr(CpuGpr::A3);
        clearGpr(CpuGpr::T0);
        clearGpr(CpuGpr::T1);
        clearGpr(CpuGpr::T2);
        clearGpr(CpuGpr::T3);
        clearGpr(CpuGpr::T4);
        clearGpr(CpuGpr::T5);
        clearGpr(CpuGpr::T6);
        clearGpr(CpuGpr::T7);
        clearGpr(CpuGpr::T8);
        clearGpr(CpuGpr::T9);
        clearGpr(CpuGpr::K0);
        clearGpr(CpuGpr::K1);
    }

    //------------------------------------------------------------------------------------------------------------------
    // Merges this state with the other state.
    // Any registers that contradict between the two states are cleared and marked as 'undefined'.
    // This can be used to test when two codepaths reach the same instruction, whether they produce differing results.
    // It can for example be used to detect constant values within a loop.
    //------------------------------------------------------------------------------------------------------------------
    inline void mergeWith(const ConstEvalRegState& other) noexcept {
        // $zero is always '0' of course, just set that again to be sure...
        bGprValueKnown[0] = true;
        gprValue[0] = 0;

        // Merge GPRs
        for (uint8_t i = 0; i < CpuGpr::NUM_GPRS; ++i) {
            const bool bGprsMatch = (
                (bGprValueKnown[i] == other.bGprValueKnown[i]) &&
                (gprValue[i] == other.gprValue[i])
            );

            if (!bGprsMatch) {
                clearGpr(i);
            }
        }

        // Merge '$hi' and '$lo'
        {
            const bool bHiMatch = (
                (bHiRegValueKnown == other.bHiRegValueKnown) &&
                (hiRegValue == other.hiRegValue)
            );

            if (!bHiMatch) {
                clearHi();
            }
        }

        {
            const bool bLoMatch = (
                (bLoRegValueKnown == other.bLoRegValueKnown) &&
                (loRegValue == other.loRegValue)
            );

            if (!bLoMatch) {
                clearLo();
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    // Set the given GPR to a known constant state and mark it as known.
    // Note: writes to '$zero' will be ignored!
    //------------------------------------------------------------------------------------------------------------------
    inline void setGpr(const uint8_t gprIdx, const uint32_t value) noexcept {
        assert(gprIdx < CpuGpr::NUM_GPRS);

        if (gprIdx > 0) {
            bGprValueKnown[gprIdx] = true;
            gprValue[gprIdx] = value;
        } else {
            bGprValueKnown[0] = true;
            gprValue[0] = 0;
        }
    }

    inline void setHi(const uint32_t value) noexcept {
        bHiRegValueKnown = true;
        hiRegValue = value;
    }

    inline void setLo(const uint32_t value) noexcept {
        bLoRegValueKnown = true;
        loRegValue = value;
    }
};

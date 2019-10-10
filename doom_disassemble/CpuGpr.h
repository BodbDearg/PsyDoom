#include "CpuOpcode.h"

//----------------------------------------------------------------------------------------------------------------------
// Constants and utilities relating to CPU general purpose registers.
// Source of info: https://problemkaputt.de/psx-spx.htm#cpuspecifications
//----------------------------------------------------------------------------------------------------------------------
namespace CpuGpr {
    constexpr uint8_t ZERO = 0;     // Constant (always 0) (this one isn't a real register)
    constexpr uint8_t AT = 1;       // Assembler temporary (destroyed by some pseudo opcodes!)
    constexpr uint8_t V0 = 2;       // Subroutine return values, may be changed by subroutines.
    constexpr uint8_t V1 = 3;
    constexpr uint8_t A0 = 4;       // Subroutine arguments, may be changed by subroutines
    constexpr uint8_t A1 = 5;
    constexpr uint8_t A2 = 6;
    constexpr uint8_t A3 = 7;
    constexpr uint8_t T0 = 8;       // Temporaries, may be changed by subroutines
    constexpr uint8_t T1 = 9;
    constexpr uint8_t T2 = 10;
    constexpr uint8_t T3 = 11;
    constexpr uint8_t T4 = 12;
    constexpr uint8_t T5 = 13;
    constexpr uint8_t T6 = 14;
    constexpr uint8_t T7 = 15;
    constexpr uint8_t S0 = 16;      // Static variables, must be saved by subs
    constexpr uint8_t S1 = 17;
    constexpr uint8_t S2 = 18;
    constexpr uint8_t S3 = 19;
    constexpr uint8_t S4 = 20;
    constexpr uint8_t S5 = 21;
    constexpr uint8_t S6 = 22;
    constexpr uint8_t S7 = 23;
    constexpr uint8_t T8 = 24;      // Temporaries, may be changed by subroutines
    constexpr uint8_t T9 = 25;
    constexpr uint8_t K0 = 26;      // Reserved for kernel (destroyed by some IRQ handlers!)
    constexpr uint8_t K1 = 27;
    constexpr uint8_t GP = 28;      // Global pointer (rarely used)
    constexpr uint8_t SP = 29;      // Stack pointer
    constexpr uint8_t FP = 30;      // Frame Pointer, or 9th Static variable, must be saved
    constexpr uint8_t RA = 31;      // Return address (used so by JAL,BLTZAL,BGEZAL opcodes)

    // How many general purpose registers there are
    constexpr uint8_t NUM_GPRS = 32;

    // Get the human readable name for a specific GPR
    inline constexpr const char* getName(const uint8_t gprIndex) noexcept {
        switch (gprIndex) {
            case ZERO:  return "$(ZERO)";
            case AT:    return "$(AT)";
            case V0:    return "$(V0)";
            case V1:    return "$(V1)";
            case A0:    return "$(A0)";
            case A1:    return "$(A1)";
            case A2:    return "$(A2)";
            case A3:    return "$(A3)";
            case T0:    return "$(T0)";
            case T1:    return "$(T1)";
            case T2:    return "$(T2)";
            case T3:    return "$(T3)";
            case T4:    return "$(T4)";
            case T5:    return "$(T5)";
            case T6:    return "$(T6)";
            case T7:    return "$(T7)";
            case S0:    return "$(S0)";
            case S1:    return "$(S1)";
            case S2:    return "$(S2)";
            case S3:    return "$(S3)";
            case S4:    return "$(S4)";
            case S5:    return "$(S5)";
            case S6:    return "$(S6)";
            case S7:    return "$(S7)";
            case T8:    return "$(T8)";
            case T9:    return "$(T9)";
            case K0:    return "$(K0)";
            case K1:    return "$(K1)";
            case GP:    return "$(GP)";
            case SP:    return "$(SP)";
            case FP:    return "$(FP)";
            case RA:    return "$(RA)";

            default: return "<INVALID_GPR>";
        }
    }
}
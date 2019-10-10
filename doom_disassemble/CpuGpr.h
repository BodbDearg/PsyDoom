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
            case ZERO:  return "$zero";
            case AT:    return "$at";
            case V0:    return "$v0";
            case V1:    return "$v1";
            case A0:    return "$a0";
            case A1:    return "$a1";
            case A2:    return "$a2";
            case A3:    return "$a3";
            case T0:    return "$t0";
            case T1:    return "$t1";
            case T2:    return "$t2";
            case T3:    return "$t3";
            case T4:    return "$t4";
            case T5:    return "$t5";
            case T6:    return "$t6";
            case T7:    return "$t7";
            case S0:    return "$s0";
            case S1:    return "$s1";
            case S2:    return "$s2";
            case S3:    return "$s3";
            case S4:    return "$s4";
            case S5:    return "$s5";
            case S6:    return "$s6";
            case S7:    return "$s7";
            case T8:    return "$t8";
            case T9:    return "$t9";
            case K0:    return "$k0";
            case K1:    return "$k1";
            case GP:    return "$gp";
            case SP:    return "$sp";
            case FP:    return "$fp";
            case RA:    return "$ra";

            default: return "<INVALID_GPR>";
        }
    }
}
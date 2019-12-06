#pragma once

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Struct defining how we handle the 'jr' instruction.
// This instruction is difficult because it is not a typical function call (no return).
// Typically it is used for jump tables (for switch statements) but sometimes it is used for PSYQ bios call stub functions also.
//------------------------------------------------------------------------------------------------------------------------------------------
struct JRInstHandler {
    // What type of 'jr' instruction handler this is
    enum class Type : uint32_t {
        JUMP_TABLE,     // The jr instruction uses a jump table
        BIOS_CALL       // The jr instruction does a bios call
    };

    uint32_t    instAddress;        // What address the 'jr' instruction handler is for
    Type        type;
    uint32_t    jumpTableAddr;      // If the handler treats the 'jr' instruction as using a jump table, the addresss of the jump table
};

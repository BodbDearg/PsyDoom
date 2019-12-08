#include "m_random.h"

#include "PsxVm/PsxVm.h"

void P_Random() noexcept {
loc_80012A18:
    v0 = lw(gp + 0x2C);                                 // Load from: gPRndIndex (8007760C)
    v0++;
    v0 &= 0xFF;
    sw(v0, gp + 0x2C);                                  // Store to: gPRndIndex (8007760C)
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x75A8;                                       // Result = RndTable[0] (80058A58)
    at += v0;
    v0 = lbu(at);
    return;
}

void M_Random() noexcept {
loc_80012A44:
    v0 = lw(gp + 0x28);                                 // Load from: gRndIndex (80077608)
    v0++;
    v0 &= 0xFF;
    sw(v0, gp + 0x28);                                  // Store to: gRndIndex (80077608)
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x75A8;                                       // Result = RndTable[0] (80058A58)
    at += v0;
    v0 = lbu(at);
    return;
}

void M_ClearRandom() noexcept {
loc_80012A70:
    sw(0, gp + 0x2C);                                   // Store to: gPRndIndex (8007760C)
    sw(0, gp + 0x28);                                   // Store to: gRndIndex (80077608)
    return;
}

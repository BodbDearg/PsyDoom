#pragma once

#include "ProgElem.h"

extern const ProgElem*  gProgramElems_Doom;
extern const uint32_t   gNumProgramElems_Doom;
extern const uint32_t   gGpRegisterValue_Doom;      // The value of $gp throughout the program (used for constant evaluation)

// For comparison: since it has debug symbols
extern const ProgElem*  gProgramElems_DestructionDerby;
extern const uint32_t   gNumProgramElems_DestructionDerby;
extern const uint32_t   gGpRegisterValue_DestructionDerby;      // The value of $gp throughout the program (used for constant evaluation)

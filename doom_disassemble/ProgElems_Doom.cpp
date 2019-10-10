#include "ProgElems.h"

static const ProgElem ELEMS[] = {
    { 0x8003290C, 0x80032934, "",               ProgElemType::FUNCTION },
    { 0x80050714, 0x800507AC, "main",           ProgElemType::FUNCTION },
};

const ProgElem*     gProgramElems_Doom = ELEMS;
const uint32_t      gNumProgramElems_Doom = sizeof(ELEMS) / sizeof(ProgElem);

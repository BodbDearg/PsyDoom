#include "ProgElems.h"

static const ProgElem ELEMS[] = {
    { 0x8003290C, 0x80032934, "",                           ProgElemType::FUNCTION },
    { 0x80050714, 0x800507AC, "main",                       ProgElemType::FUNCTION },
    { 0x800507AC, 0x8005081C, "",                           ProgElemType::FUNCTION },
    { 0x80050884, 0x80050894, "LIBAPI_InitHeap",            ProgElemType::FUNCTION },
};

const ProgElem*     gProgramElems_Doom = ELEMS;
const uint32_t      gNumProgramElems_Doom = sizeof(ELEMS) / sizeof(ProgElem);

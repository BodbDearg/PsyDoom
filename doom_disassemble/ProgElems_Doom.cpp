#include "ProgElems.h"

static const ProgElem ELEMS[] = {
    { 0x80014E54, 0x80014EBC, "",                               ProgElemType::FUNCTION },
    { 0x8003290C, 0x80032934, "",                               ProgElemType::FUNCTION },
    { 0x80049C1C, 0x80049C2C, "LIBAPI_CloseEvent",              ProgElemType::FUNCTION },
    { 0x80049C2C, 0x80049C3C, "LIBAPI_EnterCriticalSection",    ProgElemType::FUNCTION },
    { 0x80049C3C, 0x80049C4C, "LIBAPI_write",                   ProgElemType::FUNCTION },
    { 0x80049C4C, 0x80049C5C, "LIBAPI_EnableEvent",             ProgElemType::FUNCTION },
    { 0x80049C5C, 0x80049C6C, "LIBAPI_InitPAD2",                ProgElemType::FUNCTION },
    { 0x80049DDC, 0x80049DEC, "LIBAPI_DisableEvent",            ProgElemType::FUNCTION },
    { 0x80049DEC, 0x80049DFC, "LIBAPI_StartPAD2",               ProgElemType::FUNCTION },
    { 0x80049DFC, 0x80049E0C, "LIBAPI_ChangeClearPAD",          ProgElemType::FUNCTION },
    { 0x80049E0C, 0x80049E1C, "LIBAPI_OpenEvent",               ProgElemType::FUNCTION },
    { 0x80049E1C, 0x80049E2C, "LIBAPI_read",                    ProgElemType::FUNCTION },
    { 0x80049E2C, 0x80049E3C, "LIBAPI_TestEvent",               ProgElemType::FUNCTION },
    { 0x80049E3C, 0x80049E4C, "LIBAPI_ExitCriticalSection",     ProgElemType::FUNCTION },
    { 0x80049E4C, 0x80049E5C, "LIBAPI_open",                    ProgElemType::FUNCTION },
    { 0x8004AD40, 0x8004AD50, "LIBAPI_HookEntryInt",            ProgElemType::FUNCTION },
    { 0x8004AD50, 0x8004AD60, "LIBAPI_ResetEntryInt",           ProgElemType::FUNCTION },
    { 0x8004AD60, 0x8004AD70, "LIBAPI_ChangeClearRCnt",         ProgElemType::FUNCTION },
    { 0x8004AD80, 0x8004AD90, "LIBAPI_ReturnFromException",     ProgElemType::FUNCTION },
    { 0x8004E8B0, 0x8004E8C0, "LIBAPI_GPU_cw",                  ProgElemType::FUNCTION },
    { 0x80050714, 0x800507AC, "main",                           ProgElemType::FUNCTION },
    { 0x800507AC, 0x8005081C, "",                               ProgElemType::FUNCTION },
    { 0x80050884, 0x80050894, "LIBAPI_InitHeap",                ProgElemType::FUNCTION },
    { 0x80053D48, 0x80053D58, "LIBAPI_DeliverEvent",            ProgElemType::FUNCTION },
    { 0x80054324, 0x80054334, "LIBAPI_WaitEvent",               ProgElemType::FUNCTION },
    { 0x80058A18, 0x80058A28, "LIBAPI_SysEnqIntRP",             ProgElemType::FUNCTION },
    { 0x80058A28, 0x80058A38, "LIBAPI_AddDrv",                  ProgElemType::FUNCTION },
    { 0x80058A38, 0x80058A48, "LIBAPI_DelDrv",                  ProgElemType::FUNCTION },
};

const ProgElem*     gProgramElems_Doom = ELEMS;
const uint32_t      gNumProgramElems_Doom = sizeof(ELEMS) / sizeof(ProgElem);

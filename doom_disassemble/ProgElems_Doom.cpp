#include "ProgElems.h"

static const ProgElem ELEMS[] = {
    { 0x80012274, 0x800123A4, "",                                   ProgElemType::FUNCTION },
    { 0x800123E4, 0x80012424, "",                                   ProgElemType::FUNCTION },
    { 0x80012424, 0x800124A8, "",                                   ProgElemType::FUNCTION },
    { 0x800124A8, 0x800124E8, "",                                   ProgElemType::FUNCTION },
    { 0x80012850, 0x8001290C, "",                                   ProgElemType::FUNCTION },
    { 0x8001290C, 0x80012940, "",                                   ProgElemType::FUNCTION },
    { 0x8001297C, 0x800129D4, "",                                   ProgElemType::FUNCTION },
    { 0x80012A18, 0x80012A44, "",                                   ProgElemType::FUNCTION },
    { 0x80012A70, 0x80012A80, "",                                   ProgElemType::FUNCTION },
    { 0x80012A80, 0x80012AA0, "",                                   ProgElemType::FUNCTION },
    { 0x80012AA0, 0x80012B10, "",                                   ProgElemType::FUNCTION },
    { 0x80012B78, 0x80012BD8, "",                                   ProgElemType::FUNCTION }, // TODO: figure out func ptr jump
    { 0x80012E04, 0x80012F00, "",                                   ProgElemType::FUNCTION },
    { 0x80013394, 0x80013528, "",                                   ProgElemType::FUNCTION },
    { 0x80013528, 0x80013714, "",                                   ProgElemType::FUNCTION },
    { 0x80013714, 0x80013838, "",                                   ProgElemType::FUNCTION },
    { 0x80013838, 0x80013840, "empty_func1",                        ProgElemType::FUNCTION },
    { 0x80014E54, 0x80014EBC, "",                                   ProgElemType::FUNCTION },
    { 0x8001C408, 0x8001C540, "",                                   ProgElemType::FUNCTION },
    { 0x8001D184, 0x8001D704, "",                                   ProgElemType::FUNCTION },
    { 0x80021BA0, 0x80021DD8, "",                                   ProgElemType::FUNCTION },
    { 0x80021EC4, 0x80022104, "",                                   ProgElemType::FUNCTION },
    { 0x80022104, 0x80022278, "",                                   ProgElemType::FUNCTION },
    { 0x8002237C, 0x80022650, "",                                   ProgElemType::FUNCTION },
    { 0x80022650, 0x800227CC, "",                                   ProgElemType::FUNCTION },
    { 0x800227CC, 0x800228CC, "",                                   ProgElemType::FUNCTION },
    { 0x80022920, 0x80022B58, "",                                   ProgElemType::FUNCTION },
    { 0x80022B58, 0x80022E68, "",                                   ProgElemType::FUNCTION },
    { 0x800230D4, 0x80023700, "",                                   ProgElemType::FUNCTION }, // TODO: appears to be referenced by func ptrs
    { 0x80028C68, 0x80028C74, "",                                   ProgElemType::FUNCTION },
    { 0x8002B9A8, 0x8002B9E0, "",                                   ProgElemType::FUNCTION },
    { 0x8002B9E0, 0x8002BB50, "",                                   ProgElemType::FUNCTION },
    { 0x8002BB50, 0x8002BC54, "",                                   ProgElemType::FUNCTION },
    { 0x8002BC54, 0x8002BDA4, "",                                   ProgElemType::FUNCTION },
    { 0x8002BDA4, 0x8002BE68, "",                                   ProgElemType::FUNCTION },
    { 0x8002BE68, 0x8002BF2C, "",                                   ProgElemType::FUNCTION },
    { 0x8002BF2C, 0x8002C07C, "",                                   ProgElemType::FUNCTION }, // TODO (stuck on referenced func 8004C438)
    { 0x800305B0, 0x80030634, "",                                   ProgElemType::FUNCTION },
    { 0x80030F5C, 0x80031088, "",                                   ProgElemType::FUNCTION },
    { 0x800310C8, 0x80031394, "",                                   ProgElemType::FUNCTION },
    { 0x80031394, 0x800314A4, "",                                   ProgElemType::FUNCTION },
    { 0x80031558, 0x80031648, "",                                   ProgElemType::FUNCTION },
    { 0x80031648, 0x80031698, "",                                   ProgElemType::FUNCTION },
    { 0x800317AC, 0x800319E4, "",                                   ProgElemType::FUNCTION },
    { 0x800319E4, 0x80031B04, "",                                   ProgElemType::FUNCTION },
    { 0x80031B04, 0x80031BD4, "",                                   ProgElemType::FUNCTION },
    { 0x80031BD4, 0x80031C24, "",                                   ProgElemType::FUNCTION },
    { 0x80031C24, 0x80031CE0, "",                                   ProgElemType::FUNCTION },
    { 0x80031CE0, 0x80031D90, "",                                   ProgElemType::FUNCTION },
    { 0x80031D90, 0x80031E48, "",                                   ProgElemType::FUNCTION },
    { 0x80031EB4, 0x80031EDC, "",                                   ProgElemType::FUNCTION },
    { 0x80031EDC, 0x80031FD8, "",                                   ProgElemType::FUNCTION },
    { 0x80031FD8, 0x80032024, "",                                   ProgElemType::FUNCTION },
    { 0x80032024, 0x8003206C, "",                                   ProgElemType::FUNCTION },
    { 0x8003206C, 0x80032144, "",                                   ProgElemType::FUNCTION },
    { 0x80032144, 0x8003219C, "",                                   ProgElemType::FUNCTION },
    { 0x8003219C, 0x800321D0, "",                                   ProgElemType::FUNCTION },
    { 0x800321D0, 0x800323C8, "",                                   ProgElemType::FUNCTION },
    { 0x800323C8, 0x800325D8, "",                                   ProgElemType::FUNCTION },
    { 0x800325D8, 0x80032640, "",                                   ProgElemType::FUNCTION },
    { 0x80032640, 0x80032770, "",                                   ProgElemType::FUNCTION },
    { 0x80032770, 0x80032838, "",                                   ProgElemType::FUNCTION },
    { 0x80032904, 0x8003290C, "empty_func3",                        ProgElemType::FUNCTION },
    { 0x8003290C, 0x80032934, "",                                   ProgElemType::FUNCTION },
    { 0x80032934, 0x80032B0C, "",                                   ProgElemType::FUNCTION }, // TODO: confused by referenced call '80058534'
    { 0x80032B0C, 0x80032BB8, "MAYBE_err_func_no_return",           ProgElemType::FUNCTION }, // TODO: confused by referenced call '8004F6AC'
    { 0x80032BF4, 0x80032D04, "",                                   ProgElemType::FUNCTION },
    { 0x80032D04, 0x80032D84, "",                                   ProgElemType::FUNCTION },
    { 0x80032D84, 0x800332E0, "",                                   ProgElemType::FUNCTION },
    { 0x800332E0, 0x800333D8, "",                                   ProgElemType::FUNCTION },
    { 0x800333D8, 0x800333F0, "",                                   ProgElemType::FUNCTION },
    { 0x800333F0, 0x8003350C, "",                                   ProgElemType::FUNCTION },
    { 0x8003352C, 0x80033578, "",                                   ProgElemType::FUNCTION },
    { 0x80033578, 0x8003390C, "",                                   ProgElemType::FUNCTION },
    { 0x8003390C, 0x8003397C, "",                                   ProgElemType::FUNCTION },
    { 0x8003397C, 0x80033AC4, "",                                   ProgElemType::FUNCTION },
    { 0x8003472C, 0x80034A60, "",                                   ProgElemType::FUNCTION }, // TODO: figure out referenced func '80058534' - may have a switch statement jump table
    { 0x80034CB8, 0x80034D14, "",                                   ProgElemType::FUNCTION },
    { 0x80034D14, 0x80034E58, "",                                   ProgElemType::FUNCTION }, // TODO: figure out referenced func '80058534' - may have a switch statement jump table
    { 0x80034E58, 0x80034EA4, "",                                   ProgElemType::FUNCTION },
    { 0x80034EA4, 0x80034F04, "",                                   ProgElemType::FUNCTION },
    { 0x80034F04, 0x80034F54, "",                                   ProgElemType::FUNCTION },    
    { 0x80035B24, 0x80035C94, "",                                   ProgElemType::FUNCTION },
    { 0x80038558, 0x80038610, "",                                   ProgElemType::FUNCTION },
    { 0x8003F180, 0x8003F200, "",                                   ProgElemType::FUNCTION },
    { 0x8003FACC, 0x8003FB9C, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8003FE20, 0x8003FE58, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x800406D4, 0x800407C8, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x800407C8, 0x80040830, "",                                   ProgElemType::FUNCTION }, // TODO    
    { 0x80040830, 0x80040838, "empty_func2",                        ProgElemType::FUNCTION },
    { 0x80041118, 0x80041318, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x800415B4, 0x800415D4, "",                                   ProgElemType::FUNCTION },
    { 0x800415EC, 0x80041734, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004371C, 0x800437F0, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x80049C1C, 0x80049C2C, "LIBAPI_CloseEvent",                  ProgElemType::FUNCTION },
    { 0x80049C2C, 0x80049C3C, "LIBAPI_EnterCriticalSection",        ProgElemType::FUNCTION },
    { 0x80049C3C, 0x80049C4C, "LIBAPI_write",                       ProgElemType::FUNCTION },
    { 0x80049C4C, 0x80049C5C, "LIBAPI_EnableEvent",                 ProgElemType::FUNCTION },
    { 0x80049C5C, 0x80049C6C, "LIBAPI_InitPAD2",                    ProgElemType::FUNCTION },
    { 0x80049DDC, 0x80049DEC, "LIBAPI_DisableEvent",                ProgElemType::FUNCTION },
    { 0x80049DEC, 0x80049DFC, "LIBAPI_StartPAD2",                   ProgElemType::FUNCTION },
    { 0x80049DFC, 0x80049E0C, "LIBAPI_ChangeClearPAD",              ProgElemType::FUNCTION },
    { 0x80049E0C, 0x80049E1C, "LIBAPI_OpenEvent",                   ProgElemType::FUNCTION },
    { 0x80049E1C, 0x80049E2C, "LIBAPI_read",                        ProgElemType::FUNCTION },
    { 0x80049E2C, 0x80049E3C, "LIBAPI_TestEvent",                   ProgElemType::FUNCTION },
    { 0x80049E3C, 0x80049E4C, "LIBAPI_ExitCriticalSection",         ProgElemType::FUNCTION },
    { 0x80049E4C, 0x80049E5C, "LIBAPI_open",                        ProgElemType::FUNCTION },
    { 0x8004A7AC, 0x8004A7DC, "",                                   ProgElemType::FUNCTION },
    { 0x8004A8E4, 0x8004A9A4, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004A8E4, 0x8004A9A4, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004AD40, 0x8004AD50, "LIBAPI_HookEntryInt",                ProgElemType::FUNCTION },
    { 0x8004AD50, 0x8004AD60, "LIBAPI_ResetEntryInt",               ProgElemType::FUNCTION },
    { 0x8004AD60, 0x8004AD70, "LIBAPI_ChangeClearRCnt",             ProgElemType::FUNCTION },
    { 0x8004AD80, 0x8004AD90, "LIBAPI_ReturnFromException",         ProgElemType::FUNCTION },
    { 0x8004AD90, 0x8004ADD0, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004BA94, 0x8004BBDC, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004BCC8, 0x8004BEF0, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004C004, 0x8004C070, "",                                   ProgElemType::FUNCTION },
    { 0x8004C198, 0x8004C210, "",                                   ProgElemType::FUNCTION },
    { 0x8004C210, 0x8004C27C, "",                                   ProgElemType::FUNCTION },
    { 0x8004C438, 0x8004C49C, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004C500, 0x8004C5A0, "",                                   ProgElemType::FUNCTION },
    { 0x8004C72C, 0x8004C79C, "",                                   ProgElemType::FUNCTION },
    { 0x8004C79C, 0x8004C860, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004C898, 0x8004CCBC, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004CE54, 0x8004CEAC, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004E8B0, 0x8004E8C0, "LIBAPI_GPU_cw",                      ProgElemType::FUNCTION },
    { 0x8004E928, 0x8004E9F0, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004E9F0, 0x8004EA08, "",                                   ProgElemType::FUNCTION },
    { 0x8004F09C, 0x8004F0DC, "",                                   ProgElemType::FUNCTION },
    { 0x8004F0DC, 0x8004F180, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004F180, 0x8004F44C, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004F44C, 0x8004F6AC, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004FC28, 0x8004FCB8, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x8004FCB8, 0x8004FCF4, "",                                   ProgElemType::FUNCTION },
    { 0x80050100, 0x80050130, "",                                   ProgElemType::FUNCTION },
    { 0x80050190, 0x800501B4, "",                                   ProgElemType::FUNCTION },
    { 0x800502EC, 0x80050304, "",                                   ProgElemType::FUNCTION },
    { 0x80050304, 0x80050310, "",                                   ProgElemType::FUNCTION },
    { 0x80050334, 0x800503B4, "",                                   ProgElemType::FUNCTION }, // TODO
    { 0x80050714, 0x800507AC, "main",                               ProgElemType::FUNCTION },
    { 0x800507AC, 0x8005081C, "",                                   ProgElemType::FUNCTION }, // TODO: figure out func ptr jump
    { 0x80050884, 0x80050894, "LIBAPI_InitHeap",                    ProgElemType::FUNCTION },
    { 0x80053D48, 0x80053D58, "LIBAPI_DeliverEvent",                ProgElemType::FUNCTION },
    { 0x80054324, 0x80054334, "LIBAPI_WaitEvent",                   ProgElemType::FUNCTION },
    { 0x800580B4, 0x800580E0, "",                                   ProgElemType::FUNCTION },
    { 0x80058A18, 0x80058A28, "LIBAPI_SysEnqIntRP",                 ProgElemType::FUNCTION },
    { 0x80058A28, 0x80058A38, "LIBAPI_AddDrv",                      ProgElemType::FUNCTION },
    { 0x80058A38, 0x80058A48, "LIBAPI_DelDrv",                      ProgElemType::FUNCTION },
};

const ProgElem*     gProgramElems_Doom = ELEMS;
const uint32_t      gNumProgramElems_Doom = sizeof(ELEMS) / sizeof(ProgElem);

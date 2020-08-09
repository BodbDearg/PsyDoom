#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Config)

// Game settings
const char*     getCueFilePath() noexcept;
extern bool     gbUncapFramerate;
extern int32_t  gUsePalTimings;
extern bool     gbUseDemoTimings;
extern bool     gbUseMoveInputLatencyTweak;
extern bool     gbUsePlayerRocketBlastFix;
extern int32_t  gLostSoulSpawnLimit;

// Video settings
extern bool     gbFullscreen;
extern bool     gbFloorRenderGapFix;

// Input settings
extern float    gMouseTurnSpeed;
extern float    gGamepadDeadZone;
extern float    gGamepadFastTurnSpeed_High;
extern float    gGamepadFastTurnSpeed_Low;
extern float    gGamepadTurnSpeed_High;
extern float    gGamepadTurnSpeed_Low;
extern float    gAnalogToDigitalThreshold;

void init() noexcept;
void shutdown() noexcept;

END_NAMESPACE(IniUtils)

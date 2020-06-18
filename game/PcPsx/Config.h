#include "Macros.h"

BEGIN_NAMESPACE(Config)

// Game settings
extern bool     gbUncapFramerate;

// Video settings
extern bool     gbFullscreen;

// Input settings
extern float    gMouseTurnSpeed;
extern float    gGamepadDeadZone;
extern float    gGamepadFastTurnSpeed_High;
extern float    gGamepadFastTurnSpeed_Low;
extern float    gGamepadTurnSpeed_High;
extern float    gGamepadTurnSpeed_Low;

void init() noexcept;
void shutdown() noexcept;

END_NAMESPACE(IniUtils)

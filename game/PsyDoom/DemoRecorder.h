#pragma once

#include "Macros.h"

BEGIN_NAMESPACE(DemoRecorder)

void begin() noexcept;
void end() noexcept;
bool isRecording() noexcept;
void recordTick() noexcept;

END_NAMESPACE(DemoRecorder)

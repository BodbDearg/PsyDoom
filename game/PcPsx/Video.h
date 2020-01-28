#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Video related stuff for the PC-PSX port
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Macros.h"

struct SDL_Window;

BEGIN_NAMESPACE(PcPsx)

void initVideo() noexcept;
void shutdownVideo() noexcept;
void doFrameRateLimiting() noexcept;    // Waits until at least a 30 Hz tick has elapsed relative to the previous call
void displayFramebuffer() noexcept;     // Display the currently displaying PSX framebuffer
SDL_Window* getWindow() noexcept;

END_NAMESPACE(PcPsx)

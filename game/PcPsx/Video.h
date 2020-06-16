#pragma once

#include "Macros.h"

struct SDL_Window;

BEGIN_NAMESPACE(Video)

void initVideo() noexcept;
void shutdownVideo() noexcept;
void displayFramebuffer() noexcept;     // Display the currently displaying PSX framebuffer
SDL_Window* getWindow() noexcept;

END_NAMESPACE(Video)

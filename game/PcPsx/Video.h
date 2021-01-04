#pragma once

#include "Macros.h"

struct SDL_Window;

BEGIN_NAMESPACE(Video)

extern bool bIsVulkanSupported;

void initVideo() noexcept;
void shutdownVideo() noexcept;
void displayPsxFramebuffer() noexcept;
SDL_Window* getWindow() noexcept;

END_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Video related stuff for the PC-PSX port
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Macros.h"

struct SDL_Window;

BEGIN_NAMESPACE(PcPsx)

void initVideo() noexcept;
void shutdownVideo() noexcept;
void displayFramebuffer() noexcept;     // Display the currently displaying PSX framebuffer
SDL_Window* getWindow() noexcept;

END_NAMESPACE(PcPsx)

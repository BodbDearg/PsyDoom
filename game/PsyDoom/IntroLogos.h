#pragma once

#include "Macros.h"

namespace LogoPlayer {
    struct Logo;
}

BEGIN_NAMESPACE(IntroLogos)

void init() noexcept;
void shutdown() noexcept;

LogoPlayer::Logo getSonyIntroLogo() noexcept;
LogoPlayer::Logo getLegalsIntroLogo() noexcept;

END_NAMESPACE(IntroLogos)

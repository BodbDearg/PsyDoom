#pragma once

#include "LogoPlayer.h"

BEGIN_NAMESPACE(IntroLogos)

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a fixed size list of logos.
// If logo entries in the list are not valid or defined then they should just be skipped/ignored.
// The list is just large enough to accomodate the needs of various different scenarios - expand if required.
//------------------------------------------------------------------------------------------------------------------------------------------
struct LogoList {
    LogoPlayer::Logo logos[2];
};

void init() noexcept;
void shutdown() noexcept;

LogoPlayer::Logo getSonyLogo() noexcept;
LogoList getLegalLogos() noexcept;

END_NAMESPACE(IntroLogos)

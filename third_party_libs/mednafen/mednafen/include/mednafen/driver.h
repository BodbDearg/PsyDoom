#ifndef __MDFN_DRIVER_H
#define __MDFN_DRIVER_H

// DC: These got moved to workaround issues with no symlinks on Windows
#if 1
    #include <mednafen/mednafen.h>
    #include <mednafen/mednafen-driver.h>
    #include <mednafen/mempatcher-driver.h>
    #include <mednafen/movie-driver.h>
    #include <mednafen/netplay-driver.h>
    #include <mednafen/settings-driver.h>
    #include <mednafen/state-driver.h>
    #include <mednafen/video-driver.h>
#else
    #include "mednafen.h"
    #include "mednafen-driver.h"
    #include "mempatcher-driver.h"
    #include "movie-driver.h"
    #include "netplay-driver.h"
    #include "settings-driver.h"
    #include "state-driver.h"
    #include "video-driver.h"
#endif

#endif

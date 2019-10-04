#ifndef __MDFN_VIDEO_DRIVER_H
#define __MDFN_VIDEO_DRIVER_H

// DC: This got moved to workaround issues with no symlinks on Windows
#if 1
    #include <mednafen/video.h>
#else
    #include "video.h"
#endif

namespace Mednafen
{
void MDFND_DispMessage(char* text);
void MDFNI_SaveSnapshot(const MDFN_Surface *src, const MDFN_Rect *rect, const int32 *LineWidths);
}

#endif

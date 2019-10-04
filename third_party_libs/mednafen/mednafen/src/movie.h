#ifndef __MDFN_MOVIE_H
#define __MDFN_MOVIE_H

namespace Mednafen
{
class MemoryStream;
}

// DC: These got moved to workaround issues with no symlinks on Windows
#if 1
    #include <mednafen/state.h>
    #include <mednafen/movie-driver.h>
#else
    #include "state.h"
    #include "movie-driver.h"
#endif

namespace Mednafen
{
void MDFNMOV_ProcessInput(uint8 *PortData[], uint32 PortLen[], int NumPorts) noexcept;
void MDFNMOV_Stop(void) noexcept;
void MDFNMOV_AddCommand(uint8 cmd, uint32 data_len = 0, uint8* data = NULL) noexcept;
bool MDFNMOV_IsPlaying(void) noexcept;
bool MDFNMOV_IsRecording(void) noexcept;
void MDFNMOV_RecordState(void) noexcept;

// For state rewinding only.
void MDFNMOV_StateAction(StateMem* sm, const unsigned load);

void MDFNI_SelectMovie(int);
void MDFNMOV_CheckMovies(void);

}
#endif

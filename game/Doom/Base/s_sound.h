#pragma once

#include <cstdint>

enum sfxenum_t : int32_t;
struct mobj_t;
struct SampleBlock;

// Max DOOM volume level (0-100)
const int32_t S_MAX_VOL = 100;

// Enum for a CD music piece and also an index into the 'gCDTrackNum' array to get an actual track number
enum cdmusic_t : int32_t {
    cdmusic_title_screen,
    cdmusic_main_menu,
    cdmusic_credits_demo,
    cdmusic_intermission,
    cdmusic_club_doom,
    cdmusic_finale_doom1,
    cdmusic_finale_doom2,
    NUM_CD_MUSIC_TRACKS
};

extern const uint32_t gCDTrackNum[NUM_CD_MUSIC_TRACKS];

extern int32_t gCdMusicVol;

int32_t doomToWessVol(const int32_t doomVol) noexcept;
int32_t doomToPsxSpuVol(const int32_t doomVol) noexcept;

void S_SetSfxVolume(int32_t sfxVol) noexcept;
void S_SetMusicVolume(const int32_t musVol) noexcept;
void S_StopMusic() noexcept;
void S_StartMusic() noexcept;
void S_InitSampleBlock(SampleBlock& block) noexcept;
void S_UnloadSampleBlock(SampleBlock& sampleBlock) noexcept;
void S_LoadMapSoundAndMusic(const int32_t mapIdx) noexcept;
void S_Pause() noexcept;
void S_Resume() noexcept;
void S_StopSound(const mobj_t* pOrigin) noexcept;
void S_StopAll() noexcept;
void S_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept;
void S_UpdateSounds() noexcept;
void PsxSoundInit(const int32_t sfxVol, const int32_t musVol, void* const pTmpWmdLoadBuffer) noexcept;
void PsxSoundExit() noexcept;

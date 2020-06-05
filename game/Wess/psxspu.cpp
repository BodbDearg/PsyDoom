//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PlayStation SPU utilities.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "psxspu.h"

#include "PcPsx/ProgArgs.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBSPU.h"
#include "wessarc.h"

// How much SPU RAM there is in total (512 KiB)
static constexpr uint32_t SPU_RAM_SIZE = 512 * 1024;

// This much of SPU RAM is reserved at the start for the 4 capture buffers, 2 of which are used for CD Audio left/right samples.
static constexpr uint32_t SPU_CAPTURE_BUFFERS_SIZE = 4 * 1024;

// How much SPU RAM is usable if no reverb is used.
// Reverb may require additional SPU RAM to be marked off limits, for the reverb work area.
static constexpr uint32_t MAX_USABLE_SPU_RAM = SPU_RAM_SIZE - SPU_CAPTURE_BUFFERS_SIZE;

// Dummy memory manager book keeping area used by 'LIBSPU_SpuInitMalloc' and LIBSPU malloc functions.
// The SPU malloc functions are not used at all in DOOM, so this area will be unused.
static constexpr uint32_t MAX_SPU_ALLOCS = 1;
static constexpr uint32_t SPU_MALLOC_RECORDS_SIZE = SPU_MALLOC_RECSIZ * (MAX_SPU_ALLOCS + 1);

static uint8_t gPsxSpu_SpuMallocRecords[SPU_MALLOC_RECORDS_SIZE];

// Is this module initialized?
static bool gbPsxSpu_initialized;

// The end offset into usable SPU RAM.
// Note: 4 KiB must be reserved at the start of SPU RAM for the 4 capture buffers, 2 of which are used for CD Audio left/right samples.
uint32_t gPsxSpu_sram_end = MAX_USABLE_SPU_RAM;

// If true then we process the 'psxspu_fadeengine' timer callback, otherwise the callback is ignored.
// The timer callback was originally triggered via periodic timer interrupts, so this flag was used to temporarily ignore interrupts.
// It's seen throughout this code guarding sections where we do not want interrupts.
static bool gbPsxSpu_timer_callback_enabled;

// How many ticks for master and cd fade out are remaining, with a tick being decremented each time the timer callback is triggered.
// Each tick is approximately 1/120 seconds - not precisely though.
static int32_t  gPsxSpu_master_fade_ticks_left;
static int32_t  gPsxSpu_cd_fade_ticks_left;

// Master and cdrom audio volume levels (integer and fixed point)
static int32_t  gPsxSpu_master_vol           = PSXSPU_MAX_MASTER_VOL;
static int32_t  gPsxSpu_master_vol_fixed     = PSXSPU_MAX_MASTER_VOL << 16;      // Master volume in 16.16 format
static int32_t  gPsxSpu_cd_vol               = PSXSPU_MAX_CD_VOL;
static int32_t  gPsxSpu_cd_vol_fixed         = PSXSPU_MAX_CD_VOL << 16;          // CD volume in 16.16 format

// Master and cdrom destination volume levels and increment/decerement stepping for fading
static int32_t  gPsxSpu_master_destvol_fixed;
static int32_t  gPsxSpu_master_fadestep_fixed;
static int32_t  gPsxSpu_cd_destvol_fixed;
static int32_t  gPsxSpu_cd_fadestep_fixed;

// Current reverb settings
static SpuReverbAttr gPsxSpu_rev_attr;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize reverb to the specified settings
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_init_reverb(
    const SpuReverbMode reverbMode,
    const int16_t depthLeft,
    const int16_t depthRight,
    const int32_t delay,
    const int32_t feedback
) noexcept {
    gbPsxSpu_timer_callback_enabled = false;

    gPsxSpu_rev_attr.mask = SPU_REV_MODE | SPU_REV_DEPTHL | SPU_REV_DEPTHR | SPU_REV_DELAYTIME | SPU_REV_FEEDBACK;
    gPsxSpu_rev_attr.mode = (SpuReverbMode)(reverbMode | SPU_REV_MODE_CLEAR_WA);
    gPsxSpu_rev_attr.depth.left = depthLeft;
    gPsxSpu_rev_attr.depth.right = depthRight;
    gPsxSpu_rev_attr.delay = delay;
    gPsxSpu_rev_attr.feedback = feedback;

    LIBSPU_SpuSetReverbModeParam(gPsxSpu_rev_attr);
    LIBSPU_SpuSetReverbDepth(gPsxSpu_rev_attr);
    const bool bReverbEnabled = (reverbMode != SPU_REV_MODE_OFF);

    if (bReverbEnabled) {
        LIBSPU_SpuSetReverb(SPU_ON);
        gPsxSpu_sram_end = LIBSPU_SpuGetReverbOffsetAddr();     // Reverb reduces the available SPU RAM because it needs a work area
    } else {
        LIBSPU_SpuSetReverb(SPU_OFF);
        gPsxSpu_sram_end = MAX_USABLE_SPU_RAM;      // No reverb, so the amount usable is the max usable SPU RAM amount
    }

    LIBSPU_SpuSetReverbVoice(bReverbEnabled, SPU_ALLCH);
    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the reverb strength for the left and right channels
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_reverb_depth(const int16_t depthLeft, const int16_t depthRight) noexcept {
    gbPsxSpu_timer_callback_enabled = false;

    gPsxSpu_rev_attr.depth.left = depthLeft;
    gPsxSpu_rev_attr.depth.right = depthRight;
    LIBSPU_SpuSetReverbDepth(gPsxSpu_rev_attr);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the PlayStation SPU and the SPU handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_init() noexcept {
    if (gbPsxSpu_initialized)
        return;
    
    gbPsxSpu_timer_callback_enabled = false;

    LIBSPU_SpuInit();
    gbPsxSpu_initialized = true;

    // Note: 'SpuMalloc' is not used AT ALL, so this call could have probably been removed...
    LIBSPU_SpuInitMalloc(MAX_SPU_ALLOCS, gPsxSpu_SpuMallocRecords);
    LIBSPU_SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    psxspu_init_reverb(SPU_REV_MODE_OFF, 0, 0, 0, 0);

    // Set default volume levels and mixing settings
    SpuCommonAttr soundAttribs;
    soundAttribs.mask = (
        SPU_COMMON_MVOLL | SPU_COMMON_MVOLR |
        SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR |
        SPU_COMMON_CDREV | SPU_COMMON_CDMIX
    );
    soundAttribs.mvol.left = PSXSPU_MAX_MASTER_VOL;
    soundAttribs.mvol.right = PSXSPU_MAX_MASTER_VOL;
    soundAttribs.cd.volume.left = PSXSPU_MAX_CD_VOL;
    soundAttribs.cd.volume.right = PSXSPU_MAX_CD_VOL;
    soundAttribs.cd.reverb = false;
    soundAttribs.cd.mix = true;

    LIBSPU_SpuSetCommonAttr(soundAttribs);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal function: set the master volume for the SPU (directly)
//------------------------------------------------------------------------------------------------------------------------------------------
static void psxspu_set_master_volume(const int32_t vol) noexcept {
    gbPsxSpu_timer_callback_enabled = false;
    
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
    attribs.mvol.left = (int16_t) vol;
    attribs.mvol.right = (int16_t) vol;
    LIBSPU_SpuSetCommonAttr(attribs);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal function: set the master volume for cd audio on the SPU (directly)
//------------------------------------------------------------------------------------------------------------------------------------------
static void psxspu_set_cd_volume(const int32_t vol) noexcept {
    gbPsxSpu_timer_callback_enabled = false;
    
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
    attribs.cd.volume.left = (int16_t) vol;
    attribs.cd.volume.right = (int16_t) vol;
    LIBSPU_SpuSetCommonAttr(attribs);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable mixing of cd audio into the sound output
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_setcdmixon() noexcept {
    gbPsxSpu_timer_callback_enabled = false;
  
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDMIX;
    attribs.cd.mix = true;
    LIBSPU_SpuSetCommonAttr(attribs);
  
    gbPsxSpu_timer_callback_enabled =  true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disable mixing of cd audio into the sound output
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_setcdmixoff() noexcept {
    gbPsxSpu_timer_callback_enabled = false;

    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDMIX;
    attribs.cd.mix = false;
    LIBSPU_SpuSetCommonAttr(attribs);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A callback that is invoked periodically (at approximately at 120 Hz intervals) in PSX DOOM to do audio fades.
// For the real PlayStation DOOM this callback is triggered via interrupts coming from hardware timers.
// In this emulated environment we simply ensure it is called periodically.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_fadeengine() noexcept {
    // Ignore this callback if temporarily disabled
    if (!gbPsxSpu_timer_callback_enabled)
        return;

    // Do cd audio fades
    if (gPsxSpu_cd_fade_ticks_left > 0) {
        gPsxSpu_cd_fade_ticks_left--;
        gPsxSpu_cd_vol_fixed += gPsxSpu_cd_fadestep_fixed;

        // If we've reached the end ensure we haven't gone past the desired value by just setting explicitly
        if (gPsxSpu_cd_fade_ticks_left == 0) {
            gPsxSpu_cd_vol_fixed = gPsxSpu_cd_destvol_fixed;
        }

        gPsxSpu_cd_vol = gPsxSpu_cd_vol_fixed >> 16;
        psxspu_set_cd_volume(gPsxSpu_cd_vol);
    }

    // Do master volume fades
    if (gPsxSpu_master_fade_ticks_left > 0) {
        gPsxSpu_master_fade_ticks_left--;
        gPsxSpu_master_vol_fixed += gPsxSpu_master_fadestep_fixed;

        // If we've reached the end ensure we haven't gone past the desired value by just setting explicitly
        if (gPsxSpu_master_fade_ticks_left == 0) {
            gPsxSpu_master_vol_fixed = gPsxSpu_master_destvol_fixed;
        }

        gPsxSpu_master_vol = gPsxSpu_master_vol_fixed >> 16;
        psxspu_set_master_volume(gPsxSpu_master_vol);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the current cd audio volume and disable any fades on cd volume that are active
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_cd_vol(const int32_t vol) noexcept {
    gbPsxSpu_timer_callback_enabled = false;

    gPsxSpu_cd_vol = vol;
    gPsxSpu_cd_vol_fixed = vol << 16;
    gPsxSpu_cd_fade_ticks_left = 0;
    psxspu_set_cd_volume(vol);

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current (integer) volume for cd audio
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxspu_get_cd_vol() noexcept {
    return gPsxSpu_cd_vol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin doing a fade of cd music to the specified volume in the specified amount of time
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_start_cd_fade(const int32_t fadeTimeMs, const int32_t destVol) noexcept {
    // PC-PSX: ignore the command in headless mode
    #if PC_PSX_DOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    gbPsxSpu_timer_callback_enabled = false;

    if (*gbWess_WessTimerActive) {
        // Note: the timer callback fires at approximately 120 Hz, hence convert from MS to a 120 Hz tick count here
        gPsxSpu_cd_fade_ticks_left = (fadeTimeMs * 120) / 1000 + 1;
        gPsxSpu_cd_destvol_fixed = destVol * 0x10000;
        gPsxSpu_cd_fadestep_fixed = (gPsxSpu_cd_destvol_fixed - gPsxSpu_cd_vol_fixed) / gPsxSpu_cd_fade_ticks_left;
    } else {
        // If the timer callback is not active then skip doing any fade since there is no means of doing it
        gPsxSpu_cd_fade_ticks_left = 0;
    }

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop doing a fade of cd music
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_stop_cd_fade() noexcept {
    gbPsxSpu_timer_callback_enabled = false;
    gPsxSpu_cd_fade_ticks_left = 0;
    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns 'true' if the cd audio fade out is still ongoing, 'false' otherwise
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxspu_get_cd_fade_status() noexcept {
    return (gPsxSpu_cd_fade_ticks_left > 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the master volume level
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_master_vol(const int32_t vol) noexcept {
    gbPsxSpu_timer_callback_enabled = 0;
    
    gPsxSpu_master_vol = vol;
    gPsxSpu_master_vol_fixed = vol << 16;
    gPsxSpu_master_fade_ticks_left = 0;
    psxspu_set_master_volume(gPsxSpu_master_vol);

    gbPsxSpu_timer_callback_enabled = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the master volume level
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxspu_get_master_vol() noexcept {
    return gPsxSpu_master_vol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin doing a fade of master volume to the specified volume in the specified amount of time
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_start_master_fade(const int32_t fadeTimeMs, const int32_t destVol) noexcept {
    gbPsxSpu_timer_callback_enabled = false;

    if (*gbWess_WessTimerActive) {
        // Note: the timer callback fires at approximately 120 Hz, hence convert from MS to a 120 Hz tick count here
        gPsxSpu_master_fade_ticks_left = (fadeTimeMs * 120) / 1000 + 1;
        gPsxSpu_master_destvol_fixed = destVol << 16;
        gPsxSpu_master_fadestep_fixed = (gPsxSpu_master_destvol_fixed - gPsxSpu_master_vol_fixed) / gPsxSpu_master_fade_ticks_left;
    } else {
        // If the timer callback is not active then skip doing any fade since there is no means of doing it
        gPsxSpu_master_fade_ticks_left = 0;
    }

    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop doing a fade out of the master volume
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_stop_master_fade() noexcept {
    // Note: disabling callback processing before setting the tick count - in case an interrupt happens
    gbPsxSpu_timer_callback_enabled = false;
    gPsxSpu_master_fade_ticks_left = 0;
    gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns 'true' if the master volume fade out is still ongoing, 'false' otherwise
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxspu_get_master_fade_status() noexcept {
    return (gPsxSpu_master_fade_ticks_left > 1);
}

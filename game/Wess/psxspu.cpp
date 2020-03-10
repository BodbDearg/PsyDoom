//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PlayStation SPU utilities.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PSXSPU.h"

#include "PcPsx/Types.h"
#include "PsxVm/PsxVm.h"
#include "PsxVm/VmPtr.h"
#include "PsyQ/LIBSPU.h"
#include "wessarc.h"

// Is this module initialized?
static const VmPtr<bool32_t> gbPsxSpu_initialized(0x80075984);

// Dummy memory manager book keeping area used by 'LIBSPU_SpuInitMalloc' and LIBSPU malloc functions.
// The SPU malloc functions are not used at all in DOOM, so this area will be unused.
static constexpr uint32_t MAX_SPU_ALLOCS = 1;
static constexpr uint32_t SPU_MALLOC_RECORDS_SIZE = SPU_MALLOC_RECSIZ * (MAX_SPU_ALLOCS + 1);

static const VmPtr<uint8_t[SPU_MALLOC_RECORDS_SIZE]> gPsxSpu_SpuMallocRecords(0x800A9508);

// If true then we process the 'psxspu_fadeengine' timer callback, otherwise the callback is ignored.
// The timer callback was originally triggered via periodic timer interrupts, so this flag was used to temporarily ignore interrupts.
// It's seen throughout this code guarding sections where we do not want interrupts.
static const VmPtr<bool32_t> gbPsxSpu_timer_callback_enabled(0x80075988);

// How many ticks for master and cd fade out are remaining, with a tick being decremented each time the timer callback is triggered.
// Each tick is approximately 1/120 seconds - not precisely though.
static const VmPtr<int32_t> gPsxSpu_master_fade_ticks_left(0x80075994);
static const VmPtr<int32_t> gPsxSpu_cd_fade_ticks_left(0x800759A8);

// Master and cdrom audio volume levels (integer and fixed point)
static const VmPtr<int32_t> gPsxSpu_master_vol(0x80075990);
static const VmPtr<int32_t> gPsxSpu_master_vol_fixed(0x80075998);
static const VmPtr<int32_t> gPsxSpu_cd_vol(0x800759A4);
static const VmPtr<int32_t> gPsxSpu_cd_vol_fixed(0x800759AC);

// Destination volume levels and increment/decerement stepping for fading
static const VmPtr<int32_t> gPsxSpu_cd_destvol_fixed(0x800759B0);
static const VmPtr<int32_t> gPsxSpu_cd_fadestep_fixed(0x800759B4);

// Current reverb settings
static const VmPtr<SpuReverbAttr> gPsxSpu_rev_attr(0x8007f080);

// The end address of usable SPU RAM.
// This can vary depending on the reverb mode - some reverb modes require more RAM than others.
static const VmPtr<uint32_t> gPsxSpu_sram_end(0x8007598C);

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
    *gbPsxSpu_timer_callback_enabled = false;

    gPsxSpu_rev_attr->mask = SPU_REV_MODE | SPU_REV_DEPTHL | SPU_REV_DEPTHR | SPU_REV_DELAYTIME | SPU_REV_FEEDBACK;
    gPsxSpu_rev_attr->mode = (SpuReverbMode)(reverbMode | SPU_REV_MODE_CLEAR_WA);    
    gPsxSpu_rev_attr->depth.left = depthLeft;
    gPsxSpu_rev_attr->depth.right = depthRight;
    gPsxSpu_rev_attr->delay = delay;
    gPsxSpu_rev_attr->feedback = feedback;

    LIBSPU_SpuSetReverbModeParam(*gPsxSpu_rev_attr);
    LIBSPU_SpuSetReverbDepth(*gPsxSpu_rev_attr);
    const bool bReverbEnabled = (reverbMode != SPU_REV_MODE_OFF);

    if (bReverbEnabled) {
        LIBSPU_SpuSetReverb(SPU_ON);
        *gPsxSpu_sram_end = LIBSPU_SpuGetReverbOffsetAddr();
    } else {
        LIBSPU_SpuSetReverb(SPU_OFF);
        *gPsxSpu_sram_end = 0x7F000;
    }

    LIBSPU_SpuSetReverbVoice(bReverbEnabled, SPU_ALLCH);
    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the reverb strength for the left and right channels
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_reverb_depth(const int16_t depthLeft, const int16_t depthRight) noexcept {
    *gbPsxSpu_timer_callback_enabled = false;

    gPsxSpu_rev_attr->depth.left = depthLeft;
    gPsxSpu_rev_attr->depth.right = depthRight;
    LIBSPU_SpuSetReverbDepth(*gPsxSpu_rev_attr);

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the PlayStation SPU and the SPU handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_init() noexcept {
    if (*gbPsxSpu_initialized)
        return;
    
    *gbPsxSpu_timer_callback_enabled = false;

    LIBSPU_SpuInit();
    *gbPsxSpu_initialized = true;

    // Note: 'SpuMalloc' is not used AT ALL, so this call could have probably been removed...
    LIBSPU_SpuInitMalloc(MAX_SPU_ALLOCS, gPsxSpu_SpuMallocRecords.get());
    LIBSPU_SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    psxspu_init_reverb(SPU_REV_MODE_OFF, 0, 0, 0, 0);

    // Set default volume levels and mixing settings
    SpuCommonAttr soundAttribs;
    soundAttribs.mask = (
        SPU_COMMON_MVOLL | SPU_COMMON_MVOLR |
        SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR |
        SPU_COMMON_CDREV | SPU_COMMON_CDMIX
    );
    soundAttribs.mvol.left = MAX_MASTER_VOL;
    soundAttribs.mvol.right = MAX_MASTER_VOL;
    soundAttribs.cd.volume.left = MAX_CD_VOL;
    soundAttribs.cd.volume.right = MAX_CD_VOL;
    soundAttribs.cd.reverb = false;
    soundAttribs.cd.mix = true;

    LIBSPU_SpuSetCommonAttr(soundAttribs);

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal function: set the master volume for the SPU (directly)
//------------------------------------------------------------------------------------------------------------------------------------------
static void psxspu_set_master_volume(const int32_t vol) noexcept {
    *gbPsxSpu_timer_callback_enabled = false;
    
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
    attribs.mvol.left = (int16_t) vol;
    attribs.mvol.right = (int16_t) vol;
    LIBSPU_SpuSetCommonAttr(attribs);

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal function: set the master volume for cd audio on the SPU (directly)
//------------------------------------------------------------------------------------------------------------------------------------------
static void psxspu_set_cd_volume(const int32_t vol) noexcept {
    *gbPsxSpu_timer_callback_enabled = false;
    
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
    attribs.cd.volume.left = (int16_t) vol;
    attribs.cd.volume.right = (int16_t) vol;
    LIBSPU_SpuSetCommonAttr(attribs);

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable mixing of cd audio into the sound output
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_setcdmixon() noexcept {
    *gbPsxSpu_timer_callback_enabled = false;
  
    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDMIX;
    attribs.cd.mix = true;
    LIBSPU_SpuSetCommonAttr(attribs);
  
    *gbPsxSpu_timer_callback_enabled =  true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disable mixing of cd audio into the sound output
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_setcdmixoff() noexcept {
    *gbPsxSpu_timer_callback_enabled = false;

    SpuCommonAttr attribs;
    attribs.mask = SPU_COMMON_CDMIX;
    attribs.cd.mix = false;
    LIBSPU_SpuSetCommonAttr(attribs);

    *gbPsxSpu_timer_callback_enabled = true;
}

void psxspu_fadeengine() noexcept {
loc_8004560C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5988);                               // Load from: 80075988
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80045710;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x59A8);                               // Load from: 800759A8
    {
        const bool bJump = (i32(v1) <= 0);
        v1--;
        if (bJump) goto loc_80045698;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59AC);                               // Load from: 800759AC
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59B4);                               // Load from: 800759B4
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x59A8);                                // Store to: 800759A8
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
    if (v1 != 0) goto loc_80045674;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59B0);                               // Load from: 800759B0
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
loc_80045674:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x59AE);                               // Load from: 800759AE
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A4);                                // Store to: 800759A4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x59A4);                               // Load from: 800759A4
    psxspu_set_cd_volume(a0);
loc_80045698:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5994);                               // Load from: 80075994
    {
        const bool bJump = (i32(v1) <= 0);
        v1--;
        if (bJump) goto loc_80045710;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59A0);                               // Load from: 800759A0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
    if (v1 != 0) goto loc_800456EC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x599C);                               // Load from: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
loc_800456EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x599A);                               // Load from: 8007599A
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5990);                                // Store to: 80075990
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x5990);                               // Load from: 80075990
    psxspu_set_master_volume(a0);
loc_80045710:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the current cd audio volume and disable any fades on cd volume that are active
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_cd_vol(const int32_t vol) noexcept {
    *gbPsxSpu_timer_callback_enabled = false;

    *gPsxSpu_cd_vol = vol;
    *gPsxSpu_cd_vol_fixed = vol << 16;
    *gPsxSpu_cd_fade_ticks_left = 0;    
    psxspu_set_cd_volume(vol);

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current (integer) volume for cd audio
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxspu_get_cd_vol() noexcept {
    return *gPsxSpu_cd_vol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin doing a fade of cd music to the specified volume in the specified amount of time
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_start_cd_fade(const int32_t fadeTimeMs, const int32_t destVol) noexcept {
    *gbPsxSpu_timer_callback_enabled = false;

    if (*gbWess_WessTimerActive) {
        // Note: the timer callback fires at approximately 120 Hz, hence convert from MS to a 120 Hz tick count here
        *gPsxSpu_cd_fade_ticks_left = (fadeTimeMs * 120) / 1000 + 1;
        *gPsxSpu_cd_destvol_fixed = destVol * 0x10000;
        *gPsxSpu_cd_fadestep_fixed = (gPsxSpu_cd_destvol_fixed - gPsxSpu_cd_vol_fixed) / gPsxSpu_cd_fade_ticks_left;

        
    } else {
        // If the timer callback is not active then skip doing any fade since there is no means of doing it
        *gPsxSpu_cd_fade_ticks_left = 0;
    }

    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop doing a fade of cd music
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_stop_cd_fade() noexcept {
    *gbPsxSpu_timer_callback_enabled = false;
    *gPsxSpu_cd_fade_ticks_left = 0;
    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns 'true' if the cd audio fade out is still ongoing, 'false' otherwise
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxspu_get_cd_fade_status() noexcept {
    // Emulate sound a little in case calling code is polling in a loop waiting for changed spu status
    #if PC_PSX_DOOM_MODS
        emulate_sound_if_required();
    #endif

    return (*gPsxSpu_cd_fade_ticks_left > 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the master volume level
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_set_master_vol(const int32_t vol) noexcept {
    *gbPsxSpu_timer_callback_enabled = 0;
    
    *gPsxSpu_master_vol = vol;
    *gPsxSpu_master_vol_fixed = vol << 16;
    *gPsxSpu_master_fade_ticks_left = 0;
    psxspu_set_master_volume(*gPsxSpu_master_vol);

    *gbPsxSpu_timer_callback_enabled = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the master volume level
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxspu_get_master_vol() noexcept {
    return *gPsxSpu_master_vol;
}

void psxspu_start_master_fade() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    v1 = 0x10620000;                                    // Result = 10620000
    if (v0 == 0) goto loc_80045988;
    v1 |= 0x4DD3;                                       // Result = 10624DD3
    v0 = a0 << 4;
    v0 -= a0;
    v0 <<= 3;
    mult(v0, v1);
    v0 = u32(i32(v0) >> 31);
    a0 = a1 << 16;
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    v1++;
    v0 = a0 - v0;
    div(v0, v1);
    if (v1 != 0) goto loc_8004594C;
    _break(0x1C00);
loc_8004594C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80045964;
    }
    if (v0 != at) goto loc_80045964;
    _break(0x1800);
loc_80045964:
    v0 = lo;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x599C);                                // Store to: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A0);                                // Store to: 800759A0
    v0 = 1;                                             // Result = 00000001
    goto loc_80045994;
loc_80045988:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    v0 = 1;                                             // Result = 00000001
loc_80045994:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop doing a fade out of the master volume
//------------------------------------------------------------------------------------------------------------------------------------------
void psxspu_stop_master_fade() noexcept {
    // Note: disabling callback processing before setting the tick count - in case an interrupt happens
    *gbPsxSpu_timer_callback_enabled = false;
    *gPsxSpu_master_fade_ticks_left = 0;
    *gbPsxSpu_timer_callback_enabled = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns 'true' if the master volume fade out is still ongoing, 'false' otherwise
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxspu_get_master_fade_status() noexcept {    
    return (*gPsxSpu_master_fade_ticks_left > 1);
}

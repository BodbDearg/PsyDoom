#pragma once

#include <cstdint>
#include <cstddef>

//------------------------------------------------------------------------------------------------------------------------------------------
// PlayStation SPU emulation: simplified.
//
//  A stripped down emulation of a PlayStation 1 SPU, and potentially most of the PS2 SPU if the voice and RAM limits are increased.
//  Implements the most commonly used functionality of the SPU, and specifically all the functionality required by PlayStation Doom.
//  It is completely self isolated (apart from supplied external inputs) and can safely run in a separate thread.
//
//  What was removed from this SPU emulation:
//      - All links to a host system, interrupts, system bus reading/writing, DMA etc.
//      - Noise generation
//      - Voice pitch modulation by another voice (used for LFO style effects)
//      - Sweep volume mode for SPU voices
//------------------------------------------------------------------------------------------------------------------------------------------
namespace Spu {
    static constexpr int32_t ADPCM_BLOCK_SIZE           = 16;   // The size in bytes of a PSX format ADPCM block
    static constexpr int32_t ADPCM_BLOCK_NUM_SAMPLES    = 28;   // The number of samples in a PSX format ADPCM block
    
    // Flags read from the 2nd byte of a PSX ADPCM block.
    //
    // Meanings:
    //  LOOP_END:       If set then goto the repeat address after we are finished with the current ADPCM block
    //  REPEAT:         Only used if 'LOOP_END' is set, whether we are repeating normally or silencing the voice.
    //                  If NOT set when reaching a sample end , then the volumne envelope is immediately silenced.
    //  LOOP_START:     If set then save the current ADPCM address as the repeat address
    //  
    static constexpr uint8_t ADPCM_FLAG_LOOP_END    = 0x01;
    static constexpr uint8_t ADPCM_FLAG_REPEAT      = 0x02;
    static constexpr uint8_t ADPCM_FLAG_LOOP_START  = 0x04;

    // Holds information about where we are sampling from in a block of ADPCM samples.
    union AdpcmBlockPos {
        // These are what the bits of the counter mean, starting with the least significant bits
        struct {
            uint32_t gaussIdxFrac   : 4;    // Lower fractional bits of the gauss index
            uint32_t gaussIdx       : 8;    // Gauss index: this is an index into an interpolation table for interpolating the 4 most recent samples
            uint32_t sampleIdx      : 20;   // Index of the current sample in the ADPCM block. Once this exceeds '28' we need to read more ADPCM blocks
        };

        // This is simply incremented by the pitch of the voice.
        // If incremented by '0x1000' then it means we are playing the sample @ 44.1 KHz since the SPU samples at that rate and
        // discarding the lower 12 bits of that number leaves us with '1', or an advancement of 1 sample per 44.1 KHz SPU sample.
        uint32_t counter;
    };

    // Stores the settings for a voice ADSR envelope.
    // Note: this is same bit layout that the PSX spu uses also.
    struct AdsrEnvelope {
        // 0-15: At what volume level (inclusive) do we go from the decay phase into the sustain phase.
        // The actual volume level is computed as follows: (sustainLevel + 1) * 2048
        uint32_t sustainLevel : 4;

        // 0-15: Affects how long the decay portion of the envelope lasts.
        // Lower values mean a faster decay.
        uint32_t decayShift : 4;

        // 0-3: Affects how long the attack portion of the envelope lasts; higher values mean a faster attack.
        // The actual step is computed as follows: 7 - step.
        uint32_t attackStep : 2;

        // 0-31: Affects how long the attack portion of the envelope lasts.
        // Lower values mean a faster attack.
        uint32_t attackShift : 5;

        // If set then attack mode is exponential rather than linear
        uint32_t bAttackExp : 1;

        // 0-31: Affects how long the release portion of the envelope lasts.
        // Lower values mean a faster release.
        uint32_t releaseShift : 5;

        // If set then release mode is exponential rather than linear
        uint32_t bReleaseExp : 1;

        // How much to step the envelope in sustain mode.
        // The meaning of this depends on whether the sustain direction is 'increase' or 'decrease'.
        //  Increase: step =  7 - sustainStep
        //  Decrease: step = -8 + sustainStep
        uint32_t sustainStep : 2;

        // 0-31: Affects how long the sustain portion of the envelope lasts.
        // Lower values mean a faster sustain.
        uint32_t sustainShift : 5;

        // An unused bit of the envelope
        uint32_t _unused: 1;

        // If set then the sustain envelope is decreased over time.
        // If NOT set then it increases.
        uint32_t bSustainDec : 1;

        // Whether the sustain portion of the envelope increases or decreases exponentially.
        // If set then the change is exponential.
        uint32_t bSustainExp : 1;
    };

    // Settings/params for a particular phase of the envelope. Note that due to the way PSX envelopes work in exponential mode,
    // these parameters can sometimes change midway through the envelope phase depending on the current envelope level.
    struct EnvPhaseParams {
        int32_t targetLevel;    // What level the current envelope phase is trying to reach: -1 if not applicable
        int32_t step;           // The size of the envelope step
        int32_t stepCycles;     // How many cycles must be waited before doing an envelope step
    };

    // What phase of an envelope a voice is in
    enum class EnvPhase : uint8_t {
        Off,
        Attack,
        Decay,
        Sustain,
        Release
    };

    // Holds stereo volume levels
    struct Volume {
        int16_t left;
        int16_t right;
    };

    // Holds a stereo sample
    struct StereoSample {
        int16_t left;
        int16_t right;
    };

    // Reverb registers: determine how reverb is processed.
    // These are from the NO$PSX spec and in the same memory arrangement as the PSX.
    //
    // Notes:
    //  (1) All address offset values are in terms of 8 byte multiples; multiply by 8 to get the real offset.
    //  (2) For more details see the NO$PSX spec: https://problemkaputt.de/psx-spx.htm#soundprocessingunitspu
    //
    struct ReverbRegs {
        uint16_t    dispAPF1;       // Reverb APF Offset 1
        uint16_t    dispAPF2;       // Reverb APF Offset 2
        int16_t     volIIR;         // Reverb Reflection Volume 1
        int16_t     volComb1;       // Reverb Comb Volume 1
        int16_t     volComb2;       // Reverb Comb Volume 2
        int16_t     volComb3;       // Reverb Comb Volume 3
        int16_t     volComb4;       // Reverb Comb Volume 4
        int16_t     volWall;        // Reverb Reflection Volume 2
        int16_t     volAPF1;        // Reverb APF Volume 1
        int16_t     volAPF2;        // Reverb APF Volume 2
        uint16_t    addrLSame1;     // Reverb Same Side Reflection Address 1: Left
        uint16_t    addrRSame1;     // Reverb Same Side Reflection Address 1: Right
        uint16_t    addrLComb1;     // Reverb Comb Address 1: Left
        uint16_t    addrRComb1;     // Reverb Comb Address 1: Right
        uint16_t    addrLComb2;     // Reverb Comb Address 2: Left
        uint16_t    addrRComb2;     // Reverb Comb Address 2: Right
        uint16_t    addrLSame2;     // Reverb Same Side Reflection Address 2: Left
        uint16_t    addrRSame2;     // Reverb Same Side Reflection Address 2: Right
        uint16_t    addrLDiff1;     // Reverb Different Side Reflect Address 1: Left
        uint16_t    addrRDiff1;     // Reverb Different Side Reflect Address 1: Right
        uint16_t    addrLComb3;     // Reverb Comb Address 3: Left
        uint16_t    addrRComb3;     // Reverb Comb Address 3: Right
        uint16_t    addrLComb4;     // Reverb Comb Address 4: Left
        uint16_t    addrRComb4;     // Reverb Comb Address 4: Right
        uint16_t    addrLDiff2;     // Reverb Different Side Reflect Address 2: Left
        uint16_t    addrRDiff2;     // Reverb Different Side Reflect Address 2: Right
        uint16_t    addrLAPF1;      // Reverb APF Address 1: Left
        uint16_t    addrRAPF1;      // Reverb APF Address 1: Right
        uint16_t    addrLAPF2;      // Reverb APF Address 2: Left
        uint16_t    addrRAPF2;      // Reverb APF Address 2: Right
        int16_t     volLIn;         // Reverb Input Volume: Left
        int16_t     volRIn;         // Reverb Input Volume: Right
    };

    // Holds all of the state for a hardware SPU voice
    struct Voice {
        // How many previous decoded samples to store for an SPU voice.
        // These are required for both the sample interpolation (3 previous samples) and also by the ADPCM decoder (2 previous samples).
        static constexpr int32_t NUM_PREV_SAMPLES = 3;

        // Start address (in 8 byte units) of the current sound.
        // The current address of the voice is set from this on the 'key on' event.
        // Note: on the original PSX SPU this was a 16-bit quantity, so it could only reference up to 512 KiB of SRAM.
        uint32_t adpcmStartAddr8;

        // Current address that the voice is reading ADPCM samples from in SRAM, in 8 byte units.
        // Note: on the original PSX SPU this was a 16-bit quantity, so it could only reference up to 512 KiB of SRAM.
        uint32_t adpcmCurAddr8;

        // Repeat address (in 8 byte units) of the current sound.
        // If an ADPCM packet header has a 'loop start' flag set, then this field will be set.
        // If an ADPCM packet header has a 'loop end' flag set, then the SPU will jump to this address.
        // Note: on the original PSX SPU this was a 16-bit quantity, so it could only reference up to 512 KiB of SRAM.
        uint32_t adpcmRepeatAddr8;

        // Where we are currently in the ADPCM block
        AdpcmBlockPos adpcmBlockPos;

        // Current pitch/sample-rate of the voice.
        // A value of 0x1000 means 44,100 Hz, half that is 22,050 Hz and so on.
        uint16_t sampleRate;

        uint8_t bDisabled           : 1;    // Mix in this voice?
        uint8_t bKeyOn              : 1;    // Start the ADSR envelope and goto the sample start address?
        uint8_t bKeyOff             : 1;    // Goto the release phase of the ADSR envelope?
        uint8_t bRepeat             : 1;    // If set then goto the repeat address next time we load samples
        uint8_t bReachedLoopEnd     : 1;    // Set when we reach an ADPCM block with 'ADPCM_FLAG_LOOP_END' set and cleared on 'key on'; tells if the sample has reached the end at least once
        uint8_t bSamplesLoaded      : 1;    // If set then the voice has loaded an ADPCM sample block
        uint8_t bDoReverb           : 1;    // If set then reverb is enabled for the voice
        uint8_t _unused             : 1;    // Unused bit flag

        // Current envelope phase
        EnvPhase envPhase;

        // The ADSR envelope to use
        AdsrEnvelope env;

        // How many cycles to wait before processing the envelope.
        // This is generally always '1' but can be larger for really slow envelopes.
        int32_t envWaitCycles;

        // Left and right volume levels, divided by 2.
        // Note: I am not supporting the 'sweep volume' mode that original PSX SPU used, when the highest bit of these volume level fields was set.
        // These values are just purely fixed volume levels instead.
        Volume volume;

        // Current ADSR envelope volume level
        int16_t envLevel;

        // Previous and currently decoded 16-bit sound samples.
        // At the beginning of the buffer there is 'NUM_PREV_SAMPLES' samples, with the oldest sample first.
        int16_t samples[NUM_PREV_SAMPLES + ADPCM_BLOCK_NUM_SAMPLES];
    };

    // A callback which is invoked by the SPU to provide external input.
    // Can be used to mix in CD audio or anything else and run it through the reverb processing of the SPU.
    // The callback takes a single piece of user data and must return 1 sound sample.
    typedef StereoSample (*ExtInputCallback)(void* pUserData) noexcept;

    // The SPU core/device itself.
    // Templated so the static limits of the SPU can be raised and so it can function more like a PS2 SPU (or beyond), with increased limits.
    template <int32_t NumVoices, int32_t RamSize>
    struct Core {
        // Note: SRAM size must be multiples of 8 because voices address ram in multiples of 8
        static_assert(NumVoices > 0);
        static_assert(RamSize > 0);
        static_assert(RamSize % 8 == 0);

        Voice               voices[NumVoices];      // All of the hardware voices of the SPU
        Volume              masterVol;              // Master volume
        Volume              reverbVol;              // Reverb volume level
        Volume              extInputVol;            // External input volume (I'm using this for CD audio mixing)
        bool                bUnmute;                // If 'true' then the output from voices is mixed into the output
        bool                bReverbWriteEnable;     // Whether reverb can write output to the reverb work area
        bool                bExtEnabled;            // Whether to mix input from the external input source
        bool                bExtReverbEnable;       // Whether to apply reverb on the input from the external source
        ExtInputCallback    pExtInputCallback;      // Callback used to source external input: if null no external input is mixed with SPU voices
        void*               pExtInputUserData;      // User data passed to the external input callback
        uint32_t            cycleCount;             // How many cycles has the SPU done (44,100 == 1 second of audio): each cycle is generating a 16-bit left/right audio sample
        uint32_t            reverbBaseAddr;         // Start address of the reverb work area; anything past this address in SPU RAM is for reverb
        uint32_t            reverbCurAddr;          // Used for relative reads and writes to the reverb work area; continously incremented and wrapped as reverb is processed
        ReverbRegs          reverbRegs;             // Registers with settings determining how reverb is processed: determines the type of reverb
        std::byte           ram[RamSize];           // The SPU's sound RAM
    };

    // Typedef for PS1 and PS2 style SPU cores
    typedef Core<24, 512  * 1024> PS1Core;
    typedef Core<48, 2048 * 1024> PS2Core;
}

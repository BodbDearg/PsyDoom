#pragma once

#include "WmdFileTypes.h"

#include <functional>
#include <vector>

namespace AudioTools {
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Format of functions to read from or write to a stream; these throw if there are any errors.
    // Note that for the stream read function a destination of 'nullptr' may be passed to skip over bytes.
    //--------------------------------------------------------------------------------------------------------------------------------------
    typedef std::function<void (void* const pDst, const size_t size) noexcept(false)> StreamReadFunc;
    typedef std::function<void (const void* const pSrc, const size_t size) noexcept(false)> StreamWriteFunc;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // PSX sound driver specific data, after deserializing from a binary .WMD file or json.
    // This mirrors largely what is found in the 'wessarc.h' header of the main PsyDoom project.
    //--------------------------------------------------------------------------------------------------------------------------------------

    // This is taken from the 'SimpleSpu' implementation in 'Spu.h':
    struct PsxAdsrEnvelope {
        // 0-15: At what envelope level (inclusive) do we go from the decay phase into the sustain phase.
        // The actual envelope level is computed as follows: max((sustainLevel + 1) << 11, 0x7FFF)
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

    // Settings for one individual voice/sound in a patch/instrument
    struct PsxPatchVoice {
        uint16_t    sampleIdx;          // The index of the patch sample to use for this voice
        uint8_t     volume;             // Volume to play the voice at
        uint8_t     pan;                // Voice pan 64 to 191 - 128 is the voice center. Values outside this range are clamped by the PSX driver.
        uint8_t     baseNote;           // The 'base' note/semitone at which the sound sample is regarded to play back at 44,100 Hz (used to compute sample frequency for triggered notes)
        uint8_t     baseNoteFrac;       // The .8 fractional part of the base note
        uint8_t     noteMin;            // Minimum semitone at which the voice can be played - does not sound below this
        uint8_t     noteMax;            // Maximum semitone at which the voice can be played - does not sound above this
        uint8_t     pitchstepDown;      // How big each unit of pitch shift is when pitch shifting down (in 1/8192 units)
        uint8_t     pitchstepUp;        // How big each unit of pitch shift is when pitch shifting up (in 1/8192 units)
        uint8_t     priority;           // Voice priority: used to determine what to kill when we're out of voices

        // ADSR envelope for the voice
        union {
            PsxAdsrEnvelope     adsr;
            uint32_t            adsrBits;
        };
    };

    // Holds details for a sound sample used by a patch voice; just holds the size in bytes of the sound sample
    struct PsxPatchSample {
        uint32_t    size;
    };

    // Describes a patch/instrument: this is a collection of voices triggered in unison
    struct PsxPatch {
        uint16_t    firstVoiceIdx;      // Index of the first patch voice for the patch. Other patch voices follow contiguously in the voices list.
        uint16_t    numVoices;          // How many voices to use for the patch        
    };

    // Patches, patch voices and patch samples etc.
    struct PsxPatchGroup {
        uint8_t                         hwVoiceLimit;       // How many hardware voices there are
        std::vector<PsxPatchSample>     patchSamples;       // Samples used by patch voices
        std::vector<PsxPatchVoice>      patchVoices;        // Individual voices in a patch
        std::vector<PsxPatch>           patches;            // Patches/instruments
    };

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Generic module file data, after deserializing from a binary .WMD file or json.
    // This mirrors largely what is found in the 'wessarc.h' header of the main PsyDoom project.
    //--------------------------------------------------------------------------------------------------------------------------------------

    // Represents a sequencer command in a track.
    // Depending on the command type, none or all of the arguments may be used and also the range allowed might be capped.
    struct TrackCmd {
        // Track command types: some of these are handled by the sequencer generically, others are handled by the sound driver.
        // These are the exact same command ids found in the .WMD file.
        enum Type : uint8_t {
            // Manually called commands
            DriverInit      = 0,
            DriverExit      = 1,
            DriverEntry1    = 2,
            DriverEntry2    = 3,
            DriverEntry3    = 4,
            TrkOff          = 5,
            TrkMute         = 6,
            // Sound driver commands
            PatchChg        = 7,
            PatchMod        = 8,
            PitchMod        = 9,
            ZeroMod         = 10,
            ModuMod         = 11,
            VolumeMod       = 12,
            PanMod          = 13,
            PedalMod        = 14,
            ReverbMod       = 15,
            ChorusMod       = 16,
            NoteOn          = 17,
            NoteOff         = 18,
            // Sequencer commands
            StatusMark      = 19,
            GateJump        = 20,
            IterJump        = 21,
            ResetGates      = 22,
            ResetIters      = 23,
            WriteIterBox    = 24,
            SeqTempo        = 25,
            SeqGosub        = 26,
            SeqJump         = 27,
            SeqRet          = 28,
            SeqEnd          = 29,
            TrkTempo        = 30,
            TrkGosub        = 31,
            TrkJump         = 32,
            TrkRet          = 33,
            TrkEnd          = 34,
            NullEvent       = 35
        };

        Type        type;
        int32_t     arg1;
        int32_t     arg2;
        int32_t     arg3;
    };

    // Represents an individual track in a sequence
    struct Track {
        WmdSoundDriverId        driverId;               // What sound driver the track should play with
        WmdSoundClass           soundClass;             // What rough class of sounds the track contains
        uint16_t                initPpq;                // Parts per quarter note: how many parts to subdivide each quarter note into for timing purposes. Affects timing precision.
        uint16_t                initQpm;                // The quarter notes per minute (beats per minute) value to initialize the track status with, unless manually overridden.
        uint16_t                initPatchIdx;           // Which patch index to initially use for the track unless manually overridden
        uint16_t                initPitchCntrl;         // What pitch shift value to initially use for the track unless manually overridden
        uint8_t                 initVolumeCntrl;        // What volume to initially use for the track unless manually overridden
        uint8_t                 initPanCntrl;           // What pan setting to initially use for the track unless manually overridden
        uint8_t                 initReverb;             // The reverb level to initialize the track with
        uint8_t                 initMutegroupsMask;     // A bit mask defining what 'mute groups' the track is a part of: used for bulk muting of tracks by group
        uint8_t                 maxVoices;              // Maximum number of voices that the track claims to use
        uint8_t                 locStackSize;           // The maximum number of track data locations that can be remembered in the track's location stack (for save + return control flow)
        uint8_t                 priority;               // Used for prioritizing voices when we are out of hardware voices
        std::vector<uint32_t>   labels;                 // Locations to jump to in the track as a command index
        std::vector<TrackCmd>   cmds;                   // The commands for the track
    };

    // Represents an entire sequence (music or sfx) to be played by the sequencer
    struct Sequence {
        std::vector<Track>  tracks;
    };

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Container representing the entire of a Williams module file (.WMD file).
    // Note: I'm only supporting data structures relating to the PSX sound driver, other formats (N64, PC) will be ignored.
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct Module {
        uint8_t                 maxActiveSequences;     // The number of sequence work areas to allocate, or the maximum number of active sequences
        uint8_t                 maxActiveTracks;        // The number of track work areas to allocate, or the maximum number of active tracks
        uint8_t                 maxGatesPerSeq;         // Maximum number of 'gates' or on/off conditional jump switches per sequence
        uint8_t                 maxItersPerSeq;         // Maximum number of iteration counts (iters) per sequence; these are used to do jumps a certain number of times
        uint8_t                 maxCallbacks;           // Maximum number of user sequencer callbacks that can be used
        PsxPatchGroup           psxPatchGroup;          // PSX sound driver: patches
        std::vector<Sequence>   sequences;              // All of the sequences in the module file

        void readFromWmd(const StreamReadFunc& reader) noexcept(false);
    };
}

#pragma once

#include "WmdFileTypes.h"

#include <functional>
#include <vector>

namespace AudioTools {
    struct WmdPatchGroupHdr;

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
        uint8_t     reverb;             // How much reverb to apply to the voice. Note: the PSX sound driver ignores this.
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

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
    };

    // Holds details for a sound sample used by a patch voice; just holds the size in bytes of the sound sample
    struct PsxPatchSample {
        uint32_t    size;

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite, const uint32_t wmdOffsetField = 0) const noexcept(false);
    };

    // Describes a patch/instrument: this is a collection of voices triggered in unison
    struct PsxPatch {
        uint16_t    firstVoiceIdx;      // Index of the first patch voice for the patch. Other patch voices follow contiguously in the voices list.
        uint16_t    numVoices;          // How many voices to use for the patch

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
    };

    // Patches, patch voices and patch samples etc.
    struct PsxPatchGroup {
        uint8_t                         hwVoiceLimit;       // How many hardware voices there are
        std::vector<PsxPatchSample>     patchSamples;       // Samples used by patch voices
        std::vector<PsxPatchVoice>      patchVoices;        // Individual voices in a patch
        std::vector<PsxPatch>           patches;            // Patches/instruments

        void readFromWmd(const StreamReadFunc& streamRead, const WmdPatchGroupHdr& hdr) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
    };

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Generic module file data, after deserializing from a binary .WMD file or json.
    // This mirrors largely what is found in the 'wessarc.h' header of the main PsyDoom project.
    //--------------------------------------------------------------------------------------------------------------------------------------

    // Represents a sequencer command in a track.
    // Depending on the command type, none or all of the arguments may be used and also the range allowed might be capped.
    struct TrackCmd {
        WmdTrackCmdType     type;           // What type of command this is?
        uint32_t            delayQnp;       // Time until the command executes in quarter note parts (QNP). The actual delay in seconds depends on quarter note parts per minute count and parts per quarter note.
        int32_t             arg1;           // Command argument 1: meaning (if any) depends on the command
        int32_t             arg2;           // Command argument 2: meaning (if any) depends on the command
        int32_t             arg3;           // Command argument 3: meaning (if any) depends on the command

        uint32_t readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        uint32_t writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
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

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
    };

    // Represents an entire sequence (music or a sound) to be played by the sequencer
    struct Sequence {
        std::vector<Track>  tracks;
        uint16_t            unknownWmdField;    // Unknown field read from the .WMD: its purpose is unknown because it is never used. Preserving for diff purposes against original .WMD files!

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);
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

        void readFromWmd(const StreamReadFunc& streamRead) noexcept(false);
        void writeToWmd(const StreamWriteFunc& streamWrite) const noexcept(false);

        // WMD file reading utilities
        static void skipReadingWmdPatchGroup(const StreamReadFunc& streamRead, const WmdPatchGroupHdr& patchGroupHdr) noexcept(false);
        static uint32_t readVarLenQuant(const StreamReadFunc& streamRead, uint32_t& valueOut) noexcept(false);
        static uint32_t writeVarLenQuant(const StreamWriteFunc& streamWrite, const uint32_t valueIn) noexcept(false);
        static uint32_t getVarLenQuantLen(const uint32_t valueIn) noexcept;
    };
}

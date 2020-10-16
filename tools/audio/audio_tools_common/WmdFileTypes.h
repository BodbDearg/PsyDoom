//------------------------------------------------------------------------------------------------------------------------------------------
// This file contains a collection of binary data structures and values found in a Williams Module File (.WMD)
//------------------------------------------------------------------------------------------------------------------------------------------
#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(AudioTools)

//------------------------------------------------------------------------------------------------------------------------------------------
// Driver agnostic binary data structures and values found in the Williams module file (.WMD file).
// These are copied more or less verbatim from the 'wessarc.h' header of the main PsyDoom project.
//------------------------------------------------------------------------------------------------------------------------------------------

// Expected 4 byte identifier for WMD (Williams Module) files: says 'SPSX'
static constexpr uint32_t WMD_MODULE_ID = 0x58535053u;

// Expected module file version for all .WMD files
static constexpr uint32_t WMD_VERSION = 1u;

// Header for the .WMD file - this is the first thing found in the file.
// Defines general/global info and settings for the module.
struct WmdModuleHdr {
    uint32_t    moduleId;               // Should be the string 'SPSX'
    uint32_t    moduleVersion;          // Should be '1'
    uint16_t    numSequences;           // How many sequences are in the module.
    uint8_t     numPatchGroups;         // How many driver specific patch groups are defined in the module
    uint8_t     maxActiveSequences;     // The number of sequence work areas to allocate, or the maximum number of active sequences
    uint8_t     maxActiveTracks;        // The number of track work areas to allocate, or the maximum number of active tracks
    uint8_t     maxGatesPerSeq;         // Maximum number of 'gates' or on/off conditional jump switches per sequence
    uint8_t     maxItersPerSeq;         // Maximum number of iteration counts (iters) per sequence; these are used to do jumps a certain number of times
    uint8_t     maxCallbacks;           // Maximum number of user sequencer callbacks that can be used

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdModuleHdr) == 16);

// Flags specifying what types of patch group data gets loaded
enum WmdPatchGroupLoadFlags : int32_t {
    LOAD_PATCHES        = 0x01,
    LOAD_PATCH_VOICES   = 0x02,
    LOAD_PATCH_SAMPLES  = 0x04,
    LOAD_DRUM_PATCHES   = 0x08,
    LOAD_EXTRA_DATA     = 0x10
};

// Sound driver ids: these are the exact same ids found in the .WMD file
enum class WmdSoundDriverId : uint8_t {
    NoSound     = 0,    // No sound driver
    PSX         = 1,    // Sony PlayStation sound driver
    GENERIC     = 50    // Generic hardware agnostic sound driver
};

// Classes of sounds/voices: these are the exact same ids found in the .WMD file
enum class WmdSoundClass : uint8_t {
    SNDFX       = 0,    // Used for most game sounds
    MUSIC       = 1,    // Used for music sounds
    DRUMS       = 2,    // Similar to music sounds, but with more variety in which samples are used
    SFXDRUMS    = 3     // I guess similar to sfx but setup to use lots of different samples (like drums)
};

// Header for a patch group.
// A patch group is a group of patches/instruments for a specific sound driver, like the PSX sound driver.
struct WmdPatchGroupHdr {
    WmdPatchGroupLoadFlags  loadFlags;              // What things this patch group contains: should be consulted when loading the patch group data
    WmdSoundDriverId        driverId;               // Driver id
    uint8_t                 hwVoiceLimit;           // How many voices does this driver support
    uint16_t                _pad;                   // Unused
    uint16_t                numPatches;             // How many patches for this sound driver?
    uint16_t                patchSize;              // Size of the patch data structure for this sound driver
    uint16_t                numPatchVoices;         // How many patch voices for this sound driver?
    uint16_t                patchVoiceSize;         // Size of the patch voice data structure for this sound driver
    uint16_t                numPatchSamples;        // How many patch samples for this sound driver?
    uint16_t                patchSampleSize;        // Size of the patch sample data structure for this sound driver
    uint16_t                numDrumPatches;         // How many drum patches for this sound driver?
    uint16_t                drumPatchSize;          // Size of the drum patch data structure for this sound driver
    uint32_t                extraDataSize;          // Size of any additional driver specific data

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdPatchGroupHdr) == 28);

// Header for a sequence
struct WmdSequenceHdr {
    uint16_t    numTracks;      // How many tracks are in the sequence
    uint16_t    unknownField;   // Field is unused in PSX Doom - it's purpose cannot be determined because of that

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdSequenceHdr) == 4);

// Header for an individual track in a sequence
struct WmdTrackHdr {
    WmdSoundDriverId    driverId;               // What sound driver the track should play with
    uint8_t             maxVoices;              // Maximum number of voices that the track claims to use
    uint8_t             priority;               // Used for prioritizing voices when we are out of hardware voices
    uint8_t             lockChannel;            // Seems to be unused in this version of the WESS library: appears to be for assigning certain tracks to certain sound channels
    WmdSoundClass       soundClass;             // What rough class of sounds the track contains
    uint8_t             initReverb;             // The reverb level to initialize the track with
    uint16_t            initPatchIdx;           // Which patch index to initially use for the track unless manually overridden
    int16_t             initPitchCntrl;         // What pitch shift value to initially use for the track unless manually overridden
    uint8_t             initVolumeCntrl;        // What volume to initially use for the track unless manually overridden
    uint8_t             initPanCntrl;           // What pan setting to initially use for the track unless manually overridden
    uint8_t             locStackSize;           // The maximum number of track data locations that can be remembered in the track's location stack (for save + return control flow)
    uint8_t             initMutegroupsMask;     // A bit mask defining what 'mute groups' the track is a part of: used for bulk muting of tracks by group
    uint16_t            initPpq;                // Parts per quarter note: how many parts to subdivide each quarter note into for timing purposes. Affects timing precision.
    uint16_t            initQpm;                // The quarter notes per minute (beats per minute) value to initialize the track status with, unless manually overridden.
    uint16_t            numLabels;              // How many jump/position labels the track defines
    uint32_t            cmdStreamSize;          // The size of the stream containing sequencer commands and command timings for the track

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdTrackHdr) == 24);

// Track command types: some of these are handled by the sequencer generically, others are handled by the sound driver
enum class WmdTrackCmdType : uint8_t {
    // Manually called commands - should never be in a track's command set!
    DriverInit      = 0,
    DriverExit      = 1,
    DriverEntry1    = 2,
    DriverEntry2    = 3,
    DriverEntry3    = 4,
    TrkOff          = 5,
    TrkMute         = 6,

    // Driver commands
    PatchChg        = 7,
    PatchMod        = 8,        // Unused by PSX Doom
    PitchMod        = 9,
    ZeroMod         = 10,       // Unused by PSX Doom
    ModuMod         = 11,       // Used by PSX Doom but driver doesn't implement
    VolumeMod       = 12,
    PanMod          = 13,
    PedalMod        = 14,       // Used by PSX Doom but driver doesn't implement
    ReverbMod       = 15,       // Unused by PSX Doom
    ChorusMod       = 16,       // Unused by PSX Doom
    NoteOn          = 17,
    NoteOff         = 18,

    // Sequencer commands
    StatusMark      = 19,       // Unused by PSX Doom
    GateJump        = 20,       // Unused by PSX Doom
    IterJump        = 21,       // Unused by PSX Doom
    ResetGates      = 22,       // Unused by PSX Doom
    ResetIters      = 23,       // Unused by PSX Doom
    WriteIterBox    = 24,       // Unused by PSX Doom
    SeqTempo        = 25,       // Unused by PSX Doom
    SeqGosub        = 26,       // Unused by PSX Doom
    SeqJump         = 27,       // Unused by PSX Doom
    SeqRet          = 28,       // Unused by PSX Doom
    SeqEnd          = 29,       // Unused by PSX Doom
    TrkTempo        = 30,       // Unused by PSX Doom
    TrkGosub        = 31,       // Unused by PSX Doom
    TrkJump         = 32,
    TrkRet          = 33,       // Unused by PSX Doom
    TrkEnd          = 34,
    NullEvent       = 35
};

//--------------------------------------------------------------------------------------------------------------------------------------
// PlayStation sound driver specific binary data structures found in the Williams module file (.WMD file).
// These are copied more or less verbatim from the 'wessarc.h' header of the main PsyDoom project.
//--------------------------------------------------------------------------------------------------------------------------------------

// Describes a patch/instrument: this is a collection of voices triggered in unison
struct WmdPsxPatch {
    uint16_t    numVoices;          // How many voices to use for the patch
    uint16_t    firstVoiceIdx;      // Index of the first patch voice for the patch. Other patch voices follow contiguously in the voices list.

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdPsxPatch) == 4);

// Settings for one individual voice/sound in a patch/instrument
struct WmdPsxPatchVoice {
    uint8_t     priority;           // Voice priority: used to determine what to kill when we're out of voices
    uint8_t     reverb;             // Appears to be unused: reverb for the voice
    uint8_t     volume;             // Volume to play the voice at
    uint8_t     pan;                // Voice pan 64 to 191 - 128 is the voice center. Values outside this range are clamped by the PSX driver.
    uint8_t     baseNote;           // The 'base' note/semitone at which the sound sample is regarded to play back at 44,100 Hz (used to compute sample frequency for triggered notes)
    uint8_t     baseNoteFrac;       // The .8 fractional part of the base note
    uint8_t     noteMin;            // Minimum semitone at which the voice can be played - does not sound below this
    uint8_t     noteMax;            // Maximum semitone at which the voice can be played - does not sound above this
    uint8_t     pitchstepDown;      // How many semitones of range the pitch bend wheel has when bending pitch down
    uint8_t     pitchstepUp;        // How many semitones of range the pitch bend wheel has when bending pitch up
    uint16_t    sampleIdx;          // The index of the patch sample to use for this voice
    uint16_t    adsr1;              // The first (low) 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)
    uint16_t    adsr2;              // The second (high) 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdPsxPatchVoice) == 16);

// Holds details for a sound sample used by a patch voice which can be loaded or is already loaded
struct WmdPsxPatchSample {
    uint32_t    offset;         // Unused & unclear what the purpose of this is for. Increases with each patch sample in the module.
    uint32_t    size;           // The size in bytes of the sound data for this sample
    uint32_t    spuAddr;        // Where in SPU RAM the patch is currently uploaded to. Set to '0' if not uploaded to the SPU.

    void endianCorrect() noexcept;
};

static_assert(sizeof(WmdPsxPatchSample) == 12);

// Utility functions
uint32_t getWmdTrackCmdSize(const WmdTrackCmdType type) noexcept;
uint32_t getNumWmdTrackCmdArgs(const WmdTrackCmdType type) noexcept;
const char* toString(const WmdSoundDriverId value) noexcept;
const char* toString(const WmdSoundClass value) noexcept;
const char* toString(const WmdTrackCmdType value) noexcept;
WmdSoundDriverId stringToSoundDriverId(const char* const value) noexcept;
WmdSoundClass stringToSoundClass(const char* const value) noexcept;
WmdTrackCmdType stringToTrackCmdType(const char* const value) noexcept;

END_NAMESPACE(AudioTools)

#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(VagUtils)

static constexpr uint32_t VAG_FILE_ID       = 0x70474156;
static constexpr uint32_t VAG_FILE_VERSION  = 0x03;

// Header format for a PS1 .VAG.
// Note that this header is stored in BIG ENDIAN format in the file!
struct VagFileHdr {
    uint32_t    fileId;             // Should say 'VAGp'
    uint32_t    version;            // Should be '0x03'
    uint32_t    _reserved1;         // Unused...
    uint32_t    size;               // Size of the header + sound data
    uint32_t    sampleRate;         // Sound data sample rate
    uint32_t    _reserved2[3];      // Unused...
    char        name[16];           // Name for the VAG
    uint32_t    _unknown[4];        // Purpose unknown

    void endianCorrect() noexcept;
    bool validate() noexcept;
};

static_assert(sizeof(VagFileHdr) == 64);

END_NAMESPACE(VagUtils)
END_NAMESPACE(AudioTools)

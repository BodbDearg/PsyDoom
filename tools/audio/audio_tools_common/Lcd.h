#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class InputStream;
class OutputStream;

BEGIN_NAMESPACE(AudioTools)

struct PsxPatchGroup;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a single sound in an LCD file
//------------------------------------------------------------------------------------------------------------------------------------------
struct LcdSample {
    uint16_t                    patchSampleIdx;     // Which PlayStation driver patch sample this sound data is for
    std::vector<std::byte>      adpcmData;          // The ADPCM sound data for the sound
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Container representing the contents of an LCD samples file in PSX Doom (.LCD file)
//------------------------------------------------------------------------------------------------------------------------------------------
struct Lcd {
    static constexpr uint32_t MAX_LCD_FILE_SAMPLES      = 1023;     // This is the maximum number of sounds that can be written to an LCD file
    static constexpr uint32_t DOOM_MAX_LCD_FILE_SAMPLES = 100;      // This is how many sounds per LCD file that PSX Doom can handle

    // All of the samples contained in the LCD file
    std::vector<LcdSample> samples;

    bool readFromLcdFile(InputStream& in, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) noexcept;
    bool readFromLcdFile(const char* const filePath, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) noexcept;
    bool writeToLcdFile(OutputStream& out, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) const noexcept;
    bool writeToLcdFile(const char* const filePath, const PsxPatchGroup& patchGroup, std::string& errorMsgOut) const noexcept;

    const LcdSample* findPatchSample(const uint16_t patchSampleIdx) const noexcept;
    LcdSample* findPatchSample(const uint16_t patchSampleIdx) noexcept;
};

END_NAMESPACE(AudioTools)

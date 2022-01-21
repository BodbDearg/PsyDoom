#include "FatalErrors.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>

//------------------------------------------------------------------------------------------------------------------------------------------
// Entry point for 'VRAMDumpGetRect'
//------------------------------------------------------------------------------------------------------------------------------------------
// Program purpose:
//
//  Given a 1 MiB dump of PlayStation VRAM (1024x512 16-bit pixels) allows a rectangle within that dump to be extracted to a separate file.
//  Useful for extracting a portion of VRAM for the purposes of pattern matching/search in other binary data.
//  The VRAM dump can be obtained from a PlayStation emulator like 'DuckStation' that has this capability.
//------------------------------------------------------------------------------------------------------------------------------------------

// The size of vram in terms of 16-bit pixels
static constexpr uint32_t VRAM_W = 1024;
static constexpr uint32_t VRAM_H = 512;

int main(int argc, char* argv[]) noexcept {
    // Make sure all arguments are provided and retrieve them
    if (argc != 7) {
        std::printf("Usage: VRAMDumpGetRect <VRAM_DUMP_FILE> <RECT_X> <RECT_Y> <RECT_W> <RECT_H> <OUTPUT_FILE>\n");
        return 1;
    }

    const char* const vramDumpPath = argv[1];
    const int32_t rectX = std::atoi(argv[2]);
    const int32_t rectY = std::atoi(argv[3]);
    const int32_t rectW = std::atoi(argv[4]);
    const int32_t rectH = std::atoi(argv[5]);
    const char* const outputPath = argv[6];

    // Validate the rectangle to retrieve from the dump
    const bool bInvalidRect = (
        (rectX < 0) || (rectX >= VRAM_W) ||
        (rectY < 0) || (rectY >= VRAM_H) ||
        (rectW <= 0) || (rectW > VRAM_W) ||
        (rectH <= 0) || (rectH > VRAM_H) ||
        (rectX + rectW > VRAM_W) ||
        (rectY + rectH > VRAM_H)
    );

    if (bInvalidRect)
        FATAL_ERROR("Invalid VRAM rect to extract! Must be within the VRAM area of 1024x512, and must not be zero sized!");

    // Read the entire VRAM dump
    std::unique_ptr<uint16_t[]> vramDump = std::make_unique<uint16_t[]>(VRAM_W * VRAM_H);

    {
        FILE* const pVramDumpFile = std::fopen(vramDumpPath, "rb");

        if (!pVramDumpFile)
            FATAL_ERROR_F("Failed to open the VRAM dump file '%s'!", vramDumpPath);

        const bool bReadSuccess = (std::fread(vramDump.get(), VRAM_W * VRAM_H * sizeof(uint16_t), 1, pVramDumpFile) == 1);
        std::fclose(pVramDumpFile);

        if (!bReadSuccess)
            FATAL_ERROR_F("Failed to read from the VRAM dump file '%s'!", vramDumpPath);
    }

    // Makeup the data to be written to the output file
    std::unique_ptr<uint16_t[]> outputData = std::make_unique<uint16_t[]>(rectW * rectH);

    {
        const uint16_t* pRowIn = vramDump.get() + rectY * VRAM_W + rectX;
        uint16_t* pRowOut = outputData.get();

        for (int32_t y = 0; y < rectH; ++y) {
            std::memcpy(pRowOut, pRowIn, rectW * sizeof(uint16_t));
            pRowIn += VRAM_W;
            pRowOut += rectW;
        }
    }

    // Write the data to the output file and close it up
    {
        FILE* const pOutputFile = std::fopen(outputPath, "wb");

        if (!pOutputFile)
            FATAL_ERROR_F("Failed to open the output file '%s'!", outputPath);

        const bool bWriteSuccess = (std::fwrite(outputData.get(), rectW * rectH * sizeof(uint16_t), 1, pOutputFile) == 1);
        const bool bCloseSuccess = (std::fclose(pOutputFile) == 0);
        const bool bAllSuccess = (bWriteSuccess && bCloseSuccess);

        if (!bAllSuccess)
            FATAL_ERROR_F("Failed to write to the output file '%s'!", outputPath);
    }

    return 0;
}

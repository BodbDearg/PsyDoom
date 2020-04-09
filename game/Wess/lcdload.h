#pragma once

#include <cstdint>

enum class CdMapTbl_File : uint32_t;
struct master_status_structure;
struct SampleBlock;

// How many entries there are in a sample block
static constexpr uint32_t SAMPLE_BLOCK_SIZE = 100;

// Holds information for a number of loaded sound samples or patches.
// Holds the patch number for each sound, and where it is located in SPU RAM.
struct SampleBlock {
    uint16_t numsamps;                          // How many samples are defined for this block
    uint16_t sampindx[SAMPLE_BLOCK_SIZE];       // The patch number for each defined sample
    uint16_t samppos[SAMPLE_BLOCK_SIZE];        // The SPU RAM address for each defined sample
};

bool wess_dig_lcd_loader_init(master_status_structure* const pMStat) noexcept;
void wess_dig_set_sample_position(const int32_t patchIdx, const uint32_t spuAddr) noexcept;

int32_t wess_dig_lcd_data_read(
    uint8_t* const pSectorData,
    const uint32_t destSpuAddr,
    SampleBlock* const pSamples,
    const bool bOverride
) noexcept;

int32_t wess_dig_lcd_psxcd_sync() noexcept;

int32_t wess_dig_lcd_load(
    const CdMapTbl_File lcdFileToLoad,
    const uint32_t destSpuAddr,
    SampleBlock* const pSampleBlock,
    const bool bOverride
) noexcept;

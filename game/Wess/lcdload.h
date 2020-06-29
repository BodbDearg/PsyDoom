#pragma once

#include <cstdint>

enum class CdMapTbl_File : int32_t;
struct master_status_structure;
struct SampleBlock;

// How many entries there are in a sample block
static constexpr uint32_t SAMPLE_BLOCK_SIZE = 100;

// Holds information for a number of loaded sound samples.
// Holds the index of the 'patch_sample' for each sound, and where it is currently loaded to in SPU RAM.
struct SampleBlock {
    uint16_t num_samples;                               // How many samples are defined for this block
    uint16_t patch_sample_idx[SAMPLE_BLOCK_SIZE];       // The 'patch_sample' index for each defined sample
    uint16_t sample_spu_addr_8[SAMPLE_BLOCK_SIZE];      // The SPU RAM address (divided by 8) for each defined sample
};

bool wess_dig_lcd_loader_init(master_status_structure* const pMStat) noexcept;
void wess_dig_set_sample_position(const int32_t patchSampleIdx, const uint32_t spuAddr) noexcept;

int32_t wess_dig_lcd_data_read(
    uint8_t* const pSectorData,
    const uint32_t destSpuAddr,
    SampleBlock* const pSamples,
    const bool bOverride
) noexcept;

int32_t wess_dig_lcd_load(
    const CdMapTbl_File lcdFileToLoad,
    const uint32_t destSpuAddr,
    SampleBlock* const pSampleBlock,
    const bool bOverride
) noexcept;

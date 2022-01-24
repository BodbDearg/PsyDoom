#include "IntroLogos.h"

#include "DiscReader.h"
#include "Game.h"
#include "IsoFileSys.h"
#include "LogoPlayer.h"
#include "PsxVm.h"
#include "Wess/psxcd.h"

#include <md5.h>
#include <memory>

BEGIN_NAMESPACE(IntroLogos)

//------------------------------------------------------------------------------------------------------------------------------------------
// The dimensions of a PSX Doom intro logo which is embedded in the boot executable
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t PSX_DOOM_BOOT_LOGO_W = 256;
static constexpr uint32_t PSX_DOOM_BOOT_LOGO_H = 240;

//------------------------------------------------------------------------------------------------------------------------------------------
// Defines a set of embedded intro logos for a specific variant of the PlayStation Doom boot/startup executable.
// Each logo begins with 256 RGB888 palette entries (768 bytes) followed by an 8-bit color indexed image of 256x240 pixels (61,440 bytes).
// 
// Note: surprisingly all versions of the boot executable contain logos for other regions, even if they are not used.
// For example the Europe and Japan executables contain the US SCEA intro logo.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PsxDoomBootExeLogos {
    uint64_t    exeHashWord1;       // MD5 Hash of the PSX Doom boot executable containing the logo (word 1)
    uint64_t    exeHashWord2;       // MD5 Hash of the PSX Doom boot executable containing the logo (word 2)
    uint32_t    sceaLogoOffset;     // Offset of the SCEA (Sony America) logo within the executable
    uint32_t    sceeLogoOffset;     // Offset of the SCEE (Sony Europe) logo within the executable
    uint32_t    legalsLogoOffset;   // Offset of the logo containing PSX Doom legal/copyright info within the executable
};

// Embedded boot executable logos for all variants of PlayStation Doom
static constexpr PsxDoomBootExeLogos PSX_DOOM_BOOT_EXE_LOGOS[] = {
    { 0xB1B1457E43C6948E, 0xD4EC1D3C10F0358F, 0x1970C, 0x29A10, 0x39D14 },      // Doom 1.0: US (SLUS_000.77) (Original US PSX Doom release)
    { 0x5EC83BB625405725, 0xCA3067301CF27FEE, 0x1970C, 0x29A10, 0x39D14 },      // Doom 1.1: US (SLUS_000.77) (US 'Greatest Hits' re-release)
    { 0xFF0E934DE5BFA36E, 0x40841F9052CE40A3, 0x19758, 0x29A5C, 0x39D60 },      // Doom 1.1: Europe (SLES_001.32) (Original Europe PSX Doom release + 'Platinum' re-release)
    { 0x1641C74D99D15272, 0x705EDAEAB28BFF2A, 0x19704, 0x2980C, 0x39D0C },      // Doom 1.1: Japan (SLPS_003.08) (PSX Doom Japan release)
    { 0x0668FF031942802C, 0xC6384BA037DA257D, 0x1B008, 0x2B30C, 0x3B610 },      // Final Doom: US (SLUS_003.31)
    { 0x28EF0816BB969BC4, 0x87302D5B286BCEC2, 0x1B054, 0x2B358, 0x3B65C },      // Final Doom: Europe (SLES_004.87)
    { 0xE42785D83C5778AD, 0x0B0F01693A83C71C, 0x1B000, 0x2B304, 0x3B608 },      // Final Doom: Japan (SLPS_007.27)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// The MD5 hash of the PSX Doom boot executable (2 64-bit words)
//------------------------------------------------------------------------------------------------------------------------------------------
static uint64_t gPsxDoomBootExeHashWord1;
static uint64_t gPsxDoomBootExeHashWord2;

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the path (on the game disc) to the executable which is used to boot PSX Doom.
// This executable contains the embedded intro logos and was responsible for playing the intro movie.
// Returns 'nullptr' if this call is not applicable for the current game disc.
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* getPsxDoomBootExePath() noexcept {
    if (Game::gGameType == GameType::Doom) {
        switch (Game::gGameVariant) {
            case GameVariant::NTSC_U:   return "SLUS_000.77";
            case GameVariant::NTSC_J:   return "SLPS_003.08";
            case GameVariant::PAL:      return "SLES_001.32";
        }
    } else if (Game::gGameType == GameType::FinalDoom) {
        switch (Game::gGameVariant) {
            case GameVariant::NTSC_U:   return "SLUS_003.31";
            case GameVariant::NTSC_J:   return "SLPS_007.27";
            case GameVariant::PAL:      return "SLES_004.87";
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines the MD5 hash of the original PSX Doom boot executable.
// If no such executable exists on the disc then the hash is set to '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
static void determinePsxDoomBootExeHash() noexcept {
    // Initialize the hash at first to just '0'
    gPsxDoomBootExeHashWord1 = 0;
    gPsxDoomBootExeHashWord2 = 0;

    // Read the PlayStation Doom boot executable into memory
    const char* const exePath = getPsxDoomBootExePath();
    const IsoFileSysEntry* const pFsEntry = (exePath) ? PsxVm::gIsoFileSys.getEntry(exePath) : nullptr;
    const bool bValidFsEntry = (pFsEntry && (pFsEntry->size > 0));

    if (!bValidFsEntry)
        return;

    std::unique_ptr<std::byte[]> bootExeBytes = std::make_unique<std::byte[]>(pFsEntry->size);

    {
        DiscReader discReader(PsxVm::gDiscInfo);

        const bool bExeReadSuccess = (
            discReader.setTrackNum(1) &&
            discReader.trackSeekAbs((int32_t) pFsEntry->startLba * CDROM_SECTOR_SIZE) &&
            discReader.read(bootExeBytes.get(), pFsEntry->size)
        );

        if (!bExeReadSuccess)
            return;
    }

    // Hash it, and turn it into 2 64-bit words
    MD5 md5Hasher;
    md5Hasher.reset();
    md5Hasher.add(bootExeBytes.get(), pFsEntry->size);

    uint8_t md5[16] = {};
    md5Hasher.getHash(md5);

    gPsxDoomBootExeHashWord1 = (
        ((uint64_t) md5[0 ] << 56) | ((uint64_t) md5[1 ] << 48) | ((uint64_t) md5[2 ] << 40) | ((uint64_t) md5[3 ] << 32) |
        ((uint64_t) md5[4 ] << 24) | ((uint64_t) md5[5 ] << 16) | ((uint64_t) md5[6 ] <<  8) | ((uint64_t) md5[7 ] <<  0)
    );

    gPsxDoomBootExeHashWord2 = (
        ((uint64_t) md5[8 ] << 56) | ((uint64_t) md5[9 ] << 48) | ((uint64_t) md5[10] << 40) | ((uint64_t) md5[11] << 32) |
        ((uint64_t) md5[12] << 24) | ((uint64_t) md5[13] << 16) | ((uint64_t) md5[14] <<  8) | ((uint64_t) md5[15] <<  0)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: reads from the specified portion of a file in the CD-ROM image and returns the bytes if suceeded
//------------------------------------------------------------------------------------------------------------------------------------------
static std::unique_ptr<std::byte[]> readFromDiscFile(const char* const path, uint32_t offset, uint32_t size) noexcept {
    // If no path is specified or bytes requested then the result is no bytes
    if ((!path) || (size <= 0))
        return {};

    // Get the file system entry for the requested file and ensure we can do the read
    const IsoFileSysEntry* const pFsEntry = PsxVm::gIsoFileSys.getEntry(path);

    const bool bCanDoRead = (
        pFsEntry &&
        (size <= pFsEntry->size) &&
        (offset < pFsEntry->size) &&
        (offset + size <= pFsEntry->size)
    );

    if (!bCanDoRead)
        return {};

    // Try to read the bytes
    std::unique_ptr<std::byte[]> bytes = std::make_unique<std::byte[]>(size);
    DiscReader discReader(PsxVm::gDiscInfo);

    const bool bReadSuccess = (
        discReader.setTrackNum(1) &&
        discReader.trackSeekAbs((int32_t) pFsEntry->startLba * CDROM_SECTOR_SIZE + (int32_t) offset) &&
        discReader.read(bytes.get(), size)
    );

    if (!bReadSuccess) {
        bytes.reset();
    }

    return bytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: decodes a PSX Doom boot executable embedded logo into the specified LogoPlayer::Logo struct.
// Returns 'false' if the operation fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool decodePsxDoomBootExeLogo(const uint32_t logoExeOffset, LogoPlayer::Logo& output) noexcept {
    // Read the raw bytes for the logo.
    // There is a 256 entry RGB888 palette followed by an 8-bit image of 256x240 pixels.
    constexpr uint32_t NUM_LOGO_PIXELS = PSX_DOOM_BOOT_LOGO_W * PSX_DOOM_BOOT_LOGO_H;
    constexpr uint32_t LOGO_PALETTE_SIZE = 256 * 3;
    constexpr uint32_t LOGO_SIZE_IN_BYTES = LOGO_PALETTE_SIZE + NUM_LOGO_PIXELS;

    std::unique_ptr<std::byte[]> logoBytes = readFromDiscFile(getPsxDoomBootExePath(), logoExeOffset, LOGO_SIZE_IN_BYTES);

    // If that fails then ensure the output is cleared
    if (!logoBytes) {
        output.pPixels.reset();
        output.width = 0;
        output.height = 0;
        return false;
    }

    // Otherwise allocate the output pixel array and decode the image
    output.pPixels = std::make_unique<uint32_t[]>(PSX_DOOM_BOOT_LOGO_W * PSX_DOOM_BOOT_LOGO_H);

    const uint8_t* const pPalette = reinterpret_cast<const uint8_t*>(logoBytes.get());
    const uint8_t* pPixelIn = reinterpret_cast<const uint8_t*>(logoBytes.get() + LOGO_PALETTE_SIZE);
    uint32_t* pPixelOut = output.pPixels.get();

    for (uint32_t i = 0; i < NUM_LOGO_PIXELS; ++i, ++pPixelIn, ++pPixelOut) {
        // Get the color components of the pixel.
        // Note: if the index is '0' then that is the transparent color, make it black:
        const uint8_t colorIdx = *pPixelIn;

        if (colorIdx == 0) {
            *pPixelOut = 0xFF000000u;   // Special case, transparent pixel but make it black to match the screen clear color...
            continue;
        }

        const uint8_t* const pPaletteEntryRgb = pPalette + colorIdx * 3;
        const uint32_t r = pPaletteEntryRgb[0];
        const uint32_t g = pPaletteEntryRgb[1];
        const uint32_t b = pPaletteEntryRgb[2];

        // Convert to XBGR8888 and save
        *pPixelOut = 0xFF000000u | (b << 16) | (g << 8) | (r);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: returns the set of PSX Doom boot executable logos defined for this variant of the game (if any)
//------------------------------------------------------------------------------------------------------------------------------------------
static const PsxDoomBootExeLogos* getPsxDoomBootExeLogos() noexcept {
    for (const PsxDoomBootExeLogos& logos : PSX_DOOM_BOOT_EXE_LOGOS) {
        const bool bBootExeHashMatch = (
            (logos.exeHashWord1 == gPsxDoomBootExeHashWord1) &&
            (logos.exeHashWord2 == gPsxDoomBootExeHashWord2)
        );

        if (bBootExeHashMatch)
            return &logos;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the intro logos module
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    determinePsxDoomBootExeHash();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does cleanup/teardown for the intro logos module
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gPsxDoomBootExeHashWord1 = {};
    gPsxDoomBootExeHashWord2 = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: gets the Sony intro logo for the current variant of the game
//------------------------------------------------------------------------------------------------------------------------------------------
LogoPlayer::Logo getSonyIntroLogo() noexcept {
    // Only if there are embedded logos defined for this version of the game
    const PsxDoomBootExeLogos* const pLogos = getPsxDoomBootExeLogos();

    if (!pLogos)
        return {};

    // This logo is only used for the US and Europe editions of PSX Doom.
    // The Japanese version of the game doesn't show any Sony specific logo:
    const bool bGameVariantHasSonyLogo = (
        (Game::gGameVariant == GameVariant::NTSC_U) ||
        (Game::gGameVariant == GameVariant::PAL)
    );

    if (!bGameVariantHasSonyLogo)
        return {};

    // Read the logo and define it's parameters
    LogoPlayer::Logo logo = {};
    const uint32_t logoExeOffset = (Game::gGameVariant == GameVariant::NTSC_U) ? pLogos->sceaLogoOffset : pLogos->sceeLogoOffset;

    if (!decodePsxDoomBootExeLogo(logoExeOffset, logo))
        return {};

    logo.width = 256;
    logo.height = 240;
    logo.holdTime = 3.0f;
    logo.fadeOutTime = 0.5f;
    return logo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: gets the PSX Doom legals/copyright intro logo for the current variant of the game
//------------------------------------------------------------------------------------------------------------------------------------------
LogoPlayer::Logo getLegalsIntroLogo() noexcept {
    // Only if there are embedded logos defined for this version of the game
    const PsxDoomBootExeLogos* const pLogos = getPsxDoomBootExeLogos();

    if (!pLogos)
        return {};

    // Read the logo and define it's parameters
    LogoPlayer::Logo logo = {};

    if (!decodePsxDoomBootExeLogo(pLogos->legalsLogoOffset, logo))
        return {};

    logo.width = 256;
    logo.height = 240;
    logo.holdTime = 3.5f;
    return logo;
}

END_NAMESPACE(IntroLogos)

//------------------------------------------------------------------------------------------------------------------------------------------
// PalTool:
//      Converts from a PlayStation Doom format palette lump (PLAYPAL) to RAW R8G8B8 and visa versa.
//      The PlayStation palette is in T1B5G5R5 (16-bit) format.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Endian.h"
#include "FileUtils.h"

#include <cstdint>
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// The pixel data in RGB and PSX formats
//------------------------------------------------------------------------------------------------------------------------------------------
struct PixelRGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    void endianCorrect() noexcept {}
};

struct PixelPSX {
    uint16_t bits;

    void endianCorrect() noexcept {
        if constexpr (Endian::isBig()) {
            bits = Endian::byteSwap(bits);
        }
    }
};

static std::vector<PixelRGB> gPixelsRgb;
static std::vector<PixelPSX> gPixelsPsx;

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts from the PSX color format to RGB888
//------------------------------------------------------------------------------------------------------------------------------------------
static PixelRGB psxPixelToRgb(const PixelPSX& pixelIn) noexcept {
    return {
        (uint8_t) (((pixelIn.bits >> 0 ) & 0x1Fu) << 3),
        (uint8_t) (((pixelIn.bits >> 5 ) & 0x1Fu) << 3),
        (uint8_t) (((pixelIn.bits >> 10) & 0x1Fu) << 3)
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts from RGB888 to the PSX color format
//------------------------------------------------------------------------------------------------------------------------------------------
static PixelPSX rgbPixelToPsx(const PixelRGB& pixelIn) noexcept {
    // Note: +4 does rounding here
    const uint16_t r = (uint16_t) std::min((pixelIn.r + 4u) >> 3, 31u);
    const uint16_t g = (uint16_t) std::min((pixelIn.g + 4u) >> 3, 31u);
    const uint16_t b = (uint16_t) std::min((pixelIn.b + 4u) >> 3, 31u);

    return { (uint16_t)((r << 0) | (g << 5) | (b << 10)) };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Help/usage printing
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* const HELP_STR =
R"(Usage: PalTool <MODE> <INPUT FILE PATH> <OUTPUT FILE PATH>

Modes:
    -psx-to-rgb
        Convert a PlayStation Doom format palette lump (PLAYPAL) in T1B5G5R5 (16-bit) format to RAW R8G8B8.
        Example:
            PalTool -psx-to-rgb PLAYPAL.dat PLAYPAL_RGB.raw

    -rgb-to-psx
        Convert a palette in RAW R8G8B8 format to the PlayStation Doom T1B5G5R5 format.
        Example:
            PalTool -rgb-to-psx PLAYPAL_RGB.raw PLAYPAL.dat
)";

static void printHelp() noexcept {
    std::printf("%s\n", HELP_STR);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads pixels from the specified file and returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PixelT>
static bool readPixels(const char* const filePath, std::vector<PixelT>& pixels) noexcept {
    // Get the raw file data
    FileData data = FileUtils::getContentsOfFile(filePath);

    if (!data.bytes) {
        std::printf("Failed to read input file '%s'! Is the path correct?\n", filePath);
        return false;
    }

    // Read the pixels and endian correct
    const size_t numPixels = data.size / sizeof(PixelT);
    pixels.reserve(pixels.size() + numPixels);
    const PixelT* const pFilePixels = reinterpret_cast<const PixelT*>(data.bytes.get());

    for (size_t i = 0; i < numPixels; ++i) {
        PixelT pixel = pFilePixels[i];
        pixel.endianCorrect();
        pixels.push_back(pixel);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Writes pixels to the specified file and returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PixelT>
static bool writePixels(const char* const filePath, const std::vector<PixelT>& pixels) noexcept {
    // Makeup the output pixel array and endian correct it all
    std::vector<PixelT> outPixels = pixels;

    for (PixelT& pixel : outPixels) {
        pixel.endianCorrect();
    }

    // Write to the output file
    if (!FileUtils::writeDataToFile(filePath, outPixels.data(), sizeof(PixelT) * outPixels.size())) {
        std::printf("Failed to write to the output file '%s'! Is the path writeable?\n", filePath);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert pixels between formats
//------------------------------------------------------------------------------------------------------------------------------------------
template <class TPixIn, class TPixOut, class TConvertFn>
static void convertPixels(const std::vector<TPixIn>& inPixels, std::vector<TPixOut>& outPixels, const TConvertFn convertFn) noexcept {
    outPixels.clear();
    outPixels.reserve(inPixels.size());

    for (const TPixIn& inPixel : inPixels) {
        outPixels.push_back(convertFn(inPixel));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Program entrypoint
//------------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* const argv[]) noexcept {
    // Not enough arguments?
    if (argc != 4) {
        printHelp();
        return 1;
    }

    // See what command is being executed
    const char* const cmdSwitch = argv[1];
    const char* const inFilePath = argv[2];
    const char* const outFilePath = argv[3];

    if (std::strcmp(cmdSwitch, "-psx-to-rgb") == 0) {
        if (readPixels(inFilePath, gPixelsPsx)) {
            convertPixels(gPixelsPsx, gPixelsRgb, psxPixelToRgb);
            return (writePixels(outFilePath, gPixelsRgb)) ? 0 : 1;
        }
    }
    else if (std::strcmp(cmdSwitch, "-rgb-to-psx") == 0) {
        if (readPixels(inFilePath, gPixelsRgb)) {
            convertPixels(gPixelsRgb, gPixelsPsx, rgbPixelToPsx);

            // Every 256th color (palette index 0) should have the semi transparency bit cleared, all other colors should have it set
            for (size_t i = 0; i < gPixelsPsx.size(); ++i) {
                if (i % 256 != 0) {
                    gPixelsPsx[i].bits |= 0x8000u;
                }
            }

            return (writePixels(outFilePath, gPixelsPsx)) ? 0 : 1;
        }
    }
    else {
        printHelp();
    }

    return 1;
}

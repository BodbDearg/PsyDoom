#include "FileUtils.h"

bool FileUtils::readFileAsString(const char* const filePath, std::string& out) noexcept {
    std::FILE* const pFile = std::fopen(filePath, "rb");

    if (!pFile)
        return false;
    
    bool success = false;

    do {
        if (std::fseek(pFile, 0, SEEK_END) != 0)
            break;

        long size = std::ftell(pFile);

        if (size < 0 || size > INT32_MAX)
            break;

        if (std::fseek(pFile, 0, SEEK_SET) != 0)
            break;

        if (size > 0) {
            out.resize((uint32_t) size);

            if (std::fread(out.data(), (uint32_t) size, 1, pFile) != 1) {
                out.clear();
                break;
            }
        }

        success = true;
    } while (0);

    std::fclose(pFile);
    return success;
}

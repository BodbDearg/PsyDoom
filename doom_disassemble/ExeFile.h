#include "ExeWord.h"
#include <memory>

//----------------------------------------------------------------------------------------------------------------------
// Holds the contents of a playstation EXE
//----------------------------------------------------------------------------------------------------------------------
struct ExeFile {
    // The memory location the EXE is loaded into in the Playstation's memory.
    // This should normally be '0x80010000', with region 0x80000000 to 0x8000FFFF (the first 64K) being reserved for the BIOS.
    uint32_t baseAddress;

    // How many 32-bit words of code and data are in the exe
    uint32_t sizeInWords;

    // Which word index represents the entrypoint for the program
    uint32_t entryPointWordIdx;

    // The actual words of the EXE (code and data)
    std::unique_ptr<ExeWord[]> words;

    ExeFile() noexcept;

    // Load the playstation EXE from the given path.
    // Aborts the program if there is an error.
    void loadFromFile(const char* const path) noexcept;
};

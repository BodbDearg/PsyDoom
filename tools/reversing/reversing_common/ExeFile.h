#pragma once

#include "ExeWord.h"
#include "ProgElem.h"
#include "JRInstHandler.h"
#include <memory>
#include <ostream>
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds the contents of a playstation EXE
//------------------------------------------------------------------------------------------------------------------------------------------
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

    // Identified elements of the program.
    // Manually identified through investigation.
    std::vector<ProgElem> progElems;

    // Handlers for the 'jr' instruction
    std::vector<JRInstHandler> jrInstHandlers;

    // The assumed value of the '$gp' register throughout the program - manually identified for the program.
    // This is set in main() and should remain the same throughout the lifetime of the program.
    // Access to some globals depends on it, hence knowing it's value allows us to annotate access to those globals.
    uint32_t assumedGpRegisterValue;

    ExeFile() noexcept;

    // Load the playstation EXE from the given path.
    // Aborts the program if there is an error.
    void loadFromFile(const char* const path) noexcept;

    // Set the program elements for the program.
    // The list will be automatically sorted for fast lookup.
    void setProgElems(const ProgElem* const pElems, const uint32_t numElems) noexcept;

    // Set the 'jr' instruction handlers for the program
    void setJRInstHandlers(const JRInstHandler* const pHandlers, const uint32_t numHandlers) noexcept;

    // Try and find a program element at the given address.
    // Returns 'nullptr' if none found.
    const ProgElem* findProgElemAtAddr(const uint32_t addr) const noexcept;

    // Print the name of the element at the address.
    // Just prints the raw hex address if there is no such element.
    // Returns 'true' if an actual name was printed instead of a raw hex address.
    bool printNameOfElemAtAddr(const uint32_t addr, std::ostream& out) const;

    // Determine which words are referenced by jump instructions, data etc.
    // Useful in the disassembly to be able to quickly see branch targets and references to specific memory locations.
    void determineWordReferences() noexcept;
};

#pragma once

#include <cstdint>
#include <cassert>
#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a single 32-bit word in the .EXE.
// The word can either be code or data and is stored in little endian format.
//------------------------------------------------------------------------------------------------------------------------------------------
struct ExeWord {
    // The actual 32-bit value of the word.
    // This is the value that is read directly from the executable.
    uint32_t value;

    bool bIsJumpTarget;         // Is this word the target of a fixed jump instruction?
    bool bIsBranchTarget;       // Is this word the target of a branch instruction?
    bool bIsDataReferenced;     // Is this word referenced by the data in another 32-bit word? Might indicate a function pointer reference if so.

    // Start (inclusive) and end (exclusive) referencing address for the word.
    // This is used to tell when all references to a particular instruction are internal to a function - useful for boundary analysis.
    // When they are both equal to zero then no references are set to the word.
    uint32_t startReferencingAddr;
    uint32_t endReferencingAddr;

    // Get the address of the entire word, or the specified half word or byte within it based on the
    // given program base address and also the index of this word in the program:
    inline uint32_t getWordAddr(const uint32_t progBaseAddr, const uint32_t thisWordIdx) const noexcept {
        return progBaseAddr + thisWordIdx * sizeof(uint32_t);
    }

    inline uint32_t getHalfWordAddr(
        const uint32_t progBaseAddr,
        const uint32_t thisWordIdx,
        const uint8_t halfWordIdx
    ) noexcept {
        assert(halfWordIdx < 2);
        return getWordAddr(progBaseAddr, thisWordIdx) + halfWordIdx * sizeof(uint16_t);
    }

    inline uint32_t getByteAddr(
        const uint32_t progBaseAddr,
        const uint32_t thisWordIdx,
        const uint8_t byteIdx
    ) noexcept {
        assert(byteIdx < 4);
        return getWordAddr(progBaseAddr, thisWordIdx) + byteIdx;
    }

    // Make a note of the fact that the given word references this word.
    // Takes the address of the given word as input.
    inline void addReferencingWord(const uint32_t wordAddr) noexcept {
        const uint32_t wordEndAddr = wordAddr + 4;

        if (startReferencingAddr == endReferencingAddr) {
            // No references yet set!
            startReferencingAddr = wordAddr;
            endReferencingAddr = wordEndAddr;
        } else {
            // There are already references, grow the region:
            startReferencingAddr = std::min(startReferencingAddr, wordAddr);
            endReferencingAddr = std::max(endReferencingAddr, wordEndAddr);
        }
    }
};

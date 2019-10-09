#include <cstdint>
#include <cassert>

//----------------------------------------------------------------------------------------------------------------------
// Represents a single 32-bit word in the .EXE.
// The word can either be code or data and is stored in little endian format.
//----------------------------------------------------------------------------------------------------------------------
struct ExeWord {
    // The actual 32-bit value of the word.
    // This is the value that is read directly from the executable.
    uint32_t value; 

    // Whether each byte of data in the word has been categorized as some sort of code or data.
    // When the disassembler is given instructions as to what sort of information a region in the EXE represents, this
    // boolean will be set to 'true'. If multiple instructions claim the same program byte as something then a conflict/error
    // will be raised - each byte can only be classed as something once.
    bool bByteIsCategorized[4];

    // Functions to mark the EXE word as categorized or check that it is categorized
    inline void markWordCategorized() noexcept {
        bByteIsCategorized[0] = true;
        bByteIsCategorized[1] = true;
        bByteIsCategorized[2] = true;
        bByteIsCategorized[3] = true;
    }

    inline bool isWordCategorized() const noexcept {
        return (
            bByteIsCategorized[0] &&
            bByteIsCategorized[1] &&
            bByteIsCategorized[2] &&
            bByteIsCategorized[3]
        );
    }

    inline void markHalfWordCategorized(const uint8_t halfWordIdx) noexcept {
        assert(halfWordIdx < 2);

        if (halfWordIdx == 0) {
            bByteIsCategorized[0] = true;
            bByteIsCategorized[1] = true;
        } else {
            bByteIsCategorized[2] = true;
            bByteIsCategorized[3] = true;
        }
    }

    inline bool isHalfWordCategorized(const uint8_t halfWordIdx) const noexcept {
        assert(halfWordIdx < 2);

        if (halfWordIdx == 0) {
            return (bByteIsCategorized[0] && bByteIsCategorized[1]);
        } else {
            return (bByteIsCategorized[2] && bByteIsCategorized[3]);
        }        
    }

    inline void markByteCategorized(const uint8_t byteIdx) {
        assert(byteIdx < 4);
        bByteIsCategorized[byteIdx] = byteIdx;
    }

    inline bool isByteCategorized(const uint8_t byteIdx) const noexcept {
        assert(byteIdx < 4);
        return bByteIsCategorized[byteIdx];
    }

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
};

#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>

BEGIN_NAMESPACE(movie)

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a series of MDEC AC (alternating current) coefficients read from a bit stream.
// Holds a number of repeated coefficients reading '0' are followed by one non-zero coefficient.
// A special return value of this can also signify the end of the bit stream for the current block.
//------------------------------------------------------------------------------------------------------------------------------------------
struct ACCoeff {
    uint16_t    numZeroValueCoeff;
    int16_t     nonZeroCoeff;

    // Tells if this struct is a special marker representing the end of the block's bit stream
    inline bool isEOF() const { return (numZeroValueCoeff == 0xFFFFu); }

    // Returns a special value representing the end of the block's bit stream 
    static constexpr ACCoeff EOF() {
        return ACCoeff { 0xFFFFu, 0 };
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Movie block bitstream: provides the means for individual blocks in an MDEC movie frame to be read.
// Wraps an array of bytes and provides a bit oriented input stream from that array of bytes.
// 
// For more info on MDEC movie decoding see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
class MBlockBitStream {
public:
    // Represents an error type thrown by the bit stream
    enum ErrorType {
        UNEXPECTED_EOF,     // Reached an unexpected end of the data
        INVALID_ENCODING,   // The MDEC movie data is not a valid encoding
        INTERNAL_ERROR      // The MDEC decoder encountered an unexpected internal logic error (this should never be thrown, hopefully)
    };

    MBlockBitStream() noexcept;

    void open(const uint16_t* pWords, const uint32_t numWords) noexcept;
    void close() noexcept;
    inline bool isOpen() const { return (mpWords != nullptr); }

    uint16_t readBit() THROWS;

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Reads the specified number of bits (up to 16) as an unsigned integer.
    // Throws an exception if this is not possible.
    //--------------------------------------------------------------------------------------------------------------------------------------
    template <uint16_t NumReadBits>
    uint16_t readBits() THROWS {
        // Sanity check the number of bits is ok
        static_assert((NumReadBits >= 1) && (NumReadBits <= 16));

        if constexpr (NumReadBits == 1) {
            // Special case: reading a single bit
            return readBit();
        } else {
            // Regular case: reading 2 or more bits, may have to extract bits from 2 different words.
            // First check for the easy case where all bits are available in the currently loaded word:
            const uint16_t numBitsConsumed = mCurWordShift;
            const uint16_t numBitsFree = 16 - numBitsConsumed;
            constexpr uint16_t READ_BITS_MASK = 0xFFFFu >> (16u - NumReadBits);

            if (NumReadBits <= numBitsFree) {
                const uint16_t bits = (mCurWord >> numBitsConsumed) & READ_BITS_MASK;
                mCurWordShift = numBitsConsumed + NumReadBits;
                return bits;
            }

            // Harder case: must split up the read across two 16-bit words; read the first chunk of bits:
            const uint16_t lowBits = (mCurWord >> numBitsConsumed) & READ_BITS_MASK;
            const uint16_t numLowBits = 16 - numBitsConsumed;
            const uint16_t numHighBits = NumReadBits - numLowBits;
            const uint16_t highBitsMask = READ_BITS_MASK >> numLowBits;

            // Get a fresh word and read the rest of the bits
            nextWord();
            const uint16_t highBits = (mCurWord & highBitsMask) << numLowBits;
            mCurWordShift = numHighBits;
            return lowBits | highBits;
        }
    }

    ACCoeff readACCoeff() THROWS;

private:
    void nextWord() THROWS;

    ACCoeff readACCoeffSign(const uint16_t numZeroValueCoeff, const int16_t nonZeroCoeff) THROWS;
    ACCoeff readACCoeff_00() THROWS;
    ACCoeff readACCoeff_01() THROWS;
    ACCoeff readACCoeff_001() THROWS;
    ACCoeff readACCoeff_0001() THROWS;
    ACCoeff readACCoeff_0000_1() THROWS;
    ACCoeff readACCoeff_0000_01() THROWS;
    ACCoeff readACCoeff_0000_001() THROWS;
    ACCoeff readACCoeff_0000_0001() THROWS;
    ACCoeff readACCoeff_0000_0000_1() THROWS;
    ACCoeff readACCoeff_0000_0000_01() THROWS;
    ACCoeff readACCoeff_0000_0000_001() THROWS;
    ACCoeff readACCoeff_0000_0000_0001() THROWS;

    const uint16_t* mpWords;            // The array of 16-bit (little endian) words to be read
    uint32_t        mSize;              // How many 16-bit words there are in 'mpWords'
    uint32_t        mCurOffset;         // Which word we are currently on in 'mpWords'
    uint16_t        mCurWord;           // The current word we are reading bits from
    uint16_t        mCurWordShift;      // How many bits have been consumed from the current word
};

END_NAMESPACE(movie)

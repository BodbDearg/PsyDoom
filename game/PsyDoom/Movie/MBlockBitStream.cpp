#include "MBlockBitStream.h"

#include "Asserts.h"
#include "Endian.h"

BEGIN_NAMESPACE(movie)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an unopened bit stream
//------------------------------------------------------------------------------------------------------------------------------------------
MBlockBitStream::MBlockBitStream() noexcept
    : mpWords(nullptr)
    , mSize(0)
    , mCurOffset(0)
    , mCurWord(0)
    , mCurWordShift(16)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the bit stream for the specified array of words
//------------------------------------------------------------------------------------------------------------------------------------------
void MBlockBitStream::open(const uint16_t* pWords, const uint32_t numWords) noexcept {
    ASSERT(pWords || (numWords == 0));

    mpWords = pWords;
    mSize = numWords;
    mCurOffset = 0;
    mCurWord = 0;
    mCurWordShift = 16;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes up the bit stream
//------------------------------------------------------------------------------------------------------------------------------------------
void MBlockBitStream::close() noexcept {
    mpWords = nullptr;
    mSize = 0;
    mCurOffset = 0;
    mCurWord = 0;
    mCurWordShift = 16;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a single bit; throws an exception if that is not possible
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t MBlockBitStream::readBit() THROWS {
    // Get the next 16-bit word of data if all bits are consumed
    if (mCurWordShift >= 16) {
        nextWord();
    }

    // Return the next bit in the currently loaded word
    const uint16_t bit = (mCurWord >> mCurWordShift) & 0x1;
    mCurWordShift++;
    return bit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read a series of AC coefficients from the bit stream
//------------------------------------------------------------------------------------------------------------------------------------------
ACCoeff MBlockBitStream::readACCoeff() THROWS {
    // Read the first two bits of the variable length code.
    // There will always be at least two bits in one of these:
    const uint16_t topBits = readBits<2>();

    switch (topBits) {
        case 0b00:  return readACCoeff_00();            // 00
        case 0b01:  return readACCoeff_01();            // 01
        case 0b10:  return ACCoeff::EOF();              // 10 (EOF)
        case 0b11:  return readACCoeffSign(0, 1);       // 11
    }

    throw ErrorType::INTERNAL_ERROR;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the next 16-bit word into the bit stream.
// Throws an exception if there is no more data available.
//------------------------------------------------------------------------------------------------------------------------------------------
void MBlockBitStream::nextWord() THROWS {
    if (mCurOffset >= mSize)
        throw ErrorType::UNEXPECTED_EOF;

    if constexpr (Endian::isLittle()) {
        mCurWord = mpWords[mCurOffset];
    } else {
        mCurWord = Endian::byteSwap(mpWords[mCurOffset]);
    }

    mCurWordShift = 0;
    mCurOffset++;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: reads the sign bit for the given 'ACCoeff' which is split up into separate fields.
// Returns the coefficient struct, after the input sign bit has been applied.
//------------------------------------------------------------------------------------------------------------------------------------------
ACCoeff MBlockBitStream::readACCoeffSign(const uint16_t numZeroValueCoeff, const int16_t nonZeroCoeff) THROWS {
    const int16_t signedNonZeroCoeff = (readBit()) ? -nonZeroCoeff : +nonZeroCoeff;
    return { numZeroValueCoeff, signedNonZeroCoeff };
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Low level details of reading AC coefficients, broken up into functions by the preceeding bit patterns.
// Some of this is a bit tedious, but it does the job.
// 
// The details for all these bit patterns and their meaning can be found here:
//  https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
ACCoeff MBlockBitStream::readACCoeff_00() THROWS {
    if (readBit())  return readACCoeff_001();               // 001
    if (readBit())  return readACCoeff_0001();              // 0001
    if (readBit())  return readACCoeff_0000_1();            // 0000 1
    if (readBit())  return readACCoeff_0000_01();           // 0000 01
    if (readBit())  return readACCoeff_0000_001();          // 0000 001
    if (readBit())  return readACCoeff_0000_0001();         // 0000 0001
    if (readBit())  return readACCoeff_0000_0000_1();       // 0000 0000 1
    if (readBit())  return readACCoeff_0000_0000_01();      // 0000 0000 01
    if (readBit())  return readACCoeff_0000_0000_001();     // 0000 0000 001
    if (readBit())  return readACCoeff_0000_0000_0001();    // 0000 0000 0001

    throw ErrorType::INVALID_ENCODING;
}

ACCoeff MBlockBitStream::readACCoeff_01() THROWS {
    if (readBit()) {
        // 011
        return readACCoeffSign(1, 1);
    } else {
        // 010
        if (readBit()) {
            return readACCoeffSign(2, 1);   // 0101
        } else {
            return readACCoeffSign(0, 2);   // 0100
        }
    }
}

ACCoeff MBlockBitStream::readACCoeff_001() THROWS {
    if (readBit()) {
        // 0011
        if (readBit()) {
            return readACCoeffSign(3, 1);   // 0011 1
        } else {
            return readACCoeffSign(4, 1);   // 0011 0
        }
    } else {
        // 0010
        if (readBit()) {
            return readACCoeffSign(0, 3);   // 0010 1
        } else {
            // 0010 0
            const uint16_t bits = readBits<3>();

            switch (bits) {
                case 0b000: return readACCoeffSign(13, 1);      // 0010 0000
                case 0b001: return readACCoeffSign(0, 6);       // 0010 0001
                case 0b010: return readACCoeffSign(12, 1);      // 0010 0010
                case 0b011: return readACCoeffSign(11, 1);      // 0010 0011
                case 0b100: return readACCoeffSign(3, 2);       // 0010 0100
                case 0b101: return readACCoeffSign(1, 3);       // 0010 0101
                case 0b110: return readACCoeffSign(0, 5);       // 0010 0110
                case 0b111: return readACCoeffSign(10, 1);      // 0010 0111
            }
        }
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0001() THROWS {
    const uint16_t bits = readBits<2>();
    
    switch (bits) {
        case 0b00: return readACCoeffSign(7, 1);    // 0001 00
        case 0b01: return readACCoeffSign(6, 1);    // 0001 01
        case 0b10: return readACCoeffSign(1, 2);    // 0001 10
        case 0b11: return readACCoeffSign(5, 1);    // 0001 11
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_1() THROWS {
    const uint16_t bits = readBits<2>();
    
    switch (bits) {
        case 0b00: return readACCoeffSign(2, 2);    // 0000 100
        case 0b01: return readACCoeffSign(9, 1);    // 0000 101
        case 0b10: return readACCoeffSign(0, 4);    // 0000 110
        case 0b11: return readACCoeffSign(8, 1);    // 0000 111
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_01() THROWS {
    // This is an escape code.
    // Following it are 6-bits for the number of zeros (high bits).
    // Following that is a 10 bit signed (twos complement) non zero coefficient value.
    const uint16_t bits = readBits<16>();
    const uint16_t numZeros = bits >> 10;
    const int16_t coeffNonSignBits = bits & 0x1FF;
    const int16_t coeff = (bits & 0x200) ? coeffNonSignBits - 0x200 : coeffNonSignBits; // Is it negative?

    return ACCoeff{ numZeros, coeff };
}

ACCoeff MBlockBitStream::readACCoeff_0000_001() THROWS {
    const uint16_t bits = readBits<3>();

    switch (bits) {
        case 0b000: return readACCoeffSign(16, 1);      // 0000 0010 00
        case 0b001: return readACCoeffSign(5, 2);       // 0000 0010 01
        case 0b010: return readACCoeffSign(0, 7);       // 0000 0010 10
        case 0b011: return readACCoeffSign(2, 3);       // 0000 0010 11
        case 0b100: return readACCoeffSign(1, 4);       // 0000 0011 00
        case 0b101: return readACCoeffSign(15, 1);      // 0000 0011 01
        case 0b110: return readACCoeffSign(14, 1);      // 0000 0011 10
        case 0b111: return readACCoeffSign(4, 2);       // 0000 0011 11
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_0001() THROWS {
    const uint16_t bits = readBits<4>();

    switch (bits) {
        case 0b0000: return readACCoeffSign(0, 11);     // 0000 0001 0000
        case 0b0001: return readACCoeffSign(8, 2);      // 0000 0001 0001
        case 0b0010: return readACCoeffSign(4, 3);      // 0000 0001 0010
        case 0b0011: return readACCoeffSign(0, 10);     // 0000 0001 0011
        case 0b0100: return readACCoeffSign(2, 4);      // 0000 0001 0100
        case 0b0101: return readACCoeffSign(7, 2);      // 0000 0001 0101
        case 0b0110: return readACCoeffSign(21, 1);     // 0000 0001 0110
        case 0b0111: return readACCoeffSign(20, 1);     // 0000 0001 0111
        case 0b1000: return readACCoeffSign(0, 9);      // 0000 0001 1000
        case 0b1001: return readACCoeffSign(19, 1);     // 0000 0001 1001
        case 0b1010: return readACCoeffSign(18, 1);     // 0000 0001 1010
        case 0b1011: return readACCoeffSign(1, 5);      // 0000 0001 1011
        case 0b1100: return readACCoeffSign(3, 3);      // 0000 0001 1100
        case 0b1101: return readACCoeffSign(0, 8);      // 0000 0001 1101
        case 0b1110: return readACCoeffSign(6, 2);      // 0000 0001 1110
        case 0b1111: return readACCoeffSign(17, 1);     // 0000 0001 1111
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_0000_1() THROWS {
    const uint16_t bits = readBits<4>();

    switch (bits) {
        case 0b0000: return readACCoeffSign(10, 2);     // 0000 0000 1000 0
        case 0b0001: return readACCoeffSign(9, 2);      // 0000 0000 1000 1
        case 0b0010: return readACCoeffSign(5, 3);      // 0000 0000 1001 0
        case 0b0011: return readACCoeffSign(3, 4);      // 0000 0000 1001 1
        case 0b0100: return readACCoeffSign(2, 5);      // 0000 0000 1010 0
        case 0b0101: return readACCoeffSign(1, 7);      // 0000 0000 1010 1
        case 0b0110: return readACCoeffSign(1, 6);      // 0000 0000 1011 0
        case 0b0111: return readACCoeffSign(0, 15);     // 0000 0000 1011 1
        case 0b1000: return readACCoeffSign(0, 14);     // 0000 0000 1100 0
        case 0b1001: return readACCoeffSign(0, 13);     // 0000 0000 1100 1
        case 0b1010: return readACCoeffSign(0, 12);     // 0000 0000 1101 0
        case 0b1011: return readACCoeffSign(26, 1);     // 0000 0000 1101 1
        case 0b1100: return readACCoeffSign(25, 1);     // 0000 0000 1110 0
        case 0b1101: return readACCoeffSign(24, 1);     // 0000 0000 1110 1
        case 0b1110: return readACCoeffSign(23, 1);     // 0000 0000 1111 0
        case 0b1111: return readACCoeffSign(22, 1);     // 0000 0000 1111 1
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_0000_01() THROWS {
    const uint16_t bits = readBits<4>();

    switch (bits) {
        case 0b0000: return readACCoeffSign(0, 31);     // 0000 0000 0100 00
        case 0b0001: return readACCoeffSign(0, 30);     // 0000 0000 0100 01
        case 0b0010: return readACCoeffSign(0, 29);     // 0000 0000 0100 10
        case 0b0011: return readACCoeffSign(0, 28);     // 0000 0000 0100 11
        case 0b0100: return readACCoeffSign(0, 27);     // 0000 0000 0101 00
        case 0b0101: return readACCoeffSign(0, 26);     // 0000 0000 0101 01
        case 0b0110: return readACCoeffSign(0, 25);     // 0000 0000 0101 10
        case 0b0111: return readACCoeffSign(0, 24);     // 0000 0000 0101 11
        case 0b1000: return readACCoeffSign(0, 23);     // 0000 0000 0110 00
        case 0b1001: return readACCoeffSign(0, 22);     // 0000 0000 0110 01
        case 0b1010: return readACCoeffSign(0, 21);     // 0000 0000 0110 10
        case 0b1011: return readACCoeffSign(0, 20);     // 0000 0000 0110 11
        case 0b1100: return readACCoeffSign(0, 19);     // 0000 0000 0111 00
        case 0b1101: return readACCoeffSign(0, 18);     // 0000 0000 0111 01
        case 0b1110: return readACCoeffSign(0, 17);     // 0000 0000 0111 10
        case 0b1111: return readACCoeffSign(0, 16);     // 0000 0000 0111 11
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_0000_001() THROWS {
    const uint16_t bits = readBits<4>();

    switch (bits) {
        case 0b0000: return readACCoeffSign(0, 40);     // 0000 0000 0010 000
        case 0b0001: return readACCoeffSign(0, 39);     // 0000 0000 0010 001
        case 0b0010: return readACCoeffSign(0, 38);     // 0000 0000 0010 010
        case 0b0011: return readACCoeffSign(0, 37);     // 0000 0000 0010 011
        case 0b0100: return readACCoeffSign(0, 36);     // 0000 0000 0010 100
        case 0b0101: return readACCoeffSign(0, 35);     // 0000 0000 0010 101
        case 0b0110: return readACCoeffSign(0, 34);     // 0000 0000 0010 110
        case 0b0111: return readACCoeffSign(0, 33);     // 0000 0000 0010 111
        case 0b1000: return readACCoeffSign(0, 32);     // 0000 0000 0011 000
        case 0b1001: return readACCoeffSign(1, 14);     // 0000 0000 0011 001
        case 0b1010: return readACCoeffSign(1, 13);     // 0000 0000 0011 010
        case 0b1011: return readACCoeffSign(1, 12);     // 0000 0000 0011 011
        case 0b1100: return readACCoeffSign(1, 11);     // 0000 0000 0011 100
        case 0b1101: return readACCoeffSign(1, 10);     // 0000 0000 0011 101
        case 0b1110: return readACCoeffSign(1, 9);      // 0000 0000 0011 110
        case 0b1111: return readACCoeffSign(1, 8);      // 0000 0000 0011 111
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

ACCoeff MBlockBitStream::readACCoeff_0000_0000_0001() THROWS {
    const uint16_t bits = readBits<4>();

    switch (bits) {
        case 0b0000: return readACCoeffSign(1, 18);     // 0000 0000 0001 0000
        case 0b0001: return readACCoeffSign(1, 17);     // 0000 0000 0001 0001
        case 0b0010: return readACCoeffSign(1, 16);     // 0000 0000 0001 0010
        case 0b0011: return readACCoeffSign(1, 15);     // 0000 0000 0001 0011
        case 0b0100: return readACCoeffSign(6, 3);      // 0000 0000 0001 0100
        case 0b0101: return readACCoeffSign(16, 2);     // 0000 0000 0001 0101
        case 0b0110: return readACCoeffSign(15, 2);     // 0000 0000 0001 0110
        case 0b0111: return readACCoeffSign(14, 2);     // 0000 0000 0001 0111
        case 0b1000: return readACCoeffSign(13, 2);     // 0000 0000 0001 1000
        case 0b1001: return readACCoeffSign(12, 2);     // 0000 0000 0001 1001
        case 0b1010: return readACCoeffSign(11, 2);     // 0000 0000 0001 1010
        case 0b1011: return readACCoeffSign(31, 1);     // 0000 0000 0001 1011
        case 0b1100: return readACCoeffSign(30, 1);     // 0000 0000 0001 1100
        case 0b1101: return readACCoeffSign(29, 1);     // 0000 0000 0001 1101
        case 0b1110: return readACCoeffSign(28, 1);     // 0000 0000 0001 1110
        case 0b1111: return readACCoeffSign(27, 1);     // 0000 0000 0001 1111
    }

    throw ErrorType::INTERNAL_ERROR;    // Should never reach here!
}

END_NAMESPACE(movie)

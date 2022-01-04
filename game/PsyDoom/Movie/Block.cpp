#include "Block.h"

#include "MBlockBitStream.h"

#include <cstring>

BEGIN_NAMESPACE(movie)

//------------------------------------------------------------------------------------------------------------------------------------------
// A matrix defining what indexes to swap into each matrix slot to perform an 'un-zig-zig' transformation.
// This same transformation is done during MPEG1 and JPEG decoding. For more info see:
//  https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint8_t UNZIGZAG_MATRIX[Block::PIXELS_H][Block::PIXELS_W] = {
    {  0,  1,  5,  6, 14, 15, 27, 28 },
    {  2,  4,  7, 13, 16, 26, 29, 42 },
    {  3,  8, 12, 17, 25, 30, 41, 43 },
    {  9, 11, 18, 24, 31, 40, 44, 53 },
    { 10, 19, 23, 32, 39, 45, 52, 54 },
    { 20, 22, 33, 38, 46, 51, 55, 60 },
    { 21, 34, 37, 47, 50, 56, 59, 61 },
    { 35, 36, 48, 49, 57, 58, 62, 63 }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Scaling values that each matrix element must be multipled by during the dequantization step.
// This same table is used by almost every PSX game. For more info see:
//  https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t QUANTIZATION_SCALE_MATRIX[Block::PIXELS_H][Block::PIXELS_W] = {
    {  2, 16, 19, 22, 26, 27, 29, 34 },
    { 16, 16, 22, 24, 27, 29, 34, 37 },
    { 19, 22, 26, 27, 29, 34, 34, 38 },
    { 22, 22, 26, 27, 29, 34, 37, 40 },
    { 22, 26, 27, 29, 32, 35, 40, 48 },
    { 26, 27, 29, 32, 35, 40, 48, 58 },
    { 26, 27, 29, 34, 38, 46, 56, 69 },
    { 27, 29, 35, 38, 46, 56, 69, 83 }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// 0.16 fixed point scaling matrix that is used to apply the inverse discrete cosine transform.
// This same table is used by almost every PSX game. For more info see:
//  https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t IDCT_M[Block::PIXELS_H][Block::PIXELS_W] = {
    { 23170,  23170,  23170,  23170,  23170,  23170,  23170,  23170 },
    { 32138,  27245,  18204,   6392,  -6393, -18205, -27246, -32139 },
    { 30273,  12539, -12540, -30274, -30274, -12540,  12539,  30273 },
    { 27245,  -6393, -32139, -18205,  18204,  32138,   6392, -27246 },
    { 23170, -23171, -23171,  23170,  23170, -23171, -23171,  23170 },
    { 18204, -32139,   6392,  27245, -27246,  -6393,  32138, -18205 },
    { 12539, -30274,  30273, -12540, -12540,  30273, -30274,  12539 },
    {  6392, -18205,  27245, -32139,  32138, -27246,  18204,  -6393 },
};

// Transpose of the inverse discrete cosine transform matrix
static constexpr int16_t IDCT_MT[Block::PIXELS_H][Block::PIXELS_W] = {
    { IDCT_M[0][0], IDCT_M[1][0], IDCT_M[2][0], IDCT_M[3][0], IDCT_M[4][0], IDCT_M[5][0], IDCT_M[6][0], IDCT_M[7][0], },
    { IDCT_M[0][1], IDCT_M[1][1], IDCT_M[2][1], IDCT_M[3][1], IDCT_M[4][1], IDCT_M[5][1], IDCT_M[6][1], IDCT_M[7][1], },
    { IDCT_M[0][2], IDCT_M[1][2], IDCT_M[2][2], IDCT_M[3][2], IDCT_M[4][2], IDCT_M[5][2], IDCT_M[6][2], IDCT_M[7][2], },
    { IDCT_M[0][3], IDCT_M[1][3], IDCT_M[2][3], IDCT_M[3][3], IDCT_M[4][3], IDCT_M[5][3], IDCT_M[6][3], IDCT_M[7][3], },
    { IDCT_M[0][4], IDCT_M[1][4], IDCT_M[2][4], IDCT_M[3][4], IDCT_M[4][4], IDCT_M[5][4], IDCT_M[6][4], IDCT_M[7][4], },
    { IDCT_M[0][5], IDCT_M[1][5], IDCT_M[2][5], IDCT_M[3][5], IDCT_M[4][5], IDCT_M[5][5], IDCT_M[6][5], IDCT_M[7][5], },
    { IDCT_M[0][6], IDCT_M[1][6], IDCT_M[2][6], IDCT_M[3][6], IDCT_M[4][6], IDCT_M[5][6], IDCT_M[6][6], IDCT_M[7][6], },
    { IDCT_M[0][7], IDCT_M[1][7], IDCT_M[2][7], IDCT_M[3][7], IDCT_M[4][7], IDCT_M[5][7], IDCT_M[6][7], IDCT_M[7][7], },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Does one matrix multiply for the inverse discrete cosine transform.
// Assumes one of the matrixes is in 0.16 fixed point format.
//------------------------------------------------------------------------------------------------------------------------------------------
static void doIdctMatrixMultiply(
    const int16_t matrix1[Block::PIXELS_H][Block::PIXELS_W],
    const int16_t matrix2[Block::PIXELS_H][Block::PIXELS_W],
    int16_t outMatrix[Block::PIXELS_H][Block::PIXELS_W]
) noexcept {
    static_assert(Block::PIXELS_W == Block::PIXELS_H);  // Assuming a square matrix in this function

    for (uint32_t row = 0; row < Block::PIXELS_H; ++row) {
        for (uint32_t col = 0; col < Block::PIXELS_W; ++col) {
            // Note: the numbers in the value (non IDCT) matrix should be between -2048 and 2047 (12 bits needed) and the IDCT matrix itself
            // needs 16-bits of precision. The sum is done over 8 elements so that should be an additional 4-bits of precision required, for
            // a total of 32-bits used. Because of this I'm dropping 4-bits during calculations to avoid overflow, just to be safe.
            // 
            // For more on this see:
            //  https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
            int32_t sum = 0;

            for (uint32_t i = 0; i < Block::PIXELS_W; ++i) {
                sum += ((int32_t) matrix1[row][i] * matrix2[i][col]) >> 4;  // Chop off a few fractional 16.16 bits to prevent overflow
            }

            sum >>= 12; // Remove the rest of the fixed point fractional bits from the number
            outMatrix[row][col] = (int16_t) sum;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reorders the matrix values in the block so that they are no longer in MPEG1/JPEG 'zig-zag' order
//------------------------------------------------------------------------------------------------------------------------------------------
static void unZigZagBlock(Block& block) noexcept {
    // Copy the original list of values that are in zig-zag order and flatten the list to 1 dimension
    int16_t origMatrix[Block::PIXELS_W * Block::PIXELS_H];
    std::memcpy(origMatrix, block.mValues, sizeof(block.mValues));

    // Perform the un-zig-zag transform
    for (uint32_t y = 0; y < Block::PIXELS_H; ++y) {
        for (uint32_t x = 0; x < Block::PIXELS_W; ++x) {
            const uint32_t origIdx = UNZIGZAG_MATRIX[y][x];
            block.mValues[y][x] = origMatrix[origIdx];
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decoding step: de-quantizes the given block's value matrix or scales the values
//------------------------------------------------------------------------------------------------------------------------------------------
static void dequantizeBlock(Block& block, const int16_t quantizationScale) noexcept {
    // The first row has the first value treated differently, hardcode this row to avoid conditionals:
    static_assert(Block::PIXELS_W == 8);

    block.mValues[0][0] = (int16_t)(block.mValues[0][0] * QUANTIZATION_SCALE_MATRIX[0][0]);
    block.mValues[0][1] = (int16_t)((2 * block.mValues[0][1] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][1]) / 16);
    block.mValues[0][2] = (int16_t)((2 * block.mValues[0][2] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][2]) / 16);
    block.mValues[0][3] = (int16_t)((2 * block.mValues[0][3] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][3]) / 16);
    block.mValues[0][4] = (int16_t)((2 * block.mValues[0][4] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][4]) / 16);
    block.mValues[0][5] = (int16_t)((2 * block.mValues[0][5] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][5]) / 16);
    block.mValues[0][6] = (int16_t)((2 * block.mValues[0][6] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][6]) / 16);
    block.mValues[0][7] = (int16_t)((2 * block.mValues[0][7] * quantizationScale * QUANTIZATION_SCALE_MATRIX[0][7]) / 16);

    // Do the rest of the rows in a normal loop
    for (uint32_t y = 1; y < Block::PIXELS_H; ++y) {
        for (uint32_t x = 0; x < Block::PIXELS_W; ++x) {
            block.mValues[y][x] = (int16_t)((2 * block.mValues[y][x] * quantizationScale * QUANTIZATION_SCALE_MATRIX[y][x]) / 16);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decoding step: applies the inverse discrete cosine transform to a block.
// For more on this see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyInverseDiscreteCosineTransformToBlock(Block& block) noexcept {
    int16_t tmpMatrix[Block::PIXELS_H][Block::PIXELS_W];
    doIdctMatrixMultiply(IDCT_MT, block.mValues, tmpMatrix);
    doIdctMatrixMultiply(tmpMatrix, IDCT_M, block.mValues);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decoding step: reads the DC and AC coefficients for the specified block.
// Assumes the block has already been zero filled to start with.
// For more details on this, see: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
//------------------------------------------------------------------------------------------------------------------------------------------
static void readDcAndAcCoeffForBlock(Block& block, MBlockBitStream& inputStream) THROWS {
    // Read the DC coefficient firstly (10 bits signed)
    {
        const uint16_t dcBits = inputStream.readBits<10>();
        const int16_t dcNonSignBits = dcBits & 0x1FF;
        block.mValues[0][0] = (dcBits & 0x200) ? dcNonSignBits - 0x200 : dcNonSignBits;
    }

    // Read the AC coefficients until EOF is encountered.
    // If too many are provided then that is an error (63 max are allowed).
    // Note that the first entry in the array is taken up by the DC coefficient.
    constexpr uint32_t END_COEFF_IDX = Block::PIXELS_W * Block::PIXELS_H;
    int16_t* const pCoeff = &block.mValues[0][0];
    uint32_t curCoeffIdx = 1;

    while (true) {
        // Read the AC coefficients and check for the end of the stream
        const ACCoeff coeff = inputStream.readACCoeff();

        if (coeff.isEof())
            break;

        // Skip past all the zero value AC coefficients specified by this run.
        // Note: I'm assuming here that the entire block has already been zero initialized!
        curCoeffIdx += coeff.numZeroValueCoeff;

        // If there are too many coefficients then that is an encoding error
        if (curCoeffIdx >= END_COEFF_IDX)
            throw MBlockBitStream::ErrorType::INVALID_ENCODING;

        // Save the AC coefficient
        pCoeff[curCoeffIdx] = coeff.nonZeroCoeff;
        ++curCoeffIdx;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Zero initializes the block's values
//------------------------------------------------------------------------------------------------------------------------------------------
void Block::clear() noexcept {
    for (uint32_t y = 0; y < PIXELS_H; ++y) {
        for (uint32_t x = 0; x < PIXELS_W; ++x) {
            mValues[y][x] = 0;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to read and decode the block of values.
// Takes the quantization scale for the frame as input.
//------------------------------------------------------------------------------------------------------------------------------------------
bool Block::readAndDecode(MBlockBitStream& inputStream, const int16_t quantizationScale) noexcept {
    // The set of AC coefficients (63 total) doesn't have to be complete within the stream.
    // The unspecified ones must be zero-initialized if not provided:
    clear();

    // Firstly read/decode the DC coefficients and all the AC coefficients.
    // If that fails clear the block and abort with failure:
    try {
        readDcAndAcCoeffForBlock(*this, inputStream);
    } catch (...) {
        clear();
        return false;
    }

    // Reverse the zig-zag matrix order, dequantize and apply the inverse discrete cosine transform.
    // This yields the final block values.
    unZigZagBlock(*this);
    dequantizeBlock(*this, quantizationScale);
    applyInverseDiscreteCosineTransformToBlock(*this);

    // All good if we've made it to here!
    return true;
}

END_NAMESPACE(movie)

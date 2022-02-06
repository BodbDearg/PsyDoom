#include "d_vsprintf.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// A custom 'strlen' style function that gracefully handles nulls
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t D_mystrlen(const char* pStr) noexcept {
    if (pStr) {
        int32_t len = 0;

        while (*(pStr++)) {
            ++len;
        }

        return len;
    } else {
        return -1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Printf style printing to a character buffer
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t D_vsprintf(char* dstStr, const char* fmtStr, va_list args) noexcept {
    char* const startDstStr = dstStr;

    while (*fmtStr) {
        // Easy case: a regular character, no placeholders - just copy the input/format string
        if (*fmtStr != '%') {
            *dstStr = *fmtStr;
            ++fmtStr;
            ++dstStr;
            continue;
        }

        // Skip the '%' and handle the padding character specifier (if there)
        ++fmtStr;
        char paddingChar;

        if (*fmtStr == '0') {
            paddingChar = '0';
            ++fmtStr;
        } else {
            paddingChar = ' ';
        }

        // Determine the fieldwidth, if specified
        int32_t fieldWidth = 0;

        while (*fmtStr >= '0' && *fmtStr <= '9') {
            const char fmtChar = *fmtStr;
            ++fmtStr;
            fieldWidth = fieldWidth * 10 + (uint8_t)(fmtChar - '0');
        }

        // Consume 'l' (long) specifier.
        // This is not actually used for any purpose, longs are treated as 32-bit integers too in this case...
        if (*fmtStr == 'l') {
            ++fmtStr;
        }

        // Handle a single character arg if specified
        if (*fmtStr == 'c') {
            const int singleChar = va_arg(args, int);
            *dstStr = (char) singleChar;
            ++dstStr;
            ++fmtStr;
            continue;
        }

        // Handle a string arg if specified
        if (*fmtStr == 's') {
            const char* argStr = va_arg(args, const char*);

            // Padding the string if required
            const int32_t argStrLen = D_mystrlen(argStr);

            while ((fieldWidth--) > argStrLen) {
                *dstStr = paddingChar;
                ++dstStr;
            }

            // Copying the argument string verbatim to the destination
            while (*argStr != 0) {
                *dstStr = *argStr;
                ++argStr;
                ++dstStr;
            }

            ++fmtStr;
            continue;
        }

        // Dealing with a numeric of some sort - figure out the format
        uint32_t num = UINT32_MAX;
        uint32_t numBase = UINT32_MAX;

        if (*fmtStr == 'o') {
            // Octal unsigned number
            numBase = 8;
            num = va_arg(args, uint32_t);
        }
        else if (*fmtStr == 'x' || *fmtStr == 'X') {
            // Hex unsigned number
            numBase = 16;
            num = va_arg(args, uint32_t);
        }
        else if (*fmtStr == 'i' || *fmtStr == 'd' || *fmtStr == 'u') {
            // Decimal number - signed or unsigned
            numBase = 10;
            const int32_t snum = va_arg(args, int32_t);

            // Negative number? If so then print '-' and get it's absolute value
            if (snum < 0 && *fmtStr != 'u') {
                *dstStr = '-';
                ++dstStr;
                num = -snum;

                if (fieldWidth != 0) {
                    --fieldWidth;
                }
            } else {
                num = snum;
            }
        } else {
            // Invalid format specifier!
            return -1;
        }

        // Print the unsigned integer
        uint32_t numLen = 0;

        while (num != 0 || fieldWidth != 0 || numLen == 0) {
            // Make room for a new digit, shift the existing string right by 1 char
            {
                char* const pStart = dstStr;
                char* pCur = pStart + numLen;

                while (pCur != pStart) {
                    *pCur = pCur[-1];
                    --pCur;
                }
            }

            // Pad the string, or put in a digit depending on whether we have padding left to do and the value left to process
            if (numLen != 0 && fieldWidth != 0 && num == 0) {
                // Pad with a character
                *dstStr = paddingChar;
            } else {
                // Put in a digit and move along in the number processing
                *dstStr = (char)(num % numBase);

                if (*dstStr >= 10) {
                    *dstStr += 'A' - 10;
                } else {
                    *dstStr += '0';
                }

                num = num / numBase;
            }

            numLen++;

            if (fieldWidth != 0) {
                --fieldWidth;
            }
        }

        dstStr += numLen;
        ++fmtStr;
    }

    *dstStr = 0;
    return (int32_t)(startDstStr - dstStr);
}

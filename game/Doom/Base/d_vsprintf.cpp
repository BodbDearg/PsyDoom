#include "d_vsprintf.h"

#include "PsxVm/PsxVm.h"
#include "PsxVm/VmPtr.h"

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

void D_vsprintf() noexcept {
    const VmPtr<char> startDstStr = a0;
    VmPtr<char> dstStr = a0;
    VmPtr<const char> fmtStr = a1;
    VmPtr<uint32_t> argPtr = a2;

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

        // Consume 'l' (long) specifier
        a3 = 0;

        if (*fmtStr == 'l') {
            a3 = 1;
            ++fmtStr;
        }

        // Handle a single character arg if specified
        if (*fmtStr == 'c') {
            const char singleChar = (char)(uint8_t) *argPtr;
            *dstStr = singleChar;
            ++dstStr;
            ++argPtr;
            ++fmtStr;
            continue;
        }

        // Handle a string arg if specified
        if (*fmtStr == 's') {
            VmPtr<const char> argStr(*argPtr);
            ++argPtr;

            // Padding the string if required
            const int32_t argStrLen = D_mystrlen(argStr.get());

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

        v1 = (uint8_t) *fmtStr;
        v0 = 0x6F;
        {
            const bool bJump = (v1 != v0);
            v0 = 0x78;
            if (bJump) goto loc_8003123C;
        }
        t3 = 8;
        goto loc_80031250;
    loc_8003123C:
        t3 = 0x10;
        if (v1 == v0) goto loc_80031250;
        v0 = 0x58;
        {
            const bool bJump = (v1 != v0);
            v0 = 0x69;
            if (bJump) goto loc_8003125C;
        }
    loc_80031250:
        a3 = *argPtr;
        ++argPtr;
        goto loc_800312B4;
    loc_8003125C:
        t3 = 0xA;
        if (v1 == v0) goto loc_80031278;
        v0 = 0x64;
        if (v1 == v0) goto loc_80031278;

        v0 = -1;
        if (v1 != 0x75) goto loc_8003138C;

    loc_80031278:
        v1 = *argPtr;
        ++argPtr;

        if (i32(v1) < 0 && (uint8_t) *fmtStr != 0x75) {
            *dstStr = 0x2D;
            ++dstStr;
            a3 = -v1;
            if (fieldWidth != 0) {
                --fieldWidth;
            }
        } else {
            a3 = v1;
        }

    loc_800312B4:
        t0 = 0;
    loc_800312B8:
        v1 = t0 + dstStr;
        if (t0 == 0) goto loc_800312FC;
        t1 = dstStr;

        do {
            v0 = lbu(v1 - 0x1);
            sb(v0, v1);
            v1--;
        } while (v1 != t1);

        if (t0 == 0) goto loc_800312FC;
        if (fieldWidth == 0) goto loc_800312FC;
        if (a3 != 0) goto loc_800312FC;
        *dstStr = paddingChar;
        goto loc_80031348;
    loc_800312FC:
        divu(a3, t3);

        if (t3 == 0) {
            _break(0x1C00);
        }

        v0 = hi;
        *dstStr = v0;
        v1 = (uint8_t) *dstStr;
        v0 = v1 + 0x30;

        if (v1 >= 0xA) {
            v0 = v1 + 0x37;
        }

        *dstStr = v0;
        divu(a3, t3);

        if (t3 == 0) {
            _break(0x1C00);
        }
    
        a3 = lo;
    loc_80031348:
        t0++;

        if (fieldWidth != 0) {
            --fieldWidth;
        }

        if (a3 != 0) goto loc_800312B8;
        if (fieldWidth != 0) goto loc_800312B8;
        if (t0 == 0) goto loc_800312B8;
        dstStr += t0;
    loc_80031370:
        ++fmtStr;
    }

loc_80031384:
    *dstStr = 0;
    v0 = startDstStr - dstStr;

loc_8003138C:
    return;
}

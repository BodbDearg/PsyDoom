#include "d_string.h"

#include "PsxVm/PsxVm.h"

void D_memset() noexcept {
loc_80012850:
    v0 = a0 & 3;
    v1 = a1;
    if (v0 == 0) goto loc_80012880;
    a2--;
loc_80012860:
    if (i32(a2) < 0) goto loc_80012904;
    sb(v1, a0);
    a0++;
    v0 = a0 & 3;
    a2--;
    if (v0 != 0) goto loc_80012860;
    a2++;
loc_80012880:
    v0 = a1 << 24;
    v1 = a1 << 16;
    v0 |= v1;
    v1 = a1 << 8;
    v0 |= v1;
    a1 |= v0;
    v0 = (i32(a2) < 0x20);
    a3 = a0;
    if (v0 != 0) goto loc_800128DC;
    a0 += 4;
loc_800128A8:
    sw(a1, a0 + 0x18);
    sw(a1, a0 + 0x14);
    sw(a1, a0 + 0x10);
    sw(a1, a0 + 0xC);
    sw(a1, a0 + 0x8);
    sw(a1, a0 + 0x4);
    sw(a1, a0);
    a0 += 0x20;
    sw(a1, a3);
    a2 -= 0x20;
    v0 = (i32(a2) < 0x20);
    a3 += 0x20;
    if (v0 == 0) goto loc_800128A8;
loc_800128DC:
    a2--;
    v0 = -1;                                            // Result = FFFFFFFF
    a0 = a3;
    if (a2 == v0) goto loc_80012904;
    v0 = a1;
    v1 = -1;                                            // Result = FFFFFFFF
loc_800128F4:
    sb(v0, a0);
    a2--;
    a0++;
    if (a2 != v1) goto loc_800128F4;
loc_80012904:
    return;
}

void D_memcpy() noexcept {
loc_8001290C:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80012934;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8001291C:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_8001291C;
loc_80012934:
    sp += 8;
    return;
}

void D_strncpy() noexcept {
loc_80012940:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80012970;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80012950:
    v0 = lbu(a1);
    a1++;
    sb(v0, a0);
    a0++;
    if (v0 == 0) goto loc_80012970;
    v1--;
    if (v1 != a2) goto loc_80012950;
loc_80012970:
    sp += 8;
    return;
}

void D_strncasecmp() noexcept {
loc_8001297C:
    v0 = lbu(a0);
    if (v0 == 0) goto loc_800129BC;
    v1 = lbu(a1);
    if (v1 == 0) goto loc_800129C0;
    {
        const bool bJump = (v0 != v1)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800129CC;
    }
    a0++;
    a2--;
    a1++;
    if (a2 != 0) goto loc_8001297C;
    v0 = 0;                                             // Result = 00000000
    goto loc_800129CC;
loc_800129BC:
    v1 = lbu(a1);
loc_800129C0:
    v0 ^= v1;
    v0 = (v0 > 0);
loc_800129CC:
    return;
}

void strupr() noexcept {
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    if (v1 == 0) goto loc_80012A10;
loc_800129E4:
    v0 &= 0xFF;
    v0 = (v0 < 0x1A);
    if (v0 == 0) goto loc_800129F8;
    v1 -= 0x20;
loc_800129F8:
    sb(v1, a0);
    a0++;
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    if (v1 != 0) goto loc_800129E4;
loc_80012A10:
    return;
}

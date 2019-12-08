#include "m_bbox.h"

#include "PsxVm/PsxVm.h"

void M_ClearBox() noexcept {
loc_80012A80:
    v1 = 0x7FFF0000;                                    // Result = 7FFF0000
    v1 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v0 = 0x80000000;                                    // Result = 80000000
    sw(v0, a0 + 0xC);
    sw(v0, a0);
    sw(v1, a0 + 0x8);
    sw(v1, a0 + 0x4);
    return;
}

void M_AddToBox() noexcept {
loc_80012AA0:
    v0 = lw(a0 + 0x8);
    v0 = (i32(a1) < i32(v0));
    if (v0 == 0) goto loc_80012ABC;
    sw(a1, a0 + 0x8);
    goto loc_80012AD4;
loc_80012ABC:
    v0 = lw(a0 + 0xC);
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_80012AD4;
    sw(a1, a0 + 0xC);
loc_80012AD4:
    v0 = lw(a0 + 0x4);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_80012AF0;
    sw(a2, a0 + 0x4);
    goto loc_80012B08;
loc_80012AF0:
    v0 = lw(a0);
    v0 = (i32(v0) < i32(a2));
    if (v0 == 0) goto loc_80012B08;
    sw(a2, a0);
loc_80012B08:
    return;
}

void M_AddPointToBox() noexcept {
    v0 = lw(a0 + 0x8);
    v0 = (i32(a1) < i32(v0));
    if (v0 == 0) goto loc_80012B28;
    sw(a1, a0 + 0x8);
loc_80012B28:
    v0 = lw(a0 + 0xC);
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_80012B40;
    sw(a1, a0 + 0xC);
loc_80012B40:
    v0 = lw(a0 + 0x4);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_80012B58;
    sw(a2, a0 + 0x4);
loc_80012B58:
    v0 = lw(a0);
    v0 = (i32(v0) < i32(a2));
    if (v0 == 0) goto loc_80012B70;
    sw(a2, a0);
loc_80012B70:
    return;
}

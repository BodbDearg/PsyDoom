#include "m_bbox.h"

#include "PsxVm/PsxVm.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear the coordinates for the given bounding box.
// The coords are cleared such that the first point added to it would set all of the box extents.
//------------------------------------------------------------------------------------------------------------------------------------------
void M_ClearBox(fixed_t* const pBox) noexcept {
    pBox[BOXTOP] = INT32_MIN;
    pBox[BOXRIGHT] = INT32_MIN;
    pBox[BOXLEFT] = INT32_MAX;
    pBox[BOXBOTTOM] = INT32_MAX;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Include the given point in the extents of the given bounding box
//------------------------------------------------------------------------------------------------------------------------------------------
void M_AddToBox(fixed_t* const pBox, const fixed_t x, const fixed_t y) noexcept {
    if (x < pBox[BOXLEFT]) {
        pBox[BOXLEFT] = x;
    } else if (x > pBox[BOXRIGHT]) {
        pBox[BOXRIGHT] = x;
    }
    
    if (y < pBox[BOXBOTTOM]) {
        pBox[BOXBOTTOM] = y;
    } else if (y > pBox[BOXTOP]) {
        pBox[BOXTOP] = y;
    }
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

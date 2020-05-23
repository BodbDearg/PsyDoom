#include "m_bbox.h"

#include <algorithm>

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Variant of 'M_AddToBox' that checks the coord against ALL bounds: unused in PSX DOOM
//------------------------------------------------------------------------------------------------------------------------------------------
void M_AddToBox2(fixed_t* const pBox, const fixed_t x, const fixed_t y) noexcept {
    pBox[BOXLEFT] = std::min(pBox[BOXLEFT], x);
    pBox[BOXRIGHT] = std::max(pBox[BOXRIGHT], x);
    pBox[BOXBOTTOM] = std::min(pBox[BOXBOTTOM], y);
    pBox[BOXTOP] = std::max(pBox[BOXTOP], y);
}

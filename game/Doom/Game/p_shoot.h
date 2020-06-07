#pragma once

#include "Doom/doomdef.h"

struct divline_t;
struct line_t;

extern mobj_t*      gpShootMObj;
extern line_t*      gpShootLine;
extern fixed_t      gShootSlope;
extern fixed_t      gShootX;
extern fixed_t      gShootY;
extern fixed_t      gShootZ;

void P_Shoot2() noexcept;
bool PA_DoIntercept(void* const pObj, const bool bIsLine, const fixed_t hitFrac) noexcept;
bool PA_ShootLine(line_t& line, fixed_t hitFrac) noexcept;
bool PA_ShootThing(mobj_t& thing, const fixed_t hitFrac) noexcept;
bool PA_CrossBSPNode(const int32_t nodeNum) noexcept;
int32_t PA_DivlineSide(const fixed_t x, const fixed_t y, const divline_t& line) noexcept;

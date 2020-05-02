#pragma once

#include "Doom/doomdef.h"

struct divline_t;
struct line_t;

extern const VmPtr<VmPtr<mobj_t>>   gpShootMObj;
extern const VmPtr<VmPtr<line_t>>   gpShootLine;
extern const VmPtr<fixed_t>         gShootSlope;
extern const VmPtr<fixed_t>         gShootX;
extern const VmPtr<fixed_t>         gShootY;
extern const VmPtr<fixed_t>         gShootZ;

void P_Shoot2() noexcept;
bool PA_DoIntercept(void* const pObj, const bool bIsLine, const fixed_t hitFrac) noexcept;
bool PA_ShootLine(line_t& line, fixed_t hitFrac) noexcept;
bool PA_ShootThing(mobj_t& thing, const fixed_t hitFrac) noexcept;
bool PA_CrossBSPNode(const int32_t nodeNum) noexcept;
int32_t PA_DivlineSide(const fixed_t x, const fixed_t y, const divline_t& line) noexcept;

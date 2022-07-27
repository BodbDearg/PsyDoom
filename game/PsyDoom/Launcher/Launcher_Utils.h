#pragma once

#if PSYDOOM_LAUNCHER

#include "Macros.h"

#include <cstdint>
#include <string>

class Fl_Check_Button;
class Fl_Input;
class Fl_Widget;

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Stores pixel min/max extents on the x and y axes for a rectangular area (also the width and height)
//------------------------------------------------------------------------------------------------------------------------------------------
struct RectExtents {
    int32_t lx;
    int32_t rx;
    int32_t ty;
    int32_t by;
    int32_t w;
    int32_t h;
};

RectExtents getRectExtents(const Fl_Widget& widget) noexcept;
RectExtents getTextDrawExtents(const Fl_Widget& widget) noexcept;
Fl_Input* makeFl_Input(const int x, const int y, const int w, const int h) noexcept;
Fl_Check_Button* makeFl_Check_Button(const int x, const int y, const int w, const int h, const char* const label) noexcept;
std::string trimText(const char* const text) noexcept;

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER

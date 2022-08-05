#pragma once

#if PSYDOOM_LAUNCHER

#include "Macros.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Check_Button.H>
    #include <FL/Fl_Float_Input.H>
    #include <FL/Fl_Int_Input.H>
END_DISABLE_HEADER_WARNINGS

class Fl_Check_Button;
class Fl_Input;
class Fl_Int_Input;
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
Fl_Int_Input* makeFl_Int_Input(const int x, const int y, const int w, const int h) noexcept;
Fl_Check_Button* makeFl_Check_Button(const int x, const int y, const int w, const int h, const char* const label) noexcept;
std::string trimText(const char* const text) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: binds a boolean config value to the specified check button.
// If the value is changed then the specified dirty flag is also updated.
//------------------------------------------------------------------------------------------------------------------------------------------
template <bool& configValue, bool& configDirtyFlag>
void bindConfigField(Fl_Check_Button& checkBtn) noexcept {
    checkBtn.callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Check_Button* const pCheck = static_cast<Fl_Check_Button*>(pWidget);
            configValue = pCheck->value();
            configDirtyFlag = true;
        }
    );

    checkBtn.value(configValue);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: binds an integer config value to the specified input.
// If the value is changed then the specified dirty flag is also updated.
//------------------------------------------------------------------------------------------------------------------------------------------
template <int32_t& configValue, bool& configDirtyFlag>
void bindConfigField(Fl_Int_Input& intInput) noexcept {
    intInput.callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Int_Input* const pIntInput = static_cast<Fl_Int_Input*>(pWidget);
            configValue = std::atoi(pIntInput->value());
            configDirtyFlag = true;
        }
    );

    char configValueStr[64];
    std::snprintf(configValueStr, sizeof(configValueStr), "%d", configValue);
    intInput.value(configValueStr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: binds a float config value to the specified input.
// If the value is changed then the specified dirty flag is also updated.
//------------------------------------------------------------------------------------------------------------------------------------------
template <float& configValue, bool& configDirtyFlag>
void bindConfigField(Fl_Float_Input& floatInput) noexcept {
    floatInput.callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Float_Input* const pFloatInput = static_cast<Fl_Float_Input*>(pWidget);
            configValue = (float) std::atof(pFloatInput->value());
            configDirtyFlag = true;
        }
    );

    char configValueStr[128];
    std::snprintf(configValueStr, sizeof(configValueStr), "%g", configValue);
    floatInput.value(configValueStr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: binds a string config value to the specified input.
// If the value is changed then the specified dirty flag is also updated.
//------------------------------------------------------------------------------------------------------------------------------------------
template <std::string& configValue, bool& configDirtyFlag>
void bindConfigField(Fl_Input& input) noexcept {
    input.callback(
        [](Fl_Widget* const pWidget, void*) noexcept {
            Fl_Input* const pInput = static_cast<Fl_Input*>(pWidget);
            configValue = pInput->value();
            configDirtyFlag = true;
        }
    );
    
    input.value(configValue.c_str());
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER

//------------------------------------------------------------------------------------------------------------------------------------------
// Various helper functions relating to the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Launcher_Utils.h"

#if PSYDOOM_LAUNCHER

BEGIN_DISABLE_HEADER_WARNINGS
    #include <FL/Fl_Check_Button.H>
    #include <FL/Fl_Input.H>
END_DISABLE_HEADER_WARNINGS

#include <cstring>

BEGIN_NAMESPACE(Launcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the extents for a specified FLTK widget
//------------------------------------------------------------------------------------------------------------------------------------------
RectExtents getRectExtents(const Fl_Widget& widget) noexcept {
    RectExtents extents = {};
    extents.lx = widget.x();
    extents.ty = widget.y();
    extents.w = widget.w();
    extents.h = widget.h();
    extents.rx = extents.lx + extents.w;
    extents.by = extents.ty + extents.h;

    return extents;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the rectangular area occupied by text drawn for the given widgets label.
// This can be used to help layout other widgets.
//------------------------------------------------------------------------------------------------------------------------------------------
RectExtents getTextDrawExtents(const Fl_Widget& widget) noexcept {
    // Get the text size
    int textW = {};
    int textH = {};
    widget.measure_label(textW, textH);

    // Figure out where that text appears on both axes.
    // Start with the x-axis:
    const Fl_Align align = widget.align();
    int x = widget.x();
    int y = widget.y();

    if (align & FL_ALIGN_LEFT) {
        if ((align & FL_ALIGN_INSIDE) == 0) {
            x -= textW;
        }
    }
    else if (align & FL_ALIGN_RIGHT) {
        x += widget.w();

        if (align & FL_ALIGN_INSIDE) {
            x -= textW;
        }
    }
    else {
        // The text is centered on the x-axis
        x += (widget.w() - textW) / 2;
    }

    // Text y-axis position:
    if (align & FL_ALIGN_TOP) {
        if ((align & FL_ALIGN_INSIDE) == 0) {
            y -= textH;
        }
    }
    else if (align & FL_ALIGN_BOTTOM) {
        y += widget.h();

        if (align & FL_ALIGN_INSIDE) {
            y -= textH;
        }
    }
    else {
        // The text is centered on the y-axis
        y += (widget.h() - textH) / 2;
    }

    // Return the computed extents
    RectExtents extents = {};
    extents.lx = x;
    extents.rx = x + textW;
    extents.ty = y;
    extents.by = y + textH;
    extents.w = textW;
    extents.h = textH;

    return extents;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes a generic text input and styles it for the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
Fl_Input* makeFl_Input(const int x, const int y, const int w, const int h) noexcept {
    Fl_Input* const pInput = new Fl_Input(x, y, w, h);
    pInput->color(FL_DARK1, FL_DARK1);
    pInput->selection_color(FL_LIGHT1);
    return pInput;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes a check button and styles it for the launcher
//------------------------------------------------------------------------------------------------------------------------------------------
Fl_Check_Button* makeFl_Check_Button(const int x, const int y, const int w, const int h, const char* const label) noexcept {
    // Note: set the check button to have the 'flat box' style to avoid the text getting bolder when it's ticked
    const auto pCheck = new Fl_Check_Button(x, y, w, h, label);
    pCheck->box(FL_FLAT_BOX);
    return pCheck;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trims the specified text on both ends of the string and returns the result
//------------------------------------------------------------------------------------------------------------------------------------------
std::string trimText(const char* const text) noexcept {
    if (!text)
        return std::string();

    const auto isWhitespace = [](const char c) {
        return ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == '\v') || (c == '\f'));
    };

    const char* pStartChar = text;

    while (isWhitespace(pStartChar[0])) {
        ++pStartChar;
    }

    const char* pEndChar = pStartChar + std::strlen(pStartChar);

    while ((pEndChar > pStartChar) && isWhitespace(pEndChar[-1])) {
        --pEndChar;
    }

    return std::string(pStartChar, pEndChar - pStartChar);
}

END_NAMESPACE(Launcher)

#endif  // #if PSYDOOM_LAUNCHER

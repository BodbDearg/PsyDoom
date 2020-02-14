#pragma once

#include "PsxVm/VmPtr.h"

// Describes 1 frame of a status bar face sprite
struct facesprite_t {
    uint8_t xPos;
    uint8_t yPos;
    uint8_t texU;
    uint8_t texV;
    uint8_t w;
    uint8_t h;
};

// The number of face sprite definitions there are
static constexpr int32_t NUMFACES = 47;

extern const facesprite_t               gFaceSprites[NUMFACES];
extern const VmPtr<VmPtr<const char>>   gpStatusBarMsgStr;
extern const VmPtr<int32_t>             gStatusBarMsgTicsLeft;

void ST_Init() noexcept;
void ST_Start() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

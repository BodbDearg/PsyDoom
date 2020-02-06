#pragma once

#include "PsxVm/VmPtr.h"

struct texture_t;

extern const VmPtr<texture_t>           gTex_STATUS;
extern const VmPtr<VmPtr<const char>>   gpStatusBarMsgStr;
extern const VmPtr<int32_t>             gStatusBarMsgTicsLeft;

void ST_Init() noexcept;
void ST_Start() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

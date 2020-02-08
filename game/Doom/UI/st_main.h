#pragma once

#include "PsxVm/VmPtr.h"

extern const VmPtr<VmPtr<const char>>   gpStatusBarMsgStr;
extern const VmPtr<int32_t>             gStatusBarMsgTicsLeft;

void ST_Init() noexcept;
void ST_Start() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

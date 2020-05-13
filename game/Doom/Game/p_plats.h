#pragma once

#include <cstdint>

struct line_t;

// Moving platform type
enum plattype_e : uint32_t {
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
};

bool EV_DoPlat(line_t& line, const plattype_e platType, const int32_t moveAmount) noexcept;
void P_ActivateInStasis(const int32_t tag) noexcept;
void EV_StopPlat(line_t& line) noexcept;

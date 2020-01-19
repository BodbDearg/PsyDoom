#pragma once

struct subsector_t;

void R_DrawSubsector(subsector_t& subsec) noexcept;
void R_FrontZClip() noexcept;
void R_CheckEdgeVisible() noexcept;
void R_LeftEdgeClip() noexcept;
void R_RightEdgeClip() noexcept;

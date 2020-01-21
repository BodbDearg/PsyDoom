#pragma once

#include <cstdint>

struct leaf_t;
struct subsector_t;

void R_DrawSubsector(subsector_t& subsec) noexcept;
void R_FrontZClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept;
int32_t R_CheckLeafSide(const bool bAgainstRightPlane, const leaf_t& leaf) noexcept;
void R_LeftEdgeClip() noexcept;
void R_RightEdgeClip() noexcept;

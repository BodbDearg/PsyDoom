#pragma once

#include <cstdint>

struct leaf_t;
struct subsector_t;

#if PSYDOOM_LIMIT_REMOVING
    void R_InitDrawBuffers() noexcept;
#endif

void R_DrawSubsector(subsector_t& subsec) noexcept;
void R_FrontZClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept;
int32_t R_CheckLeafSide(const bool bAgainstRightPlane, const leaf_t& leaf) noexcept;
int32_t R_LeftEdgeClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept;
int32_t R_RightEdgeClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept;

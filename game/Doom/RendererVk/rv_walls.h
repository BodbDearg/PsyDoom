#if PSYDOOM_VULKAN_RENDERER

struct seg_t;

#include <cstdint>

void RV_DrawSeg(const seg_t& seg, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER

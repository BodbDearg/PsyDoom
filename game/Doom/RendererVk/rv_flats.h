#if PSYDOOM_VULKAN_RENDERER

struct subsector_t;

#include <cstdint>

void RV_DrawFlat(const subsector_t& subsec, const bool bDrawFloor, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER

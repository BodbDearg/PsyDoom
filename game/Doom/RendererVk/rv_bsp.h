#if PSYDOOM_VULKAN_RENDERER

#include <vector>

struct subsector_t;

extern std::vector<subsector_t*> gRVDrawSubsecs;

void RV_BuildDrawSubsecList() noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER

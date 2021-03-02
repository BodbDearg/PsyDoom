#include "ApiLayers.h"

#include "VkFuncs.h"

#include <algorithm>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Empty intialize the list of validation layers
//------------------------------------------------------------------------------------------------------------------------------------------
ApiLayers::ApiLayers() noexcept : mLayerProps() {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Constructs the list of validation layers available
//------------------------------------------------------------------------------------------------------------------------------------------
void ApiLayers::init(const VkFuncs& vkFuncs) noexcept {
    // Query how many API layers there are and then retrieve them
    uint32_t numLayers = 0;
    vkFuncs.vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
    mLayerProps.resize(numLayers);
    vkFuncs.vkEnumerateInstanceLayerProperties(&numLayers, mLayerProps.data());

    // Sort all of the API layers by name. They should already be in this order, but just in case!
    std::sort(
        mLayerProps.begin(),
        mLayerProps.end(),
        [&](const VkLayerProperties& layer1, const VkLayerProperties& layer2) noexcept {
            return (std::strcmp(layer1.layerName, layer2.layerName) < 0);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the layer list includes the given layer name
//------------------------------------------------------------------------------------------------------------------------------------------
bool ApiLayers::hasLayer(const char* const pName) const noexcept {
    return (getLayerByName(pName) != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the details for a layer by name or null if not found
//------------------------------------------------------------------------------------------------------------------------------------------
const VkLayerProperties* ApiLayers::getLayerByName(const char* const pName) const noexcept {
    if (mLayerProps.empty())
        return nullptr;

    // Do a binary search for the layer by it's name
    int32_t lower = 0;
    int32_t upper = (int32_t) mLayerProps.size() - 1;

    while (lower <= upper) {
        const int32_t mid = (lower + upper) / 2;
        const VkLayerProperties& layer = mLayerProps[mid];
        const int cmp = std::strcmp(pName, layer.layerName);

        if (cmp < 0) {
            upper = mid - 1;
        } else if (cmp > 0) {
            lower = mid + 1;
        } else {
            return &layer;
        }
    }

    return nullptr;     // Layer not found!
}

END_NAMESPACE(vgl)

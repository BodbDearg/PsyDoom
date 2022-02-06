#include "DeviceExtensions.h"

#include "Asserts.h"
#include "VkFuncs.h"

#include <algorithm>
#include <cstring>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Empty intialize the list of device extensions
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceExtensions::DeviceExtensions() noexcept : mExtProps() {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Constructs the list of available device extensions for the given physical device
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceExtensions::init(const VkFuncs& vkFuncs, const VkPhysicalDevice vkPhysicalDevice) noexcept {
    // Sanity check in debug, shouldn't be creating this with an invalid physical device
    ASSERT(vkPhysicalDevice);

    // Query how many extensions there are and then retrieve them
    uint32_t numExtensions = 0;
    vkFuncs.vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &numExtensions, nullptr);
    mExtProps.resize(numExtensions);
    vkFuncs.vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &numExtensions, mExtProps.data());

    // Sort all of the extensions by name. They should already be in this order, but just in case!
    std::sort(
        mExtProps.begin(),
        mExtProps.end(),
        [&](const VkExtensionProperties& ext1, const VkExtensionProperties& ext2) noexcept {
            return (std::strcmp(ext1.extensionName, ext2.extensionName) < 0);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the extension list includes the given extension name
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceExtensions::hasExtension(const char* const pName) const noexcept {
    return (getExtensionByName(pName) != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the properties for the given extension name, returning null if not found
//------------------------------------------------------------------------------------------------------------------------------------------
const VkExtensionProperties* DeviceExtensions::getExtensionByName(const char* const pName) const noexcept {
    if (mExtProps.empty())
        return nullptr;

    // Do a binary search for the extension by it's name
    int32_t lower = 0;
    int32_t upper = (int32_t) mExtProps.size() - 1;

    while (lower <= upper) {
        const int32_t mid = (lower + upper) / 2;
        const VkExtensionProperties& ext = mExtProps[mid];
        const int cmp = std::strcmp(pName, ext.extensionName);

        if (cmp < 0) {
            upper = mid - 1;
        } else if (cmp > 0) {
            lower = mid + 1;
        } else {
            return &ext;
        }
    }

    return nullptr;     // Extension not found!
}

END_NAMESPACE(vgl)

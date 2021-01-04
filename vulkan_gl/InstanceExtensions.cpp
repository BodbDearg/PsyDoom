#include "InstanceExtensions.h"

#include "VkFuncs.h"

#include <algorithm>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Empty intialize the list of instance extensions
//------------------------------------------------------------------------------------------------------------------------------------------
InstanceExtensions::InstanceExtensions() noexcept : mExtProps() {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Constructs the list of instance extensions available for the given Vulkan instance
//------------------------------------------------------------------------------------------------------------------------------------------
void InstanceExtensions::init(const VkFuncs& vkFuncs) noexcept {
    // Query how many extensions there are and then retrieve them
    uint32_t numExtensions = 0;
    vkFuncs.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);
    mExtProps.resize(numExtensions);
    vkFuncs.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, mExtProps.data());

    // Sort all of the extensions by name. They should already be in this order, but just in case!
    std::sort(
        mExtProps.begin(),
        mExtProps.end(),
        [&](const VkExtensionProperties& ext1, const VkExtensionProperties& ext2) noexcept{
            return (std::strcmp(ext1.extensionName, ext2.extensionName) < 0);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the extension list includes the given extension name
//------------------------------------------------------------------------------------------------------------------------------------------
bool InstanceExtensions::hasExtension(const char* const pName) const noexcept {
    return (getExtensionByName(pName) != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the details for an extension by name or null if not found
//------------------------------------------------------------------------------------------------------------------------------------------
const VkExtensionProperties* InstanceExtensions::getExtensionByName(const char* pName) const noexcept {
    if (mExtProps.empty())
        return nullptr;

    // Do a binary search for the extension by it's name
    size_t lower = 0;
    size_t upper = mExtProps.size() - 1;

    while (lower <= upper) {
        const size_t mid = (lower + upper) / 2;
        const VkExtensionProperties & ext = mExtProps[mid];
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

#include "DeviceSurfaceCaps.h"

#include "Asserts.h"
#include "Finally.h"
#include "PhysicalDevice.h"
#include "VkFuncs.h"
#include "WindowSurface.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Default initialize the surface caps to invalid/empty surface caps
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceSurfaceCaps::DeviceSurfaceCaps() noexcept
    : mbIsValid(false)
    , mPresentCapableQueueFamilies()
    , mVkSurfaceCapabilities()
    , mVkSurfaceFormats()
    , mVkPresentModes()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make this module only responsible for compiling the destructor logic...
//------------------------------------------------------------------------------------------------------------------------------------------
DeviceSurfaceCaps::~DeviceSurfaceCaps() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Query the capabilities of the device for the given surface and return 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceSurfaceCaps::query(const PhysicalDevice& device, const WindowSurface& windowSurface) noexcept {
    // Preconditions
    ASSERT(device.getVkPhysicalDevice());
    ASSERT(windowSurface.getVkSurface());

    // If there were caps previously queried then invalidate/clear first
    if (mbIsValid) {
        clear();
    }

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&] {
        if (!mbIsValid) {
            clear();
        }
    });

    // Firstly figure out which queue families support presenting for this device
    const VkFuncs& vkFuncs = device.getVkFuncs();
    const VkPhysicalDevice vkPhysicalDevice = device.getVkPhysicalDevice();
    const VkSurfaceKHR vkSurface = windowSurface.getVkSurface();

    {
        const uint32_t numQueueFamilies = static_cast<uint32_t>(device.getQueueFamilyProps().size());
        mPresentCapableQueueFamilies.reserve(numQueueFamilies);

        for (uint32_t queueFamilyIdx = 0; queueFamilyIdx < numQueueFamilies; ++queueFamilyIdx) {
            VkBool32 supportsPresent = VK_FALSE;

            const VkResult getSurfaceSupportResult = vkFuncs.vkGetPhysicalDeviceSurfaceSupportKHR(
                vkPhysicalDevice,
                queueFamilyIdx,
                vkSurface,
                &supportsPresent
            );

            if (getSurfaceSupportResult != VK_SUCCESS) {
                ASSERT_FAIL("Failed to get physical device surface support!");
                return false;
            }

            if (supportsPresent) {
                mPresentCapableQueueFamilies.push_back(queueFamilyIdx);
            }
        }
    }

    // Grab the surface capabilities
    const VkResult getSurfaceCapsResult = vkFuncs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vkPhysicalDevice,
        vkSurface,
        &mVkSurfaceCapabilities
    );

    if (getSurfaceCapsResult != VK_SUCCESS) {
        ASSERT_FAIL("Failed to get physical device surface capabilities!");
        return false;
    }

    // Grab the available device surface formats
    {
        uint32_t numSurfaceFormats = 0;
        const VkResult getNumSurfaceFormatsResult = vkFuncs.vkGetPhysicalDeviceSurfaceFormatsKHR(
            vkPhysicalDevice,
            vkSurface,
            &numSurfaceFormats,
            nullptr
        );

        if (getNumSurfaceFormatsResult != VK_SUCCESS) {
            ASSERT_FAIL("Failed to get physical device surface formats!");
            return false;
        }

        mVkSurfaceFormats.resize(numSurfaceFormats);
        const VkResult getSurfaceFormatsResult = vkFuncs.vkGetPhysicalDeviceSurfaceFormatsKHR(
            vkPhysicalDevice,
            vkSurface,
            &numSurfaceFormats,
            mVkSurfaceFormats.data()
        );

        if (getSurfaceFormatsResult != VK_SUCCESS) {
            ASSERT_FAIL("Failed to get physical device surface formats!");
            return false;
        }
    }

    // Get the available present modes
    {
        uint32_t numPresentModes = 0;
        const VkResult getNumPresentModesResult = vkFuncs.vkGetPhysicalDeviceSurfacePresentModesKHR(
            vkPhysicalDevice,
            vkSurface,
            &numPresentModes,
            nullptr
        );

        if (getNumPresentModesResult != VK_SUCCESS) {
            ASSERT_FAIL("Failed to get physical device surface present modes!");
            return false;
        }

        mVkPresentModes.resize(numPresentModes);
        const VkResult getPresentModesResult = vkFuncs.vkGetPhysicalDeviceSurfacePresentModesKHR(
            vkPhysicalDevice,
            vkSurface,
            &numPresentModes,
            mVkPresentModes.data()
        );

        if (getPresentModesResult != VK_SUCCESS) {
            ASSERT_FAIL("Failed to get physical device surface present modes!");
            return false;
        }
    }

    // Querying was successful if we got to here...
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reset the surface caps to the default uninitialized state
//------------------------------------------------------------------------------------------------------------------------------------------
void DeviceSurfaceCaps::clear() noexcept {
    mbIsValid = false;
    mVkPresentModes.clear();
    mVkSurfaceFormats.clear();
    mVkSurfaceCapabilities = {};
    mPresentCapableQueueFamilies.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pick a supported color surface format from the given list of formats.
//
// Notes:
//  (1) Items at the front of the list are chosen first over other items.
//  (2) The picking function chooses formats in SRGB colorspace only. 10-bit HDR for example is NOT supported.
//  (3) Returns an invalid format on failure to find a match.
//------------------------------------------------------------------------------------------------------------------------------------------
VkFormat DeviceSurfaceCaps::getSupportedSurfaceFormat(const VkFormat* const pFormats, const uint32_t numFormats) const noexcept {
    if (numFormats <= 0)
        return VK_FORMAT_UNDEFINED;

    // If we find that any format is supported then just return the first item in the list.
    // According to the Vulkan docs this will be represented by an entry with 'VK_FORMAT_UNDEFINED'.
    // Doubtful this would be encountered in reality however...
    ASSERT(pFormats);

    for (const VkSurfaceFormatKHR format : mVkSurfaceFormats) {
        if (format.format == VK_FORMAT_UNDEFINED) {
            // Can just choose what we want!
            // Prefer linear for the format when we are working with the colors in shaders etc. and non-linear for storage.
            // sRGB results in more accurate perceived colors than normalized values...
            return pFormats[0];
        }
    }

    // Ok, try to find if one of the given formats is supported and return the first supported one found
    for (uint32_t fmtIdx = 0; fmtIdx < numFormats; ++fmtIdx) {
        const VkFormat format = pFormats[fmtIdx];

        for (const VkSurfaceFormatKHR& surfaceFormat : mVkSurfaceFormats) {
            // Note: hardcoding to just picking formats for the SRGB colorspace for now
            if ((surfaceFormat.format == format) && (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
                return format;
        }
    }

    // Can't find a supported format!
    return VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells the max image extent supported by the device surface is zero sized.
// This can happen when the window has been sized to zero or when minimized in some cases.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DeviceSurfaceCaps::isZeroSizedMaxImageExtent() const noexcept {
    return ((mVkSurfaceCapabilities.maxImageExtent.width <= 0) || (mVkSurfaceCapabilities.maxImageExtent.height <= 0));
}

END_NAMESPACE(vgl)

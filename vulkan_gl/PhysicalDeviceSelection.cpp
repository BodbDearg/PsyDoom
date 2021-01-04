//------------------------------------------------------------------------------------------------------------------------------------------
// A helper module to select which physical device would be best to use for the application.
// Does not have to be used neccesarily, could potentially provide user selection of GPU devices instead.
// 
// Most people probably only have 1 discrete GPU in their system however so this code is probably good enough for most practical purposes.
// Also if a user has 2 or more GPUs then they are probably both the same type in SLI or Crossfire mode.
// In that case it would not matter which GPU we pick...
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PhysicalDeviceSelection.h"

#include "Asserts.h"
#include "Defines.h"
#include "DeviceSurfaceCaps.h"
#include "PhysicalDevice.h"
#include "Utils.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(PhysicalDeviceSelection)

//------------------------------------------------------------------------------------------------------------------------------------------
// Compare two physical devices against each other for suitability.
// Returns -1 if this device1 is considered 'better' to use, or '1' if it is considered 'worse'.
// Returns '0' if both devices are considered the same in terms of suitability to use.
//------------------------------------------------------------------------------------------------------------------------------------------
static int compareDeviceSuitability(const PhysicalDevice& device1, const PhysicalDevice& device2) noexcept {
    // Useful macro to compare the two devices based on an expression that results in a value.
    // If the values generated are unequal then the code consults the 'IsBetterValue' macro to determine
    // if this device or the other device had the 'better' value (and therefore should be selected first).
    #define COMPARE_DEVICES_ON_VALUE(ExtractValueExpr, IsBetterExpr)\
        do {\
            auto extractValue([](const PhysicalDevice& device){\
                return (ExtractValueExpr);\
            });\
            \
            const auto value1 = extractValue(device1);\
            const auto value2 = extractValue(device2);\
            \
            if (value1 != value2) {\
                return (IsBetterExpr) ? -1 : +1;\
            }\
        } while (0)

    // Prefer discrete GPU devices
    COMPARE_DEVICES_ON_VALUE(
        ((device.getProps().deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) != 0),
        (value1 == true)
    );

    // Prefer non-software GPU devices
    COMPARE_DEVICES_ON_VALUE(
        ((device.getProps().deviceType & VK_PHYSICAL_DEVICE_TYPE_CPU) != 0),
        (value1 == true)
    );

    // Prefer devices with more on-device memory available
    COMPARE_DEVICES_ON_VALUE(
        device.getDeviceMem(),
        (value1 > value2)
    );

    // Prefer devices with more system memory available
    COMPARE_DEVICES_ON_VALUE(
        device.getNonDeviceMem(),
        (value1 > value2)
    );

    #undef COMPARE_DEVICES_ON_VALUE

    // Don't know which device is better
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Query the surface capabilities of Vulkan physical devices and save them in the output vector.
// 1 entry per physical device is added to the output vector.
//------------------------------------------------------------------------------------------------------------------------------------------
static void querySurfaceCapsForAllDevices(
    const std::vector<PhysicalDevice>& physicalDevices,
    const WindowSurface& windowSurface,
    std::vector<DeviceSurfaceCaps>& deviceSurfaceCapsOut
) noexcept {
    deviceSurfaceCapsOut.resize(physicalDevices.size());
    DeviceSurfaceCaps* pDeviceSurfaceCaps = deviceSurfaceCapsOut.data();

    for (const PhysicalDevice& physicalDevice : physicalDevices) {
        pDeviceSurfaceCaps->query(physicalDevice, windowSurface);
        ++pDeviceSurfaceCaps;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given physical device is suitable for us to use with any renderer, both headless and rendering to a window
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isPhysicalDeviceSuitable(const PhysicalDevice& device) noexcept {
    // Must have at least 1 queue family that supports both graphics and compute.
    // Note that if a queue family supports graphics and compute, it also supports transfer operations by extension implicitly...
    bool bFoundCombinedGraphicsComputeQueue = false;

    {
        const std::vector<uint32_t>& computeQueueFamilies = device.getComputeQueueFamilyIndexes();

        for (uint32_t graphicsQueueIdx : device.getGraphicsQueueFamilyIndexes()) {
            if (Utils::containerContains(computeQueueFamilies, graphicsQueueIdx)) {
                bFoundCombinedGraphicsComputeQueue = true;
                break;
            }
        }
    }

    if (!bFoundCombinedGraphicsComputeQueue)
        return false;

    // Check that the device supports the min required texture size limits
    const VkPhysicalDeviceProperties& deviceProps = device.getProps();
    const VkPhysicalDeviceLimits& deviceLimits = deviceProps.limits;

    if (deviceLimits.maxImageDimension1D < Defines::MIN_REQUIRED_2D_IMAGE_SIZE_LIMIT)
        return false;

    if (deviceLimits.maxImageDimension2D < Defines::MIN_REQUIRED_2D_IMAGE_SIZE_LIMIT)
        return false;

    // All good, device is valid
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given physical device is suitable for use with a non headless renderer.
// Uses the surface capabilities to determine.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isPhysicalDeviceWithSurfaceCapsSuitable(
    const PhysicalDevice& device,
    const DeviceSurfaceCaps& deviceSurfaceCaps
) noexcept {
    // The physical device must pass basic suitability tests before we examine it's surface caps:
    if (!isPhysicalDeviceSuitable(device))
        return false;

    // If the device surface caps are not valid then the device is not valid also
    if (!deviceSurfaceCaps.isValid())
        return false;

    // Must support presenting to our surface with at least one queue family
    if (deviceSurfaceCaps.getPresentCapableQueueFamilies().empty())
        return false;

    // Must support at least 1 valid surface format and present mode
    if (deviceSurfaceCaps.getVkSurfaceFormats().empty())
        return false;

    if (deviceSurfaceCaps.getVkPresentModes().empty())
        return false;

    // Device must support the swapchain extension so we can use it for presentation
    if (!device.getExtensions().hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        return false;

    // All good, device is valid
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Select the best physical device to use from a list of devices for the given window surface.
// Returns null if no device in the given list was suitable.
//------------------------------------------------------------------------------------------------------------------------------------------
const PhysicalDevice* selectBestDevice(
    const std::vector<PhysicalDevice>& devices,
    const WindowSurface& windowSurface
) noexcept {
    // Get the surface capabilities for all the devices
    std::vector<DeviceSurfaceCaps> deviceSurfaceCaps;
    querySurfaceCapsForAllDevices(devices, windowSurface, deviceSurfaceCaps);
    ASSERT(deviceSurfaceCaps.size() == devices.size());

    // Now find the most suitable device
    const DeviceSurfaceCaps* pCurDeviceSurfaceCaps = deviceSurfaceCaps.data();
    const PhysicalDevice* pBestDevice = nullptr;

    for (const PhysicalDevice& device : devices) {
        if (isPhysicalDeviceWithSurfaceCapsSuitable(device, *pCurDeviceSurfaceCaps)) {
            if ((pBestDevice == nullptr) || (compareDeviceSuitability(device, *pBestDevice) < 0)) {
                pBestDevice = &device;
            }
        }

        ++pCurDeviceSurfaceCaps;
    }

    return pBestDevice;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Same as above but for headless mode.
// Selection criteria in this case does not need to worry about capabilities with respect to a window surface.
//------------------------------------------------------------------------------------------------------------------------------------------
const PhysicalDevice* selectBestHeadlessDevice(const std::vector<PhysicalDevice>& devices) noexcept {
    const PhysicalDevice* pBestDevice = nullptr;

    for (const PhysicalDevice& device : devices) {
        if (isPhysicalDeviceSuitable(device)) {
            if ((pBestDevice == nullptr) || (compareDeviceSuitability(device, *pBestDevice) < 0)) {
                pBestDevice = &device;
            }
        }
    }

    return pBestDevice;
}

END_NAMESPACE(PhysicalDeviceSelection)
END_NAMESPACE(vgl)

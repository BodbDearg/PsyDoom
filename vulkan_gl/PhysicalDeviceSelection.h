#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(vgl)

class DeviceSurfaceCaps;
class PhysicalDevice;
class WindowSurface;

BEGIN_NAMESPACE(PhysicalDeviceSelection)

// A filter function for deciding whether a physical device is acceptable or not; returns 'true' if the device is acceptable.
// Takes the device and the capabilities of the device with respect to the window surface as input.
typedef bool (*DeviceFilter)(const PhysicalDevice& device, const DeviceSurfaceCaps& surfaceCaps) noexcept;

// Same as above but minus the device surface capabilities; for headless device selection
typedef bool (*HeadlessDeviceFilter)(const PhysicalDevice& device) noexcept;

const PhysicalDevice* selectBestDevice(
    const std::vector<PhysicalDevice>& devices,
    const WindowSurface& windowSurface,
    const DeviceFilter deviceFilter = nullptr
) noexcept;

const PhysicalDevice* selectBestHeadlessDevice(
    const std::vector<PhysicalDevice>& devices,
    const HeadlessDeviceFilter deviceFilter = nullptr
) noexcept;

END_NAMESPACE(PhysicalDeviceSelection)
END_NAMESPACE(vgl)

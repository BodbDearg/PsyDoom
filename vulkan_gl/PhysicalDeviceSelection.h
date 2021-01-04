#pragma once

#include "Macros.h"

#include <vector>

BEGIN_NAMESPACE(vgl)

class PhysicalDevice;
class WindowSurface;

BEGIN_NAMESPACE(PhysicalDeviceSelection)

const PhysicalDevice* selectBestDevice(const std::vector<PhysicalDevice>& devices, const WindowSurface& windowSurface) noexcept;
const PhysicalDevice* selectBestHeadlessDevice(const std::vector<PhysicalDevice>& devices) noexcept;

END_NAMESPACE(PhysicalDeviceSelection)
END_NAMESPACE(vgl)

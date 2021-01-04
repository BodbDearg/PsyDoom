#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a means of synchronizing between GL command submissions and the application.
// Fences can be signalled when a command submission has finished executing.
//------------------------------------------------------------------------------------------------------------------------------------------
class Fence {
public:
    // What state the fence is initialized in, either signalled or unsignalled
    enum class InitMode {
        UNSIGNALLED,
        SIGNALLED
    };

    Fence() noexcept;
    Fence(Fence&& other) noexcept;
    ~Fence() noexcept;

    bool init(LogicalDevice& device, const InitMode initMode) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    bool isSignalled() const noexcept;
    void resetSignal() noexcept;
    void waitUntilSignalled() noexcept;
    bool waitUntilSignalledWithTimeout(const float seconds) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkFence getVkFence() const noexcept { return mVkFence; }
    
private:
    // Copy and move assign disallowed
    Fence(const Fence& other) = delete;
    Fence& operator = (const Fence& other) = delete;
    Fence& operator = (Fence&& other) = delete;

    bool            mbIsValid;
    LogicalDevice*  mpDevice;
    VkFence         mVkFence;
};

END_NAMESPACE(vgl)

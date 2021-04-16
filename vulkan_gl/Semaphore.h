#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a vulkan semaphore used for synchronization between device calls.
// The user cannot directly signal or block on a semaphore, instead it is done through the various API calls.
// It used for GPU -> GPU synchronization, as opposed to a Fence which is for GPU -> CPU synchronization.
//------------------------------------------------------------------------------------------------------------------------------------------
class Semaphore {
public:
    Semaphore() noexcept;
    Semaphore(Semaphore&& other) noexcept;
    ~Semaphore() noexcept;

    bool init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkSemaphore getVkSemaphore() const noexcept { return mVkSemaphore; }

private:
    // Copy and move assign disallowed
    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator = (const Semaphore& other) = delete;
    Semaphore& operator = (Semaphore&& other) = delete;

    bool            mbIsValid;
    LogicalDevice*  mpDevice;
    VkSemaphore     mVkSemaphore;
};

END_NAMESPACE(vgl)

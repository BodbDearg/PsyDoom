#pragma once

#include "DeviceSurfaceCaps.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
class Semaphore;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan swapchain.
// A swapchain describes the surface formats rendered to for the output device, how swaps are performed and so forth...
//------------------------------------------------------------------------------------------------------------------------------------------
class Swapchain {
public:
    // Constant for an invalid/non-existant swapchain image index
    static constexpr const uint32_t INVALID_IMAGE_IDX = UINT32_MAX;

    Swapchain() noexcept;
    ~Swapchain() noexcept;

    bool init(
        LogicalDevice& device,
        const VkFormat winSurfaceFormat,
        const VkColorSpaceKHR winSurfaceColorspace,
        const bool bTripleBuffer
    ) noexcept;

    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline bool needsRecreate() const noexcept { return mbNeedsRecreate; }
    inline uint32_t getAcquiredImageIdx() const noexcept { return mAcquiredImageIdx; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline const VkSurfaceFormatKHR& getSurfaceFormat() const noexcept { return mSurfaceFormat; }
    inline const VkPresentModeKHR& getPresentMode() const noexcept { return mPresentMode; }
    inline uint32_t getSwapExtentWidth() const noexcept { return mSwapExtentW; }
    inline uint32_t getSwapExtentHeight() const noexcept { return mSwapExtentH; }
    inline uint32_t getLength() const noexcept { return mLength; }
    inline VkSwapchainKHR getVkSwapchain() const noexcept { return mVkSwapchain; }
    inline const std::vector<VkImage>& getVkImages() const noexcept { return mVkImages; }
    inline const std::vector<VkImageView>& getVkImageViews() const noexcept { return mVkImageViews; }

    void setNeedsRecreate() noexcept;
    bool presentAcquiredImage(const Semaphore& renderFinishedSemaphore) noexcept;
    uint32_t acquireImage(Semaphore& imageReadySemaphore) noexcept;

private:
    // Copy and move are disallowed
    Swapchain(const Swapchain& other) = delete;
    Swapchain(Swapchain&& other) = delete;
    Swapchain& operator = (const Swapchain& other) = delete;
    Swapchain& operator = (Swapchain&& other) = delete;
    
    void choosePresentMode(const bool bTripleBuffer) noexcept;
    bool chooseSwapExtent() noexcept;
    void chooseSwapchainLength(const bool bTripleBuffer) noexcept;
    bool createSwapchain() noexcept;
    bool createSwapchainImageViews() noexcept;

    bool                        mbIsValid;              // True if this object was created & initialized successfully
    bool                        mbNeedsRecreate;        // True if the swapchain needs recreation, due to window resizing for instance
    uint32_t                    mAcquiredImageIdx;      // Currently acquired image index, or INVALID_IMAGE_IDX if not acquired.
    LogicalDevice*              mpDevice;               // The logical Vulkan device this swap chain is for
    DeviceSurfaceCaps           mDeviceSurfaceCaps;     // Capabilities of the device with respect to the window surface
    VkSurfaceFormatKHR          mSurfaceFormat;         // Chosen surface format for the swap chain
    VkPresentModeKHR            mPresentMode;           // Chosen present mode for the swap chain
    uint32_t                    mSwapExtentW;           // Size of the swap area: width
    uint32_t                    mSwapExtentH;           // Size of the swap area: height
    uint32_t                    mLength;                // How many images are in the swap chain queue, 3 = tripple buffering, 2 = double buffering, 1 = immediate
    VkSwapchainKHR              mVkSwapchain;           // Handle to the actual Vulkan swap chain
    std::vector<VkImage>        mVkImages;              // Handles to the Vulkan images in the swap chain
    std::vector<VkImageView>    mVkImageViews;          // Image views for images in the swap chain
};

END_NAMESPACE(vgl)

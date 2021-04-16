#include "VkFuncs.h"

#include "Asserts.h"
#include "FatalErrors.h"

#include <SDL_vulkan.h>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Load a single global function not associated with any Vulkan API instance
//------------------------------------------------------------------------------------------------------------------------------------------
template <class DstFuncT>
static void loadGlobalFunc(
    DstFuncT& dstFunc,
    const char* const funcName,
    const PFN_vkGetInstanceProcAddr getInstanceProcAddr,
    bool bOptional
) noexcept {
    dstFunc = (DstFuncT) getInstanceProcAddr(nullptr, funcName);

    if ((!dstFunc) && (!bOptional)) {
        FatalErrors::raiseF("Failed to retrieve the required Vulkan function '%s'!\nRequired Vulkan functionality missing!", funcName);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load a single Vulkan instance level function
//------------------------------------------------------------------------------------------------------------------------------------------
template <class DstFuncT>
static void loadInstanceFunc(
    DstFuncT& dstFunc,
    const char* const funcName,
    const VkInstance vkInstance,
    const PFN_vkGetInstanceProcAddr getInstanceProcAddr,
    bool bOptional
) noexcept {
    dstFunc = (DstFuncT) getInstanceProcAddr(vkInstance, funcName);

    if ((!dstFunc) && (!bOptional)) {
        FatalErrors::raiseF("Failed to retrieve the required Vulkan function '%s'!\nRequired Vulkan functionality missing!", funcName);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load a single Vulkan device level function
//------------------------------------------------------------------------------------------------------------------------------------------
template <class DstFuncT>
static void loadDeviceFunc(
    DstFuncT& dstFunc,
    const char* const funcName,
    const VkDevice vkDevice,
    const PFN_vkGetDeviceProcAddr getDeviceProcAddr,
    bool bOptional
) noexcept {
    dstFunc = (DstFuncT) getDeviceProcAddr(vkDevice, funcName);

    if ((!dstFunc) && (!bOptional)) {
        FatalErrors::raiseF("Failed to retrieve the required Vulkan function '%s'!\nRequired Vulkan functionality missing!", funcName);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load functions not associated with any Vulkan API instance
//------------------------------------------------------------------------------------------------------------------------------------------
void VkFuncs::loadGlobalFuncs() noexcept {
    // This one is special, we get it from SDL as it handles all of the dynamic library loading for us...
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) SDL_Vulkan_GetVkGetInstanceProcAddr();

    if (!vkGetInstanceProcAddr) {
        FatalErrors::raise("Failed to retrieve the required Vulkan function 'vkGetInstanceProcAddr'!\nRequired Vulkan functionality missing!");
    }

    // Load all of the functions
    #define LOAD_GLOBAL_FUNC(NAME)\
        do {\
            loadGlobalFunc(NAME, #NAME, vkGetInstanceProcAddr, false);\
        } while (0)

    LOAD_GLOBAL_FUNC(vkCreateInstance);
    LOAD_GLOBAL_FUNC(vkEnumerateInstanceExtensionProperties);
    LOAD_GLOBAL_FUNC(vkEnumerateInstanceLayerProperties);

    #undef LOAD_GLOBAL_FUNC
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load instance level functions for the given Vulkan API instance.
// Note: assumes global functions have already been loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
void VkFuncs::loadInstanceFuncs(const VkInstance vkInstance) noexcept {
    // Must have loaded this function by loading all global functions first!
    ASSERT(vkGetInstanceProcAddr);

    // Load all of the functions
    #define LOAD_INST_FUNC(NAME)\
        do {\
            loadInstanceFunc(NAME, #NAME, vkInstance, vkGetInstanceProcAddr, false);\
        } while (0)

    #define LOAD_OPT_INST_FUNC(NAME)\
        do {\
            loadInstanceFunc(NAME, #NAME, vkInstance, vkGetInstanceProcAddr, true);\
        } while (0)

    LOAD_INST_FUNC(vkBeginCommandBuffer);
    LOAD_INST_FUNC(vkCmdBeginRenderPass);
    LOAD_INST_FUNC(vkCmdBindDescriptorSets);
    LOAD_INST_FUNC(vkCmdBindIndexBuffer);
    LOAD_INST_FUNC(vkCmdBindPipeline);
    LOAD_INST_FUNC(vkCmdBindVertexBuffers);
    LOAD_INST_FUNC(vkCmdBlitImage);
    LOAD_INST_FUNC(vkCmdClearColorImage);
    LOAD_INST_FUNC(vkCmdCopyBuffer);
    LOAD_INST_FUNC(vkCmdCopyBufferToImage);
    LOAD_INST_FUNC(vkCmdCopyImage);
    LOAD_INST_FUNC(vkCmdDispatch);
    LOAD_INST_FUNC(vkCmdDraw);
    LOAD_INST_FUNC(vkCmdDrawIndexed);
    LOAD_INST_FUNC(vkCmdEndRenderPass);
    LOAD_INST_FUNC(vkCmdExecuteCommands);
    LOAD_INST_FUNC(vkCmdNextSubpass);
    LOAD_INST_FUNC(vkCmdPipelineBarrier);
    LOAD_INST_FUNC(vkCmdPushConstants);
    LOAD_INST_FUNC(vkCmdSetScissor);
    LOAD_INST_FUNC(vkCmdSetViewport);
    LOAD_INST_FUNC(vkCreateDevice);
    LOAD_INST_FUNC(vkDestroyDevice);
    LOAD_INST_FUNC(vkDestroyInstance);
    LOAD_INST_FUNC(vkDestroySurfaceKHR);
    LOAD_INST_FUNC(vkEndCommandBuffer);
    LOAD_INST_FUNC(vkEnumerateDeviceExtensionProperties);
    LOAD_INST_FUNC(vkEnumeratePhysicalDevices);
    LOAD_INST_FUNC(vkGetDeviceProcAddr);
    LOAD_INST_FUNC(vkGetPhysicalDeviceFeatures);
    LOAD_INST_FUNC(vkGetPhysicalDeviceFormatProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR);

    LOAD_OPT_INST_FUNC(vkCreateDebugReportCallbackEXT);
    LOAD_OPT_INST_FUNC(vkCreateDebugUtilsMessengerEXT);
    LOAD_OPT_INST_FUNC(vkDestroyDebugReportCallbackEXT);
    LOAD_OPT_INST_FUNC(vkDestroyDebugUtilsMessengerEXT);

    #undef LOAD_INST_FUNC
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load device level functions for the given Vulkan logical device.
// Note: assumes global and instance level functions have already been loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
void VkFuncs::loadDeviceFuncs(const VkDevice vkDevice) noexcept {
    // Must have loaded this function by loading all instance functions first!
    ASSERT(vkGetDeviceProcAddr);

    // Load all of the functions
    #define LOAD_DEV_FUNC(NAME)\
        do {\
            loadDeviceFunc(NAME, #NAME, vkDevice, vkGetDeviceProcAddr, false);\
        } while (0)

    LOAD_DEV_FUNC(vkAcquireNextImageKHR);
    LOAD_DEV_FUNC(vkAllocateCommandBuffers);
    LOAD_DEV_FUNC(vkAllocateDescriptorSets);
    LOAD_DEV_FUNC(vkAllocateMemory);
    LOAD_DEV_FUNC(vkBindBufferMemory);
    LOAD_DEV_FUNC(vkBindImageMemory);
    LOAD_DEV_FUNC(vkCreateBuffer);
    LOAD_DEV_FUNC(vkCreateCommandPool);
    LOAD_DEV_FUNC(vkCreateComputePipelines);
    LOAD_DEV_FUNC(vkCreateDescriptorPool);
    LOAD_DEV_FUNC(vkCreateDescriptorSetLayout);
    LOAD_DEV_FUNC(vkCreateFence);
    LOAD_DEV_FUNC(vkCreateFramebuffer);
    LOAD_DEV_FUNC(vkCreateGraphicsPipelines);
    LOAD_DEV_FUNC(vkCreateImage);
    LOAD_DEV_FUNC(vkCreateImageView);
    LOAD_DEV_FUNC(vkCreatePipelineLayout);
    LOAD_DEV_FUNC(vkCreateRenderPass);
    LOAD_DEV_FUNC(vkCreateSampler);
    LOAD_DEV_FUNC(vkCreateSemaphore);
    LOAD_DEV_FUNC(vkCreateShaderModule);
    LOAD_DEV_FUNC(vkCreateSwapchainKHR);
    LOAD_DEV_FUNC(vkDestroyBuffer);
    LOAD_DEV_FUNC(vkDestroyCommandPool);
    LOAD_DEV_FUNC(vkDestroyDescriptorPool);
    LOAD_DEV_FUNC(vkDestroyDescriptorSetLayout);
    LOAD_DEV_FUNC(vkDestroyFence);
    LOAD_DEV_FUNC(vkDestroyFramebuffer);
    LOAD_DEV_FUNC(vkDestroyImage);
    LOAD_DEV_FUNC(vkDestroyImageView);
    LOAD_DEV_FUNC(vkDestroyPipeline);
    LOAD_DEV_FUNC(vkDestroyPipelineLayout);
    LOAD_DEV_FUNC(vkDestroyRenderPass);
    LOAD_DEV_FUNC(vkDestroySampler);
    LOAD_DEV_FUNC(vkDestroySemaphore);
    LOAD_DEV_FUNC(vkDestroyShaderModule);
    LOAD_DEV_FUNC(vkDestroySwapchainKHR);
    LOAD_DEV_FUNC(vkDeviceWaitIdle);
    LOAD_DEV_FUNC(vkFreeCommandBuffers);
    LOAD_DEV_FUNC(vkFreeDescriptorSets);
    LOAD_DEV_FUNC(vkFreeMemory);
    LOAD_DEV_FUNC(vkGetBufferMemoryRequirements);
    LOAD_DEV_FUNC(vkGetDeviceQueue);
    LOAD_DEV_FUNC(vkGetFenceStatus);
    LOAD_DEV_FUNC(vkGetImageMemoryRequirements);
    LOAD_DEV_FUNC(vkGetSwapchainImagesKHR);
    LOAD_DEV_FUNC(vkMapMemory);
    LOAD_DEV_FUNC(vkQueuePresentKHR);
    LOAD_DEV_FUNC(vkQueueSubmit);
    LOAD_DEV_FUNC(vkQueueWaitIdle);
    LOAD_DEV_FUNC(vkResetCommandPool);
    LOAD_DEV_FUNC(vkResetFences);
    LOAD_DEV_FUNC(vkUnmapMemory);
    LOAD_DEV_FUNC(vkUpdateDescriptorSets);
    LOAD_DEV_FUNC(vkWaitForFences);

    #undef LOAD_DEV_FUNC
}

END_NAMESPACE(vgl)

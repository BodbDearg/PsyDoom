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

    LOAD_INST_FUNC(vkAcquireNextImageKHR);
    LOAD_INST_FUNC(vkAllocateCommandBuffers);
    LOAD_INST_FUNC(vkAllocateDescriptorSets);
    LOAD_INST_FUNC(vkAllocateMemory);
    LOAD_INST_FUNC(vkBeginCommandBuffer);
    LOAD_INST_FUNC(vkBindBufferMemory);
    LOAD_INST_FUNC(vkBindImageMemory);
    LOAD_INST_FUNC(vkCmdBeginRenderPass);
    LOAD_INST_FUNC(vkCmdBindDescriptorSets);
    LOAD_INST_FUNC(vkCmdBindIndexBuffer);
    LOAD_INST_FUNC(vkCmdBindPipeline);
    LOAD_INST_FUNC(vkCmdBindVertexBuffers);
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
    LOAD_INST_FUNC(vkCreateBuffer);
    LOAD_INST_FUNC(vkCreateCommandPool);
    LOAD_INST_FUNC(vkCreateComputePipelines);
    LOAD_INST_FUNC(vkCreateDescriptorPool);
    LOAD_INST_FUNC(vkCreateDescriptorSetLayout);
    LOAD_INST_FUNC(vkCreateDevice);
    LOAD_INST_FUNC(vkCreateFence);
    LOAD_INST_FUNC(vkCreateFramebuffer);
    LOAD_INST_FUNC(vkCreateGraphicsPipelines);
    LOAD_INST_FUNC(vkCreateImage);
    LOAD_INST_FUNC(vkCreateImageView);
    LOAD_INST_FUNC(vkCreatePipelineLayout);
    LOAD_INST_FUNC(vkCreateRenderPass);
    LOAD_INST_FUNC(vkCreateSampler);
    LOAD_INST_FUNC(vkCreateSemaphore);
    LOAD_INST_FUNC(vkCreateShaderModule);
    LOAD_INST_FUNC(vkCreateSwapchainKHR);
    LOAD_INST_FUNC(vkDestroyBuffer);
    LOAD_INST_FUNC(vkDestroyCommandPool);
    LOAD_INST_FUNC(vkDestroyDescriptorPool);
    LOAD_INST_FUNC(vkDestroyDescriptorSetLayout);
    LOAD_INST_FUNC(vkDestroyDevice);
    LOAD_INST_FUNC(vkDestroyFence);
    LOAD_INST_FUNC(vkDestroyFramebuffer);
    LOAD_INST_FUNC(vkDestroyImage);
    LOAD_INST_FUNC(vkDestroyImageView);
    LOAD_INST_FUNC(vkDestroyInstance);
    LOAD_INST_FUNC(vkDestroyPipeline);
    LOAD_INST_FUNC(vkDestroyPipelineLayout);
    LOAD_INST_FUNC(vkDestroyRenderPass);
    LOAD_INST_FUNC(vkDestroySampler);
    LOAD_INST_FUNC(vkDestroySemaphore);
    LOAD_INST_FUNC(vkDestroyShaderModule);
    LOAD_INST_FUNC(vkDestroySurfaceKHR);
    LOAD_INST_FUNC(vkDestroySwapchainKHR);
    LOAD_INST_FUNC(vkDeviceWaitIdle);
    LOAD_INST_FUNC(vkEndCommandBuffer);
    LOAD_INST_FUNC(vkEnumerateDeviceExtensionProperties);
    LOAD_INST_FUNC(vkEnumeratePhysicalDevices);
    LOAD_INST_FUNC(vkFreeCommandBuffers);
    LOAD_INST_FUNC(vkFreeDescriptorSets);
    LOAD_INST_FUNC(vkFreeMemory);
    LOAD_INST_FUNC(vkGetBufferMemoryRequirements);
    LOAD_INST_FUNC(vkGetDeviceQueue);
    LOAD_INST_FUNC(vkGetFenceStatus);
    LOAD_INST_FUNC(vkGetImageMemoryRequirements);
    LOAD_INST_FUNC(vkGetPhysicalDeviceFeatures);
    LOAD_INST_FUNC(vkGetPhysicalDeviceFormatProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    LOAD_INST_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR);
    LOAD_INST_FUNC(vkGetSwapchainImagesKHR);
    LOAD_INST_FUNC(vkMapMemory);
    LOAD_INST_FUNC(vkQueuePresentKHR);
    LOAD_INST_FUNC(vkQueueSubmit);
    LOAD_INST_FUNC(vkQueueWaitIdle);
    LOAD_INST_FUNC(vkResetCommandPool);
    LOAD_INST_FUNC(vkResetFences);
    LOAD_INST_FUNC(vkUnmapMemory);
    LOAD_INST_FUNC(vkUpdateDescriptorSets);
    LOAD_INST_FUNC(vkWaitForFences);
    // TODO: use 'vkGetDeviceProcAddr' to load whichever of these we can

    LOAD_OPT_INST_FUNC(vkCreateDebugReportCallbackEXT);
    LOAD_OPT_INST_FUNC(vkCreateDebugUtilsMessengerEXT);
    LOAD_OPT_INST_FUNC(vkDestroyDebugReportCallbackEXT);
    LOAD_OPT_INST_FUNC(vkDestroyDebugUtilsMessengerEXT);

    #undef LOAD_INST_FUNC
}

END_NAMESPACE(vgl)

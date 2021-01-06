#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Container for all of the functions that we will use from the Vulkan API.
// These are all loaded dynamically from the Vulkan DLL or whatever source the functions come from for the current platform.
//------------------------------------------------------------------------------------------------------------------------------------------
struct VkFuncs {
    void loadGlobalFuncs() noexcept;
    void loadInstanceFuncs(const VkInstance vkInstance) noexcept;
    void loadDeviceFuncs(const VkDevice vkDevice) noexcept;

    // Macro to make defining functions less error prone
    #define DEFINE_VK_FUNC(NAME)\
        PFN_##NAME NAME;

    // Global functions not associated with any Vulkan API instance
    DEFINE_VK_FUNC(vkCreateInstance)
    DEFINE_VK_FUNC(vkEnumerateInstanceExtensionProperties)
    DEFINE_VK_FUNC(vkEnumerateInstanceLayerProperties)
    DEFINE_VK_FUNC(vkGetInstanceProcAddr)

    // Vulkan instance level functions, not associated with any particular device
    DEFINE_VK_FUNC(vkBeginCommandBuffer)
    DEFINE_VK_FUNC(vkCmdBeginRenderPass)
    DEFINE_VK_FUNC(vkCmdBindDescriptorSets)
    DEFINE_VK_FUNC(vkCmdBindIndexBuffer)
    DEFINE_VK_FUNC(vkCmdBindPipeline)
    DEFINE_VK_FUNC(vkCmdBindVertexBuffers)
    DEFINE_VK_FUNC(vkCmdBlitImage)
    DEFINE_VK_FUNC(vkCmdClearColorImage)
    DEFINE_VK_FUNC(vkCmdCopyBuffer)
    DEFINE_VK_FUNC(vkCmdCopyBufferToImage)
    DEFINE_VK_FUNC(vkCmdCopyImage)
    DEFINE_VK_FUNC(vkCmdDispatch)
    DEFINE_VK_FUNC(vkCmdDraw)
    DEFINE_VK_FUNC(vkCmdDrawIndexed)
    DEFINE_VK_FUNC(vkCmdEndRenderPass)
    DEFINE_VK_FUNC(vkCmdExecuteCommands)
    DEFINE_VK_FUNC(vkCmdNextSubpass)
    DEFINE_VK_FUNC(vkCmdPipelineBarrier)
    DEFINE_VK_FUNC(vkCmdPushConstants)
    DEFINE_VK_FUNC(vkCmdSetScissor)
    DEFINE_VK_FUNC(vkCmdSetViewport)
    DEFINE_VK_FUNC(vkCreateDevice)
    DEFINE_VK_FUNC(vkDestroyDevice)
    DEFINE_VK_FUNC(vkDestroyInstance)
    DEFINE_VK_FUNC(vkDestroySurfaceKHR)
    DEFINE_VK_FUNC(vkEndCommandBuffer)
    DEFINE_VK_FUNC(vkEnumerateDeviceExtensionProperties)
    DEFINE_VK_FUNC(vkEnumeratePhysicalDevices)
    DEFINE_VK_FUNC(vkGetDeviceProcAddr)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceFeatures)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceFormatProperties)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceProperties)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)
    DEFINE_VK_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR)

    // Vulkan instance level functions that are optional (may or may not be present, depending on extensions available)
    DEFINE_VK_FUNC(vkCreateDebugReportCallbackEXT)
    DEFINE_VK_FUNC(vkCreateDebugUtilsMessengerEXT)
    DEFINE_VK_FUNC(vkDestroyDebugReportCallbackEXT)
    DEFINE_VK_FUNC(vkDestroyDebugUtilsMessengerEXT)

    // Vulkan device level functions
    DEFINE_VK_FUNC(vkAcquireNextImageKHR)
    DEFINE_VK_FUNC(vkAllocateCommandBuffers)
    DEFINE_VK_FUNC(vkAllocateDescriptorSets)
    DEFINE_VK_FUNC(vkAllocateMemory)
    DEFINE_VK_FUNC(vkBindBufferMemory)
    DEFINE_VK_FUNC(vkBindImageMemory)
    DEFINE_VK_FUNC(vkCreateBuffer)
    DEFINE_VK_FUNC(vkCreateCommandPool)
    DEFINE_VK_FUNC(vkCreateComputePipelines)
    DEFINE_VK_FUNC(vkCreateDescriptorPool)
    DEFINE_VK_FUNC(vkCreateDescriptorSetLayout)
    DEFINE_VK_FUNC(vkCreateFence)
    DEFINE_VK_FUNC(vkCreateFramebuffer)
    DEFINE_VK_FUNC(vkCreateGraphicsPipelines)
    DEFINE_VK_FUNC(vkCreateImage)
    DEFINE_VK_FUNC(vkCreateImageView)
    DEFINE_VK_FUNC(vkCreatePipelineLayout)
    DEFINE_VK_FUNC(vkCreateRenderPass)
    DEFINE_VK_FUNC(vkCreateSampler)
    DEFINE_VK_FUNC(vkCreateSemaphore)
    DEFINE_VK_FUNC(vkCreateShaderModule)
    DEFINE_VK_FUNC(vkCreateSwapchainKHR)
    DEFINE_VK_FUNC(vkDestroyBuffer)
    DEFINE_VK_FUNC(vkDestroyCommandPool)
    DEFINE_VK_FUNC(vkDestroyDescriptorPool)
    DEFINE_VK_FUNC(vkDestroyDescriptorSetLayout)
    DEFINE_VK_FUNC(vkDestroyFence)
    DEFINE_VK_FUNC(vkDestroyFramebuffer)
    DEFINE_VK_FUNC(vkDestroyImage)
    DEFINE_VK_FUNC(vkDestroyImageView)
    DEFINE_VK_FUNC(vkDestroyPipeline)
    DEFINE_VK_FUNC(vkDestroyPipelineLayout)
    DEFINE_VK_FUNC(vkDestroyRenderPass)
    DEFINE_VK_FUNC(vkDestroySampler)
    DEFINE_VK_FUNC(vkDestroySemaphore)
    DEFINE_VK_FUNC(vkDestroyShaderModule)
    DEFINE_VK_FUNC(vkDestroySwapchainKHR)
    DEFINE_VK_FUNC(vkDeviceWaitIdle)
    DEFINE_VK_FUNC(vkFreeCommandBuffers)
    DEFINE_VK_FUNC(vkFreeDescriptorSets)
    DEFINE_VK_FUNC(vkFreeMemory)
    DEFINE_VK_FUNC(vkGetBufferMemoryRequirements)
    DEFINE_VK_FUNC(vkGetDeviceQueue)
    DEFINE_VK_FUNC(vkGetFenceStatus)
    DEFINE_VK_FUNC(vkGetImageMemoryRequirements)
    DEFINE_VK_FUNC(vkGetSwapchainImagesKHR)
    DEFINE_VK_FUNC(vkMapMemory)
    DEFINE_VK_FUNC(vkQueuePresentKHR)
    DEFINE_VK_FUNC(vkQueueSubmit)
    DEFINE_VK_FUNC(vkQueueWaitIdle)
    DEFINE_VK_FUNC(vkResetCommandPool)
    DEFINE_VK_FUNC(vkResetFences)
    DEFINE_VK_FUNC(vkUnmapMemory)
    DEFINE_VK_FUNC(vkUpdateDescriptorSets)
    DEFINE_VK_FUNC(vkWaitForFences)

    // Done with this macro
    #undef DEFINE_VK_FUNC
};

END_NAMESPACE(vgl)

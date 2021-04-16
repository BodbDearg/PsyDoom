#include "VulkanInstance.h"

#include "Asserts.h"
#include "Finally.h"
#include "PhysicalDevice.h"
#include "VkFuncs.h"

#include <cstdio>
#include <SDL_vulkan.h>

BEGIN_NAMESPACE(vgl)

// Name of the Vulkan validation layer to use
static constexpr const char* const KHRONOS_VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

//------------------------------------------------------------------------------------------------------------------------------------------
// Function called by the Vulkan debug validation layers.
// Called whenever there is a message (error, info, etc.) to log.
//------------------------------------------------------------------------------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanValidationLayerCallback(
    VkDebugReportFlagsEXT flags,
    [[maybe_unused]] VkDebugReportObjectTypeEXT objectType,
    [[maybe_unused]] uint64_t object,
    [[maybe_unused]] size_t location,
    [[maybe_unused]] int32_t messageCode,
    [[maybe_unused]] const char* pLayerPrefix,
    const char* pMessage,
    [[maybe_unused]] void* pUserData
) noexcept {
    // Set to '1' for more verbose logging
    #define VULKAN_VALIDATION_LAYER_VERBOSE_LOGGING 0

    // See what type of message we are dealing with:
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        std::printf("[VULKAN ERROR]: %s\n\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        std::printf("[VULKAN WARNING]: %s\n\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        std::printf("[VULKAN PERFORMANCE WARNING]: %s\n\n", pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        #if VULKAN_VALIDATION_LAYER_VERBOSE_LOGGING
            std::printf("[VULKAN INFO]: %s\n\n", pMessage);
        #endif
    }
    else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        #if VULKAN_VALIDATION_LAYER_VERBOSE_LOGGING
            std::printf("[VULKAN DEBUG]: %s\n\n", pMessage);
        #endif
    }

    // Don't abort whatever call triggered this. This is normally only returned as true when
    // testing validation layers themselves, according to the Vulkan docs:
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
    return VK_FALSE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the settings used to construct a debug report callback create info
//------------------------------------------------------------------------------------------------------------------------------------------
static VkDebugReportCallbackCreateInfoEXT getDebugReportCallbackCreateInfo() noexcept {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.pfnCallback = vulkanValidationLayerCallback;
    createInfo.flags = (
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT
    );
    return createInfo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Constructs an unintialized Vulkan API instance
//------------------------------------------------------------------------------------------------------------------------------------------
VulkanInstance::VulkanInstance(VkFuncs& funcs) noexcept
    : mbIsValid(false)
    , mpSdlWindow(nullptr)
    , mVkFuncs(funcs)
    , mVkInstance(VK_NULL_HANDLE)
    , mApiLayers()
    , mExtensions()
    , mPhysicalDevices()
    , mValidationLayerCallback(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the instance
//------------------------------------------------------------------------------------------------------------------------------------------
VulkanInstance::~VulkanInstance() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the Vulkan API instance and returns 'true' if this was successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool VulkanInstance::init(SDL_Window* const pSdlWindow) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]() {
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // First query global API functions, then get instance extensions and API layers available
    mpSdlWindow = pSdlWindow;
    mVkFuncs.loadGlobalFuncs();
    mApiLayers.init(mVkFuncs);
    mExtensions.init(mVkFuncs);

    #ifndef NDEBUG
        const bool bTryEnableValidationLayers = (
            mExtensions.hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) &&
            mExtensions.hasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME) &&
            mApiLayers.hasLayer(KHRONOS_VALIDATION_LAYER_NAME)
        );
    #else
        const bool bTryEnableValidationLayers = false;
    #endif

    // Create the instance, then load instance specific API functions
    if (!createVkInstance(bTryEnableValidationLayers))
        return false;

    mVkFuncs.loadInstanceFuncs(mVkInstance);

    // Enable validation layer reporting if we can in debug builds so we can get messages about invalid API usage
    if (bTryEnableValidationLayers && mVkFuncs.vkCreateDebugReportCallbackEXT) {
        enableValidationLayerReporting();
    }

    // Query What physical devices are there to finish up
    queryPhysicalDevices();
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the Vulkan API instance
//------------------------------------------------------------------------------------------------------------------------------------------
void VulkanInstance::destroy(const bool bForceIfInvalid) noexcept {
    // Only do this if we have to...
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;
    mPhysicalDevices.clear();
    disableValidationLayerReporting();

    if (mVkInstance) {
        mVkFuncs.vkDestroyInstance(mVkInstance, nullptr);   // Destroy the Vulkan API instance itself
        mVkInstance = VK_NULL_HANDLE;
    }

    mExtensions = {};
    mApiLayers = {};
    mpSdlWindow = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan API instance and returns 'true' if that operation was successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool VulkanInstance::createVkInstance(const bool bEnableValidationLayers) noexcept {
    // Sanity checks
    ASSERT(!mVkInstance);

    // Fill out this struct with details about our app:
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "PsyDoom";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Don't care, just give it whatever...
    appInfo.pEngineName = "PsyDoom";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // Don't care, just give it whatever...
    appInfo.apiVersion = VK_API_VERSION_1_1;

    // Get the settings to use for validation layer reporting for instance creation and destruction.
    VkDebugReportCallbackCreateInfoEXT dbgReportCallbackCInfo = getDebugReportCallbackCreateInfo();

    // Get the required Vulkan extensions needed by SDL to create a window surface
    std::vector<const char*> reqExtNames;

    {
        // Note: the SDL window CAN be null here if we are making a headless Vulkan instance not associated with any SDL Window.
        unsigned int reqExtCount = {};
        SDL_Vulkan_GetInstanceExtensions(mpSdlWindow, &reqExtCount, nullptr);
        reqExtNames.resize(reqExtCount);
        SDL_Vulkan_GetInstanceExtensions(mpSdlWindow, &reqExtCount, reqExtNames.data());
    }

    // The required API layers: none by default, unless we want validation layers
    std::vector<const char*> reqLayerNames;

    // Request validation layers and the debug utils extension (for reporting) if required
    if (bEnableValidationLayers) {
        reqExtNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        reqExtNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        reqLayerNames.push_back(KHRONOS_VALIDATION_LAYER_NAME);
    }

    // Fill out this struct with details about the instance we want, required extensions, API layers and so on:
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = reqExtNames.data();
    createInfo.enabledExtensionCount = (uint32_t) reqExtNames.size();
    createInfo.ppEnabledLayerNames = reqLayerNames.data();
    createInfo.enabledLayerCount = (uint32_t) reqLayerNames.size();

    if (bEnableValidationLayers) {
        // This line here enables validation layer reporting for instance creation and destruction.
        // Normally you must setup your own reporting object but that requires an instance and thus won't be used for instance create/destroy.
        // If we point this list to a debug report 'create info' then the API will do reporting for instance create/destroy accordingly.
        createInfo.pNext = &dbgReportCallbackCInfo;
    }

    // Create the instance, with no custom allocator:
    if (mVkFuncs.vkCreateInstance(&createInfo, nullptr, &mVkInstance) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create the Vulkan instance!");
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the list of physical devices available
//------------------------------------------------------------------------------------------------------------------------------------------
void VulkanInstance::queryPhysicalDevices() noexcept {
    mPhysicalDevices.clear();

    // Get the number of handles and then the handles for each physical device
    uint32_t deviceCount = 0;
    mVkFuncs.vkEnumeratePhysicalDevices(mVkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDeviceHandles(deviceCount);
    mVkFuncs.vkEnumeratePhysicalDevices(mVkInstance, &deviceCount, physicalDeviceHandles.data());

    // Now save the details for each physical device
    mPhysicalDevices.reserve(deviceCount);

    for (uint32_t i = 0; i < deviceCount; ++i) {
        mPhysicalDevices.emplace_back(mVkFuncs, *this, physicalDeviceHandles[i]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enables validation layer reporting for all Vulkan API calls, except instance creation and destruction
//------------------------------------------------------------------------------------------------------------------------------------------
void VulkanInstance::enableValidationLayerReporting() noexcept {
    // If we already have a validation layer callback do nothing else, our work is already done:
    if (mValidationLayerCallback)
        return;

    // Fill in the details about the debug report callback to be created and register the callback
    VkDebugReportCallbackCreateInfoEXT createInfo = getDebugReportCallbackCreateInfo();
    mVkFuncs.vkCreateDebugReportCallbackEXT(mVkInstance, &createInfo, nullptr, &mValidationLayerCallback);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down validation layer reporting for all Vulkan API calls, except instance creation and destruction
//------------------------------------------------------------------------------------------------------------------------------------------
void VulkanInstance::disableValidationLayerReporting() noexcept {
    if (mValidationLayerCallback && mVkFuncs.vkDestroyDebugReportCallbackEXT) {
        mVkFuncs.vkDestroyDebugReportCallbackEXT(mVkInstance, mValidationLayerCallback, nullptr);
    }

    mValidationLayerCallback = VK_NULL_HANDLE;
}

END_NAMESPACE(vgl)

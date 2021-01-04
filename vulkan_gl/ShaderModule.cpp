#include "ShaderModule.h"

#include "Finally.h"
#include "LogicalDevice.h"
#include "RetirementMgr.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized shader module
//------------------------------------------------------------------------------------------------------------------------------------------
ShaderModule::ShaderModule() noexcept
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mVkShaderStageFlagBits()
    , mVkShaderModule(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a shader to this object
//------------------------------------------------------------------------------------------------------------------------------------------
ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mpDevice(other.mpDevice)
    , mVkShaderStageFlagBits(other.mVkShaderStageFlagBits)
    , mVkShaderModule(other.mVkShaderModule)

{
    other.mbIsValid = false;
    other.mpDevice = nullptr;
    other.mVkShaderStageFlagBits = {};
    other.mVkShaderModule = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the shader
//------------------------------------------------------------------------------------------------------------------------------------------
ShaderModule::~ShaderModule() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the shader with the specified stage flags (giving type) and shader code
//------------------------------------------------------------------------------------------------------------------------------------------
bool ShaderModule::init(
    LogicalDevice& device,
    const VkShaderStageFlagBits stageFlags,
    const uint32_t* const pCode,
    const size_t codeSize
) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // If there is no data we can't make a shader!
    if (!pCode)
        return false;

    // Save basic info
    mpDevice = &device;
    mVkShaderStageFlagBits = stageFlags;

    // Create the Vulkan shader module
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = pCode;

    const VkFuncs& vkFuncs = device.getVkFuncs();

    if (vkFuncs.vkCreateShaderModule(device.getVkDevice(), &createInfo, nullptr, &mVkShaderModule) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a shader module!");
        return false;
    }

    ASSERT(mVkShaderModule);

    // Success!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the shader module and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void ShaderModule::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Normal cleanup logic
    mbIsValid = false;

    if (mVkShaderModule) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyShaderModule(mpDevice->getVkDevice(), mVkShaderModule, nullptr);
        mVkShaderModule = VK_NULL_HANDLE;
    }

    mpDevice = nullptr;
    mVkShaderStageFlagBits = {};
}

END_NAMESPACE(vgl)

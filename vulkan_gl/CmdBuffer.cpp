#include "CmdBuffer.h"

#include "Asserts.h"
#include "CmdPool.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "Macros.h"
#include "RetirementMgr.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
CmdBuffer::CmdBuffer() noexcept
    : mbIsValid(false)
    , mLevel()
    , mpCmdPool(nullptr)
    , mVkCommandBuffer(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a command buffer to this object
//------------------------------------------------------------------------------------------------------------------------------------------
CmdBuffer::CmdBuffer(CmdBuffer&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mLevel(other.mLevel)
    , mpCmdPool(other.mpCmdPool)
    , mVkCommandBuffer(other.mVkCommandBuffer)
{
    other.mbIsValid = false;
    other.mLevel = {};
    other.mpCmdPool = nullptr;
    other.mVkCommandBuffer = VK_NULL_HANDLE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the command buffer
//------------------------------------------------------------------------------------------------------------------------------------------
CmdBuffer::~CmdBuffer() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the command buffer and allocates from the given command pool - returns 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool CmdBuffer::init(CmdPool& cmdPool, const VkCommandBufferLevel level) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(cmdPool.isValid());
    ASSERT(cmdPool.getDevice()->getVkDevice());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true, true);
        }
    });

    // Save basic info
    mpCmdPool = &cmdPool;
    mLevel = level;

    // Create the command buffer
    VkCommandBufferAllocateInfo bufferAllocInfo = {};
    bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocInfo.commandPool = cmdPool.getVkCommandPool();
    bufferAllocInfo.level = level;
    bufferAllocInfo.commandBufferCount = 1;

    const VkFuncs& vkFuncs = cmdPool.getDevice()->getVkFuncs();

    if (vkFuncs.vkAllocateCommandBuffers(cmdPool.getDevice()->getVkDevice(), &bufferAllocInfo, &mVkCommandBuffer) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create the command buffers!");
        return false;
    }

    ASSERT(mVkCommandBuffer);

    // Success!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the fence and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void CmdBuffer::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpCmdPool) || mpCmdPool->isValid(), "Parent pool must still be valid if defined!");
    ASSERT_LOG((!mpCmdPool) || mpCmdPool->getDevice()->getVkDevice(), "Command pool parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpCmdPool->getDevice()->getRetirementMgr().retire(*this);
        return;
    }

    // Regular cleanup logic
    mbIsValid = false;
    
    if (mVkCommandBuffer) {
        ASSERT(mpCmdPool);
        ASSERT(mpCmdPool->getDevice()->getVkDevice());
        const VkFuncs& vkFuncs = mpCmdPool->getDevice()->getVkFuncs();

        vkFuncs.vkFreeCommandBuffers(
            mpCmdPool->getDevice()->getVkDevice(),
            mpCmdPool->getVkCommandPool(),
            1,
            &mVkCommandBuffer
        );

        mVkCommandBuffer = VK_NULL_HANDLE;
    }

    mpCmdPool = nullptr;
    mLevel = {};
}

END_NAMESPACE(vgl)

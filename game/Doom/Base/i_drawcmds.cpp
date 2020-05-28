#include "i_drawcmds.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Module dealing with low level drawing commands.
// Holds the primitives buffer and contains functionality for submitting draw primitives.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"

// GPU packets beginning and end pointer and the 64 KiB buffer used to hold GPU primitives
const VmPtr<VmPtr<std::byte>>       gpGpuPrimsBeg(0x80077C14);
const VmPtr<VmPtr<std::byte>>       gpGpuPrimsEnd(0x80077C18);
const VmPtr<std::byte[0x10000]>     gGpuCmdsBuffer(0x80086550);
const VmPtr<std::byte>              gGpuCmdsBufferEnd(0x80096550);

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: makes up the tag which is at the beginning of every draw primitive in the command buffer.
// Note: the address given is restricted to 24-bits and is expected to be relative to the 0x80000000 memory segment.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t makePrimTag(const uint32_t primSize, void* const pNextPrim) noexcept {
    // Sanity check
    #if PC_PSX_DOOM_MODS
        ASSERT(primSize % 4 == 0 && primSize >= 4);
    #endif

    const uint32_t numPrimDataWords = (primSize / sizeof(uint32_t)) - 1;    // Note: the tag size is excluded!
    const uint32_t nextPrimAddr = ptrToVmAddr(pNextPrim);
    return (numPrimDataWords << 24) | (nextPrimAddr & 0x00FFFFFF);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// GPU status check: tells if CPU to GPU dma enabled
// PC-PSX: this is not needed anymore.
//------------------------------------------------------------------------------------------------------------------------------------------
#if !PC_PSX_DOOM_MODS
static bool isCpuToGpuDmaEnabled() noexcept {
    return (getGpuStat() & 0x04000000);
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Flushes the GPU command queue and sends all commands to the GPU.
// Note: this is only allowed however if CPU to GPU DMA is enabled!
//------------------------------------------------------------------------------------------------------------------------------------------
static void flushGpuCmds() noexcept {
    while (*gpGpuPrimsBeg != *gpGpuPrimsEnd) {
        // Abort for now if CPU to GPU DMA is off; hope that it gets turned on later or the queue gets cleared somehow?
        // Not sure what situation is would occur in...
        //
        // PC-PSX: I'm disabling this check for good measure, just to be on the safe side.
        // Drawing is pretty much native now so emulator DMA should not be getting in our way.
        //
        #if !PC_PSX_DOOM_MODS
            if (!isCpuToGpuDmaEnabled())
                break;
        #endif
        
        // Read the tag for this primitive the next primitive address
        const uint32_t tag = ((uint32_t*) gpGpuPrimsBeg->get())[0];
        const uint32_t nextPrimAddr = (tag & 0x00FFFFFF) | 0x80000000;

        // Submit the primitive to the GPU and move onto the next one
        PsxVm::submitGpuPrimitive(gpGpuPrimsBeg->get());
        *gpGpuPrimsBeg = nextPrimAddr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a primitive to the gpu command buffers to be rendered.
// The primitive must be one of the primitives defined in the PlayStation SDK for 'LIBGPU'.
//
// Note: this was not an actual function in PSXDOOM.EXE from what I can see but the same code is inlined pratically everywhere there is drawing.
// Perhaps this was an inline header function or a series of crazy macros, who knows?
// In any case much cleaner to make an actual function of it for this port and re-use wherever needed.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_AddPrim(const void* const pPrim) noexcept {
    // Determine the size of the primitive payload/data in words and total size in bytes
    const uint32_t primHdrWord = ((const uint32_t*) pPrim)[0];
    const uint32_t numPrimDataWords = primHdrWord >> 24;
    const uint32_t primSize = (numPrimDataWords + 1) * sizeof(uint32_t);

    // Make enough room to insert the new primitive into the command buffer.
    // Continue looping until we've made the room!
    while (true) {
        // Is the command queue not wrapping around?
        if (*gpGpuPrimsBeg <= *gpGpuPrimsEnd) {
            // Command queue doesn't wrap around currently.
            // If there's already enough room for the primitive then we are done:
            if (gpGpuPrimsEnd->get() + primSize < gGpuCmdsBufferEnd.get())
                break;
            
            // Okay, there's not enough room for the primitive.
            // Need to insert a dummy primitive/tag to link back to the start of the command buffer, and then wraparound the queue.
            // Note: There should be room for a 32-bit integer, due to the way the size checks are done!
            const uint32_t tag = makePrimTag(sizeof(uint32_t), gGpuCmdsBuffer.get());
            ((uint32_t*) gpGpuPrimsEnd->get())[0] = tag;
            *gpGpuPrimsEnd = gGpuCmdsBuffer;
        }

        // The command queue is wrapping around if we are getting here.
        // In this case, check to see if there is enough room for the primitive and if so then we are done:
        if (gpGpuPrimsEnd->get() + primSize < gpGpuPrimsBeg->get())
            break;

        // There is not enough room for the primitive and the queue is wrapping!
        // In this situation flush the primitives queue by submitting the commands to the GPU.
        // When we are done there should be plenty of room for the draw command and more.
        flushGpuCmds();
    }

    // Write the given primitive into the command buffer
    {
        // Write the primitive tag
        const uint32_t nextPrimAddr24 = (gpGpuPrimsEnd->addr() + primSize) & 0x00FFFFFF;
        const uint32_t primTag = (numPrimDataWords << 24) | nextPrimAddr24;

        uint32_t* pDstWord = (uint32_t*) gpGpuPrimsEnd->get();
        *pDstWord = primTag;
        ++pDstWord;

        // Copy the rest of the primitive data words
        uint32_t* pSrcWord = (uint32_t*) pPrim + 1;

        for (uint32_t numWordsLeft = numPrimDataWords; numWordsLeft > 0; --numWordsLeft) {
            *pDstWord = *pSrcWord;
            ++pDstWord;
            ++pSrcWord;
        }

        // Move along in the command buffer
        *gpGpuPrimsEnd += primSize;
    }

    // Not sure why this was here...
    // This is logic to flush the GPU commands buffer similar to when we are out of space for writing a command.
    // Seems somewhat counter intuitive to go to the trouble of doing a command queue when all drawing is forced to be immediate?!
    flushGpuCmds();
}

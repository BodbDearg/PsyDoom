#include "Old_LIBGPU.h"

#if !PSYDOOM_MODS

#include "Asserts.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a linked list of GPU drawing primitives.
// These may include drawing primitves, or primitives that set GPU state.
//
// PsyDoom: had to add a pointer to the base of the GPU commands buffer.
// This is because the 'next primitive pointer' is now a 24-bit offset within the command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_DrawOTag(const void* const pPrimList, std::byte* const pGpuCmdBuffer) noexcept {
    ASSERT(pGpuCmdBuffer);
    const uint32_t* pCurPrim = (const uint32_t*) pPrimList;

    while (true) {
        // Read the tag for this primitive and determine the next primitive address (24-bit relative, and absolute).
        const uint32_t tag = pCurPrim[0];
        const uint32_t nextPrimOffset24 = tag & 0x00FFFFFF;

        // Submit the primitive to the GPU
        PsxVm::submitGpuPrimitive(pCurPrim);

        // Stop if we've reached the end of the primitive list, otherwise move onto the next one
        if (nextPrimOffset24 == 0x00FFFFFF)
            break;

        pCurPrim = (uint32_t*)(pGpuCmdBuffer + nextPrimOffset24);
    }
}

#endif  // #if !PSYDOOM_MODS

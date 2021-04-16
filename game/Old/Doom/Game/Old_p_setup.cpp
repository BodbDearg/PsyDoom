#if !PSYDOOM_MODS

#include "Doom/Game/p_setup.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads a list of memory blocks containing WAD lumps from the given file
//------------------------------------------------------------------------------------------------------------------------------------------
void P_LoadBlocks(const CdFileId file) noexcept {
    // Try and load the memory blocks containing lumps from the given file.
    // Retry this a number of times before giving up, if the initial load attempt fails.
    // Presumably this was to try and recover from a bad CD...
    int32_t numLoadAttempts = 0;
    int32_t fileSize = -1;
    std::byte* pBlockData = nullptr;
    memblock_t initialAllocHeader = {};

    while (true) {
        // If there have been too many failed load attempts then issue an error
        if (numLoadAttempts >= 4) {
            I_Error("P_LoadBlocks: Data Failure");
        }

        ++numLoadAttempts;

        // Open the blocks file and get it's size
        const int32_t openFileIdx = OpenFile(file);
        fileSize = SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::END);

        // Alloc room to hold the file: note that we reduce the alloc size by 'sizeof(fileblock_t)' since the blocks
        // file already includes space for a 'fileblock_t' header for each lump. We also save the current memblock
        // header just in case loading fails, so we can restore it prior to deallocation...
        std::byte* const pAlloc = (std::byte*) Z_Malloc(*gpMainMemZone, fileSize - sizeof(fileblock_t), PU_STATIC, nullptr);
        initialAllocHeader = ((memblock_t*) pAlloc)[-1];
        pBlockData = (std::byte*) &((fileblock_t*) pAlloc)[-1];

        // Read the file contents
        SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::SET);
        ReadFile(openFileIdx, pBlockData, fileSize);
        CloseFile(openFileIdx);

        // Process all of the memory blocks in the file and make sure they are ok.
        // Once they are verified then we can start linking them in with other memory blocks in the heap:
        bool bLoadedOk = true;
        int32_t bytesLeft = fileSize;
        std::byte* pCurBlockData = pBlockData;

        do {
            // Verify the block has a valid zoneid
            fileblock_t& fileBlock = *(fileblock_t*) pCurBlockData;

            if (fileBlock.id != ZONEID) {
                bLoadedOk = false;
                break;
            }

            // Verify the lump number is valid
            if (fileBlock.lumpNum >= gNumLumps) {
                bLoadedOk = false;
                break;
            }

            // Verify the compression mode is valid
            if (fileBlock.isUncompressed >= 2) {
                bLoadedOk = false;
                break;
            }

            // Verify the decompressed size is valid
            if (fileBlock.isUncompressed == 0) {
                // Get the decompressed size of the data following the file block header and make sure it is what we expect
                const uint32_t inflatedSize = getDecodedSize(&(&fileBlock)[1]);
                const lumpinfo_t& lump = gpLumpInfo[fileBlock.lumpNum];

                if (inflatedSize != lump.size) {
                    bLoadedOk = false;
                    break;
                }
            }

            // Advance onto the next block and make sure we haven't gone past the end of the data
            const int32_t blockSize = fileBlock.size;
            bytesLeft -= blockSize;

            if (bytesLeft < 0) {
                bLoadedOk = false;
                break;
            }

            pCurBlockData += blockSize;
        } while (bytesLeft != 0);

        // If everything was loaded ok then link the first block into the heap block list and finish up.
        // Will do the rest of the linking in the loop below:
        if (bLoadedOk) {
            memblock_t& memblock = *(memblock_t*) pBlockData;
            memblock.prev = initialAllocHeader.prev;
            break;
        }

        // Load failed: restore the old alloc header and free the memory block.
        // Will try again a certain number of times to try and counteract unreliable CDs.
        ((memblock_t*) pBlockData)[0] = initialAllocHeader;
        Z_Free2(*gpMainMemZone, pAlloc);
    }

    // Once all the blocks are loaded and verified then setup all of the block links.
    // Also mark blocks for lumps that are already loaded as freeable.
    std::byte* pCurBlockData = pBlockData;
    int32_t bytesLeft = fileSize;

    do {
        // Note: making a copy of the fileblock header to avoid possible strict aliasing weirdness reading and writing to the
        // same memory using different struct types. The original code did not do that but this should be functionally the same.
        fileblock_t fileblock = *(fileblock_t*) pCurBlockData;
        memblock_t& memblock = *(memblock_t*) pCurBlockData;

        // Check if this lump is already loaded
        void*& lumpCacheEntry = gpLumpCache[fileblock.lumpNum];

        if (lumpCacheEntry) {
            // If the lump is already loaded then mark this memory block as freeable
            memblock.user = nullptr;
            memblock.tag = 0;
            memblock.id = 0;
        } else {
            // Lump not loaded, set the lump cache entry to point to the newly loaded data.
            // Also save whether the lump is compressed or not:
            memblock.user = &lumpCacheEntry;
            lumpCacheEntry = &memblock + 1;
            gpbIsUncompressedLump[fileblock.lumpNum] = fileblock.isUncompressed;
        }

        // Is this the last loaded block in the file?
        // If it is then set the size based on where the next block in the heap starts, otherwise just use the size defined in the file.
        bytesLeft -= fileblock.size;

        if (bytesLeft != 0) {
            memblock_t* const pNextMemblock = (memblock_t*)(pCurBlockData + fileblock.size);
            memblock.next = pNextMemblock;
        } else {
            const uint32_t blockSize = (uint32_t)((std::byte*) initialAllocHeader.next - pCurBlockData);

            if (initialAllocHeader.next) {
                memblock.size = blockSize;
            }

            memblock.next = initialAllocHeader.next;
        }

        // Set backlinks for the next block
        if (memblock.next) {
            memblock.next->prev = &memblock;
        }

        // Move onto the next block loaded
        pCurBlockData = (std::byte*) memblock.next;

    } while (bytesLeft != 0);

    // After all that is done, make sure the heap is valid
    Z_CheckHeap(*gpMainMemZone);
}

#endif  // #if !PSYDOOM_MODS

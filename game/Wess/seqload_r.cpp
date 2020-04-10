//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): sequence loader range/batch functions.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "seqload_r.h"

#include "seqload.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the total size of a range of sequences, starting at the given index
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_range_sizeof(const int32_t firstSeqIdx, const int32_t numSeqs) noexcept {
    if ((!*gbWess_seq_loader_enable) || (numSeqs == 0))
        return 0;

    const int32_t endSeqIdx = firstSeqIdx + numSeqs;
    int32_t totalSize = 0;

    for (int32_t seqIdx = 0; seqIdx < endSeqIdx; ++seqIdx) {
        totalSize += wess_seq_sizeof(seqIdx);
    }

    return totalSize;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Load a range of sequences into the given memory block, which is expected to be big enough, and return how much memory was used
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_range_load(const int32_t firstSeqIdx, const int32_t numSeqs, void* const pSeqMem) noexcept {
    // If the sequencer loader is not intialized or we can't open the module file containing sequences then loading fails
    if ((!*gbWess_seq_loader_enable) || (!open_sequence_data()))
        return 0;

    // Try load all of the sequences specified
    const int32_t endSeqIdx = firstSeqIdx + numSeqs;
    
    int32_t totalBytesUsed = 0;
    uint8_t* pSeqMemBytes = (uint8_t*) pSeqMem;

    for (int32_t seqIdx = firstSeqIdx; seqIdx < endSeqIdx; ++seqIdx) {
        const int32_t seqBytesUsed = wess_seq_load(seqIdx, pSeqMemBytes);
        totalBytesUsed += seqBytesUsed;
        pSeqMemBytes += seqBytesUsed;
    }

    // Cleanup by closing the module file and return how much memory was used
    close_sequence_data();
    return totalBytesUsed;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears out the sequence data references for a range of sequences.
// Does not actually free memory, just nulls pointers.
// Returns 'true' if any sequences were 'freed'.
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_seq_range_free(const int32_t firstSeqIdx, const int32_t numSeqs) noexcept {
    // Don't do anything if the sequence loader is not initialized or if no sequences were specified
    if ((!*gbWess_seq_loader_enable) || (numSeqs == 0))
        return false;

    // 'Free' the range of sequences
    const int32_t endSeqIdx = firstSeqIdx + numSeqs;

    for (int32_t seqIdx = firstSeqIdx; seqIdx < endSeqIdx; ++seqIdx) {
        wess_seq_free(seqIdx);
    }

    return true;
}

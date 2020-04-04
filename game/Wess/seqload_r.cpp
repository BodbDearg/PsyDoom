#include "seqload_r.h"

#include "PsxVm/PsxVm.h"
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

void wess_seq_range_free() noexcept {
loc_80049B90:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s3, sp + 0x1C);
    s3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049BF8;
    s1 = a0;
    if (s0 != 0) goto loc_80049BCC;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049BFC;
loc_80049BCC:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 == v0);
        v0 = s3;                                        // Result = 00000000
        if (bJump) goto loc_80049BFC;
    }
    s3 = 1;                                             // Result = 00000001
    s2 = -1;                                            // Result = FFFFFFFF
loc_80049BE4:
    a0 = s1;
    v0 = wess_seq_free(a0);
    s0--;
    s1++;
    if (s0 != s2) goto loc_80049BE4;
loc_80049BF8:
    v0 = s3;
loc_80049BFC:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): sequence/song loader.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "seqload.h"

#include "PsxVm/PsxVm.h"
#include "wessarc.h"

static const VmPtr<int32_t>                             gWess_num_sequences(0x8007596C);            // TODO: COMMENT
static const VmPtr<int32_t>                             gWess_seqld_moduleRefCount(0x80075974);     // TODO: COMMENT
static const VmPtr<VmPtr<PsxCd_File>>                   gpWess_seqld_moduleFile(0x80075980);        // TODO: COMMENT
static const VmPtr<CdMapTbl_File>                       gWess_seqld_moduleFileId(0x80075964);       // TODO: COMMENT
static const VmPtr<bool32_t>                            gbWess_seq_loader_enable(0x80075960);       // TODO: COMMENT
static const VmPtr<VmPtr<master_status_structure>>      gpWess_seqld_mstat(0x80075968);             // TODO: COMMENT
static const VmPtr<track_header>                        gWess_seqld_baseTrackHdr(0x8007F068);       // TODO: COMMENT

static SeqLoaderErrorHandler    gpWess_seqld_errorHandler;      // Callback invoked if there are problems loading sequences
static int32_t                  gWess_seqld_errorModule;        // Module value passed to the error handling callback

//------------------------------------------------------------------------------------------------------------------------------------------
// Invokes the sequence loader error handler with the given error code
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_load_err(const Seq_Load_Error errorCode) noexcept {
    if (gpWess_seqld_errorHandler) {
        gpWess_seqld_errorHandler(gWess_seqld_errorModule, errorCode);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Install an error callback which can be invoked if errors occur with the sequence loader
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_loader_install_error_handler(const SeqLoaderErrorHandler handler, const int32_t module) noexcept {
    gpWess_seqld_errorHandler = handler;
    gWess_seqld_errorModule = module;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given sequence index is valid for the open module (.WMD) file
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Seq_Seq_Num_Valid(const int32_t seqIdx) noexcept {
    return ((seqIdx >= 0) && (seqIdx < *gWess_num_sequences));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Retain or add a reference to the open module file used to load sequences.
// If the file is not opened yet then it is opened.
// Returns 'true' if the file is successfully opened.
//------------------------------------------------------------------------------------------------------------------------------------------
bool open_sequence_data() noexcept {
    if (*gWess_seqld_moduleRefCount == 0) {
        *gpWess_seqld_moduleFile = module_open(*gWess_seqld_moduleFileId);

        if (!gpWess_seqld_moduleFile->get()) {
            wess_seq_load_err(SEQLOAD_FOPEN);
            return false;
        }
    }

    *gWess_seqld_moduleRefCount += 1;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Release a reference to the open module file used to load sequences.
// If the reference count falls to '0' then the file is closed.
//------------------------------------------------------------------------------------------------------------------------------------------
void close_sequence_data() noexcept {
    if (*gWess_seqld_moduleRefCount == 1) {
        module_close(*gpWess_seqld_moduleFile->get());
    }

    if (*gWess_seqld_moduleRefCount > 0) {
        *gWess_seqld_moduleRefCount -= 1;
    }
}

void load_sequence_data() noexcept {
loc_8004487C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x50;
    sw(s5, sp + 0x3C);
    s5 = a0;
    sw(s1, sp + 0x2C);
    s1 = a1;
    sw(fp, sp + 0x48);
    fp = s1;
    sw(ra, sp + 0x4C);
    sw(s7, sp + 0x44);
    sw(s6, sp + 0x40);
    sw(s4, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s0, sp + 0x28);
    if (v0 == 0) goto loc_80044FF0;
    v0 = Is_Seq_Seq_Num_Valid(a0);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80044FF4;
    }
    v0 = open_sequence_data();
    {
        const bool bJump = (v0 != 0);
        v0 = s5 << 2;
        if (bJump) goto loc_800448E8;
    }
    a0 = 1;                                             // Result = 00000001
    goto loc_80044E9C;
loc_800448E8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    s7 = lw(v0 + 0x10);
    a1 = lw(v0 + 0x8);
    sw(s1, v0 + 0x4);
    if (s7 == 0) goto loc_80044924;
    v0 = s7 << 5;
    s1 += v0;
    goto loc_80044928;
loc_80044924:
    s1 += 0x20;
loc_80044928:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a2 = 0;                                             // Result = 00000000
    v0 = module_seek(*vmAddrToPtr<PsxCd_File>(a0), a1, (PsxCd_SeekMode) a2);
    s0 = 4;                                             // Result = 00000004
    if (v0 != 0) goto loc_80044E98;
    a1 = 4;                                             // Result = 00000004
    v0 = s5 << 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 += s5;
    v1 = lw(v1 + 0xC);
    s2 = v0 << 2;
    a0 = lw(v1 + 0x10);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a0 += s2;
    v0 = module_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s2;
    s4 = lh(v0);
    v0 = -1;                                            // Result = FFFFFFFF
    s4--;
    s0 = 0x18;                                          // Result = 00000018
    if (s4 == v0) goto loc_80044E60;
    s6 = 0x80080000;                                    // Result = 80080000
    s6 -= 0xFB0;                                        // Result = 8007F050
    s3 = s2;
    s2 = 0;                                             // Result = 00000000
loc_800449BC:
    a0 = s6;                                            // Result = 8007F050
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a1 = 0x18;                                          // Result = 00000018
    v0 = module_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v1 = lbu(s6);                                       // Load from: 8007F050
    t0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044ACC;
    v0 = 0x32;                                          // Result = 00000032
    {
        const bool bJump = (v1 == v0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80044ACC;
    }
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = lbu(a1 + 0x8);
    a0--;
    {
        const bool bJump = (a0 == v0);
        v0 = a0 << 2;
        if (bJump) goto loc_80044AE8;
    }
    t1 = v1;
    a3 = a1;
    v1 = lw(a1 + 0x18);
    v0 += a0;
    v0 <<= 2;
    v0 += a0;
    a1 = v0 << 2;
    a2 = a1 + v1;
loc_80044A34:
    v0 = lw(a2 + 0x4C);
    if (t1 != v0) goto loc_80044AD4;
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    v0 = 3;                                             // Result = 00000003
    if (v1 == 0) goto loc_80044A5C;
    if (v1 != v0) goto loc_80044A70;
loc_80044A5C:
    v0 = lw(a2 + 0x50);
    v0 &= 1;
    if (v0 != 0) goto loc_80044ACC;
loc_80044A70:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xFAC);                               // Load from: 8007F054
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044AA4;
    }
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 2;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044ACC;
    }
loc_80044AA4:
    if (v1 != v0) goto loc_80044AD4;
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 4;
    if (v0 == 0) goto loc_80044AD4;
loc_80044ACC:
    t0 = 1;                                             // Result = 00000001
    goto loc_80044AE8;
loc_80044AD4:
    a1 -= 0x54;
    a0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 -= 0x54;
    if (a0 != v0) goto loc_80044A34;
loc_80044AE8:
    a2 = 1;                                             // Result = 00000001
    if (t0 == 0) goto loc_80044E24;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    v1 = lw(s6);                                        // Load from: 8007F050
    a0 = lw(s6 + 0x4);                                  // Load from: 8007F054
    a1 = lw(s6 + 0x8);                                  // Load from: 8007F058
    a2 = lw(s6 + 0xC);                                  // Load from: 8007F05C
    sw(v1, v0);
    sw(a0, v0 + 0x4);
    sw(a1, v0 + 0x8);
    sw(a2, v0 + 0xC);
    v1 = lw(s6 + 0x10);                                 // Load from: 8007F060
    a0 = lw(s6 + 0x14);                                 // Load from: 8007F064
    sw(v1, v0 + 0x10);
    sw(a0, v0 + 0x14);
    v1 = lbu(s6);                                       // Load from: 8007F050
    v0 = 0x32;                                          // Result = 00000032
    if (v1 != v0) goto loc_80044D20;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sb(0, v0);
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    a0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044BA8;
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044BFC;
    }
loc_80044BA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a3 = v0;
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044BCC:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 1;
    a0++;
    if (v0 != 0) goto loc_80044C5C;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044BCC;
    goto loc_80044D20;
loc_80044BFC:
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044CCC;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a3 = v0;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044C2C:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 2;
    a0++;
    if (v0 != 0) goto loc_80044C80;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044C2C;
    goto loc_80044D20;
loc_80044C5C:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044C80:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044CA4:
    v0 = lw(a1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(v1 + 0x4C);
    v0 += s2;
loc_80044CC4:
    sb(v1, v0);
    goto loc_80044D20;
loc_80044CCC:
    if (v1 != v0) goto loc_80044D20;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v0;
    a1 = v1;
    v1 = lw(a1 + 0x18);
loc_80044CFC:
    v0 = lw(v1 + 0x50);
    v0 &= 4;
    a0++;
    if (v0 != 0) goto loc_80044CA4;
    v0 = (i32(a0) < i32(a2));
    v1 += 0x54;
    if (v0 != 0) goto loc_80044CFC;
loc_80044D20:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
loc_80044D28:
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lh(v0 + 0x12);
    a0 = lw(v0 + 0x18);
    v1 <<= 2;
    s1 += v1;
    s0 = v1;
    a1 = s0;
    v0 = module_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    a0 = 2;                                             // Result = 00000002
    if (s0 != v0) goto loc_80044E9C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lw(v0 + 0x14);
    a0 = lw(v0 + 0x1C);
    s1 += v1;
    v0 = s1 & 1;
    s1 += v0;
    v0 = s1 & 2;
    s1 += v0;
    s0 = v1;
    a1 = s0;
    v0 = module_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    s2 += 0x20;                                         // Result = 00000020
    if (s0 != v0) goto loc_80044E90;
    s4--;
    goto loc_80044E54;
loc_80044E24:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lh(a1 - 0xF9E);                                // Load from: 8007F062
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF9C);                                // Load from: 8007F064
    a1 <<= 2;
    a1 += v0;
    v0 = module_seek(*vmAddrToPtr<PsxCd_File>(a0), a1, (PsxCd_SeekMode) a2);
    a0 = 3;                                             // Result = 00000003
    if (v0 != 0) goto loc_80044E9C;
    s4--;
loc_80044E54:
    v0 = -1;                                            // Result = FFFFFFFF
    s0 = 0x18;                                          // Result = 00000018
    if (s4 != v0) goto loc_800449BC;
loc_80044E60:
    v0 = s5 << 2;
    if (s7 == 0) goto loc_80044EAC;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    sh(s7, v0);
    goto loc_80044FE8;
loc_80044E90:
    a0 = 2;                                             // Result = 00000002
    goto loc_80044E9C;
loc_80044E98:
    a0 = 3;                                             // Result = 00000003
loc_80044E9C:
    wess_seq_load_err(SEQLOAD_FSEEK);
    v0 = 0;                                             // Result = 00000000
    goto loc_80044FF4;
loc_80044EAC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = s5 << 2;
    v0 = lw(v0 + 0xC);
    a0 += s5;
    v0 = lw(v0 + 0x10);
    a0 <<= 2;
    v0 += a0;
    v0 = lw(v0 + 0x4);
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xF98;                                        // Result = 8007F068
    v1 = lw(a3);                                        // Load from: 8007F068
    a1 = lw(a3 + 0x4);                                  // Load from: 8007F06C
    a2 = lw(a3 + 0x8);                                  // Load from: 8007F070
    sw(v1, v0);
    sw(a1, v0 + 0x4);
    sw(a2, v0 + 0x8);
    v1 = lw(a3 + 0xC);                                  // Load from: 8007F074
    a1 = lw(a3 + 0x10);                                 // Load from: 8007F078
    a2 = lw(a3 + 0x14);                                 // Load from: 8007F07C
    sw(v1, v0 + 0xC);
    sw(a1, v0 + 0x10);
    sw(a2, v0 + 0x14);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5970);                              // Load from: 80075970
    a1 = lw(v0 + 0x1C);
    a2 = lw(v0 + 0x14);
    sb(v1, a1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = lw(v0 + 0x1C);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5971);                              // Load from: 80075971
    s1 += a2;
    sb(v0, v1 + 0x1);
    v0 = s1 & 1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    s1 += v0;
    v0 = lw(v1 + 0xC);
    v1 = s1 & 2;
    v0 = lw(v0 + 0x10);
    s1 += v1;
    a0 += v0;
    v0 = 1;                                             // Result = 00000001
    sh(v0, a0);
loc_80044FE8:
    close_sequence_data();
loc_80044FF0:
    v0 = s1 - fp;
loc_80044FF4:
    ra = lw(sp + 0x4C);
    fp = lw(sp + 0x48);
    s7 = lw(sp + 0x44);
    s6 = lw(sp + 0x40);
    s5 = lw(sp + 0x3C);
    s4 = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x50;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the sequencer loader using the given master status structure and module file, returns 'true' on success.
// Optionally the module file can be pre-opened in preparation for access later.
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_seq_loader_init(master_status_structure* const pMStat, const CdMapTbl_File moduleFileId, const bool bOpenModuleFile) noexcept {    
    // Some very basic initialization
    *gbWess_seq_loader_enable = false;
    *gWess_seqld_moduleFileId = moduleFileId;
    *gpWess_seqld_mstat = pMStat;

    // If there is no master stat then this fails
    if (!pMStat)
        return false;

    // Save the number of sequences in the module
    *gbWess_seq_loader_enable = true;
    *gWess_num_sequences = pMStat->pmod_info->mod_hdr.sequences;

    // TODO: COMMENT ON THIS
    track_header& baseTrackHdr = *gWess_seqld_baseTrackHdr;
    baseTrackHdr.priority = 128;
    baseTrackHdr.initvolume_cntrl = 127;
    baseTrackHdr.initpan_cntrl = 64;
    baseTrackHdr.initppq = 120;
    baseTrackHdr.initqpm = 120;
    baseTrackHdr.voices_type = SoundDriverId::NoSound_ID;
    baseTrackHdr.voices_max = 0;
    baseTrackHdr.reverb = 0;
    baseTrackHdr.voices_class = SoundClass::SNDFX_CLASS;
    baseTrackHdr.initpatchnum = 0;
    baseTrackHdr.initpitch_cntrl = 0;
    baseTrackHdr.substack_count = 0;
    baseTrackHdr.mutebits = 0;
    baseTrackHdr.labellist_count = 0;
    baseTrackHdr.data_size = 2;
    
    // If requested, pre-open the module file also to have it ready
    if (bOpenModuleFile) {
        if (!open_sequence_data()) {
            wess_seq_load_err(SEQLOAD_FOPEN);
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the sequence loader
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_loader_exit() noexcept {
    close_sequence_data();
    *gbWess_seq_loader_enable = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns how many bytes still need to be loaded for the given sequence, which will be '0' when the sequence is loaded.
// Returns '0' also if the sequence loader is not initialized or the sequence number invalid.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_sizeof(const int32_t seqIdx) noexcept {
    if ((*gbWess_seq_loader_enable) && Is_Seq_Seq_Num_Valid(seqIdx)) {
        master_status_structure& mstat = *gpWess_seqld_mstat->get();
        sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqIdx];
        return (seqInfo.ptrk_info) ? 0 : seqInfo.trkinfolength;
    }

    return 0;
}

void wess_seq_load() noexcept {
loc_800451F4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    if (v0 == 0) goto loc_80045278;
    v0 = Is_Seq_Seq_Num_Valid(a0);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80045238;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_8004527C;
loc_80045238:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_8004527C;
    }
    a0 = s0;
    a1 = s2;
    load_sequence_data();
    s1 = v0;
loc_80045278:
    v0 = s1;
loc_8004527C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_free() noexcept {
loc_80045298:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    if (v0 == 0) goto loc_8004530C;
    v0 = Is_Seq_Seq_Num_Valid(a0);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_800452D4;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80045310;
loc_800452D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x4);
    {
        const bool bJump = (v0 == 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_80045310;
    }
    sw(0, v1 + 0x4);
    s1 = 1;                                             // Result = 00000001
loc_8004530C:
    v0 = s1;
loc_80045310:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

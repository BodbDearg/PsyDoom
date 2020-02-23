#include "LIBGPU.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "LIBETC.h"
#include "PcPsx/Endian.h"
#include "PcPsx/Finally.h"
#include "PsxVm/VmSVal.h"

#include <device/gpu/gpu.h>

// N.B: needs to happen AFTER Avocado includes due to clashes caused by the MIPS register macros
#include "PsxVm/PsxVm.h"

void LIBGPU_ResetGraph() noexcept {
loc_8004BCC8:
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = 0x80080000;                                    // Result = 80080000
    s2 += 0x356;                                        // Result = 80080356
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(s2);                                       // Load from: 80080356
    v0 = (v0 < 2);
    s1 = a0;
    if (v0 != 0) goto loc_8004BD14;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B28;                                       // Result = STR_Sys_ResetGraph_Msg1[0] (80011B28)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s1;
    ptr_call(v0);
loc_8004BD14:
    v0 = s1 & 3;
    s0 = s2 - 2;                                        // Result = 80080354
    if (v0 == 0) goto loc_8004BD38;
    {
        const bool bJump = (i32(v0) < 0);
        v0 = (i32(v0) < 4);
        if (bJump) goto loc_8004BEA4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004BED4;
    }
    goto loc_8004BE80;
loc_8004BD38:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B3C;                                       // Result = STR_Sys_ResetGraph_Msg2[0] (80011B3C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5D14;                                       // Result = gLIBGPU_SYS_driver_table[0] (80075D14)
    a2 = s0;                                            // Result = 80080354
    LIBC2_printf();
    a0 = s0;                                            // Result = 80080354
    a1 = 0;                                             // Result = 00000000
    a2 = 0x80;                                          // Result = 00000080
    LIBGPU_SYS_memset();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at + 0x355);                                 // Store to: 80080355
    LIBETC_ResetCallback();
    v0 = 0xFF0000;                                      // Result = 00FF0000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D54);                               // Load from: gpLIBGPU_SYS_driver_table (80075D54)
    v0 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 &= v0;
    LIBAPI_GPU_cw();
    a0 = 0;
    LIBGPU_SYS__reset();
    a0 = v0;
    v0 = a0;
    v1 = v0 & 0xFF;
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 2);
        if (bJump) goto loc_8004BDF0;
    }
    if (v0 == 0) goto loc_8004BDDC;
    v0 = 0x400;                                         // Result = 00000400
    if (v1 == 0) goto loc_8004BE64;
    goto loc_8004BEAC;
loc_8004BDDC:
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 == v0);
        v0 = s1 & 8;
        if (bJump) goto loc_8004BE1C;
    }
    goto loc_8004BEAC;
loc_8004BDF0:
    v0 = s1 & 8;
    {
        const bool bJump = (v0 == 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8004BE00;
    }
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
loc_8004BE00:
    v0 = 0x400;                                         // Result = 00000400
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    goto loc_8004BEAC;
loc_8004BE1C:
    {
        const bool bJump = (v0 == 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8004BE60;
    }
    a0 = 0x9000000;                                     // Result = 09000000
    sb(v0, s2 - 0x2);                                   // Store to: 80080354
    v0 = 0x400;                                         // Result = 00000400
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    a0 |= 1;                                            // Result = 09000001
    LIBGPU_SYS__ctl();
    goto loc_8004BEAC;
loc_8004BE60:
    v0 = 0x400;                                         // Result = 00000400
loc_8004BE64:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x358);                                 // Store to: 80080358
    v0 = 0x200;                                         // Result = 00000200
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x35A);                                 // Store to: 8008035A
    goto loc_8004BEAC;
loc_8004BE80:
    a0 = 1;
    LIBGPU_SYS__reset();
    goto loc_8004BEAC;
loc_8004BEA4:
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004BED4;
loc_8004BEAC:
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x364;                                        // Result = 80080364
    a0 = s0;                                            // Result = 80080364
    a1 = -1;                                            // Result = FFFFFFFF
    a2 = 0x5C;                                          // Result = 0000005C
    LIBGPU_SYS_memset();
    a0 = s0 + 0x5C;                                     // Result = gLIBGPU_SYS_p0_82[3] (800803C0)
    a1 = -1;                                            // Result = FFFFFFFF
    a2 = 0x14;                                          // Result = 00000014
    LIBGPU_SYS_memset();
loc_8004BED4:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBGPU_SetGraphDebug() noexcept {
loc_8004C004:
    sp -= 0x18;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x356;                                        // Result = 80080356
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lbu(v1);                                       // Load from: 80080356
    sb(a0, v1);                                         // Store to: 80080356
    a0 &= 0xFF;
    v0 = s0;
    if (a0 == 0) goto loc_8004C05C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = lbu(v1);                                       // Load from: 80080356
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lbu(a2 + 0x354);                               // Load from: 80080354
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lbu(a3 + 0x357);                               // Load from: 80080357
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1B74;                                       // Result = STR_Sys_SetDebug_Msg[0] (80011B74)
    ptr_call(v0);
    v0 = s0;
loc_8004C05C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_GetGraphType() noexcept {
loc_8004C11C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x354);                               // Load from: 80080354
    return;
}

void LIBGPU_DrawSyncCallback() noexcept {
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x356);                               // Load from: 80080356
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (v0 < 2);
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_8004C174;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1BB4;                                       // Result = STR_Sys_DrawSyncCallBack_Msg[0] (80011BB4)
    a1 = s0;
    ptr_call(v0);
loc_8004C174:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x360);                                // Load from: 80080360
    at = 0x80080000;                                    // Result = 80080000
    sw(s0, at + 0x360);                                 // Store to: 80080360
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SetDispMask() noexcept {
loc_8004C198:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x356);                               // Load from: 80080356
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (v0 < 2);
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_8004C1D4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1BD0;                                       // Result = STR_Sys_SetDisplayMask_Msg[0] (80011BD0)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s0;
    ptr_call(v0);
loc_8004C1D4:
    a0 = 0x3000000;                                     // Result = 03000000
    a0 |= 1;                                            // Result = 03000001
    if (s0 == 0) goto loc_8004C1EC;
    a0 = 0x3000000;                                     // Result = 03000000
loc_8004C1EC:
    LIBGPU_SYS__ctl();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for all GPU operations to complete or return the amount left.
//
// Mode:
//  0 = Block until all drawing operations are complete
//  1 = Return the number of drawing operations currently in progress.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBGPU_DrawSync([[maybe_unused]] const int32_t mode) noexcept {
    // This function doesn't need to do anything in this emulated environment.
    // When we submit something to the 'gpu' then Avocado executes it immediately and blocks before returning.
    return 0;
}

void _thunk_LIBGPU_DrawSync() noexcept {
    v0 = LIBGPU_DrawSync(a0);
}

void LIBGPU_checkRECT() noexcept {
loc_8004C27C:
    sp -= 0x20;
    t0 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 + 0x356);                               // Load from: 80080356
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x1C);
    if (v1 == v0) goto loc_8004C2B4;
    v0 = 2;                                             // Result = 00000002
    if (v1 == v0) goto loc_8004C34C;
    goto loc_8004C390;
loc_8004C2B4:
    a1 = lh(s0 + 0x4);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 + 0x358);                                // Load from: 80080358
    v0 = (i32(v1) < i32(a1));
    if (v0 != 0) goto loc_8004C33C;
    a3 = lh(s0);
    v0 = a1 + a3;
    v0 = (i32(v1) < i32(v0));
    if (v0 != 0) goto loc_8004C33C;
    v1 = lh(s0 + 0x2);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lh(a0 + 0x35A);                                // Load from: 8008035A
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_8004C33C;
    a2 = lh(s0 + 0x6);
    v0 = v1 + a2;
    v0 = (i32(a0) < i32(v0));
    if (v0 != 0) goto loc_8004C33C;
    if (i32(a1) <= 0) goto loc_8004C33C;
    if (i32(a3) < 0) goto loc_8004C33C;
    if (i32(v1) < 0) goto loc_8004C33C;
    if (i32(a2) > 0) goto loc_8004C390;
loc_8004C33C:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1BF8;                                       // Result = STR_Sys_BadRect_Err[0] (80011BF8)
    goto loc_8004C354;
loc_8004C34C:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1C18;                                       // Result = STR_Sys_ClearImage_Msg[0] (80011C18)
loc_8004C354:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = t0;
    ptr_call(v0);
    a1 = lh(s0);
    a2 = lh(s0 + 0x2);
    a3 = lh(s0 + 0x4);
    v0 = lh(s0 + 0x6);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1C04;                                       // Result = STR_Sys_BadRect_Err[C] (80011C04)
    sw(v0, sp + 0x10);
    ptr_call(v1);
loc_8004C390:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Upload the given image data to the given area in VRAM.
// The image data is expected to be 32-bit aligned and in multiples of 32-bits.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_LoadImage(const RECT& dstRect, const uint32_t* const pImageData) noexcept {
    // Copy the rect onto the VM stack so that invoked functions can get at it.
    // The given rect might exist outside of the PlayStation's memory space and only exist in the host app memory space:
    VmSVal<RECT> dstRectCopy(dstRect);
    
    // This sanity checks the rect
    a0 = 0x80011C28;                                    // Result = STR_Sys_LoadImage_Msg[0] (80011C28)
    a1 = dstRectCopy.addr();
    LIBGPU_checkRECT();

    // Get the low level GPU function table
    v0 = lw(0x80075D54);                                // Load from: gpLIBGPU_SYS_driver_table (80075D54)

    // Do the upload to PSX RAM.
    // TODO: make this just load to the PSX VRAM directly.
    a0 = lw(v0 + 0x20);         // LIBGPU_SYS__dws
    a1 = dstRectCopy.addr();    
    a2 = 8;
    a3 = ptrToVmAddr(pImageData);
    LIBGPU_SYS__addque2();
}

void _thunk_LIBGPU_LoadImage() noexcept {
    LIBGPU_LoadImage(*vmAddrToPtr<RECT>(a0), vmAddrToPtr<const uint32_t>(a1));
}

int32_t LIBGPU_MoveImage(const RECT& src, const int32_t dstX, const int32_t dstY) noexcept {
loc_8004C500:
    VmSVal<RECT> srcVmStack = src;

    a0 = srcVmStack.addr();
    a1 = dstX;
    a2 = dstY;

    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s0, sp + 0x10);
    s0 = a2;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1C40;                                       // Result = STR_Sys_MoveImage_Msg[0] (80011C40)
    sw(ra, sp + 0x1C);
    a1 = s2;
    LIBGPU_checkRECT();
    s0 <<= 16;
    s1 &= 0xFFFF;
    s0 |= s1;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5D64;                                       // Result = 80075D64
    v0 = lw(s2);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D54);                               // Load from: gpLIBGPU_SYS_driver_table (80075D54)
    a2 = 0x14;                                          // Result = 00000014
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5D68);                                // Store to: 80075D68
    sw(v0, a1);                                         // Store to: 80075D64
    v0 = lw(s2 + 0x4);
    a3 = 0;                                             // Result = 00000000
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5D6C);                                // Store to: 80075D6C
    a0 = lw(v1 + 0x18); // LIBGPU_SYS__cwc
    a1 -= 8;                                            // Result = 80075D5C
    LIBGPU_SYS__addque2();

    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return v0;
}

void _thunk_LIBGPU_MoveImage() noexcept {
    v0 = LIBGPU_MoveImage(*vmAddrToPtr<const RECT>(a0), a1, a2);
}

void LIBGPU_DrawOTag() noexcept {
loc_8004C72C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x356);                               // Load from: 80080356
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (v0 < 2);
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_8004C768;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1C7C;                                       // Result = STR_Sys_DrawOTag_Msg[0] (80011C7C)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s0;
    ptr_call(v0);
loc_8004C768:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D54);                               // Load from: gpLIBGPU_SYS_driver_table (80075D54)
    a1 = s0;
    a0 = lw(v0 + 0x18);     // LIBGPU_SYS__cwc
    a2 = 0;                                             // Result = 00000000
    LIBGPU_SYS__addque();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the drawing environment and potentially clears the screen also.
// The draw environment includes the display area, texture page and texture window settings, blending settings and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
DRAWENV& LIBGPU_PutDrawEnv(DRAWENV& env) noexcept {
    gpu::GPU& gpu = *PsxVm::gpGpu;
    
    // Set drawing area and offset
    gpu.drawingArea.left = env.clip.x;
    gpu.drawingArea.top = env.clip.y;
    gpu.drawingArea.right = env.clip.x + env.clip.w - 1;
    gpu.drawingArea.bottom = env.clip.y + env.clip.h - 1;
    
    gpu.drawingOffsetX = env.ofs[0];
    gpu.drawingOffsetY = env.ofs[1];
    
    // Set the texture window offset and mask.
    // Note that the units are specified in multiples of 8 for the GPU.
    gpu.gp0_e2.offsetX = env.tw.x / 8;
    gpu.gp0_e2.offsetY = env.tw.y / 8;

    if (env.tw.w == 0) {
        gpu.gp0_e2.maskX = 0;
    } else if (env.tw.w > 1) {
        gpu.gp0_e2.maskX = (env.tw.w / 8) - 1;
    } else {
        gpu.gp0_e2.maskX = 1;
    }
    
    if (env.tw.h == 0) {
        gpu.gp0_e2.maskY = 0;
    } else if (env.tw.h > 1) {
        gpu.gp0_e2.maskY = (env.tw.h / 8) - 1;
    } else {
        gpu.gp0_e2.maskY = 1;
    }

    // Set texture page position and bit rate
    gpu.gp0_e1.texturePageBaseX = (env.tpage & 0x0F) >> 0;      // This is multiples of 64
    gpu.gp0_e1.texturePageBaseY = (env.tpage & 0x10) >> 4;      // This is multiples of 256
    
    switch ((env.tpage >> 7) & 0x3) {
        case 0: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit4;
        case 1: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit8;
        case 2: gpu.gp0_e1.texturePageColors = gpu::GP0_E1::TexturePageColors::bit15;
        default: break;
    }

    // Set transparency mode
    switch ((env.tpage >> 5) & 0x3) {
        case 0: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::Bby2plusFby2;
        case 1: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BplusF;
        case 2: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BminusF;
        case 3: gpu.gp0_e1.semiTransparency = gpu::SemiTransparency::BplusFby4;
        default: break;
    }

    // Set dithering and draw to display area flags
    gpu.gp0_e1.dither24to15 = (env.dtd != 0);
    gpu.gp0_e1.drawingToDisplayArea = (env.dfe != 0) ?
        gpu::GP0_E1::DrawingToDisplayArea::allowed :
        gpu::GP0_E1::DrawingToDisplayArea::prohibited;

    // Clear the screen if specified by the DRAWENV.
    // Fill the draw area with the specified background color.
    if (env.isbg) {
        RGB rgb = {};
        rgb.r = env.r0;
        rgb.g = env.g0;
        rgb.b = env.b0;

        const int32_t drawW = (gpu.drawingArea.right - gpu.drawingArea.left + 1) & 0xFFFF;
        const int32_t drawH = (gpu.drawingArea.bottom - gpu.drawingArea.top + 1) & 0xFFFF;

        gpu.arguments[0] = rgb.raw;
        gpu.arguments[1] = gpu.drawingArea.left | (gpu.drawingArea.top << 16);
        gpu.arguments[2] = drawW | (drawH << 16);

        gpu.cmdFillRectangle();
    }

    return env;
}

DISPENV& LIBGPU_PutDispEnv(DISPENV& env) noexcept {
    // TODO: this is a temporary measure
    VmSVal<DISPENV> envStack = env;
    auto convertOnExit = finally([&](){ env = *envStack; });

    a0 = envStack.addr();

loc_8004C898:
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x356;                                        // Result = 80080356
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lbu(v0);                                       // Load from: 80080356
    v0 = (v0 < 2);
    s2 = 0x8000000;                                     // Result = 08000000
    if (v0 != 0) goto loc_8004C8EC;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1CA8;                                       // Result = STR_Sys_PutDisplayEnv_Msg[0] (80011CA8)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D58);                               // Load from: gpLIBGPU_GPU_printf (80075D58)
    a1 = s0;
    ptr_call(v0);
loc_8004C8EC:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x354);                               // Load from: 80080354
    v0--;
    v0 = (v0 < 2);
    if (v0 == 0) goto loc_8004C92C;
    a0 = s0;
    LIBGPU_SYS_get_dx();
    v1 = lhu(s0 + 0x2);
    v0 &= 0xFFF;
    v1 &= 0xFFF;
    v1 <<= 12;
    v1 |= v0;
    v0 = 0x5000000;                                     // Result = 05000000
    goto loc_8004C948;
loc_8004C92C:
    v0 = lhu(s0 + 0x2);
    v1 = lhu(s0);
    v0 &= 0x3FF;
    v0 <<= 10;
    v1 &= 0x3FF;
    v0 |= v1;
    v1 = 0x5000000;                                     // Result = 05000000
loc_8004C948:
    a0 = v0 | v1;    
    LIBGPU_SYS__ctl();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x3C8);                                // Load from: gLIBGPU_SYS_p0_82[5] (800803C8)
    v0 = lw(s0 + 0x8);
    if (v1 != v0) goto loc_8004C998;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x3CC);                                // Load from: gLIBGPU_SYS_p0_82[6] (800803CC)
    v0 = lw(s0 + 0xC);
    if (v1 == v0) goto loc_8004CB54;
loc_8004C998:
    LIBETC_GetVideoMode();
    v1 = lh(s0 + 0x8);
    sb(v0, s0 + 0x12);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    a1 = v0 + 0x260;
    v0 = lbu(s0 + 0x12);
    a0 = lh(s0 + 0xA);
    s1 = a0 + 0x13;
    if (v0 != 0) goto loc_8004C9CC;
    s1 = a0 + 0x10;
loc_8004C9CC:
    v1 = lh(s0 + 0xC);
    v0 = v1 << 2;
    if (v1 == 0) goto loc_8004C9EC;
    v0 += v1;
    v0 <<= 1;
    a2 = a1 + v0;
    goto loc_8004C9F0;
loc_8004C9EC:
    a2 = a1 + 0xA00;
loc_8004C9F0:
    v0 = lh(s0 + 0xE);
    s3 = s1 + v0;
    if (v0 != 0) goto loc_8004CA04;
    s3 = s1 + 0xF0;
loc_8004CA04:
    v0 = (i32(a1) < 0x1F4);
    a0 = 0x1F4;                                         // Result = 000001F4
    if (v0 != 0) goto loc_8004CA24;
    a0 = a1;
    v0 = (i32(a0) < 0xCDB);
    a1 = a0;
    if (v0 != 0) goto loc_8004CA28;
    a0 = 0xCDA;                                         // Result = 00000CDA
loc_8004CA24:
    a1 = a0;
loc_8004CA28:
    v1 = a1 + 0x50;
    v0 = (i32(a2) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a2) < 0xCDB);
        if (bJump) goto loc_8004CA44;
    }
    v1 = a2;
    if (v0 != 0) goto loc_8004CA44;
    v1 = 0xCDA;                                         // Result = 00000CDA
loc_8004CA44:
    a2 = v1;
    if (i32(s1) < 0) goto loc_8004CA98;
    v0 = lbu(s0 + 0x12);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s1) < 0x137);
        if (bJump) goto loc_8004CA6C;
    }
    if (v0 == 0) goto loc_8004CA78;
    a0 = s1;
    goto loc_8004CA9C;
loc_8004CA6C:
    v0 = (i32(s1) < 0xFF);
    if (v0 != 0) goto loc_8004CA90;
loc_8004CA78:
    v0 = lbu(s0 + 0x12);
    a0 = 0xFE;                                          // Result = 000000FE
    if (v0 == 0) goto loc_8004CA9C;
    a0 = 0x136;                                         // Result = 00000136
    goto loc_8004CA9C;
loc_8004CA90:
    a0 = s1;
    goto loc_8004CA9C;
loc_8004CA98:
    a0 = 0;                                             // Result = 00000000
loc_8004CA9C:
    s1 = a0;
    v1 = s1 + 1;
    v0 = (i32(s3) < i32(v1));
    if (v0 != 0) goto loc_8004CAF8;
    v0 = lbu(s0 + 0x12);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < 0x139);
        if (bJump) goto loc_8004CAD0;
    }
    if (v0 == 0) goto loc_8004CADC;
    v1 = s3;
    goto loc_8004CAF8;
loc_8004CAD0:
    v0 = (i32(s3) < 0x101);
    if (v0 != 0) goto loc_8004CAF4;
loc_8004CADC:
    v0 = lbu(s0 + 0x12);
    v1 = 0x100;                                         // Result = 00000100
    if (v0 == 0) goto loc_8004CAF8;
    v1 = 0x138;                                         // Result = 00000138
    goto loc_8004CAF8;
loc_8004CAF4:
    v1 = s3;
loc_8004CAF8:
    s3 = v1;
    v0 = a2 & 0xFFF;                                    // Result = 00000244
    v0 <<= 12;                                          // Result = 00244000
    a0 = a1 & 0xFFF;                                    // Result = 000001F4
    v1 = 0x6000000;                                     // Result = 06000000
    a0 |= v1;                                           // Result = 060001F4
    a0 |= v0;                                           // Result = 062441F4
    LIBGPU_SYS__ctl();

    v0 = s3 & 0x3FF;
    v0 <<= 10;
    a0 = s1 & 0x3FF;
    v1 = 0x7000000;                                     // Result = 07000000
    a0 |= v1;
    a0 |= v0;
    LIBGPU_SYS__ctl();
loc_8004CB54:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x3D0);                                // Load from: gLIBGPU_SYS_p0_82[7] (800803D0)
    v0 = lw(s0 + 0x10);
    if (v1 != v0) goto loc_8004CB9C;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x3C0);                                // Load from: gLIBGPU_SYS_p0_82[3] (800803C0)
    v0 = lw(s0);
    if (v1 != v0) goto loc_8004CB9C;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x3C4);                                // Load from: gLIBGPU_SYS_p0_82[4] (800803C4)
    v0 = lw(s0 + 0x4);
    if (v1 == v0) goto loc_8004CC84;
loc_8004CB9C:
    LIBETC_GetVideoMode();
    sb(v0, s0 + 0x12);
    v1 = lbu(s0 + 0x12);
    v0 = 1;                                             // Result = 00000001
    if (v1 != v0) goto loc_8004CBBC;
    s2 |= 8;                                            // Result = 08000008
loc_8004CBBC:
    v0 = lbu(s0 + 0x11);
    if (v0 == 0) goto loc_8004CBD0;
    s2 |= 0x10;
loc_8004CBD0:
    v0 = lbu(s0 + 0x10);
    if (v0 == 0) goto loc_8004CBE4;
    s2 |= 0x20;
loc_8004CBE4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x357;                                        // Result = 80080357
    v0 = lbu(v0);                                       // Load from: 80080357
    if (v0 == 0) goto loc_8004CC00;
    s2 |= 0x80;
loc_8004CC00:
    v1 = lh(s0 + 0x4);
    v0 = (i32(v1) < 0x119);
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x161);
        if (bJump) goto loc_8004CC48;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x191);
        if (bJump) goto loc_8004CC24;
    }
    s2 |= 1;
    goto loc_8004CC48;
loc_8004CC24:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x231);
        if (bJump) goto loc_8004CC34;
    }
    s2 |= 0x40;
    goto loc_8004CC48;
loc_8004CC34:
    if (v0 == 0) goto loc_8004CC44;
    s2 |= 2;
    goto loc_8004CC48;
loc_8004CC44:
    s2 |= 3;
loc_8004CC48:
    v0 = lbu(s0 + 0x12);
    v1 = lh(s0 + 0x6);
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x121);
        if (bJump) goto loc_8004CC5C;
    }
    v0 = (i32(v1) < 0x101);
loc_8004CC5C:
    if (v0 != 0) goto loc_8004CC68;
    s2 |= 0x24;
loc_8004CC68:
    a0 = s2;
    LIBGPU_SYS__ctl();
loc_8004CC84:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x3C0;                                        // Result = gLIBGPU_SYS_p0_82[3] (800803C0)
    a1 = s0;
    a2 = 0x14;                                          // Result = 00000014
    LIBC2_memcpy();
    v0 = s0;
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;

    return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a draw primitive that sets the current texture window.
// Use the specified RECT as the window.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetTexWindow(DR_TWIN& prim, const RECT& texWin) noexcept {
    LIBGPU_setlen(prim, 2);
    prim.code[0] = LIBGPU_SYS_get_tw(&texWin);
    prim.code[1] = 0;
}

void _thunk_LIBGPU_SetTexWindow() noexcept {
    LIBGPU_SetTexWindow(*vmAddrToPtr<DR_TWIN>(a0), *vmAddrToPtr<RECT>(a1));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates a 'set draw mode' primitive with the specified settings.
// Note that the texture window is optional and if not specified then the current window will be used.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetDrawMode(
    DR_MODE& modePrim,
    const bool bCanDrawInDisplayArea,
    const bool bDitheringOn,
    const uint32_t texPageId,
    const RECT* const pNewTexWindow
) noexcept {
    // Set primitive size
    modePrim.tag &= 0x00FFFFFF;
    modePrim.tag |= uint32_t(2) << 24;

    // Set draw mode
    a0 = bCanDrawInDisplayArea;
    a1 = bDitheringOn;
    a2 = texPageId & 0xFFFF;
    LIBGPU_SYS_get_mode();
    modePrim.code[0] = v0;

    // Set a texture window if given, or use the existing one if not given
    if (pNewTexWindow) {
        // Copy the RECT given to the stack so it's accessible to the PSX
        VmSVal<RECT> newTexWin(*pNewTexWindow);
        a0 = newTexWin.addr();
        _thunk_LIBGPU_SYS_get_tw();
    } else {
        a0 = 0;
        _thunk_LIBGPU_SYS_get_tw();
    }

    modePrim.code[1] = v0;
}

void _thunk_LIBGPU_SetDrawMode() noexcept {
    LIBGPU_SetDrawMode(*vmAddrToPtr<DR_MODE>(a0), a1, a2, a3, vmAddrToPtr<const RECT>(lw(sp + 0x10)));
}

void LIBGPU_SYS_get_mode() noexcept {
loc_8004D158:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x354;                                        // Result = 80080354
    v0 = lbu(v0);                                       // Load from: 80080354
    v0--;
    v0 = (v0 < 2);
    if (v0 == 0) goto loc_8004D194;
    v1 = 0xE1000000;                                    // Result = E1000000
    if (a1 == 0) goto loc_8004D184;
    v1 |= 0x800;                                        // Result = E1000800
loc_8004D184:
    v0 = a2 & 0x27FF;
    if (a0 == 0) goto loc_8004D1AC;
    v0 |= 0x1000;
    goto loc_8004D1AC;
loc_8004D194:
    v1 = 0xE1000000;                                    // Result = E1000000
    if (a1 == 0) goto loc_8004D1A0;
    v1 |= 0x200;                                        // Result = E1000200
loc_8004D1A0:
    v0 = a2 & 0x9FF;
    if (a0 == 0) goto loc_8004D1AC;
    v0 |= 0x400;
loc_8004D1AC:
    v0 |= v1;
    return;
}

void LIBGPU_SYS_get_cs() noexcept {
loc_8004D1B4:
    a2 = a0;
    a0 <<= 16;
    v1 = u32(i32(a0) >> 16);
    if (i32(v1) < 0) goto loc_8004D1F8;
    a3 = 0x80080000;                                    // Result = 80080000
    a3 += 0x358;                                        // Result = 80080358
    v0 = lhu(a3);                                       // Load from: 80080358
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(v1));
    a0 = a2;
    if (v0 == 0) goto loc_8004D1FC;
    a0 = lhu(a3);                                       // Load from: 80080358
    v0 = a1 << 16;
    goto loc_8004D200;
loc_8004D1F8:
    a0 = 0;                                             // Result = 00000000
loc_8004D1FC:
    v0 = a1 << 16;
loc_8004D200:
    v1 = u32(i32(v0) >> 16);
    a2 = a0;
    if (i32(v1) < 0) goto loc_8004D23C;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x35A;                                        // Result = 8008035A
    v0 = lhu(a0);                                       // Load from: 8008035A
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8004D240;
    a1 = lhu(a0);                                       // Load from: 8008035A
    goto loc_8004D240;
loc_8004D23C:
    a1 = 0;                                             // Result = 00000000
loc_8004D240:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x354;                                        // Result = 80080354
    v0 = lbu(v0);                                       // Load from: 80080354
    v0--;
    v0 = (v0 < 2);
    v1 = a1 & 0xFFF;
    if (v0 != 0) goto loc_8004D270;
    v1 = a1 & 0x3FF;
    v1 <<= 10;
    v0 = a2 & 0x3FF;                                    // Result = 00000000
    goto loc_8004D278;
loc_8004D270:
    v1 <<= 12;
    v0 = a2 & 0xFFF;                                    // Result = 00000000
loc_8004D278:
    a0 = 0xE3000000;                                    // Result = E3000000
    v0 |= a0;                                           // Result = E3000000
    v0 |= v1;
    return;
}

void LIBGPU_SYS_get_ce() noexcept {
loc_8004D288:
    a2 = a0;
    a0 <<= 16;
    v1 = u32(i32(a0) >> 16);
    if (i32(v1) < 0) goto loc_8004D2CC;
    a3 = 0x80080000;                                    // Result = 80080000
    a3 += 0x358;                                        // Result = 80080358
    v0 = lhu(a3);                                       // Load from: 80080358
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(v1));
    a0 = a2;
    if (v0 == 0) goto loc_8004D2D0;
    a0 = lhu(a3);                                       // Load from: 80080358
    v0 = a1 << 16;
    goto loc_8004D2D4;
loc_8004D2CC:
    a0 = 0;                                             // Result = 00000000
loc_8004D2D0:
    v0 = a1 << 16;
loc_8004D2D4:
    v1 = u32(i32(v0) >> 16);
    a2 = a0;
    if (i32(v1) < 0) goto loc_8004D310;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x35A;                                        // Result = 8008035A
    v0 = lhu(a0);                                       // Load from: 8008035A
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8004D314;
    a1 = lhu(a0);                                       // Load from: 8008035A
    goto loc_8004D314;
loc_8004D310:
    a1 = 0;                                             // Result = 00000000
loc_8004D314:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x354;                                        // Result = 80080354
    v0 = lbu(v0);                                       // Load from: 80080354
    v0--;
    v0 = (v0 < 2);
    v1 = a1 & 0xFFF;
    if (v0 != 0) goto loc_8004D344;
    v1 = a1 & 0x3FF;
    v1 <<= 10;
    v0 = a2 & 0x3FF;                                    // Result = 00000000
    goto loc_8004D34C;
loc_8004D344:
    v1 <<= 12;
    v0 = a2 & 0xFFF;                                    // Result = 00000000
loc_8004D34C:
    a0 = 0xE4000000;                                    // Result = E4000000
    v0 |= a0;                                           // Result = E4000000
    v0 |= v1;
    return;
}

void LIBGPU_SYS_get_ofs() noexcept {
loc_8004D35C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x354;                                        // Result = 80080354
    v0 = lbu(v0);                                       // Load from: 80080354
    v0--;
    v0 = (v0 < 2);
    v1 = a1 & 0xFFF;
    if (v0 != 0) goto loc_8004D38C;
    v1 = a1 & 0x7FF;
    v1 <<= 11;
    v0 = a0 & 0x7FF;
    goto loc_8004D394;
loc_8004D38C:
    v1 <<= 12;
    v0 = a0 & 0xFFF;
loc_8004D394:
    a0 = 0xE5000000;                                    // Result = E5000000
    v0 |= a0;
    v0 |= v1;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal (non-exposed) PSY-Q SDK Function:
// Encodes the given rect into a 32-bit texture window setting used by the hardware.
// For more details see "GP0(E2h) - Texture Window setting":
//      https://problemkaputt.de/psx-spx.htm#gpudisplaycontrolcommandsgp1
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t LIBGPU_SYS_get_tw(const RECT* const pRect) noexcept {
    // If no rect then return an invalid encoding
    if (!pRect)
        return 0;
    
    // Encode the texture window using 5 bits for each piece of info
    constexpr uint32_t CMD_HEADER = 0xE2000000;
    const uint32_t twOffsetX = (uint16_t)(pRect->x & 0xFF) >> 3;
    const uint32_t twOffsetY = (uint16_t)(pRect->y & 0xFF) >> 3;
    const uint32_t twMaskX = (uint16_t)(-pRect->w & 0xFF) >> 3;
    const uint32_t twMaskY = (uint16_t)(-pRect->h & 0xFF) >> 3;

    return (CMD_HEADER | (twOffsetY << 15) | (twOffsetX << 10) | (twMaskY << 5) | twMaskX);
}

void _thunk_LIBGPU_SYS_get_tw() noexcept {
    v0 = LIBGPU_SYS_get_tw(vmAddrToPtr<const RECT>(a0));
}

void LIBGPU_SYS_get_dx() noexcept {
loc_8004D428:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x354;                                        // Result = 80080354
    v0 = lbu(v0);                                       // Load from: 80080354
    v1 = v0 & 0xFF;
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 == v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8004D458;
    }
    if (v1 == v0) goto loc_8004D480;
    goto loc_8004D4D8;
loc_8004D458:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x357);                               // Load from: 80080357
    {
        const bool bJump = (v0 == 0);
        v0 = 0x400;                                     // Result = 00000400
        if (bJump) goto loc_8004D4D8;
    }
    v1 = lh(a0 + 0x4);
    a0 = lh(a0);
loc_8004D474:
    v0 -= v1;
    v0 -= a0;
    goto loc_8004D4DC;
loc_8004D480:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 + 0x357);                               // Load from: 80080357
    if (v0 == 0) goto loc_8004D4B8;
    v0 = lhu(a0 + 0x4);
    a0 = lh(a0);
    v0 <<= 16;
    v1 = u32(i32(v0) >> 16);
    v0 >>= 31;
    v1 += v0;
    v1 = u32(i32(v1) >> 1);
    v0 = 0x400;                                         // Result = 00000400
    goto loc_8004D474;
loc_8004D4B8:
    v0 = lhu(a0);
    v0 <<= 16;
    v1 = u32(i32(v0) >> 16);
    v0 >>= 31;
    v1 += v0;
    v0 = u32(i32(v1) >> 1);
    goto loc_8004D4DC;
loc_8004D4D8:
    v0 = lh(a0);
loc_8004D4DC:
    return;
}

void LIBGPU_SYS__dws() noexcept {
loc_8004D824:
    sp -= 0x38;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    LIBGPU_SYS_set_alarm();
    v0 = lhu(s0);
    sh(v0, sp + 0x10);
    v0 = lhu(s0 + 0x2);
    sh(v0, sp + 0x12);
    v1 = lhu(s0 + 0x4);
    sh(v1, sp + 0x14);
    v0 = lhu(s0 + 0x6);
    sh(v0, sp + 0x16);
    v0 = v1 << 16;
    a0 = u32(i32(v0) >> 16);
    s4 = 0;                                             // Result = 00000000
    if (i32(a0) < 0) goto loc_8004D8BC;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x358;                                        // Result = 80080358
    v0 = lhu(a1);                                       // Load from: 80080358
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(a0));
    if (v0 == 0) goto loc_8004D8C0;
    v1 = lhu(a1);                                       // Load from: 80080358
    goto loc_8004D8C0;
loc_8004D8BC:
    v1 = 0;                                             // Result = 00000000
loc_8004D8C0:
    a1 = lh(sp + 0x16);
    sh(v1, sp + 0x14);
    v1 = a1;
    if (i32(a1) < 0) goto loc_8004D900;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 += 0x35A;                                        // Result = 8008035A
    v0 = lhu(a2);                                       // Load from: 8008035A
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < i32(a1));
    a0 = v1;
    if (v0 == 0) goto loc_8004D904;
    a0 = lhu(a2);                                       // Load from: 8008035A
    v0 = a0 << 16;
    goto loc_8004D908;
loc_8004D900:
    a0 = 0;                                             // Result = 00000000
loc_8004D904:
    v0 = a0 << 16;
loc_8004D908:
    v1 = lh(sp + 0x14);
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    sh(a0, sp + 0x16);
    v0 = lo;
    v0++;
    v1 = v0 >> 31;
    v0 += v1;
    a0 = u32(i32(v0) >> 1);
    s0 = u32(i32(v0) >> 5);
    if (i32(a0) > 0) goto loc_8004D93C;
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004DA7C;
loc_8004D93C:
    v1 = s0;
    v0 = v1 << 4;
    s0 = a0 - v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    s3 = v1;
    v0 = lw(v0);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    a0 = 0xA0000000;                                    // Result = A0000000
    if (v0 != 0) goto loc_8004D99C;
    s2 = 0x4000000;                                     // Result = 04000000
loc_8004D96C:
    LIBGPU_SYS_get_alarm();
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004DA7C;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v0 &= s2;
    a0 = 0xA0000000;                                    // Result = A0000000
    if (v0 == 0) goto loc_8004D96C;
loc_8004D99C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = 0x4000000;                                     // Result = 04000000
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v0 = 0x1000000;                                     // Result = 01000000
    sw(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    if (s4 == 0) goto loc_8004D9D0;
    a0 = 0xB0000000;                                    // Result = B0000000
loc_8004D9D0:
    sw(a0, v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v0 = lw(sp + 0x10);
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v0 = lw(sp + 0x14);
    s0--;
    sw(v0, v1);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 == v0) goto loc_8004DA2C;
    a0 = -1;                                            // Result = FFFFFFFF
loc_8004DA0C:
    v1 = lw(s1);
    s1 += 4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    s0--;
    sw(v1, v0);
    if (s0 != a0) goto loc_8004DA0C;
loc_8004DA2C:
    v1 = 0x4000000;                                     // Result = 04000000
    if (s3 == 0) goto loc_8004DA78;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v1 |= 2;                                            // Result = 04000002
    sw(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D78);                               // Load from: 80075D78
    a0 = 0x1000000;                                     // Result = 01000000
    sw(s1, v0);
    v0 = s3 << 16;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D7C);                               // Load from: 80075D7C
    v0 |= 0x10;
    sw(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    a0 |= 0x201;                                        // Result = 01000201
    sw(a0, v0);
loc_8004DA78:
    v0 = 0;                                             // Result = 00000000
loc_8004DA7C:
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void LIBGPU_SYS__ctl() noexcept {
loc_8004DD64:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    sw(a0, v0);
    
    v0 = a0 >> 24;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x3D4;                                        // Result = gLIBGPU_SYS_ctlbuf[0] (800803D4)
    at += v0;
    sb(a0, at);
    return;
}

void LIBGPU_SYS__getctl() noexcept {
loc_8004DD90:
    at = 0x80080000;                                    // Result = 80080000
    at += 0x3D4;                                        // Result = gLIBGPU_SYS_ctlbuf[0] (800803D4)
    at += a0;
    v0 = lbu(at);
    return;
}

void LIBGPU_SYS__cwc() noexcept {
loc_8004DDF8:
    v1 = 0x4000000;                                     // Result = 04000000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v1 |= 2;                                            // Result = 04000002
    sw(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D78);                               // Load from: 80075D78
    sw(a0, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D7C);                               // Load from: 80075D7C
    v1 = 0x1000000;                                     // Result = 01000000
    sw(0, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v1 |= 0x401;                                        // Result = 01000401
    sw(v1, v0);
    return;
}

void LIBGPU_SYS__addque() noexcept {
loc_8004DE74:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a3 = a2;
    a2 = 0;                                             // Result = 00000000
    LIBGPU_SYS__addque2();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS__addque2() noexcept {
loc_8004DE98:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(s2, sp + 0x18);
    sw(ra, sp + 0x20);
    s2 = a3;
    LIBGPU_SYS_set_alarm();
    goto loc_8004DEFC;
loc_8004DECC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D9C);                               // Load from: 80075D9C
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5D9C);                                // Store to: 80075D9C
    LIBGPU_SYS_get_alarm();
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004E198;
    }
    LIBGPU_SYS__exeque();
loc_8004DEFC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0++;
    v0 &= 0x3F;
    if (v0 == v1) goto loc_8004DECC;
    a0 = 0;                                             // Result = 00000000
    LIBETC_SetIntrMask();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x35C;                                        // Result = 8008035C
    a0 = 1;                                             // Result = 00000001
    sw(a0, v1);                                         // Store to: 8008035C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 + 0x355);                               // Load from: 80080355
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x348);                                 // Store to: 80080348
    if (v1 != 0) goto loc_8004DF5C;
    a0 = 0;                                             // Result = 00000000
    LIBGPU_SYS__sync();
    a0 = s0;
    goto loc_8004DFE8;
loc_8004DF5C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    if (v1 != v0) goto loc_8004DFF8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v0 = lw(v0);
    v1 = 0x1000000;                                     // Result = 01000000
    v0 &= v1;
    if (v0 != 0) goto loc_8004DFF8;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x360);                                // Load from: 80080360
    if (v0 != 0) goto loc_8004DFF8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(a0);
    v1 = 0x10000000;                                    // Result = 10000000
    v0 &= v1;
    v1 = a0;
    if (v0 != 0) goto loc_8004DFE4;
    a0 = 0x10000000;                                    // Result = 10000000
loc_8004DFD0:
    v0 = lw(v1);
    v0 &= a0;
    if (v0 == 0) goto loc_8004DFD0;
loc_8004DFE4:
    a0 = s0;
loc_8004DFE8:
    a1 = s2;
    ptr_call(s3);
    goto loc_8004E184;
loc_8004DFF8:
    a1 = 0x80050000;                                    // Result = 80050000
    a1 -= 0x1E48;                                       // Result = LIBGPU_SYS__exeque (8004E1B8)
    a0 = 2;                                             // Result = 00000002
    LIBETC_DMACallback();
    a2 = 0;                                             // Result = 00000000
    if (s1 == 0) goto loc_8004E0BC;
    t0 = 0x80080000;                                    // Result = 80080000
    t0 += 0x4E0;                                        // Result = gLIBGPU_SYS__que[3] (800804E0)
    a3 = s0;
    v0 = s1;
loc_8004E020:
    if (i32(v0) >= 0) goto loc_8004E02C;
    v0 += 3;
loc_8004E02C:
    v0 = u32(i32(v0) >> 2);
    v0 = (i32(a2) < i32(v0));
    a0 = a2 << 2;
    if (v0 == 0) goto loc_8004E070;
    a1 = lw(a3);
    a3 += 4;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    a2++;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    v0 += t0;
    a0 += v0;
    sw(a1, a0);
    v0 = s1;
    goto loc_8004E020;
loc_8004E070:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    a0 = v0 << 1;
    a0 += v0;
    a0 <<= 5;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x4E0;                                        // Result = gLIBGPU_SYS__que[3] (800804E0)
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D8;                                        // Result = gLIBGPU_SYS__que[1] (800804D8)
    at += a0;
    sw(v0, at);
    goto loc_8004E0E4;
loc_8004E0BC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D8;                                        // Result = gLIBGPU_SYS__que[1] (800804D8)
    at += v0;
    sw(s0, at);
loc_8004E0E4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4DC;                                        // Result = gLIBGPU_SYS__que[2] (800804DC)
    at += v0;
    sw(s2, at);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += v0;
    sw(s3, at);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 + 0x348);                                // Load from: 80080348
    v0++;
    v0 &= 0x3F;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5D94);                                // Store to: gpLIBGPU_SYS__qin (80075D94)
    LIBETC_SetIntrMask();
    LIBGPU_SYS__exeque();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0 -= v1;
    v0 &= 0x3F;
    goto loc_8004E198;
loc_8004E184:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 + 0x348);                                // Load from: 80080348
    LIBETC_SetIntrMask();
    v0 = 0;                                             // Result = 00000000
loc_8004E198:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void LIBGPU_SYS__exeque() noexcept {
loc_8004E1B8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(v0);
    s0 = 0x1000000;                                     // Result = 01000000
    v0 &= s0;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004E468;
    }
    a0 = 0;                                             // Result = 00000000
    LIBETC_SetIntrMask();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x34C);                                 // Store to: 8008034C
    if (a0 == v1) goto loc_8004E410;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v0 = lw(v0);
    v0 &= s0;
    if (v0 != 0) goto loc_8004E410;
loc_8004E228:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += v1;
    v0 = lw(at);
    if (v0 == 0) goto loc_8004E39C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0++;
    v0 &= 0x3F;
    if (v0 != v1) goto loc_8004E29C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x360;                                        // Result = 80080360
    v0 = lw(v0);                                        // Load from: 80080360
    a0 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_8004E29C;
    a1 = 0;                                             // Result = 00000000
    LIBETC_DMACallback();
loc_8004E29C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(a0);
    v1 = 0x10000000;                                    // Result = 10000000
    v0 &= v1;
    v1 = a0;
    if (v0 != 0) goto loc_8004E2D4;
    a0 = 0x10000000;                                    // Result = 10000000
loc_8004E2C0:
    v0 = lw(v1);
    v0 &= a0;
    if (v0 == 0) goto loc_8004E2C0;
loc_8004E2D4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    v1 = a1 << 1;
    v1 += a1;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D8;                                        // Result = gLIBGPU_SYS__que[1] (800804D8)
    at += v0;
    a0 = lw(at);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v1 <<= 5;
    v0 = a1 << 1;
    v0 += a1;
    v0 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4DC;                                        // Result = gLIBGPU_SYS__que[2] (800804DC)
    at += v0;
    a1 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += v1;
    v0 = lw(at);
    ptr_call(v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 5;
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += v0;
    sw(0, at);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0++;
    v0 &= 0x3F;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5D98);                                // Store to: gpLIBGPU_SYS__qout (80075D98)
    goto loc_8004E3D4;
loc_8004E39C:
    a0 = 1;                                             // Result = 00000001
    LIBGPU_SYS__reset();
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1CC0;                                       // Result = STR_Sys_GpuExeque_Null_Func_Err[0] (80011CC0)
    LIBC2_printf();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5DA0);                               // Load from: 80075DA0
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5DA0);                                // Store to: 80075DA0
    goto loc_8004E410;
loc_8004E3D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    if (v1 == v0) goto loc_8004E410;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v0 = lw(v0);
    v1 = 0x1000000;                                     // Result = 01000000
    v0 &= v1;
    if (v0 == 0) goto loc_8004E228;
loc_8004E410:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 + 0x34C);                                // Load from: 8008034C
    LIBETC_SetIntrMask();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 += 0x35C;                                        // Result = 8008035C
    v0 = lw(v1);                                        // Load from: 8008035C
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8004E468;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x360);                                // Load from: 80080360
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8004E468;
    }
    sw(0, v1);                                          // Store to: 8008035C
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x360);                                // Load from: 80080360
    ptr_call(v0);
    v0 = 0;                                             // Result = 00000000
loc_8004E468:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS__reset() noexcept {
loc_8004E47C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = 0;                                             // Result = 00000000
    LIBETC_SetIntrMask();
    a1 = 0;                                             // Result = 00000000
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5D98);                                 // Store to: gpLIBGPU_SYS__qout (80075D98)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    a0 = 0;                                             // Result = 00000000
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x350);                                 // Store to: 80080350
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5D94);                                // Store to: gpLIBGPU_SYS__qin (80075D94)
loc_8004E4BC:
    at = 0x80080000;                                    // Result = 80080000
    at += 0x4D4;                                        // Result = gLIBGPU_SYS__que[0] (800804D4)
    at += a0;
    sw(0, at);
    a1++;
    v0 = (i32(a1) < 0x40);
    a0 += 0x60;
    if (v0 != 0) goto loc_8004E4BC;
    v0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8004E4F4;
    {
        const bool bJump = (s0 == v0);
        v0 = 0x401;                                     // Result = 00000401
        if (bJump) goto loc_8004E544;
    }
    goto loc_8004E590;
loc_8004E4F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D80);                               // Load from: 80075D80
    v0 = 0x401;                                         // Result = 00000401
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D90);                               // Load from: 80075D90
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x3D4;                                        // Result = gLIBGPU_SYS_ctlbuf[0] (800803D4)
    v0 = lw(v1);
    a1 = 0;                                             // Result = 00000000
    v0 |= 0x800;
    sw(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    a2 = 0x100;                                         // Result = 00000100
    sw(0, v0);
    LIBGPU_SYS_memset();
    goto loc_8004E590;
loc_8004E544:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D80);                               // Load from: 80075D80
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D90);                               // Load from: 80075D90
    v0 = lw(v1);
    v0 |= 0x800;
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = 0x2000000;                                     // Result = 02000000
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = 0x1000000;                                     // Result = 01000000
    sw(v0, v1);
loc_8004E590:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 + 0x350);                                // Load from: 80080350
    LIBETC_SetIntrMask();
    v0 = 0;                                             // Result = 00000000
    if (s0 != 0) goto loc_8004E630;
    v1 = 0x10000000;                                    // Result = 10000000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v1 |= 7;                                            // Result = 10000007
    sw(v1, v0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v1 = 0xFF0000;                                      // Result = 00FF0000
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 &= v1;
    v1 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v0 != v1);
        v1 = 0xE1000000;                                // Result = E1000000
        if (bJump) goto loc_8004E5E8;
    }
    v0 = 3;                                             // Result = 00000003
    goto loc_8004E630;
loc_8004E5E8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v1 |= 0x1000;                                       // Result = E1001000
    v0 &= 0x3FFF;
    v0 |= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D70);                               // Load from: GPU_REG_GP0 (80075D70)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v0 = lw(v1);
    v0 >>= 12;
    v0 &= 1;
loc_8004E630:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS__sync() noexcept {
loc_8004E644:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    if (a0 != 0) goto loc_8004E6F8;
    LIBGPU_SYS_set_alarm();
    goto loc_8004E67C;
loc_8004E664:
    LIBGPU_SYS__exeque();
    LIBGPU_SYS_get_alarm();
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004E778;
    }
loc_8004E67C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    if (v1 == v0) goto loc_8004E6B0;
    goto loc_8004E664;
loc_8004E6A0:
    LIBGPU_SYS_get_alarm();
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_8004E778;
    }
loc_8004E6B0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v0 = lw(v0);
    v1 = 0x1000000;                                     // Result = 01000000
    v0 &= v1;
    if (v0 != 0) goto loc_8004E6A0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8004E6A0;
    }
    goto loc_8004E778;
loc_8004E6F8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0 -= v1;
    s0 = v0 & 0x3F;
    if (s0 == 0) goto loc_8004E724;
    LIBGPU_SYS__exeque();
loc_8004E724:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    v0 = lw(v0);
    v1 = 0x1000000;                                     // Result = 01000000
    v0 &= v1;
    if (v0 != 0) goto loc_8004E764;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    v0 = lw(v0);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    if (v0 != 0) goto loc_8004E774;
loc_8004E764:
    v0 = s0;
    if (s0 != 0) goto loc_8004E778;
    v0 = 1;                                             // Result = 00000001
    goto loc_8004E778;
loc_8004E774:
    v0 = s0;
loc_8004E778:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS_set_alarm() noexcept {
loc_8004E78C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v0 += 0xF0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x1CD4);                                // Store to: 80081CD4
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x1CD8);                                 // Store to: 80081CD8
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBGPU_SYS_get_alarm() noexcept {
loc_8004E7C0:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x1CD4);                               // Load from: 80081CD4
    v1 = (i32(v1) < i32(v0));
    if (v1 != 0) goto loc_8004E810;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x1CD8);                               // Load from: 80081CD8
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x1CD8);                                // Store to: 80081CD8
    v0 = 0x780000;                                      // Result = 00780000
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8004E870;
loc_8004E810:
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5D74);                               // Load from: GPU_REG_GP1 (80075D74)
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1CD8;                                       // Result = STR_Sys_GpuTimeout_Err[0] (80011CD8)
    v0 = lw(a2);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5D94);                               // Load from: gpLIBGPU_SYS__qin (80075D94)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D78);                               // Load from: 80075D78
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5D98);                               // Load from: gpLIBGPU_SYS__qout (80075D98)
    v0 = lw(v0);
    a1 -= v1;
    sw(v0, sp + 0x10);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5D80);                               // Load from: 80075D80
    a2 = lw(a2);
    a3 = lw(v0);
    a1 &= 0x3F;
    LIBC2_printf();
    a0 = 1;                                             // Result = 00000001
    LIBGPU_SYS__reset();
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004E874;
loc_8004E870:
    v0 = 0;                                             // Result = 00000000
loc_8004E874:
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBGPU_SYS_memset() noexcept {
loc_8004E884:
    sp -= 8;
    v0 = a2 - 1;
    if (a2 == 0) goto loc_8004E8A4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8004E894:
    sb(a1, a0);
    v0--;
    a0++;
    if (v0 != v1) goto loc_8004E894;
loc_8004E8A4:
    sp += 8;
    return;
}

uint16_t LIBGPU_GetTPage(
    const int32_t texFmt,
    const int32_t semiTransRate,
    const int32_t tpageX,
    const int32_t tpageY
) noexcept {
    a0 = texFmt;
    a1 = semiTransRate;
    a2 = tpageX;
    a3 = tpageY;

    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s1, sp + 0x14);
    sw(ra, sp + 0x20);
    s1 = a3;
    LIBGPU_GetGraphType();
    v1 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v0 == v1);
        v1 = s0 & 3;
        if (bJump) goto loc_8004E974;
    }
    LIBGPU_GetGraphType();
    v1 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v0 != v1);
        v1 = s0 & 3;
        if (bJump) goto loc_8004E99C;
    }
loc_8004E974:
    v1 <<= 9;
    v0 = s2 & 3;
    v0 <<= 7;
    v1 |= v0;
    v0 = s1 & 0x300;
    v0 = u32(i32(v0) >> 3);
    v1 |= v0;
    v0 = s3 & 0x3FF;
    v0 = u32(i32(v0) >> 6);
    goto loc_8004E9CC;
loc_8004E99C:
    v1 <<= 7;
    v0 = s2 & 3;
    v0 <<= 5;
    v1 |= v0;
    v0 = s1 & 0x100;
    v0 = u32(i32(v0) >> 4);
    v1 |= v0;
    v0 = s3 & 0x3FF;
    v0 = u32(i32(v0) >> 6);
    v1 |= v0;
    v0 = s1 & 0x200;
    v0 <<= 2;
loc_8004E9CC:
    v0 |= v1;
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;

    return (uint16_t) v0;
}

void _thunk_LIBGPU_GetTPage() noexcept {
    v0 = LIBGPU_GetTPage(a0, a1, a2, a3);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the CLUT id for a CLUT uploaded to the given coordinate in VRAM.
// Note: the x address is limited to being in multiples of 16.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t LIBGPU_GetClut(const int32_t x, const int32_t y) noexcept {
    return (uint16_t)((y << 6) | ((x >> 4) & 0x3F));
}

void LIBGPU_NextPrim() noexcept {
    v1 = 0xFF0000;                                      // Result = 00FF0000
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 &= v1;
    v1 = 0x80000000;                                    // Result = 80000000
    v0 |= v1;
    return;
}

void LIBGPU_IsEndPrim() noexcept {
    v1 = 0xFF0000;                                      // Result = 00FF0000
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 &= v1;
    v0 ^= v1;
    v0 = (v0 < 1);
    return;
}

void LIBGPU_AddPrim() noexcept {
loc_8004EB50:
    a2 = 0xFF0000;                                      // Result = 00FF0000
    a2 |= 0xFFFF;                                       // Result = 00FFFFFF
    a3 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a1);
    v0 = lw(a0);
    v1 &= a3;
    v0 &= a2;
    v1 |= v0;
    sw(v1, a1);
    v0 = lw(a0);
    a1 &= a2;
    v0 &= a3;
    v0 |= a1;
    sw(v0, a0);
    return;
}

void LIBGPU_AddPrims() noexcept {
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = lw(a0);
    v1 &= t0;
    v0 &= a3;
    v1 |= v0;
    sw(v1, a2);
    v0 = lw(a0);
    a1 &= a3;
    v0 &= t0;
    v0 |= a1;
    sw(v0, a0);
    return;
}

void LIBGPU_CatPrim() noexcept {
    a2 = 0xFF0000;                                      // Result = 00FF0000
    a2 |= 0xFFFF;                                       // Result = 00FFFFFF
    v1 = 0xFF000000;                                    // Result = FF000000
    v0 = lw(a0);
    a1 &= a2;
    v0 &= v1;
    v0 |= a1;
    sw(v0, a0);
    return;
}

void LIBGPU_TermPrim() noexcept {
loc_8004EBEC:
    v1 = 0xFF0000;                                      // Result = 00FF0000
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 |= v1;
    sw(v0, a0);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable/disable semi-transparency on the specified drawing primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetSemiTrans(void* const pPrim, const bool bTransparent) noexcept {
    uint8_t& primCode = ((uint8_t*) pPrim)[7];

    if (bTransparent) {
        primCode |= 0x2;
    } else {
        primCode &= 0xFD;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable or disable texture shading on a specified primitive.
// When shading is disabled, the texture is displayed as-is.
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetShadeTex(void* const pPrim, const bool bDisableShading) noexcept {
    uint8_t& primCode = ((uint8_t*) pPrim)[7];

    if (bDisableShading) {
        primCode |= 1;
    } else {
        primCode &= 0xFE;
    }
}

void _thunk_LIBGPU_SetShadeTex() noexcept {
    LIBGPU_SetShadeTex(vmAddrToPtr<void>(a0), a1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded textured triangle primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyFT3(POLY_FT3& poly) noexcept {
    LIBGPU_setlen(poly, 7);
    poly.code = 0x24;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded quad primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyF4(POLY_F4& poly) noexcept {
    LIBGPU_setlen(poly, 5);
    poly.code = 0x28;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a flat shaded textured quad primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetPolyFT4(POLY_FT4& poly) noexcept {
    LIBGPU_setlen(poly, 9);
    poly.code = 0x2C;
}

void LIBGPU_SetSprt8() noexcept {
loc_8004ECF4:
    v0 = 3;                                             // Result = 00000003
    sb(v0, a0 + 0x3);
    v0 = 0x74;                                          // Result = 00000074
    sb(v0, a0 + 0x7);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize a sprite primitive
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetSprt(SPRT & sprt) noexcept {
    LIBGPU_setlen(sprt, 4);
    sprt.code = 0x64;
}

void LIBGPU_SetTile() noexcept {
loc_8004ED6C:
    v0 = 3;                                             // Result = 00000003
    sb(v0, a0 + 0x3);
    v0 = 0x60;                                          // Result = 00000060
    sb(v0, a0 + 0x7);
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the specified primitive as a flat shaded and unconnected line
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBGPU_SetLineF2(LINE_F2& line) noexcept {
    LIBGPU_setlen(line, 3);
    line.code = 0x40;
}

void LIBGPU_MargePrim() noexcept {
    v0 = lbu(a0 + 0x3);
    v1 = lbu(a1 + 0x3);
    v0 += v1;
    v1 = v0 + 1;
    v0 = (i32(v1) < 0x21);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8004EE64;
    }
    sb(v1, a0 + 0x3);
    goto loc_8004EE68;
loc_8004EE64:
    v0 = -1;                                            // Result = FFFFFFFF
loc_8004EE68:
    return;
}

void LIBGPU_SetDumpFnt() noexcept {
loc_8004F09C:
    if (i32(a0) < 0) goto loc_8004F0D4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = (i32(v0) < i32(a0));
    if (v0 != 0) goto loc_8004F0D4;
    v0 = 0x80050000;                                    // Result = 80050000
    v0 -= 0x954;                                        // Result = LIBGPU_FntPrint (8004F6AC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5E5C);                                // Store to: gLIBGPU_FONT_UNKNOWN_2 (80075E5C)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5D58);                                // Store to: gpLIBGPU_GPU_printf (80075D58)
loc_8004F0D4:
    return;
}

void LIBGPU_FntLoad() noexcept {
loc_8004F0DC:
    sp -= 0x30;
    sw(s2, sp + 0x28);
    s2 = a0;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(s0, sp + 0x20);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5E60;                                       // Result = gLIBGPU_FONT_FntLoad_Clut[0] (80075E60)
    a0 = s0;                                            // Result = gLIBGPU_FONT_FntLoad_Clut[0] (80075E60)
    a1 = s2;
    sw(ra, sp + 0x2C);
    a2 = s1 + 0x80;
    LIBGPU_LoadClut();
    a0 = s0 + 0x200;                                    // Result = gLIBGPU_FONT_FntLoad_FontTex[0] (80076060)
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x60E0);                                // Store to: gLIBGPU_FONT_clut (800860E0)
    v0 = 0x80;                                          // Result = 00000080
    sw(v0, sp + 0x14);
    v0 = 0x20;                                          // Result = 00000020
    a3 = s2;
    sw(s1, sp + 0x10);
    sw(v0, sp + 0x18);
    LIBGPU_LoadTPage();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x5DA8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[0] (80075DA8)
    a1 = 0;                                             // Result = 00000000
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x60DC);                                // Store to: gLIBGPU_FONT_tpage (800860DC)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5E58);                                 // Store to: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    a2 = 0xB0;                                          // Result = 000000B0
    LIBC2_memset();
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void LIBGPU_FntOpen() noexcept {
loc_8004F180:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    sp -= 0x40;
    sw(s2, sp + 0x28);
    s2 = lw(sp + 0x54);
    sw(s0, sp + 0x20);
    s0 = lw(sp + 0x50);
    sw(s3, sp + 0x2C);
    s3 = a0;
    sw(s4, sp + 0x30);
    s4 = a1;
    sw(s5, sp + 0x34);
    s5 = a2;
    sw(s6, sp + 0x38);
    s6 = a3;
    sw(ra, sp + 0x3C);
    v0 = (i32(v1) < 4);
    sw(s1, sp + 0x24);
    if (v0 != 0) goto loc_8004F1D4;
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004F420;
loc_8004F1D4:
    if (v1 != 0) goto loc_8004F1E4;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6860);                                 // Store to: gLIBGPU_FONT_UNKNOWN_3 (80076860)
loc_8004F1E4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6860);                               // Load from: gLIBGPU_FONT_UNKNOWN_3 (80076860)
    v0 = s2 + a0;
    v0 = (i32(v0) < 0x401);
    a1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8004F208;
    v0 = 0x400;                                         // Result = 00000400
    s2 = v0 - a0;
loc_8004F208:
    a0 = v1 << 1;
    a0 += v1;
    a0 <<= 2;
    a0 -= v1;
    a0 <<= 2;
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5DB8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[4] (80075DB8)
    a0 += s1;
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lhu(a3 + 0x60DC);                              // Load from: gLIBGPU_FONT_tpage (800860DC)
    a2 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    _thunk_LIBGPU_SetDrawMode();
    {
        const bool bJump = (s0 == 0);
        s0 = s1 - 0x10;                                 // Result = gLIBGPU_FONT_FntLoad_Font[0] (80075DA8)
        if (bJump) goto loc_8004F2E4;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    a0 = v0 << 1;
    a0 += v0;
    a0 <<= 2;
    a0 -= v0;
    a0 <<= 2;
    a0 += s0;
    LIBGPU_SetTile();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    v0 += s0;
    sb(0, v0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    v0 += s0;
    sb(0, v0 + 0x5);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    v0 += s0;
    sb(0, v0 + 0x6);
loc_8004F2E4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    a0 = s1 - 0x10;                                     // Result = gLIBGPU_FONT_FntLoad_Font[0] (80075DA8)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 2;
    v1 -= v0;
    v1 <<= 2;
    a0 += v1;
    sh(s3, a0 + 0x8);
    sh(s4, a0 + 0xA);
    sh(s5, a0 + 0xC);
    sh(s6, a0 + 0xE);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6860);                               // Load from: gLIBGPU_FONT_UNKNOWN_3 (80076860)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x1CDC;                                       // Result = gLIBGPU_FONT_str_6[0] (80081CDC)
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DC4;                                       // Result = gLIBGPU_FONT_FntLoad_Font[7] (80075DC4)
    at += v1;
    sw(s2, at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DD0;                                       // Result = gLIBGPU_FONT_FntLoad_Font[A] (80075DD0)
    at += v1;
    sw(0, at);
    v0 += a0;
    a0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DCC;                                       // Result = gLIBGPU_FONT_FntLoad_Font[9] (80075DCC)
    at += v1;
    sw(v0, at);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x20DC;                                       // Result = gLIBGPU_FONT_sprt_7[0] (800820DC)
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DCC;                                       // Result = gLIBGPU_FONT_FntLoad_Font[9] (80075DCC)
    at += v1;
    a1 = lw(at);
    a0 += v0;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DC8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[8] (80075DC8)
    at += v1;
    sw(a0, at);
    sb(0, a1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DC8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[8] (80075DC8)
    at += v0;
    s0 = lw(at);
    s1 = 0;                                             // Result = 00000000
    if (i32(s2) <= 0) goto loc_8004F3F8;
loc_8004F3C8:
    a0 = s0;
    LIBGPU_SetSprt8();
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    _thunk_LIBGPU_SetShadeTex();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E0);                              // Load from: gLIBGPU_FONT_clut (800860E0)
    s1++;
    sh(v0, s0 + 0xE);
    v0 = (i32(s1) < i32(s2));
    s0 += 0x10;
    if (v0 != 0) goto loc_8004F3C8;
loc_8004F3F8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6860);                               // Load from: gLIBGPU_FONT_UNKNOWN_3 (80076860)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v1 += s2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x6860);                                // Store to: gLIBGPU_FONT_UNKNOWN_3 (80076860)
    v1 = v0 + 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5E58);                                // Store to: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
loc_8004F420:
    ra = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x40;
    return;
}

void LIBGPU_FntFlush() noexcept {
loc_8004F44C:
    sp -= 0x48;
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    if (i32(a0) < 0) goto loc_8004F494;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = a0 << 1;
        if (bJump) goto loc_8004F4D8;
    }
loc_8004F494:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5E5C);                               // Load from: gLIBGPU_FONT_UNKNOWN_2 (80075E5C)
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DCC;                                       // Result = gLIBGPU_FONT_FntLoad_Font[9] (80075DCC)
    at += v0;
    v0 = lw(at);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 << 1;
        if (bJump) goto loc_8004F4D8;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_8004F678;
loc_8004F4D8:
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x5DA8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[0] (80075DA8)
    s4 = v0 + v1;
    a3 = s4 + 0x10;
    sw(a3, sp + 0x10);
    a0 = lw(sp + 0x10);
    s1 = lw(s4 + 0x24);
    s6 = lw(s4 + 0x20);
    fp = lw(s4 + 0x1C);
    s0 = lh(s4 + 0x8);
    s5 = lh(s4 + 0xA);
    v0 = lh(s4 + 0xC);
    v1 = lh(s4 + 0xE);
    s7 = s0 + v0;
    v1 += s5;
    sw(v1, sp + 0x18);
    LIBGPU_TermPrim();
    v0 = lbu(s1);
    if (v0 == 0) goto loc_8004F640;
    s3 = s6 + 0xA;
loc_8004F540:
    v0 = 0x20;                                          // Result = 00000020
    if (fp == 0) goto loc_8004F640;
    v1 = lbu(s1);
    s2 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_8004F5F0;
    v0 = (i32(v1) < 0x21);
    {
        const bool bJump = (v0 == 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8004F584;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_8004F57C;
    }
    if (v1 != v0) goto loc_8004F584;
    s2 = 1;                                             // Result = 00000001
    goto loc_8004F604;
loc_8004F57C:
    s0 += 0x20;
    goto loc_8004F5F4;
loc_8004F584:
    v0 = lbu(s1);
    v0 -= 0x61;
    v0 = (v0 < 0x1A);
    if (v0 == 0) goto loc_8004F5A8;
    v0 = lbu(s1);
    v1 = v0 - 0x40;
    goto loc_8004F5B4;
loc_8004F5A8:
    v0 = lbu(s1);
    v1 = v0 - 0x20;
loc_8004F5B4:
    v0 = v1;
    if (i32(v1) >= 0) goto loc_8004F5C0;
    v0 = v1 + 0xF;
loc_8004F5C0:
    a0 = u32(i32(v0) >> 4);
    v0 = a0 << 4;
    v0 = v1 - v0;
    a2 = v0 << 3;
    v0 = a0 << 3;
    a0 = lw(sp + 0x10);
    a1 = s6;
    sb(a2, s3 + 0x2);
    sb(v0, s3 + 0x3);
    sh(s0, s3 - 0x2);
    sh(s5, s3);
    LIBGPU_AddPrim();
loc_8004F5F0:
    s0 += 8;
loc_8004F5F4:
    v0 = (i32(s0) < i32(s7));
    if (v0 != 0) goto loc_8004F604;
    s2 = 1;                                             // Result = 00000001
loc_8004F604:
    if (s2 == 0) goto loc_8004F624;
    s5 += 8;
    a3 = lw(sp + 0x18);
    s0 = lh(s4 + 0x8);
    v0 = (i32(s5) < i32(a3));
    if (v0 == 0) goto loc_8004F640;
loc_8004F624:
    s3 += 0x10;
    s6 += 0x10;
    s1++;
    v0 = lbu(s1);
    fp--;
    if (v0 != 0) goto loc_8004F540;
loc_8004F640:
    v0 = lbu(s4 + 0x7);
    if (v0 == 0) goto loc_8004F65C;
    a0 = lw(sp + 0x10);
    a1 = s4;
    LIBGPU_AddPrim();
loc_8004F65C:
    a0 = lw(sp + 0x10);
    LIBGPU_DrawOTag();
    v1 = lw(s4 + 0x24);
    v0 = lw(sp + 0x10);
    sw(0, s4 + 0x28);
    sb(0, v1);
loc_8004F678:
    ra = lw(sp + 0x44);
    fp = lw(sp + 0x40);
    s7 = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x48;
    return;
}

void LIBGPU_FntPrint() noexcept {
loc_8004F6AC:
    sw(a0, sp);
    sw(a1, sp + 0x4);
    sw(a2, sp + 0x8);
    sw(a3, sp + 0xC);
    sp -= 0x238;
    v0 = sp + 0x23C;
    sw(ra, sp + 0x230);
    sw(s5, sp + 0x22C);
    sw(s4, sp + 0x228);
    sw(s3, sp + 0x224);
    sw(s2, sp + 0x220);
    sw(s1, sp + 0x21C);
    sw(s0, sp + 0x218);
    sw(a0, sp + 0x238);
    sw(v0, sp + 0x210);
    if (i32(a0) < 0) goto loc_8004F704;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E58);                               // Load from: gLIBGPU_FONT_UNKNOWN_1 (80075E58)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = sp + 0x240;
        if (bJump) goto loc_8004F748;
    }
loc_8004F704:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5E5C);                               // Load from: gLIBGPU_FONT_UNKNOWN_2 (80075E5C)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 2;
    v1 -= v0;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5DCC;                                       // Result = gLIBGPU_FONT_FntLoad_Font[9] (80075DCC)
    at += v1;
    v1 = lw(at);
    s3 = a0;
    sw(v0, sp + 0x238);
    if (v1 != 0) goto loc_8004F750;
loc_8004F740:
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004FA7C;
loc_8004F748:
    sw(v0, sp + 0x210);
    s3 = lw(sp + 0x23C);
loc_8004F750:
    v1 = lw(sp + 0x238);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x5DA8;                                       // Result = gLIBGPU_FONT_FntLoad_Font[0] (80075DA8)
    s1 = v0 + v1;
    v1 = lw(s1 + 0x28);
    v0 = lw(s1 + 0x1C);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_8004F740;
    a1 = lbu(s3);
    if (a1 == 0) goto loc_8004FA64;
    s5 = 0x25;                                          // Result = 00000025
    s4 = 0xCCCC0000;                                    // Result = CCCC0000
    s4 |= 0xCCCD;                                       // Result = CCCCCCCD
loc_8004F7AC:
    if (a1 != s5) goto loc_8004F7C8;
    s3++;
    a1 = lbu(s3);
    s2 = 0;                                             // Result = 00000000
    if (a1 != s5) goto loc_8004F800;
loc_8004F7C8:
    a0 = lw(s1 + 0x28);
    v1 = lw(s1 + 0x24);
    v0 = a0 + 1;
    v1 += a0;
    sw(v0, s1 + 0x28);
    sb(a1, v1);
    v1 = lw(s1 + 0x28);
    v0 = lw(s1 + 0x1C);
    v0 = (i32(v0) < i32(v1));
    s3++;
    if (v0 == 0) goto loc_8004FA54;
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_8004FA7C;
loc_8004F800:
    v0 = a1 ^ 0x30;
    v0 = (v0 < 1);
    a3 = v0;
    goto loc_8004F82C;
loc_8004F810:
    v0 = s2 << 2;
    v0 += s2;
    v0 <<= 1;
    v0 -= 0x30;
    s2 = v0 + a1;
    s3++;
    a1 = lbu(s3);
loc_8004F82C:
    v0 = a1 - 0x30;
    v0 = (v0 < 0xA);
    if (v0 != 0) goto loc_8004F810;
    s0 = sp + 0x210;
    if (i32(s2) > 0) goto loc_8004F84C;
    s2 = 1;                                             // Result = 00000001
loc_8004F84C:
    v1 = a1 - 0x58;
    v0 = (v1 < 0x21);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8004F9B0;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1DF4;                                       // Result = JumpTable_LIBGPU_FntPrint[0] (80011DF4)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x8004F8F8: goto loc_8004F8F8;
        case 0x8004F9B0: goto loc_8004F9B0;
        case 0x8004F970: goto loc_8004F970;
        case 0x8004F878: goto loc_8004F878;
        case 0x8004F990: goto loc_8004F990;
        default: jump_table_err(); break;
    }
loc_8004F878:
    v1 = lw(sp + 0x210);
    v0 = v1 + 4;
    sw(v0, sp + 0x210);
    a0 = lw(v1);
    a1 = 0;                                             // Result = 00000000
    if (i32(a0) >= 0) goto loc_8004F8A0;
    a0 = -a0;
    a1 = 0x2D;                                          // Result = 0000002D
loc_8004F8A0:
    a2 = 0;                                             // Result = 00000000
loc_8004F8A4:
    multu(a0, s4);
    s0--;
    a2++;
    v1 = hi;
    v1 >>= 3;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v0 = a0 - v0;
    v0 += 0x30;
    sb(v0, s0);
    a0 = v1;
    if (a2 == 0) goto loc_8004F8A4;
    if (a0 != 0) goto loc_8004F8A4;
    v0 = (i32(a2) < i32(s2));
    if (a1 == 0) goto loc_8004F9B4;
    s0--;
    sb(a1, s0);
    a2++;
    goto loc_8004F9B0;
loc_8004F8F8:
    v1 = lw(sp + 0x210);
    a2 = 0;                                             // Result = 00000000
    v0 = v1 + 4;
    sw(v0, sp + 0x210);
    a0 = lw(v1);
loc_8004F90C:
    s0--;
loc_8004F910:
    v0 = a0 & 0xF;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6864);                               // Load from: gpLIBGPU_FONT_hexchars_upper (80076864)
    a0 >>= 4;
    v1 += v0;
    v0 = lbu(v1);
    a2++;
    sb(v0, s0);
    if (a2 == 0) goto loc_8004F90C;
    s0--;
    if (a0 != 0) goto loc_8004F910;
    s0++;
    v0 = a3 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a2) < i32(s2));
        if (bJump) goto loc_8004F9B0;
    }
    v1 = 0x30;                                          // Result = 00000030
    if (v0 == 0) goto loc_8004F9FC;
loc_8004F954:
    s0--;
    a2++;
    v0 = (i32(a2) < i32(s2));
    sb(v1, s0);
    if (v0 != 0) goto loc_8004F954;
    goto loc_8004F9B4;
loc_8004F970:
    v0 = lw(sp + 0x210);
    s0--;
    v1 = v0 + 4;
    sw(v1, sp + 0x210);
    v0 = lbu(v0);
    a2 = 1;                                             // Result = 00000001
    sb(v0, s0);
    goto loc_8004F9B0;
loc_8004F990:
    v1 = lw(sp + 0x210);
    v0 = v1 + 4;
    sw(v0, sp + 0x210);
    s0 = lw(v1);
    a0 = s0;
    LIBC2_strlen();
    a2 = v0;
loc_8004F9B0:
    v0 = (i32(a2) < i32(s2));
loc_8004F9B4:
    if (v0 == 0) goto loc_8004F9FC;
    a1 = 0x20;                                          // Result = 00000020
loc_8004F9C0:
    a0 = lw(s1 + 0x28);
    v1 = lw(s1 + 0x24);
    v0 = a0 + 1;
    v1 += a0;
    sw(v0, s1 + 0x28);
    sb(a1, v1);
    v1 = lw(s1 + 0x28);
    v0 = lw(s1 + 0x1C);
    v0 = (i32(v0) < i32(v1));
    s2--;
    if (v0 != 0) goto loc_8004F740;
    v0 = (i32(a2) < i32(s2));
    if (v0 != 0) goto loc_8004F9C0;
loc_8004F9FC:
    a2--;
    v0 = -1;                                            // Result = FFFFFFFF
    a1 = -1;                                            // Result = FFFFFFFF
    if (a2 == v0) goto loc_8004FA50;
loc_8004FA0C:
    a0 = lw(s1 + 0x28);
    v0 = a0 + 1;
    sw(v0, s1 + 0x28);
    v0 = lw(s1 + 0x24);
    v1 = lbu(s0);
    v0 += a0;
    sb(v1, v0);
    v1 = lw(s1 + 0x28);
    v0 = lw(s1 + 0x1C);
    v0 = (i32(v0) < i32(v1));
    s0++;
    if (v0 != 0) goto loc_8004F740;
    a2--;
    if (a2 != a1) goto loc_8004FA0C;
loc_8004FA50:
    s3++;
loc_8004FA54:
    a1 = lbu(s3);
    if (a1 != 0) goto loc_8004F7AC;
loc_8004FA64:
    v0 = lw(s1 + 0x24);
    v1 = lw(s1 + 0x28);
    v0 += v1;
    sb(0, v0);
    v0 = lw(s1 + 0x28);
loc_8004FA7C:
    ra = lw(sp + 0x230);
    s5 = lw(sp + 0x22C);
    s4 = lw(sp + 0x228);
    s3 = lw(sp + 0x224);
    s2 = lw(sp + 0x220);
    s1 = lw(sp + 0x21C);
    s0 = lw(sp + 0x218);
    sp += 0x238;
    return;
}

void LIBGPU_LoadTPage() noexcept {
loc_8004FAD4:
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = lw(sp + 0x40);
    v1 = lw(sp + 0x44);
    v0 = lw(sp + 0x48);
    t0 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(s2, sp + 0x20);
    s2 = a2;
    sw(s1, sp + 0x1C);
    s1 = a3;
    sw(ra, sp + 0x28);
    sh(s1, sp + 0x10);
    sh(v0, sp + 0x16);
    v0 = 1;                                             // Result = 00000001
    sh(s3, sp + 0x12);
    if (s0 == v0) goto loc_8004FB64;
    v0 = (i32(s0) < 2);
    if (v0 == 0) goto loc_8004FB38;
    a0 = sp + 0x10;
    if (s0 == 0) goto loc_8004FB4C;
    goto loc_8004FB80;
loc_8004FB38:
    v0 = 2;                                             // Result = 00000002
    a0 = sp + 0x10;
    if (s0 == v0) goto loc_8004FB78;
    goto loc_8004FB80;
loc_8004FB4C:
    v0 = v1;
    if (i32(v1) >= 0) goto loc_8004FB58;
    v0 = v1 + 3;
loc_8004FB58:
    v0 = u32(i32(v0) >> 2);
    sh(v0, sp + 0x14);
    goto loc_8004FB7C;
loc_8004FB64:
    v0 = v1 >> 31;
    v0 += v1;
    v0 = u32(i32(v0) >> 1);
    sh(v0, sp + 0x14);
    goto loc_8004FB7C;
loc_8004FB78:
    sh(v1, sp + 0x14);
loc_8004FB7C:
    a0 = sp + 0x10;
loc_8004FB80:
    a1 = t0;
    _thunk_LIBGPU_LoadImage();
    a0 = s0;
    a1 = s2;
    a2 = s1;
    a3 = s3;
    _thunk_LIBGPU_GetTPage();
    v0 &= 0xFFFF;
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void LIBGPU_LoadClut() noexcept {
loc_8004FBC0:
    sp -= 0x28;
    v0 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(s1, sp + 0x1C);
    s1 = a2;
    a0 = sp + 0x10;
    a1 = v0;
    v0 = 0x100;                                         // Result = 00000100
    sh(v0, sp + 0x14);
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x20);
    sh(s0, sp + 0x10);
    sh(s1, sp + 0x12);
    sh(v0, sp + 0x16);
    _thunk_LIBGPU_LoadImage();
    a0 = s0;
    a1 = s1;
    v0 = LIBGPU_GetClut(a0, a1);
    v0 &= 0xFFFF;
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the given draw environment struct to the defaults with the given draw area
//------------------------------------------------------------------------------------------------------------------------------------------
DRAWENV& LIBGPU_SetDefDrawEnv(DRAWENV& env, const int32_t x, const int32_t y, const int32_t w, const  int32_t h) noexcept {
  env.clip.x = (int16_t) x;
  env.clip.y = (int16_t) y;
  env.clip.w = (int16_t) w;
  env.clip.h = (int16_t) h;

  env.ofs[0] = (int16_t) x;
  env.ofs[1] = (int16_t) y;

  env.tw.x = 0;
  env.tw.y = 0;
  env.tw.w = 0;
  env.tw.h = 0;
  env.tpage = LIBGPU_GetTPage(0, 0, 640, 0);

  env.dtd = 1;
  env.dfe = (h != 480);

  env.r0 = 0;
  env.g0 = 0;
  env.b0 = 0;  
  
  env.isbg = 0;
  return env;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the given display environment struct to the defaults with the given display area
//------------------------------------------------------------------------------------------------------------------------------------------
DISPENV& LIBGPU_SetDefDispEnv(DISPENV& disp, const int32_t x, const int32_t y, const int32_t w, const int32_t h) noexcept {
  disp.disp.x = (int16_t) x;
  disp.disp.y = (int16_t) y;
  disp.disp.w = (int16_t) w;
  disp.disp.h = (int16_t) h;

  disp.screen.x = 0;
  disp.screen.y = 0;
  disp.screen.w = 0;
  disp.screen.h = 0;

  disp.isinter = 0;
  disp.isrgb24 = 0;
  
  disp._pad[0] = 0;
  disp._pad[1] = 0;
  
  return disp;
}

#include "r_draw.h"

#include "Doom/Base/i_main.h"
#include "Doom/Game/p_setup.h"
#include "PcPsx/Finally.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGTE.h"
#include "r_local.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_segs.h"
#include "r_things.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws everything in the subsector: floors, ceilings, walls and things
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsector(subsector_t& subsec) noexcept {
    // The PSX scratchpad is used to store 2 leafs, grab that memory here.
    // The code below ping-pongs between both leafs, using them as either input or output leafs for each clipping operation/
    // I don't know why this particular address is used though...
    leaf_t* const pLeafs = (leaf_t*) getScratchAddr(42);
    leaf_t& leaf1 = pLeafs[0];
    
    // Cache the entire leaf for the subsector to the scratchpad.
    // Also transform any leaf vertices that were not yet transformed up until this point.
    {
        const leafedge_t* pSrcEdge = gpLeafEdges->get() + subsec.firstLeafEdge;
        leafedge_t* pDstEdge = leaf1.edges;
        
        for (int32_t edgeIdx = 0; edgeIdx < subsec.numLeafEdges; ++edgeIdx, ++pSrcEdge, ++pDstEdge) {
            // Cache the leaf edge
            vertex_t& vert = *pSrcEdge->vertex;
            
            pDstEdge->vertex = &vert;
            pDstEdge->seg = pSrcEdge->seg;
            
            // Transform this leaf edge's vertexes if they need to be transformed
            if (vert.frameUpdated != *gNumFramesDrawn) {
                const SVECTOR viewToPt = {
                    (int16_t)((vert.x - *gViewX) >> 16),
                    0,
                    (int16_t)((vert.y - *gViewY) >> 16)
                };
                
                VECTOR viewVec;
                int32_t rotFlags;
                LIBGTE_RotTrans(viewToPt, viewVec, rotFlags);
                
                vert.viewx = viewVec.vx;
                vert.viewy = viewVec.vz;
                
                if (viewVec.vz > 3) {
                    vert.scale = (HALF_SCREEN_W * FRACUNIT) / viewVec.vz;
                    vert.screenx = ((vert.scale * vert.viewx) >> FRACBITS) + HALF_SCREEN_W;
                }
                
                vert.frameUpdated = *gNumFramesDrawn;
            }
        }
        
        leaf1.numEdges = subsec.numLeafEdges;
    }
    
    // Begin the process of clipping the leaf.
    // Ping pong between the two leaf buffers for input and output..
    uint32_t curLeafIdx = 0;
    
    // TODO: MAKE A GLOBAL FOR THIS
    sw(0, gp + 0xC40);      // Store to: 80078220
    
    // Clip the leaf against the front plane if required
    {
        leafedge_t* pEdge = leaf1.edges;
        
        for (int32_t edgeIdx = 0; edgeIdx < subsec.numLeafEdges; ++edgeIdx, ++pEdge) {
            if (pEdge->vertex->viewy <= NEAR_CLIP_DIST + 1) {
                a0 = ptrToVmAddr(pLeafs + curLeafIdx);
                a1 = ptrToVmAddr(pLeafs + (curLeafIdx ^ 1));
                R_FrontZClip();
                curLeafIdx ^= 1;
                break;
            }
        }
    }
    
    // TODO: comment/explain and handle return value
    a0 = 0;
    a1 = ptrToVmAddr(pLeafs + curLeafIdx);
    R_CheckEdgeVisible();
    
    if (i32(v0) < 0)
        return;
    
    // Clip the leaf against the left view frustrum plane if required
    if (i32(v0) > 0) {
        a0 = ptrToVmAddr(pLeafs + curLeafIdx);
        a1 = ptrToVmAddr(pLeafs + (curLeafIdx ^ 1));
        R_LeftEdgeClip();
        curLeafIdx ^= 1;
        
        if (i32(v0) < 3)
            return;
    }
    
    // TODO: comment/explain and handle return value properly
    a0 = 1;
    a1 = ptrToVmAddr(pLeafs + curLeafIdx);
    R_CheckEdgeVisible();
    
    if (i32(v0) < 0)
        return;
    
    // Clip the leaf against the right view frustrum plane if required
    if (i32(v0) > 0) {
        a0 = ptrToVmAddr(pLeafs + curLeafIdx);
        a1 = ptrToVmAddr(pLeafs + (curLeafIdx ^ 1));
        R_RightEdgeClip();
        curLeafIdx ^= 1;
        
        if (i32(v0) < 3)
            return;
    }
    
    // Terminate the list of leaf edges by putting the first edge past the end.
    // This allows the renderer to access one past the end without worrying about bounds checks.
    // Handy for when working with edges!
    leaf_t& drawleaf = pLeafs[curLeafIdx];
    drawleaf.edges[drawleaf.numEdges] = drawleaf.edges[0];
    
    // Draw all visible walls/segs in the leaf
    {
        leafedge_t* pEdge = drawleaf.edges;
        
        for (int32_t edgeIdx = 0; edgeIdx < drawleaf.numEdges; ++edgeIdx, ++pEdge) {
            // Only draw the seg if its marked as visible
            seg_t* const pSeg = pEdge->seg.get();
            
            if (pSeg && (pSeg->flags & SGF_VISIBLE_COLS)) {
                a0 = ptrToVmAddr(pEdge);
                R_DrawSubsectorSeg();       // TODO: RENAME - it's drawing a leaf edge
            }
        }
    }
    
    // Draw the floor if above it
    sector_t& drawsec = **gpCurDrawSector;
    
    if (*gViewZ > drawsec.floorheight) {
        a0 = ptrToVmAddr(&drawleaf);
        a1 = 0;
        R_DrawSubsectorFlat();
    }
    
    // Draw the ceiling if below it and it is not a sky ceiling
    if ((drawsec.ceilingpic != -1) && (*gViewZ < drawsec.ceilingheight)) {
        a0 = ptrToVmAddr(&drawleaf);
        a1 = 1;
        R_DrawSubsectorFlat();
    }
    
    // Draw all sprites in the subsector
    a0 = ptrToVmAddr(&subsec);
    R_DrawSubsectorSprites();
}

void R_FrontZClip() noexcept {
loc_8002CA8C:
    sp -= 0x50;
    sw(s3, sp + 0x34);
    s3 = a0 + 4;
    sw(s1, sp + 0x2C);
    sw(a1, sp + 0x18);
    s1 = a1 + 4;
    sw(s7, sp + 0x44);
    s7 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x4C);
    sw(fp, sp + 0x48);
    sw(s6, sp + 0x40);
    sw(s5, sp + 0x3C);
    sw(s4, sp + 0x38);
    sw(s2, sp + 0x30);
    sw(s0, sp + 0x28);
    sw(a0, sp + 0x10);
    fp = lw(a0);
    s2 = 0;                                             // Result = 00000000
    if (i32(fp) <= 0) goto loc_8002CD28;
loc_8002CADC:
    v0 = fp - 1;
    s4 = s3 + 8;
    if (s7 != v0) goto loc_8002CAF4;
    a2 = lw(sp + 0x10);
    s4 = a2 + 4;
loc_8002CAF4:
    a2 = 4;                                             // Result = 00000004
    v0 = lw(s3);
    v1 = lw(s4);
    v0 = lw(v0 + 0x10);
    v1 = lw(v1 + 0x10);
    s5 = a2 - v0;
    s6 = a2 - v1;
    if (s5 != 0) goto loc_8002CB28;
    v0 = lw(s3);
    v1 = lw(s3 + 0x4);
    sw(v0, s1);
    sw(v1, s1 + 0x4);
    goto loc_8002CCF8;
loc_8002CB28:
    if (i32(s5) >= 0) goto loc_8002CB60;
    v0 = lw(s3);
    v1 = lw(s3 + 0x4);
    sw(v0, s1);
    sw(v1, s1 + 0x4);
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 0x15);                              // Result = 00000001
    s1 += 8;
    if (v0 != 0) goto loc_8002CB60;
    I_Error("FrontZClip: Point Overflow");
loc_8002CB60:
    v0 = ~s6;
    if (s6 == 0) goto loc_8002CD18;
    v0 >>= 31;
    v1 = s5 >> 31;
    if (v0 != v1) goto loc_8002CD18;
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002CBB8;
    I_Error("FrontZClip: exceeded max new vertexes\n");
loc_8002CBB8:
    a0 = s5 << 16;
    v0 = s5 - s6;
    div(a0, v0);
    if (v0 != 0) goto loc_8002CBD0;
    _break(0x1C00);
loc_8002CBD0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CBE8;
    }
    if (a0 != at) goto loc_8002CBE8;
    tge(zero, zero, 0x5D);
loc_8002CBE8:
    a0 = lo;
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0 + 0xC);
    v1 = lw(v1 + 0xC);
    v0 -= v1;
    mult(a0, v0);
    a2 = 4;                                             // Result = 00000004
    sw(a2, s0 + 0x10);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += v1;
    sw(v0, s0 + 0xC);
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002CC88;
    _break(0x1C00);
loc_8002CC88:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CCA0;
    }
    if (v1 != at) goto loc_8002CCA0;
    tge(zero, zero, 0x5D);
loc_8002CCA0:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    if (i32(s5) <= 0) goto loc_8002CCF0;
    if (i32(s6) >= 0) goto loc_8002CCF0;
    v0 = lw(s3 + 0x4);
    sw(v0, s1 + 0x4);
    goto loc_8002CCF4;
loc_8002CCF0:
    sw(0, s1 + 0x4);
loc_8002CCF4:
    sw(s0, s1);
loc_8002CCF8:
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 0x15);                              // Result = 00000001
    s1 += 8;
    if (v0 != 0) goto loc_8002CD18;
    I_Error("FrontZClip: Point Overflow");
loc_8002CD18:
    s7++;
    v0 = (i32(s7) < i32(fp));
    s3 += 8;
    if (v0 != 0) goto loc_8002CADC;
loc_8002CD28:
    a2 = lw(sp + 0x18);
    sw(s2, a2);
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

void R_CheckEdgeVisible() noexcept {
loc_8002CD68:
    t0 = a1 + 4;
    a3 = 0x1F800000;                                    // Result = 1F800000
    a1 = lw(a1);
    a2 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_8002CDE0;
    v0 = (i32(a2) < i32(a1));
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8002CE38;
    t1 = 1;                                             // Result = 00000001
loc_8002CD90:
    v0 = lw(t0);
    v1 = lw(v0 + 0x10);
    v0 = lw(v0 + 0xC);
    v1 = -v1;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002CDBC;
    sw(t1, a3);
    a2--;
    goto loc_8002CDC4;
loc_8002CDBC:
    sw(0, a3);
    a2++;
loc_8002CDC4:
    a0++;
    t0 += 8;
    v0 = (i32(a0) < i32(a1));
    a3 += 4;
    if (v0 != 0) goto loc_8002CD90;
    goto loc_8002CE38;
loc_8002CDE0:
    v0 = (i32(a2) < i32(a1));
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8002CE38;
    t1 = 1;                                             // Result = 00000001
loc_8002CDF0:
    v0 = lw(t0);
    v1 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002CE1C;
    sw(t1, a3);
    a2--;
    goto loc_8002CE24;
loc_8002CE1C:
    sw(0, a3);
    a2++;
loc_8002CE24:
    a0++;
    t0 += 8;
    v0 = (i32(a0) < i32(a1));
    a3 += 4;
    if (v0 != 0) goto loc_8002CDF0;
loc_8002CE38:
    v0 = 0x1F800000;                                    // Result = 1F800000
    v0 = lw(v0);                                        // Load from: 1F800000
    sw(v0, a3);
    if (a2 != a1) goto loc_8002CE50;
    v0 = 0;                                             // Result = 00000000
    goto loc_8002CE60;
loc_8002CE50:
    v1 = -a1;
    v0 = -1;                                            // Result = FFFFFFFF
    if (a2 == v1) goto loc_8002CE60;
    v0 = 1;                                             // Result = 00000001
loc_8002CE60:
    return;
}

void R_LeftEdgeClip() noexcept {
loc_8002CE68:
    sp -= 0x48;
    sw(s3, sp + 0x2C);
    sw(a1, sp + 0x10);
    s3 = a1 + 4;
    sw(s6, sp + 0x38);
    s6 = 0;                                             // Result = 00000000
    sw(s4, sp + 0x30);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s5, sp + 0x34);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    s7 = lw(a0);
    fp = a0 + 4;
    if (i32(s7) <= 0) goto loc_8002D0CC;
    s1 = fp;
    s5 = 0x1F800000;                                    // Result = 1F800000
loc_8002CEC0:
    v0 = lw(s5);
    if (v0 != 0) goto loc_8002CEEC;
    v0 = lw(s1);
    v1 = lw(s1 + 0x4);
    sw(v0, s3);
    sw(v1, s3 + 0x4);
    s3 += 8;
    s4++;                                               // Result = 00000001
    v0 = lw(s5);
loc_8002CEEC:
    v1 = lw(s5 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002CF08;
    }
    {
        const bool bJump = (v1 != v0);
        v0 = s7 - 1;
        if (bJump) goto loc_8002D0B8;
    }
    goto loc_8002CF10;
loc_8002CF08:
    v0 = s7 - 1;
    if (v1 != 0) goto loc_8002D0B8;
loc_8002CF10:
    v0 = (i32(s6) < i32(v0));
    s2 = fp;
    if (v0 == 0) goto loc_8002CF20;
    s2 = s1 + 8;
loc_8002CF20:
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002CF60;
    I_Error("LeftEdgeClip: exceeded max new vertexes\n");
loc_8002CF60:
    v1 = lw(s1);
    a0 = lw(s2);
    v0 = lw(v1 + 0xC);
    a1 = lw(v1 + 0x10);
    v1 = lw(a0 + 0xC);
    a0 = lw(a0 + 0x10);
    v0 += a1;
    v1 += a0;
    a2 = v0 << 16;
    v0 -= v1;
    div(a2, v0);
    if (v0 != 0) goto loc_8002CF98;
    _break(0x1C00);
loc_8002CF98:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CFB0;
    }
    if (a2 != at) goto loc_8002CFB0;
    tge(zero, zero, 0x5D);
loc_8002CFB0:
    a2 = lo;
    a0 -= a1;
    mult(a2, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += a1;
    sw(v0, s0 + 0x10);
    v0 = -v0;
    sw(v0, s0 + 0xC);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002D03C;
    _break(0x1C00);
loc_8002D03C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D054;
    }
    if (v1 != at) goto loc_8002D054;
    tge(zero, zero, 0x5D);
loc_8002D054:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    sw(s0, s3);
    v0 = lw(s1 + 0x4);
    s4++;                                               // Result = 00000001
    sw(v0, s3 + 0x4);
    v0 = (i32(s4) < 0x15);                              // Result = 00000001
    s3 += 8;
    if (v0 != 0) goto loc_8002D0B8;
    I_Error("LeftEdgeClip: Point Overflow");
loc_8002D0B8:
    s1 += 8;
    s6++;
    v0 = (i32(s6) < i32(s7));
    s5 += 4;
    if (v0 != 0) goto loc_8002CEC0;
loc_8002D0CC:
    a3 = lw(sp + 0x10);
    v0 = s4;                                            // Result = 00000000
    sw(s4, a3);
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

void R_RightEdgeClip() noexcept {
loc_8002D10C:
    sp -= 0x48;
    sw(s3, sp + 0x2C);
    sw(a1, sp + 0x10);
    s3 = a1 + 4;
    sw(s6, sp + 0x38);
    s6 = 0;                                             // Result = 00000000
    sw(s4, sp + 0x30);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s5, sp + 0x34);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    s7 = lw(a0);
    fp = a0 + 4;
    if (i32(s7) <= 0) goto loc_8002D36C;
    s1 = fp;
    s5 = 0x1F800000;                                    // Result = 1F800000
loc_8002D164:
    v0 = lw(s5);
    if (v0 != 0) goto loc_8002D190;
    v0 = lw(s1);
    v1 = lw(s1 + 0x4);
    sw(v0, s3);
    sw(v1, s3 + 0x4);
    s3 += 8;
    s4++;                                               // Result = 00000001
    v0 = lw(s5);
loc_8002D190:
    v1 = lw(s5 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002D1AC;
    }
    {
        const bool bJump = (v1 != v0);
        v0 = s7 - 1;
        if (bJump) goto loc_8002D358;
    }
    goto loc_8002D1B4;
loc_8002D1AC:
    v0 = s7 - 1;
    if (v1 != 0) goto loc_8002D358;
loc_8002D1B4:
    v0 = (i32(s6) < i32(v0));
    s2 = fp;
    if (v0 == 0) goto loc_8002D1C4;
    s2 = s1 + 8;
loc_8002D1C4:
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002D204;
    I_Error("RightEdgeClip: exceeded max new vertexes\n");
loc_8002D204:
    v1 = lw(s1);
    a0 = lw(s2);
    v0 = lw(v1 + 0xC);
    a1 = lw(v1 + 0x10);
    v1 = lw(a0 + 0xC);
    a0 = lw(a0 + 0x10);
    v0 -= a1;
    v1 -= a0;
    a2 = v0 << 16;
    v0 -= v1;
    div(a2, v0);
    if (v0 != 0) goto loc_8002D23C;
    _break(0x1C00);
loc_8002D23C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D254;
    }
    if (a2 != at) goto loc_8002D254;
    tge(zero, zero, 0x5D);
loc_8002D254:
    a2 = lo;
    a0 -= a1;
    mult(a2, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += a1;
    sw(v0, s0 + 0x10);
    sw(v0, s0 + 0xC);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002D2DC;
    _break(0x1C00);
loc_8002D2DC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D2F4;
    }
    if (v1 != at) goto loc_8002D2F4;
    tge(zero, zero, 0x5D);
loc_8002D2F4:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    v1++;
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    sw(s0, s3);
    v0 = lw(s1 + 0x4);
    s4++;                                               // Result = 00000001
    sw(v0, s3 + 0x4);
    v0 = (i32(s4) < 0x15);                              // Result = 00000001
    s3 += 8;
    if (v0 != 0) goto loc_8002D358;
    I_Error("RightEdgeClip: Point Overflow");
loc_8002D358:
    s1 += 8;
    s6++;
    v0 = (i32(s6) < i32(s7));
    s5 += 4;
    if (v0 != 0) goto loc_8002D164;
loc_8002D36C:
    a3 = lw(sp + 0x10);
    v0 = s4;                                            // Result = 00000000
    sw(s4, a3);
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

#include "i_crossfade.h"

#include "i_drawcmds.h"
#include "i_main.h"
#include "PcPsx/Video.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Cross fades the currently displaying framebuffer with the currently drawn (but not yet displaying) framebuffer (back buffer).
// These original framebuffers are left untouched for the duration of the fade, and some of the texture cache VRAM must be overwritten for
// 2 fade framebuffers in order to do drawing and display for the cross fade effect.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CrossFadeFrameBuffers() noexcept {
    // Clear out what we can from the texture cache
    I_PurgeTexCache();

    // Setup the draw and display environments that we will ping-pong between while doing the cross fade.
    // Note that these will occupy some of the VRAM space normally used by the texture cache.
    DRAWENV drawEnvs[2];
    DISPENV dispEnvs[2];
    
    LIBGPU_SetDefDrawEnv(drawEnvs[0], 512, 256, 256, 240);
    drawEnvs[0].isbg = 1;
    drawEnvs[0].dtd = 0;
    drawEnvs[0].dfe = 1;

    LIBGPU_SetDefDrawEnv(drawEnvs[1], 768, 256, 256, 240);
    drawEnvs[1].isbg = 1;
    drawEnvs[1].dtd = 0;
    drawEnvs[1].dfe = 1;

    LIBGPU_SetDefDispEnv(dispEnvs[0], 768, 256, 256, 240);
    LIBGPU_SetDefDispEnv(dispEnvs[1], 512, 256, 256, 240);
    
    // Copy the current framebuffer to the 1st fade framebuffer, that will be initially displayed
    LIBGPU_MoveImage(gDispEnvs[*gCurDispBufferIdx].disp, 768, 256);
    LIBGPU_DrawSync(0);

    // Do the initial switch the framebuffers used for the fade
    LIBETC_VSync(0);
    LIBGPU_PutDrawEnv(drawEnvs[0]);
    LIBGPU_PutDispEnv(dispEnvs[0]);

    // Setup the quads used for rendering the cross fade.
    // Each one is sourcing its texture from a different framebuffer.
    POLY_FT4 polyPrim1;
    POLY_FT4 polyPrim2;

    LIBGPU_SetPolyFT4(polyPrim1);
    
    LIBGPU_setXY4(polyPrim1,
        0,              0,
        SCREEN_W - 1,   0,
        0,              SCREEN_H - 1,
        SCREEN_W - 1,   SCREEN_H - 1
    );

    LIBGPU_setUV4(polyPrim1,
        0,              0,
        SCREEN_W - 1,   0,
        0,              SCREEN_H - 1,
        SCREEN_W - 1,   SCREEN_H - 1
    );
    
    polyPrim1.clut = 0;
    polyPrim1.tpage = LIBGPU_GetTPage(2, 0, (*gCurDispBufferIdx == 0) ? 256 : 0, 0);
  
    LIBGPU_SetPolyFT4(polyPrim2);
    LIBGPU_SetSemiTrans(&polyPrim2, true);

    LIBGPU_setXY4(polyPrim2,
        0,              0,
        SCREEN_W - 1,   0,
        0,              SCREEN_H - 1,
        SCREEN_W - 1,   SCREEN_H - 1
    );

    LIBGPU_setUV4(polyPrim2,
        0,              0,
        SCREEN_W - 1,   0,
        0,              SCREEN_H - 1,
        SCREEN_W - 1,   SCREEN_H - 1
    );

    polyPrim2.tpage = LIBGPU_GetTPage(2, 0, (*gCurDispBufferIdx == 0) ? 0 : 256, 0);
    polyPrim2.clut = 0;

    // Run the cross fade until completion    
    uint32_t framebufferIdx = 0;
    
    for (int32_t fade = 255; fade >= 0; fade -= 5) {
        // Set the color for the 2 polygon primitves and submit
        LIBGPU_setRGB0(polyPrim1, (uint8_t)  fade, (uint8_t)  fade, (uint8_t)  fade);
        LIBGPU_setRGB0(polyPrim2, (uint8_t) ~fade, (uint8_t) ~fade, (uint8_t) ~fade);

        I_AddPrim(&polyPrim1);
        I_AddPrim(&polyPrim2);

        // Finish up drawing and swap the framebuffers
        framebufferIdx = framebufferIdx ^ 1;

        I_SubmitGpuCmds();
        LIBGPU_DrawSync(0);
        
        LIBETC_VSync(0);
        LIBGPU_PutDrawEnv(drawEnvs[framebufferIdx]);
        LIBGPU_PutDispEnv(dispEnvs[framebufferIdx]);

        // PC-PSX: copy the PSX framebuffer to the display
        #if PC_PSX_DOOM_MODS
            PcPsx::displayFramebuffer();
        #endif
    }
    
    I_SubmitGpuCmds();
    I_DrawPresent();
}

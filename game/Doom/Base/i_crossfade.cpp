#include "i_crossfade.h"

#include "i_drawcmds.h"
#include "i_main.h"
#include "i_texcache.h"
#include "PcPsx/Config.h"
#include "PcPsx/Input.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Video.h"
#include "PcPsx/Vulkan/VCrossfader.h"
#include "PcPsx/Vulkan/VRenderer.h"
#include "PcPsx/Vulkan/VRenderPath_Psx.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

#if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: called before a crossfade to do some bootstrap/setup.
// Should be called before rendering the the image to be faded into.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_PreCrossfade() noexcept {
    // We skip doing crossfades if fast loading is enabled...
    if (Config::gbUseFastLoading)
        return;

    #if PSYDOOM_VULKAN_RENDERER
        if (Video::gBackendType == Video::BackendType::Vulkan) {
            // If we are on the PSX render path ensure we will stay on it, render path switching is not allowed during crossfade.
            // Otherwise allow the crossfader module to do it's setup and schedule a render path switch.
            if (VRenderer::isUsingPsxRenderPath()) {
                VRenderer::setNextRenderPath(VRenderer::gRenderPath_Psx);
            } else {
                VCrossfader::doPreCrossfadeSetup();
            }
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cross fades the currently displaying framebuffer with the currently drawn (but not yet displaying) framebuffer (back buffer).
// These original framebuffers are left untouched for the duration of the fade, and some of the texture cache VRAM must be overwritten for
// 2 fade framebuffers in order to do drawing and display for the cross fade effect.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CrossfadeFrameBuffers() noexcept {
    // Clear out what we can from the texture cache
    I_PurgeTexCache();

    // We skip doing crossfades if fast loading is enabled...
    if (Config::gbUseFastLoading)
        return;

    // If we are using the Vulkan renderer then delegate to it's crossfader
    #if PSYDOOM_VULKAN_RENDERER
        if (Video::isUsingVulkanRenderPath()) {
            constexpr uint32_t NUM_VBLANKS = 52;
            VCrossfader::doCrossfade(NUM_VBLANKS);
            return;
        }
    #endif

    // Setup the draw and display environments that we will ping-pong between while doing the cross fade.
    // Note that these will occupy some of the VRAM space normally used by the texture cache.
    DRAWENV drawEnvs[2];
    DISPENV dispEnvs[2];

    LIBGPU_SetDefDrawEnv(drawEnvs[0], 512, 256, 256, 240);
    drawEnvs[0].isbg = true;
    drawEnvs[0].dtd = false;
    drawEnvs[0].dfe = true;

    LIBGPU_SetDefDrawEnv(drawEnvs[1], 768, 256, 256, 240);
    drawEnvs[1].isbg = true;
    drawEnvs[1].dtd = false;
    drawEnvs[1].dfe = true;

    LIBGPU_SetDefDispEnv(dispEnvs[0], 768, 256, 256, 240);
    LIBGPU_SetDefDispEnv(dispEnvs[1], 512, 256, 256, 240);

    // Copy the current framebuffer to the 1st fade framebuffer, that will be initially displayed
    LIBGPU_MoveImage(gDispEnvs[gCurDispBufferIdx].disp, 768, 256);
    LIBGPU_DrawSync(0);

    // Do the initial switch the framebuffers used for the fade
    LIBETC_VSync(0);
    LIBGPU_PutDrawEnv(drawEnvs[0]);
    LIBGPU_PutDispEnv(dispEnvs[0]);

    // Get the texture page id for the two framebuffers we will be rendering during the crossfade
    const uint16_t tpage1 = LIBGPU_GetTPage(2, 0, (gCurDispBufferIdx == 0) ? 256 : 0, 0);
    const uint16_t tpage2 = LIBGPU_GetTPage(2, 0, (gCurDispBufferIdx == 0) ? 0 : 256, 0);

    // Run the cross fade until completion
    uint32_t framebufferIdx = 0;
    int32_t lastTotalVBlanks = I_GetTotalVBlanks();

    for (int32_t fade = 255; fade >= 0; fade -= 5) {
        // PsyDoom: now drawing using sprites rather than polygons (as per the original code) to avoid the last row and column being clipped.
        // According to the NO$PSX specs: "Polygons are displayed up to <excluding> their lower-right coordinates."
        const uint8_t fade1 = (uint8_t)  fade;
        const uint8_t fade2 = (uint8_t) ~fade;
        I_DrawColoredSprite(tpage1, 0, 0, 0, 0, 0, SCREEN_W, SCREEN_H, fade1, fade1, fade1, false);
        I_DrawColoredSprite(tpage2, 0, 0, 0, 0, 0, SCREEN_W, SCREEN_H, fade2, fade2, fade2, true);

        // Finish up drawing and swap the framebuffers
        framebufferIdx = framebufferIdx ^ 1;

        I_SubmitGpuCmds();
        LIBGPU_DrawSync(0);

        LIBETC_VSync(0);
        LIBGPU_PutDrawEnv(drawEnvs[framebufferIdx]);
        LIBGPU_PutDispEnv(dispEnvs[framebufferIdx]);

        // PsyDoom: copy the PSX framebuffer to the display
        Video::displayFramebuffer();

        // PsyDoom: delay 1 vblank to limit fade speed
        int32_t curTotalVBlanks = I_GetTotalVBlanks();

        while (curTotalVBlanks <= lastTotalVBlanks) {
            Utils::doPlatformUpdates();
            Utils::threadYield();
            curTotalVBlanks = I_GetTotalVBlanks();
        }

        lastTotalVBlanks = curTotalVBlanks;

        // PsyDoom: if it's time to quit then exit early
        if (Input::isQuitRequested())
            break;
    }

    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if PSYDOOM_MODS

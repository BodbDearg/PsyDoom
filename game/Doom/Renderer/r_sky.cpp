#include "r_sky.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "r_data.h"
#include "r_main.h"

// How many bits to chop off for viewing angles to get the X offset to apply to the sky
static constexpr uint32_t ANGLETOSKYSHIFT = 22;

// The CLUT to use for the sky
const VmPtr<uint16_t> gPaletteClutId_CurMapSky(0x800782CC);

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the sky sprite.
// Unlike most other versions of DOOM, the PSX version draws the sky as a single sprite rather than individual columns.
// This means that the cylindrical mapping that is normally present is not here in this version of the game.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSky() noexcept {
    // Do we need to upload the fire sky texture? If so then upload it...
    // This code only executes for the fire sky - the regular sky is already in VRAM at this point.
    texture_t& skytex = *gpSkyTexture->get();

    if (skytex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {        
        const void* const pLumpData = (*gpLumpCache)[skytex.lumpNum].get();
        const uint16_t* const pTexData = (const std::uint16_t*) pLumpData + 4;      // TODO: find out why it is skipping two words and comment
        RECT vramRect = getTextureVramRect(skytex);

        LIBGPU_LoadImage(vramRect, pTexData);
        skytex.uploadFrameNum = *gNumFramesDrawn;
    }
    
    // Set the draw mode firstly
    {
        RECT textureWindow = {
            skytex.texPageCoordX,
            skytex.texPageCoordY,
            skytex.width,
            skytex.height
        };

        DR_MODE& drawMode = *(DR_MODE*) getScratchAddr(128);
        LIBGPU_SetDrawMode(drawMode, false, false, skytex.texPageId, &textureWindow);
        I_AddPrim(&drawMode);
    }

    // Setup and draw the sky sprite
    SPRT& spr = *(SPRT*) getScratchAddr(128);
    LIBGPU_SetSprt(spr);
    LIBGPU_SetShadeTex(&spr, true);
    LIBGPU_setXY0(spr, 0, 0);
    LIBGPU_setWH(spr, SCREEN_W, skytex.height);
    LIBGPU_setUV0(spr, (uint8_t)(skytex.texPageCoordX - (*gViewAngle >> ANGLETOSKYSHIFT)), skytex.texPageCoordY);
    spr.clut = *gPaletteClutId_CurMapSky;
    
    I_AddPrim(&spr);
}

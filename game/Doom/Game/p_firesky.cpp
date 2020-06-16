#include "p_firesky.h"

#include "Doom/Base/m_random.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Renderer/r_data.h"
#include "doomdata.h"
#include "PcPsx/Assert.h"

// This wraps x coordinates to 64 px bounds
static const uint8_t FIRESKY_X_WRAP_MASK = FIRESKY_W - 1;

// This RNG seed is used exclusively for the fire sky
static uint32_t gFireSkyRndIndex;

//------------------------------------------------------------------------------------------------------------------------------------------
// Does one update round/iteration of the famous PlayStation Doom 'fire sky' effect.
// After the effect is done, the fire sky texture is also invalidated, so that it is uploaded to VRAM next time it is drawn.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UpdateFireSky(texture_t& skyTex) noexcept {
    // Check that the texture is actually the size it's supposed to be...
    #if PC_PSX_DOOM_MODS
        ASSERT(skyTex.width == FIRESKY_W);
        ASSERT(skyTex.height == FIRESKY_H);
    #endif

    // Grab the lump data for the fire sky and the 1st (top) row of fire
    uint8_t* const pLumpData = (uint8_t*) gpLumpCache[skyTex.lumpNum];
    uint8_t* const pRow0 = pLumpData + sizeof(texlump_header_t);
    
    // Fire propagates up, so we always sample from a row below the destination
    uint8_t* pSrcRow = pRow0 + FIRESKY_W;

    // Loop through all the pixels in the fire sky except the bottom row and propagate the fire upwards and left/right
    for (int32_t x = 0; x < FIRESKY_W; ++x) {
        for (int32_t y = 1; y < FIRESKY_H; ++y) {
            // Destination row is 1 above the source
            uint8_t* const pDstRow = pSrcRow - FIRESKY_W;
            
            // Sample the 'temperature' in the source row.
            // If it's a dead pixel then just output a zero (black) pixel in the destination row:
            const uint8_t srcTemp = pSrcRow[x];

            if (srcTemp == 0) {
                pDstRow[x] = 0;
            } else {
                // Source pixel is not zero temp: propagate its 'heat' to the row above.
                // Vary destination x and heat decay randomly:
                const uint8_t dstXRand = gRndTable[gFireSkyRndIndex++ & 0xFF] & 3;
                const uint8_t tempRand = gRndTable[gFireSkyRndIndex++ & 0xFF] & 1;

                // Update the chosen pixel in the row above and do heat decay randomly
                const uint8_t dstX = (x + 1 - dstXRand) & FIRESKY_X_WRAP_MASK;
                pDstRow[dstX] = srcTemp - tempRand;
            }

            pSrcRow += FIRESKY_W;
        }

        // Rewind by the number of rows we processed so it's ready to process another round of rows
        pSrcRow -= (FIRESKY_W * (FIRESKY_H - 1));
    }

    // Mark the sky texture as 'not uploaded' to VRAM even though it may be there.
    // This invalidation causes it to be re-upoaded the next time it is drawn, so the updates done here will be visible.
    skyTex.uploadFrameNum = TEX_INVALID_UPLOAD_FRAME_NUM;
}

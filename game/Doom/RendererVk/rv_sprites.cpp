//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: things/sprites
//------------------------------------------------------------------------------------------------------------------------------------------

#if PSYDOOM_VULKAN_RENDERER

#include "rv_sprites.h"

#include "Doom/Base/i_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <vector>

// Entries for a subsector thing: stores the thing, it's xyz position (Doom xzy) and it's depth after being transformed
struct DepthThing {
    mobj_t* pThing;
    float   x, y, z;
    float   depth;
};

// Map-objects/things in the current subsector
static std::vector<DepthThing> gSubsecThings;

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets a list of things/map-objects in the given subsector, sorted back to front
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_GetSubsectorThings(const subsector_t& subsec, std::vector<DepthThing>& subsecThings) noexcept {
    // Get all of the things firstly and their depth
    sector_t& sector = *subsec.sector;

    for (mobj_t* pThing = sector.thinglist; pThing; pThing = pThing->snext) {
        // Ignore the thing if not in this subsector
        if (pThing->subsector != &subsec)
            continue;

        // Transform its xyz (Doom xzy) position by the view projection matrix to get the depth of the thing
        DepthThing& depthThing = subsecThings.emplace_back();
        depthThing.pThing = pThing;
        depthThing.x = RV_FixedToFloat(pThing->x);
        depthThing.y = RV_FixedToFloat(pThing->z);
        depthThing.z = RV_FixedToFloat(pThing->y);

        float viewXYZ[3];
        gViewProjMatrix.transform3d(&depthThing.x, viewXYZ);
        depthThing.depth = viewXYZ[2];
    }

    // Sort the thing list back to front
    std::sort(
        subsecThings.begin(),
        subsecThings.end(),
        [](const DepthThing& dt1, const DepthThing& dt2) noexcept {
            return (dt1.depth > dt2.depth);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get and cache the texture to use for the given thing and sprite frame, and get whether it is flipped.
// This code is copied more or less directly from 'R_DrawSubsectorSprites'.
//------------------------------------------------------------------------------------------------------------------------------------------
static texture_t& RV_CacheThingSpriteFrame(const mobj_t& thing, const spriteframe_t& frame, bool& bFlipSrite) noexcept {
    // Decide on which sprite lump to use and whether the sprite is flipped.
    // If the frame supports rotations then decide on the exact orientation to use, otherwise use the default.
    int32_t lumpNum;

    if (frame.rotate) {
        const angle_t angToThing = R_PointToAngle2(gViewX, gViewY, thing.x, thing.y);
        const uint32_t dirIdx = (angToThing - thing.angle + (ANG45 / 2) * 9) >> 29;     // Note: same calculation as PC Doom

        lumpNum = frame.lump[dirIdx];
        bFlipSrite = frame.flip[dirIdx];
    } else {
        lumpNum = frame.lump[0];
        bFlipSrite = frame.flip[0];
    }

    // Upload the sprite texture to VRAM if not already uploaded and return the texture to use
    const int32_t sprIndex = lumpNum - gFirstSpriteLumpNum;
    texture_t& tex = gpSpriteTextures[sprIndex];
    I_CacheTex(tex);
    return tex;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the given thing.
// Some of this code is copied directly from 'R_DrawSubsectorSprites'.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawThing(const DepthThing& depthThing, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept {
    // Grab the sprite frame to use and make sure it is resident in VRAM
    const mobj_t& thing = *depthThing.pThing;
    const spritedef_t& spriteDef = gSprites[thing.sprite];
    const spriteframe_t& frame = spriteDef.spriteframes[thing.frame & FF_FRAMEMASK];

    // Make sure the sprite is resident in VRAM and get whether it is flipped
    bool bFlipSprite = {};
    const texture_t& tex = RV_CacheThingSpriteFrame(thing, frame, bFlipSprite);

    // Get the texture window params for the sprite
    uint16_t texWinX;
    uint16_t texWinY;
    uint16_t texWinW;
    uint16_t texWinH;
    RV_GetTexWinXyWh(tex, texWinX, texWinY, texWinW, texWinH);

    // Determine sprite render alpha and the pipeline/blend-mode to use.
    // Note that semi-transparency multiply of '128' means fully opaque.
    uint8_t stMulR = 128;
    uint8_t stMulG = 128;
    uint8_t stMulB = 128;
    uint8_t stMulA = 128;
    VPipelineType drawPipeline = VPipelineType::View_Alpha;

    if (thing.flags & MF_BLEND_ON) {
        if (thing.flags & MF_BLEND_MODE_BIT1) {
            drawPipeline = VPipelineType::View_Additive;

            if (thing.flags & MF_BLEND_MODE_BIT2) {
                // Additive with 25% opacity
                stMulR = 32;
                stMulG = 32;
                stMulB = 32;
            } else {
                // Additive with 100% opacity ...
            }
        } else {
            if (thing.flags & MF_BLEND_MODE_BIT2) {
                // Subtractive blend with 100% opacity
                drawPipeline = VPipelineType::View_Subtractive;
            } else {
                // Alpha blend with 50% opacity
                drawPipeline = VPipelineType::View_Alpha;
                stMulA = 64;
            }
        }
    }

    // Aspect correction scaling copied from 'R_DrawSubsectorSprites'.
    // See the comments there for more about this...
    constexpr float ASPECT_CORRECT = 4.0f / 5.0f;

    // Get the width and height to draw the sprite with and offsetting to use
    const float spriteW = (float) tex.width * ASPECT_CORRECT;
    const float spriteH = (float) tex.height;
    const float offsetX = ((float) -tex.width + (float) tex.offsetX) * ASPECT_CORRECT;
    const float offsetY = - (float) tex.height + (float) tex.offsetY;

    // Get the x/y axis vectors for the view rotation: these will be used to construct the sprite billboard
    float axisX[4];
    float axisY[4];
    gSpriteBillboardMatrix.getRow(0, axisX);
    gSpriteBillboardMatrix.getRow(1, axisY);

    // Compute the worldspace coords of the 4 vertices used
    const float worldOffset[3] = {
        axisX[0] * offsetX + axisY[0] * offsetY,
        axisX[1] * offsetX + axisY[1] * offsetY,
        axisX[2] * offsetX + axisY[2] * offsetY
    };

    const float worldXSize[3] = { axisX[0] * spriteW, axisX[1] * spriteW, axisX[2] * spriteW };
    const float worldYSize[3] = { axisY[0] * spriteH, axisY[1] * spriteH, axisY[2] * spriteH };

    float p1[3] = { depthThing.x + worldOffset[0], depthThing.y + worldOffset[1], depthThing.z + worldOffset[2] };
    float p2[3] = { p1[0], p1[1], p1[2] };
    float p3[3] = { p1[0], p1[1], p1[2] };
    float p4[3] = { p1[0], p1[1], p1[2] };

    p2[0] += worldXSize[0];
    p2[1] += worldXSize[1];
    p2[2] += worldXSize[2];
    p3[0] += worldXSize[0];
    p3[1] += worldXSize[1];
    p3[2] += worldXSize[2];
    p3[0] += worldYSize[0];
    p3[1] += worldYSize[1];
    p3[2] += worldYSize[2];
    p4[0] += worldYSize[0];
    p4[1] += worldYSize[1];
    p4[2] += worldYSize[2];

    // Compute the UV coords for the sprite
    float ul = 0.0f;
    float ur = (float) tex.width * 0.5f;    // Halved beause the texture format is 8bpp instead of 16bpp
    float vt = 0.0f;
    float vb = spriteH;

    if (bFlipSprite) {
        std::swap(ul, ur);
    }

    // Decide what color to shade the sprite with
    uint8_t sprColR, sprColG, sprColB;

    if (thing.frame & FF_FULLBRIGHT) {
        sprColR = LIGHT_INTENSTIY_MAX;
        sprColG = LIGHT_INTENSTIY_MAX;
        sprColB = LIGHT_INTENSTIY_MAX;
    } else {
        sprColR = colR;
        sprColG = colG;
        sprColB = colB;
    }

    // Submit the triangles
    VDrawing::add3dViewTriangle(
        p1[0], p1[1], p1[2], ul, vb,
        p3[0], p3[1], p3[2], ur, vt,
        p2[0], p2[1], p2[2], ur, vb,
        sprColR, sprColG, sprColB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        VLightDimMode::None,
        drawPipeline,
        stMulR, stMulG, stMulB, stMulA
    );

    VDrawing::add3dViewTriangle(
        p3[0], p3[1], p3[2], ur, vt,
        p1[0], p1[1], p1[2], ul, vb,
        p4[0], p4[1], p4[2], ul, vt,
        sprColR, sprColG, sprColB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        VLightDimMode::None,
        drawPipeline,
        stMulR, stMulG, stMulB, stMulA
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws all sprites in the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsectorSprites(const subsector_t& subsec, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept {
    // Get a sorted list of things, draw each thing and then finish
    RV_GetSubsectorThings(subsec, gSubsecThings);

    for (const DepthThing& depthThing : gSubsecThings) {
        // Don't draw the player's thing!
        if (depthThing.pThing == gpViewPlayer->mo)
            continue;

        RV_DrawThing(depthThing, colR, colG, colB);
    }

    gSubsecThings.clear();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER

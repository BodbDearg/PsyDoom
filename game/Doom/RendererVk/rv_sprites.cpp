//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: things/sprites
//------------------------------------------------------------------------------------------------------------------------------------------

#if PSYDOOM_VULKAN_RENDERER

#include "rv_sprites.h"

#include "Doom/Base/i_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_bsp.h"
#include "rv_data.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a piece of a sprite.
// Sprites get split up by the infinitely high walls defined by BSP tree splits, on subsector boundaries.
// Splitting sprites allows the individual pieces to be ordered correctly in relation to world geometry and other sprites.
// This method is used by the recent Doom64 re-release, and is also suggested by John Carmack in his 1997 Doom source release notes.
//------------------------------------------------------------------------------------------------------------------------------------------
struct SpriteFrag {
    int32_t         nextSubsecFragIdx;                  // Index of the next sprite fragment in the linked list for the subsector
    float           depth;                              // Depth of the sprite fragment
    float           x1, z1;                             // 1st billboard endpoint: xz world position
    float           x2, z2;                             // 2nd billboard endpoint: xz world position
    float           yt, yb;                             // World top and bottom 'y' position
    float           ul, ur;                             // 'U' Texture coordinate for left and right side of the sprite
    float           vt, vb;                             // 'V' Texture coordinate for top and bottom of the sprite
    VPipelineType   drawPipeline;                       // Which pipeline to render the sprite with
    uint8_t         colR, colG, colB;                   // Color to shade the sprite with
    uint8_t         stMulR, stMulG, stMulB, stMulA;     // Semi-transparency multiply vector for semi-transparent pixels
    uint16_t        texWinX, texWinY;                   // Sprite texture window location
    uint16_t        texWinW, texWinH;                   // Sprite texture window size
};

// All of the sprite fragments to be drawn in this frame
static std::vector<SpriteFrag> gRvSpriteFrags;

// The sprite fragment linked list for each draw subsector (-1 if no sprite fragments)
static std::vector<int32_t> gRvDrawSubsecSprFrags;

// Depth sorted sprite fragments to be drawn for the current draw subsector.
// This temporary list is re-used for each subsector to avoid allocations.
static std::vector<const SpriteFrag*> gRvSortedFrags;

// XYZ position for the current thing which is having sprite fragments generated
static float gSpriteFragThingPos[3];

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
// Populates a sprite fragment entry (covering the entire sprite) for the given thing and using the specified sector color
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_InitSpriteFrag(
    mobj_t& thing,
    SpriteFrag& sprFrag,
    const uint8_t secR,
    const uint8_t secG,
    const uint8_t secB
) noexcept {
    // Transform its xyz (Doom xzy) position by the view projection matrix to obtain the depth of the thing.
    // This will be useful later for depth sorting.
    const float thingPos[3] = {
        RV_FixedToFloat(thing.x),
        RV_FixedToFloat(thing.z),
        RV_FixedToFloat(thing.y)
    };

    float thingDepth;

    {
        float viewXYZ[3];
        gViewProjMatrix.transform3d(thingPos, viewXYZ);
        thingDepth = viewXYZ[2];
    }

    // Grab the sprite frame to use
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
    // Note that a semi-transparency multiply of '128' means 100% strength (no change), or '1.0'.
    uint8_t stMulR = 128;
    uint8_t stMulG = 128;
    uint8_t stMulB = 128;
    uint8_t stMulA = 128;
    VPipelineType drawPipeline = VPipelineType::World_AlphaSprite;

    if (thing.flags & MF_BLEND_ON) {
        if (thing.flags & MF_BLEND_MODE_BIT1) {
            drawPipeline = VPipelineType::World_AdditiveSprite;

            if (thing.flags & MF_BLEND_MODE_BIT2) {
                // Additive blend with 25% opacity
                stMulR = 32;
                stMulG = 32;
                stMulB = 32;
            } else {
                // Additive blend with 100% opacity ...
            }
        } else {
            if (thing.flags & MF_BLEND_MODE_BIT2) {
                // Subtractive blend with 100% opacity
                drawPipeline = VPipelineType::World_SubtractiveSprite;
            } else {
                // Alpha blend with 50% opacity
                drawPipeline = VPipelineType::World_AlphaSprite;
                stMulA = 64;
            }
        }
    }

    // Aspect correction scaling value copied from 'R_DrawSubsectorSprites'.
    // See the comments there for more about this...
    constexpr float ASPECT_CORRECT = 4.0f / 5.0f;

    // Get the width and height to draw the sprite with and the offsetting to use
    const float spriteW = (float) tex.width * ASPECT_CORRECT;
    const float spriteH = (float) tex.height;
    const float offsetY = -(float) tex.height + (float) tex.offsetY;
    const float offsetX = bFlipSprite ? 
        (-(float) tex.width + (float) tex.offsetX) * ASPECT_CORRECT :
        (-(float) tex.offsetX) * ASPECT_CORRECT;

    // Get the x axis vector for the view rotation matrix: this will be used to construct the sprite billboard
    float axisX[4];
    gSpriteBillboardMatrix.getRow(0, axisX);

    // Compute the world space xyz position of the sprite (bottom left corner)
    const float worldPos[3] = {
        thingPos[0] + offsetX * axisX[0],
        thingPos[1] + offsetY,
        thingPos[2] + offsetX * axisX[2]
    };

    // Compute the worldspace xz coords for the endpoints and top/bottom of the sprite
    const float p1[2] = {
        worldPos[0],
        worldPos[2]
    };

    const float p2[2] = {
        worldPos[0] + spriteW * axisX[0],
        worldPos[2] + spriteW * axisX[2]
    };

    const float yb = { worldPos[1] };
    const float yt = { worldPos[1] + spriteH };

    // Compute the UV coords for the sprite
    float ul = 0.0f;
    float ur = tex.width;
    const float vt = 0.0f;
    const float vb = spriteH;

    if (bFlipSprite) {
        std::swap(ul, ur);
    }

    // Decide what color to shade the sprite with: some sprites are shaded at 125% intensity (fireballs etc.)
    uint8_t sprColR, sprColG, sprColB;

    if (thing.frame & FF_FULLBRIGHT) {
        sprColR = LIGHT_INTENSTIY_MAX;
        sprColG = LIGHT_INTENSTIY_MAX;
        sprColB = LIGHT_INTENSTIY_MAX;
    } else {
        sprColR = secR;
        sprColG = secG;
        sprColB = secB;
    }

    // Finally populate the sprite fragment
    sprFrag.nextSubsecFragIdx = -1;
    sprFrag.depth = thingDepth;
    sprFrag.x1 = p1[0];
    sprFrag.z1 = p1[1];
    sprFrag.x2 = p2[0];
    sprFrag.z2 = p2[1];
    sprFrag.yt = yt;
    sprFrag.yb = yb;
    sprFrag.ul = ul;
    sprFrag.ur = ur;
    sprFrag.vt = vt;
    sprFrag.vb = vb;
    sprFrag.drawPipeline = drawPipeline;
    sprFrag.colR = sprColR;
    sprFrag.colG = sprColG;
    sprFrag.colB = sprColB;
    sprFrag.stMulR = stMulR;
    sprFrag.stMulG = stMulG;
    sprFrag.stMulB = stMulB;
    sprFrag.stMulA = stMulA;
    sprFrag.texWinX = texWinX;
    sprFrag.texWinY = texWinY;
    sprFrag.texWinW = texWinW;
    sprFrag.texWinH = texWinH;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the given sprite fragment
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawSpriteFrag(const SpriteFrag& sprFrag) noexcept {
    VDrawing::setDrawPipeline(sprFrag.drawPipeline);
    VDrawing::addWorldQuad(
        sprFrag.x1, sprFrag.yb, sprFrag.z1, sprFrag.ul, sprFrag.vb,
        sprFrag.x2, sprFrag.yb, sprFrag.z2, sprFrag.ur, sprFrag.vb,
        sprFrag.x2, sprFrag.yt, sprFrag.z2, sprFrag.ur, sprFrag.vt,
        sprFrag.x1, sprFrag.yt, sprFrag.z1, sprFrag.ul, sprFrag.vt,
        sprFrag.colR, sprFrag.colG, sprFrag.colB,
        gClutX, gClutY,
        sprFrag.texWinX, sprFrag.texWinY,
        sprFrag.texWinW, sprFrag.texWinH,
        VLightDimMode::None,
        sprFrag.stMulR, sprFrag.stMulG, sprFrag.stMulB, sprFrag.stMulA
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the sprite fragment visit the specified subsector.
// Adds it to the draw list of sprite fragments for that subsector.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SpriteFrag_VisitSubsector(const subsector_t& subsec, const SpriteFrag& frag) noexcept {
    // If the subsector is not drawn then ignore and don't assign the sprite to a draw list
    const int32_t drawSubsecIdx = subsec.vkDrawSubsecIdx;

    if (drawSubsecIdx < 0)
        return;

    ASSERT(drawSubsecIdx < gRvDrawSubsecSprFrags.size());

    // Add this sprite fragment to the draw list for the subsector
    const int32_t sprFragIdx = (int32_t) gRvSpriteFrags.size();
    SpriteFrag& drawFrag = gRvSpriteFrags.emplace_back(frag);
    drawFrag.nextSubsecFragIdx = gRvDrawSubsecSprFrags[drawSubsecIdx];
    gRvDrawSubsecSprFrags[drawSubsecIdx] = sprFragIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used to check if a sprite fragment can be split against the specified seg.
// Checks to see if the seg is blocking and if the sprite fragment intersects it.
// Returns 'true' if the sprite fragment is allowed to be split against the seg.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_SpriteFrag_CanSplit_VisitSeg(const rvseg_t& seg, SpriteFrag& frag) noexcept {
    // Is the seg two sided? If so then don't consider it blocking provided the back sector floor height is <= the sprite height.
    if (seg.backsector) {
        if (RV_FixedToFloat(seg.backsector->floorheight) <= gSpriteFragThingPos[1])
            return true;
    }

    // Get the endpoints for the line for this segment.
    // Note: could intersect against just the segment, but doing the whole line adds more tolerance for error.
    const line_t& line = *seg.linedef;
    const vertex_t& lineV1 = *line.vertex1;
    const vertex_t& lineV2 = *line.vertex2;
    const float lineX1 = RV_FixedToFloat(lineV1.x);
    const float lineY1 = RV_FixedToFloat(lineV1.y);
    const float lineX2 = RV_FixedToFloat(lineV2.x);
    const float lineY2 = RV_FixedToFloat(lineV2.y);

    // Compute the intersection of two lines using the following equation method:
    //  http://paulbourke.net/geometry/pointlineplane/
    // See: "Intersection point of two line segments in 2 dimensions"
    const float lineDx = lineX2 - lineX1;
    const float lineDy = lineY2 - lineY1;
    const float fragDx = frag.x2 - frag.x1;
    const float fragDy = frag.z2 - frag.z1;
    
    const float a = frag.z1 - lineY1;
    const float b = frag.x1 - lineX1;

    const float numerator1 = (lineDx * a) - (lineDy * b);
    const float numerator2 = (fragDx * a) - (fragDy * b);
    const float denominator = lineDy * fragDx - lineDx * fragDy;

    // If the denominator is '0' then there is no intersection (parallel lines) and a split can happen
    if (denominator == 0)
        return true;

    // Otherwise the finite line segments intersect if 'tx' and 'ty' are between 0 and 1.
    // Note that these time values can be used as linear interpolation values to get the actual x and y intersect point.
    // We don't need that here though so don't bother...
    const float tx = numerator1 / denominator;
    const float ty = numerator2 / denominator;
    const bool bLinesIntersect = ((tx >= 0.0f) && (tx <= 1.0f) && (ty >= 0.0f) && (ty <= 1.0f));

    return (!bLinesIntersect);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used to check if a sprite fragment can be split against the specified subsector.
// Performs raycasts between the sprite endpoints to see if they cross blocking segs.
// Returns 'true' if a split is allowed.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_SpriteFrag_CanSplit_VisitSubsector(const subsector_t& subsec, SpriteFrag& frag) noexcept {
    // Check if the fragment can be split against all segs in the subsector
    const int32_t numSegs = subsec.numsegs;
    const rvseg_t* const pSegs = gpRvSegs.get() + subsec.firstseg;

    for (int32_t i = 0; i < numSegs; ++i) {
        if (!RV_SpriteFrag_CanSplit_VisitSeg(pSegs[i], frag))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used to check if a sprite fragment can be split against the specified BSP tree node.
// Tests to make sure the sprite fragment doesn't cross any lines that we don't want it to split against, like one sided walls.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_SpriteFrag_CanSplit_VisitBspNode(const int32_t nodeIdx, SpriteFrag& frag) noexcept {
    // Is this node number a subsector?
    if (nodeIdx & NF_SUBSECTOR) {
        // Note: this strange check is in the PC engine too...
        // Under what circumstances can the node number be '-1'?
        if (nodeIdx == -1) {
            return RV_SpriteFrag_CanSplit_VisitSubsector(gpSubsectors[0], frag);
        } else {
            return RV_SpriteFrag_CanSplit_VisitSubsector(gpSubsectors[nodeIdx & (~NF_SUBSECTOR)], frag);
        }
    } else {
        // This is not a subsector, continue traversing the BSP tree and testing against it
        node_t& node = gpBspNodes[nodeIdx];

        // Compute which side of the split the sprite endpoints are on using the cross product.
        // This is pretty much the same code found in 'R_PointOnSide':
        const float nodePx = RV_FixedToFloat(node.line.x);
        const float nodePy = RV_FixedToFloat(node.line.y);
        const float nodeDx = RV_FixedToFloat(node.line.dx);
        const float nodeDy = RV_FixedToFloat(node.line.dy);

        const float relX1 = frag.x1 - nodePx;
        const float relX2 = frag.x2 - nodePx;
        const float relZ1 = frag.z1 - nodePy;
        const float relZ2 = frag.z2 - nodePy;

        const float lprod1 = nodeDx * relZ1;
        const float rprod1 = nodeDy * relX1;
        const float lprod2 = nodeDx * relZ2;
        const float rprod2 = nodeDy * relX2;

        const int32_t side1 = (lprod1 < rprod1) ? 0 : 1;
        const int32_t side2 = (lprod2 < rprod2) ? 0 : 1;

        // Check both sides of the tree the sprite's billboard line is on
        if (!RV_SpriteFrag_CanSplit_VisitBspNode(node.children[side1], frag))
            return false;

        if (side1 != side2) {
            if (!RV_SpriteFrag_CanSplit_VisitBspNode(node.children[side2], frag))
                return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does recursive traversal of the BSP tree against the specified sprite fragment.
// Splits up the fragment along BSP split boundaries as needed and assigns the fragments to a destination subsector.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SpriteFrag_VisitBspNode(const int32_t nodeIdx, SpriteFrag& frag) noexcept {
    // Is this node number a subsector? If so then add the sprite fragment to it's draw lists
    if (nodeIdx & NF_SUBSECTOR) {
        // Note: this strange check is in the PC engine too...
        // Under what circumstances can the node number be '-1'?
        if (nodeIdx == -1) {
            RV_SpriteFrag_VisitSubsector(gpSubsectors[0], frag);
        } else {
            RV_SpriteFrag_VisitSubsector(gpSubsectors[nodeIdx & (~NF_SUBSECTOR)], frag);
        }
    } else {
        // This is not a subsector, continue traversing the BSP tree and splitting the sprite fragment
        node_t& node = gpBspNodes[nodeIdx];

        // Compute which side of the split the sprite endpoints are on using the cross product.
        // This is pretty much the same code found in 'R_PointOnSide':
        const float nodePx = RV_FixedToFloat(node.line.x);
        const float nodePy = RV_FixedToFloat(node.line.y);
        const float nodeDx = RV_FixedToFloat(node.line.dx);
        const float nodeDy = RV_FixedToFloat(node.line.dy);
        
        const float relX1 = frag.x1 - nodePx;
        const float relX2 = frag.x2 - nodePx;
        const float relZ1 = frag.z1 - nodePy;
        const float relZ2 = frag.z2 - nodePy;
        
        const float lprod1 = nodeDx * relZ1;
        const float rprod1 = nodeDy * relX1;
        const float lprod2 = nodeDx * relZ2;
        const float rprod2 = nodeDy * relX2;

        const bool bSide1 = (lprod1 < rprod1);
        const bool bSide2 = (lprod2 < rprod2);

        // Do we need to do a split or not?
        if (bSide1 == bSide2) {
            // No split needed, just recurse into the appropriate side
            if (bSide1) {
                RV_SpriteFrag_VisitBspNode(node.children[0], frag);
            } else {
                RV_SpriteFrag_VisitBspNode(node.children[1], frag);
            }
        } else {
            // Need to split (less common case): first check if a split against this BSP node is allowed
            if (!RV_SpriteFrag_CanSplit_VisitBspNode(nodeIdx, frag)) {
                // Can't split, decide which part of the tree to place the sprite in based on the sprite's center point.
                // If splits are not possible then ultimately we will put the thing's sprite in its natural subsector for rendering.
                const float lprod_center = nodeDx * (gSpriteFragThingPos[2] - nodePy);
                const float rprod_center = nodeDy * (gSpriteFragThingPos[0] - nodePx);
                const bool bCenterSide = (lprod_center < rprod_center);

                if (bCenterSide) {
                    RV_SpriteFrag_VisitBspNode(node.children[0], frag);
                } else {
                    RV_SpriteFrag_VisitBspNode(node.children[1], frag);
                }
            }
            else {
                // Can split! Decide how far along the sprite's billboard the split will happen.
                // First get the un-normalized normal vector for the node.
                const float nodeNx = -nodeDy;
                const float nodeNy = +nodeDx;

                // Compute the scaled perpendicular distance of each billboard point to the node plane
                const float dist1 = std::abs(relX1 * nodeNx + relZ1 * nodeNy);
                const float dist2 = std::abs(relX2 * nodeNx + relZ2 * nodeNy);

                // Compute the 'time' of the intersection/split
                const float splitT = std::clamp(dist1 / (dist1 + dist2), 0.0f, 1.0f);
                const float splitT_inv = 1.0f - splitT;

                // Produce a 2nd fragment by splitting the first, and modify the 1st fragment
                SpriteFrag frag2 = frag;
                frag2.x1 = frag.x1 * splitT_inv + frag.x2 * splitT;
                frag2.z1 = frag.z1 * splitT_inv + frag.z2 * splitT;
                frag2.ul = frag.ul * splitT_inv + frag.ur * splitT;

                SpriteFrag frag1 = frag;
                frag1.x2 = frag2.x1;
                frag1.z2 = frag2.z1;
                frag1.ur = frag2.ul;

                // Recurse using the split fragments.
                // Splits shouldn't happen TOO often so hopefully stack space should not be an issue.
                if (bSide1) {
                    RV_SpriteFrag_VisitBspNode(node.children[0], frag1);
                    RV_SpriteFrag_VisitBspNode(node.children[1], frag2);
                } else {
                    RV_SpriteFrag_VisitBspNode(node.children[1], frag1);
                    RV_SpriteFrag_VisitBspNode(node.children[0], frag2);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates all of the sprite fragments for sprites contained in the specfied subsector
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_BuildSubsectorSpriteFrags(const subsector_t& subsec, const int32_t drawSubsecIdx) noexcept {
    // Sanity check!
    ASSERT(drawSubsecIdx < gRvDrawSubsecs.size());

    // Early out if there are no things in the sector
    sector_t& sector = *subsec.sector;

    if (!sector.thinglist)
        return;

    // Get the light/color value for the sector
    uint8_t secR;
    uint8_t secG;
    uint8_t secB;
    RV_GetSectorColor(*subsec.sector, secR, secG, secB);

    // Build all fragments for this subsector
    for (mobj_t* pThing = sector.thinglist; pThing; pThing = pThing->snext) {
        // Ignore the thing if not in this subsector
        if (pThing->subsector != &subsec)
            continue;

        // Ignore this thing if it's the player
        if (pThing->player == gpViewPlayer)
            continue;

        // Allocate and initialize a full sprite fragment for the thing
        SpriteFrag sprFrag;
        RV_InitSpriteFrag(*pThing, sprFrag, secR, secG, secB);

        // Then split that fragment initially into two halves, with the division line being at the center.
        // This helps the sprite split along subsector boundaries as much as possible in each direction from it's center.
        // Due to the way the 'can split' tests work if we don't do this then the sprite might not split in some cases were we want it.
        SpriteFrag sprFrag1 = sprFrag;
        sprFrag1.x2 = (sprFrag.x1 + sprFrag1.x2) * 0.5f;
        sprFrag1.z2 = (sprFrag.z1 + sprFrag1.z2) * 0.5f;
        sprFrag1.ur = (sprFrag.ul + sprFrag1.ur) * 0.5f;

        SpriteFrag sprFrag2 = sprFrag;
        sprFrag2.x1 = sprFrag1.x2;
        sprFrag2.z1 = sprFrag1.z2;
        sprFrag2.ul = sprFrag1.ur;

        // Split up the sprite fragment halves if neccessary and remember the position of the thing being split.
        // The thing position is used to resolve cases that we can't split and decide on a sprite subsector.
        gSpriteFragThingPos[0] = RV_FixedToFloat(pThing->x);
        gSpriteFragThingPos[1] = RV_FixedToFloat(pThing->z);
        gSpriteFragThingPos[2] = RV_FixedToFloat(pThing->y);

        const int32_t bspRootNodeIdx = gNumBspNodes - 1;
        RV_SpriteFrag_VisitBspNode(bspRootNodeIdx, sprFrag1);
        RV_SpriteFrag_VisitBspNode(bspRootNodeIdx, sprFrag2);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds a list of all the sprite fragments to be drawn for this frame
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_BuildSpriteFragLists() noexcept {
    // Clear the list of sprite fragments to draw and init each draw subsector as having no sprite frags.
    // ALso prealloc a minimum amount of memory for all of the draw vectors.
    const int32_t numDrawSubsecs = (int32_t) gRvDrawSubsecs.size();

    gRvSpriteFrags.clear();
    gRvSpriteFrags.reserve(8192);
    gRvDrawSubsecSprFrags.clear();
    gRvDrawSubsecSprFrags.reserve(4196);
    gRvDrawSubsecSprFrags.resize((size_t) numDrawSubsecs, -1);
    gRvSortedFrags.reserve(256);

    // Run through all of the draw subsectors and build a list of sprite fragments for each
    for (int32_t drawSubsecIdx = 0; drawSubsecIdx < numDrawSubsecs; ++drawSubsecIdx) {
        RV_BuildSubsectorSpriteFrags(*gRvDrawSubsecs[drawSubsecIdx], drawSubsecIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw sprite fragments for the specified draw subsector index
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecSpriteFrags(const int32_t drawSubsecIdx) noexcept {
    // Firstly gather all of the sprite fragments for this draw subsector
    ASSERT(gRvSortedFrags.empty());
    ASSERT(drawSubsecIdx < gRvDrawSubsecs.size());

    int32_t nextSprIdx = gRvDrawSubsecSprFrags[drawSubsecIdx];
    const SpriteFrag* const pAllSprFrags = gRvSpriteFrags.data();

    while (nextSprIdx >= 0) {
        ASSERT(nextSprIdx < gRvSpriteFrags.size());
        const SpriteFrag& sprFrag = pAllSprFrags[nextSprIdx];
        gRvSortedFrags.emplace_back(&sprFrag);
        nextSprIdx = sprFrag.nextSubsecFragIdx;
    }

    // Sort all of the sprite fragments back to front
    std::sort(
        gRvSortedFrags.begin(),
        gRvSortedFrags.end(),
        [](const SpriteFrag* const pFrag1, const SpriteFrag* const pFrag2) noexcept {
            return (pFrag1->depth > pFrag2->depth);
        }
    );

    // Draw all the sorted fragments and clear the temporary list to finish up
    for (const SpriteFrag* const pSprFrag : gRvSortedFrags) {
        RV_DrawSpriteFrag(*pSprFrag);
    }

    gRvSortedFrags.clear();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER

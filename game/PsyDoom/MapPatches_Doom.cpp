//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to the original PlayStation Doom
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "DemoPlayer.h"
#include "Doom/Renderer/r_data.h"

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP04: Command Control
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandControl() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_Visual()) {
        // Fix bugs where step textures appear black: these steps need to have their 'lower unpegged' flag cleared
        modifyLinedefs(
            [](line_t& line) { line.flags &= ~ML_DONTPEGBOTTOM; },
            337, 746, 747, 748
        );

        // This step needs a texture coordinate adjustment
        gpSides[253].rowoffset = 0;
        gpSides[253].rowoffset.snap();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP13: Command Center
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandCenter() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a trap pillar containing a Baron not lowering.
        // The pillar doesn't lower because the Baron is stuck in the ceiling - fix by raising the ceiling.
        gpSectors[71].ceilingheight = 232 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP19: House Of Pain
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HouseOfPain() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a bug where an unintended door linedef causes one of the ceilings to collapse permanently.
        // Remove the line action that causes the bug.
        gpLines[435].special = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP20: Unholy Cathedral
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_UnholyCathedral() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix a secret door not opening because it requires the player to walk across it's line.
        // The door should be the yellow key also. Only apply this patch if not playing a classic demo because it affects gameplay!
        gpLines[505].special = 34;
    }

    if (shouldApplyMapPatches_Visual()) {
        // For the secret door bug above make sure the floor texture matches the outside floor texture (like on PC)
        modifySectors(
            [](sector_t& sector){ sector.floorpic = gpSectors[30].floorpic; },
            164, 165
        );

        // Fix a missing upper wall texture when the ceiling closes in the trap room with the BFG
        modifyLinedefs(
            [](line_t& line) {
                gpSides[line.sidenum[0]].toptexture = R_TextureNumForName("MARBLE04");
                gpSides[line.sidenum[1]].toptexture = R_TextureNumForName("MARBLE04");
            },
            950, 1078
        );

        // Fix the alignment of the scrolling skull wall textures
        auto setLineFrontTexOffset = [](const int32_t lineNum, const int32_t offsetX, const int32_t offsetY) noexcept {
            side_t& side = gpSides[gpLines[lineNum].sidenum[0]];
            side.textureoffset.snapToValue(offsetX * FRACUNIT);
            side.rowoffset.snapToValue(offsetY * FRACUNIT);
        };

        setLineFrontTexOffset(1113, 32, -48);   // Top skulls
        setLineFrontTexOffset(1114,  0, -48);
        setLineFrontTexOffset(1115, 32, -48);
    
        setLineFrontTexOffset(1118, 32,   0);   // Middle skulls
        setLineFrontTexOffset(1119,  0,  48);
        setLineFrontTexOffset(1120, 32,   0);

        setLineFrontTexOffset(1123, 32, -48);   // Bottom Skulls
        setLineFrontTexOffset(1125, 32, -48);
    }
}  

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP22: Limbo
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Limbo() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix a bug where a single step/lower-wall is not rendered.
        // This texture was not assigned in the original map, but was defaulted to texture index '0' by the original PSX Doom code in 'P_Init'.
        // PsyDoom now allows walls to deliberately have no texture assigned so they can be invisible, but that change causes a bug with this
        // step (beside the lift) not appearing in the room with the lift and the Baron. Fix by replicating the original PSX behavior.
        gpSides[967].bottomtexture = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP31: Entryway
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Entryway() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix grass beyond the sky wall sometimes appearing - don't render it!
        modifySectors(
            [](sector_t& sector) { sector.floorpic = -1; },
            3, 25
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP47: The Citadel
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCitadel() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix the starting hut: make it so you can see past it, since it is shorter than buildings around it
        addVoidFlagToLinedefs(170, 171, 172, 173, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP46: The Courtyard
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCourtyard() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_PsyDoom()) {
        // Fix a wall top/floor sometimes not appearing: this wall is at the starting area and borders the sky
        addVoidFlagToLinedefs(45, 17, 18, 51);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP58: The Mansion
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheMansion() noexcept {
    applyOriginalMapCommonPatches();

    if (shouldApplyMapPatches_GamePlay()) {
        // Fix the secret for the Club Doom exit often not registering.
        // Shift the secret to a sector where it should always be counted.
        gpSectors[63].special = 0;
        gpSectors[84].special = 9;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_Doom[] = {
    {  56435, 0x7921ADB466CEE45E, 0x4476F208866BF8A7, applyOriginalMapCommonPatches },      // MAP01
    { 119369, 0x4ED7CD6367900B52, 0x9609E85DB101DC09, applyOriginalMapCommonPatches },      // MAP02
    { 110284, 0x9BFF3A037128D1CA, 0x12F445D3F9B8BAC6, applyOriginalMapCommonPatches },      // MAP03
    {  92341, 0x1D79B5BDE5426081, 0x4E9413A01EAF4B4A, patchMap_CommandControl },            // MAP04
    {  89865, 0x0A8ACFFC833D6E36, 0x070A7A5CDDEE1CE0, applyOriginalMapCommonPatches },      // MAP05
    { 124094, 0x2097E86807523FF3, 0xA2F0C52632B12372, applyOriginalMapCommonPatches },      // MAP06
    { 108814, 0xD89ECAA4823454FD, 0xC7C178FA280CA569, applyOriginalMapCommonPatches },      // MAP07
    {  51882, 0x94BC7E609E1AC29A, 0xC1B6D482725C2C34, applyOriginalMapCommonPatches },      // MAP08
    {  47025, 0x492736BF0840ED38, 0x92A3AA841280B742, applyOriginalMapCommonPatches },      // MAP09
    {  97045, 0x48FFA0D005CB2DDA, 0x2631E9D5AB867200, applyOriginalMapCommonPatches },      // MAP10
    {  75368, 0x6D99C761DE799820, 0xAEDB0E4CA9441431, applyOriginalMapCommonPatches },      // MAP11
    { 119221, 0xB0E9622905A41337, 0xED94BA27D70017BF, applyOriginalMapCommonPatches },      // MAP12
    {  83505, 0x8635E6DB6360B27C, 0xD5835A25E276A0C4, patchMap_CommandCenter        },      // MAP13
    {  85802, 0x556287C93A6396F9, 0xC019D5F66797A596, applyOriginalMapCommonPatches },      // MAP14
    {  83539, 0xFDA28FD54C7E9A92, 0xE7F93F0E3C5C1D7F, applyOriginalMapCommonPatches },      // MAP15
    {  27956, 0x39B94C1CF5E19EB0, 0xE0A691816A8C166A, applyOriginalMapCommonPatches },      // MAP16
    {  56466, 0x4F240435B71CA6CA, 0xFA106C3EC5548BF0, applyOriginalMapCommonPatches },      // MAP17
    {  71253, 0x0541C17B11B2DC05, 0x577D152A01E48073, applyOriginalMapCommonPatches },      // MAP18
    {  75515, 0xFE716B01FE414A2A, 0xA3A7AFA1956DF697, patchMap_HouseOfPain },               // MAP19
    { 143483, 0x36A01960BAD36249, 0x2BC3BF03E0ED6D64, patchMap_UnholyCathedral },           // MAP20
    {  86538, 0x403A02FD929949E5, 0xB4185CB43CEA9B46, applyOriginalMapCommonPatches },      // MAP21
    { 109754, 0x1E3E66448FE6645C, 0x3DCA2CA78FC862F3, patchMap_Limbo },                     // MAP22
    {  32935, 0x55A24A4ED4053AC3, 0x636CDB24CE519EF8, applyOriginalMapCommonPatches },      // MAP23
    {  52915, 0xA8CCE876F52671B2, 0xDA2BB82C5D03383C, applyOriginalMapCommonPatches },      // MAP24
    {  72352, 0x255311EE3A46B4F4, 0x30E325760C3C0D55, applyOriginalMapCommonPatches },      // MAP25
    { 111520, 0x85B038429CCD933B, 0x8488BBE9B15A5F8C, applyOriginalMapCommonPatches },      // MAP26
    {  82104, 0x52B9EDF6AA65FD8C, 0x3D965AFD07455BA6, applyOriginalMapCommonPatches },      // MAP27
    { 146652, 0x1C5AD3B2CC520748, 0x79223365451D6965, applyOriginalMapCommonPatches },      // MAP28
    { 163970, 0x85E5F59863FC567A, 0x825E1D627586324B, applyOriginalMapCommonPatches },      // MAP29
    { 146600, 0x0776A66BD2962C70, 0xEA25B44BFB2863F0, applyOriginalMapCommonPatches },      // MAP30
    {  46210, 0x41EA6956972B2510, 0xE4760C46A4BBD40D, patchMap_Entryway     },              // MAP31
    {  63255, 0x787980722B2A3ABF, 0xDA758F7A7236BAD9, applyOriginalMapCommonPatches },      // MAP32
    {  71907, 0x9354072B9094E9BE, 0xFDA856CDE67680DC, applyOriginalMapCommonPatches },      // MAP33
    {  67614, 0xE36C70A633E0AE7D, 0x9223DF3ADFDF8808, applyOriginalMapCommonPatches },      // MAP34
    { 114123, 0x52229ABCD304D8BA, 0x6EAEA8DB75133B5A, applyOriginalMapCommonPatches },      // MAP35
    { 129248, 0xE2245D687CCABC7C, 0x01497DF00B763463, applyOriginalMapCommonPatches },      // MAP36
    {  26682, 0x2B0A8D80B5411593, 0x3A427EE05B7353F6, applyOriginalMapCommonPatches },      // MAP37
    {  82063, 0xBFEDBDE9F8B8CCE2, 0x78D6E2C3A9AB74AB, applyOriginalMapCommonPatches },      // MAP38
    {  91388, 0x22B7D106F531FB4E, 0xFE3FAB276C892BD4, applyOriginalMapCommonPatches },      // MAP39
    { 130676, 0xD84B13024E326B64, 0x548472C7F8B24A27, applyOriginalMapCommonPatches },      // MAP40
    { 116024, 0x59800E5259D02FD8, 0x28EB273CFC8E41CC, applyOriginalMapCommonPatches },      // MAP41
    { 109934, 0x7E22F4311F3955D5, 0x16E918F5C11AD780, applyOriginalMapCommonPatches },      // MAP42
    { 192997, 0x7B86B9C35B754883, 0xD5F5CE44AB12898D, applyOriginalMapCommonPatches },      // MAP43
    { 110145, 0xE296122ADE38AB74, 0x13505BF841234D4C, applyOriginalMapCommonPatches },      // MAP44
    { 158462, 0x37D6A1335F058A41, 0xA82656A6FDEB132B, applyOriginalMapCommonPatches },      // MAP45
    { 105883, 0x0CA0922874005BC1, 0x37173A0C68F8FA6A, patchMap_TheCourtyard },              // MAP46
    { 186755, 0x73E10EF08AE21FD5, 0x8115F467FE2CD3CA, patchMap_TheCitadel   },              // MAP47
    {  54866, 0xF41440631C2B6FB2, 0x728E55510D5AE858, applyOriginalMapCommonPatches },      // MAP48
    {  74303, 0x522256004AD8E073, 0x2C190C108C98B31D, applyOriginalMapCommonPatches },      // MAP49
    {  64540, 0x47EA67DBBA5F33DC, 0x2280784D842FECC1, applyOriginalMapCommonPatches },      // MAP50
    { 106555, 0x9FCBB09C2A8C8B67, 0xF00080B9655646C8, applyOriginalMapCommonPatches },      // MAP51
    { 117839, 0x67138B444A196EC4, 0x229285E95F31ADE4, applyOriginalMapCommonPatches },      // MAP52
    { 131947, 0xC966739D25AC3FFD, 0xB7CDA8E3CF9A5186, applyOriginalMapCommonPatches },      // MAP53
    {  45962, 0x27D515F2A59962E3, 0x3E35ABAD09E87EA1, applyOriginalMapCommonPatches },      // MAP54
    {  19237, 0xB5116FF7C0CBCF38, 0x7C6C9E29F2EA963B, applyOriginalMapCommonPatches },      // MAP55
    {  85042, 0x98C00035EA735BF3, 0x36C5C0BA592334C9, applyOriginalMapCommonPatches },      // MAP56
    {  58333, 0xBBE3159AAEE4F03D, 0x086F778E5A08DBAD, applyOriginalMapCommonPatches },      // MAP57
    { 194653, 0x158C832CA1D1C539, 0x9C4ED57B29C13E66, patchMap_TheMansion           },      // MAP58
    {  79441, 0x0ECE269F1AA74445, 0x0B254FDF53895D4F, applyOriginalMapCommonPatches },      // MAP59
};

const PatchList gPatches_Doom = { gPatchArray_Doom, C_ARRAY_SIZE(gPatchArray_Doom) };

END_NAMESPACE(MapPatches)

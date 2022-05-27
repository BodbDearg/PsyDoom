//------------------------------------------------------------------------------------------------------------------------------------------
// A module containing map patches to apply to PlayStation Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatches.h"

#include "Doom/Renderer/r_data.h"

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP09: Nessus
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Nessus() noexcept {
    applyOriginalMapCommonPatches();

    // Fix the BFG secret being inaccessible - transfer it to a neighboring sector:
    gpSectors[57].special = 0;
    gpSectors[60].special = 9;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP21: Lunar Mining Project
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_LunarMiningProject() noexcept {
    applyOriginalMapCommonPatches();

    // Fix a missing texture on a small lip in the mines
    gpSides[gpLines[718].sidenum[1]].bottomtexture = R_TextureNumForName("ROCK06");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix issues for MAP23: Ballistyx
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Ballistyx() noexcept {
    applyOriginalMapCommonPatches();

    // Make various linedefs not render sky walls or be see through for occlusion purposes
    addVoidFlagToLinedefs(
        // Altar hole: don't draw sky walls
        1212, 1211, 1215, 1210, 1214, 1213,
        // Altar pillars: don't occlude and prevent geometry behind from rendering (needed for floating platform hack)
        1216, 1291, 1290, 1207, 1209, 1292, 1293, 1199,
        // Yellow key cage area: the outer wall floors sometimes don't render due to occluding sky walls; make them not occlude:
        1525, 1064, 1526, 1052, 371, 1053, 374, 1509, 373, 1054, 1524, 1055
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map patches for this game type
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatchArray_FinalDoom[] = {
    { 149065, 0xEB9558B4F55AB3B1, 0xEFF704814B4411D6, applyOriginalMapCommonPatches },      // MAP01
    { 117111, 0xADAA213B6086EE11, 0x978D2CC01B168F5D, applyOriginalMapCommonPatches },      // MAP02 (NTSC)
    { 117187, 0xAA3783F676D9B0CE, 0x9DE393E81F9FF509, applyOriginalMapCommonPatches },      // MAP02 (PAL, why different?)
    { 135628, 0xC80C9E89D5679E63, 0x7AD86A771493F84E, applyOriginalMapCommonPatches },      // MAP03
    {  88648, 0xCA130E217D79594D, 0x1241B18E3E7D8110, applyOriginalMapCommonPatches },      // MAP04
    { 132897, 0x2E8E3492E704154D, 0x0DE2F0470D62DDB8, applyOriginalMapCommonPatches },      // MAP05
    {  88004, 0xC12D7BD6D414250C, 0xF981017C0C8ADF20, applyOriginalMapCommonPatches },      // MAP06
    { 165920, 0x903B721BA84B1FFD, 0xCED86BF62E5CE0BE, applyOriginalMapCommonPatches },      // MAP07
    { 151747, 0x93EA3A4DE9DA978B, 0x3D27F6255CA0B9CC, applyOriginalMapCommonPatches },      // MAP08
    { 102104, 0x1504DC20E04BE8F1, 0x3A63FD22BC9C0D8C, patchMap_Nessus               },      // MAP09
    { 139820, 0x5EDEF8B2A51779E8, 0x8D1314A4F889EFCC, applyOriginalMapCommonPatches },      // MAP10
    {  96211, 0x42B2A3CE9B37CA2A, 0x72D00C8E1681AEB4, applyOriginalMapCommonPatches },      // MAP11
    { 106776, 0xAD3AADE890018818, 0x7D70AC984E7211CC, applyOriginalMapCommonPatches },      // MAP12
    { 152855, 0xD4905C759C1713E1, 0x1B78CCD5275A40EB, applyOriginalMapCommonPatches },      // MAP13
    {  54706, 0x979F686C4297312E, 0xB9EA33C07E20F4E3, applyOriginalMapCommonPatches },      // MAP14
    {  77891, 0x20F93855131B2C1A, 0xD98E0D6C4EAEC765, applyOriginalMapCommonPatches },      // MAP15
    { 156972, 0xC4DF66BEDEE0E1C4, 0xFB56E82FA017FD9D, applyOriginalMapCommonPatches },      // MAP16
    { 179622, 0x97DFE2C07BE92D3C, 0xEC29BA71305623B3, applyOriginalMapCommonPatches },      // MAP17
    { 131823, 0xADD51543E9578AB7, 0xA3E479551A015464, applyOriginalMapCommonPatches },      // MAP18
    { 177868, 0x5BDC5BC7E62822C1, 0x3F374AD0091C79F1, applyOriginalMapCommonPatches },      // MAP19
    { 105404, 0x5849A9F98647AF13, 0x59C891E67F19FC69, applyOriginalMapCommonPatches },      // MAP20
    { 162561, 0x5BA4490CA5C13E9A, 0x23D505C31AF4CADF, patchMap_LunarMiningProject   },      // MAP21
    {  96826, 0x9B6446A94907229A, 0x6DC9F5EDDB9D4F2D, applyOriginalMapCommonPatches },      // MAP22
    { 167847, 0x3BC3E6570C2D06B3, 0x18756B0D2C98BE86, patchMap_Ballistyx },                 // MAP23
    { 121920, 0x445D7FDA25066B71, 0xAC3893B22E188D4D, applyOriginalMapCommonPatches },      // MAP24
    { 113719, 0xFBA63EF7487AB574, 0xE21B77623A0DE2AA, applyOriginalMapCommonPatches },      // MAP25
    { 127601, 0x1008C54A53E8B33E, 0x8E35C49173174DCD, applyOriginalMapCommonPatches },      // MAP26
    { 113829, 0x25A6925BB713C346, 0x7AF7C07603DEA325, applyOriginalMapCommonPatches },      // MAP27
    { 141807, 0x3461BD1E919965AB, 0x07C36C7B648205F6, applyOriginalMapCommonPatches },      // MAP28
    { 107736, 0xD9789CCEA024CCCC, 0x61CCB6C421B65C47, applyOriginalMapCommonPatches },      // MAP29 (NTSC)
    { 107736, 0x0599BE06504C6FAD, 0x1DCB1C8AD6410764, applyOriginalMapCommonPatches },      // MAP29 (PAL, why different?)
    { 110131, 0x2C157281E504283E, 0x914845A33B9F0503, applyOriginalMapCommonPatches },      // MAP30
};

const PatchList gPatches_FinalDoom = { gPatchArray_FinalDoom, C_ARRAY_SIZE(gPatchArray_FinalDoom) };

END_NAMESPACE(MapPatches)

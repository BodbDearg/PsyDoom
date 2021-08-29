//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows patching original game maps to fix a few select issues.
// Provides mechanisms to check when patches should be applied and calls patch functions to do the modifications.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatcher.h"

#include "Asserts.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_local.h"
#include "Game.h"
#include "MapHash.h"

BEGIN_NAMESPACE(MapPatcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for a function that patches map data
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*PatcherFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a patch to be applied to map data.
// Defines when the patch is applied and the function that will apply the patch.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchDef {
    int32_t         mapSize;        // Filter: combined size (in bytes) of all the lumps for the map
    uint64_t        md5Word1;       // Filter: The MD5 for the combined map lump data (bytes 0-7)
    uint64_t        md5Word2;       // Filter: The MD5 for the combined map lump data (bytes 8-15)
    PatcherFunc     patcherFunc;    // The function that does the work of patching
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static void modifyLinedefs([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List> 
static void modifyLinedefs(const ModFuncT & func, const int32_t linedefIdx, Int32List... linedefIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLinedefs(func, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears mysterious sector flags (of unknown purpose) other than 'SF_NO_REVERB' found in original game maps.
// 
// The 'SF_NO_REVERB' flag was the only one originally used by the retail version of the game but for some reason various sectors in the
// game can use any of the 16 available bits as sector flags. This would not be a problem except that PsyDoom now assigns meaning to the
// most of the flag bits (for 2-colored lighting and other features) and this can cause issues for original game maps.
// Hence we must clear all sector flag bits (other than the 1st) for original game maps.
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearMysterySectorFlags() noexcept {
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        // Note: flags is now only interpreted as 8-bits in the level data, the other 8-bits are used for 'ceilColorid'
        pSectors[i].flags &= (~SF_GHOSTPLAT);

        // Originally this was the high 8-bits of the 'flags' field.
        // Original maps never use 2 colored lighting so just set it to the floor color.
        pSectors[i].ceilColorid = pSectors[i].colorid;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Common patches applied to ALL original game maps
//------------------------------------------------------------------------------------------------------------------------------------------
static void applyCommonMapPatches() noexcept {
    clearMysterySectorFlags();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix bugs in Doom MAP04, 'Command Control' where step textures appear black
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_CommandControl() noexcept {
    applyCommonMapPatches();

    // These steps need to have their 'lower unpegged' flag cleared
    modifyLinedefs(
        [](line_t& line) { line.flags &= ~ML_DONTPEGBOTTOM; },
        337, 746, 747, 748
    );

    // This step needs a texture coordinate adjustment
    gpSides[253].rowoffset = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a bug in Doom with MAP19 ('House Of Pain') where an unintended door linedef causes one of the ceilings to collapse permanently.
// Removes the line action which causes the bug.
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_HouseOfPain() noexcept {
    applyCommonMapPatches();
    gpLines[435].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a bug in Doom with MAP22 ('Limbo') where a single step/lower-wall is not rendered
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Limbo() noexcept {
    applyCommonMapPatches();

    // This texture was not assigned in the original map, but was defaulted to texture index '0' by the original PSX Doom code in 'P_Init'.
    // PsyDoom now allows walls to deliberately have no texture assigned so they can be invisible, but that change causes a bug with this
    // step (beside the lift) not appearing in the room with the lift and the Baron. Fix by replicating the original PSX behavior.
    gpSides[967].bottomtexture = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fixes the starting hut for MAP47, 'The Citadel' for Doom.
// Makes it so you can see past it, since it is shorter than buildings around it.
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_TheCitadel() noexcept {
    applyCommonMapPatches();
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        170, 171, 172, 173, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix various issues in MAP23, 'Ballistyx' for Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
static void patchMap_Ballistyx() noexcept {
    applyCommonMapPatches();

    // Make various linedefs not render sky walls or be see through for occlusion purposes
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        // Altar hole: don't draw sky walls
        1212, 1211, 1215, 1210, 1214, 1213,
        // Altar pillars: don't occlude and prevent geometry behind from rendering (needed for floating platform hack)
        1216, 1291, 1290, 1207, 1209, 1292, 1293, 1199,
        // Yellow key cage area: the outer wall floors sometimes don't render due to occluding sky walls; make them not occlude:
        1525, 1064, 1526, 1052, 371, 1053, 374, 1509, 373, 1054, 1524, 1055
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map fixes to apply
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatches[] = {
    // Doom
    {  56435, 0x5EE4CE66B4AD2179, 0xA7F86B8608F27644, applyCommonMapPatches },          // MAP01
    { 119369, 0x520B906763CDD74E, 0x09DC01B15DE80996, applyCommonMapPatches },          // MAP02
    { 110284, 0xCAD12871033AFF9B, 0xC6BAB8F9D345F412, applyCommonMapPatches },          // MAP03
    {  92341, 0x816042E5BDB5791D, 0x4A4BAF1EA013944E, patchMap_CommandControl },        // MAP04
    {  89865, 0x366E3D83FCCF8A0A, 0xE01CEEDD5C7A0A07, applyCommonMapPatches },          // MAP05
    { 124094, 0xF33F520768E89720, 0x7223B13226C5F0A2, applyCommonMapPatches },          // MAP06
    { 108814, 0xFD543482A4CA9ED8, 0x69A50C28FA78C1C7, applyCommonMapPatches },          // MAP07
    {  51882, 0x9AC21A9E607EBC94, 0x342C5C7282D4B6C1, applyCommonMapPatches },          // MAP08
    {  47025, 0x38ED4008BF362749, 0x42B7801284AAA392, applyCommonMapPatches },          // MAP09
    {  97045, 0xDA2DCB05D0A0FF48, 0x007286ABD5E93126, applyCommonMapPatches },          // MAP10
    {  75368, 0x209879DE61C7996D, 0x311444A94C0EDBAE, applyCommonMapPatches },          // MAP11
    { 119221, 0x3713A4052962E9B0, 0xBF1700D727BA94ED, applyCommonMapPatches },          // MAP12
    {  83505, 0x7CB26063DBE63586, 0xC4A076E2255A83D5, applyCommonMapPatches },          // MAP13
    {  85802, 0xF996633AC9876255, 0x96A59767F6D519C0, applyCommonMapPatches },          // MAP14
    {  83539, 0x929A7E4CD58FA2FD, 0x7F1D5C3C0E3FF9E7, applyCommonMapPatches },          // MAP15
    {  27956, 0xB09EE1F51C4CB939, 0x6A168C6A8191A6E0, applyCommonMapPatches },          // MAP16
    {  56466, 0xCAA61CB73504244F, 0xF08B54C53E6C10FA, applyCommonMapPatches },          // MAP17
    {  71253, 0x05DCB2117BC14105, 0x7380E4012A157D57, applyCommonMapPatches },          // MAP18
    {  75515, 0x2A4A41FE016B71FE, 0x97F66D95A1AFA7A3, patchMap_HouseOfPain },           // MAP19
    { 143483, 0x4962D3BA6019A036, 0x646DEDE003BFC32B, applyCommonMapPatches },          // MAP20
    {  86538, 0xE5499992FD023A40, 0x469BEA3CB45C18B4, applyCommonMapPatches },          // MAP21
    { 109754, 0x5C64E68F44663E1E, 0xF362C88FA72CCA3D, patchMap_Limbo },                 // MAP22
    {  32935, 0xC33A05D44E4AA255, 0xF89E51CE24DB6C63, applyCommonMapPatches },          // MAP23
    {  52915, 0xB27126F576E8CCA8, 0x3C38035D2CB82BDA, applyCommonMapPatches },          // MAP24
    {  72352, 0xF4B4463AEE115325, 0x550D3C0C7625E330, applyCommonMapPatches },          // MAP25
    { 111520, 0x3B93CD9C4238B085, 0x8C5F5AB1E9BB8884, applyCommonMapPatches },          // MAP26
    {  82104, 0x8CFD65AAF6EDB952, 0xA65B4507FD5A963D, applyCommonMapPatches },          // MAP27
    { 146652, 0x480752CCB2D35A1C, 0x65691D4565332279, applyCommonMapPatches },          // MAP28
    { 163970, 0x7A56FC6398F5E585, 0x4B328675621D5E82, applyCommonMapPatches },          // MAP29
    { 146600, 0x702C96D26BA67607, 0xF06328FB4BB425EA, applyCommonMapPatches },          // MAP30
    {  46210, 0x10252B975669EA41, 0x0DD4BBA4460C76E4, applyCommonMapPatches },          // MAP31
    {  63255, 0xBF3A2A2B72807978, 0xD9BA36727A8F75DA, applyCommonMapPatches },          // MAP32
    {  71907, 0xBEE994902B075493, 0xDC8076E6CD56A8FD, applyCommonMapPatches },          // MAP33
    {  67614, 0x7DAEE033A6706CE3, 0x0888DFDF3ADF2392, applyCommonMapPatches },          // MAP34
    { 114123, 0xBAD804D3BC9A2252, 0x5A3B1375DBA8AE6E, applyCommonMapPatches },          // MAP35
    { 129248, 0x7CBCCA7C685D24E2, 0x6334760BF07D4901, applyCommonMapPatches },          // MAP36
    {  26682, 0x931541B5808D0A2B, 0xF653735BE07E423A, applyCommonMapPatches },          // MAP37
    {  82063, 0xE2CCB8F8E9BDEDBF, 0xAB74ABA9C3E2D678, applyCommonMapPatches },          // MAP38
    {  91388, 0x4EFB31F506D1B722, 0xD42B896C27AB3FFE, applyCommonMapPatches },          // MAP39
    { 130676, 0x646B324E02134BD8, 0x274AB2F8C7728454, applyCommonMapPatches },          // MAP40
    { 116024, 0xD82FD059520E8059, 0xCC418EFC3C27EB28, applyCommonMapPatches },          // MAP41
    { 109934, 0xD555391F31F4227E, 0x80D71AC1F518E916, applyCommonMapPatches },          // MAP42
    { 192997, 0x8348755BC3B9867B, 0x8D8912AB44CEF5D5, applyCommonMapPatches },          // MAP43
    { 110145, 0x74AB38DE2A1296E2, 0x4C4D2341F85B5013, applyCommonMapPatches },          // MAP44
    { 158462, 0x418A055F33A1D637, 0x2B13EBFDA65626A8, applyCommonMapPatches },          // MAP45
    { 105883, 0xC15B00742892A00C, 0x6AFAF8680C3A1737, applyCommonMapPatches },          // MAP46
    { 186755, 0xD51FE28AF00EE173, 0xCAD32CFE67F41581, patchMap_TheCitadel },            // MAP47
    {  54866, 0xB26F2B1C634014F4, 0x58E85A0D51558E72, applyCommonMapPatches },          // MAP48
    {  74303, 0x73E0D84A00562252, 0x1DB3988C100C192C, applyCommonMapPatches },          // MAP49
    {  64540, 0xDC335FBADB67EA47, 0xC1EC2F844D788022, applyCommonMapPatches },          // MAP50
    { 106555, 0x678B8C2A9CB0CB9F, 0xC8465665B98000F0, applyCommonMapPatches },          // MAP51
    { 117839, 0xC46E194A448B1367, 0xE4AD315FE9859222, applyCommonMapPatches },          // MAP52
    { 131947, 0xFD3FAC259D7366C9, 0x86519ACFE3A8CDB7, applyCommonMapPatches },          // MAP53
    {  45962, 0xE36299A5F215D527, 0xA17EE809ADAB353E, applyCommonMapPatches },          // MAP54
    {  19237, 0x38CFCBC0F76F11B5, 0x3B96EAF2299E6C7C, applyCommonMapPatches },          // MAP55
    {  85042, 0xF35B73EA3500C098, 0xC9342359BAC0C536, applyCommonMapPatches },          // MAP56
    {  58333, 0x3DF0E4AE9A15E3BB, 0xADDB085A8E776F08, applyCommonMapPatches },          // MAP57
    { 194653, 0x39C5D1A12C838C15, 0x663EC1297BD54E9C, applyCommonMapPatches },          // MAP58
    {  79441, 0x4544A71A9F26CE0E, 0x4F5D8953DF4F250B, applyCommonMapPatches },          // MAP59
    // Final Doom
    { 149065, 0xB1B35AF5B45895EB, 0xD611444B8104F7EF, applyCommonMapPatches },          // MAP01
    { 117111, 0x11EE86603B21AAAD, 0x5D8F161BC02C8D97, applyCommonMapPatches },          // MAP02 (NTSC)
    { 117187, 0xCEB0D976F68337AA, 0x09F59F1FE893E39D, applyCommonMapPatches },          // MAP02 (PAL, why different?)
    { 135628, 0x639E67D5899E0CC8, 0x4EF89314776AD87A, applyCommonMapPatches },          // MAP03
    {  88648, 0x4D59797D210E13CA, 0x10817D3E8EB14112, applyCommonMapPatches },          // MAP04
    { 132897, 0x4D1504E792348E2E, 0xB8DD620D47F0E20D, applyCommonMapPatches },          // MAP05
    {  88004, 0x0C2514D4D67B2DC1, 0x20DF8A0C7C0181F9, applyCommonMapPatches },          // MAP06
    { 165920, 0xFD1F4BA81B723B90, 0xBEE05C2EF66BD8CE, applyCommonMapPatches },          // MAP07
    { 151747, 0x8B97DAE94D3AEA93, 0xCCB9A05C25F6273D, applyCommonMapPatches },          // MAP08
    { 102104, 0xF1E84BE020DC0415, 0x8C0D9CBC22FD633A, applyCommonMapPatches },          // MAP09
    { 139820, 0xE87917A5B2F8DE5E, 0xCCEF89F8A414138D, applyCommonMapPatches },          // MAP10
    {  96211, 0x2ACA379BCEA3B242, 0xB4AE81168E0CD072, applyCommonMapPatches },          // MAP11
    { 106776, 0x18880190E8AD3AAD, 0xCC11724E98AC707D, applyCommonMapPatches },          // MAP12
    { 152855, 0xE113179C755C90D4, 0xEB405A27D5CC781B, applyCommonMapPatches },          // MAP13
    {  54706, 0x2E3197426C689F97, 0xE3F4207EC033EAB9, applyCommonMapPatches },          // MAP14
    {  77891, 0x1A2C1B135538F920, 0x65C7AE4E6C0D8ED9, applyCommonMapPatches },          // MAP15
    { 156972, 0xC4E1E0DEBE66DFC4, 0x9DFD17A02FE856FB, applyCommonMapPatches },          // MAP16
    { 179622, 0x3C2DE97BC0E2DF97, 0xB323563071BA29EC, applyCommonMapPatches },          // MAP17
    { 131823, 0xB78A57E94315D5AD, 0x6454011A5579E4A3, applyCommonMapPatches },          // MAP18
    { 177868, 0xC12228E6C75BDC5B, 0xF1791C09D04A373F, applyCommonMapPatches },          // MAP19
    { 105404, 0x13AF4786F9A94958, 0x69FC197FE691C859, applyCommonMapPatches },          // MAP20
    { 162561, 0x9A3EC1A50C49A45B, 0xDFCAF41AC305D523, applyCommonMapPatches },          // MAP21
    {  96826, 0x9A220749A946649B, 0x2D4F9DDBEDF5C96D, applyCommonMapPatches },          // MAP22
    { 167847, 0xB3062D0C57E6C33B, 0x86BE982C0D6B7518, patchMap_Ballistyx },             // MAP23
    { 121920, 0x716B0625DA7F5D44, 0x4D8D182EB29338AC, applyCommonMapPatches },          // MAP24
    { 113719, 0x74B57A48F73EA6FB, 0xAAE20D3A62771BE2, applyCommonMapPatches },          // MAP25
    { 127601, 0x3EB3E8534AC50810, 0xCD4D177391C4358E, applyCommonMapPatches },          // MAP26
    { 113829, 0x46C313B75B92A625, 0x25A3DE0376C0F77A, applyCommonMapPatches },          // MAP27
    { 141807, 0xAB6599911EBD6134, 0xF60582647B6CC307, applyCommonMapPatches },          // MAP28
    { 107736, 0xCCCC24A0CE9C78D9, 0x475CB621C4B6CC61, applyCommonMapPatches },          // MAP29 (NTSC)
    { 107736, 0xAD6F4C5006BE9905, 0x640741D68A1CCB1D, applyCommonMapPatches },          // MAP29 (PAL, why different?)
    { 110131, 0x3E2804E58172152C, 0x03059F3BA3454891, applyCommonMapPatches },          // MAP30
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to apply patches for the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void applyPatches() noexcept {
    // Cache these globals locally since they will be compared a lot
    const int32_t mapSize = MapHash::gDataSize;
    const uint64_t md5Word1 = MapHash::gWord1;
    const uint64_t md5Word2 = MapHash::gWord2;

    // Check to see which patch applies (if any)
    for (const PatchDef& patch : gPatches) {
        ASSERT(patch.patcherFunc);

        // Wrong map data size?
        if (mapSize != patch.mapSize)
            continue;

        // Wrong MD5 hash for all the map data?
        if ((md5Word1 != patch.md5Word1) || (md5Word2 != patch.md5Word2))
            continue;

        // Match, apply the patch and abort the search
        patch.patcherFunc();
        break;
    }
}

END_NAMESPACE(MapPatcher)

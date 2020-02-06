#include "i_misc.h"

#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "i_drawcmds.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBC2.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

// Where on the UI texture are each of the font characters located and the size of each character
struct fontchar_t {
    std::uint8_t u;
    std::uint8_t v;
    std::uint8_t w;
    std::uint8_t h;
};

static constexpr fontchar_t BIG_FONT_CHARS[] = {
	{   0, 195,  11,  16 }, // 0 - 0
	{  12, 195,  11,  16 }, // 1 - 1
	{  24, 195,  11,  16 }, // 2 - 2
	{  36, 195,  11,  16 }, // 3 - 3
	{  48, 195,  11,  16 }, // 4 - 4
	{  60, 195,  11,  16 }, // 5 - 5
	{  72, 195,  11,  16 }, // 6 - 6
	{  84, 195,  11,  16 }, // 7 - 7
	{  96, 195,  11,  16 }, // 8 - 8
	{ 108, 195,  11,  16 }, // 9 - 9
	{ 232, 195,  11,  16 }, // - - 10
	{ 120, 195,  11,  15 }, // % - 11
	{   0, 211,   7,  16 }, // ! - 12
	{   8, 211,   7,  16 }, // . - 13
	{  16, 211,  15,  16 }, // A - 14
	{  32, 211,  13,  16 }, // B - 15
	{  46, 211,  12,  16 }, // C - 16
	{  60, 211,  13,  16 }, // D - 17
	{  74, 211,  13,  16 }, // E - 18
	{  88, 211,  13,  16 }, // F - 19
	{ 102, 211,  13,  16 }, // G - 20
	{ 116, 211,  13,  16 }, // H - 21
	{ 130, 211,   6,  16 }, // I - 22
	{ 136, 211,  12,  16 }, // J - 23
	{ 148, 211,  14,  16 }, // K - 24
	{ 162, 211,  13,  16 }, // L - 25
	{ 176, 211,  15,  16 }, // M - 26
	{ 192, 211,  15,  16 }, // N - 27
	{ 208, 211,  13,  16 }, // O - 28
	{ 222, 211,  13,  16 }, // P - 29
	{ 236, 211,  13,  16 }, // Q - 30
	{   0, 227,  13,  16 }, // R - 31
	{  14, 227,  13,  16 }, // S - 32
	{  28, 227,  14,  16 }, // T - 33
	{  42, 227,  13,  16 }, // U - 34
	{  56, 227,  15,  16 }, // V - 35
	{  72, 227,  15,  16 }, // W - 36
	{  88, 227,  15,  16 }, // X - 37
	{ 104, 227,  13,  16 }, // Y - 38
	{ 118, 227,  13,  16 }, // Z - 39
	{ 132, 230,  13,  13 }, // a - 40
	{ 146, 230,  12,  13 }, // b - 41
	{ 158, 230,  11,  13 }, // c - 42
	{ 170, 230,  11,  13 }, // d - 43
    { 182, 230,  10,  13 }, // e - 44
	{ 192, 230,  11,  13 }, // f - 45
	{ 204, 230,  11,  13 }, // g - 46
	{ 216, 230,  12,  13 }, // h - 47
	{ 228, 230,   5,  13 }, // i - 48
	{ 234, 230,  10,  13 }, // j - 49
	{   0, 243,  12,  13 }, // k - 50
	{  12, 243,   9,  13 }, // l - 51
	{  22, 243,  13,  13 }, // m - 52
	{  36, 243,  13,  13 }, // n - 53
	{  50, 243,  11,  13 }, // o - 54
	{  62, 243,  11,  13 }, // p - 55
	{  74, 243,  11,  13 }, // q - 56
	{  86, 243,  11,  13 }, // r - 57
	{  98, 243,  12,  13 }, // s - 58
	{ 112, 243,  11,  13 }, // t - 59
	{ 124, 243,  11,  13 }, // u - 60
	{ 136, 243,  13,  13 }, // v - 61
	{ 150, 243,  13,  13 }, // w - 62
	{ 164, 243,  13,  13 }, // x - 63
	{ 178, 243,  13,  13 }, // y - 64
	{ 192, 243,  13,  13 }  // z - 65
};

// Starting indices for various individual and groups of big font chars
enum : int32_t {
    BIG_FONT_DIGITS         = 0,
    BIG_FONT_MINUS          = 10,
    BIG_FONT_PERCENT        = 11,
    BIG_FONT_EXCLAMATION    = 12,
    BIG_FONT_PERIOD         = 13,
    BIG_FONT_UCASE_ALPHA    = 14,
    BIG_FONT_LCASE_ALPHA    = 40
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a number using the large font at the specified pixel location
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawNumber(const int32_t x, const int32_t y, const int32_t value) noexcept {
    // Basic setup of the drawing primitive
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    #if PC_PSX_DOOM_MODS
        // Set these primitive properties prior to drawing rather than allowing them to be undefined as in the original code.
        // I think this drawing code just happened to work because of the types of operations which occurred just before it.
        // They must have produced primitive state changes that we actually desired for this function...
        // Relying on external draw code and ordering however is brittle, so be explicit here and set exactly what we need:    
        {
            // Set the draw mode to remove the current texture window and texture page to the STATUS graphic
            DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);        
            I_AddPrim(&drawModePrim);
        }

        LIBGPU_SetSprt(spritePrim);
        LIBGPU_setRGB0(spritePrim, 127, 127, 127);
        spritePrim.clut = gPaletteClutIds[UIPAL];
    #endif

    spritePrim.y0 = (int16_t) y;            // Always on the same row
    spritePrim.tv0 = 195;                   // Digits are all on the same line in VRAM
    LIBGPU_setWH(spritePrim, 11, 16);       // Digits are always this size

    // Work with unsigned while we are printing, until the end
    bool bNegativeVal;
    int32_t valueAbs;

    if (value >= 0) {
        bNegativeVal = false;
        valueAbs = value;
    } else {
        bNegativeVal = true;
        valueAbs = -value;
    }

    // Figure out what digits to print
    constexpr uint32_t MAX_DIGITS = 16;
    int32_t digits[MAX_DIGITS];
    int32_t digitIdx = 0;

    while (digitIdx < MAX_DIGITS) {
        digits[digitIdx] = valueAbs % 10;
        valueAbs /= 10;

        if (valueAbs <= 0)
            break;

        ++digitIdx;
    }

    // Print the digits, starting with the least significant and move backwards across the screen
    const int32_t numDigits = digitIdx + 1;
    int32_t curX = x;

    for (digitIdx = 0; digitIdx < numDigits; ++digitIdx) {
        const int32_t digit = digits[digitIdx];

        spritePrim.x0 = (int16_t) curX;
        spritePrim.tu0 = BIG_FONT_CHARS[BIG_FONT_DIGITS + digit].u;
        I_AddPrim(&spritePrim);

        curX -= 11;
    }

    // Print the minus symbol if the value was negative
    if (bNegativeVal) {
        spritePrim.x0 = (int16_t) curX;
        spritePrim.tu0 = BIG_FONT_CHARS[BIG_FONT_EXCLAMATION].u;
        I_AddPrim(&spritePrim);
    }
}

void _thunk_I_DrawNumber() noexcept {
    I_DrawNumber(a0, a1, a2);
}

void I_DrawStringSmall() noexcept {
loc_8003A9D4:
    sp -= 0x10;
    t5 = a0;
    v0 = 8;                                             // Result = 00000008
    sw(s2, sp + 0x8);
    sw(s1, sp + 0x4);
    sw(s0, sp);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a1, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    a0 = lbu(a2);
    a2++;
    if (a0 == 0) goto loc_8003ACEC;
    s2 = 0x1F800000;                                    // Result = 1F800000
    s2 += 0x200;                                        // Result = 1F800200
    t4 = 0xFF0000;                                      // Result = 00FF0000
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    s1 = 0x80080000;                                    // Result = 80080000
    s1 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = s1 & t4;                                       // Result = 00086550
    t9 = 0x4000000;                                     // Result = 04000000
    t8 = 0x80000000;                                    // Result = 80000000
    t7 = -1;                                            // Result = FFFFFFFF
loc_8003AA3C:
    v0 = a0 - 0x61;
    v0 = (v0 < 0x1A);
    if (v0 == 0) goto loc_8003AA50;
    a0 -= 0x20;
loc_8003AA50:
    a0 -= 0x21;
    v0 = (a0 < 0x40);
    if (v0 != 0) goto loc_8003AA74;
    t5 += 8;
    goto loc_8003ACDC;
loc_8003AA68:
    v0 = t2 + 4;
    v0 += a0;
    goto loc_8003ABF4;
loc_8003AA74:
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t5, at + 0x208);                                 // Store to: 1F800208
    v1 = a0;
    if (i32(a0) >= 0) goto loc_8003AA88;
    v1 = a0 + 0x1F;
loc_8003AA88:
    t3 = s2 + 4;                                        // Result = 1F800204
    v1 = u32(i32(v1) >> 5);
    v0 = v1 << 5;
    v0 = a0 - v0;
    v0 <<= 3;
    v1 <<= 3;
    t1 = 0x1F800000;                                    // Result = 1F800000
    t1 = lbu(t1 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 -= 0x58;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v1, at + 0x20D);                                 // Store to: 1F80020D
    t2 = t1 << 2;
    t6 = t2 + 4;
loc_8003AACC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8003AB30;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003AA68;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s1, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003AB30:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t2 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003ABE4;
    if (v1 == a0) goto loc_8003AACC;
loc_8003AB54:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_8003AACC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_8003ABC0;
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003ABA4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != t0) goto loc_8003ABA4;
loc_8003ABC0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003AACC;
    goto loc_8003AB54;
loc_8003ABE4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t6;
loc_8003ABF4:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a3);
    sb(t1, a3 + 0x3);
    t1--;
    a3 += 4;
    if (t1 == t7) goto loc_8003ACBC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003AC30:
    v0 = lw(t3);
    t3 += 4;
    t1--;
    sw(v0, a3);
    a3 += 4;
    if (t1 != v1) goto loc_8003AC30;
    goto loc_8003ACBC;
loc_8003AC50:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_8003ACD8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_8003ACBC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8003ACA0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8003ACA0;
loc_8003ACBC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003AC50;
loc_8003ACD8:
    t5 += 8;
loc_8003ACDC:
    a0 = lbu(a2);
    a2++;
    if (a0 != 0) goto loc_8003AA3C;
loc_8003ACEC:
    s2 = lw(sp + 0x8);
    s1 = lw(sp + 0x4);
    s0 = lw(sp);
    sp += 0x10;
    return;
}

void I_DrawPausedOverlay() noexcept {
loc_8003AD04:
    v1 = *gCurPlayerIndex;
    sp -= 0x58;
    sw(ra, sp + 0x54);
    sw(s0, sp + 0x50);
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s0 = v1 + v0;
    v0 = lw(s0 + 0xC0);
    v0 &= 0x100;
    a1 = 0x6B;                                          // Result = 0000006B
    if (v0 != 0) goto loc_8003AD64;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A70;                                       // Result = gTex_PAUSE[0] (80097A70)
    a3 = gPaletteClutIds[MAINPAL];
    a2 = 0x6C;                                          // Result = 0000006C
    _thunk_I_CacheAndDrawSprite();
loc_8003AD64:
    v1 = lw(s0 + 0xC0);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_8003ADD8;
    }
    a2 = *gMapNumToCheatWarpTo;
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1634;                                       // Result = STR_WarpToLevel[0] (80011634)
    a0 = sp + 0x10;
    LIBC2_sprintf();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x28;                                          // Result = 00000028
    a2 = sp + 0x10;
    _thunk_I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x3C;                                          // Result = 0000003C
    a2 = *gMapNumToCheatWarpTo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += v0;
    _thunk_I_DrawString();
    goto loc_8003B0DC;
loc_8003ADCC:
    v0 = t3 + 4;                                        // Result = 00000018
    v0 += a0;
    goto loc_8003AFB8;
loc_8003ADD8:
    t0 = 5;                                             // Result = 00000005
    if (v0 == 0) goto loc_8003B0DC;
    t2 = 0x1F800000;                                    // Result = 1F800000
    t2 += 0x204;                                        // Result = 1F800204
    t3 = 0x14;                                          // Result = 00000014
    t1 = 0xFF0000;                                      // Result = 00FF0000
    t1 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = t7 & t1;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t8 = 0x18;                                          // Result = 00000018
    v0 = 5;                                             // Result = 00000005
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x28;                                          // Result = 00000028
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 = 0x100;                                         // Result = 00000100
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0xF0;                                          // Result = 000000F0
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x204);                                  // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x205);                                  // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x206);                                  // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x208);                                  // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20A);                                  // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20E);                                  // Store to: 1F80020E
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x210);                                  // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x214);                                 // Store to: 1F800214
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x216);                                 // Store to: 1F800216
loc_8003AE90:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t3 + a0;
        if (bJump) goto loc_8003AEF4;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003ADCC;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003AEF4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t3 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003AFA8;
    if (v1 == a0) goto loc_8003AE90;
loc_8003AF18:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8003AE90;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t1;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_8003AF84;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003AF68:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003AF68;
loc_8003AF84:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003AE90;
    goto loc_8003AF18;
loc_8003AFA8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t8;
loc_8003AFB8:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;                                               // Result = 00000004
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003B018;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003B000:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003B000;
loc_8003B018:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003B0CC;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003B044:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003B0CC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_8003B0B0;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003B094:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003B094;
loc_8003B0B0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003B044;
loc_8003B0CC:
    a0 = *gVramViewerTexPage;
    I_VramViewerDraw();
loc_8003B0DC:
    ra = lw(sp + 0x54);
    s0 = lw(sp + 0x50);
    sp += 0x58;
    return;
}

void I_UpdatePalette() noexcept {
loc_8003B0F0:
    v1 = *gCurPlayerIndex;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    a1 = v1 + v0;
    v0 = lw(a1 + 0x34);
    a2 = lw(a1 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0 = u32(i32(v0) >> 6);
        if (bJump) goto loc_8003B144;
    }
    v1 = 0xC;                                           // Result = 0000000C
    v1 -= v0;
    v0 = (i32(a2) < i32(v1));
    if (v0 == 0) goto loc_8003B144;
    a2 = v1;
loc_8003B144:
    a0 = lw(a1 + 0x44);
    *gbDoViewLighting = true;
    v0 = (i32(a0) < 0x3D);
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003B16C;
    v0 = a0 & 8;
    if (v0 == 0) goto loc_8003B174;
loc_8003B16C:
    *gbDoViewLighting = false;
loc_8003B174:
    a0 = lw(a1 + 0x30);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B190;
    }
    if (v0 == 0) goto loc_8003B1A0;
loc_8003B190:
    *gbDoViewLighting = false;
    v1 = 0xE;                                           // Result = 0000000E
    goto loc_8003B210;
loc_8003B1A0:
    v0 = a2 + 7;
    if (a2 == 0) goto loc_8003B1C4;
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 8);
    if (v0 != 0) goto loc_8003B1BC;
    v1 = 7;                                             // Result = 00000007
loc_8003B1BC:
    v1++;
    goto loc_8003B210;
loc_8003B1C4:
    a0 = lw(a1 + 0x3C);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B1E0;
    }
    if (v0 == 0) goto loc_8003B1E8;
loc_8003B1E0:
    v1 = 0xD;                                           // Result = 0000000D
    goto loc_8003B210;
loc_8003B1E8:
    v0 = lw(a1 + 0xDC);
    {
        const bool bJump = (v0 == 0);
        v0 += 7;
        if (bJump) goto loc_8003B210;
    }
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 4);
    v1 += 9;
    if (v0 != 0) goto loc_8003B210;
    v1 = 3;                                             // Result = 00000003
    v1 += 9;                                            // Result = 0000000C
loc_8003B210:
    v0 = v1 << 1;
    at = gPaletteClutIds;
    at += v0;
    v0 = lhu(at);
    at = 0x80070000;                                    // Result = 80070000
    *g3dViewPaletteClutId = (uint16_t) v0;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// For the given string returns the 'x' coordinate to draw it in the center of the screen.
// Assumes the big font is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t I_GetStringXPosToCenter(const char* const str) noexcept {
    // Go through the entire string and get the width of all characters that would be drawn
    int32_t width = 0;
    const char* pCurChar = str;

    for (char c = *pCurChar; c != 0; ++pCurChar, c = *pCurChar) {
        // Figure out which font character to use, and y positioning
        int32_t charIdx = 0;

        if ((c >= 'A') && (c <= 'Z')) {
            charIdx = BIG_FONT_UCASE_ALPHA + (c - 'A');
        } else if ((c >= 'a') && (c <= 'z')) {
            charIdx = BIG_FONT_LCASE_ALPHA + (c - 'a');
        } else if ((c >= '0') && (c <= '9')) {
            charIdx = BIG_FONT_DIGITS + (c - '0');
        } else if (c == '%') {
            charIdx = BIG_FONT_PERCENT;
        } else if (c == '!') {
            charIdx = BIG_FONT_EXCLAMATION;
        } else if (c == '.') {
            charIdx = BIG_FONT_PERIOD;
        } else if (c == '-') {
            charIdx = BIG_FONT_MINUS;
        } else {
            width += 6;     // Whitespace
            continue;
        }

        const fontchar_t& fontchar = BIG_FONT_CHARS[charIdx];
        width += fontchar.w;
    }

    // Figure out an x position to center this string in the middle of the screen
    return (SCREEN_W - width) / 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the given string using the big font at the given pixel location.
// If '-1' is specified for the x coordinate, the string is drawn centered horizontally in the middle of the screen.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawString(const int32_t x, const int32_t y, const char* const str) noexcept {
    // Set the draw mode to remove the current texture window and texture page to the STATUS graphic
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);
        LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);        
        I_AddPrim(&drawModePrim);
    }

    // Some basic setup of the sprite primitive for all characters
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_SetShadeTex(&spritePrim, true);
    spritePrim.clut = gPaletteClutIds[UIPAL];

    #if PC_PSX_DOOM_MODS
        // The color RGB value was undefined for this function in the original code - would use whatever the previous value was.
        // This is a little brittle so ensure we are not dependent on external draw operations being done in a certain order:
        LIBGPU_setRGB0(spritePrim, 127, 127, 127);
    #endif

    // Decide on starting x position: can either be so the string is centered in the screen, or just the value verbatim
    int32_t curX = (x != -1) ? x : I_GetStringXPosToCenter(str);

    // Draw all the characters in the string
    const char* pCurChar = str;

    for (char c = *pCurChar; c != 0; ++pCurChar, c = *pCurChar) {
        // Figure out which font character to use, and y positioning
        int32_t curY = y;
        int32_t charIdx = 0;

        if ((c >= 'A') && (c <= 'Z')) {
            charIdx = BIG_FONT_UCASE_ALPHA + (c - 'A');
        } else if ((c >= 'a') && (c <= 'z')) {
            charIdx = BIG_FONT_LCASE_ALPHA + (c - 'a');
            curY += 3;
        } else if ((c >= '0') && (c <= '9')) {
            charIdx = BIG_FONT_DIGITS + (c - '0');
        } else if (c == '%') {
            charIdx = BIG_FONT_PERCENT;
        } else if (c == '!') {
            charIdx = BIG_FONT_EXCLAMATION;
        } else if (c == '.') {
            charIdx = BIG_FONT_PERIOD;
        } else if (c == '-') {
            charIdx = BIG_FONT_MINUS;
        } else {
            curX += 6;      // Whitespace
            continue;
        }

        // Populate and submit the sprite primitive
        const fontchar_t& fontchar = BIG_FONT_CHARS[charIdx];

        LIBGPU_setXY0(spritePrim, (int16_t) curX, (int16_t) curY);
        LIBGPU_setUV0(spritePrim, fontchar.u, fontchar.v);
        LIBGPU_setWH(spritePrim, fontchar.w, fontchar.h);
	
        I_AddPrim(&spritePrim);

        // Move past the drawn character
        curX += fontchar.w;
    }
}

void _thunk_I_DrawString() noexcept {
    I_DrawString(a0, a1, vmAddrToPtr<const char>(a2));
}

#include "st_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "in_main.h"
#include "PcPsx/Finally.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include <cstdio>

static constexpr int32_t GIBTIME    = 2;    // How long frames in the gib animation last
static constexpr int32_t FLASHDELAY = 4;    // Tics in between keycard flashes
static constexpr int32_t FLASHTIMES = 6;    // Number of times to toggle keycard state between visible and invisible (when flashing)

// Positions for each of the micronumbers on the status bar
static constexpr int32_t NUMMICROS = 8;

static const int16_t gMicronumsX[NUMMICROS] = { 199, 212, 225, 238, 199, 212, 225, 238 };
static const int16_t gMicronumsY[NUMMICROS] = { 204, 204, 204, 204, 216, 216, 216, 216 };

// Keycard y positions on the status bar
static const int16_t gCardY[NUMCARDS] = { 204, 212, 220, 204, 212, 220 };

// The definitions for each face sprite
const facesprite_t gFaceSprites[NUMFACES] = {
    { 118, 202,   0,  41, 19, 29 },     // STFST01  - 0
    { 118, 202,  20,  41, 19, 29 },     // STFST02  - 1
    { 118, 202, 234, 137, 19, 29 },     // STFST00  - 2
    { 118, 202,  40,  41, 21, 31 },     // STFTL00  - 3
    { 118, 202,  62,  41, 21, 31 },     // STFTR00  - 4
    { 118, 202,  84,  41, 19, 31 },     // STFOUCH0 - 5
    { 118, 202, 104,  41, 19, 31 },     // STFEVL0  - 6 (EVILFACE)
    { 118, 202, 124,  41, 19, 31 },     // STFKILL0 - 7
    { 118, 202, 144,  41, 19, 31 },     // STFST11  - 8
    { 118, 202, 164,  41, 19, 31 },     // STFST10  - 9
    { 118, 202, 184,  41, 19, 31 },     // STFST12  - 10
    { 118, 202, 204,  41, 20, 31 },     // STFTL10  - 11
    { 118, 202, 226,  41, 21, 31 },     // STFTR10  - 12
    { 118, 202,   0,  73, 19, 31 },     // STFOUCH1 - 13
    { 118, 202,  20,  73, 19, 31 },     // STFEVL1  - 14
    { 118, 202,  40,  73, 19, 31 },     // STFKILL1 - 15
    { 118, 202,  60,  73, 19, 31 },     // STFST21  - 16
    { 118, 202,  80,  73, 19, 31 },     // STFST20  - 17
    { 118, 202, 100,  73, 19, 31 },     // STFST22  - 18
    { 118, 202, 120,  73, 22, 31 },     // STFTL20  - 19
    { 118, 202, 142,  73, 22, 31 },     // STFTR20  - 20
    { 118, 202, 166,  73, 19, 31 },     // STFOUCH2 - 21
    { 118, 202, 186,  73, 19, 31 },     // STFEVL2  - 22
    { 118, 202, 206,  73, 19, 31 },     // STFKILL2 - 23
    { 118, 202, 226,  73, 19, 31 },     // STFST31  - 24
    { 118, 202,   0, 105, 19, 31 },     // STFST30  - 25
    { 118, 202,  20, 105, 19, 31 },     // STFST32  - 26
    { 118, 202,  40, 105, 23, 31 },     // STFTL30  - 27
    { 118, 202,  64, 105, 23, 31 },     // STFTR30  - 28
    { 118, 202,  88, 105, 19, 31 },     // STFOUCH3 - 29
    { 118, 202, 108, 105, 19, 31 },     // STFEVL3  - 30
    { 118, 202, 128, 105, 19, 31 },     // STFKILL3 - 31
    { 118, 202, 148, 105, 19, 31 },     // STFST41  - 32
    { 118, 202, 168, 105, 19, 31 },     // STFST40  - 33
    { 118, 202, 188, 105, 19, 31 },     // STFST42  - 34
    { 118, 202, 208, 105, 24, 31 },     // STFTL40  - 35
    { 118, 202, 232, 105, 23, 31 },     // STFTR40  - 36
    { 118, 202,   0, 137, 18, 31 },     // STFOUCH4 - 37
    { 118, 202,  20, 137, 19, 31 },     // STFEVL4  - 38
    { 118, 202,  40, 137, 19, 31 },     // STFKILL4 - 39
    { 118, 202,  60, 137, 19, 31 },     // STFGOD0  - 40 (GODFACE)
    { 118, 202,  80, 137, 19, 31 },     // STFDEAD0 - 41 (DEADFACE)
    { 118, 202, 100, 137, 19, 30 },     // STSPLAT0 - 42 (FIRSTSPLAT)
    { 114, 201, 120, 137, 27, 30 },     // STSPLAT1 - 43
    { 114, 204, 148, 137, 28, 30 },     // STSPLAT2 - 44
    { 114, 204, 176, 137, 28, 30 },     // STSPLAT3 - 45
    { 114, 204, 204, 137, 28, 30 }      // STSPLAT4 - 46
};

// State relating to flashing keycards on the status bar
struct sbflash_t {
    int16_t     active;     // Is the flash currently active?
    int16_t     doDraw;     // Are we currently drawing the keycard as part of the flash?
    int16_t     delay;      // Ticks until next draw/no-draw change
    int16_t     times;      // How many flashes are left
};

static const VmPtr<sbflash_t[NUMCARDS]> gFlashCards(0x800A94B4);

// Status bar
const VmPtr<stbar_t> gStatusBar(0x80098714);

// Face related state
static const VmPtr<int32_t>                 gFaceTics(0x80078134);
static const VmPtr<bool32_t>                gbDrawSBFace(0x80078130);
static const VmPtr<VmPtr<facesprite_t>>     gpCurSBFaceSprite(0x80078230);
static const VmPtr<bool32_t>                gbGibDraw(0x80078058);
static const VmPtr<bool32_t>                gbDoSpclFace(0x80077ECC);
static const VmPtr<int32_t>                 gNewFace(0x80078024);
static const VmPtr<spclface_e>              gSpclFaceType(0x80077F08);

//------------------------------------------------------------------------------------------------------------------------------------------
// 'Status bar' one time initialization.
// Loads and caches in VRAM the UI texture atlas used throughout the game.
//------------------------------------------------------------------------------------------------------------------------------------------
void ST_Init() noexcept {
    // Expect no use of the texture cache at this point
    if (*gTCacheFillPage != 0) {
        I_Error("ST_Init: initial texture cache foulup\n");
    }

    // Load this into the first texture cache page and expect it to be resident there
    I_LoadAndCacheTexLump(*gTex_STATUS, "STATUS", 0);

    if (*gTCacheFillPage != 0) {
        I_Error("ST_Init: final texture cache foulup\n");
    }

    // Lock down the first texture cache page (keep in VRAM at all times) and move the next fill location to the next page after
    *gLockedTexPagesMask |= 1;
    
    *gTCacheFillPage = 1;
    *gTCacheFillCellX = 0;
    *gTCacheFillCellY = 0;
    *gTCacheFillRowCellH = 0;
    
    // The STATUS texture can now be evicted from memory since it will always be in VRAM
    Z_FreeTags(**gpMainMemZone, PU_CACHE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Status bar initialization done at the beginning of each level
//------------------------------------------------------------------------------------------------------------------------------------------
void ST_InitEveryLevel() noexcept {
    gStatusBar->gotgibbed = false;
    gStatusBar->specialFace = f_none;
    gStatusBar->messageTicsLeft = 0;

    *gbDrawSBFace = true;
    *gFaceTics = 0;
    *gpCurSBFaceSprite = 0x80073E68;    // FIXME: StatusBarFaceSpriteInfo[0] (80073E68)
    *gbGibDraw = false;
    *gbDoSpclFace = false;

    for (int32_t cardIdx = 0; cardIdx < NUMCARDS; ++cardIdx) {
        gStatusBar->tryopen[cardIdx] = false;
        gFlashCards[cardIdx].active = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the status bar.
// Also updates the current palette used by the game.
//------------------------------------------------------------------------------------------------------------------------------------------
void ST_Ticker() noexcept {
    player_t& player = gPlayers[*gCurPlayerIndex];

    // Do face animation
    *gFaceTics -= 1;

    if (*gFaceTics <= 0) {
        *gFaceTics = M_Random() & 15;
        *gNewFace = M_Random() & 3;

        if (*gNewFace == 3) {
            *gNewFace = 1;
        }

        *gbDoSpclFace = false;
    }
    
    // Handle if we are doing a special face
    if (gStatusBar->specialFace != f_none) {
        *gbDoSpclFace = true;
        *gSpclFaceType = gStatusBar->specialFace;
        *gFaceTics = TICRATE;
        gStatusBar->specialFace = f_none;
    }

    // Handle being gibbed
    if (gStatusBar->gotgibbed) {
        *gbGibDraw = true;
        gStatusBar->gibframeTicsLeft = GIBTIME;
        gStatusBar->gibframe = 0;
        gStatusBar->gotgibbed = false;
    }
    
    // Animate being gibbed
    if (*gbGibDraw) {
        gStatusBar->gibframeTicsLeft -= 1;

        if (gStatusBar->gibframeTicsLeft <= 0) {
            gStatusBar->gibframe += 1;
            gStatusBar->gibframeTicsLeft = GIBTIME;

            // Is the gib animation fully played out? If so we render nothing...
            if (gStatusBar->gibframe >= 5) {
                *gbGibDraw = false;
                *gbDrawSBFace = false;
            }
        }
    }

    // Handle new messages to display
    if (player.message) {
        gStatusBar->message = player.message;
        gStatusBar->messageTicsLeft = TICRATE * 5;
        player.message = nullptr;
    }

    // Decrease message time left
    if (gStatusBar->messageTicsLeft != 0) {
        gStatusBar->messageTicsLeft -= 1;
    }

    // Update the keycard flash for all keys
    for (int32_t cardIdx = 0; cardIdx < NUMCARDS; ++cardIdx) {
        sbflash_t& flashCard = gFlashCards[cardIdx];

        // Are we starting up a keycard flash or processing a current one?
        if (gStatusBar->tryopen[cardIdx]) {
            // Starting up a keycard flash
            gStatusBar->tryopen[cardIdx] = false;
            flashCard.active = true;
            flashCard.delay = FLASHDELAY;
            flashCard.times = FLASHTIMES + 1;
            flashCard.doDraw = false;
        }
        else if (flashCard.active) {
            // Processing an existing keycard flash
            flashCard.delay -= 1;

            if (flashCard.delay == 0) {
                // Time to change flash keyframe (visible/invisible state)
                flashCard.delay = FLASHDELAY;
                flashCard.doDraw ^= 1;
                flashCard.times -= 1;

                // Are we done the flash?
                if (flashCard.times == 0) {
                    flashCard.active = false;
                }
                
                // Play the item pickup sound if flashing on
                if (flashCard.doDraw && flashCard.active) {
                    S_StartSound(nullptr, sfx_itemup);
                }
            }
        }
    }
    
    // Decide on the face frame to use
    if ((player.cheats & CF_GODMODE) || (player.powers[pw_invulnerability] != 0)) {
        // Godmode/invulnerability face
        gStatusBar->face = GODFACE;
    } else {
        if (*gbGibDraw) {
            // Player is being gibbed: use that face
            gStatusBar->face = gStatusBar->gibframe + FIRSTSPLAT;
        }
        else if (player.health != 0) {
            // Player is alive: decide on face based on special face type and current health
            const int32_t healthSeg = player.health / 20;
            const int32_t healthFrameOffset = (healthSeg >= 4) ? 0 : (4 - healthSeg) * 8;

            if (*gbDoSpclFace) {
                gStatusBar->face = *gSpclFaceType + healthFrameOffset;
            } else {
                gStatusBar->face = *gNewFace + healthFrameOffset;
            }
        } else {
            // Player is dead: use dead face
            gStatusBar->face = DEADFACE;
        }
    }
    
    // Save the sprite info for the face that will be drawn
    *gpCurSBFaceSprite = 0x80073E68 + gStatusBar->face * sizeof(facesprite_t);  // FIXME: StatusBarFaceSpriteInfo[0] (80073E68)

    // Update the current palette in use
    I_UpdatePalette();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do drawing for the HUD status bar
//------------------------------------------------------------------------------------------------------------------------------------------
void ST_Drawer() noexcept {
    // Setup the current texture page and texture window.
    // PC-PSX: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that.
    {
        DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);

        #if PC_PSX_DOOM_MODS
            RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);
        #endif

        I_AddPrim(&drawModePrim);
    }

    // Setup some sprite primitive state that is used for all the draw calls that follow
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_SetShadeTex(&spritePrim, true);
    spritePrim.clut = gPaletteClutIds[UIPAL];
    
    // Draw the current status bar message, or the map name (if in the automap)
    player_t& player = gPlayers[*gCurPlayerIndex];

    if (gStatusBar->messageTicsLeft > 0) {
        I_DrawStringSmall(7, 193, gStatusBar->message.get());
    } else {
        if (player.automapflags & AF_ACTIVE) {
            constexpr const char* const MAP_TITLE_FMT = "LEVEL %d:%s";
            char mapTitle[64];

            // PC-PSX: use 'snprintf' just to be safe here
            #if PC_PSX_DOOM_MODS
                std::snprintf(mapTitle, C_ARRAY_SIZE(mapTitle), MAP_TITLE_FMT, *gGameMap, gMapNames[*gGameMap - 1]);
            #else
                std::sprintf(mapTitle, MAP_TITLE_FMT, *gGameMap, gMapNames[*gGameMap - 1]);
            #endif

            I_DrawStringSmall(7, 193, mapTitle);
        }
    }

    // Draw the background for the status bar
    LIBGPU_setXY0(spritePrim, 0, 200);
    LIBGPU_setUV0(spritePrim, 0, 0);
    LIBGPU_setWH(spritePrim, 256, 40);

    I_AddPrim(&spritePrim);

    // Figure out what weapon to display ammo for and what to show for the ammo amount
    const weapontype_t weapon = (player.pendingweapon == wp_nochange) ?
        player.readyweapon :
        player.pendingweapon;

    const weaponinfo_t& weaponInfo = gWeaponInfo[weapon];
    const ammotype_t ammoType = weaponInfo.ammo;
    const int32_t ammo = (ammoType != am_noammo) ? player.ammo[ammoType] : 0;

    // Draw ammo, health and armor amounts
    I_DrawNumber(28, 204, ammo);
    I_DrawNumber(71, 204, player.health);
    I_DrawNumber(168, 204, player.armorpoints);

    // Draw keycards and skull keys
    {
        LIBGPU_setWH(spritePrim, 11, 8);
        spritePrim.x0 = 100;
        spritePrim.tv0 = 184;

        uint8_t texU = 114;

        for (int32_t cardIdx = 0; cardIdx < NUMCARDS; ++cardIdx) {
            const bool bHaveCard = player.cards[cardIdx];

            // Draw the card if we have it or if it's currently flashing
            if (bHaveCard || (gFlashCards[cardIdx].active && gFlashCards[cardIdx].doDraw)) {
                spritePrim.tu0 = texU;
                spritePrim.y0 = gCardY[cardIdx];

                I_AddPrim(&spritePrim);
            }

            texU += 11;
        }
    }

    // Draw weapon selector or frags (if deathmatch)
    if (*gNetGame != gt_deathmatch) {
        // Draw the weapon number box/container
        LIBGPU_setXY0(spritePrim, 200, 205);
        LIBGPU_setUV0(spritePrim, 180, 184);
        LIBGPU_setWH(spritePrim, 51, 23);

        I_AddPrim(&spritePrim);

        // Draw the micro numbers for each weapon.
        // Note that numbers '1' and '2' are already baked into the status bar graphic, so we start at the shotgun.
        {
            LIBGPU_setWH(spritePrim, 4, 6);
            spritePrim.tv0 = 184;

            uint8_t texU = 232;

            for (int32_t weaponIdx = wp_shotgun; weaponIdx < NUMMICROS; ++weaponIdx) {
                if (player.weaponowned[weaponIdx]) {
                    LIBGPU_setXY0(spritePrim, gMicronumsX[weaponIdx] + 5, gMicronumsY[weaponIdx] + 3);
                    spritePrim.tu0 = texU;

                    I_AddPrim(&spritePrim);
                }

                texU += 4;
            }
        }

        // Draw the white box or highlight for the currently selected weapon
        const int32_t microNumIdx = WEAPON_MICRO_INDEXES[weapon];

        LIBGPU_setXY0(spritePrim, gMicronumsX[microNumIdx], gMicronumsY[microNumIdx]);
        LIBGPU_setUV0(spritePrim, 164, 192);
        LIBGPU_setWH(spritePrim, 12, 12);
        
        I_AddPrim(&spritePrim);
    } else {
        // Draw the frags container box
        LIBGPU_setXY0(spritePrim, 209, 221);
        LIBGPU_setUV0(spritePrim, 208, 243);
        LIBGPU_setWH(spritePrim, 33, 8);

        I_AddPrim(&spritePrim);

        // Draw the number of frags
        I_DrawNumber(225, 204, player.frags);
    }

    // Draw the doomguy face if enabled
    if (*gbDrawSBFace) {
        const facesprite_t& sprite = **gpCurSBFaceSprite;

        LIBGPU_setXY0(spritePrim, sprite.xPos, sprite.yPos);
        LIBGPU_setUV0(spritePrim, sprite.texU, sprite.texV);
        LIBGPU_setWH(spritePrim, sprite.w, sprite.h);

        I_AddPrim(&spritePrim);
    }

    // Draw the paused overlay, level warp and vram viewer
    if (*gbGamePaused) {
        I_DrawPausedOverlay();
    }
}

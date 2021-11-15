#pragma once

#include "Doom/doomdef.h"

// Describes 1 frame of a status bar face sprite
struct facesprite_t {
    uint8_t xPos;
    uint8_t yPos;
    uint8_t texU;
    uint8_t texV;
    uint8_t w;
    uint8_t h;
};

// Special face type
enum spclface_e : int32_t {
    f_none,         // Not a face - no special face
    f_normal,       // Regular face
    f_eyebrow,      // Eyebrow raised face
    f_faceleft,     // Damaged and turn face left
    f_faceright,    // Damaged and turn face right
    f_hurtbad,      // Super surpised look when receiving a lot of damage
    f_gotgat,       // Evil smile picking up a weapon
    f_mowdown,      // Grimmace while continously firing weapon
    NUMSPCLFACES
};

// State relating to flashing keycards on the status bar
struct sbflash_t {
    int16_t     active;     // Is the flash currently active?
    int16_t     doDraw;     // Are we currently drawing the keycard as part of the flash?
    int16_t     delay;      // Ticks until next draw/no-draw change
    int16_t     times;      // How many flashes are left
};

// Container for most status bar related state.
// PsyDoom: all 'bool' fields here were originally 'uint32_t', changed them to express meaning better.
struct stbar_t {
    uint32_t        face;                   // Index of the face sprite to currently use
    spclface_e      specialFace;            // What special face to do next
    bool            tryopen[NUMCARDS];      // Whether we are doing a keycard flash for each of the key types
    bool            gotgibbed;              // True if the player just got gibbed
    int32_t         gibframe;               // What frame of the gib animation is currently showing
    int32_t         gibframeTicsLeft;       // How many game ticks left in the current gib animation frame
    const char*     message;                // The current message to show on the status bar (string must be valid at all times)
    int32_t         messageTicsLeft;        // How many game ticks left to show the status bar message for
#if PSYDOOM_MODS
    char            alertMessage[32];       // PsyDoom: a message displayed near the center of the screen which is not interrupted by pickups
    int32_t         alertMessageTicsLeft;   // PsyDoom: how many tics left before the alert message is done displaying
#endif
};

// The number of face sprite definitions there are
static constexpr int32_t NUMFACES = 47;

// Some of the indexes into the face sprite array
static constexpr int32_t EVILFACE   = 6;
static constexpr int32_t GODFACE    = 40;
static constexpr int32_t DEADFACE   = 41;
static constexpr int32_t FIRSTSPLAT = 42;   // Gib frames

// Which slot (by index) on the weapon micronumbers display each weapon maps to
static constexpr int32_t WEAPON_MICRO_INDEXES[NUMWEAPONS] = { 0, 1, 2, 3, 4, 5, 6, 7, 0 };

extern sbflash_t                gFlashCards[NUMCARDS];
extern const facesprite_t       gFaceSprites[NUMFACES];
extern stbar_t                  gStatusBar;
extern int32_t                  gFaceTics;
extern bool                     gbDrawSBFace;
extern const facesprite_t*      gpCurSBFaceSprite;
extern bool                     gbGibDraw;
extern bool                     gbDoSpclFace;
extern int32_t                  gNewFace;
extern spclface_e               gSpclFaceType;

void ST_Init() noexcept;
void ST_InitEveryLevel() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

#if PSYDOOM_MODS
    void ST_AlertMessage(const char* const msg, const uint32_t numTics) noexcept;
#endif

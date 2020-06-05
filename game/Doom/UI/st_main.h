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

static_assert(sizeof(facesprite_t) == 6);

// Special face type
enum spclface_e : int32_t {
    f_none,
    f_unknown1,     // TODO: what is this?
    f_unknown2,     // TODO: what is this?
    f_faceleft,     // Damaged and turn face left
    f_faceright,    // Damaged and turn face right
    f_hurtbad,      // Super surpised look when receiving a lot of damage
    f_gotgat,       // Evil smile picking up a weapon
    f_mowdown,      // Grimmace while continously firing weapon
    NUMSPCLFACES
};

// Container for most status bar related state
struct stbar_t {
    uint32_t        face;                   // TODO: COMMENT
    spclface_e      specialFace;            // TODO: COMMENT
    uint32_t        tryopen[NUMCARDS];      // TODO: COMMENT
    uint32_t        gotgibbed;              // TODO: COMMENT
    int32_t         gibframe;               // TODO: COMMENT
    int32_t         gibframeTicsLeft;       // TODO: COMMENT
    const char*     message;                // TODO: COMMENT
    int32_t         messageTicsLeft;        // TODO: COMMENT
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

extern const facesprite_t   gFaceSprites[NUMFACES];
extern stbar_t              gStatusBar;

void ST_Init() noexcept;
void ST_InitEveryLevel() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

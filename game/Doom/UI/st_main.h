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
    uint32_t            face;
    spclface_e          specialFace;
    uint32_t            tryopen[NUMCARDS];
    uint32_t            gotgibbed;
    int32_t             gibframe;
    int32_t             gibframeTicsLeft;
    VmPtr<const char>   message;
    int32_t             messageTicsLeft;
};

static_assert(sizeof(stbar_t) == 52);

// The number of face sprite definitions there are
static constexpr int32_t NUMFACES = 47;

// Some of the indexes into the face sprite array
static constexpr int32_t EVILFACE   = 6;
static constexpr int32_t GODFACE    = 40;
static constexpr int32_t DEADFACE   = 41;
static constexpr int32_t FIRSTSPLAT = 42;   // Gib frames

extern const facesprite_t       gFaceSprites[NUMFACES];
extern const VmPtr<stbar_t>     gStatusBar;

void ST_Init() noexcept;
void ST_InitEveryLevel() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

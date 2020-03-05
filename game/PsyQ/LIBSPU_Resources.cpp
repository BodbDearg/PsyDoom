#include "LIBSPU.h"

const SpuReverbDef gReverbDefs[SPU_REV_MODE_MAX] = {
    // SPU_REV_MODE_OFF
    {
        0x0,        // fieldBits
        0x0,        // apfOffset1
        0x0,        // apfOffset2
        0x0,        // reflectionVolume1
        0x0,        // combVolume1
        0x0,        // combVolume2
        0x0,        // combVolume3
        0x0,        // combVolume4
        0x0,        // reflectionVolume2
        0x0,        // apfVolume1
        0x0,        // apfVolume2
        0x0,        // sameSideRefractAddr1Left
        0x0,        // sameSideRefractAddr1Right
        0x0,        // combAddr1Left
        0x0,        // combAddr1Right
        0x0,        // combAddr2Left
        0x0,        // combAddr2Right
        0x0,        // sameSideRefractAddr2Left
        0x0,        // sameSideRefractAddr2Right
        0x0,        // diffSideReflectAddr1Left
        0x0,        // diffSideReflectAddr1Right
        0x0,        // combAddr3Left
        0x0,        // combAddr3Right
        0x0,        // combAddr4Left
        0x0,        // combAddr4Right
        0x0,        // diffSideReflectAddr2Left
        0x0,        // diffSideReflectAddr2Right
        0x0,        // apfAddr1Left
        0x0,        // apfAddr1Right
        0x0,        // apfAddr2Left
        0x0,        // apfAddr2Right
        0x0,        // inputVolLeft
        0x0,        // inputVolRight
    },
    // SPU_REV_MODE_ROOM
    {
        0x0,        // fieldBits
        0x7D,       // apfOffset1
        0x5B,       // apfOffset2
        0x6D80,     // reflectionVolume1
        0x54B8,     // combVolume1
        0xBED0,     // combVolume2
        0x0,        // combVolume3
        0x0,        // combVolume4
        0xBA80,     // reflectionVolume2
        0x5800,     // apfVolume1
        0x5300,     // apfVolume2
        0x4D6,      // sameSideRefractAddr1Left
        0x333,      // sameSideRefractAddr1Right
        0x3F0,      // combAddr1Left
        0x227,      // combAddr1Right
        0x374,      // combAddr2Left
        0x1EF,      // combAddr2Right
        0x334,      // sameSideRefractAddr2Left
        0x1B5,      // sameSideRefractAddr2Right
        0x0,        // diffSideReflectAddr1Left
        0x0,        // diffSideReflectAddr1Right
        0x0,        // combAddr3Left
        0x0,        // combAddr3Right
        0x0,        // combAddr4Left
        0x0,        // combAddr4Right
        0x0,        // diffSideReflectAddr2Left
        0x0,        // diffSideReflectAddr2Right
        0x1B4,      // apfAddr1Left
        0x136,      // apfAddr1Right
        0xB8,       // apfAddr2Left
        0x5C,       // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_STUDIO_A
    {
        0x0,        // fieldBits
        0x33,       // apfOffset1
        0x25,       // apfOffset2
        0x70F0,     // reflectionVolume1
        0x4FA8,     // combVolume1
        0xBCE0,     // combVolume2
        0x4410,     // combVolume3
        0xC0F0,     // combVolume4
        0x9C00,     // reflectionVolume2
        0x5280,     // apfVolume1
        0x4EC0,     // apfVolume2
        0x3E4,      // sameSideRefractAddr1Left
        0x31B,      // sameSideRefractAddr1Right
        0x3A4,      // combAddr1Left
        0x2AF,      // combAddr1Right
        0x372,      // combAddr2Left
        0x266,      // combAddr2Right
        0x31C,      // sameSideRefractAddr2Left
        0x25D,      // sameSideRefractAddr2Right
        0x25C,      // diffSideReflectAddr1Left
        0x18E,      // diffSideReflectAddr1Right
        0x22F,      // combAddr3Left
        0x135,      // combAddr3Right
        0x1D2,      // combAddr4Left
        0xB7,       // combAddr4Right
        0x18F,      // diffSideReflectAddr2Left
        0xB5,       // diffSideReflectAddr2Right
        0xB4,       // apfAddr1Left
        0x80,       // apfAddr1Right
        0x4C,       // apfAddr2Left
        0x26,       // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_STUDIO_B
    {
        0x0,        // fieldBits
        0xB1,       // apfOffset1
        0x7F,       // apfOffset2
        0x70F0,     // reflectionVolume1
        0x4FA8,     // combVolume1
        0xBCE0,     // combVolume2
        0x4510,     // combVolume3
        0xBEF0,     // combVolume4
        0xB4C0,     // reflectionVolume2
        0x5280,     // apfVolume1
        0x4EC0,     // apfVolume2
        0x904,      // sameSideRefractAddr1Left
        0x76B,      // sameSideRefractAddr1Right
        0x824,      // combAddr1Left
        0x65F,      // combAddr1Right
        0x7A2,      // combAddr2Left
        0x616,      // combAddr2Right
        0x76C,      // sameSideRefractAddr2Left
        0x5ED,      // sameSideRefractAddr2Right
        0x5EC,      // diffSideReflectAddr1Left
        0x42E,      // diffSideReflectAddr1Right
        0x50F,      // combAddr3Left
        0x305,      // combAddr3Right
        0x462,      // combAddr4Left
        0x2B7,      // combAddr4Right
        0x42F,      // diffSideReflectAddr2Left
        0x265,      // diffSideReflectAddr2Right
        0x264,      // apfAddr1Left
        0x1B2,      // apfAddr1Right
        0x100,      // apfAddr2Left
        0x80,       // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_STUDIO_C
    {
        0x0,        // fieldBits
        0xE3,       // apfOffset1
        0xA9,       // apfOffset2
        0x6F60,     // reflectionVolume1
        0x4FA8,     // combVolume1
        0xBCE0,     // combVolume2
        0x4510,     // combVolume3
        0xBEF0,     // combVolume4
        0xA680,     // reflectionVolume2
        0x5680,     // apfVolume1
        0x52C0,     // apfVolume2
        0xDFB,      // sameSideRefractAddr1Left
        0xB58,      // sameSideRefractAddr1Right
        0xD09,      // combAddr1Left
        0xA3C,      // combAddr1Right
        0xBD9,      // combAddr2Left
        0x973,      // combAddr2Right
        0xB59,      // sameSideRefractAddr2Left
        0x8DA,      // sameSideRefractAddr2Right
        0x8D9,      // diffSideReflectAddr1Left
        0x5E9,      // diffSideReflectAddr1Right
        0x7EC,      // combAddr3Left
        0x4B0,      // combAddr3Right
        0x6EF,      // combAddr4Left
        0x3D2,      // combAddr4Right
        0x5EA,      // diffSideReflectAddr2Left
        0x31D,      // diffSideReflectAddr2Right
        0x31C,      // apfAddr1Left
        0x238,      // apfAddr1Right
        0x154,      // apfAddr2Left
        0xAA,       // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_HALL
    {
        0x0,        // fieldBits
        0x1A5,      // apfOffset1
        0x139,      // apfOffset2
        0x6000,     // reflectionVolume1
        0x5000,     // combVolume1
        0x4C00,     // combVolume2
        0xB800,     // combVolume3
        0xBC00,     // combVolume4
        0xC000,     // reflectionVolume2
        0x6000,     // apfVolume1
        0x5C00,     // apfVolume2
        0x15BA,     // sameSideRefractAddr1Left
        0x11BB,     // sameSideRefractAddr1Right
        0x14C2,     // combAddr1Left
        0x10BD,     // combAddr1Right
        0x11BC,     // combAddr2Left
        0xDC1,      // combAddr2Right
        0x11C0,     // sameSideRefractAddr2Left
        0xDC3,      // sameSideRefractAddr2Right
        0xDC0,      // diffSideReflectAddr1Left
        0x9C1,      // diffSideReflectAddr1Right
        0xBC4,      // combAddr3Left
        0x7C1,      // combAddr3Right
        0xA00,      // combAddr4Left
        0x6CD,      // combAddr4Right
        0x9C2,      // diffSideReflectAddr2Left
        0x5C1,      // diffSideReflectAddr2Right
        0x5C0,      // apfAddr1Left
        0x41A,      // apfAddr1Right
        0x274,      // apfAddr2Left
        0x13A,      // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_SPACE
    {
        0x0,        // fieldBits
        0x33D,      // apfOffset1
        0x231,      // apfOffset2
        0x7E00,     // reflectionVolume1
        0x5000,     // combVolume1
        0xB400,     // combVolume2
        0xB000,     // combVolume3
        0x4C00,     // combVolume4
        0xB000,     // reflectionVolume2
        0x6000,     // apfVolume1
        0x5400,     // apfVolume2
        0x1ED6,     // sameSideRefractAddr1Left
        0x1A31,     // sameSideRefractAddr1Right
        0x1D14,     // combAddr1Left
        0x183B,     // combAddr1Right
        0x1BC2,     // combAddr2Left
        0x16B2,     // combAddr2Right
        0x1A32,     // sameSideRefractAddr2Left
        0x15EF,     // sameSideRefractAddr2Right
        0x15EE,     // diffSideReflectAddr1Left
        0x1055,     // diffSideReflectAddr1Right
        0x1334,     // combAddr3Left
        0xF2D,      // combAddr3Right
        0x11F6,     // combAddr4Left
        0xC5D,      // combAddr4Right
        0x1056,     // diffSideReflectAddr2Left
        0xAE1,      // diffSideReflectAddr2Right
        0xAE0,      // apfAddr1Left
        0x7A2,      // apfAddr1Right
        0x464,      // apfAddr2Left
        0x232,      // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_ECHO
    {
        0x0,        // fieldBits
        0x1,        // apfOffset1
        0x1,        // apfOffset2
        0x7FFF,     // reflectionVolume1
        0x7FFF,     // combVolume1
        0x0,        // combVolume2
        0x0,        // combVolume3
        0x0,        // combVolume4
        0x8100,     // reflectionVolume2
        0x0,        // apfVolume1
        0x0,        // apfVolume2
        0x1FFF,     // sameSideRefractAddr1Left
        0xFFF,      // sameSideRefractAddr1Right
        0x1005,     // combAddr1Left
        0x5,        // combAddr1Right
        0x0,        // combAddr2Left
        0x0,        // combAddr2Right
        0x1005,     // sameSideRefractAddr2Left
        0x5,        // sameSideRefractAddr2Right
        0x0,        // diffSideReflectAddr1Left
        0x0,        // diffSideReflectAddr1Right
        0x0,        // combAddr3Left
        0x0,        // combAddr3Right
        0x0,        // combAddr4Left
        0x0,        // combAddr4Right
        0x0,        // diffSideReflectAddr2Left
        0x0,        // diffSideReflectAddr2Right
        0x1004,     // apfAddr1Left
        0x1002,     // apfAddr1Right
        0x4,        // apfAddr2Left
        0x2,        // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_DELAY
    {
        0x0,        // fieldBits
        0x1,        // apfOffset1
        0x1,        // apfOffset2
        0x7FFF,     // reflectionVolume1
        0x7FFF,     // combVolume1
        0x0,        // combVolume2
        0x0,        // combVolume3
        0x0,        // combVolume4
        0x0,        // reflectionVolume2
        0x0,        // apfVolume1
        0x0,        // apfVolume2
        0x1FFF,     // sameSideRefractAddr1Left
        0xFFF,      // sameSideRefractAddr1Right
        0x1005,     // combAddr1Left
        0x5,        // combAddr1Right
        0x0,        // combAddr2Left
        0x0,        // combAddr2Right
        0x1005,     // sameSideRefractAddr2Left
        0x5,        // sameSideRefractAddr2Right
        0x0,        // diffSideReflectAddr1Left
        0x0,        // diffSideReflectAddr1Right
        0x0,        // combAddr3Left
        0x0,        // combAddr3Right
        0x0,        // combAddr4Left
        0x0,        // combAddr4Right
        0x0,        // diffSideReflectAddr2Left
        0x0,        // diffSideReflectAddr2Right
        0x1004,     // apfAddr1Left
        0x1002,     // apfAddr1Right
        0x4,        // apfAddr2Left
        0x2,        // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    },
    // SPU_REV_MODE_PIPE
    {
        0x0,        // fieldBits
        0x17,       // apfOffset1
        0x13,       // apfOffset2
        0x70F0,     // reflectionVolume1
        0x4FA8,     // combVolume1
        0xBCE0,     // combVolume2
        0x4510,     // combVolume3
        0xBEF0,     // combVolume4
        0x8500,     // reflectionVolume2
        0x5F80,     // apfVolume1
        0x54C0,     // apfVolume2
        0x371,      // sameSideRefractAddr1Left
        0x2AF,      // sameSideRefractAddr1Right
        0x2E5,      // combAddr1Left
        0x1DF,      // combAddr1Right
        0x2B0,      // combAddr2Left
        0x1D7,      // combAddr2Right
        0x358,      // sameSideRefractAddr2Left
        0x26A,      // sameSideRefractAddr2Right
        0x1D6,      // diffSideReflectAddr1Left
        0x11E,      // diffSideReflectAddr1Right
        0x12D,      // combAddr3Left
        0xB1,       // combAddr3Right
        0x11F,      // combAddr4Left
        0x59,       // combAddr4Right
        0x1A0,      // diffSideReflectAddr2Left
        0xE3,       // diffSideReflectAddr2Right
        0x58,       // apfAddr1Left
        0x40,       // apfAddr1Right
        0x28,       // apfAddr2Left
        0x14,       // apfAddr2Right
        0x8000,     // inputVolLeft
        0x8000,     // inputVolRight
    }
};

const uint16_t gReverbWorkAreaBaseAddrs[SPU_REV_MODE_MAX] = {
    0xFFFE,     // SPU_REV_MODE_OFF
    0xFB28,     // SPU_REV_MODE_ROOM
    0xFC18,     // SPU_REV_MODE_STUDIO_A
    0xF6F8,     // SPU_REV_MODE_STUDIO_B
    0xF204,     // SPU_REV_MODE_STUDIO_C
    0xEA44,     // SPU_REV_MODE_HALL
    0xE128,     // SPU_REV_MODE_SPACE
    0xCFF8,     // SPU_REV_MODE_ECHO
    0xCFF8,     // SPU_REV_MODE_DELAY
    0xF880      // SPU_REV_MODE_PIPE
};

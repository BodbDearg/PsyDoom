//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing constants for various different game types
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GameConstants.h"

#include "Doom/cdmaptbl.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Renderer/r_data.h"
#include "FatalErrors.h"
#include "Game.h"
#include "PlayerPrefs.h"

// Game ids for networking
static constexpr uint32_t NET_GAMEID_DOOM                       = 0xAA11AA22;
static constexpr uint32_t NET_GAMEID_FINAL_DOOM                 = 0xAB11AB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_BETA3               = 0xAB00AB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_TESTMAP_DOOM        = 0xBB00BB22;
static constexpr uint32_t NET_GAMEID_GEC_ME_TESTMAP_FINAL_DOOM  = 0xBB00BB23;
static constexpr uint32_t NET_GAMEID_GEC_ME_BETA4               = 0xAB00AB23;

// Doom logo Y positions
#if PSYDOOM_MODS
    static constexpr int16_t DOOM_LOGO_YPOS = 10;           // Normal case: PsyDoom moves this up slightly to make room for the 'quit' option.
    static constexpr int16_t DOOM_LOGO_YPOS_GEC_ME = 12;    // GEC Master Edition.
#else
    static constexpr int16_t DOOM_LOGO_YPOS = 20;
#endif

// Extra palettes for 'GEC Master Edition (Beta 3)'
static const palette_t GEC_ME_BETA3_EXTRA_PALETTES[] = {
    // PAL 26: Credits screen - GEC logo
    { 
        0x0000, 0x8801, 0x8040, 0x8421, 0x8002, 0x8802, 0x8042, 0x8842, 0x8c21, 0x8c22, 0x9422, 0x8c42, 0x9442, 0x8060, 0x8861, 0x8481,
        0x8881, 0x8062, 0x8862, 0x8082, 0x8882, 0x8c61, 0x8c81, 0x8c62, 0x8c82, 0x8003, 0x8823, 0x8043, 0x8823, 0x8025, 0x8825, 0x8044,
        0x8844, 0x8c23, 0x9403, 0x8c43, 0x9443, 0x9005, 0x9425, 0x9045, 0x9045, 0x8064, 0x8863, 0x8483, 0x8883, 0x8064, 0x8865, 0x80a5,
        0x8884, 0x8c63, 0x9063, 0x8c83, 0x9084, 0x8c65, 0x9065, 0x8c84, 0x94a4, 0x9842, 0x9404, 0x9c23, 0x9843, 0x9c43, 0x9805, 0x9c04,
        0x9844, 0x9c44, 0x9c45, 0x9863, 0x9c64, 0x9864, 0x9c64, 0x9884, 0x9c85, 0x9c84, 0xa084, 0xa084, 0xa4a5, 0x88a1, 0x84e1, 0x88e1,
        0x84a2, 0x88c2, 0x88e2, 0x8cc2, 0x84e1, 0x8902, 0x8922, 0x8d02, 0x84c3, 0x88c3, 0x88e3, 0x80a5, 0x88c5, 0x8cc3, 0x90c3, 0x8ce3,
        0x94e3, 0x8cc4, 0x94c4, 0x8ce5, 0x94e4, 0x8d03, 0x9104, 0x8d23, 0x9123, 0x9104, 0x9104, 0x9524, 0x94e4, 0x94a5, 0x94e5, 0x9504,
        0x9d05, 0x9925, 0x8026, 0x8806, 0x8046, 0x8846, 0x8007, 0x8807, 0x8447, 0x8847, 0x8c06, 0x9006, 0x8c26, 0x9046, 0x8c27, 0x8c47,
        0x9427, 0x8065, 0x8866, 0x8086, 0x8886, 0x8067, 0x8867, 0x80a7, 0x88a6, 0x8c66, 0x9466, 0x8c86, 0x90a6, 0x8c67, 0x9067, 0x8c87,
        0x8028, 0x8448, 0x8848, 0x8029, 0x8449, 0x8849, 0x9048, 0x8488, 0x8868, 0x8488, 0x8888, 0x8869, 0x8889, 0x9088, 0x9805, 0xa026,
        0x9866, 0x9c86, 0x9885, 0x9c85, 0x9867, 0x9c87, 0x98a7, 0x9c87, 0xa066, 0xa466, 0xa0a5, 0xa486, 0xa067, 0xa467, 0xa087, 0xa4a7,
        0x9868, 0x9888, 0x9888, 0x9c88, 0x9489, 0xa088, 0xa468, 0xa487, 0x80c6, 0x88c6, 0x84e6, 0x88a7, 0x80e7, 0x88e6, 0x8cc6, 0x90c6,
        0x8ce6, 0x94e6, 0x8cc7, 0x90a7, 0x8ce7, 0x94e7, 0x9107, 0x88c8, 0x84e7, 0x90c8, 0x90c8, 0x90c8, 0x90c8, 0x90c9, 0x8ce9, 0x90e9,
        0x8908, 0x98c6, 0xa0c6, 0xa4a5, 0xa0c6, 0xa0c7, 0xa4c7, 0x9ce7, 0xa4e7, 0x9906, 0xa107, 0x98e8, 0xa0c8, 0x9909, 0xa107, 0xa508,
        0xa128, 0xa528, 0xa0ea, 0xa4e9, 0xa529, 0xa887, 0xa8c8, 0xa908, 0xa909, 0xa949, 0x8d43, 0x8d63, 0x9d47, 0x9d47, 0x9d47, 0xa949,
        0x884a, 0x8c8a, 0x906a, 0x8c8a, 0x94aa, 0x90aa, 0x98eb, 0x9d0a, 0x9cea, 0xa4ea, 0xa50b, 0xad6b, 0x8000, 0x8000, 0x8000, 0x8000
    },
    // PAL 27: Credits screen - Doomworld logo
    {
        0x0000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8420, 0x8420, 0x8420, 0x8420, 0x8420, 0x8420, 0x8420, 0x8420,
        0x8421, 0x8840, 0x8840, 0x8840, 0x8840, 0x8840, 0x8840, 0x8840, 0x8840, 0x8c60, 0x8c60, 0x8c60, 0x8c60, 0x8c60, 0x8c60, 0x8841,
        0x8841, 0x8841, 0x8841, 0x8841, 0x8c61, 0x8c61, 0x8c61, 0x8c61, 0x8c61, 0x8842, 0x8c62, 0x8c62, 0x8c62, 0x8c62, 0x8c62, 0x8c63,
        0x9080, 0x9080, 0x9080, 0x9080, 0x94a0, 0x94a0, 0x94a0, 0x94a0, 0x9081, 0x9081, 0x9081, 0x9081, 0x94a1, 0x94a1, 0x94a1, 0x98c0,
        0x98c0, 0x98c0, 0x98c0, 0x9ce0, 0x98c1, 0x98c1, 0x98c1, 0x9082, 0x9082, 0x9082, 0x9082, 0x9082, 0x94a2, 0x94a2, 0x94a2, 0x94a2,
        0x9083, 0x9083, 0x9083, 0x9083, 0x9083, 0x94a3, 0x94a3, 0x94a3, 0x94a3, 0x94a3, 0x98c2, 0x98c2, 0x98c2, 0x98c2, 0x9ce2, 0x9ce2,
        0x98c3, 0x98c3, 0x98c3, 0x98c3, 0x9ce3, 0x9ce3, 0x9084, 0x94a4, 0x94a4, 0x94a4, 0x94a4, 0x94a4, 0x94a4, 0x94a5, 0x98c4, 0x98c4,
        0x98c4, 0x98c4, 0x98c4, 0x9ce4, 0x9ce4, 0x9ce4, 0x9ce4, 0x98c5, 0x98c5, 0x98c5, 0x98c5, 0x98c5, 0x98c5, 0x9ce5, 0x9ce5, 0x9ce5,
        0x9ce5, 0x9ce5, 0x98c6, 0x9ce6, 0x9ce6, 0x9ce6, 0x9ce6, 0x9ce6, 0x9ce7, 0xa104, 0xa104, 0xa104, 0xa104, 0xa105, 0xa105, 0xa105,
        0xa105, 0xa525, 0xa106, 0xa106, 0xa106, 0xa106, 0xa106, 0xa106, 0xa526, 0xa526, 0xa526, 0xa526, 0xa107, 0xa107, 0xa107, 0xa107,
        0xa527, 0xa527, 0xa527, 0xa527, 0xa946, 0xa947, 0xa947, 0xad67, 0xa108, 0xa528, 0xa528, 0xa528, 0xa528, 0xa528, 0xa529, 0xa948,
        0xa948, 0xa948, 0xa948, 0xa948, 0xad68, 0xad68, 0xa949, 0xa949, 0xa949, 0xa949, 0xa949, 0xa949, 0xad69, 0xad69, 0xad69, 0xad69,
        0xad69, 0xa94a, 0xad6a, 0xad6a, 0xad6a, 0xad6a, 0xad6b, 0xb188, 0xb189, 0xb189, 0xb5a9, 0xb18a, 0xb18a, 0xb18a, 0xb18a, 0xb5aa,
        0xb5aa, 0xb5aa, 0xb18b, 0xb18b, 0xb18b, 0xb18b, 0xb18b, 0xb18b, 0xb5ab, 0xb5ab, 0xb5ab, 0xb9cb, 0xb18c, 0xb5ac, 0xb5ac, 0xb5ac,
        0xb5ac, 0xb5ac, 0xb5ac, 0xb5ad, 0xb9cc, 0xb9cc, 0xb9cc, 0xb9cc, 0xbdec, 0xb9cd, 0xb9cd, 0xb9cd, 0xb9cd, 0xb9cd, 0xbded, 0xbded,
        0xbded, 0xbded, 0xb9ce, 0xbdee, 0xbdee, 0xbdee, 0xbdee, 0xbdef, 0xc20e, 0xc20e, 0xc20f, 0xc20f, 0xc20f, 0xc20f, 0xc62f, 0xc62f 
    },
    // PAL 28: Credits screen - text for the GEC and Doomworld credits
    {
        0x0000, 0x8421, 0x800b, 0x842d, 0x8000, 0x8c71, 0x94b6, 0x8115, 0x8137, 0x98db, 0x8158, 0x859b, 0x89bb, 0x8ddb, 0xae7d, 0xcafe,
        0xdb5e, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000
    },
    // PAL 29: Main menu screen background
    {
        0x0000, 0x8421, 0x8800, 0x8c00, 0x8821, 0x8c21, 0x8841, 0x8c41, 0x8023, 0x8842, 0x8c42, 0x8c62, 0x8c63, 0x9021, 0x9421, 0x9421,
        0x9421, 0x9041, 0x9061, 0x9c41, 0x9822, 0x9442, 0x9062, 0x9443, 0x9063, 0x9842, 0x9842, 0x9c42, 0x9c63, 0x9481, 0x9ca3, 0x8405,
        0x8025, 0x8445, 0x8426, 0x8446, 0x9845, 0x8c84, 0x8ca5, 0x9084, 0x9084, 0x90a5, 0x9884, 0x90c6, 0x90c7, 0x90e7, 0x98c6, 0x98c6,
        0x9cc7, 0x9ce7, 0x9ce7, 0xa423, 0xa062, 0xa063, 0xa463, 0xa463, 0xa463, 0xac62, 0xa083, 0xb083, 0xa464, 0xb064, 0xa084, 0xa084,
        0xa4a5, 0xa884, 0xac84, 0xaca5, 0xa8c5, 0xaca6, 0xa8c6, 0xacc6, 0xace7, 0xb4a5, 0xb4c5, 0xb4e5, 0xb8c5, 0xb0c6, 0xb8e7, 0x8903,
        0x8903, 0x8d44, 0x8d44, 0x9185, 0x9185, 0x95a6, 0x95a6, 0x95e7, 0x95e7, 0xb566, 0x8428, 0x8848, 0x8869, 0x8c69, 0x886a, 0x8c6a,
        0x884b, 0x8ce8, 0x8c8b, 0x9488, 0x90e8, 0x988b, 0x98ca, 0x884c, 0x8c6d, 0x848c, 0x8c8c, 0x8c8d, 0x8ced, 0x908e, 0x90af, 0x90ce,
        0x90ef, 0x98cf, 0xbce8, 0x9109, 0x912a, 0x990a, 0x914b, 0x954b, 0x8d2e, 0x8d0f, 0x916c, 0x916d, 0x9d0f, 0x958d, 0x918e, 0x91ae,
        0x95ae, 0xa108, 0xa529, 0xa94a, 0xad6b, 0xb528, 0xb529, 0xb908, 0xbd29, 0xb549, 0xb52a, 0xbd4a, 0xbd8b, 0xb18c, 0xc485, 0xc0c7,
        0xc106, 0xc14a, 0xcd4b, 0xc58c, 0xc5ac, 0xcdac, 0xcdad, 0xc5cd, 0xd1ce, 0xd5ef, 0x9a28, 0x9a28, 0x9e69, 0x9e69, 0xa2aa, 0xa2aa,
        0xa6eb, 0xa6eb, 0xab2b, 0xab2b, 0xaf6c, 0xaf6c, 0x8000, 0x8c91, 0x8cf0, 0x8cd1, 0x94b0, 0x94b1, 0x94d1, 0x9cf1, 0x9093, 0x94d2,
        0x94d3, 0x94f3, 0x98d3, 0x94f4, 0x94d5, 0x98f4, 0x8d30, 0x8d12, 0x9511, 0x9531, 0x9d11, 0x9d31, 0x9551, 0x9533, 0x9d13, 0x9553,
        0x9573, 0x8db2, 0x9190, 0x91d0, 0x95d0, 0x91f1, 0x95f1, 0x95b3, 0x95d3, 0x99f3, 0x9514, 0x9535, 0x9554, 0x9574, 0x9555, 0x9d74,
        0x9d36, 0x95b4, 0x9595, 0x95d5, 0x95f5, 0x9596, 0x95d6, 0xa150, 0xa133, 0xa174, 0xa5d5, 0xa5b6, 0xa1d6, 0xb1d6, 0xfc1f, 0x9212,
        0x9612, 0x9613, 0x9233, 0x9633, 0x9634, 0x9235, 0x9635, 0x9254, 0x9655, 0x9275, 0x9675, 0x9656, 0x9276, 0x9676, 0x9676, 0x96b5,
        0x9297, 0x9697, 0x96b7, 0x9eb6, 0x9ed6, 0xa656, 0xaa56, 0xb637, 0xaa97, 0xc632, 0xda10, 0xde31, 0x8000, 0x8000, 0x8000, 0x8000
    },
    // PAL 30: 'DATA' sprite atlas lump in 'MEDOOM.WAD' (contains 'MASTER EDITION' text)
    {
        0x0000, 0x8108, 0x8cea, 0x8443, 0x8022, 0x992b, 0x910b, 0x88a6, 0x8421, 0x94ea, 0x952e, 0x9950, 0x8cc9, 0x910d, 0x9d92, 0xa9f9,
        0x8cc8, 0x88a7, 0x954f, 0xa1b5, 0xa5f8, 0x9971, 0x9db3, 0xa5d7, 0x8004, 0x800b, 0x9cfd, 0x8003, 0x8006, 0x8007, 0xa1b4, 0xae1a,
        0xb23b, 0xb65c, 0xb67d, 0x8c63, 0x98db, 0x94b9, 0x94b6, 0x9094, 0x8c71, 0x884f, 0x842d, 0x927e, 0xa6bd, 0x89d8, 0x852e, 0x850e,
        0x80ed, 0x80aa, 0x8089, 0x8067, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
        0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000
    },
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_Doom(GameConstants& consts, const bool bIsDemoVersion) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";

    if (!bIsDemoVersion) {
        consts.demos[0] = ClassicDemoDef{ "DEMO1.LMP", false, (Game::gGameVariant == GameVariant::PAL), true  };
        consts.demos[1] = ClassicDemoDef{ "DEMO2.LMP", false, (Game::gGameVariant == GameVariant::PAL), false };
    } else {
        consts.demos[0] = ClassicDemoDef{ "DEMO2.LMP", false, (Game::gGameVariant == GameVariant::PAL), true  };    // Demo only uses 'DEMO2.LMP'!
    }

    consts.saveFilePrefix = "Doom_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_Doom;
    consts.netGameId = NET_GAMEID_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_DOOM;
    consts.texPalette_BUTTONS = MAINPAL;
    consts.numPalettesRequired = NUMPALETTES_DOOM;
    consts.bUseFinalDoomSkyPalettes = false;
    consts.doomLogoYPos = DOOM_LOGO_YPOS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'Final Doom'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_FinalDoom(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "PSXDOOM/ABIN/MOVIE.STR";
    consts.demos[0] = ClassicDemoDef{ "DEMO1.LMP", true, (Game::gGameVariant == GameVariant::PAL), true  };
    consts.demos[1] = ClassicDemoDef{ "DEMO2.LMP", true, (Game::gGameVariant == GameVariant::PAL), false };
    consts.saveFilePrefix = "FDoom_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_FDoom;
    consts.netGameId = NET_GAMEID_FINAL_DOOM;
    consts.baseNumAnims = BASE_NUM_ANIMS_FDOOM;
    consts.texPalette_BUTTONS = UIPAL2;
    consts.numPalettesRequired = NUMPALETTES_FINAL_DOOM;
    consts.bUseFinalDoomSkyPalettes = true;
    consts.doomLogoYPos = DOOM_LOGO_YPOS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'GEC Master Edition (Beta 3)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_Beta3(GameConstants& consts) noexcept {
    // Default to the settings used by Final Doom for any unspecified fields
    populateConsts_FinalDoom(consts);

    // Quick hack to support the Master Edition:
    // (1) Load the Final Doom WAD first so that the .ROM format maps reference the correct lump numbers for floor and wall textures.
    // (2) Load the regular Doom WAD on top so we get extra/missing assets from that WAD.
    // (3) Load the GEC WAD so we can get a few resources for the credits screen.
    // (4) But we want Final Doom assets to take precedence over everything, so load that WAD on top of everything again.
    // 
    // This method is a bit hacky, but does the job to merge the contents of the 2 separate IWADs.
    consts.mainWads[0] = CdFile::PSXFINAL_WAD;
    consts.mainWads[1] = CdFile::MEDOOM_WAD;
    consts.mainWads[2] = CdFile::PSXDOOM_WAD;
    consts.mainWads[3] = CdFile::PSXFINAL_WAD;

    // Lump name remapping to support this game:
    consts.mainWadLumpRemappers[0] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Use the main menu background from 'MEDOOM.WAD' instead of 'PSXFINAL.WAD'
        if (oldName == "BACK")      { return "_UNUSED1";  }
        // Use the 'DOOM' logo from 'MEDOOM.WAD' instead of 'PSXFINAL.WAD'
        if (oldName == "DOOM")      { return "_UNUSED2";  }

        // Leave this lump name alone!
        return oldName;
    };

    consts.mainWadLumpRemappers[1] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Need to access this palette in 'MEDOOM.WAD' separately to the Final Doom 'PLAYPAL' for decoding intro logos
        if (oldName == "PLAYPAL")   { return "GECINPAL";  }

        // Leave this lump name alone!
        return oldName;
    };

    consts.mainWadLumpRemappers[2] = [](const WadLumpName& oldName) noexcept -> WadLumpName {
        // Rename the 'Doom' version of this texture so we can use it instead of the 'Final Doom' version in 'Doom' episode maps
        if (oldName == "REDROK01")  { return "REDROKX1";  }

        // Leave this lump name alone!
        return oldName;
    };

    // Demo files: note that we have to remap the map numbers for the 'Final Doom' episodes!
    {
        const ClassicDemoDef::MapNumOverrideFn remapMasterLevelsMapNum = [](const int32_t mapNum) noexcept { return mapNum + 30; };
        const ClassicDemoDef::MapNumOverrideFn remapTntMapNum          = [](const int32_t mapNum) noexcept { return mapNum + 50; };
        const ClassicDemoDef::MapNumOverrideFn remapPlutoniaMapNum     = [](const int32_t mapNum) noexcept { return mapNum + 70; };

        consts.demos[0] = ClassicDemoDef{ "DEMO1.LMP", false, false, true                           };
        consts.demos[1] = ClassicDemoDef{ "DEMO2.LMP", false, false, true                           };
        consts.demos[2] = ClassicDemoDef{ "DEMO3.LMP", true,  false, true,  remapMasterLevelsMapNum };
        consts.demos[3] = ClassicDemoDef{ "DEMO4.LMP", true,  false, true,  remapMasterLevelsMapNum };
        consts.demos[4] = ClassicDemoDef{ "DEMO5.LMP", true,  false, true,  remapTntMapNum          };
        consts.demos[5] = ClassicDemoDef{ "DEMO6.LMP", true,  false, true,  remapTntMapNum          };
        consts.demos[6] = ClassicDemoDef{ "DEMO7.LMP", true,  false, true,  remapPlutoniaMapNum     };
        consts.demos[7] = ClassicDemoDef{ "DEMO8.LMP", true,  false, false, remapPlutoniaMapNum     };
    }

    // All other differing constants
    consts.introMovies[0] = "DATA/MOVIE.STR";
    consts.introMovies[1] = "DATA/GEC.STR";
    consts.introMovies[2] = "DATA/DWORLD.STR";
    consts.saveFilePrefix = "GecMe_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_GecMe;
    consts.netGameId = NET_GAMEID_GEC_ME_BETA3;
    consts.pExtraPalettes = GEC_ME_BETA3_EXTRA_PALETTES;
    consts.numExtraPalettes = C_ARRAY_SIZE(GEC_ME_BETA3_EXTRA_PALETTES);
    consts.doomLogoYPos = DOOM_LOGO_YPOS_GEC_ME;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for '[GEC] Master Edition tools: single map test disc (Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_TestMap_Doom(GameConstants& consts) noexcept {
    populateConsts_Doom(consts, false);
    consts.introMovies[0] = "";
    consts.demos[0] = {};
    consts.demos[1] = {};
    consts.netGameId = NET_GAMEID_GEC_ME_TESTMAP_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for '[GEC] Master Edition tools: single map test disc (Final Doom format)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_TestMap_FinalDoom(GameConstants& consts) noexcept {
    populateConsts_FinalDoom(consts);
    consts.introMovies[0] = "";
    consts.demos[0] = {};
    consts.demos[1] = {};
    consts.netGameId = NET_GAMEID_GEC_ME_TESTMAP_FINAL_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the values of all constants for 'GEC Master Edition (Beta 4)'
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateConsts_GEC_ME_Beta4(GameConstants& consts) noexcept {
    consts.mainWads[0] = CdFile::PSXDOOM_WAD;
    consts.introMovies[0] = "DATA/MOVIE.STR";
    consts.introMovies[1] = "DATA/GEC.STR";
    consts.introMovies[2] = "DATA/DWORLD.STR";
    consts.demos[0] = ClassicDemoDef{ "NDEMO1.LMP", true, false, true  };   // TODO: GEC ME BETA 4: Verify these demos
    consts.demos[1] = ClassicDemoDef{ "NDEMO2.LMP", true, false, true  };
    consts.demos[2] = ClassicDemoDef{ "NDEMO3.LMP", true, false, true  };
    consts.demos[3] = ClassicDemoDef{ "NDEMO4.LMP", true, false, true  };
    consts.demos[4] = ClassicDemoDef{ "NDEMO5.LMP", true, false, true  };
    consts.demos[5] = ClassicDemoDef{ "NDEMO6.LMP", true, false, true  };
    consts.demos[6] = ClassicDemoDef{ "NDEMO7.LMP", true, false, true  };
    consts.demos[7] = ClassicDemoDef{ "NDEMO8.LMP", true, false, false };
    consts.saveFilePrefix = "GecMe_";
    consts.pLastPasswordField = &PlayerPrefs::gLastPassword_GecMe;
    consts.numPalettesRequired = 31;
    consts.netGameId = NET_GAMEID_GEC_ME_BETA4;
    consts.baseNumAnims = BASE_NUM_ANIMS_FDOOM;     // TODO: GEC ME BETA 4: check this
    consts.texPalette_BUTTONS = UIPAL2;             // TODO: GEC ME BETA 4: check this
    consts.bUseFinalDoomSkyPalettes = true;         // TODO: GEC ME BETA 4: check this
    consts.doomLogoYPos = DOOM_LOGO_YPOS_GEC_ME;    // TODO: GEC ME BETA 4: check this
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Populates the set of game constants for the current game type
//------------------------------------------------------------------------------------------------------------------------------------------
void GameConstants::populate(const GameType gameType, const bool bIsDemoVersion) noexcept {
    // Default init first
    *this = {};

    // Then populate all fields explicitly
    switch (gameType) {
        case GameType::Doom:                        populateConsts_Doom(*this, bIsDemoVersion);         break;
        case GameType::FinalDoom:                   populateConsts_FinalDoom(*this);                    break;
        case GameType::GEC_ME_Beta3:                populateConsts_GEC_ME_Beta3(*this);                 break;
        case GameType::GEC_ME_TestMap_Doom:         populateConsts_GEC_ME_TestMap_Doom(*this);          break;
        case GameType::GEC_ME_TestMap_FinalDoom:    populateConsts_GEC_ME_TestMap_FinalDoom(*this);     break;
        case GameType::GEC_ME_Beta4:                populateConsts_GEC_ME_Beta4(*this);                 break;

        default:
            FatalErrors::raise("GameConstants::populate(): unhandled game type!");
    }
}

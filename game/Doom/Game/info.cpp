#include "info.h"

#include "p_enemy.h"
#include "p_pspr.h"

const char* gSprNames[NUMSPRITES] = {
    "TROO", "SHTG", "PUNG", "PISG", "PISF", "SHTF", "SHT2", "CHGG",     // 000 - 007
    "CHGF", "MISG", "MISF", "SAWG", "PLSG", "PLSF", "BFGG", "BFGF",     // 008 - 015
    "BLUD", "PUFF", "BAL1", "BAL2", "BAL7", "PLSS", "PLSE", "MISL",     // 016 - 023
    "BFS1", "BFE1", "BFE2", "TFOG", "IFOG", "PLAY", "POSS", "SPOS",     // 024 - 031
    "FATB", "FBXP", "SKEL", "MANF", "FATT", "CPOS", "SARG", "HEAD",     // 032 - 039
    "BOSS", "BOS2", "SKUL", "SPID", "BSPI", "APLS", "APBX", "CYBR",     // 040 - 047
    "PAIN", "ARM1", "ARM2", "BAR1", "BEXP", "FCAN", "BON1", "BON2",     // 048 - 055
    "BKEY", "RKEY", "YKEY", "BSKU", "RSKU", "YSKU", "STIM", "MEDI",     // 056 - 063
    "SOUL", "PINV", "PSTR", "PINS", "MEGA", "SUIT", "PMAP", "PVIS",     // 064 - 071
    "CLIP", "AMMO", "ROCK", "BROK", "CELL", "CELP", "SHEL", "SBOX",     // 072 - 079
    "BPAK", "BFUG", "MGUN", "CSAW", "LAUN", "PLAS", "SHOT", "SGN2",     // 080 - 087
    "COLU", "SMT2", "POL2", "POL5", "POL4", "POL1", "GOR2", "GOR3",     // 088 - 095
    "GOR4", "GOR5", "SMIT", "COL1", "COL2", "COL3", "COL4", "COL6",     // 096 - 103
    "CAND", "CBRA", "TRE1", "ELEC", "FSKU", "SMBT", "SMGT", "SMRT",     // 104 - 111
    "HANC", "BLCH", "HANL", "DED1", "DED2", "DED3", "DED4", "DED5",     // 112 - 119
    "DED6", "TLMP", "TLP2", "COL5", "CEYE", "TBLU", "TGRN", "TRED",     // 120 - 127
    "GOR1", "POL3", "POL6", "TRE2", "HDB1", "HDB2", "HDB3", "HDB4",     // 128 - 135
    "HDB5", "HDB6", "POB1", "POB2", "BRS1"                              // 136 - 140
};

state_t gStates[NUMSTATES] = {
    { SPR_TROO,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_NULL
    { SPR_SHTG,  4,                   0,   A_Light0,         S_NULL,           0,  0 },  // S_LIGHTDONE
    { SPR_PUNG,  0,                   1,   A_WeaponReady,    S_PUNCH,          0,  0 },  // S_PUNCH
    { SPR_PUNG,  0,                   1,   A_Lower,          S_PUNCHDOWN,      0,  0 },  // S_PUNCHDOWN
    { SPR_PUNG,  0,                   1,   A_Raise,          S_PUNCHUP,        0,  0 },  // S_PUNCHUP
    { SPR_PUNG,  1,                   2,   nullptr,          S_PUNCH2,         0,  0 },  // S_PUNCH1
    { SPR_PUNG,  2,                   2,   A_Punch,          S_PUNCH3,         0,  0 },  // S_PUNCH2
    { SPR_PUNG,  3,                   2,   nullptr,          S_PUNCH4,         0,  0 },  // S_PUNCH3
    { SPR_PUNG,  2,                   2,   nullptr,          S_PUNCH5,         0,  0 },  // S_PUNCH4
    { SPR_PUNG,  1,                   2,   A_ReFire,         S_PUNCH,          0,  0 },  // S_PUNCH5
    { SPR_PISG,  0,                   1,   A_WeaponReady,    S_PISTOL,         0,  0 },  // S_PISTOL
    { SPR_PISG,  0,                   1,   A_Lower,          S_PISTOLDOWN,     0,  0 },  // S_PISTOLDOWN
    { SPR_PISG,  0,                   1,   A_Raise,          S_PISTOLUP,       0,  0 },  // S_PISTOLUP
    { SPR_PISG,  0,                   2,   nullptr,          S_PISTOL2,        0,  0 },  // S_PISTOL1
    { SPR_PISG,  1,                   3,   A_FirePistol,     S_PISTOL3,        0,  0 },  // S_PISTOL2
    { SPR_PISG,  2,                   3,   nullptr,          S_PISTOL4,        0,  0 },  // S_PISTOL3
    { SPR_PISG,  1,                   2,   A_ReFire,         S_PISTOL,         0,  0 },  // S_PISTOL4
    { SPR_PISF,  0 | FF_FULLBRIGHT,   3,   A_Light1,         S_LIGHTDONE,      0,  0 },  // S_PISTOLFLASH
    { SPR_SHTG,  0,                   1,   A_WeaponReady,    S_SGUN,           0,  0 },  // S_SGUN
    { SPR_SHTG,  0,                   1,   A_Lower,          S_SGUNDOWN,       0,  0 },  // S_SGUNDOWN
    { SPR_SHTG,  0,                   1,   A_Raise,          S_SGUNUP,         0,  0 },  // S_SGUNUP
    { SPR_SHTG,  0,                   2,   nullptr,          S_SGUN2,          0,  0 },  // S_SGUN1
    { SPR_SHTG,  0,                   2,   A_FireShotgun,    S_SGUN3,          0,  0 },  // S_SGUN2
    { SPR_SHTG,  1,                   3,   nullptr,          S_SGUN4,          0,  0 },  // S_SGUN3
    { SPR_SHTG,  2,                   2,   nullptr,          S_SGUN5,          0,  0 },  // S_SGUN4
    { SPR_SHTG,  3,                   2,   nullptr,          S_SGUN6,          0,  0 },  // S_SGUN5
    { SPR_SHTG,  2,                   2,   nullptr,          S_SGUN7,          0,  0 },  // S_SGUN6
    { SPR_SHTG,  1,                   2,   nullptr,          S_SGUN8,          0,  0 },  // S_SGUN7
    { SPR_SHTG,  0,                   2,   nullptr,          S_SGUN9,          0,  0 },  // S_SGUN8
    { SPR_SHTG,  0,                   3,   A_ReFire,         S_SGUN,           0,  0 },  // S_SGUN9
    { SPR_SHTF,  0 | FF_FULLBRIGHT,   2,   A_Light1,         S_SGUNFLASH2,     0,  0 },  // S_SGUNFLASH1
    { SPR_SHTF,  1 | FF_FULLBRIGHT,   1,   A_Light2,         S_LIGHTDONE,      0,  0 },  // S_SGUNFLASH2
    { SPR_SHT2,  0,                   1,   A_WeaponReady,    S_DSGUN,          0,  0 },  // S_DSGUN
    { SPR_SHT2,  0,                   1,   A_Lower,          S_DSGUNDOWN,      0,  0 },  // S_DSGUNDOWN
    { SPR_SHT2,  0,                   1,   A_Raise,          S_DSGUNUP,        0,  0 },  // S_DSGUNUP
    { SPR_SHT2,  0,                   2,   nullptr,          S_DSGUN2,         0,  0 },  // S_DSGUN1
    { SPR_SHT2,  0,                   3,   A_FireShotgun2,   S_DSGUN3,         0,  0 },  // S_DSGUN2
    { SPR_SHT2,  1,                   3,   nullptr,          S_DSGUN4,         0,  0 },  // S_DSGUN3
    { SPR_SHT2,  2,                   3,   A_CheckReload,    S_DSGUN5,         0,  0 },  // S_DSGUN4
    { SPR_SHT2,  3,                   3,   A_OpenShotgun2,   S_DSGUN6,         0,  0 },  // S_DSGUN5
    { SPR_SHT2,  4,                   3,   nullptr,          S_DSGUN7,         0,  0 },  // S_DSGUN6
    { SPR_SHT2,  5,                   3,   A_LoadShotgun2,   S_DSGUN8,         0,  0 },  // S_DSGUN7
    { SPR_SHT2,  6,                   2,   nullptr,          S_DSGUN9,         0,  0 },  // S_DSGUN8
    { SPR_SHT2,  7,                   2,   A_CloseShotgun2,  S_DSGUN10,        0,  0 },  // S_DSGUN9
    { SPR_SHT2,  0,                   2,   A_ReFire,         S_DSGUN,          0,  0 },  // S_DSGUN10
    { SPR_SHT2,  1,                   3,   nullptr,          S_DSNR2,          0,  0 },  // S_DSNR1
    { SPR_SHT2,  0,                   1,   nullptr,          S_DSGUNDOWN,      0,  0 },  // S_DSNR2
    { SPR_SHT2,  8 | FF_FULLBRIGHT,   2,   A_Light1,         S_DSGUNFLASH2,    0,  0 },  // S_DSGUNFLASH1
    { SPR_SHT2,  9 | FF_FULLBRIGHT,   2,   A_Light2,         S_LIGHTDONE,      0,  0 },  // S_DSGUNFLASH2
    { SPR_CHGG,  0,                   1,   A_WeaponReady,    S_CHAIN,          0,  0 },  // S_CHAIN
    { SPR_CHGG,  0,                   1,   A_Lower,          S_CHAINDOWN,      0,  0 },  // S_CHAINDOWN
    { SPR_CHGG,  0,                   1,   A_Raise,          S_CHAINUP,        0,  0 },  // S_CHAINUP
    { SPR_CHGG,  0,                   2,   A_FireCGun,       S_CHAIN2,         0,  0 },  // S_CHAIN1
    { SPR_CHGG,  1,                   2,   A_FireCGun,       S_CHAIN3,         0,  0 },  // S_CHAIN2
    { SPR_CHGG,  1,                   0,   A_ReFire,         S_CHAIN,          0,  0 },  // S_CHAIN3
    { SPR_CHGF,  0 | FF_FULLBRIGHT,   3,   A_Light1,         S_LIGHTDONE,      0,  0 },  // S_CHAINFLASH1
    { SPR_CHGF,  1 | FF_FULLBRIGHT,   2,   A_Light2,         S_LIGHTDONE,      0,  0 },  // S_CHAINFLASH2
    { SPR_MISG,  0,                   1,   A_WeaponReady,    S_MISSILE,        0,  0 },  // S_MISSILE
    { SPR_MISG,  0,                   1,   A_Lower,          S_MISSILEDOWN,    0,  0 },  // S_MISSILEDOWN
    { SPR_MISG,  0,                   1,   A_Raise,          S_MISSILEUP,      0,  0 },  // S_MISSILEUP
    { SPR_MISG,  1,                   4,   A_GunFlash,       S_MISSILE2,       0,  0 },  // S_MISSILE1
    { SPR_MISG,  1,                   6,   A_FireMissile,    S_MISSILE3,       0,  0 },  // S_MISSILE2
    { SPR_MISG,  1,                   0,   A_ReFire,         S_MISSILE,        0,  0 },  // S_MISSILE3
    { SPR_MISF,  0 | FF_FULLBRIGHT,   2,   A_Light1,         S_MISSILEFLASH2,  0,  0 },  // S_MISSILEFLASH1
    { SPR_MISF,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_MISSILEFLASH3,  0,  0 },  // S_MISSILEFLASH2
    { SPR_MISF,  2 | FF_FULLBRIGHT,   2,   A_Light2,         S_MISSILEFLASH4,  0,  0 },  // S_MISSILEFLASH3
    { SPR_MISF,  3 | FF_FULLBRIGHT,   2,   A_Light2,         S_LIGHTDONE,      0,  0 },  // S_MISSILEFLASH4
    { SPR_SAWG,  2,                   2,   A_WeaponReady,    S_SAWB,           0,  0 },  // S_SAW
    { SPR_SAWG,  3,                   2,   A_WeaponReady,    S_SAW,            0,  0 },  // S_SAWB
    { SPR_SAWG,  2,                   1,   A_Lower,          S_SAWDOWN,        0,  0 },  // S_SAWDOWN
    { SPR_SAWG,  2,                   1,   A_Raise,          S_SAWUP,          0,  0 },  // S_SAWUP
    { SPR_SAWG,  0,                   2,   A_Saw,            S_SAW2,           0,  0 },  // S_SAW1
    { SPR_SAWG,  1,                   2,   A_Saw,            S_SAW3,           0,  0 },  // S_SAW2
    { SPR_SAWG,  1,                   0,   A_ReFire,         S_SAW,            0,  0 },  // S_SAW3
    { SPR_PLSG,  0,                   1,   A_WeaponReady,    S_PLASMA,         0,  0 },  // S_PLASMA
    { SPR_PLSG,  0,                   1,   A_Lower,          S_PLASMADOWN,     0,  0 },  // S_PLASMADOWN
    { SPR_PLSG,  0,                   1,   A_Raise,          S_PLASMAUP,       0,  0 },  // S_PLASMAUP
    { SPR_PLSG,  0,                   2,   A_FirePlasma,     S_PLASMA2,        0,  0 },  // S_PLASMA1
    { SPR_PLSG,  1,                   10,  A_ReFire,         S_PLASMA,         0,  0 },  // S_PLASMA2
    { SPR_PLSF,  0 | FF_FULLBRIGHT,   2,   A_Light1,         S_LIGHTDONE,      0,  0 },  // S_PLASMAFLASH1
    { SPR_PLSF,  1 | FF_FULLBRIGHT,   2,   A_Light1,         S_LIGHTDONE,      0,  0 },  // S_PLASMAFLASH2
    { SPR_BFGG,  0,                   1,   A_WeaponReady,    S_BFG,            0,  0 },  // S_BFG
    { SPR_BFGG,  0,                   1,   A_Lower,          S_BFGDOWN,        0,  0 },  // S_BFGDOWN
    { SPR_BFGG,  0,                   1,   A_Raise,          S_BFGUP,          0,  0 },  // S_BFGUP
    { SPR_BFGG,  0,                   10,  A_BFGsound,       S_BFG2,           0,  0 },  // S_BFG1
    { SPR_BFGG,  1,                   5,   A_GunFlash,       S_BFG3,           0,  0 },  // S_BFG2
    { SPR_BFGG,  1,                   5,   A_FireBFG,        S_BFG4,           0,  0 },  // S_BFG3
    { SPR_BFGG,  1,                   10,  A_ReFire,         S_BFG,            0,  0 },  // S_BFG4
    { SPR_BFGF,  0 | FF_FULLBRIGHT,   5,   A_Light1,         S_BFGFLASH2,      0,  0 },  // S_BFGFLASH1
    { SPR_BFGF,  1 | FF_FULLBRIGHT,   3,   A_Light2,         S_LIGHTDONE,      0,  0 },  // S_BFGFLASH2
    { SPR_BLUD,  2,                   4,   nullptr,          S_BLOOD2,         0,  0 },  // S_BLOOD1
    { SPR_BLUD,  1,                   4,   nullptr,          S_BLOOD3,         0,  0 },  // S_BLOOD2
    { SPR_BLUD,  0,                   4,   nullptr,          S_NULL,           0,  0 },  // S_BLOOD3
    { SPR_PUFF,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_PUFF2,          0,  0 },  // S_PUFF1
    { SPR_PUFF,  1,                   2,   nullptr,          S_PUFF3,          0,  0 },  // S_PUFF2
    { SPR_PUFF,  2,                   2,   nullptr,          S_PUFF4,          0,  0 },  // S_PUFF3
    { SPR_PUFF,  3,                   2,   nullptr,          S_NULL,           0,  0 },  // S_PUFF4
    { SPR_BAL1,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_TBALL2,         0,  0 },  // S_TBALL1
    { SPR_BAL1,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_TBALL1,         0,  0 },  // S_TBALL2
    { SPR_BAL1,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_TBALLX2,        0,  0 },  // S_TBALLX1
    { SPR_BAL1,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_TBALLX3,        0,  0 },  // S_TBALLX2
    { SPR_BAL1,  4 | FF_FULLBRIGHT,   3,   nullptr,          S_NULL,           0,  0 },  // S_TBALLX3
    { SPR_BAL2,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_RBALL2,         0,  0 },  // S_RBALL1
    { SPR_BAL2,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_RBALL1,         0,  0 },  // S_RBALL2
    { SPR_BAL2,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_RBALLX2,        0,  0 },  // S_RBALLX1
    { SPR_BAL2,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_RBALLX3,        0,  0 },  // S_RBALLX2
    { SPR_BAL2,  4 | FF_FULLBRIGHT,   3,   nullptr,          S_NULL,           0,  0 },  // S_RBALLX3
    { SPR_BAL7,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BRBALL2,        0,  0 },  // S_BRBALL1
    { SPR_BAL7,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_BRBALL1,        0,  0 },  // S_BRBALL2
    { SPR_BAL7,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_BRBALLX2,       0,  0 },  // S_BRBALLX1
    { SPR_BAL7,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_BRBALLX3,       0,  0 },  // S_BRBALLX2
    { SPR_BAL7,  4 | FF_FULLBRIGHT,   3,   nullptr,          S_NULL,           0,  0 },  // S_BRBALLX3
    { SPR_PLSS,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_PLASBALL2,      0,  0 },  // S_PLASBALL
    { SPR_PLSS,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_PLASBALL,       0,  0 },  // S_PLASBALL2
    { SPR_PLSE,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_PLASEXP2,       0,  0 },  // S_PLASEXP
    { SPR_PLSE,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_PLASEXP3,       0,  0 },  // S_PLASEXP2
    { SPR_PLSE,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_PLASEXP4,       0,  0 },  // S_PLASEXP3
    { SPR_PLSE,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_PLASEXP5,       0,  0 },  // S_PLASEXP4
    { SPR_PLSE,  4 | FF_FULLBRIGHT,   2,   nullptr,          S_NULL,           0,  0 },  // S_PLASEXP5
    { SPR_MISL,  0 | FF_FULLBRIGHT,   1,   nullptr,          S_ROCKET,         0,  0 },  // S_ROCKET
    { SPR_BFS1,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BFGSHOT2,       0,  0 },  // S_BFGSHOT
    { SPR_BFS1,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_BFGSHOT,        0,  0 },  // S_BFGSHOT2
    { SPR_BFE1,  0 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGLAND2,       0,  0 },  // S_BFGLAND
    { SPR_BFE1,  1 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGLAND3,       0,  0 },  // S_BFGLAND2
    { SPR_BFE1,  2 | FF_FULLBRIGHT,   4,   A_BFGSpray,       S_BFGLAND4,       0,  0 },  // S_BFGLAND3
    { SPR_BFE1,  3 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGLAND5,       0,  0 },  // S_BFGLAND4
    { SPR_BFE1,  4 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGLAND6,       0,  0 },  // S_BFGLAND5
    { SPR_BFE1,  5 | FF_FULLBRIGHT,   4,   nullptr,          S_NULL,           0,  0 },  // S_BFGLAND6
    { SPR_BFE2,  0 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGEXP2,        0,  0 },  // S_BFGEXP
    { SPR_BFE2,  1 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGEXP3,        0,  0 },  // S_BFGEXP2
    { SPR_BFE2,  2 | FF_FULLBRIGHT,   4,   nullptr,          S_BFGEXP4,        0,  0 },  // S_BFGEXP3
    { SPR_BFE2,  3 | FF_FULLBRIGHT,   4,   nullptr,          S_NULL,           0,  0 },  // S_BFGEXP4
    { SPR_MISL,  1 | FF_FULLBRIGHT,   4,   A_Explode,        S_EXPLODE2,       0,  0 },  // S_EXPLODE1
    { SPR_MISL,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_EXPLODE3,       0,  0 },  // S_EXPLODE2
    { SPR_MISL,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_NULL,           0,  0 },  // S_EXPLODE3
    { SPR_TFOG,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG01,         0,  0 },  // S_TFOG
    { SPR_TFOG,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG02,         0,  0 },  // S_TFOG01
    { SPR_TFOG,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG2,          0,  0 },  // S_TFOG02
    { SPR_TFOG,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG3,          0,  0 },  // S_TFOG2
    { SPR_TFOG,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG4,          0,  0 },  // S_TFOG3
    { SPR_TFOG,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG5,          0,  0 },  // S_TFOG4
    { SPR_TFOG,  4 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG6,          0,  0 },  // S_TFOG5
    { SPR_TFOG,  5 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG7,          0,  0 },  // S_TFOG6
    { SPR_TFOG,  6 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG8,          0,  0 },  // S_TFOG7
    { SPR_TFOG,  7 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG9,          0,  0 },  // S_TFOG8
    { SPR_TFOG,  8 | FF_FULLBRIGHT,   3,   nullptr,          S_TFOG10,         0,  0 },  // S_TFOG9
    { SPR_TFOG,  9 | FF_FULLBRIGHT,   3,   nullptr,          S_NULL,           0,  0 },  // S_TFOG10
    { SPR_IFOG,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG01,         0,  0 },  // S_IFOG
    { SPR_IFOG,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG02,         0,  0 },  // S_IFOG01
    { SPR_IFOG,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG2,          0,  0 },  // S_IFOG02
    { SPR_IFOG,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG3,          0,  0 },  // S_IFOG2
    { SPR_IFOG,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG4,          0,  0 },  // S_IFOG3
    { SPR_IFOG,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_IFOG5,          0,  0 },  // S_IFOG4
    { SPR_IFOG,  4 | FF_FULLBRIGHT,   3,   nullptr,          S_NULL,           0,  0 },  // S_IFOG5
    { SPR_PLAY,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_PLAY
    { SPR_PLAY,  0,                   2,   nullptr,          S_PLAY_RUN2,      0,  0 },  // S_PLAY_RUN1
    { SPR_PLAY,  1,                   2,   nullptr,          S_PLAY_RUN3,      0,  0 },  // S_PLAY_RUN2
    { SPR_PLAY,  2,                   2,   nullptr,          S_PLAY_RUN4,      0,  0 },  // S_PLAY_RUN3
    { SPR_PLAY,  3,                   2,   nullptr,          S_PLAY_RUN1,      0,  0 },  // S_PLAY_RUN4
    { SPR_PLAY,  4,                   2,   nullptr,          S_PLAY_ATK2,      0,  0 },  // S_PLAY_ATK1
    { SPR_PLAY,  5 | FF_FULLBRIGHT,   4,   nullptr,          S_PLAY,           0,  0 },  // S_PLAY_ATK2
    { SPR_PLAY,  6,                   2,   nullptr,          S_PLAY_PAIN2,     0,  0 },  // S_PLAY_PAIN
    { SPR_PLAY,  6,                   2,   A_Pain,           S_PLAY,           0,  0 },  // S_PLAY_PAIN2
    { SPR_PLAY,  7,                   5,   nullptr,          S_PLAY_DIE2,      0,  0 },  // S_PLAY_DIE1
    { SPR_PLAY,  8,                   5,   A_Scream,         S_PLAY_DIE3,      0,  0 },  // S_PLAY_DIE2
    { SPR_PLAY,  9,                   5,   A_Fall,           S_PLAY_DIE4,      0,  0 },  // S_PLAY_DIE3
    { SPR_PLAY,  10,                  5,   nullptr,          S_PLAY_DIE5,      0,  0 },  // S_PLAY_DIE4
    { SPR_PLAY,  11,                  5,   nullptr,          S_PLAY_DIE6,      0,  0 },  // S_PLAY_DIE5
    { SPR_PLAY,  12,                  5,   nullptr,          S_PLAY_DIE7,      0,  0 },  // S_PLAY_DIE6
    { SPR_PLAY,  13,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_PLAY_DIE7
    { SPR_PLAY,  14,                  2,   nullptr,          S_PLAY_XDIE2,     0,  0 },  // S_PLAY_XDIE1
    { SPR_PLAY,  15,                  2,   A_XScream,        S_PLAY_XDIE3,     0,  0 },  // S_PLAY_XDIE2
    { SPR_PLAY,  16,                  2,   A_Fall,           S_PLAY_XDIE4,     0,  0 },  // S_PLAY_XDIE3
    { SPR_PLAY,  17,                  2,   nullptr,          S_PLAY_XDIE5,     0,  0 },  // S_PLAY_XDIE4
    { SPR_PLAY,  18,                  2,   nullptr,          S_PLAY_XDIE6,     0,  0 },  // S_PLAY_XDIE5
    { SPR_PLAY,  19,                  2,   nullptr,          S_PLAY_XDIE7,     0,  0 },  // S_PLAY_XDIE6
    { SPR_PLAY,  20,                  2,   nullptr,          S_PLAY_XDIE8,     0,  0 },  // S_PLAY_XDIE7
    { SPR_PLAY,  21,                  2,   nullptr,          S_PLAY_XDIE9,     0,  0 },  // S_PLAY_XDIE8
    { SPR_PLAY,  22,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_PLAY_XDIE9
    { SPR_POSS,  0,                   5,   A_Look,           S_POSS_STND2,     0,  0 },  // S_POSS_STND
    { SPR_POSS,  1,                   5,   A_Look,           S_POSS_STND,      0,  0 },  // S_POSS_STND2
    { SPR_POSS,  0,                   2,   A_Chase,          S_POSS_RUN2,      0,  0 },  // S_POSS_RUN1
    { SPR_POSS,  0,                   2,   A_Chase,          S_POSS_RUN3,      0,  0 },  // S_POSS_RUN2
    { SPR_POSS,  1,                   2,   A_Chase,          S_POSS_RUN4,      0,  0 },  // S_POSS_RUN3
    { SPR_POSS,  1,                   2,   A_Chase,          S_POSS_RUN5,      0,  0 },  // S_POSS_RUN4
    { SPR_POSS,  2,                   2,   A_Chase,          S_POSS_RUN6,      0,  0 },  // S_POSS_RUN5
    { SPR_POSS,  2,                   2,   A_Chase,          S_POSS_RUN7,      0,  0 },  // S_POSS_RUN6
    { SPR_POSS,  3,                   2,   A_Chase,          S_POSS_RUN8,      0,  0 },  // S_POSS_RUN7
    { SPR_POSS,  3,                   2,   A_Chase,          S_POSS_RUN1,      0,  0 },  // S_POSS_RUN8
    { SPR_POSS,  4,                   5,   A_FaceTarget,     S_POSS_ATK2,      0,  0 },  // S_POSS_ATK1
    { SPR_POSS,  5,                   4,   A_PosAttack,      S_POSS_ATK3,      0,  0 },  // S_POSS_ATK2
    { SPR_POSS,  4,                   4,   nullptr,          S_POSS_RUN1,      0,  0 },  // S_POSS_ATK3
    { SPR_POSS,  6,                   1,   nullptr,          S_POSS_PAIN2,     0,  0 },  // S_POSS_PAIN
    { SPR_POSS,  6,                   2,   A_Pain,           S_POSS_RUN1,      0,  0 },  // S_POSS_PAIN2
    { SPR_POSS,  7,                   2,   nullptr,          S_POSS_DIE2,      0,  0 },  // S_POSS_DIE1
    { SPR_POSS,  8,                   3,   A_Scream,         S_POSS_DIE3,      0,  0 },  // S_POSS_DIE2
    { SPR_POSS,  9,                   2,   A_Fall,           S_POSS_DIE4,      0,  0 },  // S_POSS_DIE3
    { SPR_POSS,  10,                  3,   nullptr,          S_POSS_DIE5,      0,  0 },  // S_POSS_DIE4
    { SPR_POSS,  11,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_POSS_DIE5
    { SPR_POSS,  12,                  2,   nullptr,          S_POSS_XDIE2,     0,  0 },  // S_POSS_XDIE1
    { SPR_POSS,  13,                  3,   A_XScream,        S_POSS_XDIE3,     0,  0 },  // S_POSS_XDIE2
    { SPR_POSS,  14,                  2,   A_Fall,           S_POSS_XDIE4,     0,  0 },  // S_POSS_XDIE3
    { SPR_POSS,  15,                  3,   nullptr,          S_POSS_XDIE5,     0,  0 },  // S_POSS_XDIE4
    { SPR_POSS,  16,                  2,   nullptr,          S_POSS_XDIE6,     0,  0 },  // S_POSS_XDIE5
    { SPR_POSS,  17,                  3,   nullptr,          S_POSS_XDIE7,     0,  0 },  // S_POSS_XDIE6
    { SPR_POSS,  18,                  2,   nullptr,          S_POSS_XDIE8,     0,  0 },  // S_POSS_XDIE7
    { SPR_POSS,  19,                  3,   nullptr,          S_POSS_XDIE9,     0,  0 },  // S_POSS_XDIE8
    { SPR_POSS,  20,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_POSS_XDIE9
    { SPR_SPOS,  0,                   5,   A_Look,           S_SPOS_STND2,     0,  0 },  // S_SPOS_STND
    { SPR_SPOS,  1,                   5,   A_Look,           S_SPOS_STND,      0,  0 },  // S_SPOS_STND2
    { SPR_SPOS,  0,                   1,   A_Chase,          S_SPOS_RUN2,      0,  0 },  // S_SPOS_RUN1
    { SPR_SPOS,  0,                   2,   A_Chase,          S_SPOS_RUN3,      0,  0 },  // S_SPOS_RUN2
    { SPR_SPOS,  1,                   1,   A_Chase,          S_SPOS_RUN4,      0,  0 },  // S_SPOS_RUN3
    { SPR_SPOS,  1,                   2,   A_Chase,          S_SPOS_RUN5,      0,  0 },  // S_SPOS_RUN4
    { SPR_SPOS,  2,                   1,   A_Chase,          S_SPOS_RUN6,      0,  0 },  // S_SPOS_RUN5
    { SPR_SPOS,  2,                   2,   A_Chase,          S_SPOS_RUN7,      0,  0 },  // S_SPOS_RUN6
    { SPR_SPOS,  3,                   1,   A_Chase,          S_SPOS_RUN8,      0,  0 },  // S_SPOS_RUN7
    { SPR_SPOS,  3,                   2,   A_Chase,          S_SPOS_RUN1,      0,  0 },  // S_SPOS_RUN8
    { SPR_SPOS,  4,                   5,   A_FaceTarget,     S_SPOS_ATK2,      0,  0 },  // S_SPOS_ATK1
    { SPR_SPOS,  5 | FF_FULLBRIGHT,   5,   A_SPosAttack,     S_SPOS_ATK3,      0,  0 },  // S_SPOS_ATK2
    { SPR_SPOS,  4,                   5,   nullptr,          S_SPOS_RUN1,      0,  0 },  // S_SPOS_ATK3
    { SPR_SPOS,  6,                   1,   nullptr,          S_SPOS_PAIN2,     0,  0 },  // S_SPOS_PAIN
    { SPR_SPOS,  6,                   2,   A_Pain,           S_SPOS_RUN1,      0,  0 },  // S_SPOS_PAIN2
    { SPR_SPOS,  7,                   2,   nullptr,          S_SPOS_DIE2,      0,  0 },  // S_SPOS_DIE1
    { SPR_SPOS,  8,                   3,   A_Scream,         S_SPOS_DIE3,      0,  0 },  // S_SPOS_DIE2
    { SPR_SPOS,  9,                   2,   A_Fall,           S_SPOS_DIE4,      0,  0 },  // S_SPOS_DIE3
    { SPR_SPOS,  10,                  3,   nullptr,          S_SPOS_DIE5,      0,  0 },  // S_SPOS_DIE4
    { SPR_SPOS,  11,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_SPOS_DIE5
    { SPR_SPOS,  12,                  2,   nullptr,          S_SPOS_XDIE2,     0,  0 },  // S_SPOS_XDIE1
    { SPR_SPOS,  13,                  3,   A_XScream,        S_SPOS_XDIE3,     0,  0 },  // S_SPOS_XDIE2
    { SPR_SPOS,  14,                  2,   A_Fall,           S_SPOS_XDIE4,     0,  0 },  // S_SPOS_XDIE3
    { SPR_SPOS,  15,                  3,   nullptr,          S_SPOS_XDIE5,     0,  0 },  // S_SPOS_XDIE4
    { SPR_SPOS,  16,                  2,   nullptr,          S_SPOS_XDIE6,     0,  0 },  // S_SPOS_XDIE5
    { SPR_SPOS,  17,                  3,   nullptr,          S_SPOS_XDIE7,     0,  0 },  // S_SPOS_XDIE6
    { SPR_SPOS,  18,                  2,   nullptr,          S_SPOS_XDIE8,     0,  0 },  // S_SPOS_XDIE7
    { SPR_SPOS,  19,                  3,   nullptr,          S_SPOS_XDIE9,     0,  0 },  // S_SPOS_XDIE8
    { SPR_SPOS,  20,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_SPOS_XDIE9
    { SPR_PUFF,  1,                   2,   nullptr,          S_SMOKE2,         0,  0 },  // S_SMOKE1
    { SPR_PUFF,  2,                   2,   nullptr,          S_SMOKE3,         0,  0 },  // S_SMOKE2
    { SPR_PUFF,  1,                   2,   nullptr,          S_SMOKE4,         0,  0 },  // S_SMOKE3
    { SPR_PUFF,  2,                   2,   nullptr,          S_SMOKE5,         0,  0 },  // S_SMOKE4
    { SPR_PUFF,  3,                   2,   nullptr,          S_NULL,           0,  0 },  // S_SMOKE5
    { SPR_FATB,  0 | FF_FULLBRIGHT,   1,   A_Tracer,         S_TRACER2,        0,  0 },  // S_TRACER
    { SPR_FATB,  1 | FF_FULLBRIGHT,   1,   A_Tracer,         S_TRACER,         0,  0 },  // S_TRACER2
    { SPR_FBXP,  0 | FF_FULLBRIGHT,   4,   nullptr,          S_TRACEEXP2,      0,  0 },  // S_TRACEEXP1
    { SPR_FBXP,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_TRACEEXP3,      0,  0 },  // S_TRACEEXP2
    { SPR_FBXP,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_NULL,           0,  0 },  // S_TRACEEXP3
    { SPR_SKEL,  0,                   5,   A_Look,           S_SKEL_STND2,     0,  0 },  // S_SKEL_STND
    { SPR_SKEL,  1,                   5,   A_Look,           S_SKEL_STND,      0,  0 },  // S_SKEL_STND2
    { SPR_SKEL,  0,                   2,   A_Chase,          S_SKEL_RUN2,      0,  0 },  // S_SKEL_RUN1
    { SPR_SKEL,  0,                   2,   A_Chase,          S_SKEL_RUN3,      0,  0 },  // S_SKEL_RUN2
    { SPR_SKEL,  1,                   2,   A_Chase,          S_SKEL_RUN4,      0,  0 },  // S_SKEL_RUN3
    { SPR_SKEL,  1,                   2,   A_Chase,          S_SKEL_RUN5,      0,  0 },  // S_SKEL_RUN4
    { SPR_SKEL,  2,                   2,   A_Chase,          S_SKEL_RUN6,      0,  0 },  // S_SKEL_RUN5
    { SPR_SKEL,  2,                   2,   A_Chase,          S_SKEL_RUN7,      0,  0 },  // S_SKEL_RUN6
    { SPR_SKEL,  3,                   2,   A_Chase,          S_SKEL_RUN8,      0,  0 },  // S_SKEL_RUN7
    { SPR_SKEL,  3,                   2,   A_Chase,          S_SKEL_RUN9,      0,  0 },  // S_SKEL_RUN8
    { SPR_SKEL,  4,                   2,   A_Chase,          S_SKEL_RUN10,     0,  0 },  // S_SKEL_RUN9
    { SPR_SKEL,  4,                   2,   A_Chase,          S_SKEL_RUN11,     0,  0 },  // S_SKEL_RUN10
    { SPR_SKEL,  5,                   2,   A_Chase,          S_SKEL_RUN12,     0,  0 },  // S_SKEL_RUN11
    { SPR_SKEL,  5,                   2,   A_Chase,          S_SKEL_RUN1,      0,  0 },  // S_SKEL_RUN12
    { SPR_SKEL,  6,                   0,   A_FaceTarget,     S_SKEL_FIST2,     0,  0 },  // S_SKEL_FIST1
    { SPR_SKEL,  6,                   3,   A_SkelWhoosh,     S_SKEL_FIST3,     0,  0 },  // S_SKEL_FIST2
    { SPR_SKEL,  7,                   3,   A_FaceTarget,     S_SKEL_FIST4,     0,  0 },  // S_SKEL_FIST3
    { SPR_SKEL,  8,                   3,   A_SkelFist,       S_SKEL_RUN1,      0,  0 },  // S_SKEL_FIST4
    { SPR_SKEL,  9 | FF_FULLBRIGHT,   0,   A_FaceTarget,     S_SKEL_MISS2,     0,  0 },  // S_SKEL_MISS1
    { SPR_SKEL,  9 | FF_FULLBRIGHT,   5,   A_FaceTarget,     S_SKEL_MISS3,     0,  0 },  // S_SKEL_MISS2
    { SPR_SKEL,  10,                  5,   A_SkelMissile,    S_SKEL_MISS4,     0,  0 },  // S_SKEL_MISS3
    { SPR_SKEL,  10,                  5,   A_FaceTarget,     S_SKEL_RUN1,      0,  0 },  // S_SKEL_MISS4
    { SPR_SKEL,  11,                  2,   nullptr,          S_SKEL_PAIN2,     0,  0 },  // S_SKEL_PAIN
    { SPR_SKEL,  11,                  2,   A_Pain,           S_SKEL_RUN1,      0,  0 },  // S_SKEL_PAIN2
    { SPR_SKEL,  11,                  3,   nullptr,          S_SKEL_DIE2,      0,  0 },  // S_SKEL_DIE1
    { SPR_SKEL,  12,                  3,   nullptr,          S_SKEL_DIE3,      0,  0 },  // S_SKEL_DIE2
    { SPR_SKEL,  13,                  3,   A_Scream,         S_SKEL_DIE4,      0,  0 },  // S_SKEL_DIE3
    { SPR_SKEL,  14,                  3,   A_Fall,           S_SKEL_DIE5,      0,  0 },  // S_SKEL_DIE4
    { SPR_SKEL,  15,                  3,   nullptr,          S_SKEL_DIE6,      0,  0 },  // S_SKEL_DIE5
    { SPR_SKEL,  16,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_SKEL_DIE6
    { SPR_MANF,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_FATSHOT2,       0,  0 },  // S_FATSHOT1
    { SPR_MANF,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_FATSHOT1,       0,  0 },  // S_FATSHOT2
    { SPR_MISL,  1 | FF_FULLBRIGHT,   4,   nullptr,          S_FATSHOTX2,      0,  0 },  // S_FATSHOTX1
    { SPR_MISL,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_FATSHOTX3,      0,  0 },  // S_FATSHOTX2
    { SPR_MISL,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_NULL,           0,  0 },  // S_FATSHOTX3
    { SPR_FATT,  0,                   7,   A_Look,           S_FATT_STND2,     0,  0 },  // S_FATT_STND
    { SPR_FATT,  1,                   7,   A_Look,           S_FATT_STND,      0,  0 },  // S_FATT_STND2
    { SPR_FATT,  0,                   2,   A_Chase,          S_FATT_RUN2,      0,  0 },  // S_FATT_RUN1
    { SPR_FATT,  0,                   2,   A_Chase,          S_FATT_RUN3,      0,  0 },  // S_FATT_RUN2
    { SPR_FATT,  1,                   2,   A_Chase,          S_FATT_RUN4,      0,  0 },  // S_FATT_RUN3
    { SPR_FATT,  1,                   2,   A_Chase,          S_FATT_RUN5,      0,  0 },  // S_FATT_RUN4
    { SPR_FATT,  2,                   2,   A_Chase,          S_FATT_RUN6,      0,  0 },  // S_FATT_RUN5
    { SPR_FATT,  2,                   2,   A_Chase,          S_FATT_RUN7,      0,  0 },  // S_FATT_RUN6
    { SPR_FATT,  3,                   2,   A_Chase,          S_FATT_RUN8,      0,  0 },  // S_FATT_RUN7
    { SPR_FATT,  3,                   2,   A_Chase,          S_FATT_RUN9,      0,  0 },  // S_FATT_RUN8
    { SPR_FATT,  4,                   2,   A_Chase,          S_FATT_RUN10,     0,  0 },  // S_FATT_RUN9
    { SPR_FATT,  4,                   2,   A_Chase,          S_FATT_RUN11,     0,  0 },  // S_FATT_RUN10
    { SPR_FATT,  5,                   2,   A_Chase,          S_FATT_RUN12,     0,  0 },  // S_FATT_RUN11
    { SPR_FATT,  5,                   2,   A_Chase,          S_FATT_RUN1,      0,  0 },  // S_FATT_RUN12
    { SPR_FATT,  6,                   10,  A_FatRaise,       S_FATT_ATK2,      0,  0 },  // S_FATT_ATK1
    { SPR_FATT,  7 | FF_FULLBRIGHT,   5,   A_FatAttack1,     S_FATT_ATK3,      0,  0 },  // S_FATT_ATK2
    { SPR_FATT,  8,                   2,   A_FaceTarget,     S_FATT_ATK4,      0,  0 },  // S_FATT_ATK3
    { SPR_FATT,  6,                   2,   A_FaceTarget,     S_FATT_ATK5,      0,  0 },  // S_FATT_ATK4
    { SPR_FATT,  7 | FF_FULLBRIGHT,   5,   A_FatAttack2,     S_FATT_ATK6,      0,  0 },  // S_FATT_ATK5
    { SPR_FATT,  8,                   2,   A_FaceTarget,     S_FATT_ATK7,      0,  0 },  // S_FATT_ATK6
    { SPR_FATT,  6,                   2,   A_FaceTarget,     S_FATT_ATK8,      0,  0 },  // S_FATT_ATK7
    { SPR_FATT,  7 | FF_FULLBRIGHT,   5,   A_FatAttack3,     S_FATT_ATK9,      0,  0 },  // S_FATT_ATK8
    { SPR_FATT,  8,                   2,   A_FaceTarget,     S_FATT_ATK10,     0,  0 },  // S_FATT_ATK9
    { SPR_FATT,  6,                   2,   A_FaceTarget,     S_FATT_RUN1,      0,  0 },  // S_FATT_ATK10
    { SPR_FATT,  9,                   1,   nullptr,          S_FATT_PAIN2,     0,  0 },  // S_FATT_PAIN
    { SPR_FATT,  9,                   1,   A_Pain,           S_FATT_RUN1,      0,  0 },  // S_FATT_PAIN2
    { SPR_FATT,  10,                  2,   nullptr,          S_FATT_DIE2,      0,  0 },  // S_FATT_DIE1
    { SPR_FATT,  11,                  2,   A_Scream,         S_FATT_DIE3,      0,  0 },  // S_FATT_DIE2
    { SPR_FATT,  12,                  2,   A_Fall,           S_FATT_DIE4,      0,  0 },  // S_FATT_DIE3
    { SPR_FATT,  13,                  2,   nullptr,          S_FATT_DIE5,      0,  0 },  // S_FATT_DIE4
    { SPR_FATT,  14,                  2,   nullptr,          S_FATT_DIE6,      0,  0 },  // S_FATT_DIE5
    { SPR_FATT,  15,                  2,   nullptr,          S_FATT_DIE7,      0,  0 },  // S_FATT_DIE6
    { SPR_FATT,  16,                  2,   nullptr,          S_FATT_DIE8,      0,  0 },  // S_FATT_DIE7
    { SPR_FATT,  17,                  2,   nullptr,          S_FATT_DIE9,      0,  0 },  // S_FATT_DIE8
    { SPR_FATT,  18,                  2,   nullptr,          S_FATT_DIE10,     0,  0 },  // S_FATT_DIE9
    { SPR_FATT,  19,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_FATT_DIE10
    { SPR_CPOS,  0,                   5,   A_Look,           S_CPOS_STND2,     0,  0 },  // S_CPOS_STND
    { SPR_CPOS,  1,                   5,   A_Look,           S_CPOS_STND,      0,  0 },  // S_CPOS_STND2
    { SPR_CPOS,  0,                   2,   A_Chase,          S_CPOS_RUN2,      0,  0 },  // S_CPOS_RUN1
    { SPR_CPOS,  0,                   2,   A_Chase,          S_CPOS_RUN3,      0,  0 },  // S_CPOS_RUN2
    { SPR_CPOS,  1,                   2,   A_Chase,          S_CPOS_RUN4,      0,  0 },  // S_CPOS_RUN3
    { SPR_CPOS,  1,                   2,   A_Chase,          S_CPOS_RUN5,      0,  0 },  // S_CPOS_RUN4
    { SPR_CPOS,  2,                   2,   A_Chase,          S_CPOS_RUN6,      0,  0 },  // S_CPOS_RUN5
    { SPR_CPOS,  2,                   2,   A_Chase,          S_CPOS_RUN7,      0,  0 },  // S_CPOS_RUN6
    { SPR_CPOS,  3,                   2,   A_Chase,          S_CPOS_RUN8,      0,  0 },  // S_CPOS_RUN7
    { SPR_CPOS,  3,                   2,   A_Chase,          S_CPOS_RUN1,      0,  0 },  // S_CPOS_RUN8
    { SPR_CPOS,  4,                   5,   A_FaceTarget,     S_CPOS_ATK2,      0,  0 },  // S_CPOS_ATK1
    { SPR_CPOS,  5 | FF_FULLBRIGHT,   2,   A_CPosAttack,     S_CPOS_ATK3,      0,  0 },  // S_CPOS_ATK2
    { SPR_CPOS,  4 | FF_FULLBRIGHT,   2,   A_CPosAttack,     S_CPOS_ATK4,      0,  0 },  // S_CPOS_ATK3
    { SPR_CPOS,  5,                   0,   A_CPosRefire,     S_CPOS_ATK2,      0,  0 },  // S_CPOS_ATK4
    { SPR_CPOS,  6,                   1,   nullptr,          S_CPOS_PAIN2,     0,  0 },  // S_CPOS_PAIN
    { SPR_CPOS,  6,                   1,   A_Pain,           S_CPOS_RUN1,      0,  0 },  // S_CPOS_PAIN2
    { SPR_CPOS,  7,                   2,   nullptr,          S_CPOS_DIE2,      0,  0 },  // S_CPOS_DIE1
    { SPR_CPOS,  8,                   2,   A_Scream,         S_CPOS_DIE3,      0,  0 },  // S_CPOS_DIE2
    { SPR_CPOS,  9,                   2,   A_Fall,           S_CPOS_DIE4,      0,  0 },  // S_CPOS_DIE3
    { SPR_CPOS,  10,                  2,   nullptr,          S_CPOS_DIE5,      0,  0 },  // S_CPOS_DIE4
    { SPR_CPOS,  11,                  2,   nullptr,          S_CPOS_DIE6,      0,  0 },  // S_CPOS_DIE5
    { SPR_CPOS,  12,                  2,   nullptr,          S_CPOS_DIE7,      0,  0 },  // S_CPOS_DIE6
    { SPR_CPOS,  13,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_CPOS_DIE7
    { SPR_CPOS,  14,                  2,   nullptr,          S_CPOS_XDIE2,     0,  0 },  // S_CPOS_XDIE1
    { SPR_CPOS,  15,                  2,   A_XScream,        S_CPOS_XDIE3,     0,  0 },  // S_CPOS_XDIE2
    { SPR_CPOS,  16,                  2,   A_Fall,           S_CPOS_XDIE4,     0,  0 },  // S_CPOS_XDIE3
    { SPR_CPOS,  17,                  2,   nullptr,          S_CPOS_XDIE5,     0,  0 },  // S_CPOS_XDIE4
    { SPR_CPOS,  18,                  2,   nullptr,          S_CPOS_XDIE6,     0,  0 },  // S_CPOS_XDIE5
    { SPR_CPOS,  19,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_CPOS_XDIE6
    { SPR_TROO,  0,                   5,   A_Look,           S_TROO_STND2,     0,  0 },  // S_TROO_STND
    { SPR_TROO,  1,                   5,   A_Look,           S_TROO_STND,      0,  0 },  // S_TROO_STND2
    { SPR_TROO,  0,                   1,   A_Chase,          S_TROO_RUN2,      0,  0 },  // S_TROO_RUN1
    { SPR_TROO,  0,                   2,   A_Chase,          S_TROO_RUN3,      0,  0 },  // S_TROO_RUN2
    { SPR_TROO,  1,                   1,   A_Chase,          S_TROO_RUN4,      0,  0 },  // S_TROO_RUN3
    { SPR_TROO,  1,                   2,   A_Chase,          S_TROO_RUN5,      0,  0 },  // S_TROO_RUN4
    { SPR_TROO,  2,                   1,   A_Chase,          S_TROO_RUN6,      0,  0 },  // S_TROO_RUN5
    { SPR_TROO,  2,                   2,   A_Chase,          S_TROO_RUN7,      0,  0 },  // S_TROO_RUN6
    { SPR_TROO,  3,                   1,   A_Chase,          S_TROO_RUN8,      0,  0 },  // S_TROO_RUN7
    { SPR_TROO,  3,                   2,   A_Chase,          S_TROO_RUN1,      0,  0 },  // S_TROO_RUN8
    { SPR_TROO,  4,                   4,   A_FaceTarget,     S_TROO_ATK2,      0,  0 },  // S_TROO_ATK1
    { SPR_TROO,  5,                   4,   A_FaceTarget,     S_TROO_ATK3,      0,  0 },  // S_TROO_ATK2
    { SPR_TROO,  6,                   3,   A_TroopAttack,    S_TROO_RUN1,      0,  0 },  // S_TROO_ATK3
    { SPR_TROO,  7,                   1,   nullptr,          S_TROO_PAIN2,     0,  0 },  // S_TROO_PAIN
    { SPR_TROO,  7,                   1,   A_Pain,           S_TROO_RUN1,      0,  0 },  // S_TROO_PAIN2
    { SPR_TROO,  8,                   4,   nullptr,          S_TROO_DIE2,      0,  0 },  // S_TROO_DIE1
    { SPR_TROO,  9,                   4,   A_Scream,         S_TROO_DIE3,      0,  0 },  // S_TROO_DIE2
    { SPR_TROO,  10,                  3,   nullptr,          S_TROO_DIE4,      0,  0 },  // S_TROO_DIE3
    { SPR_TROO,  11,                  3,   A_Fall,           S_TROO_DIE5,      0,  0 },  // S_TROO_DIE4
    { SPR_TROO,  12,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_TROO_DIE5
    { SPR_TROO,  13,                  2,   nullptr,          S_TROO_XDIE2,     0,  0 },  // S_TROO_XDIE1
    { SPR_TROO,  14,                  3,   A_XScream,        S_TROO_XDIE3,     0,  0 },  // S_TROO_XDIE2
    { SPR_TROO,  15,                  2,   nullptr,          S_TROO_XDIE4,     0,  0 },  // S_TROO_XDIE3
    { SPR_TROO,  16,                  3,   A_Fall,           S_TROO_XDIE5,     0,  0 },  // S_TROO_XDIE4
    { SPR_TROO,  17,                  2,   nullptr,          S_TROO_XDIE6,     0,  0 },  // S_TROO_XDIE5
    { SPR_TROO,  18,                  3,   nullptr,          S_TROO_XDIE7,     0,  0 },  // S_TROO_XDIE6
    { SPR_TROO,  19,                  2,   nullptr,          S_TROO_XDIE8,     0,  0 },  // S_TROO_XDIE7
    { SPR_TROO,  20,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_TROO_XDIE8
    { SPR_SARG,  0,                   5,   A_Look,           S_SARG_STND2,     0,  0 },  // S_SARG_STND
    { SPR_SARG,  1,                   5,   A_Look,           S_SARG_STND,      0,  0 },  // S_SARG_STND2
    { SPR_SARG,  0,                   1,   A_Chase,          S_SARG_RUN2,      0,  0 },  // S_SARG_RUN1
    { SPR_SARG,  0,                   1,   A_Chase,          S_SARG_RUN3,      0,  0 },  // S_SARG_RUN2
    { SPR_SARG,  1,                   1,   A_Chase,          S_SARG_RUN4,      0,  0 },  // S_SARG_RUN3
    { SPR_SARG,  1,                   1,   A_Chase,          S_SARG_RUN5,      0,  0 },  // S_SARG_RUN4
    { SPR_SARG,  2,                   1,   A_Chase,          S_SARG_RUN6,      0,  0 },  // S_SARG_RUN5
    { SPR_SARG,  2,                   1,   A_Chase,          S_SARG_RUN7,      0,  0 },  // S_SARG_RUN6
    { SPR_SARG,  3,                   1,   A_Chase,          S_SARG_RUN8,      0,  0 },  // S_SARG_RUN7
    { SPR_SARG,  3,                   1,   A_Chase,          S_SARG_RUN1,      0,  0 },  // S_SARG_RUN8
    { SPR_SARG,  4,                   4,   A_FaceTarget,     S_SARG_ATK2,      0,  0 },  // S_SARG_ATK1
    { SPR_SARG,  5,                   4,   A_FaceTarget,     S_SARG_ATK3,      0,  0 },  // S_SARG_ATK2
    { SPR_SARG,  6,                   4,   A_SargAttack,     S_SARG_RUN1,      0,  0 },  // S_SARG_ATK3
    { SPR_SARG,  7,                   1,   nullptr,          S_SARG_PAIN2,     0,  0 },  // S_SARG_PAIN
    { SPR_SARG,  7,                   1,   A_Pain,           S_SARG_RUN1,      0,  0 },  // S_SARG_PAIN2
    { SPR_SARG,  8,                   4,   nullptr,          S_SARG_DIE2,      0,  0 },  // S_SARG_DIE1
    { SPR_SARG,  9,                   4,   A_Scream,         S_SARG_DIE3,      0,  0 },  // S_SARG_DIE2
    { SPR_SARG,  10,                  2,   nullptr,          S_SARG_DIE4,      0,  0 },  // S_SARG_DIE3
    { SPR_SARG,  11,                  2,   A_Fall,           S_SARG_DIE5,      0,  0 },  // S_SARG_DIE4
    { SPR_SARG,  12,                  2,   nullptr,          S_SARG_DIE6,      0,  0 },  // S_SARG_DIE5
    { SPR_SARG,  13,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_SARG_DIE6
    { SPR_HEAD,  0,                   5,   A_Look,           S_HEAD_STND,      0,  0 },  // S_HEAD_STND
    { SPR_HEAD,  0,                   2,   A_Chase,          S_HEAD_RUN1,      0,  0 },  // S_HEAD_RUN1
    { SPR_HEAD,  1,                   3,   A_FaceTarget,     S_HEAD_ATK2,      0,  0 },  // S_HEAD_ATK1
    { SPR_HEAD,  2,                   3,   A_FaceTarget,     S_HEAD_ATK3,      0,  0 },  // S_HEAD_ATK2
    { SPR_HEAD,  3 | FF_FULLBRIGHT,   3,   A_HeadAttack,     S_HEAD_RUN1,      0,  0 },  // S_HEAD_ATK3
    { SPR_HEAD,  4,                   1,   nullptr,          S_HEAD_PAIN2,     0,  0 },  // S_HEAD_PAIN
    { SPR_HEAD,  4,                   2,   A_Pain,           S_HEAD_PAIN3,     0,  0 },  // S_HEAD_PAIN2
    { SPR_HEAD,  5,                   3,   nullptr,          S_HEAD_RUN1,      0,  0 },  // S_HEAD_PAIN3
    { SPR_HEAD,  6,                   4,   nullptr,          S_HEAD_DIE2,      0,  0 },  // S_HEAD_DIE1
    { SPR_HEAD,  7,                   4,   A_Scream,         S_HEAD_DIE3,      0,  0 },  // S_HEAD_DIE2
    { SPR_HEAD,  8,                   4,   nullptr,          S_HEAD_DIE4,      0,  0 },  // S_HEAD_DIE3
    { SPR_HEAD,  9,                   4,   nullptr,          S_HEAD_DIE5,      0,  0 },  // S_HEAD_DIE4
    { SPR_HEAD,  10,                  4,   A_Fall,           S_HEAD_DIE6,      0,  0 },  // S_HEAD_DIE5
    { SPR_HEAD,  11,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_HEAD_DIE6
    { SPR_BOSS,  0,                   5,   A_Look,           S_BOSS_STND2,     0,  0 },  // S_BOSS_STND
    { SPR_BOSS,  1,                   5,   A_Look,           S_BOSS_STND,      0,  0 },  // S_BOSS_STND2
    { SPR_BOSS,  0,                   1,   A_Chase,          S_BOSS_RUN2,      0,  0 },  // S_BOSS_RUN1
    { SPR_BOSS,  0,                   2,   A_Chase,          S_BOSS_RUN3,      0,  0 },  // S_BOSS_RUN2
    { SPR_BOSS,  1,                   1,   A_Chase,          S_BOSS_RUN4,      0,  0 },  // S_BOSS_RUN3
    { SPR_BOSS,  1,                   2,   A_Chase,          S_BOSS_RUN5,      0,  0 },  // S_BOSS_RUN4
    { SPR_BOSS,  2,                   1,   A_Chase,          S_BOSS_RUN6,      0,  0 },  // S_BOSS_RUN5
    { SPR_BOSS,  2,                   2,   A_Chase,          S_BOSS_RUN7,      0,  0 },  // S_BOSS_RUN6
    { SPR_BOSS,  3,                   1,   A_Chase,          S_BOSS_RUN8,      0,  0 },  // S_BOSS_RUN7
    { SPR_BOSS,  3,                   2,   A_Chase,          S_BOSS_RUN1,      0,  0 },  // S_BOSS_RUN8
    { SPR_BOSS,  4,                   4,   A_FaceTarget,     S_BOSS_ATK2,      0,  0 },  // S_BOSS_ATK1
    { SPR_BOSS,  5,                   4,   A_FaceTarget,     S_BOSS_ATK3,      0,  0 },  // S_BOSS_ATK2
    { SPR_BOSS,  6,                   4,   A_BruisAttack,    S_BOSS_RUN1,      0,  0 },  // S_BOSS_ATK3
    { SPR_BOSS,  7,                   1,   nullptr,          S_BOSS_PAIN2,     0,  0 },  // S_BOSS_PAIN
    { SPR_BOSS,  7,                   1,   A_Pain,           S_BOSS_RUN1,      0,  0 },  // S_BOSS_PAIN2
    { SPR_BOSS,  8,                   4,   nullptr,          S_BOSS_DIE2,      0,  0 },  // S_BOSS_DIE1
    { SPR_BOSS,  9,                   4,   A_Scream,         S_BOSS_DIE3,      0,  0 },  // S_BOSS_DIE2
    { SPR_BOSS,  10,                  4,   nullptr,          S_BOSS_DIE4,      0,  0 },  // S_BOSS_DIE3
    { SPR_BOSS,  11,                  4,   A_Fall,           S_BOSS_DIE5,      0,  0 },  // S_BOSS_DIE4
    { SPR_BOSS,  12,                  4,   nullptr,          S_BOSS_DIE6,      0,  0 },  // S_BOSS_DIE5
    { SPR_BOSS,  13,                  4,   nullptr,          S_BOSS_DIE7,      0,  0 },  // S_BOSS_DIE6
    { SPR_BOSS,  14,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_BOSS_DIE7
    { SPR_BOS2,  0,                   5,   A_Look,           S_BOS2_STND2,     0,  0 },  // S_BOS2_STND
    { SPR_BOS2,  1,                   5,   A_Look,           S_BOS2_STND,      0,  0 },  // S_BOS2_STND2
    { SPR_BOS2,  0,                   2,   A_Chase,          S_BOS2_RUN2,      0,  0 },  // S_BOS2_RUN1
    { SPR_BOS2,  0,                   2,   A_Chase,          S_BOS2_RUN3,      0,  0 },  // S_BOS2_RUN2
    { SPR_BOS2,  1,                   2,   A_Chase,          S_BOS2_RUN4,      0,  0 },  // S_BOS2_RUN3
    { SPR_BOS2,  1,                   2,   A_Chase,          S_BOS2_RUN5,      0,  0 },  // S_BOS2_RUN4
    { SPR_BOS2,  2,                   2,   A_Chase,          S_BOS2_RUN6,      0,  0 },  // S_BOS2_RUN5
    { SPR_BOS2,  2,                   2,   A_Chase,          S_BOS2_RUN7,      0,  0 },  // S_BOS2_RUN6
    { SPR_BOS2,  3,                   2,   A_Chase,          S_BOS2_RUN8,      0,  0 },  // S_BOS2_RUN7
    { SPR_BOS2,  3,                   2,   A_Chase,          S_BOS2_RUN1,      0,  0 },  // S_BOS2_RUN8
    { SPR_BOS2,  4,                   4,   A_FaceTarget,     S_BOS2_ATK2,      0,  0 },  // S_BOS2_ATK1
    { SPR_BOS2,  5,                   4,   A_FaceTarget,     S_BOS2_ATK3,      0,  0 },  // S_BOS2_ATK2
    { SPR_BOS2,  6,                   4,   A_BruisAttack,    S_BOS2_RUN1,      0,  0 },  // S_BOS2_ATK3
    { SPR_BOS2,  7,                   1,   nullptr,          S_BOS2_PAIN2,     0,  0 },  // S_BOS2_PAIN
    { SPR_BOS2,  7,                   1,   A_Pain,           S_BOS2_RUN1,      0,  0 },  // S_BOS2_PAIN2
    { SPR_BOS2,  8,                   4,   nullptr,          S_BOS2_DIE2,      0,  0 },  // S_BOS2_DIE1
    { SPR_BOS2,  9,                   4,   A_Scream,         S_BOS2_DIE3,      0,  0 },  // S_BOS2_DIE2
    { SPR_BOS2,  10,                  4,   nullptr,          S_BOS2_DIE4,      0,  0 },  // S_BOS2_DIE3
    { SPR_BOS2,  11,                  4,   A_Fall,           S_BOS2_DIE5,      0,  0 },  // S_BOS2_DIE4
    { SPR_BOS2,  12,                  4,   nullptr,          S_BOS2_DIE6,      0,  0 },  // S_BOS2_DIE5
    { SPR_BOS2,  13,                  4,   nullptr,          S_BOS2_DIE7,      0,  0 },  // S_BOS2_DIE6
    { SPR_BOS2,  14,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_BOS2_DIE7
    { SPR_SKUL,  0 | FF_FULLBRIGHT,   5,   A_Look,           S_SKULL_STND2,    0,  0 },  // S_SKULL_STND
    { SPR_SKUL,  1 | FF_FULLBRIGHT,   5,   A_Look,           S_SKULL_STND,     0,  0 },  // S_SKULL_STND2
    { SPR_SKUL,  0 | FF_FULLBRIGHT,   3,   A_Chase,          S_SKULL_RUN2,     0,  0 },  // S_SKULL_RUN1
    { SPR_SKUL,  1 | FF_FULLBRIGHT,   3,   A_Chase,          S_SKULL_RUN1,     0,  0 },  // S_SKULL_RUN2
    { SPR_SKUL,  2 | FF_FULLBRIGHT,   5,   A_FaceTarget,     S_SKULL_ATK2,     0,  0 },  // S_SKULL_ATK1
    { SPR_SKUL,  3 | FF_FULLBRIGHT,   2,   A_SkullAttack,    S_SKULL_ATK3,     0,  0 },  // S_SKULL_ATK2
    { SPR_SKUL,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_SKULL_ATK4,     0,  0 },  // S_SKULL_ATK3
    { SPR_SKUL,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_SKULL_ATK3,     0,  0 },  // S_SKULL_ATK4
    { SPR_SKUL,  4 | FF_FULLBRIGHT,   1,   nullptr,          S_SKULL_PAIN2,    0,  0 },  // S_SKULL_PAIN
    { SPR_SKUL,  4 | FF_FULLBRIGHT,   2,   A_Pain,           S_SKULL_RUN1,     0,  0 },  // S_SKULL_PAIN2
    { SPR_SKUL,  5 | FF_FULLBRIGHT,   3,   nullptr,          S_SKULL_DIE2,     0,  0 },  // S_SKULL_DIE1
    { SPR_SKUL,  6 | FF_FULLBRIGHT,   3,   A_Scream,         S_SKULL_DIE3,     0,  0 },  // S_SKULL_DIE2
    { SPR_SKUL,  7 | FF_FULLBRIGHT,   3,   nullptr,          S_SKULL_DIE4,     0,  0 },  // S_SKULL_DIE3
    { SPR_SKUL,  8 | FF_FULLBRIGHT,   3,   A_Fall,           S_SKULL_DIE5,     0,  0 },  // S_SKULL_DIE4
    { SPR_SKUL,  9,                   3,   nullptr,          S_SKULL_DIE6,     0,  0 },  // S_SKULL_DIE5
    { SPR_SKUL,  10,                  3,   nullptr,          S_NULL,           0,  0 },  // S_SKULL_DIE6
    { SPR_SPID,  0,                   5,   A_Look,           S_SPID_STND2,     0,  0 },  // S_SPID_STND
    { SPR_SPID,  1,                   5,   A_Look,           S_SPID_STND,      0,  0 },  // S_SPID_STND2
    { SPR_SPID,  0,                   2,   A_Metal,          S_SPID_RUN2,      0,  0 },  // S_SPID_RUN1
    { SPR_SPID,  0,                   2,   A_Chase,          S_SPID_RUN3,      0,  0 },  // S_SPID_RUN2
    { SPR_SPID,  1,                   2,   A_Chase,          S_SPID_RUN4,      0,  0 },  // S_SPID_RUN3
    { SPR_SPID,  1,                   2,   A_Chase,          S_SPID_RUN5,      0,  0 },  // S_SPID_RUN4
    { SPR_SPID,  2,                   2,   A_Metal,          S_SPID_RUN6,      0,  0 },  // S_SPID_RUN5
    { SPR_SPID,  2,                   2,   A_Chase,          S_SPID_RUN7,      0,  0 },  // S_SPID_RUN6
    { SPR_SPID,  3,                   2,   A_Chase,          S_SPID_RUN8,      0,  0 },  // S_SPID_RUN7
    { SPR_SPID,  3,                   2,   A_Chase,          S_SPID_RUN9,      0,  0 },  // S_SPID_RUN8
    { SPR_SPID,  4,                   2,   A_Metal,          S_SPID_RUN10,     0,  0 },  // S_SPID_RUN9
    { SPR_SPID,  4,                   2,   A_Chase,          S_SPID_RUN11,     0,  0 },  // S_SPID_RUN10
    { SPR_SPID,  5,                   2,   A_Chase,          S_SPID_RUN12,     0,  0 },  // S_SPID_RUN11
    { SPR_SPID,  5,                   2,   A_Chase,          S_SPID_RUN1,      0,  0 },  // S_SPID_RUN12
    { SPR_SPID,  0 | FF_FULLBRIGHT,   10,  A_FaceTarget,     S_SPID_ATK2,      0,  0 },  // S_SPID_ATK1
    { SPR_SPID,  6 | FF_FULLBRIGHT,   2,   A_SpidAttack,     S_SPID_ATK3,      0,  0 },  // S_SPID_ATK2
    { SPR_SPID,  7 | FF_FULLBRIGHT,   2,   A_SpidAttack,     S_SPID_ATK4,      0,  0 },  // S_SPID_ATK3
    { SPR_SPID,  7 | FF_FULLBRIGHT,   1,   A_SpidRefire,     S_SPID_ATK2,      0,  0 },  // S_SPID_ATK4
    { SPR_SPID,  8,                   2,   nullptr,          S_SPID_PAIN2,     0,  0 },  // S_SPID_PAIN
    { SPR_SPID,  8,                   2,   A_Pain,           S_SPID_RUN1,      0,  0 },  // S_SPID_PAIN2
    { SPR_SPID,  9,                   8,   A_Scream,         S_SPID_DIE2,      0,  0 },  // S_SPID_DIE1
    { SPR_SPID,  10,                  3,   A_Fall,           S_SPID_DIE3,      0,  0 },  // S_SPID_DIE2
    { SPR_SPID,  11,                  3,   nullptr,          S_SPID_DIE4,      0,  0 },  // S_SPID_DIE3
    { SPR_SPID,  12,                  3,   nullptr,          S_SPID_DIE5,      0,  0 },  // S_SPID_DIE4
    { SPR_SPID,  13,                  3,   nullptr,          S_SPID_DIE6,      0,  0 },  // S_SPID_DIE5
    { SPR_SPID,  14,                  3,   nullptr,          S_SPID_DIE7,      0,  0 },  // S_SPID_DIE6
    { SPR_SPID,  15,                  3,   nullptr,          S_SPID_DIE8,      0,  0 },  // S_SPID_DIE7
    { SPR_SPID,  16,                  3,   nullptr,          S_SPID_DIE9,      0,  0 },  // S_SPID_DIE8
    { SPR_SPID,  17,                  3,   nullptr,          S_SPID_DIE10,     0,  0 },  // S_SPID_DIE9
    { SPR_SPID,  18,                  10,  nullptr,          S_SPID_DIE11,     0,  0 },  // S_SPID_DIE10
    { SPR_SPID,  18,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_SPID_DIE11
    { SPR_BSPI,  0,                   5,   A_Look,           S_BSPI_STND2,     0,  0 },  // S_BSPI_STND
    { SPR_BSPI,  1,                   5,   A_Look,           S_BSPI_STND,      0,  0 },  // S_BSPI_STND2
    { SPR_BSPI,  0,                   10,  nullptr,          S_BSPI_RUN1,      0,  0 },  // S_BSPI_SIGHT
    { SPR_BSPI,  0,                   2,   A_BabyMetal,      S_BSPI_RUN2,      0,  0 },  // S_BSPI_RUN1
    { SPR_BSPI,  0,                   2,   A_Chase,          S_BSPI_RUN3,      0,  0 },  // S_BSPI_RUN2
    { SPR_BSPI,  1,                   2,   A_Chase,          S_BSPI_RUN4,      0,  0 },  // S_BSPI_RUN3
    { SPR_BSPI,  1,                   2,   A_Chase,          S_BSPI_RUN5,      0,  0 },  // S_BSPI_RUN4
    { SPR_BSPI,  2,                   2,   A_Chase,          S_BSPI_RUN6,      0,  0 },  // S_BSPI_RUN5
    { SPR_BSPI,  2,                   2,   A_Chase,          S_BSPI_RUN7,      0,  0 },  // S_BSPI_RUN6
    { SPR_BSPI,  3,                   2,   A_BabyMetal,      S_BSPI_RUN8,      0,  0 },  // S_BSPI_RUN7
    { SPR_BSPI,  3,                   2,   A_Chase,          S_BSPI_RUN9,      0,  0 },  // S_BSPI_RUN8
    { SPR_BSPI,  4,                   2,   A_Chase,          S_BSPI_RUN10,     0,  0 },  // S_BSPI_RUN9
    { SPR_BSPI,  4,                   2,   A_Chase,          S_BSPI_RUN11,     0,  0 },  // S_BSPI_RUN10
    { SPR_BSPI,  5,                   2,   A_Chase,          S_BSPI_RUN12,     0,  0 },  // S_BSPI_RUN11
    { SPR_BSPI,  5,                   2,   A_Chase,          S_BSPI_RUN1,      0,  0 },  // S_BSPI_RUN12
    { SPR_BSPI,  0 | FF_FULLBRIGHT,   10,  A_FaceTarget,     S_BSPI_ATK2,      0,  0 },  // S_BSPI_ATK1
    { SPR_BSPI,  6 | FF_FULLBRIGHT,   2,   A_BspiAttack,     S_BSPI_ATK3,      0,  0 },  // S_BSPI_ATK2
    { SPR_BSPI,  7 | FF_FULLBRIGHT,   2,   nullptr,          S_BSPI_ATK4,      0,  0 },  // S_BSPI_ATK3
    { SPR_BSPI,  7 | FF_FULLBRIGHT,   1,   A_SpidRefire,     S_BSPI_ATK2,      0,  0 },  // S_BSPI_ATK4
    { SPR_BSPI,  8,                   2,   nullptr,          S_BSPI_PAIN2,     0,  0 },  // S_BSPI_PAIN
    { SPR_BSPI,  8,                   2,   A_Pain,           S_BSPI_RUN1,      0,  0 },  // S_BSPI_PAIN2
    { SPR_BSPI,  9,                   10,  A_Scream,         S_BSPI_DIE2,      0,  0 },  // S_BSPI_DIE1
    { SPR_BSPI,  10,                  3,   A_Fall,           S_BSPI_DIE3,      0,  0 },  // S_BSPI_DIE2
    { SPR_BSPI,  11,                  3,   nullptr,          S_BSPI_DIE4,      0,  0 },  // S_BSPI_DIE3
    { SPR_BSPI,  12,                  3,   nullptr,          S_BSPI_DIE5,      0,  0 },  // S_BSPI_DIE4
    { SPR_BSPI,  13,                  3,   nullptr,          S_BSPI_DIE6,      0,  0 },  // S_BSPI_DIE5
    { SPR_BSPI,  14,                  3,   nullptr,          S_BSPI_DIE7,      0,  0 },  // S_BSPI_DIE6
    { SPR_BSPI,  15,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_BSPI_DIE7
    { SPR_APLS,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLAZ2,    0,  0 },  // S_ARACH_PLAZ
    { SPR_APLS,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLAZ,     0,  0 },  // S_ARACH_PLAZ2
    { SPR_APBX,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLEX2,    0,  0 },  // S_ARACH_PLEX
    { SPR_APBX,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLEX3,    0,  0 },  // S_ARACH_PLEX2
    { SPR_APBX,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLEX4,    0,  0 },  // S_ARACH_PLEX3
    { SPR_APBX,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_ARACH_PLEX5,    0,  0 },  // S_ARACH_PLEX4
    { SPR_APBX,  4 | FF_FULLBRIGHT,   2,   nullptr,          S_NULL,           0,  0 },  // S_ARACH_PLEX5
    { SPR_CYBR,  0,                   5,   A_Look,           S_CYBER_STND2,    0,  0 },  // S_CYBER_STND
    { SPR_CYBR,  1,                   5,   A_Look,           S_CYBER_STND,     0,  0 },  // S_CYBER_STND2
    { SPR_CYBR,  0,                   2,   A_Hoof,           S_CYBER_RUN2,     0,  0 },  // S_CYBER_RUN1
    { SPR_CYBR,  0,                   2,   A_Chase,          S_CYBER_RUN3,     0,  0 },  // S_CYBER_RUN2
    { SPR_CYBR,  1,                   2,   A_Chase,          S_CYBER_RUN4,     0,  0 },  // S_CYBER_RUN3
    { SPR_CYBR,  1,                   2,   A_Chase,          S_CYBER_RUN5,     0,  0 },  // S_CYBER_RUN4
    { SPR_CYBR,  2,                   2,   A_Chase,          S_CYBER_RUN6,     0,  0 },  // S_CYBER_RUN5
    { SPR_CYBR,  2,                   2,   A_Chase,          S_CYBER_RUN7,     0,  0 },  // S_CYBER_RUN6
    { SPR_CYBR,  3,                   2,   A_Metal,          S_CYBER_RUN8,     0,  0 },  // S_CYBER_RUN7
    { SPR_CYBR,  3,                   2,   A_Chase,          S_CYBER_RUN1,     0,  0 },  // S_CYBER_RUN8
    { SPR_CYBR,  4,                   3,   A_FaceTarget,     S_CYBER_ATK2,     0,  0 },  // S_CYBER_ATK1
    { SPR_CYBR,  5,                   6,   A_CyberAttack,    S_CYBER_ATK3,     0,  0 },  // S_CYBER_ATK2
    { SPR_CYBR,  4,                   6,   A_FaceTarget,     S_CYBER_ATK4,     0,  0 },  // S_CYBER_ATK3
    { SPR_CYBR,  5,                   6,   A_CyberAttack,    S_CYBER_ATK5,     0,  0 },  // S_CYBER_ATK4
    { SPR_CYBR,  4,                   6,   A_FaceTarget,     S_CYBER_ATK6,     0,  0 },  // S_CYBER_ATK5
    { SPR_CYBR,  5,                   6,   A_CyberAttack,    S_CYBER_RUN1,     0,  0 },  // S_CYBER_ATK6
    { SPR_CYBR,  6,                   5,   A_Pain,           S_CYBER_RUN1,     0,  0 },  // S_CYBER_PAIN
    { SPR_CYBR,  7,                   5,   nullptr,          S_CYBER_DIE2,     0,  0 },  // S_CYBER_DIE1
    { SPR_CYBR,  8,                   5,   A_Scream,         S_CYBER_DIE3,     0,  0 },  // S_CYBER_DIE2
    { SPR_CYBR,  9,                   5,   nullptr,          S_CYBER_DIE4,     0,  0 },  // S_CYBER_DIE3
    { SPR_CYBR,  10,                  5,   nullptr,          S_CYBER_DIE5,     0,  0 },  // S_CYBER_DIE4
    { SPR_CYBR,  11,                  5,   nullptr,          S_CYBER_DIE6,     0,  0 },  // S_CYBER_DIE5
    { SPR_CYBR,  12,                  5,   A_Fall,           S_CYBER_DIE7,     0,  0 },  // S_CYBER_DIE6
    { SPR_CYBR,  13,                  5,   nullptr,          S_CYBER_DIE8,     0,  0 },  // S_CYBER_DIE7
    { SPR_CYBR,  14,                  5,   nullptr,          S_CYBER_DIE9,     0,  0 },  // S_CYBER_DIE8
    { SPR_CYBR,  15,                  15,  nullptr,          S_CYBER_DIE10,    0,  0 },  // S_CYBER_DIE9
    { SPR_CYBR,  15,                 -1,   A_BossDeath,      S_NULL,           0,  0 },  // S_CYBER_DIE10
    { SPR_PAIN,  0,                   5,   A_Look,           S_PAIN_STND,      0,  0 },  // S_PAIN_STND
    { SPR_PAIN,  0,                   2,   A_Chase,          S_PAIN_RUN2,      0,  0 },  // S_PAIN_RUN1
    { SPR_PAIN,  0,                   2,   A_Chase,          S_PAIN_RUN3,      0,  0 },  // S_PAIN_RUN2
    { SPR_PAIN,  1,                   2,   A_Chase,          S_PAIN_RUN4,      0,  0 },  // S_PAIN_RUN3
    { SPR_PAIN,  1,                   2,   A_Chase,          S_PAIN_RUN5,      0,  0 },  // S_PAIN_RUN4
    { SPR_PAIN,  2,                   2,   A_Chase,          S_PAIN_RUN6,      0,  0 },  // S_PAIN_RUN5
    { SPR_PAIN,  2,                   2,   A_Chase,          S_PAIN_RUN1,      0,  0 },  // S_PAIN_RUN6
    { SPR_PAIN,  3,                   3,   A_FaceTarget,     S_PAIN_ATK2,      0,  0 },  // S_PAIN_ATK1
    { SPR_PAIN,  4,                   3,   A_FaceTarget,     S_PAIN_ATK3,      0,  0 },  // S_PAIN_ATK2
    { SPR_PAIN,  5 | FF_FULLBRIGHT,   3,   A_FaceTarget,     S_PAIN_ATK4,      0,  0 },  // S_PAIN_ATK3
    { SPR_PAIN,  5 | FF_FULLBRIGHT,   0,   A_PainAttack,     S_PAIN_RUN1,      0,  0 },  // S_PAIN_ATK4
    { SPR_PAIN,  6,                   3,   nullptr,          S_PAIN_PAIN2,     0,  0 },  // S_PAIN_PAIN
    { SPR_PAIN,  6,                   3,   A_Pain,           S_PAIN_RUN1,      0,  0 },  // S_PAIN_PAIN2
    { SPR_PAIN,  7 | FF_FULLBRIGHT,   4,   nullptr,          S_PAIN_DIE2,      0,  0 },  // S_PAIN_DIE1
    { SPR_PAIN,  8 | FF_FULLBRIGHT,   4,   A_Scream,         S_PAIN_DIE3,      0,  0 },  // S_PAIN_DIE2
    { SPR_PAIN,  9 | FF_FULLBRIGHT,   4,   nullptr,          S_PAIN_DIE4,      0,  0 },  // S_PAIN_DIE3
    { SPR_PAIN,  10 | FF_FULLBRIGHT,  4,   nullptr,          S_PAIN_DIE5,      0,  0 },  // S_PAIN_DIE4
    { SPR_PAIN,  11 | FF_FULLBRIGHT,  4,   A_PainDie,        S_PAIN_DIE6,      0,  0 },  // S_PAIN_DIE5
    { SPR_PAIN,  12 | FF_FULLBRIGHT,  4,   nullptr,          S_NULL,           0,  0 },  // S_PAIN_DIE6
    { SPR_ARM1,  0,                   3,   nullptr,          S_ARM1A,          0,  0 },  // S_ARM1
    { SPR_ARM1,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_ARM1,           0,  0 },  // S_ARM1A
    { SPR_ARM2,  0,                   3,   nullptr,          S_ARM2A,          0,  0 },  // S_ARM2
    { SPR_ARM2,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_ARM2,           0,  0 },  // S_ARM2A
    { SPR_BAR1,  0,                   3,   nullptr,          S_BAR2,           0,  0 },  // S_BAR1
    { SPR_BAR1,  1,                   3,   nullptr,          S_BAR1,           0,  0 },  // S_BAR2
    { SPR_BEXP,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BEXP2,          0,  0 },  // S_BEXP
    { SPR_BEXP,  1 | FF_FULLBRIGHT,   3,   A_Scream,         S_BEXP3,          0,  0 },  // S_BEXP2
    { SPR_BEXP,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_BEXP4,          0,  0 },  // S_BEXP3
    { SPR_BEXP,  3 | FF_FULLBRIGHT,   5,   A_Explode,        S_BEXP5,          0,  0 },  // S_BEXP4
    { SPR_BEXP,  4 | FF_FULLBRIGHT,   5,   nullptr,          S_NULL,           0,  0 },  // S_BEXP5
    { SPR_FCAN,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BBAR2,          0,  0 },  // S_BBAR1
    { SPR_FCAN,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_BBAR3,          0,  0 },  // S_BBAR2
    { SPR_FCAN,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_BBAR1,          0,  0 },  // S_BBAR3
    { SPR_BON1,  0,                   3,   nullptr,          S_BON1A,          0,  0 },  // S_BON1
    { SPR_BON1,  1,                   3,   nullptr,          S_BON1B,          0,  0 },  // S_BON1A
    { SPR_BON1,  2,                   3,   nullptr,          S_BON1C,          0,  0 },  // S_BON1B
    { SPR_BON1,  3,                   3,   nullptr,          S_BON1D,          0,  0 },  // S_BON1C
    { SPR_BON1,  2,                   3,   nullptr,          S_BON1E,          0,  0 },  // S_BON1D
    { SPR_BON1,  1,                   3,   nullptr,          S_BON1,           0,  0 },  // S_BON1E
    { SPR_BON2,  0,                   3,   nullptr,          S_BON2A,          0,  0 },  // S_BON2
    { SPR_BON2,  1,                   3,   nullptr,          S_BON2B,          0,  0 },  // S_BON2A
    { SPR_BON2,  2,                   3,   nullptr,          S_BON2C,          0,  0 },  // S_BON2B
    { SPR_BON2,  3,                   3,   nullptr,          S_BON2D,          0,  0 },  // S_BON2C
    { SPR_BON2,  2,                   3,   nullptr,          S_BON2E,          0,  0 },  // S_BON2D
    { SPR_BON2,  1,                   3,   nullptr,          S_BON2,           0,  0 },  // S_BON2E
    { SPR_BKEY,  0,                   5,   nullptr,          S_BKEY2,          0,  0 },  // S_BKEY
    { SPR_BKEY,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_BKEY,           0,  0 },  // S_BKEY2
    { SPR_RKEY,  0,                   5,   nullptr,          S_RKEY2,          0,  0 },  // S_RKEY
    { SPR_RKEY,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_RKEY,           0,  0 },  // S_RKEY2
    { SPR_YKEY,  0,                   5,   nullptr,          S_YKEY2,          0,  0 },  // S_YKEY
    { SPR_YKEY,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_YKEY,           0,  0 },  // S_YKEY2
    { SPR_BSKU,  0,                   5,   nullptr,          S_BSKULL2,        0,  0 },  // S_BSKULL
    { SPR_BSKU,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_BSKULL,         0,  0 },  // S_BSKULL2
    { SPR_RSKU,  0,                   5,   nullptr,          S_RSKULL2,        0,  0 },  // S_RSKULL
    { SPR_RSKU,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_RSKULL,         0,  0 },  // S_RSKULL2
    { SPR_YSKU,  0,                   5,   nullptr,          S_YSKULL2,        0,  0 },  // S_YSKULL
    { SPR_YSKU,  1 | FF_FULLBRIGHT,   5,   nullptr,          S_YSKULL,         0,  0 },  // S_YSKULL2
    { SPR_STIM,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_STIM
    { SPR_MEDI,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MEDI
    { SPR_SOUL,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL2,          0,  0 },  // S_SOUL
    { SPR_SOUL,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL3,          0,  0 },  // S_SOUL2
    { SPR_SOUL,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL4,          0,  0 },  // S_SOUL3
    { SPR_SOUL,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL5,          0,  0 },  // S_SOUL4
    { SPR_SOUL,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL6,          0,  0 },  // S_SOUL5
    { SPR_SOUL,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_SOUL,           0,  0 },  // S_SOUL6
    { SPR_PINV,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_PINV2,          0,  0 },  // S_PINV
    { SPR_PINV,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_PINV3,          0,  0 },  // S_PINV2
    { SPR_PINV,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_PINV4,          0,  0 },  // S_PINV3
    { SPR_PINV,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_PINV,           0,  0 },  // S_PINV4
    { SPR_PSTR,  0 | FF_FULLBRIGHT,  -1,   nullptr,          S_NULL,           0,  0 },  // S_PSTR
    { SPR_PINS,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_PINS2,          0,  0 },  // S_PINS
    { SPR_PINS,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_PINS3,          0,  0 },  // S_PINS2
    { SPR_PINS,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_PINS4,          0,  0 },  // S_PINS3
    { SPR_PINS,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_PINS,           0,  0 },  // S_PINS4
    { SPR_MEGA,  0 | FF_FULLBRIGHT,   6,   nullptr,          S_MEGA2,          0,  0 },  // S_MEGA
    { SPR_MEGA,  1 | FF_FULLBRIGHT,   6,   nullptr,          S_MEGA3,          0,  0 },  // S_MEGA2
    { SPR_MEGA,  2 | FF_FULLBRIGHT,   6,   nullptr,          S_MEGA4,          0,  0 },  // S_MEGA3
    { SPR_MEGA,  3 | FF_FULLBRIGHT,   6,   nullptr,          S_MEGA,           0,  0 },  // S_MEGA4
    { SPR_SUIT,  0 | FF_FULLBRIGHT,  -1,   nullptr,          S_NULL,           0,  0 },  // S_SUIT
    { SPR_PMAP,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP2,          0,  0 },  // S_PMAP
    { SPR_PMAP,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP3,          0,  0 },  // S_PMAP2
    { SPR_PMAP,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP4,          0,  0 },  // S_PMAP3
    { SPR_PMAP,  3 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP5,          0,  0 },  // S_PMAP4
    { SPR_PMAP,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP6,          0,  0 },  // S_PMAP5
    { SPR_PMAP,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_PMAP,           0,  0 },  // S_PMAP6
    { SPR_PVIS,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_PVIS2,          0,  0 },  // S_PVIS
    { SPR_PVIS,  1,                   3,   nullptr,          S_PVIS,           0,  0 },  // S_PVIS2
    { SPR_CLIP,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_CLIP
    { SPR_AMMO,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_AMMO
    { SPR_ROCK,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_ROCK
    { SPR_BROK,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BROK
    { SPR_CELL,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_CELL
    { SPR_CELP,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_CELP
    { SPR_SHEL,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SHEL
    { SPR_SBOX,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SBOX
    { SPR_BPAK,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BPAK
    { SPR_BFUG,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BFUG
    { SPR_MGUN,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MGUN
    { SPR_CSAW,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_CSAW
    { SPR_LAUN,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_LAUN
    { SPR_PLAS,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_PLAS
    { SPR_SHOT,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SHOT
    { SPR_SGN2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SHOT2
    { SPR_COLU,  0 | FF_FULLBRIGHT,  -1,   nullptr,          S_NULL,           0,  0 },  // S_COLU
    { SPR_SMT2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_STALAG
    { SPR_PLAY,  13,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_DEADTORSO
    { SPR_PLAY,  18,                 -1,   nullptr,          S_NULL,           0,  0 },  // S_DEADBOTTOM
    { SPR_POL2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HEADSONSTICK
    { SPR_POL5,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_GIBS
    { SPR_POL4,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HEADONASTICK
    { SPR_POL1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEADSTICK
    { SPR_GOR2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MEAT2
    { SPR_GOR3,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MEAT3
    { SPR_GOR4,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MEAT4
    { SPR_GOR5,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_MEAT5
    { SPR_SMIT,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_STALAGTITE
    { SPR_COL1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_TALLGRNCOL
    { SPR_COL2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SHRTGRNCOL
    { SPR_COL3,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_TALLREDCOL
    { SPR_COL4,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SHRTREDCOL
    { SPR_COL6,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SKULLCOL
    { SPR_CAND,  0 | FF_FULLBRIGHT,  -1,   nullptr,          S_NULL,           0,  0 },  // S_CANDLESTIK
    { SPR_CBRA,  0 | FF_FULLBRIGHT,  -1,   nullptr,          S_NULL,           0,  0 },  // S_CANDELABRA
    { SPR_TRE1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_TORCHTREE
    { SPR_ELEC,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_TECHPILLAR
    { SPR_FSKU,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_FLOATSKULL2,    0,  0 },  // S_FLOATSKULL
    { SPR_FSKU,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_FLOATSKULL3,    0,  0 },  // S_FLOATSKULL2
    { SPR_FSKU,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_FLOATSKULL,     0,  0 },  // S_FLOATSKULL3
    { SPR_SMBT,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BTORCHSHRT2,    0,  0 },  // S_BTORCHSHRT
    { SPR_SMBT,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_BTORCHSHRT3,    0,  0 },  // S_BTORCHSHRT2
    { SPR_SMBT,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_BTORCHSHRT4,    0,  0 },  // S_BTORCHSHRT3
    { SPR_SMBT,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_BTORCHSHRT,     0,  0 },  // S_BTORCHSHRT4
    { SPR_SMGT,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_GTORCHSHRT2,    0,  0 },  // S_GTORCHSHRT
    { SPR_SMGT,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_GTORCHSHRT3,    0,  0 },  // S_GTORCHSHRT2
    { SPR_SMGT,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_GTORCHSHRT4,    0,  0 },  // S_GTORCHSHRT3
    { SPR_SMGT,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_GTORCHSHRT,     0,  0 },  // S_GTORCHSHRT4
    { SPR_SMRT,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_RTORCHSHRT2,    0,  0 },  // S_RTORCHSHRT
    { SPR_SMRT,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_RTORCHSHRT3,    0,  0 },  // S_RTORCHSHRT2
    { SPR_SMRT,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_RTORCHSHRT4,    0,  0 },  // S_RTORCHSHRT3
    { SPR_SMRT,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_RTORCHSHRT,     0,  0 },  // S_RTORCHSHRT4
    { SPR_HANC,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGCHAIN
    { SPR_BLCH,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BLOODCHAIN
    { SPR_HANL,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGLAMP
    { SPR_DED1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD1
    { SPR_DED2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD2
    { SPR_DED3,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD3
    { SPR_DED4,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD4
    { SPR_DED5,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD5
    { SPR_DED6,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_DEAD6
    { SPR_TLMP,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_TECHLAMP2,      0,  0 },  // S_TECHLAMP
    { SPR_TLMP,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_TECHLAMP3,      0,  0 },  // S_TECHLAMP2
    { SPR_TLMP,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_TECHLAMP4,      0,  0 },  // S_TECHLAMP3
    { SPR_TLMP,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_TECHLAMP,       0,  0 },  // S_TECHLAMP4
    { SPR_TLP2,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_TECH2LAMP2,     0,  0 },  // S_TECH2LAMP
    { SPR_TLP2,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_TECH2LAMP3,     0,  0 },  // S_TECH2LAMP2
    { SPR_TLP2,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_TECH2LAMP4,     0,  0 },  // S_TECH2LAMP3
    { SPR_TLP2,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_TECH2LAMP,      0,  0 },  // S_TECH2LAMP4
    { SPR_COL5,  0,                   7,   nullptr,          S_HEARTCOL2,      0,  0 },  // S_HEARTCOL
    { SPR_COL5,  1,                   7,   nullptr,          S_HEARTCOL,       0,  0 },  // S_HEARTCOL2
    { SPR_CEYE,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_EVILEYE2,       0,  0 },  // S_EVILEYE
    { SPR_CEYE,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_EVILEYE3,       0,  0 },  // S_EVILEYE2
    { SPR_CEYE,  2 | FF_FULLBRIGHT,   3,   nullptr,          S_EVILEYE4,       0,  0 },  // S_EVILEYE3
    { SPR_CEYE,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_EVILEYE,        0,  0 },  // S_EVILEYE4
    { SPR_TBLU,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_BLUETORCH2,     0,  0 },  // S_BLUETORCH
    { SPR_TBLU,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_BLUETORCH3,     0,  0 },  // S_BLUETORCH2
    { SPR_TBLU,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_BLUETORCH4,     0,  0 },  // S_BLUETORCH3
    { SPR_TBLU,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_BLUETORCH,      0,  0 },  // S_BLUETORCH4
    { SPR_TGRN,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_GREENTORCH2,    0,  0 },  // S_GREENTORCH
    { SPR_TGRN,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_GREENTORCH3,    0,  0 },  // S_GREENTORCH2
    { SPR_TGRN,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_GREENTORCH4,    0,  0 },  // S_GREENTORCH3
    { SPR_TGRN,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_GREENTORCH,     0,  0 },  // S_GREENTORCH4
    { SPR_TRED,  0 | FF_FULLBRIGHT,   2,   nullptr,          S_REDTORCH2,      0,  0 },  // S_REDTORCH
    { SPR_TRED,  1 | FF_FULLBRIGHT,   2,   nullptr,          S_REDTORCH3,      0,  0 },  // S_REDTORCH2
    { SPR_TRED,  2 | FF_FULLBRIGHT,   2,   nullptr,          S_REDTORCH4,      0,  0 },  // S_REDTORCH3
    { SPR_TRED,  3 | FF_FULLBRIGHT,   2,   nullptr,          S_REDTORCH,       0,  0 },  // S_REDTORCH4
    { SPR_GOR1,  0,                   5,   nullptr,          S_BLOODYTWITCH2,  0,  0 },  // S_BLOODYTWITCH
    { SPR_GOR1,  1,                   7,   nullptr,          S_BLOODYTWITCH3,  0,  0 },  // S_BLOODYTWITCH2
    { SPR_GOR1,  2,                   4,   nullptr,          S_BLOODYTWITCH4,  0,  0 },  // S_BLOODYTWITCH3
    { SPR_GOR1,  1,                   3,   nullptr,          S_BLOODYTWITCH,   0,  0 },  // S_BLOODYTWITCH4
    { SPR_POL3,  0 | FF_FULLBRIGHT,   3,   nullptr,          S_HEADCANDLES2,   0,  0 },  // S_HEADCANDLES
    { SPR_POL3,  1 | FF_FULLBRIGHT,   3,   nullptr,          S_HEADCANDLES,    0,  0 },  // S_HEADCANDLES2
    { SPR_POL6,  0,                   3,   nullptr,          S_LIVESTICK2,     0,  0 },  // S_LIVESTICK
    { SPR_POL6,  1,                   4,   nullptr,          S_LIVESTICK,      0,  0 },  // S_LIVESTICK2
    { SPR_TRE2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BIGTREE
    { SPR_HDB1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGNOGUTS
    { SPR_HDB2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGBNOBRAIN
    { SPR_HDB3,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGTLOOKDN
    { SPR_HDB4,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGTSKULL
    { SPR_HDB5,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGTLOOKUP
    { SPR_HDB6,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_HANGTNOBRAIN
    { SPR_POB1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_COLONGIBS
    { SPR_POB2,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_SMALLPOOL
    { SPR_BRS1,  0,                  -1,   nullptr,          S_NULL,           0,  0 },  // S_BRAINSTEM
};

const VmPtr<mobjinfo_t[NUMMOBJTYPES]> gMObjInfo(0x8005E03C);


#include "info.h"

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

const VmPtr<state_t[NUMSTATES]>             gStates(0x80058D8C);
const VmPtr<mobjinfo_t[NUMMOBJTYPES]>       gMObjInfo(0x8005E03C);

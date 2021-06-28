#pragma once

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Sequence identifiers for all sfx and original music tracks in the game
//------------------------------------------------------------------------------------------------------------------------------------------
enum sfxenum_t : int32_t {
    sfx_None,       // No sound
    sfx_sgcock,     // Weapon pickup sound
    sfx_punch,      // Punch hit
    sfx_itmbk,      // Deathmatch item respawn
    sfx_firsht2,    // Demon/Baron/Cacodemon etc. fireball sound
    sfx_barexp,     // Barrel/rocket explode
    sfx_firxpl,     // Demon fireball hit
    sfx_pistol,     // Pistol fire
    sfx_shotgn,     // Shotgun fire
    sfx_plasma,     // Plasma rifle fire
    sfx_bfg,        // BFG start firing
    sfx_sawup,      // Chainsaw being started up
    sfx_sawidl,     // Chainsaw idle loop
    sfx_sawful,     // Chainsaw saw
    sfx_sawhit,     // Chainsaw hit
    sfx_rlaunc,     // Rocket fire sound
    sfx_rxplod,     // BFG explosion sound
    sfx_pstart,     // Elevator start
    sfx_pstop,      // Elevator/mover stop (also menu up/down sound)
    sfx_doropn,     // Regular/slow door open
    sfx_dorcls,     // Regular/slow door close
    sfx_stnmov,     // Floor/crusher move sound
    sfx_swtchn,     // Switch activate
    sfx_swtchx,     // Exit switch activate
    sfx_itemup,     // Bonus pickup
    sfx_wpnup,      // Weapon pickup sound
    sfx_oof,        // Ooof sound after falling hard, or when trying to use unusable wall
    sfx_telept,     // Teleport sound
    sfx_noway,      // Ooof sound after falling hard, or when trying to use unusable wall
    sfx_dshtgn,     // Super shotgun fire
    sfx_dbopn,      // SSG open barrel
    sfx_dbload,     // SSG load shells
    sfx_dbcls,      // SSG close barrel
    sfx_plpain,     // Player pain sound
    sfx_pldeth,     // Player death sound
    sfx_slop,       // Gib/squelch sound
    sfx_posit1,     // Former human sight: 1
    sfx_posit2,     // Former human sight: 2
    sfx_posit3,     // Former human sight: 3 (unused)
    sfx_podth1,     // Former human death: 1
    sfx_podth2,     // Former human death: 2
    sfx_podth3,     // Former human death: 3 (unused)
    sfx_posact,     // Former human idle
    sfx_popain,     // Former human pain
    sfx_dmpain,     // Demon pain
    sfx_dmact,      // Demon idle/growl
    sfx_claw,       // Imp/Baron etc. melee claw
    sfx_bgsit1,     // Imp sight: 1
    sfx_bgsit2,     // Imp sight: 2
    sfx_bgdth1,     // Imp death: 1
    sfx_bgdth2,     // Imp death: 2
    sfx_bgact,      // Imp idle
    sfx_sgtsit,     // Demon sight
    sfx_sgtatk,     // Demon attack
    sfx_sgtdth,     // Demon death
    sfx_brssit,     // Baron sight
    sfx_brsdth,     // Baron death
    sfx_cacsit,     // Cacodemon sight
    sfx_cacdth,     // Cacodemon death
    sfx_sklatk,     // Lost Soul attack
    sfx_skldth,     // (Unused) Intended for Lost Soul death?
    sfx_kntsit,     // Knight sight
    sfx_kntdth,     // Knight death
    sfx_pesit,      // Pain Elemental sight
    sfx_pepain,     // Pain Elemental pain
    sfx_pedth,      // Pain Elemental death
    sfx_bspsit,     // Arachnotron sight
    sfx_bspdth,     // Arachnotron death
    sfx_bspact,     // Arachnotron idle
    sfx_bspwlk,     // Arachnotron hoof
    sfx_manatk,     // Mancubus attack
    sfx_mansit,     // Mancubus sight
    sfx_mnpain,     // Mancubus pain
    sfx_mandth,     // Mancubus death
    sfx_firsht,     // Demon/Baron/Cacodemon etc. fireball sound
    sfx_skesit,     // Revenant sight
    sfx_skedth,     // Revenant death
    sfx_skeact,     // Revenant idle
    sfx_skeatk,     // Revenant missile fire
    sfx_skeswg,     // Revenant throw punch
    sfx_skepch,     // Revenant punch land
    sfx_cybsit,     // Cyberdemon sight
    sfx_cybdth,     // Cyberdemon death
    sfx_hoof,       // Cyberdemon hoof up
    sfx_metal,      // Cyberdemon thud down (metal)
    sfx_spisit,     // Spider Mastermind sight
    sfx_spidth,     // Spider Mastermind death
    sfx_bdopn,      // Fast/blaze door open
    sfx_bdcls,      // Fast/blaze door close
    sfx_getpow,     // Powerup pickup
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: sounds for PC Doom II actors which have been added back into the game.
// Also need to preserve sequence numbers for original game music - creating explicit entries for those tracks here.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS
    music_01,       // Retribution Dawns
    music_02,       // The Broken Ones
    music_03,       // Sanity's Edge
    music_04,       // Hell's Churn
    music_05,       // Digitized Pain
    music_06,       // Corrupted Core
    music_07,       // Mind Massacre
    music_08,       // Mutation
    music_09,       // A Calm Panic Rises
    music_10,       // Corrupted
    music_11,       // Breath Of Horror
    music_12,       // Beyond Fear
    music_13,       // Lamentation
    music_14,       // Twisted Beyond Reason
    music_15,       // The Slow Demonic Pulse
    music_16,       // In The Grip Of Madness
    music_17,       // Lurkers
    music_18,       // Creeping Brutality
    music_19,       // Steadfast Extermination
    music_20,       // Hopeless Despair
    music_21,       // Malignant
    music_22,       // Tendrils Of Hate
    music_23,       // Bells Of Agony
    music_24,       // Infectious
    music_25,       // Unhallowed
    music_26,       // Breath Of Corruption
    music_27,       // The Foulness Consumes
    music_28,       // Demon Drone
    music_29,       // Vexation
    music_30,       // Larva Circuits
    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: reimplemented Arch-vile
    //--------------------------------------------------------------------------------------------------------------------------------------
    sfx_vilsit,     // Arch-vile sight
    sfx_vipain,     // Arch-vile pain
    sfx_vildth,     // Arch-vile death
    sfx_vilact,     // Arch-vile idle
    sfx_vilatk,     // Arch-vile attack
    sfx_flamst,     // Arch-vile flames (start)
    sfx_flame,      // Arch-vile flames burn
    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: reimplemented Wolf SS
    //--------------------------------------------------------------------------------------------------------------------------------------
    sfx_sssit,      // Wolfenstein-SS sight
    sfx_ssdth,      // Wolfenstein-SS death
    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: reimplemented Commander Keen
    //--------------------------------------------------------------------------------------------------------------------------------------
    sfx_keenpn,     // Commander Keen pain
    sfx_keendt,     // Commander Keen death
    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: reimplemented Icon Of Sin
    //--------------------------------------------------------------------------------------------------------------------------------------
    sfx_bossit,     // Icon of Sin sight
    sfx_bospit,     // Icon of Sin cube spit
    sfx_bospn,      // Icon of Sin pain
    sfx_bosdth,     // Icon of Sin death
    sfx_boscub,     // Icon of Sin spawn cube fly

    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: NOTE: new user music tracks could use sequence ids following the above ones...
    // So a new user track 'music_31' could start at sequence index '136' and so on.
    //--------------------------------------------------------------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------------------------------------------------------
    // PsyDoom: 'NUMSFX' is not sufficient to describe the range of sound effects anymore.
    // There are now two separate sequence id ranges for sound effects, describe them here.
    //--------------------------------------------------------------------------------------------------------------------------------------
    SFX_RANGE1_BEG = sfx_None,
    SFX_RANGE1_END = sfx_getpow + 1,
    SFX_RANGE2_BEG = sfx_vilsit,
    SFX_RANGE2_END = sfx_boscub + 1,
#else
    NUMSFX
#endif
};

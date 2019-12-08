#include "PSXSND.h"

#include "digload.h"
#include "Doom/Base/s_sound.h"
#include "psxcd.h"
#include "PsxVm/PsxVm.h"
#include "seqload.h"
#include "seqload_r.h"
#include "wessapi.h"

void PsxSoundInit() noexcept {
loc_800415EC:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    wess_init();
    psxcd_init();
    a0 = 0xC9;                                          // Result = 000000C9
    psxcd_open();
    a0 = s1;
    s0 = v0;
    a1 = lw(s0 + 0x4);
    a2 = s0;
    psxcd_read();
    a0 = s0;
    psxcd_close();
    s2 = 0x80080000;                                    // Result = 80080000
    s2 -= 0x1364;                                       // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    a0 = s2;                                            // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    ZeroHalfWord();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    ZeroHalfWord();
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7A78;                                       // Result = gPSXSND_wmdMemBuffer[0] (80078588)
    a2 = lw(gp + 0x824);                                // Load from: gPSXSND_maxWmdSize (80077E04)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 += 0x7E08;                                       // Result = gPSXSND_soundSettingsLists[0] (80077E08)
    a0 = s1;
    wess_load_module();
    wess_get_master_status();
    a0 = v0;
    wess_dig_lcd_loader_init();
    wess_get_master_status();
    a0 = v0;
    a1 = 0xC9;                                          // Result = 000000C9
    a2 = 1;                                             // Result = 00000001
    wess_seq_loader_init();
    wess_get_wmd_end();
    a0 = 0;                                             // Result = 00000000
    a1 = 0x5A;                                          // Result = 0000005A
    a2 = v0;
    wess_seq_range_load();
    s0 = v0;
    wess_get_wmd_end();
    v0 += s0;
    sw(v0, gp + 0x83C);                                 // Store to: gpMusSequencesEnd (80077E1C)
    a0 = s3;
    S_SetSfxVolume();
    a0 = s4;
    S_SetMusicVolume();
    a0 = 0xC8;                                          // Result = 000000C8
    a1 = 0x1010;                                        // Result = 00001010
    a2 = s2;                                            // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    sw(0, gp + 0x840);                                  // Store to: gbDidLoadDoomSfxLcd (80077E20)
    a3 = 0;                                             // Result = 00000000
    wess_dig_lcd_load();
    v0 += 0x1010;
    sw(v0, gp + 0x838);                                 // Store to: gNextSoundUploadAddr (80077E18)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x840);                                 // Store to: gbDidLoadDoomSfxLcd (80077E20)
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PsxSoundExit() noexcept {
    return;
}

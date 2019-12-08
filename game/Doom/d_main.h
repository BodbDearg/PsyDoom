void D_DoomMain() noexcept;
void RunLegals() noexcept;
void RunTitle() noexcept;
void RunDemo() noexcept;
void RunCredits() noexcept;
void I_SetDebugDrawStringPos() noexcept;
void I_DebugDrawString() noexcept;
void MiniLoop() noexcept;

// wessseq
void Read_Vlq() noexcept;
void Write_Vlq() noexcept;
void Len_Vlq() noexcept;
void Eng_DriverInit() noexcept;
void Eng_DriverExit() noexcept;
void Eng_DriverEntry1() noexcept;
void Eng_DriverEntry2() noexcept;
void Eng_DriverEntry3() noexcept;
void Eng_TrkOff() noexcept;
void Eng_TrkMute() noexcept;
void Eng_PatchChg() noexcept;
void Eng_PatchMod() noexcept;
void Eng_PitchMod() noexcept;
void Eng_ZeroMod() noexcept;
void Eng_ModuMod() noexcept;
void Eng_VolumeMod() noexcept;
void Eng_PanMod() noexcept;
void Eng_PedalMod() noexcept;
void Eng_ReverbMod() noexcept;
void Eng_ChorusMod() noexcept;
void Eng_NoteOn() noexcept;
void Eng_NoteOff() noexcept;
void Eng_StatusMark() noexcept;
void Eng_GateJump() noexcept;
void Eng_IterJump() noexcept;
void Eng_ResetGates() noexcept;
void Eng_ResetIters() noexcept;
void Eng_WriteIterBox() noexcept;
void Eng_SeqTempo() noexcept;
void Eng_SeqGosub() noexcept;
void Eng_SeqJump() noexcept;
void Eng_SeqRet() noexcept;
void Eng_SeqEnd() noexcept;
void Eng_TrkTempo() noexcept;
void Eng_TrkGosub() noexcept;
void Eng_TrkJump() noexcept;
void Eng_TrkRet() noexcept;
void Eng_TrkEnd() noexcept;
void Eng_NullEvent() noexcept;
void SeqEngine() noexcept;

// digload
void wess_dig_lcd_loader_init() noexcept;
void wess_dig_set_sample_position() noexcept;
void lcd_open() noexcept;
void lcd_upload_spu_samples() noexcept;
void lcd_close() noexcept;
void wess_dig_lcd_load() noexcept;

// wessapim
void wess_master_sfx_volume_get() noexcept;
void wess_master_mus_volume_get() noexcept;
void wess_master_sfx_vol_set() noexcept;
void wess_master_mus_vol_set() noexcept;
void wess_pan_mode_get() noexcept;
void wess_pan_mode_set() noexcept;

// seqloadr
void wess_seq_range_sizeof() noexcept;
void wess_seq_range_load() noexcept;
void wess_seq_range_free() noexcept;

// d_main
void D_DoomMain() noexcept;
void RunLegals() noexcept;
void RunTitle() noexcept;
void RunDemo() noexcept;
void RunCredits() noexcept;
void I_SetDebugDrawStringPos() noexcept;
void I_DebugDrawString() noexcept;
void MiniLoop() noexcept;

// r_main
void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
void R_SlopeDiv() noexcept;
void R_PointToAngle2() noexcept;
void R_PointOnSide() noexcept;
void R_PointInSubsector() noexcept;

// vsprintf
void D_mystrlen() noexcept;
void D_vsprintf() noexcept;

// l_main
void START_Legals() noexcept;
void STOP_Legals() noexcept;
void TIC_Legals() noexcept;
void DRAW_Legals() noexcept;

// t_main
void START_Title() noexcept;
void STOP_Title() noexcept;
void TIC_Title() noexcept;
void DRAW_Title() noexcept;

// m_main
void RunMenu() noexcept;
void M_Start() noexcept;
void M_Stop() noexcept;
void M_Ticker() noexcept;
void M_Drawer() noexcept;

// i_crossfade
void I_CrossFadeFrameBuffers() noexcept;

// c_main
void START_Credits() noexcept;
void STOP_Credits() noexcept;
void TIC_Credits() noexcept;
void DRAW_Credits() noexcept;

// pw_main
void START_PasswordScreen() noexcept;
void STOP_PasswordScreen() noexcept;
void TIC_PasswordScreen() noexcept;
void DRAW_PasswordScreen() noexcept;

// ctrl_main
void START_ControlsScreen() noexcept;
void STOP_ControlsScreen() noexcept;
void TIC_ControlsScreen() noexcept;
void DRAW_ControlsScreen() noexcept;

// p_password
void P_ComputePassword() noexcept;
void P_ProcessPassword() noexcept;

// st_main
void ST_Init() noexcept;
void ST_Start() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;

// i_misc
void I_DrawNumber() noexcept;
void I_DrawStringSmall() noexcept;
void I_DrawPausedOverlay() noexcept;
void P_UpdatePalette() noexcept;
void I_GetStringXPosToCenter() noexcept;
void I_DrawString() noexcept;

// am_main
void AM_Start() noexcept;
void AM_Control() noexcept;
void AM_Drawer() noexcept;
void DrawLine() noexcept;

// in_main
void IN_Start() noexcept;
void IN_Stop() noexcept;
void IN_Ticker() noexcept;
void IN_Drawer() noexcept;
void IN_SingleDrawer() noexcept;
void IN_CoopDrawer() noexcept;
void IN_DeathmatchDrawer() noexcept;

// f_finale
void F1_Start() noexcept;
void F1_Stop() noexcept;
void F1_Ticker() noexcept;
void F1_Drawer() noexcept;
void F2_Start() noexcept;
void F2_Stop() noexcept;
void F2_Ticker() noexcept;
void F2_Drawer() noexcept;

// o_main
void O_Init() noexcept;
void O_Shutdown() noexcept;
void O_Control() noexcept;
void O_Drawer() noexcept;

// m_fixed
void FixedMul() noexcept;
void FixedDiv() noexcept;

void PsxSoundInit() noexcept;
void PsxSoundExit() noexcept;
void trackstart() noexcept;
void trackstop() noexcept;
void queue_wess_seq_pause() noexcept;
void queue_wess_seq_restart() noexcept;
void queue_wess_seq_pauseall() noexcept;
void queue_wess_seq_restartall() noexcept;
void zeroset() noexcept;
void wess_install_error_handler() noexcept;
void wess_get_master_status() noexcept;
void Is_System_Active() noexcept;
void Is_Module_Loaded() noexcept;
void Is_Seq_Num_Valid() noexcept;
void Register_Early_Exit() noexcept;
void wess_install_handler() noexcept;
void wess_restore_handler() noexcept;
void wess_init() noexcept;
void wess_exit() noexcept;
void wess_get_wmd_start() noexcept;
void wess_get_wmd_end() noexcept;
void free_mem_if_mine() noexcept;
void wess_unload_module() noexcept;
void wess_memcpy() noexcept;
void conditional_read() noexcept;
void wess_load_module() noexcept;
void filltrackstat() noexcept;
void assigntrackstat() noexcept;
void wess_seq_structrig() noexcept;
void wess_seq_trigger() noexcept;
void wess_seq_trigger_special() noexcept;
void wess_seq_status() noexcept;
void wess_seq_stop() noexcept;
void wess_seq_stopall() noexcept;
void wess_low_level_init() noexcept;
void wess_low_level_exit() noexcept;
void wess_malloc() noexcept;
void wess_free() noexcept;
void GetIntsPerSec() noexcept;
void CalcPartsPerInt() noexcept;
void WessInterruptHandler() noexcept;
void init_WessTimer() noexcept;
void exit_WessTimer() noexcept;
void Wess_init_for_LoadFileData() noexcept;
void module_open() noexcept;
void module_read() noexcept;
void module_seek() noexcept;
void module_tell() noexcept;
void module_close() noexcept;
void get_num_Wess_Sound_Drivers() noexcept;
void data_open() noexcept;
void data_read_chunk() noexcept;
void data_read() noexcept;
void data_close() noexcept;
void updatetrackstat() noexcept;
void wess_seq_trigger_type() noexcept;
void wess_seq_trigger_type_special() noexcept;
void queue_wess_seq_update_type_special() noexcept;
void wess_seq_stoptype() noexcept;
void wess_seq_load_err() noexcept;
void wess_seq_loader_install_error_handler() noexcept;
void Is_Seq_Seq_Num_Valid() noexcept;
void open_sequence_data() noexcept;
void close_sequence_data() noexcept;
void load_sequence_data() noexcept;
void wess_seq_loader_init() noexcept;
void wess_seq_loader_exit() noexcept;
void wess_seq_sizeof() noexcept;
void wess_seq_load() noexcept;
void wess_seq_free() noexcept;
void psxspu_init_reverb() noexcept;
void psxspu_set_reverb_depth() noexcept;
void psxspu_init() noexcept;
void psxspu_update_master_vol() noexcept;
void psxspu_update_master_vol_mode() noexcept;
void psxspu_setcdmixon() noexcept;
void psxspu_setcdmixoff() noexcept;
void psxspu_fadeengine() noexcept;
void psxspu_set_cd_vol() noexcept;
void psxspu_get_cd_vol() noexcept;
void psxspu_start_cd_fade() noexcept;
void psxspu_stop_cd_fade() noexcept;
void psxspu_get_cd_fade_status() noexcept;
void psxspu_set_master_vol() noexcept;
void psxspu_get_master_vol() noexcept;
void psxspu_start_master_fade() noexcept;
void psxspu_stop_master_fade() noexcept;
void psxspu_get_master_fade_status() noexcept;
void start_record_music_mute() noexcept;
void end_record_music_mute() noexcept;
void add_music_mute_note() noexcept;
void PSX_UNKNOWN_DrvFunc() noexcept;
void TriggerPSXVoice() noexcept;
void PSX_DriverInit() noexcept;
void PSX_DriverExit() noexcept;
void PSX_DriverEntry1() noexcept;
void PSX_DriverEntry2() noexcept;
void PSX_DriverEntry3() noexcept;
void PSX_TrkOff() noexcept;
void PSX_TrkMute() noexcept;
void PSX_PatchChg() noexcept;
void PSX_PatchMod() noexcept;
void PSX_PitchMod() noexcept;
void PSX_ZeroMod() noexcept;
void PSX_ModuMod() noexcept;
void PSX_VolumeMod() noexcept;
void PSX_PanMod() noexcept;
void PSX_PedalMod() noexcept;
void PSX_ReverbMod() noexcept;
void PSX_ChorusMod() noexcept;
void PSX_voiceon() noexcept;
void PSX_voiceparmoff() noexcept;
void PSX_voicerelease() noexcept;
void PSX_voicenote() noexcept;
void PSX_NoteOn() noexcept;
void PSX_NoteOff() noexcept;
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
void wess_dig_lcd_loader_init() noexcept;
void wess_dig_set_sample_position() noexcept;
void lcd_open() noexcept;
void lcd_upload_spu_samples() noexcept;
void lcd_close() noexcept;
void wess_dig_lcd_load() noexcept;
void wess_master_sfx_volume_get() noexcept;
void wess_master_mus_volume_get() noexcept;
void wess_master_sfx_vol_set() noexcept;
void wess_master_mus_vol_set() noexcept;
void wess_pan_mode_get() noexcept;
void wess_pan_mode_set() noexcept;
void wess_seq_range_sizeof() noexcept;
void wess_seq_range_load() noexcept;
void wess_seq_range_free() noexcept;

void psx_main() noexcept;

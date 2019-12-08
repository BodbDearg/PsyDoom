void D_DoomMain() noexcept;
void RunLegals() noexcept;
void RunTitle() noexcept;
void RunDemo() noexcept;
void RunCredits() noexcept;
void I_SetDebugDrawStringPos() noexcept;
void I_DebugDrawString() noexcept;

void MiniLoop() noexcept;
void G_DoLoadLevel() noexcept;
void G_PlayerFinishLevel() noexcept;
void G_PlayerReborn() noexcept;
void G_DoReborn() noexcept;
void G_SetGameComplete() noexcept;
void G_InitNew() noexcept;
void G_RunGame() noexcept;
void G_PlayDemoPtr() noexcept;
void empty_func1() noexcept;
void P_RunMobjBase() noexcept;
void P_XYMovement() noexcept;
void P_FloatChange() noexcept;
void P_ZMovement() noexcept;
void P_MobjThinker() noexcept;
void PB_TryMove() noexcept;
void PB_UnsetThingPosition() noexcept;
void PB_SetThingPosition() noexcept;
void PB_CheckPosition() noexcept;
void PB_BoxCrossLine() noexcept;
void PB_CheckLine() noexcept;
void PB_CheckThing() noexcept;
void PB_CheckLines() noexcept;
void PB_CheckThings() noexcept;

void P_CheckPosition() noexcept;
void P_TryMove() noexcept;
void P_InterceptVector() noexcept;
void PIT_UseLines() noexcept;
void P_UseLines() noexcept;
void PIT_RadiusAttack() noexcept;
void P_RadiusAttack() noexcept;
void P_AimLineAttack() noexcept;
void P_LineAttack() noexcept;
void P_AproxDistance() noexcept;
void P_PointOnLineSide() noexcept;
void P_PointOnDivlineSide() noexcept;
void P_MakeDivline() noexcept;
void P_LineOpening() noexcept;
void P_UnsetThingPosition() noexcept;
void P_SetThingPosition() noexcept;
void P_BlockLinesIterator() noexcept;
void P_BlockThingsIterator() noexcept;
void P_RemoveMObj() noexcept;
void P_RespawnSpecials() noexcept;
void P_SetMObjState() noexcept;
void P_ExplodeMissile() noexcept;
void P_SpawnMObj() noexcept;
void P_SpawnPlayer() noexcept;
void P_SpawnMapThing() noexcept;
void P_SpawnPuff() noexcept;
void P_SpawnBlood() noexcept;
void P_CheckMissileSpawn() noexcept;
void P_SpawnMissile() noexcept;
void P_SpawnPlayerMissile() noexcept;
void P_TryMove2() noexcept;
void UNKNOWN_DoomFunc3() noexcept;
void P_UnsetThingPosition2() noexcept;
void P_SetThingPosition2() noexcept;
void PM_CheckPosition() noexcept;
void PM_BoxCrossLine() noexcept;
void PIT_CheckLine() noexcept;
void PIT_CheckThing() noexcept;
void PM_CheckLines() noexcept;
void PM_CheckThings() noexcept;

void P_Shoot2() noexcept;
void PA_DoIntercept() noexcept;
void PA_ShootLine() noexcept;
void PA_ShootThing() noexcept;
void PA_SightCrossLine() noexcept;
void PA_CrossSubsector() noexcept;
void PointOnVectorSide() noexcept;
void PA_CrossBSPNode() noexcept;
void P_CheckSights() noexcept;
void P_CheckSight() noexcept;
void PS_SightCrossLine() noexcept;
void P_CrossSubsector() noexcept;
void PS_CrossBSPNode() noexcept;
void P_SlideMove() noexcept;
void P_CompletableFrac() noexcept;
void SL_PointOnSide() noexcept;
void SL_CrossFrac() noexcept;
void CheckLineEnds() noexcept;
void ClipToLine() noexcept;
void SL_CheckLine() noexcept;
void SL_PointOnSide2() noexcept;
void SL_CheckSpecialLines() noexcept;

void P_AddThinker() noexcept;
void P_RemoveThinker() noexcept;
void P_RunThinkers() noexcept;
void P_RunMobjLate() noexcept;
void P_CheckCheats() noexcept;
void P_Ticker() noexcept;
void P_Drawer() noexcept;
void P_Start() noexcept;
void P_Stop() noexcept;
void P_PlayerMove() noexcept;
void P_PlayerXYMovement() noexcept;
void P_PlayerZMovement() noexcept;
void P_PlayerMobjThink() noexcept;
void P_BuildMove() noexcept;
void P_Thrust() noexcept;
void P_CalcHeight() noexcept;
void P_MovePlayer() noexcept;
void P_DeathThink() noexcept;
void P_PlayerThink() noexcept;
void R_BSP() noexcept;
void R_RenderBSPNode() noexcept;
void R_CheckBBox() noexcept;
void R_Subsector() noexcept;
void R_AddLine() noexcept;
void R_InitData() noexcept;
void R_InitTextures() noexcept;
void R_InitFlats() noexcept;
void R_InitSprites() noexcept;
void R_TextureNumForName() noexcept;
void R_FlatNumForName() noexcept;
void R_InitPalette() noexcept;
void R_DrawSky() noexcept;
void R_DrawSubsector() noexcept;
void R_FrontZClip() noexcept;
void R_CheckEdgeVisible() noexcept;
void R_LeftEdgeClip() noexcept;
void R_RightEdgeClip() noexcept;
void R_DrawSubsectorSeg() noexcept;
void R_DrawWallColumns() noexcept;
void R_DrawSubsectorFlat() noexcept;
void R_DrawFlatSpans() noexcept;
void R_DrawSubsectorSprites() noexcept;
void R_DrawWeapon() noexcept;
void R_Init() noexcept;
void R_RenderPlayerView() noexcept;
void R_SlopeDiv() noexcept;
void R_PointToAngle2() noexcept;
void R_PointOnSide() noexcept;
void R_PointInSubsector() noexcept;
void D_mystrlen() noexcept;
void D_vsprintf() noexcept;

void StartGame() noexcept;
void init_sony_system() noexcept;
void I_Error() noexcept;
void I_ReadGamepad() noexcept;
void I_CacheTexForLumpName() noexcept;
void I_CacheAndDrawSprite() noexcept;
void I_DrawSprite() noexcept;
void I_DrawPlaque() noexcept;
void I_IncDrawnFrameCount() noexcept;
void I_DrawPresent() noexcept;
void I_VsyncCallback() noexcept;
void I_Init() noexcept;
void I_CacheTex() noexcept;
void I_RemoveTexCacheEntry() noexcept;
void I_ResetTexCache() noexcept;
void I_VramViewerDraw() noexcept;
void I_NetSetup() noexcept;
void I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_NetSendRecv() noexcept;
void I_SubmitGpuCmds() noexcept;
void I_LocalButtonsToNet() noexcept;
void I_NetButtonsToLocal() noexcept;
void START_Legals() noexcept;
void STOP_Legals() noexcept;
void TIC_Legals() noexcept;
void DRAW_Legals() noexcept;
void START_Title() noexcept;
void STOP_Title() noexcept;
void TIC_Title() noexcept;
void DRAW_Title() noexcept;
void RunMenu() noexcept;
void M_Start() noexcept;
void M_Stop() noexcept;
void M_Ticker() noexcept;
void M_Drawer() noexcept;
void I_CrossFadeFrameBuffers() noexcept;
void START_Credits() noexcept;
void STOP_Credits() noexcept;
void TIC_Credits() noexcept;
void DRAW_Credits() noexcept;
void START_PasswordScreen() noexcept;
void STOP_PasswordScreen() noexcept;
void TIC_PasswordScreen() noexcept;
void DRAW_PasswordScreen() noexcept;
void START_ControlsScreen() noexcept;
void STOP_ControlsScreen() noexcept;
void TIC_ControlsScreen() noexcept;
void DRAW_ControlsScreen() noexcept;
void P_ComputePassword() noexcept;
void P_ProcessPassword() noexcept;
void ST_Init() noexcept;
void ST_Start() noexcept;
void ST_Ticker() noexcept;
void ST_Drawer() noexcept;
void I_DrawNumber() noexcept;
void I_DrawStringSmall() noexcept;
void I_DrawPausedOverlay() noexcept;
void P_UpdatePalette() noexcept;
void I_GetStringXPosToCenter() noexcept;
void I_DrawString() noexcept;
void AM_Start() noexcept;
void AM_Control() noexcept;
void AM_Drawer() noexcept;
void DrawLine() noexcept;
void IN_Start() noexcept;
void IN_Stop() noexcept;
void IN_Ticker() noexcept;
void IN_Drawer() noexcept;
void IN_SingleDrawer() noexcept;
void IN_CoopDrawer() noexcept;
void IN_DeathmatchDrawer() noexcept;
void F1_Start() noexcept;
void F1_Stop() noexcept;
void F1_Ticker() noexcept;
void F1_Drawer() noexcept;
void F2_Start() noexcept;
void F2_Stop() noexcept;
void F2_Ticker() noexcept;
void F2_Drawer() noexcept;
void O_Init() noexcept;
void O_Shutdown() noexcept;
void O_Control() noexcept;
void O_Drawer() noexcept;

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
void psyq_main() noexcept;

#if !PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Platform implementation for starting a sound.
//
// Note: this function originally took 3 parameters, but since the 3rd param was completely unused we cannnot infer what it was.
// I've just removed this unknown 3rd param here for this reimplementation, since it serves no purpose.
//------------------------------------------------------------------------------------------------------------------------------------------
static void I_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept {
    // Ignore the request if the sound sequence number is invalid
    if (soundId >= NUMSFX)
        return;

    // Grab the listener (player) and default the pan/volume for now
    mobj_t* pListener = gPlayers[gCurPlayerIndex].mo;
    int32_t vol = WESS_MAX_MASTER_VOL;
    int32_t pan = WESS_PAN_CENTER;
    const bool bAttenuateSound = (pOrigin && (pOrigin != pListener));

    if (bAttenuateSound) {
        // Figure out the approximate distance to the sound source and don't play if too far away
        const fixed_t dx = std::abs(pListener->x - pOrigin->x);
        const fixed_t dy = std::abs(pListener->y - pOrigin->y);
        const fixed_t approxDist = dx + dy - d_rshift<1>(std::min(dx, dy));

        if (approxDist > S_CLIPPING_DIST)
            return;

        // Figure out the relative angle to the player.
        // Not sure what the addition of UINT32_MAX is about, was in Linux Doom also but not commented.
        angle_t angle = R_PointToAngle2(pListener->x, pListener->y, pOrigin->x, pOrigin->y);

        if (angle <= pListener->angle) {
            angle += UINT32_MAX;
        }

        angle -= pListener->angle;

        // Figure out pan amount based on the angle
        {
            const fixed_t sina = gFineSine[angle >> ANGLETOFINESHIFT];
            pan = WESS_PAN_CENTER - d_rshift<1>(d_fixed_to_int(FixedMul(sina, S_STEREO_SWING)));
        }

        // Figure out volume level
        if (approxDist < S_CLOSE_DIST) {
            vol = WESS_MAX_MASTER_VOL;
        } else {
            vol = (d_fixed_to_int(S_CLIPPING_DIST - approxDist) * WESS_MAX_MASTER_VOL) / S_ATTENUATOR;
        }

        // If the origin is exactly where the player is then do no pan
        if ((pOrigin->x == pListener->x) && (pOrigin->y == pListener->y)) {
            pan = WESS_PAN_CENTER;
        }

        // If volume is zero then don't play
        if (vol <= 0)
            return;
    }

    // Disable reverb if the origin is in a sector that forbids it
    TriggerPlayAttr sndAttribs;

    if (pOrigin && (pOrigin->subsector->sector->flags & SF_NO_REVERB)) {
        sndAttribs.reverb = 0;
    } else {
        sndAttribs.reverb = WESS_MAX_REVERB_DEPTH;
    }

    // Set the other sound attributes and play the sound.
    // Note that the original code also wrote volume and pan to unused globals here but I've omitted that code since it is useless:
    sndAttribs.attribs_mask = TRIGGER_VOLUME | TRIGGER_PAN | TRIGGER_REVERB;
    sndAttribs.volume_cntrl = (uint8_t) vol;
    sndAttribs.pan_cntrl = (uint8_t) pan;

    wess_seq_trigger_type_special(soundId, (uintptr_t) pOrigin, &sndAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the sound effect with the given id.
// The origin parameter is optional and helps determine panning/attenuation.
//------------------------------------------------------------------------------------------------------------------------------------------
void S_StartSound(mobj_t* const pOrigin, const sfxenum_t soundId) noexcept {
    I_StartSound(pOrigin, soundId);
}
#endif  // #if !PSYDOOM_MODS

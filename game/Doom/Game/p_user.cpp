#include "p_user.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_slide.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PcPsx/Config.h"
#include "PcPsx/Input.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"

#include <algorithm>
#include <cmath>

// Accelerating turn speeds: normal
static constexpr fixed_t ANGLE_TURN[10] = { 
    300, 300, 500, 500, 600, 700, 800, 900, 900, 1000
};

// Accelerating turn speeds: fast
static constexpr fixed_t FAST_ANGLE_TURN[10] = {
    800, 800, 900, 1000, 1000, 1200, 1200, 1300, 1300, 1400
};

static constexpr fixed_t STOPSPEED              = FRACUNIT / 16;                // Stop the player completely if velocity falls under this amount
static constexpr fixed_t FRICTION               = 0xd200;                       // Friction amount to apply to player movement: approximately 0.82
static constexpr fixed_t FORWARD_MOVE[2]        = { 0x40000, 0x60000 };         // Movement speeds: forward/back
static constexpr fixed_t SIDE_MOVE[2]           = { 0x38000, 0x58000 };         // Movement speeds: strafe left/right
static constexpr fixed_t MAX_FORWARD_MOVE       = FORWARD_MOVE[1];              // For convenience
static constexpr fixed_t MAX_SIDE_MOVE          = SIDE_MOVE[1];                 // For convenience
static constexpr int32_t TURN_ACCEL_TICS        = C_ARRAY_SIZE(ANGLE_TURN);     // Number of tics/stages in turn acceleration before it hits max speed
static constexpr int32_t TURN_TO_ANGLE_SHIFT    = 17;                           // How many bits to shift the turn amount left to scale it to an angle
static constexpr fixed_t MAXBOB                 = 16 * FRACUNIT;                // Maximum amount of view bobbing per frame (16 pixels)

static bool gbOnGround;     // Flag set to true when the player is on the ground

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to move the given player map object according to it's current velocity.
// Also crosses special lines in the process, and interacts with special things touched.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_PlayerMove(mobj_t& mobj) noexcept {
    // This is the amount to be moved
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[gPlayerNum];
    fixed_t moveDx = (mobj.momx >> 2) * elapsedVBlanks;
    fixed_t moveDy = (mobj.momy >> 2) * elapsedVBlanks;

    // Do the sliding movement
    gpSlideThing = &mobj;
    P_SlideMove();
    line_t* const pSpecialLineToCross = gpSpecialLine;

    // If we did not succeed in doing a sliding movement, or cannot go to the suggested location then try stairstepping.
    // With stairstepping we try to do the x and y axis movements alone, instead of together:
    const bool bSlideMoveBlocked = ((gSlideX == mobj.x) && (gSlideY == mobj.y));

    if (bSlideMoveBlocked || (!P_TryMove(mobj, gSlideX, gSlideY))) {
        // Clamp the movement amount so we don't go beyond the max
        moveDx = std::clamp(moveDx, -MAXMOVE, +MAXMOVE);
        moveDy = std::clamp(moveDy, -MAXMOVE, +MAXMOVE);

        // Try move in the y direction first
        if (!P_TryMove(mobj, mobj.x, mobj.y + moveDy)) {
            // Y move failed: try move in the x direction and kill all velocity in disallowed movement directions
            if (!P_TryMove(mobj, mobj.x + moveDx, mobj.y)) {
                mobj.momy = 0;
                mobj.momx = 0;
            } else {
                mobj.momx = moveDx;
                mobj.momy = 0;
            }
        } else {
            // Y move succeeded: kill all velocity in the x direction
            mobj.momx = 0;
            mobj.momy = moveDy;
        }
    }

    // Trigger line specials if we crossed a special line
    if (pSpecialLineToCross) {
        P_CrossSpecialLine(*pSpecialLineToCross, mobj);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do movement in the XY direction for the player map object and apply friction
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_PlayerXYMovement(mobj_t& mobj) noexcept {
    // Physically move the player and cross special lines, touch things etc.
    P_PlayerMove(mobj);
    
    // No friction when the player is in the air
    if (mobj.z > mobj.floorz)
        return;

    // If the player is a corpse and over a step then don't apply friction: slide down it
    if ((mobj.flags & MF_CORPSE) && (mobj.floorz != mobj.subsector->sector->floorheight))
        return;
        
    // If the player has reached a low enough velocity then stop completely, otherwise apply friction
    if ((std::abs(mobj.momx) < STOPSPEED) && (std::abs(mobj.momy) < STOPSPEED)) {
        mobj.momx = 0;
        mobj.momy = 0;
    } else {
        mobj.momx = (mobj.momx >> 8) * (FRICTION >> 8);
        mobj.momy = (mobj.momy >> 8) * (FRICTION >> 8);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does height clipping and movement z velocity for the player thing.
// Also applies gravity when the thing is in the air.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_PlayerZMovement(mobj_t& mobj) noexcept {
    player_t& player = *mobj.player;

    // Do smooth stepping up a step
    if (mobj.z < mobj.floorz) {
        player.viewheight -= mobj.floorz - mobj.z;                          // Adjust the view height by the difference
        player.deltaviewheight = (VIEWHEIGHT - player.viewheight) >> 2;     // This does a nice curved motion up (fast at first, then slowing down)
    }
    
    // Advance z position by the velocity
    mobj.z += mobj.momz;

    // Clip z position against the floor
    if (mobj.z <= mobj.floorz) {
        // Hitting the floor: kill any downward velocity and clamp z position to the floor
        if (mobj.momz < 0) {
            if (mobj.momz < -GRAVITY * 2) {
                // If we hit the floor hard then play the 'oof' sound
                player.deltaviewheight = mobj.momz >> 3;
                S_StartSound(&mobj, sfx_oof);
            }

            mobj.momz = 0;
        }

        mobj.z = mobj.floorz;
    }
    else {
        // Not hitting the floor: do acceleration due to gravity.
        // Gravity is doubled on the first falling tic to account for the fact that it was always being applied, even on the floor:
        mobj.momz -= (mobj.momz == 0) ? GRAVITY : GRAVITY / 2;
    }

    // Clip height against the ceiling and kill upwards velocity if we're hitting it
    if (mobj.z + mobj.height > mobj.ceilingz) {
        mobj.z = mobj.ceilingz - mobj.height;
        mobj.momz = std::min(mobj.momz, 0);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do movement and state transitions for the player's map object
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_PlayerMobjThink(mobj_t& mobj) noexcept {
    // Do XY and Z movement and velocity updates as required
    if ((mobj.momx != 0) || (mobj.momy != 0)) {
        P_PlayerXYMovement(mobj);
    }

    if ((mobj.z != mobj.floorz) || (mobj.momz != 0)) {
        P_PlayerZMovement(mobj);
    }

    // Transition to the next state if required, or never if the tic count is '-1' (forever).
    // Oddly, this logic does not call a state transition function for the player thing - not required because of player control?
    if (mobj.tics == -1)
        return;

    mobj.tics--;

    if (mobj.tics <= 0) {
        // Time to go to the next state: but don't call the state transition function as that's not needed for the player
        state_t& newState = gStates[mobj.state->nextstate];
        mobj.state = &newState;
        mobj.tics = newState.tics;
        mobj.sprite = newState.sprite;
        mobj.frame = newState.frame;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads inputs to decide how much the player will try to move and turn for a tic
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_BuildMove(player_t& player) noexcept {
    // Grab some useful stuff: elapsed vblanks, currend and old buttons and gamepad bindings
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[gPlayerNum];
    const uint32_t curBtns = gTicButtons[gPlayerNum];
    const uint32_t oldBtns = gOldTicButtons[gPlayerNum];

    const padbuttons_t* const pBtnBindings = gpPlayerCtrlBindings[gPlayerNum];
    
    // Do turn acceleration if turn is held continously for 2 frames or more
    const bool bLeftTurnAccel = ((curBtns & PAD_LEFT) && (oldBtns & PAD_LEFT));
    const bool bRightTurnAccel = ((curBtns & PAD_RIGHT) && (oldBtns & PAD_RIGHT));
    
    if (bLeftTurnAccel || bRightTurnAccel) {
        player.turnheld = std::min(player.turnheld + 1, TURN_ACCEL_TICS - 1);
    } else {
        player.turnheld = 0;
    }
    
    // Initially assume no turning or movement
    player.angleturn = 0;
    player.sidemove = 0;
    player.forwardmove = 0;

    // What movement speed is being used, run or normal?
    const uint32_t speedMode = (curBtns & pBtnBindings[cbind_speed]) ? 1 : 0;
    
    // Do strafe left/right controls
    if (curBtns & pBtnBindings[cbind_strafe_left]) {
        player.sidemove = (-SIDE_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
    }
    else if (curBtns & pBtnBindings[cbind_strafe_right]) {
        player.sidemove = (+SIDE_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
    }

    // Do turning or strafing controls (if strafe button held)
    const bool bDoFastTurn = ((speedMode != 0) && ((curBtns & (PAD_UP | PAD_DOWN)) == 0));

    if (curBtns & pBtnBindings[cbind_strafe]) {
        // Strafe button held: turn buttons become strafing buttons
        if (curBtns & PAD_LEFT) {
            player.sidemove = (-SIDE_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
        }
        else if (curBtns & PAD_RIGHT) {
            player.sidemove = (+SIDE_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
        }
    } 
    else {
        // No strafe button held: do normal turning
        fixed_t turnAmt = 0;

        if (bDoFastTurn) {
            // Do fast turning when run is pressed and we are not moving forward/back
            if (curBtns & PAD_LEFT) {
                turnAmt = +FAST_ANGLE_TURN[player.turnheld];
            } else if (curBtns & PAD_RIGHT) {
                turnAmt = -FAST_ANGLE_TURN[player.turnheld];
            }
        } else {
            // Otherwise do the slow turning speed
            if (curBtns & PAD_LEFT) {
                turnAmt = +ANGLE_TURN[player.turnheld];
            } else if (curBtns & PAD_RIGHT) {
                turnAmt = -ANGLE_TURN[player.turnheld];
            }
        }

        // Apply the turn amount adjusted for framerate
        turnAmt *= elapsedVBlanks;
        turnAmt /= VBLANKS_PER_TIC;
        player.angleturn = turnAmt << TURN_TO_ANGLE_SHIFT;
    }

    // PC-PSX: do analog controller turning.
    // TODO: make the controller turn axes bindable/configurable.
    #if PC_PSX_DOOM_MODS
    {
        // Get the axis value adjusted for the deadzone
        const float axis = Input::getAdjustedControllerInputValue(ControllerInput::AXIS_RIGHT_X, Config::gGamepadDeadZone);

        // Figure out how much of the high and low turn speeds to use.
        // Use the higher turn speed as the stick is pressed more:
        const float turnSpeedLow = (bDoFastTurn) ? Config::gGamepadFastTurnSpeed_Low : Config::gGamepadTurnSpeed_Low;
        const float turnSpeedHigh = (bDoFastTurn) ? Config::gGamepadFastTurnSpeed_High : Config::gGamepadTurnSpeed_High;
        const float turnSpeedMix = std::abs(axis);
        const float turnSpeed = turnSpeedLow * (1.0f - turnSpeedMix) + turnSpeedHigh * turnSpeedMix;

        // Apply the turn
        fixed_t analogTurn = (fixed_t)(axis * turnSpeed);
        analogTurn *= elapsedVBlanks;
        analogTurn /= VBLANKS_PER_TIC;
        player.angleturn -= analogTurn << TURN_TO_ANGLE_SHIFT;
    }
    #endif

    // PC-PSX: do mouse turning
    #if PC_PSX_DOOM_MODS
    {
        const float timeScale = (float) elapsedVBlanks / (float) VBLANKS_PER_TIC;
        const fixed_t mouseTurn = (fixed_t)(Input::getMouseXMovement() * Config::gMouseTurnSpeed * timeScale);
        player.angleturn -= mouseTurn << TURN_TO_ANGLE_SHIFT;
    }
    #endif

    // Do forward/backward movement controls
    if (curBtns & PAD_UP) {
        player.forwardmove = (+FORWARD_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
    } else if (curBtns & PAD_DOWN) {
        player.forwardmove = (-FORWARD_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
    }
    
    // PC-PSX: do analog movements.
    // TODO: make the controller move axes bindable/configurable.
    {
        const float axis = Input::getAdjustedControllerInputValue(ControllerInput::AXIS_LEFT_Y, Config::gGamepadDeadZone);
        const fixed_t move = (fixed_t)(axis * FORWARD_MOVE[speedMode]);
        player.forwardmove -= (move * elapsedVBlanks) / VBLANKS_PER_TIC;
        player.forwardmove = std::clamp(player.forwardmove, -MAX_FORWARD_MOVE, +MAX_FORWARD_MOVE);
    }

    {
        const float axis = Input::getAdjustedControllerInputValue(ControllerInput::AXIS_LEFT_X, Config::gGamepadDeadZone);
        const fixed_t move = (fixed_t)(axis * SIDE_MOVE[speedMode]);
        player.sidemove += (move * elapsedVBlanks) / VBLANKS_PER_TIC;
        player.sidemove = std::clamp(player.sidemove, -MAX_SIDE_MOVE, +MAX_SIDE_MOVE);
    }

    // If the player is not moving at all change the animation frame to standing
    mobj_t& mobj = *player.mo;

    if ((mobj.momx == 0) && (mobj.momy == 0) && (player.forwardmove == 0) && (player.sidemove == 0)) {
        // Only switch to the standing frame if we are running; if we are already standing do nothing:
        state_t& state = *mobj.state;

        const bool bIsInRunState = (
            (&state == &gStates[S_PLAY_RUN1]) ||
            (&state == &gStates[S_PLAY_RUN2]) ||
            (&state == &gStates[S_PLAY_RUN3]) ||
            (&state == &gStates[S_PLAY_RUN4])
        );

        if (bIsInRunState) {
            P_SetMObjState(mobj, S_PLAY);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies the specified amount of velocity to the player's map object in the given direction
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_Thrust(player_t& player, const angle_t angle, const fixed_t amount) noexcept {
    mobj_t& mobj = *player.mo;
    mobj.momx += (amount >> 8) * (gFineCosine[angle >> ANGLETOFINESHIFT] >> 8);
    mobj.momy += (amount >> 8) * (gFineSine[angle >> ANGLETOFINESHIFT] >> 8);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the player's current view height (for rendering) and does smooth stepping up stairs.
// Also applies view bobbing to the view height.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_CalcHeight(player_t& player) noexcept {
    // Compute movement bobbing amount
    mobj_t& mobj = *player.mo;

    {
        const int32_t speedX = mobj.momx >> 8;
        const int32_t speedY = mobj.momy >> 8;

        player.bob = (speedX * speedX) + (speedY * speedY);
        player.bob >>= 4;
        player.bob = std::min(player.bob, MAXBOB);
    }
    
    // When we are not on the ground just set the view z based on map object z and clamp below the ceiling
    const fixed_t maxViewZ = mobj.ceilingz - 4 * FRACUNIT;

    if (!gbOnGround) {
        player.viewz = mobj.z + VIEWHEIGHT;
        player.viewz = std::min(player.viewz, maxViewZ);    // Don't get too close to the ceiling!
        return;
    }

    // Do view height movements due smooth stepping up stairs
    if (player.playerstate == PST_LIVE) {
        player.viewheight += player.deltaviewheight;

        // Raised too far?
        if (player.viewheight > VIEWHEIGHT) {
            player.viewheight = VIEWHEIGHT;
            player.deltaviewheight = 0;     // Stop moving up
        }

        // Gone down to low?
        if (player.viewheight < VIEWHEIGHT / 2) {
            player.viewheight = VIEWHEIGHT / 2;
            player.deltaviewheight = std::max(player.deltaviewheight, 1);   // Start moving up
        }

        // If we are moving up still, accelerate the the movement over time
        if (player.deltaviewheight != 0) {
            player.deltaviewheight += FRACUNIT / 2;

            // Don't stop moving yet if we were moving down previously but now have flipped around
            if (player.deltaviewheight == 0) {
                player.deltaviewheight = 1;
            }
        }
    }

    // Compute the bob amplitude, which uses a sine wave as it's basis.
    // We step a certain amount of the bob with every vblank elapsed:
    constexpr int32_t BOB_PHASE_TIME = (VBLANKS_PER_SEC * 2) / 3;
    constexpr int32_t BOB_PHASE_STEP = FINEANGLES / BOB_PHASE_TIME;

    const uint32_t bobPhase = (gTicCon * BOB_PHASE_STEP) & FINEMASK;
    const fixed_t bobAmplitude = gFineSine[bobPhase];

    // Compute the final view z based on map object z, view height and bob amount
    player.viewz = mobj.z + player.viewheight + (player.bob >> 17) * bobAmplitude;

    // Clamp the view z so it's not to close to the ceiling (if need be)
    player.viewz = std::min(player.viewz, maxViewZ);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Applies turn rotation and velocity due to movement inputs to the player.
// Also puts the player into the run animation if moving.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_MovePlayer(player_t& player) noexcept {
    // Apply angle turning
    mobj_t& mobj = *player.mo;
    mobj.angle += player.angleturn;

    // Save whether we are on the ground
    gbOnGround = (mobj.z <= mobj.floorz);
    
    // Apply side and forward/backward movement velocity
    if ((player.forwardmove != 0) && gbOnGround) {
        P_Thrust(player, mobj.angle, player.forwardmove);
    }

    if ((player.sidemove != 0) && gbOnGround) {
        P_Thrust(player, mobj.angle - ANG90, player.sidemove);
    }

    // Switch to the running animation frames if moving and currently showing the standing still ones
    const bool bIsMoving = ((player.forwardmove != 0) || (player.sidemove != 0));

    if (bIsMoving && (mobj.state == &gStates[S_PLAY])) {
        P_SetMObjState(mobj, S_PLAY_RUN1);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does updates for the player when dead: drops down the view height, weapons and faces the killer etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_DeathThink(player_t& player) noexcept {
    // Drop the currently held weapon
    P_MovePsprites(player);

    // Lower the view to the ground
    if (player.viewheight > 0x80000) {
        player.viewheight -= FRACUNIT;
    }

    // Is the player on the ground now?
    mobj_t& playerMobj = *player.mo;
    gbOnGround = (playerMobj.z <= playerMobj.floorz);

    // Update the current view height
    P_CalcHeight(player);

    // If we were killed by an enemy then turn to face it
    mobj_t* const pAttacker = player.attacker;

    if (pAttacker && (pAttacker != &playerMobj)) {
        // Killed by an enemy: turn to face it, then fade out the reddamage tint
        const angle_t angleToAttacker = R_PointToAngle2(playerMobj.x, playerMobj.y, pAttacker->x, pAttacker->y);
        const angle_t angleDelta = angleToAttacker - playerMobj.angle;

        if ((angleDelta >= ANG5) && (angleDelta <= (angle_t) -ANG5)) {
            // Still turning to face the killer: turn the right way
            if (angleDelta < ANG180) {
                playerMobj.angle += ANG5;
            } else {
                playerMobj.angle -= ANG5;
            }
        }
        else {
            // Now looking directly at the killer: snap to it's angle and fade out the damage tint
            playerMobj.angle = angleToAttacker;

            if (player.damagecount != 0) {
                player.damagecount--;
            }
        }
    } 
    else {
        // Not killed by an enemy: just fade out the red damage tint
        if (player.damagecount != 0) {
            player.damagecount--;
        }
    }

    // Respawn if the right buttons are pressed and the player's view has dropped enough
    const bool bRespawnBtnPressed = (gTicButtons[gPlayerNum] & (PAD_ACTION_BTNS | PAD_SHOULDER_BTNS));

    if (bRespawnBtnPressed && (player.viewheight <= 8 * FRACUNIT)) {
        player.playerstate = PST_REBORN;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does all player updates such as movement, controls, weapon switching and so forth
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PlayerThink(player_t& player) noexcept {
    // Grab the current and previous buttons, and the gamepad bindings
    const uint32_t curBtns = gTicButtons[gPlayerNum];
    const uint32_t oldBtns = gOldTicButtons[gPlayerNum];
    const padbuttons_t* pBtnBindings = gpPlayerCtrlBindings[gPlayerNum];

    // Do weapon switching if the player is still alive (and even if paused)
    if (player.playerstate == PST_LIVE) {
        // Get the current weapon being switched to, or the ready weapon if not switching weapons
        const weapontype_t curWeaponType = (player.pendingweapon == wp_nochange) ? player.readyweapon : player.pendingweapon;
        
        // Get the current micro box that is highlighted (for this weapon)
        int32_t weaponMicroBoxIdx = WEAPON_MICRO_INDEXES[curWeaponType];
        int32_t nextWeaponIdx = weaponMicroBoxIdx;

        // See if next/previous weapon buttons have just been pressed
        const bool bGotoPrevWeapon = Utils::padBtnJustPressed(pBtnBindings[cbind_weapon_back], curBtns, oldBtns);
        const bool bGotoNextWeapon = Utils::padBtnJustPressed(pBtnBindings[cbind_weapon_forward], curBtns, oldBtns);

        // See if we are switching weapons
        if (bGotoPrevWeapon) {
            // Try to go to the previous weapon
            if ((weaponMicroBoxIdx == 0) && player.weaponowned[wp_chainsaw]) {
                // When we are on the 1st slot and own the chainsaw then allow toggling between fists and chainsaw.
                // This assignment doesn't mean anything, it's just set to trigger a weapon change (see below).
                weaponMicroBoxIdx = 1;
            }
            else if (weaponMicroBoxIdx > 0) {
                // If above the first slot go to the previous weapon.
                // Keep going back until we find a weapon that is owned - which we should always have (fists/pistol).
                nextWeaponIdx--;

                while ((!player.weaponowned[nextWeaponIdx]) && (nextWeaponIdx > 0)) {
                    nextWeaponIdx--;
                }
            }
        }
        else if (bGotoNextWeapon) {
            // Go to the next weapon: keep incrementing the weapon index until we find one that is owned or hit the last microbox index
            if (weaponMicroBoxIdx < 8) {
                nextWeaponIdx++;

                while ((!player.weaponowned[nextWeaponIdx]) && (nextWeaponIdx < 8)) {
                    nextWeaponIdx++;
                }
            }

            // If we incremented onto the chainsaw (value 8) then go back to the bfg (wraparound is not allowed)
            if (nextWeaponIdx == wp_chainsaw) {
                nextWeaponIdx = wp_bfg;

                // If the bfg is not owned keep going back in the weapon list until we find one that is owned (there will always be something owned)
                while (!player.weaponowned[nextWeaponIdx]) {
                    nextWeaponIdx--;
                }
            }
        }

        // Did we decide to change weapon?
        if (nextWeaponIdx != weaponMicroBoxIdx) {
            // If we are on the fists and triggering a change then toggle between those and the chainsaw
            if ((nextWeaponIdx == wp_fist) && (player.readyweapon != wp_chainsaw)) {
                nextWeaponIdx = (player.weaponowned[wp_chainsaw] != 0) ? wp_chainsaw : wp_fist;
            }

            // Actually change the weapon
            player.pendingweapon = (weapontype_t) nextWeaponIdx;
        }
    }

    // Updates for when the game is NOT paused
    if (!gbGamePaused) {
        // Do physical movements due to velocity and state transitions for the player.
        // PC-PSX: do this AFTER gathering inputs to reduce input lag, except in the case of demos (for compatibility).
        #if PC_PSX_DOOM_MODS
            const bool bUseOrigMovement = (gbDemoPlayback || gbDemoRecording);
        #else
            const bool bUseOrigMovement = false;
        #endif

        mobj_t& playerMobj = *player.mo;

        // Originally the player was physically moved BEFORE gathering inputs
        if (bUseOrigMovement) {
            P_PlayerMobjThink(playerMobj);
        }

        // Gather inputs for the next move
        P_BuildMove(player);

        // Is the player dead? If so do death updates, otherwise do all the normal updates
        if (player.playerstate == PST_DEAD) {
            P_DeathThink(player);

            // PC-PSX: just being consistent with the 'alive' case if we are using tweaked movement
            if (!bUseOrigMovement) {
                P_PlayerMobjThink(playerMobj);
            }
        } else {
            // Attacking with the chainsaw causes some movement forward
            if (playerMobj.flags & MF_JUSTATTACKED) {
                player.angleturn = 0;
                player.forwardmove = FRACUNIT * 25 / 32;
                player.sidemove = 0;
                playerMobj.flags &= ~MF_JUSTATTACKED;
            }

            // Apply thrust unless delayed after exiting a teleporter etc.
            if (playerMobj.reactiontime == 0) {
                P_MovePlayer(player);
            } else {
                playerMobj.reactiontime--;
            }

            // PC-PSX: actually move the player at this point, AFTER inputs
            if (!bUseOrigMovement) {
                P_PlayerMobjThink(playerMobj);
            }

            // Adjust view height
            P_CalcHeight(player);

            // Do sector specials: lava and slime etc. hurting the player
            if (playerMobj.subsector->sector->special) {
                P_PlayerInSpecialSector(player);

                if (player.playerstate == PST_DEAD)
                    return;
            }

            // Use special lines if the use button is pressed
            if (curBtns & pBtnBindings[cbind_use]) {
                if (!player.usedown) {
                    P_UseLines(player);
                    player.usedown = true;
                }
            } else {
                player.usedown = false;
            }

            // Go into the attack state and update the status bar for this player if fire is pressed for a long time on certain weapons
            if (curBtns & pBtnBindings[cbind_attack]) {
                P_SetMObjState(playerMobj, S_PLAY_ATK1);
                player.attackdown++;

                // Should we do the grimmace face after fire has been pressed a long time?
                const bool bDoGrimmaceFace = (
                    (gPlayerNum == gCurPlayerIndex) &&
                    (player.attackdown > TICRATE * 2) &&
                    ((player.readyweapon == wp_chaingun) || (player.readyweapon == wp_plasma))
                );

                if (bDoGrimmaceFace) {
                    gStatusBar.specialFace = f_mowdown;
                }
            } else {
                // Not attacking this tic: reset the count for number of tics attacking
                player.attackdown = 0;
            }

            // Update the player's weapon sprites
            P_MovePsprites(player);

            // Tic special powers if enough time has elapsed and reduce their duration.
            if (gGameTic > gPrevGameTic) {
                // Note: berserk is special and counts up to diminish the red hue effect after picking it up.
                // It never runs out (theoretically it CAN if there is an overflow) unlike other powers...
                if (player.powers[pw_strength]) {
                    player.powers[pw_strength]++;
                }

                if (player.powers[pw_invulnerability] != 0) {
                    player.powers[pw_invulnerability]--;
                }

                if (player.powers[pw_invisibility]) {
                    player.powers[pw_invisibility]--;

                    // Remove the invisibility blend mask if it's out
                    if (player.powers[pw_invisibility] == 0) {
                        playerMobj.flags &= ~MF_BLEND_ADD_25;
                    } else {
                        // If we are getting low on invisibility (4 seconds or less) then blink it out:
                        if (player.powers[pw_invisibility] <= TICRATE * 4) {
                            if ((player.powers[pw_invisibility] & 7) == 0) {
                                playerMobj.flags ^= MF_BLEND_ADD_25;
                            }
                        }
                    }
                }

                if (player.powers[pw_infrared]) {
                    player.powers[pw_infrared]--;
                }

                if (player.powers[pw_ironfeet]) {
                    player.powers[pw_ironfeet]--;
                }

                // Reduce damage and bonus tints
                if (player.damagecount != 0) {
                    player.damagecount--;
                }

                if (player.bonuscount != 0) {
                    player.bonuscount--;
                }
            }
        }
    }
}

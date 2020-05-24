#include "p_user.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_slide.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PcPsx/Utils.h"
#include "PsxVm/PsxVm.h"
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
static constexpr int32_t TURN_ACCEL_TICS        = C_ARRAY_SIZE(ANGLE_TURN);     // Number of tics/stages in turn acceleration before it hits max speed
static constexpr int32_t TURN_TO_ANGLE_SHIFT    = 17;                           // How many bits to shift the turn amount left to scale it to an angle
static constexpr fixed_t MAXBOB                 = 16 * FRACUNIT;                // Maximum amount of view bobbing per frame (16 pixels)

static const VmPtr<bool32_t>    gbOnGround(0x800781CC);     // Flag set to true when the player is on the ground

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to move the given player map object according to it's current velocity.
// Also crosses special lines in the process, and interacts with special things touched.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PlayerMove(mobj_t& mobj) noexcept {
    // This is the amount to be moved
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[*gPlayerNum];
    fixed_t moveDx = (mobj.momx >> 2) * elapsedVBlanks;
    fixed_t moveDy = (mobj.momy >> 2) * elapsedVBlanks;

    // Do the sliding movement
    *gpSlideThing = &mobj;
    P_SlideMove();
    line_t* const pSpecialLineToCross = gpSpecialLine->get();

    // If we did not succeed in doing a sliding movement, or cannot go to the suggested location then try stairstepping.
    // With stairstepping we try to do the x and y axis movements alone, instead of together:
    const bool bSlideMoveBlocked = ((*gSlideX == mobj.x) && (*gSlideY == mobj.y));

    if (bSlideMoveBlocked || (!P_TryMove(mobj, *gSlideX, *gSlideY))) {
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
void P_PlayerXYMovement(mobj_t& mobj) noexcept {
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
void P_PlayerZMovement(mobj_t& mobj) noexcept {
    player_t& player = *mobj.player;

    // Do smooth stepping up a step
    if (mobj.z < mobj.floorz) {
        // TODO: comment on exactly what this is doing
        player.viewheight -= mobj.floorz - mobj.z;
        player.deltaviewheight = (VIEWHEIGHT - player.viewheight) >> 2;
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
void P_PlayerMobjThink(mobj_t& mobj) noexcept {
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
void P_BuildMove(player_t& player) noexcept {
    // Grab some useful stuff: elapsed vblanks, currend and old buttons and gamepad bindings
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[*gPlayerNum];
    const uint32_t curBtns = gTicButtons[*gPlayerNum];
    const uint32_t oldBtns = gOldTicButtons[*gPlayerNum];

    const padbuttons_t* const pBtnBindings = gpPlayerCtrlBindings[*gPlayerNum].get();
    
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

        if ((speedMode != 0) && ((curBtns & (PAD_UP | PAD_DOWN)) == 0)) {
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

    // Do forward/backward movement controls
    if (curBtns & PAD_UP) {
        player.forwardmove = (+FORWARD_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
    }
    else if (curBtns & PAD_DOWN) {
        player.forwardmove = (-FORWARD_MOVE[speedMode] * elapsedVBlanks) / VBLANKS_PER_TIC;
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
void P_Thrust(player_t& player, const angle_t angle, const fixed_t amount) noexcept {
    mobj_t& mobj = *player.mo;
    mobj.momx += (amount >> 8) * (gFineCosine[angle >> ANGLETOFINESHIFT] >> 8);
    mobj.momy += (amount >> 8) * (gFineSine[angle >> ANGLETOFINESHIFT] >> 8);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the player's current view height (for rendering) and does smooth stepping up stairs.
// Also applies view bobbing to the view height.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CalcHeight(player_t& player) noexcept {
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

    if (!*gbOnGround) {
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

    const uint32_t bobPhase = (*gTicCon * BOB_PHASE_STEP) & FINEMASK;
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
void P_MovePlayer(player_t& player) noexcept {
    // Apply angle turning
    mobj_t& mobj = *player.mo;
    mobj.angle += player.angleturn;

    // Save whether we are on the ground
    *gbOnGround = (mobj.z <= mobj.floorz);
    
    // Apply side and forward/backward movement velocity
    if ((player.forwardmove != 0) && *gbOnGround) {
        P_Thrust(player, mobj.angle, player.forwardmove);
    }

    if ((player.sidemove != 0) && *gbOnGround) {
        P_Thrust(player, mobj.angle - ANG90, player.sidemove);
    }

    // Switch to the running animation frames if moving and currently showing the standing still ones
    const bool bIsMoving = ((player.forwardmove != 0) || (player.sidemove != 0));

    if (bIsMoving && (mobj.state.get() == &gStates[S_PLAY])) {
        P_SetMObjState(mobj, S_PLAY_RUN1);
    }
}

void P_DeathThink() noexcept {
loc_8002A6A0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    P_MovePsprites(*vmAddrToPtr<player_t>(a0));
    v1 = lw(s0 + 0x18);
    v0 = 0x80000;                                       // Result = 00080000
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFFF0000;                                // Result = FFFF0000
        if (bJump) goto loc_8002A6D0;
    }
    v0 += v1;
    sw(v0, s0 + 0x18);
loc_8002A6D0:
    v0 = lw(s0);
    v1 = lw(v0 + 0x8);
    v0 = lw(v0 + 0x38);
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    sw(v0, gp + 0xBEC);                                 // Store to: gbOnGround (800781CC)
    a0 = s0;
    P_CalcHeight(*vmAddrToPtr<player_t>(a0));
    v1 = lw(s0 + 0xE0);
    if (v1 == 0) goto loc_8002A78C;
    v0 = lw(s0);
    if (v1 == v0) goto loc_8002A78C;
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    t0 = 0xFC710000;                                    // Result = FC710000
    t0 |= 0xC71D;                                       // Result = FC71C71D
    v1 = 0xF8E30000;                                    // Result = F8E30000
    a1 = lw(s0);
    v1 |= 0x8E3A;                                       // Result = F8E38E3A
    a0 = lw(a1 + 0x24);
    a3 = v0;
    a2 = v0 - a0;
    v0 = a2 + t0;
    v1 = (v1 < v0);
    if (v1 == 0) goto loc_8002A768;
    sw(a3, a1 + 0x24);
    goto loc_8002A78C;
loc_8002A768:
    v0 = 0x38E0000;                                     // Result = 038E0000
    if (i32(a2) < 0) goto loc_8002A780;
    v0 |= 0x38E3;                                       // Result = 038E38E3
    v0 += a0;
    sw(v0, a1 + 0x24);
    goto loc_8002A7A0;
loc_8002A780:
    v0 = a0 + t0;
    sw(v0, a1 + 0x24);
    goto loc_8002A7A0;
loc_8002A78C:
    v0 = lw(s0 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002A7A0;
    }
    sw(v0, s0 + 0xD8);
loc_8002A7A0:
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    v0 = lbu(at);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x80000;                                   // Result = 00080000
        if (bJump) goto loc_8002A7E4;
    }
    v1 = lw(s0 + 0x18);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8002A7E4;
    }
    sw(v0, s0 + 0x4);
loc_8002A7E4:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_PlayerThink() noexcept {
loc_8002A7F8:
    v0 = *gPlayerNum;
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s0 + 0x4);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    s2 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    a1 = lw(at);
    at = ptrToVmAddr(&gpPlayerCtrlBindings[0]);
    at += v0;
    s1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    if (v1 != 0) goto loc_8002AA14;
    a0 = lw(s0 + 0x70);
    {
        const bool bJump = (a0 != v0);
        v0 = a0 << 2;
        if (bJump) goto loc_8002A874;
    }
    a0 = lw(s0 + 0x6C);
    v0 = a0 << 2;
loc_8002A874:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x408C;                                       // Result = gWeaponMicroIndexes[0] (8007408C)
    at += v0;
    a0 = lw(at);
    v1 = lw(s1 + 0x18);
    v0 = s2 & v1;
    a2 = a0;
    if (v0 == 0) goto loc_8002A924;
    v0 = a1 & v1;
    if (v0 != 0) goto loc_8002A924;
    if (a0 != 0) goto loc_8002A8C4;
    v0 = lw(s0 + 0x94);
    if (v0 == 0) goto loc_8002A8C4;
    a2 = 1;                                             // Result = 00000001
    goto loc_8002A9E0;
loc_8002A8C4:
    if (i32(a0) <= 0) goto loc_8002A9E0;
    a0--;
    v0 = a0 << 2;
    v0 += s0;
    v0 = lw(v0 + 0x74);
    if (v0 != 0) goto loc_8002A9E0;
    if (i32(a0) <= 0) goto loc_8002A9E0;
    a0--;
    v0 = a0 << 2;
    v1 = v0 + s0;
    v0 = lw(v1 + 0x74);
loc_8002A900:
    if (v0 != 0) goto loc_8002A9E0;
    if (i32(a0) <= 0) goto loc_8002A9E0;
    v1 -= 4;
    v0 = lw(v1 + 0x74);
    a0--;
    goto loc_8002A900;
loc_8002A924:
    v1 = lw(s1 + 0x1C);
    v0 = s2 & v1;
    {
        const bool bJump = (v0 == 0);
        v0 = a1 & v1;
        if (bJump) goto loc_8002A9E0;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < 8);
        if (bJump) goto loc_8002A9E0;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    a0++;
    v0 = a0 << 2;
    v0 += s0;
    v0 = lw(v0 + 0x74);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    v0 = (i32(a0) < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    a0++;
    v0 = a0 << 2;
    v1 = v0 + s0;
    v0 = lw(v1 + 0x74);
    {
        const bool bJump = (v0 != 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
loc_8002A98C:
    v0 = (i32(a0) < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8002A9B0;
    }
    v1 += 4;
    v0 = lw(v1 + 0x74);
    a0++;
    if (v0 == 0) goto loc_8002A98C;
    v0 = 8;                                             // Result = 00000008
loc_8002A9B0:
    if (a0 != v0) goto loc_8002A9E0;
    v0 = lw(s0 + 0x90);
    a0 = 7;                                             // Result = 00000007
    if (v0 != 0) goto loc_8002A9E0;
    v1 = s0 + 0x1C;
loc_8002A9CC:
    v1 -= 4;
    v0 = lw(v1 + 0x74);
    a0--;
    if (v0 == 0) goto loc_8002A9CC;
loc_8002A9E0:
    if (a0 == a2) goto loc_8002AA14;
    v0 = 8;                                             // Result = 00000008
    if (a0 != 0) goto loc_8002AA10;
    v1 = lw(s0 + 0x6C);
    if (v1 == v0) goto loc_8002AA10;
    v0 = lw(s0 + 0x94);
    v0 = (v0 > 0);
    a0 = v0 << 3;
loc_8002AA10:
    sw(a0, s0 + 0x70);
loc_8002AA14:
    v0 = *gbGamePaused;
    if (v0 != 0) goto loc_8002ACCC;
    a0 = lw(s0);
    P_PlayerMobjThink(*vmAddrToPtr<mobj_t>(a0));
    a0 = s0;
    P_BuildMove(*vmAddrToPtr<player_t>(a0));
    v1 = lw(s0 + 0x4);
    v0 = 1;                                             // Result = 00000001
    if (v1 != v0) goto loc_8002AA5C;
    a0 = s0;
    P_DeathThink();
    goto loc_8002ACCC;
loc_8002AA5C:
    v0 = lw(s0);
    v0 = lw(v0 + 0x64);
    v0 &= 0x80;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xC800;                                    // Result = 0000C800
        if (bJump) goto loc_8002AA98;
    }
    a0 = lw(s0);
    sw(0, s0 + 0x10);
    sw(v0, s0 + 0x8);
    sw(0, s0 + 0xC);
    v0 = lw(a0 + 0x64);
    v1 = -0x81;                                         // Result = FFFFFF7F
    v0 &= v1;
    sw(v0, a0 + 0x64);
loc_8002AA98:
    v1 = lw(s0);
    v0 = lw(v1 + 0x78);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AAB8;
    }
    sw(v0, v1 + 0x78);
    goto loc_8002AAC0;
loc_8002AAB8:
    a0 = s0;
    P_MovePlayer(*vmAddrToPtr<player_t>(a0));
loc_8002AAC0:
    a0 = s0;
    P_CalcHeight(*vmAddrToPtr<player_t>(a0));
    v0 = lw(s0);
    v0 = lw(v0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    if (v0 == 0) goto loc_8002AB08;
    a0 = s0;
    P_PlayerInSpecialSector(*vmAddrToPtr<player_t>(a0));
    v1 = lw(s0 + 0x4);
    v0 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_8002ACCC;
loc_8002AB08:
    v0 = lw(s1 + 0x4);
    v0 &= s2;
    if (v0 == 0) goto loc_8002AB40;
    v0 = lw(s0 + 0xBC);
    if (v0 != 0) goto loc_8002AB44;
    a0 = s0;
    P_UseLines(*vmAddrToPtr<player_t>(a0));
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0xBC);
    goto loc_8002AB44;
loc_8002AB40:
    sw(0, s0 + 0xBC);
loc_8002AB44:
    v0 = lw(s1);
    v0 &= s2;
    if (v0 == 0) goto loc_8002ABC8;
    a0 = lw(s0);
    a1 = 0x9F;                                          // Result = 0000009F
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    v0 = lw(s0 + 0xB8);
    v0++;
    sw(v0, s0 + 0xB8);
    v0 = (i32(v0) < 0x1F);
    if (v0 != 0) goto loc_8002ABCC;
    v1 = *gPlayerNum;
    v0 = *gCurPlayerIndex;
    {
        const bool bJump = (v1 != v0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8002ABCC;
    }
    v1 = lw(s0 + 0x6C);
    {
        const bool bJump = (v1 == v0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8002ABB4;
    }
    if (v1 != v0) goto loc_8002ABCC;
loc_8002ABB4:
    v0 = 7;                                             // Result = 00000007
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
    goto loc_8002ABCC;
loc_8002ABC8:
    sw(0, s0 + 0xB8);
loc_8002ABCC:
    a0 = s0;
    P_MovePsprites(*vmAddrToPtr<player_t>(a0));
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002ACCC;
    v0 = lw(s0 + 0x34);
    {
        const bool bJump = (v0 == 0);
        v0++;
        if (bJump) goto loc_8002AC08;
    }
    sw(v0, s0 + 0x34);
loc_8002AC08:
    v0 = lw(s0 + 0x30);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AC1C;
    }
    sw(v0, s0 + 0x30);
loc_8002AC1C:
    v0 = lw(s0 + 0x38);
    v1 = v0 - 1;
    if (v0 == 0) goto loc_8002AC7C;
    sw(v1, s0 + 0x38);
    if (v1 != 0) goto loc_8002AC50;
    a0 = lw(s0);
    v1 = 0x8FFF0000;                                    // Result = 8FFF0000
    v0 = lw(a0 + 0x64);
    v1 |= 0xFFFF;                                       // Result = 8FFFFFFF
    v0 &= v1;
    sw(v0, a0 + 0x64);
    goto loc_8002AC7C;
loc_8002AC50:
    v0 = (i32(v1) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 7;
        if (bJump) goto loc_8002AC7C;
    }
    a0 = 0x70000000;                                    // Result = 70000000
    if (v0 != 0) goto loc_8002AC7C;
    v0 = lw(s0);
    v1 = lw(v0 + 0x64);
    v1 ^= a0;
    sw(v1, v0 + 0x64);
loc_8002AC7C:
    v0 = lw(s0 + 0x44);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002AC90;
    }
    sw(v0, s0 + 0x44);
loc_8002AC90:
    v0 = lw(s0 + 0x3C);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACA4;
    }
    sw(v0, s0 + 0x3C);
loc_8002ACA4:
    v0 = lw(s0 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACB8;
    }
    sw(v0, s0 + 0xD8);
loc_8002ACB8:
    v0 = lw(s0 + 0xDC);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002ACCC;
    }
    sw(v0, s0 + 0xDC);
loc_8002ACCC:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

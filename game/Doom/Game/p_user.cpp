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
#include "PsxVm/PsxVm.h"
#include <algorithm>
#include <cmath>

static constexpr fixed_t STOPSPEED  = FRACUNIT / 16;    // Stop the player completely if velocity falls under this amount
static constexpr fixed_t FRICTION   = 0xd200;           // Friction amount to apply to player movement: approximately 0.82

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to move the given player map object according to it's current velocity.
// Also crosses special lines in the process, and interacts with special things touched.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PlayerMove(mobj_t& mobj) noexcept {
    // This is the amount to be moved
    fixed_t moveDx = (mobj.momx >> 2) * gPlayersElapsedVBlanks[*gPlayerNum];
    fixed_t moveDy = (mobj.momy >> 2) * gPlayersElapsedVBlanks[*gPlayerNum];

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

void P_BuildMove() noexcept {
loc_80029DD4:
    sp -= 0x18;
    v0 = *gPlayerNum;
    a1 = a0;
    sw(ra, sp + 0x10);
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerCtrlBindings[0]);
    at += v0;
    t0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    a2 = lw(at);
    v1 = lw(t0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    a0 = lw(at);
    a3 = a2 & v1;
    v1 = a2 & 0x8000;
    a3 = (i32(a3) > 0);
    if (v1 == 0) goto loc_80029E3C;
    v0 = a0 & 0x8000;
    if (v0 != 0) goto loc_80029E50;
loc_80029E3C:
    v0 = a2 & 0x2000;
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 0x2000;
        if (bJump) goto loc_80029E64;
    }
    if (v0 == 0) goto loc_80029E64;
loc_80029E50:
    v0 = lw(a1 + 0x128);
    v0++;
    sw(v0, a1 + 0x128);
    goto loc_80029E68;
loc_80029E64:
    sw(0, a1 + 0x128);
loc_80029E68:
    v0 = lw(a1 + 0x128);
    v0 = (i32(v0) < 0xA);
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80029E80;
    }
    sw(v0, a1 + 0x128);
loc_80029E80:
    sw(0, a1 + 0x10);
    sw(0, a1 + 0xC);
    sw(0, a1 + 0x8);
    v0 = lw(t0 + 0x10);
    v0 &= a2;
    v1 = a3 << 2;
    if (v0 == 0) goto loc_80029EDC;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
    goto loc_80029F28;
loc_80029EDC:
    v0 = lw(t0 + 0x14);
    v0 &= a2;
    if (v0 == 0) goto loc_80029F40;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
loc_80029F28:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80029F38;
    v0 += 3;
loc_80029F38:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0xC);
loc_80029F40:
    v0 = lw(t0 + 0x8);
    v0 &= a2;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x8000;
        if (bJump) goto loc_80029FF8;
    }
    v1 = a3 << 2;
    if (v0 == 0) goto loc_80029F98;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
    goto loc_80029FDC;
loc_80029F98:
    v0 = a2 & 0x2000;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7840;                                       // Result = SideMove[0] (80067840)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
loc_80029FDC:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80029FEC;
    v0 += 3;
loc_80029FEC:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0xC);
    goto loc_8002A18C;
loc_80029FF8:
    v0 = a2 & 0x5000;
    if (a3 == 0) goto loc_8002A0C8;
    {
        const bool bJump = (v0 != 0);
        v0 = a2 & 0x8000;
        if (bJump) goto loc_8002A0CC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x2000;
        if (bJump) goto loc_8002A064;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7870;                                       // Result = FastAngleTurn[0] (80067870)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A05C;
    v0 += 3;
loc_8002A05C:
    v0 = u32(i32(v0) >> 2);
    goto loc_8002A184;
loc_8002A064:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7870;                                       // Result = FastAngleTurn[0] (80067870)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A0B8;
    v0 += 3;
loc_8002A0B8:
    v0 = u32(i32(v0) >> 2);
    v0 <<= 17;
    v0 = -v0;
    goto loc_8002A188;
loc_8002A0C8:
    v0 = a2 & 0x8000;
loc_8002A0CC:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x2000;
        if (bJump) goto loc_8002A128;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7848;                                       // Result = AngleTurn[0] (80067848)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A120;
    v0 += 3;
loc_8002A120:
    v0 = u32(i32(v0) >> 2);
    goto loc_8002A184;
loc_8002A128:
    {
        const bool bJump = (v0 == 0);
        v0 = a2 & 0x1000;
        if (bJump) goto loc_8002A190;
    }
    v0 = *gPlayerNum;
    v1 = lw(a1 + 0x128);
    v0 <<= 2;
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7848;                                       // Result = AngleTurn[0] (80067848)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A17C;
    v0 += 3;
loc_8002A17C:
    v0 = u32(i32(v0) >> 2);
    v0 = -v0;
loc_8002A184:
    v0 <<= 17;
loc_8002A188:
    sw(v0, a1 + 0x10);
loc_8002A18C:
    v0 = a2 & 0x1000;
loc_8002A190:
    v1 = a3 << 2;
    if (v0 == 0) goto loc_8002A1D0;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    a0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7838;                                       // Result = ForwardMove[0] (80067838)
    at += v1;
    v0 = lw(at);
    mult(a0, v0);
    goto loc_8002A214;
loc_8002A1D0:
    v0 = a2 & 0x4000;
    if (v0 == 0) goto loc_8002A22C;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7838;                                       // Result = ForwardMove[0] (80067838)
    at += v1;
    v1 = lw(at);
    v0 = -v0;
    mult(v0, v1);
loc_8002A214:
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8002A224;
    v0 += 3;
loc_8002A224:
    v0 = u32(i32(v0) >> 2);
    sw(v0, a1 + 0x8);
loc_8002A22C:
    a0 = lw(a1);
    v0 = lw(a0 + 0x48);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a0 + 0x4C);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a1 + 0x8);
    if (v0 != 0) goto loc_8002A2A8;
    v0 = lw(a1 + 0xC);
    if (v0 != 0) goto loc_8002A2A8;
    v1 = lw(a0 + 0x60);
    a1 = 0x80060000;                                    // Result = 80060000
    a1 -= 0x6180;                                       // Result = State_S_PLAY_RUN1[0] (80059E80)
    v0 = a1 + 0x1C;                                     // Result = State_S_PLAY_RUN2[0] (80059E9C)
    if (v1 == a1) goto loc_8002A2A0;
    {
        const bool bJump = (v1 == v0);
        v0 = a1 + 0x38;                                 // Result = State_S_PLAY_RUN3[0] (80059EB8)
        if (bJump) goto loc_8002A2A0;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = a1 + 0x54;                                 // Result = State_S_PLAY_RUN4[0] (80059ED4)
        if (bJump) goto loc_8002A2A0;
    }
    if (v1 != v0) goto loc_8002A2A8;
loc_8002A2A0:
    a1 = 0x9A;                                          // Result = 0000009A
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8002A2A8:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_Thrust() noexcept {
    a1 >>= 19;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a1 <<= 2;
    v0 += a1;
    v0 = lw(v0);
    a2 = u32(i32(a2) >> 8);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a3 = lw(a0);
    v1 = lw(a3 + 0x48);
    v0 = lo;
    v0 += v1;
    sw(v0, a3 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a0);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
    return;
}

void P_CalcHeight() noexcept {
loc_8002A32C:
    a1 = a0;
    v0 = lw(a1);
    v0 = lw(v0 + 0x48);
    v0 = u32(i32(v0) >> 8);
    mult(v0, v0);
    v0 = lw(a1);
    v1 = lo;
    sw(v1, a1 + 0x20);
    v0 = lw(v0 + 0x4C);
    v0 = u32(i32(v0) >> 8);
    mult(v0, v0);
    v0 = lo;
    v0 += v1;
    v0 = u32(i32(v0) >> 4);
    v1 = 0x100000;                                      // Result = 00100000
    sw(v0, a1 + 0x20);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_8002A388;
    sw(v1, a1 + 0x20);
loc_8002A388:
    v0 = lw(gp + 0xBEC);                                // Load from: gbOnGround (800781CC)
    a0 = 0x290000;                                      // Result = 00290000
    if (v0 != 0) goto loc_8002A3D0;
    v0 = lw(a1);
    v1 = lw(v0 + 0x8);
    v0 = lw(a1);
    v1 += a0;
    sw(v1, a1 + 0x14);
    a0 = lw(v0 + 0x3C);
    v0 = 0xFFFC0000;                                    // Result = FFFC0000
    a0 += v0;
    v1 = (i32(a0) < i32(v1));
    if (v1 == 0) goto loc_8002A4E0;
    sw(a0, a1 + 0x14);
    goto loc_8002A4E0;
loc_8002A3D0:
    v1 = *gTicCon;
    v0 = v1 << 1;
    v0 += v1;
    v1 = v0 << 4;
    v0 += v1;
    v0 <<= 4;
    v0 &= 0x7FF0;
    v1 = lw(a1 + 0x20);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    v0 = lw(at);
    v1 = u32(i32(v1) >> 17);
    mult(v1, v0);
    v0 = lw(a1 + 0x4);
    a0 = lo;
    if (v0 != 0) goto loc_8002A4A4;
    v0 = lw(a1 + 0x18);
    v1 = lw(a1 + 0x1C);
    v0 += v1;
    v1 = 0x290000;                                      // Result = 00290000
    sw(v0, a1 + 0x18);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x140000;                                  // Result = 00140000
        if (bJump) goto loc_8002A44C;
    }
    sw(v1, a1 + 0x18);
    sw(0, a1 + 0x1C);
loc_8002A44C:
    v1 = lw(a1 + 0x18);
    v0 |= 0x7FFF;                                       // Result = 00147FFF
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 0x140000;                                  // Result = 00140000
        if (bJump) goto loc_8002A478;
    }
    v1 = lw(a1 + 0x1C);
    v0 |= 0x8000;                                       // Result = 00148000
    sw(v0, a1 + 0x18);
    if (i32(v1) > 0) goto loc_8002A488;
    v0 = 1;                                             // Result = 00000001
    sw(v0, a1 + 0x1C);
loc_8002A478:
    v0 = lw(a1 + 0x1C);
    if (v0 == 0) goto loc_8002A4A4;
loc_8002A488:
    v0 = lw(a1 + 0x1C);
    v1 = 0x8000;                                        // Result = 00008000
    v0 += v1;
    sw(v0, a1 + 0x1C);
    if (v0 != 0) goto loc_8002A4A4;
    v0 = 1;                                             // Result = 00000001
    sw(v0, a1 + 0x1C);
loc_8002A4A4:
    v0 = lw(a1);
    v1 = lw(a1 + 0x18);
    v0 = lw(v0 + 0x8);
    v0 += v1;
    v1 = lw(a1);
    v0 += a0;
    sw(v0, a1 + 0x14);
    a0 = lw(v1 + 0x3C);
    v1 = 0xFFFC0000;                                    // Result = FFFC0000
    a0 += v1;
    v0 = (i32(a0) < i32(v0));
    if (v0 == 0) goto loc_8002A4E0;
    sw(a0, a1 + 0x14);
loc_8002A4E0:
    return;
}

void P_MovePlayer() noexcept {
loc_8002A4E8:
    sp -= 0x18;
    a3 = a0;
    sw(ra, sp + 0x10);
    a0 = lw(a3);
    v1 = lw(a3 + 0x10);
    v0 = lw(a0 + 0x24);
    v0 += v1;
    sw(v0, a0 + 0x24);
    v0 = lw(a3);
    v1 = lw(v0 + 0x8);
    v0 = lw(v0 + 0x38);
    a2 = lw(a3 + 0x8);
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    sw(v0, gp + 0xBEC);                                 // Store to: gbOnGround (800781CC)
    if (a2 == 0) goto loc_8002A5B0;
    a2 = u32(i32(a2) >> 8);
    if (v0 == 0) goto loc_8002A5B0;
    a1 = lw(a3);
    v1 = lw(a1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 >>= 19;
    v1 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a1 + 0x48);
    v0 = lo;
    v0 += a0;
    sw(v0, a1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a3);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
loc_8002A5B0:
    a2 = lw(a3 + 0xC);
    if (a2 == 0) goto loc_8002A64C;
    v0 = lw(gp + 0xBEC);                                // Load from: gbOnGround (800781CC)
    {
        const bool bJump = (v0 == 0);
        v0 = 0xC0000000;                                // Result = C0000000
        if (bJump) goto loc_8002A64C;
    }
    a1 = lw(a3);
    v1 = lw(a1 + 0x24);
    v1 += v0;
    v1 >>= 19;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    a2 = u32(i32(a2) >> 8);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a1 + 0x48);
    v0 = lo;
    v0 += a0;
    sw(v0, a1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    v0 = u32(i32(v0) >> 8);
    mult(a2, v0);
    a0 = lw(a3);
    v1 = lw(a0 + 0x4C);
    v0 = lo;
    v0 += v1;
    sw(v0, a0 + 0x4C);
loc_8002A64C:
    v0 = lw(a3 + 0x8);
    if (v0 != 0) goto loc_8002A66C;
    v0 = lw(a3 + 0xC);
    if (v0 == 0) goto loc_8002A690;
loc_8002A66C:
    a0 = lw(a3);
    v1 = lw(a0 + 0x60);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x619C;                                       // Result = State_S_PLAY[0] (80059E64)
    if (v1 != v0) goto loc_8002A690;
    a1 = 0x9B;                                          // Result = 0000009B
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8002A690:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
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
    P_CalcHeight();
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
    P_BuildMove();
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
    P_MovePlayer();
loc_8002AAC0:
    a0 = s0;
    P_CalcHeight();
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

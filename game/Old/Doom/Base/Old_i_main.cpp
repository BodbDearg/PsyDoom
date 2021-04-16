#include "Doom/Base/i_main.h"

#if !PSYDOOM_MODS

constexpr int32_t   NET_PACKET_SIZE     = 8;        // The size of a packet and the packet buffers in a networked game
constexpr uint8_t   NET_PACKET_HEADER   = 0xAA;     // The 1st byte in every network packet: used for error detection purposes

// Pointer to control bindings for player 1 and 2
padbuttons_t* gpPlayerCtrlBindings[MAXPLAYERS];

// Control bindings for the remote player
static padbuttons_t gOtherPlayerCtrlBindings[NUM_CTRL_BINDS];

// PSX Kernel events that fire when reads and writes complete for Serial I/O in a multiplayer game
static uint32_t gSioReadDoneEvent;
static uint32_t gSioWriteDoneEvent;

// File descriptors for the input/output streams used for multiplayer games.
// These are opened against the Serial I/O device (PlayStation Link Cable).
static int32_t gNetInputFd;
static int32_t gNetOutputFd;

//------------------------------------------------------------------------------------------------------------------------------------------
// Submits any pending draw primitives in the gpu commands buffer to the GPU
//------------------------------------------------------------------------------------------------------------------------------------------
void I_SubmitGpuCmds() noexcept {
    // Submit the primitives list to the GPU if it's not empty
    if (gpGpuPrimsBeg != gpGpuPrimsEnd) {
        // Note: this marks the end of the primitive list, by setting the 'tag' field of an invalid primitive to 0xFFFFFF.
        // This is similar to LIBGPU_TermPrim, except we don't bother using a valid primitive struct.
        ((uint32_t*) gpGpuPrimsEnd)[0] = 0x00FFFFFF;
        LIBGPU_DrawOTag(gpGpuPrimsBeg, gGpuCmdsBuffer);
    }

    // Clear the primitives list
    gpGpuPrimsBeg = gGpuCmdsBuffer;
    gpGpuPrimsEnd = gGpuCmdsBuffer;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the setup for a network game: synchronizes between players, then sends the game details and control bindings
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetSetup() noexcept {
    // Set the output packet header
    gNetOutputPacket[0] = NET_PACKET_HEADER;

    // Player number determination: if the current PlayStation is first in the game (NOT cleared to send) then it becomes Player 1.
    const bool bIsPlayer1 = (!LIBCOMB_CombCTS());

    // Read and write a dummy 8 bytes between the two players.
    // Allow the player to abort with the select button also, if a network game is no longer desired.
    if (bIsPlayer1) {
        // Player 1 waits to read from Player 2 firstly
        gCurPlayerIndex = 0;
        LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);

        // Wait until the read is done before continuing, or abort if 'select' is pressed
        do {
            if (LIBETC_PadRead(0) & PAD_SELECT) {
                gbDidAbortGame = true;
                LIBCOMB_CombCancelRead();
                return;
            }
        } while (!LIBAPI_TestEvent(gSioReadDoneEvent));

        // Wait until we are cleared to send to the receiver
        while (!LIBCOMB_CombCTS()) {}

        // Send the dummy packet to the client
        LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);
    } else {
        // Player 2 writes a packet to Player 1 firstly
        gCurPlayerIndex = 1;
        LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);
        LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);

        // Wait until the read is done before continuing, or abort if 'select' is pressed
        do {
            if (LIBETC_PadRead(0) & PAD_SELECT) {
                gbDidAbortGame = true;
                LIBCOMB_CombCancelRead();
                return;
            }
        } while (!LIBAPI_TestEvent(gSioReadDoneEvent));
    }

    // Do a synchronization handshake between the players
    I_NetHandshake();

    // Send the game details if player 1, if player 2 then receive them:
    if (gCurPlayerIndex == 0) {
        // Fill in the packet details with game type, skill, map and this player's control bindings
        gNetOutputPacket[1] = (uint8_t) gStartGameType;
        gNetOutputPacket[2] = (uint8_t) gStartSkill;
        gNetOutputPacket[3] = (uint8_t) gStartMapOrEpisode;

        const uint32_t thisPlayerBtns = I_LocalButtonsToNet(gCtrlBindings);
        gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
        gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
        gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
        gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);

        // Wait until cleared to send then send the packet.
        while (!LIBCOMB_CombCTS()) {}
        LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);

        // Read the control bindings for the other player and wait until it is read.
        LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);
        while (!LIBAPI_TestEvent(gSioReadDoneEvent)) {}

        const uint32_t otherPlayerBtns = (
            ((uint32_t) gNetInputPacket[4] << 0) |
            ((uint32_t) gNetInputPacket[5] << 8) |
            ((uint32_t) gNetInputPacket[6] << 16) |
            ((uint32_t) gNetInputPacket[7] << 24)
        );

        // Save the control bindings for both players
        gpPlayerCtrlBindings[0] = gCtrlBindings;
        gpPlayerCtrlBindings[1] = I_NetButtonsToLocal(otherPlayerBtns);
    }
    else {
        // Read the game details and control bindings for the other player and wait until it is read.
        LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);
        while (!LIBAPI_TestEvent(gSioReadDoneEvent)) {}

        // Save the game details and the control bindings
        const uint32_t otherPlayerBtns = (
            ((uint32_t) gNetInputPacket[4] << 0) |
            ((uint32_t) gNetInputPacket[5] << 8) |
            ((uint32_t) gNetInputPacket[6] << 16) |
            ((uint32_t) gNetInputPacket[7] << 24)
        );

        gStartGameType = (gametype_t) gNetInputPacket[1];
        gStartSkill = (skill_t) gNetInputPacket[2];
        gStartMapOrEpisode = gNetInputPacket[3];
        gpPlayerCtrlBindings[0] = I_NetButtonsToLocal(otherPlayerBtns);
        gpPlayerCtrlBindings[1] = gCtrlBindings;

        // For the output packet send the control bindings of this player to the other player
        const uint32_t thisPlayerBtns = I_LocalButtonsToNet(gCtrlBindings);
        gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
        gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
        gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
        gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);

        // Wait until we are cleared to send to the receiver and send the output packet.
        while (!LIBCOMB_CombCTS()) {}
        LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);
    }

    gbDidAbortGame = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs a synchronization handshake between the two PlayStations involved in a networked game over Serial Cable.
// Sends a sequence of expected 8 bytes, and expects to receive the same 8 bytes back.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetHandshake() noexcept {
    // Send the values 0-7 and verify we get the same values back
    uint8_t syncByte = 0;

    while (syncByte < 8) {
        // Send the sync byte and get the other one back
        gNetOutputPacket[0] = syncByte;
        I_NetSendRecv();

        // Is it what we expected? If it isn't then start over, otherwise move onto the next sync byte:
        if (gNetInputPacket[0] == gNetOutputPacket[0]) {
            syncByte++;
        } else {
            syncByte = 0;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sends the packet for the current frame in a networked game and receives the packet from the other player.
// Also does error checking, to make sure that the connection is still OK.
// Returns 'true' if a network error has occurred.
//------------------------------------------------------------------------------------------------------------------------------------------
bool I_NetUpdate() noexcept {
    // The 1st two bytes of the network packet are for error detection, header and position sanity check:
    mobj_t& player1Mobj = *gPlayers[0].mo;
    mobj_t& player2Mobj = *gPlayers[1].mo;

    gNetOutputPacket[0] = NET_PACKET_HEADER;
    gNetOutputPacket[1] = (uint8_t)(player1Mobj.x ^ player1Mobj.y ^ player2Mobj.x ^ player2Mobj.y);

    // Send the elapsed tics for this player and the buttons pressed
    gNetOutputPacket[2] = (uint8_t)(gPlayersElapsedVBlanks[gCurPlayerIndex]);

    const uint32_t thisPlayerBtns = gTicButtons[gCurPlayerIndex];
    gNetOutputPacket[4] = (uint8_t)(thisPlayerBtns >> 0);
    gNetOutputPacket[5] = (uint8_t)(thisPlayerBtns >> 8);
    gNetOutputPacket[6] = (uint8_t)(thisPlayerBtns >> 16);
    gNetOutputPacket[7] = (uint8_t)(thisPlayerBtns >> 24);

    I_NetSendRecv();

    // See if the packet we received from the other player is what we expect.
    // If it isn't then show a 'network error' message:
    const bool bNetworkError = (
        (gNetInputPacket[0] != NET_PACKET_HEADER) ||
        (gNetInputPacket[1] != gNetOutputPacket[1])
    );

    if (bNetworkError) {
        // Uses the current image as the basis for the next frame; copy the presented framebuffer to the drawing framebuffer:
        LIBGPU_DrawSync(0);
        LIBGPU_MoveImage(
            gDispEnvs[gCurDispBufferIdx].disp,
            gDispEnvs[gCurDispBufferIdx ^ 1].disp.x,
            gDispEnvs[gCurDispBufferIdx ^ 1].disp.y
        );

        // Show the 'Network error' plaque
        I_IncDrawnFrameCount();
        I_CacheTex(gTex_NETERR);
        I_DrawSprite(
            gTex_NETERR.texPageId,
            gPaletteClutIds[UIPAL],
            84,
            109,
            gTex_NETERR.texPageCoordX,
            gTex_NETERR.texPageCoordY,
            gTex_NETERR.width,
            gTex_NETERR.height
        );

        I_SubmitGpuCmds();
        I_DrawPresent();

        // Try and do a sync handshake between the players
        I_NetHandshake();

        // Clear the other player's buttons, and this player's previous buttons (not sure why that would matter)
        gTicButtons[1] = 0;
        gOldTicButtons[0] = 0;

        // There was a network error!
        return true;
    }

    // Read and save the buttons for the other player and their elapsed vblank count.
    // Note that the vblank count for player 1 is what determines the speed of the game for both players.
    const uint32_t otherPlayerBtns = (
        ((uint32_t) gNetInputPacket[4] << 0) |
        ((uint32_t) gNetInputPacket[5] << 8) |
        ((uint32_t) gNetInputPacket[6] << 16) |
        ((uint32_t) gNetInputPacket[7] << 24)
    );

    if (gCurPlayerIndex == 0) {
        gTicButtons[1] = otherPlayerBtns;
        gPlayersElapsedVBlanks[1] = gNetInputPacket[2];
    } else {
        gTicButtons[0] = otherPlayerBtns;
        gPlayersElapsedVBlanks[0] = gNetInputPacket[2];
    }

    // No network error occured
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sends and receives a single packet of data between the two players in a networked game (over the serial link cable)
//------------------------------------------------------------------------------------------------------------------------------------------
void I_NetSendRecv() noexcept {
    while (true) {
        // Which of the two players are we doing comms for?
        if (gCurPlayerIndex == 0) {
            // Player 1: start by waiting until we are clear to send
            while (!LIBCOMB_CombCTS()) {}

            // Write the output packet
            LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);

            // Read the input packet and wait until it's done.
            // Timeout after 5 seconds and retry the entire send/receive procedure.
            LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);
            const int32_t startVBlanks = LIBETC_VSync(-1);

            while (true) {
                // If the read is done then we can finish up this round of sending/receiving
                if (LIBAPI_TestEvent(gSioReadDoneEvent))
                    return;

                // Timeout after 5 seconds if the read still isn't done...
                if (LIBETC_VSync(-1) - startVBlanks >= VBLANKS_PER_SEC * 5)
                    break;
            }
        } else {
            // Player 2: start by reading the input packet
            LIBAPI_read(gNetInputFd, gNetInputPacket, NET_PACKET_SIZE);

            // Wait until the input packet is read.
            // Timeout after 5 seconds and retry the entire send/receive procedure.
            const int32_t startVBlanks = LIBETC_VSync(-1);

            while (true) {
                // Is the input packet done yet?
                if (LIBAPI_TestEvent(gSioReadDoneEvent)) {
                    // Yes we read it! Write the output packet and finish up once we are clear to send.
                    while (!LIBCOMB_CombCTS()) {}
                    LIBAPI_write(gNetOutputFd, gNetOutputPacket, NET_PACKET_SIZE);
                    return;
                }

                // Timeout after 5 seconds if the read still isn't done...
                if (LIBETC_VSync(-1) - startVBlanks >= VBLANKS_PER_SEC * 5)
                    break;
            }
        }

        // Clear the error bits before we retry the send again
        LIBCOMB_CombResetError();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Encodes the given control bindings into a 32-bit integer suitable for network/link-cable transmission
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t I_LocalButtonsToNet(const padbuttons_t pCtrlBindings[NUM_CTRL_BINDS]) noexcept {
    // Encode each control binding using 4-bits.
    // Technically it could be done in 3 bits since there are only 8 possible buttons, but 8 x 4-bits will still fit in the 32-bits also.
    uint32_t encodedBindings = 0;

    for (int32_t bindingIdx = 0; bindingIdx < NUM_CTRL_BINDS; ++bindingIdx) {
        // Find out which button this action is bound to
        int32_t buttonIdx = 0;

        for (; buttonIdx < NUM_BINDABLE_BTNS; ++buttonIdx) {
            // Is this the button used for this action?
            if (gBtnMasks[buttonIdx] == pCtrlBindings[bindingIdx])
                break;
        }

        // Encode the button using a 4 bit slot in the 32-bit integer
        encodedBindings |= buttonIdx << (bindingIdx * 4);
    }

    return encodedBindings;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// The reverse operation to 'I_LocalButtonsToNet'.
// Decodes the given 32-bit integer containing control bindings for the other player in the networked game.
// The bindings are saved to the 'gOtherPlayerCtrlBindings' global array and this is also what is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
padbuttons_t* I_NetButtonsToLocal(const uint32_t encodedBindings) noexcept {
    for (int32_t bindingIdx = 0; bindingIdx < NUM_CTRL_BINDS; ++bindingIdx) {
        const uint32_t buttonIdx = (encodedBindings >> (bindingIdx * 4)) & 0xF;     // Note: Vulnerability/overflow: button index CAN be out of bounds!
        gOtherPlayerCtrlBindings[bindingIdx] = gBtnMasks[buttonIdx];
    }

    return gOtherPlayerCtrlBindings;
}

#endif  // !PSYDOOM_MODS

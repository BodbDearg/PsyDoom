#include "Doom/Base/i_main.h"

#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Texture cache related stuff.
// VRAM for PSX Doom is managed as follows:
//
//  (1) There is 1 MiB in total of VRAM split into 16 texture 'pages'.
//  (2) Each texture page is 256x256 when interpreted as 8 bits per pixel data and 64 KiB in size.
//  (3) The first 4 texture pages are reserved for the front and back framebuffer.
//      Each framebuffer is 256x256 pixels (note: some vertical space is unused) but since the data is
//      16 bits per pixel rather than 8 bits, each screen occupies 2 texture pages rather than 1.
//  (4) Following that, 11 pages are free to be used in any way for map textures and sprites.
//      This is what is referred to as the 'texture cache'.
//  (5) 1 texture page at the end appears to be reserved/unused during gameplay.
//      Perhaps it was held off limits in case memory got tight and a quick fix was needed, or kept reserved for debug stuff?
//  (6) Each of the 11 256x256 pages are broken up into a grid of 16x16 cells, where each cell is 16 pixels wide and tall.
//      Each cell stores a pointer to the 'texture_t' occupying the cell, if any.
//  (7) When adding a sprite or texture to the cache, the code will search through the cells for a free space
//      large enough to accomodate the given resource size, rounded up to 16 pixel increments. The search starts after
//      where the last texture was placed and proceeds across and downwards (in that order) to fill the current texture page.
//      If the current texture page has been filled then the algorithm moves around to the next page, potentially wrapping
//      around back to the first.
//  (8) If a request to add a texture to the cache comes across occupied VRAM cells with textures uploaded in the current frame,
//      then a 'Texture Cache Overflow' fatal error is declared. This means that the algorithm has looped around once already and
//      filled the cache entirely in the same frame. Uploading any more textures would overwrite textures needed for current drawing
//      so that this point the renderer gives up and dies. Perhaps a more robust fix for this problem might have been to simply ignore
//      the request, or flush all pending drawing ops and clear the cache - but perhaps those solutions were deemed unacceptable...
//  (9) One other small detail also is that certain texture pages can be marked as 'locked'.
//      These 'locked' pages are left alone when clearing or adding textures to the cache.
//      Map floor and wall textures are placed in 'locked' pages so that they are never unloaded during level gameplay.
//  (10) Lastly it is worth mentioning that since textures in unlocked pages can be evicted at any time, these textures must also be
//       backed up and retained in main RAM. So essentially the renderer needs to keep a copy of all sprite data in main RAM also.
//------------------------------------------------------------------------------------------------------------------------------------------
struct tcachepage_t {
    texture_t* cells[TCACHE_CELLS_Y][TCACHE_CELLS_X];
};

struct tcache_t {
    tcachepage_t pages[NUM_TCACHE_PAGES];
};

// The texture page index in VRAM that the texture cache starts at.
// The first 4 pages are reserved for the framebuffer!
static constexpr uint32_t TCACHE_BASE_PAGE = 4;

// The texture coordinates of each texture cache page in VRAM.
// Notes:
//  (1) Since the PsyQ SDK expects these coordinates to be in terms of a 16-bit texture, X is halved here because Doom's textures are actually 8 bit.
//  (2) The coordinates ignore the first 4 pages in VRAM, since that is used for the framebuffer.
//  (3) The extra 'reserved' or unused texture page that is not used in the cache is also here too, hence 1 extra coordinate.
constexpr uint16_t TEX_PAGE_VRAM_TEXCOORDS[NUM_TCACHE_PAGES + 1][2] = {
    { 512,  0,  },
    { 640,  0,  },
    { 768,  0,  },
    { 896,  0,  },
    { 0,    256 },
    { 128,  256 },
    { 256,  256 },
    { 384,  256 },
    { 512,  256 },
    { 640,  256 },
    { 768,  256 },
    { 896,  256 }
};

// Texture cache variables.
// The texture cache data structure, where we are filling in the cache next and the current fill row height (in cells).
static tcache_t* gpTexCache;

uint32_t    gTCacheFillPage;
uint32_t    gTCacheFillCellX;
uint32_t    gTCacheFillCellY;
uint32_t    gTCacheFillRowCellH;

// Texture cache: a bit mask of which texture pages (past the initial 4 which are reserved for the framebuffer) are 'locked' during gameplay.
// Locked texture pages are not allowed to be unloaded and are used for UI sprites, wall and floor textures.
// Sprites on the other hand are placed in 'unlocked' texture pages and can be evicted at any time.
uint32_t gLockedTexPagesMask;

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
// Upload the specified texture into VRAM if it's not already resident.
// If there's no more room for textures in VRAM for this frame then the game will die with an error.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_CacheTex(texture_t& tex) noexcept {
    // First update the frame the texture was added to the cache, for tracking overflows
    tex.uploadFrameNum = gNumFramesDrawn;

    // If the texture is already in the cache then there is nothing else to do
    if (tex.texPageId != 0)
        return;

    const uint32_t startTCacheFillPage = gTCacheFillPage;
    texture_t** pTexStartCacheCell = nullptr;

    {
    find_free_tcache_location:
        // Move onto another row in the texture cache if this row can't accomodate the texture
        if (gTCacheFillCellX + tex.width16 > TCACHE_CELLS_X) {
            gTCacheFillCellY += gTCacheFillRowCellH;
            gTCacheFillCellX = 0;
            gTCacheFillRowCellH = 0;
        }

        // Move onto another page in the texture cache if this page can't accomodate the texture.
        // Find one that is not locked and which is available for modification.
        if (gTCacheFillCellY + tex.height16 > TCACHE_CELLS_Y) {
            const uint32_t lockedTPages = gLockedTexPagesMask;

            // PsyDoom: if all the pages are locked this code will loop forever.
            // If this situation arises go straight to the overflow error so we can at least report the problem.
            #if PSYDOOM_MODS
                if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
            #endif
                    // Continue moving to the next texture page and wraparound if required until we find one that is not locked
                    do {
                        gTCacheFillPage++;
                        gTCacheFillPage -= (gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
                    } while ((lockedTPages >> gTCacheFillPage) & 1);
            #if PSYDOOM_MODS
                }
            #endif

            // If we wound up back where we started then there's nowhere in the cache to fit this texture.
            // This is where the imfamous overflow error kicks in...
            if (gTCacheFillPage == startTCacheFillPage) {
                I_Error("Texture Cache Overflow\n");
            }

            gTCacheFillCellX = 0;
            gTCacheFillCellY = 0;
            gTCacheFillRowCellH = 0;
        }

        // At the current fill location search all of the cells in the texture cache that this texture would occupy.
        // Make sure all of the cells are free and available for use before we can proceed.
        // If cells are not free then evict whatever is in the cache if allowed, otherwise skip past it.
        tcache_t& tcache = *gpTexCache;
        tcachepage_t& tcachepage = tcache.pages[gTCacheFillPage];
        pTexStartCacheCell = &tcachepage.cells[gTCacheFillCellY][gTCacheFillCellX];

        {
            // Iterate through all the cells this texture would occupy
            texture_t** pCacheEntry = pTexStartCacheCell;

            for (int32_t y = 0; y < tex.height16; ++y) {
                for (int32_t x = 0; x < tex.width16; ++x) {
                    // Check to see if this cell is empty and move past it.
                    // If it's already empty then we don't need to do anything:
                    texture_t* const pCellTex = *pCacheEntry;
                    ++pCacheEntry;

                    if (!pCellTex)
                        continue;

                    // Cell is not empty! If the texture in the cell is in use for this frame then we can't evict it.
                    // In this case skip past the texture and try again:
                    if (pCellTex->uploadFrameNum == gNumFramesDrawn) {
                        gTCacheFillCellX += pCellTex->width16;

                        // We may need to skip onto the next row on retry also, make sure we have the right row height recorded.
                        // The row height is the max of all the texture heights on the row basically:
                        if (gTCacheFillRowCellH < pCellTex->height16) {
                            gTCacheFillRowCellH = pCellTex->height16;
                        }

                        goto find_free_tcache_location;
                    }

                    // The cell is not empty but we can evict the texture, do that here now
                    I_RemoveTexCacheEntry(*pCellTex);
                }

                pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Move onto the next row of cells
            }
        }
    }

    // Fill all of the cells in the cache occupied by this texture with references to it
    {
        texture_t** pCacheEntry = pTexStartCacheCell;

        for (int32_t y = 0; y < tex.height16; ++y) {
            for (int32_t x = 0; x < tex.width16; ++x) {
                *pCacheEntry = &tex;
                ++pCacheEntry;
            }

            pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Move to the next row of cells
        }
    }

    // Record on the texture where it is located in the cache (top left corner)
    tex.ppTexCacheEntries = pTexStartCacheCell;

    // Make sure the texture's lump is loaded and decompress if required
    const std::byte* pTexData = (const std::byte*) W_CacheLumpNum(tex.lumpNum, PU_CACHE, false);
    const bool bIsTexCompressed = (!gpbIsUncompressedLump[tex.lumpNum]);

    if (bIsTexCompressed) {
        #if PSYDOOM_LIMIT_REMOVING
            gTmpBuffer.ensureSize(getDecodedSize(pTexData));
            decode(pTexData, gTmpBuffer.bytes());
            pTexData = gTmpBuffer.bytes();
        #else
            // PsyDoom: check for buffer overflows and issue an error if we exceed the limits
            #if PSYDOOM_MODS
                if (getDecodedSize(pTexData) > TMP_BUFFER_SIZE) {
                    I_Error("I_CacheTex: lump %d size > 64 KiB!", tex.lumpNum);
                }
            #endif

            decode(pTexData, gTmpBuffer);
            pTexData = gTmpBuffer;
        #endif
    }

    // Upload the texture to VRAM at the current fill location
    {
        const uint16_t tpageU = TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][0];
        const uint16_t tpageV = TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][1];

        RECT dstVramRect;
        LIBGPU_setRECT(
            dstVramRect,
            (int16_t)(tpageU + gTCacheFillCellX * TCACHE_CELL_SIZE / 2),
            (int16_t)(tpageV + gTCacheFillCellY * TCACHE_CELL_SIZE),
            tex.width / 2,
            tex.height
        );

        LIBGPU_LoadImage(dstVramRect, (uint16_t*)(pTexData + sizeof(texlump_header_t)));
    }

    // Save the textures page coordinate
    tex.texPageCoordX = (uint8_t)(gTCacheFillCellX * TCACHE_CELL_SIZE);
    tex.texPageCoordY = (uint8_t)(gTCacheFillCellY * TCACHE_CELL_SIZE);

    // Get and save the texture page id.
    tex.texPageId = LIBGPU_GetTPage(
        1,                                                                  // Format 1 = 8 bpp (indexed)
        0,                                                                  // Note: transparency bits are added later during rendering
        (gTCacheFillPage + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),      // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
        TEX_PAGE_VRAM_TEXCOORDS[gTCacheFillPage][1]
    );

    // Advance the fill position in the texture cache.
    // Also expand the fill row height if this texture is taller than the current height.
    gTCacheFillCellX += tex.width16;

    if (gTCacheFillRowCellH < tex.height16) {
        gTCacheFillRowCellH = tex.height16;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the given texture from the cache and frees any cells that it is using.
// Note this should only be called if the texture is actually in the cache!
//------------------------------------------------------------------------------------------------------------------------------------------
void I_RemoveTexCacheEntry(texture_t& tex) noexcept {
    // PsyDoom: added sanity check: this could trash textures in the cache if the texture is already freed!
    #if PSYDOOM_MODS
        ASSERT(tex.texPageId != 0);
    #endif

    // Wipe the texture page id used and clear any cells the texture occupies
    tex.texPageId = 0;
    texture_t** pCacheEntry = tex.ppTexCacheEntries;

    for (int32_t y = 0; y < tex.height16; ++y) {
        for (int32_t x = 0; x < tex.width16; ++x) {
            *pCacheEntry = nullptr;
            ++pCacheEntry;
        }

        pCacheEntry += TCACHE_CELLS_X - tex.width16;    // Next row
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evicts all textures in non-locked pages in the texture cache.
// Also resets the next position that we will populate the cache at.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_PurgeTexCache() noexcept {
    tcache_t& tcache = *gpTexCache;

    for (int32_t texPageIdx = 0; texPageIdx < NUM_TCACHE_PAGES; ++texPageIdx) {
        // Leave this texture page alone and skip past it if it is locked
        const uint32_t lockedTCachePages = gLockedTexPagesMask;

        // PsyDoom: this could potentially skip past the last texture cache page if it is locked.
        // It doesn't happen in practice because the last pages are always used for sprites and unlocked, but
        // change the method used here to skip just in case we do get undefined behavior...
        #if PSYDOOM_MODS
            if ((lockedTCachePages >> texPageIdx) & 1)
                continue;
        #else
            while ((lockedTPages >> texPageIdx) & 1) {
                ++texPageIdx;
            }
        #endif

        // Run though all of the cells in the texture cache page.
        // For each cell where we find an occupying texture, clear any cells that the found texture occupies.
        {
            texture_t** pPageCacheEntry = &tcache.pages[texPageIdx].cells[0][0];

            for (uint32_t cellIdx = 0; cellIdx < NUM_TCACHE_PAGE_CELLS; ++cellIdx, ++pPageCacheEntry) {
                // Ignore the texture cache cell if not occupied by a texture
                if (!*pPageCacheEntry)
                    continue;

                // Get the texture occupying this cell and clear it's texture page.
                texture_t& tex = **pPageCacheEntry;
                tex.texPageId = 0;

                // Clear all cells occupied by the texture
                texture_t** pTexCacheEntry = tex.ppTexCacheEntries;

                for (int32_t y = 0; y < tex.height16; ++y) {
                    for (int32_t x = 0; x < tex.width16; ++x) {
                        *pTexCacheEntry = nullptr;
                        ++pTexCacheEntry;
                    }

                    pTexCacheEntry += TCACHE_CELLS_X - tex.width16;     // Move to the next row of cells
                }
            }
        }
    }

    // Decide on the initial texture cache fill page: find the first page that is not locked.
    //
    // PsyDoom: I also added an additional safety check here.
    // If all the pages are locked for some reason this code would loop forever, check for that situation first.
    const uint32_t lockedTPages = gLockedTexPagesMask;
    gTCacheFillPage = 0;

    #if PSYDOOM_MODS
        if ((lockedTPages & ALL_TPAGES_MASK) != ALL_TPAGES_MASK) {
    #endif
            // Move onto the next texture cache page and wraparound, until we find an unlocked page to settle on
            while ((lockedTPages >> gTCacheFillPage) & 1) {
                gTCacheFillPage++;
                gTCacheFillPage -= (gTCacheFillPage / NUM_TCACHE_PAGES) * NUM_TCACHE_PAGES;
            }
    #if PSYDOOM_MODS
        }
    #endif

    // Reset the other fill parameters
    gTCacheFillCellX = 0;
    gTCacheFillCellY = 0;
    gTCacheFillRowCellH = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the 'vram viewer' screen that was only enabled in development builds of PSX DOOM.
// This code was still compiled into the retail binary however, but was inaccessible outside of memory hacks.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_VramViewerDraw(const int32_t texPageNum) noexcept {
    // PsyDoom: adjust the bottom edge of the VRAM view to account for overscan of 8-pixels
    #if PSYDOOM_MODS
        const int16_t BY = 232;
    #else
        const int16_t BY = 240;
    #endif

    // Draw a sprite covering the screen which covers the entire vram page
    {
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            POLY_FT4 polyPrim = {};
        #else
            POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setRGB0(polyPrim, 128, 128, 128);

        // PsyDoom: fix these right and bottom UVs now that we can do 16-bit coordinates.
        // This fixes slight pixel stretching issues in the VRAM viewer.
        #if PSYDOOM_LIMIT_REMOVING
            constexpr LibGpuUV BUV = 256;
            constexpr LibGpuUV RUV = 256;
        #else
            constexpr LibGpuUV BUV = 255;
            constexpr LibGpuUV RUV = 255;
        #endif

        LIBGPU_setUV4(polyPrim,
            0,      0,
            RUV,    0,
            0,      BUV,
            RUV,    BUV
        );

        LIBGPU_setXY4(polyPrim,
            0,      0,
            256,    0,
            0,      BY,
            256,    BY
        );

        polyPrim.clut = gPaletteClutIds[MAINPAL];

        polyPrim.tpage = LIBGPU_GetTPage(
            1,                                                          // Format 1 = 8 bpp (indexed)
            0,                                                          // Don't care about transparency
            (texPageNum + TCACHE_BASE_PAGE) * (TCACHE_PAGE_SIZE / 2),   // Note: must skip the first 4 pages (framebuffer) and also coord is divided by 2 because the data is 8-bit rather than 16-bit
            TEX_PAGE_VRAM_TEXCOORDS[texPageNum][1]
        );

        I_AddPrim(polyPrim);
    }

    // Draw red lines around all entries on this texture cache page
    const tcachepage_t& cachePage = gpTexCache->pages[texPageNum];
    const texture_t* const* pCacheEntry = &cachePage.cells[0][0];

    for (int32_t entryIdx = 0; entryIdx < TCACHE_CELLS_X * TCACHE_CELLS_Y; ++entryIdx, ++pCacheEntry) {
        // Skip past this texture cache cell if there's no texture occupying it
        const texture_t* pTex = *pCacheEntry;

        if (!pTex)
            continue;

        // Setup some stuff for all lines drawn.
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state.
        #if PSYDOOM_MODS
            LINE_F2 linePrim = {};
        #else
            LINE_F2& linePrim = *(LINE_F2*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetLineF2(linePrim);
        LIBGPU_setRGB0(linePrim, 255, 0, 0);

        // Figure out the y position and height for the texture cache entry.
        // Have to rescale in using a 24.8 fixed point format because the screen is 240 pixels high, and the texture cache is 256 pixels high.
        // I'm not sure why there is negative checks here however, texcoords are never negative?
        #if PSYDOOM_MODS
            // PsyDoom: need to adjust based on overscan adjustments above
            int32_t y = (d_lshift<8>((int32_t) pTex->texPageCoordY)) * BY / TCACHE_PAGE_SIZE;
            int32_t h = (d_lshift<8>((int32_t) pTex->height))        * BY / TCACHE_PAGE_SIZE;
        #else
            int32_t y = (d_lshift<8>((int32_t) pTex->texPageCoordY)) * SCREEN_H / TCACHE_PAGE_SIZE;
            int32_t h = (d_lshift<8>((int32_t) pTex->height))        * SCREEN_H / TCACHE_PAGE_SIZE;
        #endif

        if (y < 0) { y += 255; }
        if (h < 0) { h += 255; }

        y = d_rshift<8>(y);
        h = d_rshift<8>(h);

        const int16_t x = pTex->texPageCoordX;
        const int16_t w = pTex->width;

        // Draw all the box lines for the cache entry
        LIBGPU_setXY2(linePrim, x, (int16_t) y, x + w, (int16_t) y);            // Top
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x + w, (int16_t) y, x + w, (int16_t)(y + h));   // Right
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x + w, (int16_t)(y + h), x, (int16_t)(y + h));  // Bottom
        I_AddPrim(linePrim);

        LIBGPU_setXY2(linePrim, x, (int16_t)(y + h), x, (int16_t) y);           // Left
        I_AddPrim(linePrim);
    }
}

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

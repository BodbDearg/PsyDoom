#include "CDXAFileStreamer.h"

#include "Asserts.h"
#include "FileInputStream.h"
#include "PsyDoom/DiscInfo.h"
#include "PsyDoom/IsoFileSys.h"

#include <algorithm>

BEGIN_NAMESPACE(mdec)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates a CD-XA file streamer with no open file
//------------------------------------------------------------------------------------------------------------------------------------------
CDXAFileStreamer::CDXAFileStreamer() noexcept
    : mFile()
    , mCurSector(0)
    , mEndSector(0)
    , mSectorBuffer()
    , mUsedSectorBufferSlots()
    , mFreeSectorBufferSlots()
{
}

CDXAFileStreamer::~CDXAFileStreamer() noexcept = default;

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a file is currently open for streaming
//------------------------------------------------------------------------------------------------------------------------------------------
bool CDXAFileStreamer::isOpen() const noexcept {
    return (mFile != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to open the specified file for CD-XA streaming.
// If an existing file is open then it gets closed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool CDXAFileStreamer::open(
    const DiscInfo& disc,
    const IsoFileSys& fileSys,
    const char* const fileName,
    const uint32_t bufferSize
) noexcept {
    // Cleanup any currently open file
    close();

    // Try to locate the specified file and abort if not found:
    const IsoFileSysEntry* const pFsEntry = fileSys.getEntry(fileName);

    if (!pFsEntry)
        return false;

    // Grab the data track from the disc and verify that it holds raw CD sector data (2352 byte sector size).
    // Need access to raw CD sector data for this streamer to work; if the disc image doesn't have that then we can't proceed!
    const DiscTrack* const pTrack = disc.getTrack(1);
    const bool bValidDataTrack = (pTrack && (pTrack->blockSize == sizeof(CDXASector)));

    if (!bValidDataTrack)
        return false;

    // Try to open the file specified by the track and seek to the correct location for the file.
    // If it fails then abort the opening process.
    try {
        mFile = std::make_unique<FileInputStream>(pTrack->sourceFilePath.c_str());
        mFile->skipBytes(pTrack->fileOffset);
    } catch (...) {
        close();
        return false;
    }
    
    // Setup the sector buffer and buffer slot management.
    // Note: some of these fields should already be setup, hence checking these assumptions in debug.
    ASSERT(mCurSector == 0);
    ASSERT(mEndSector == 0);
    ASSERT(mSectorBuffer.empty());
    ASSERT(mUsedSectorBufferSlots.empty());
    ASSERT(mFreeSectorBufferSlots.empty());

    const uint32_t MIN_BUFFER_SIZE = 1u;
    const uint32_t realBufferSize = std::max(bufferSize, MIN_BUFFER_SIZE);  // Don't allow the buffer size to be below this!

    mSectorBuffer.resize(realBufferSize);
    mUsedSectorBufferSlots.reserve(realBufferSize);
    mFreeSectorBufferSlots.reserve(realBufferSize);

    for (uint32_t i = 0; i < realBufferSize; ++i) {
        mFreeSectorBufferSlots.push_back(i);
    }

    // Figure out how many physical/raw CD sectors the file consumes, rounding up to the nearest whole sector.
    // This is the size of the XA file stream...
    mEndSector = (pFsEntry->size + pTrack->blockPayloadSize - 1) / pTrack->blockPayloadSize;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes up the current file being streamed
//------------------------------------------------------------------------------------------------------------------------------------------
void CDXAFileStreamer::close() noexcept {
    mFile.reset();
    mCurSector = 0;
    mEndSector = 0;
    mSectorBuffer.clear();
    mUsedSectorBufferSlots.clear();
    mFreeSectorBufferSlots.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read a sector.
// May free up the oldest read sector in the sector buffer if there are no slots free.
// May also close up the stream if a read error occurs.
//------------------------------------------------------------------------------------------------------------------------------------------
const CDXASector* CDXAFileStreamer::readSector() noexcept {
    // Is there a file opened for streaming or is there data ahead?
    if (!mFile)
        return nullptr;

    // Are we at the end of the stream?
    if (mCurSector >= mEndSector)
        return nullptr;

    // Allocate a sector and then read it's contents from the file
    CDXASector& sector = allocBufferSector();

    try {
        mFile->read(sector);
    } catch (...) {
        close();
        return false;
    }

    // Success! Mark this sector as read and return its contents:
    mCurSector++;
    return &sector;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Marks the given sector as no longer being used, freeing it up for use by the streamer for read operations.
// Note: it's undefined behavior to pass in a sector not belonging to this streamer!
//------------------------------------------------------------------------------------------------------------------------------------------
void CDXAFileStreamer::freeSector(const CDXASector& sector) noexcept {
    // Figure out which slot index this is
    const uint32_t slotIdx = (uint32_t)(&sector - mSectorBuffer.data());
    ASSERT(slotIdx < mSectorBuffer.size());

    // Free the slot if it's currently in use
    const auto usedSlotsEndIter = mUsedSectorBufferSlots.end();
    const auto usedSlotIter = std::find(mUsedSectorBufferSlots.begin(), usedSlotsEndIter, slotIdx);

    if (usedSlotIter == usedSlotsEndIter)
        return;

    mUsedSectorBufferSlots.erase(usedSlotIter);
    mFreeSectorBufferSlots.push_back(slotIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates a buffer sector for use by the streamer.
// May free up the oldest read sector if there are no buffer slots available.
//------------------------------------------------------------------------------------------------------------------------------------------
CDXASector& CDXAFileStreamer::allocBufferSector() noexcept {
    // Are there any free buffer slots? If so then use that:
    if (!mFreeSectorBufferSlots.empty()) {
        const uint32_t slotIdx = mFreeSectorBufferSlots.back();
        mFreeSectorBufferSlots.pop_back();
        mUsedSectorBufferSlots.push_back(slotIdx);
        return mSectorBuffer[slotIdx];
    }

    // Otherwise steal the slot for the oldest sector read
    const uint32_t slotIdx = mUsedSectorBufferSlots.front();
    mUsedSectorBufferSlots.erase(mUsedSectorBufferSlots.begin());
    mUsedSectorBufferSlots.push_back(slotIdx);  // Now becomes the most recently read sector...
    return mSectorBuffer[slotIdx];
}

END_NAMESPACE(mdec)

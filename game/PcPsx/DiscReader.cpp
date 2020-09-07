#include "DiscReader.h"

#include "Asserts.h"
#include "DiscInfo.h"

#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the disc reader: the reference to the disc info must remain valid for the lifetime of this object
//------------------------------------------------------------------------------------------------------------------------------------------
DiscReader::DiscReader(DiscInfo& discInfo) noexcept
    : mDiscInfo(discInfo)
    , mpCurTrack(nullptr)
    , mCurTrackIdx(-1)
    , mCurOffset(0)
    , mpOpenFile(nullptr)
{
}

DiscReader::~DiscReader() noexcept {
    closeTrack();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the current track which is open for reading or '0' if none
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t DiscReader::getTrackNum() const noexcept {
    return mCurTrackIdx + 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switch to the specified track for reading and return 'true' if successful.
// Note: the offset in the track is initialized to '0' if the current track is changed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscReader::setTrackNum(int32_t trackNum) noexcept {
    // If not changing tracks then do nothing
    if (trackNum == mCurTrackIdx + 1)
        return true;

    // If the track doesn't exist then we can't open it
    const DiscTrack* const pTrack = mDiscInfo.getTrack(trackNum);

    if (!pTrack) {
        closeTrack();
        return false;
    }

    // Open the file for the new track if it's different to the current file
    if ((!mpCurTrack) || (mpCurTrack->sourceFilePath != pTrack->sourceFilePath)) {
        // Need to switch files: close the old track and open the new one
        closeTrack();
        mpOpenFile = std::fopen(pTrack->sourceFilePath.c_str(), "rb");

        if (!mpOpenFile)
            return false;
    }

    // Success - save the current track number and track!
    mpCurTrack = pTrack;
    mCurTrackIdx = trackNum - 1;
    
    // Seek to where we should be in the track data
    mCurOffset = -1;

    if (!trackSeekAbs(0)) {
        closeTrack();
        return false;
    }

    // All good if we've got to here
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Is a track currently open for reading?
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscReader::isTrackOpen() noexcept {
    return (mpOpenFile != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the track that is currently open for reading
//------------------------------------------------------------------------------------------------------------------------------------------
void DiscReader::closeTrack() noexcept {
    FILE* const pFile = (FILE*) mpOpenFile;

    if (pFile) {
        std::fclose(pFile);
        mpOpenFile = nullptr;
    }

    mCurOffset = 0;
    mCurTrackIdx = -1;
    mpCurTrack = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to the given absolute data offset in the track's actual payload data
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscReader::trackSeekAbs(const int32_t offsetAbs) noexcept {
    // Validate that there is a valid open track and that the offset is in range
    if (!mpCurTrack)
        return false;

    ASSERT(mpOpenFile);

    if ((offsetAbs < 0) || (offsetAbs > mpCurTrack->trackPayloadSize))
        return false;

    // If we are already at this offset then there is nothing we need do
    if (mCurOffset == offsetAbs)
        return true;
    
    // Do the seek and save the result if successful
    const int32_t physicalOffset = dataOffsetToPhysical(offsetAbs);
    
    if (std::fseek((FILE*) mpOpenFile, physicalOffset, SEEK_SET) != 0)
        return false;
    
    mCurOffset = offsetAbs;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek relatively by the given amount for the track
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscReader::trackSeekRel(const int32_t offsetRel) noexcept {
    // Validate that there is a valid open track and that the offset is in range
    if (!mpCurTrack)
        return false;

    ASSERT(mpOpenFile);
    const int32_t newOffset = mCurOffset + offsetRel;

    if ((newOffset < 0) || (newOffset > mpCurTrack->trackPayloadSize))
        return false;

    // If we are already at this offset then there is nothing we need do
    if (mCurOffset == newOffset)
        return true;

    // Do the seek and save the result if successful
    const int32_t physicalOffset = dataOffsetToPhysical(newOffset);
    
    if (std::fseek((FILE*) mpOpenFile, physicalOffset, SEEK_SET) != 0)
        return false;
    
    mCurOffset = newOffset;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to read the specified number of bytes into the given buffer.
// If the read fails for some reason then all bytes are zeroed.
// If the read succeeds then the current offset in the track is advanced.
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscReader::read(void* const pBuffer, const int32_t numBytes) noexcept {
    ASSERT(pBuffer);
    ASSERT(numBytes >= 0);
    
    // If there is no track open then the read fails
    if (!mpCurTrack) {
        std::memset(pBuffer, 0, (size_t) numBytes);
        return false;
    }

    // Continue reading until there no bytes left
    const int32_t blockPayloadSize = mpCurTrack->blockPayloadSize;

    std::byte* pDstBytes = (std::byte*) pBuffer;
    int32_t bytesLeft = numBytes;

    while (bytesLeft > 0) {
        // How many bytes are left in this sector? Try and read as much as we can or need from the sector:
        const int32_t sectorBytesLeft = blockPayloadSize - (mCurOffset % blockPayloadSize);
        const int32_t thisReadSize = std::min(bytesLeft, sectorBytesLeft);

        if (std::fread(pDstBytes, thisReadSize, 1, (FILE*) mpOpenFile) != 1) {
            std::memset(pBuffer, 0, (size_t) numBytes);
            return false;
        }

        // Read succeeded: seek to the next sector if we have consumed all of this sector's bytes
        bytesLeft -= thisReadSize;
        pDstBytes += thisReadSize;

        // Try to seek past what we just read and issue an error if that fails
        if (!trackSeekRel(thisReadSize)) {
            std::memset(pBuffer, 0, (size_t) numBytes);
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the offset in the currently open track
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t DiscReader::tell() const noexcept {
    return mCurOffset;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the actual offset in the current track's file for the given data offset.
// This offset includes raw CD sector framing and so on, if present.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t DiscReader::dataOffsetToPhysical(const int32_t dataOffset) const noexcept {
    if (mpCurTrack) {
        const int32_t lba = dataOffset / mpCurTrack->blockPayloadSize;
        const int32_t payloadOffset = dataOffset % mpCurTrack->blockPayloadSize;
        return mpCurTrack->fileOffset + lba * mpCurTrack->blockSize + mpCurTrack->blockPayloadOffset + payloadOffset;
    } else {
        return 0;
    }
}

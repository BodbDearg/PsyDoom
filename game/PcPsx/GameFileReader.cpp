#include "GameFileReader.h"

#include "Asserts.h"
#include "FatalErrors.h"
#include "String8_16.h"
#include "Wess/psxcd.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the file reader with no file open
//------------------------------------------------------------------------------------------------------------------------------------------
GameFileReader::GameFileReader() noexcept
    : mCdFile()
    , mpFile(nullptr)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automaticaly closes the file to cleanup
//------------------------------------------------------------------------------------------------------------------------------------------
GameFileReader::~GameFileReader() noexcept {
    close();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move the reader's resources from one object to another
//------------------------------------------------------------------------------------------------------------------------------------------
GameFileReader::GameFileReader(GameFileReader&& other) noexcept
    : mCdFile(other.mCdFile)
    , mpFile(other.mpFile)
{
    other.mCdFile = {};
    other.mpFile = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a file is currently open for reading
//------------------------------------------------------------------------------------------------------------------------------------------
bool GameFileReader::isOpen() noexcept {
    return ((mCdFile.fileHandle > 0) || (mCdFile.overrideFileHandle > 0) || mpFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the file currently being read
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::close() noexcept {
    if (mpFile) {
        std::fclose(mpFile);
        mpFile = nullptr;
    } else {
        psxcd_close(mCdFile);
        mCdFile = {};
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to open the specified file for reading.
// Note: it is illegal/undefined behavior to try to open another file while one is already open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::open(const char* const filePath) noexcept {
    ASSERT(!isOpen());
    mpFile = std::fopen(filePath, "rb");

    if (!mpFile) {
        FatalErrors::raiseF("GameFileReader::open: error opening file '%s'!", filePath);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to open the specified file for reading.
// Note: it is illegal/undefined behavior to try to open another file while one is already open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::open(const CdFileId fileId) noexcept {
    ASSERT(!isOpen());
    mCdFile = *psxcd_open(fileId);      // Note: will always return a valid pointer or fail with a fatal error!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seeks relative to the current location in the opened file.
// The file is expected to be open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::seekRelative(const int32_t offset) noexcept {
    ASSERT(isOpen());

    if (mpFile) {
        if (std::fseek(mpFile, offset, SEEK_CUR) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    } else {
        if (psxcd_seek(mCdFile, offset, PsxCd_SeekMode::CUR) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seeks to an absolute location in the opened file.
// The file is expected to be open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::seekAbsolute(const int32_t offset) noexcept {
    ASSERT(isOpen());

    if (mpFile) {
        if (std::fseek(mpFile, offset, SEEK_SET) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    } else {
        if (psxcd_seek(mCdFile, offset, PsxCd_SeekMode::SET) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a number of bytes from the file into the specified buffer.
// The file is expected to be open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::read(void* const pBuffer, const int32_t numBytes) noexcept {
    ASSERT(isOpen());
    ASSERT(numBytes >= 0);
    ASSERT(pBuffer || (numBytes == 0));

    if (numBytes <= 0)
        return;

    if (mpFile) {
        if (std::fread(pBuffer, (uint32_t) numBytes, 1, mpFile) != 1) {
            FatalErrors::raiseF("GameFileReader::read: operation failed - IO error!");
        }
    } else {
        if (psxcd_read(pBuffer, numBytes, mCdFile) != numBytes) {
            FatalErrors::raiseF("GameFileReader::read: operation failed - IO error!");
        }
    }
}

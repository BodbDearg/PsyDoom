#include "GameFileReader.h"

#include "Asserts.h"
#include "FatalErrors.h"
#include "Wess/psxcd.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the file reader with no file open
//------------------------------------------------------------------------------------------------------------------------------------------
GameFileReader::GameFileReader() noexcept
    : mpCdFile(nullptr)
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
// Tells if a file is currently open for reading
//------------------------------------------------------------------------------------------------------------------------------------------
bool GameFileReader::isOpen() noexcept {
    return (mpCdFile || mpFile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the file currently being read
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::close() noexcept {
    if (mpCdFile) {
        psxcd_close(*mpCdFile);
        mpCdFile = nullptr;
    } else if (mpFile) {
        std::fclose(mpFile);
        mpFile = nullptr;
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
    mpCdFile = psxcd_open(fileId);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seeks relative to the current location in the opened file.
// The file is expected to be open.
//------------------------------------------------------------------------------------------------------------------------------------------
void GameFileReader::seekRelative(const int32_t offset) noexcept {
    ASSERT(isOpen());

    if (mpCdFile) {
        if (psxcd_seek(*mpCdFile, offset, PsxCd_SeekMode::CUR) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    } else {
        if (std::fseek(mpFile, offset, SEEK_CUR) != 0) {
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

    if (mpCdFile) {
        if (psxcd_seek(*mpCdFile, offset, PsxCd_SeekMode::SET) != 0) {
            FatalErrors::raiseF("GameFileReader::seek: operation failed - IO error!");
        }
    } else {
        if (std::fseek(mpFile, offset, SEEK_SET) != 0) {
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

    if (mpCdFile) {
        if (psxcd_read(pBuffer, numBytes, *mpCdFile) != numBytes) {
            FatalErrors::raiseF("GameFileReader::read: operation failed - IO error!");
        }
    } else {
        if (std::fread(pBuffer, (uint32_t) numBytes, 1, mpFile) != 1) {
            FatalErrors::raiseF("GameFileReader::read: operation failed - IO error!");
        }
    }
}

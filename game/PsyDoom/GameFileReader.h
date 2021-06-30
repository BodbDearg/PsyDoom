#pragma once

#include "Wess/psxcd.h"

#include <cstdio>

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides a consistent interface to read from a game file from several possible sources:
//  (1) A file within the game's CD image.
//  (2) A file within the game's CD image that has been overriden on disk.
//  (3) A real file on disk.
//
// Note: if any IO errors occur then the problem will be treated as fatal and the game will be terminated.
//------------------------------------------------------------------------------------------------------------------------------------------
class GameFileReader {
public:
    GameFileReader() noexcept;
    GameFileReader(GameFileReader&& other) noexcept;
    ~GameFileReader() noexcept;
    
    bool isOpen() noexcept;
    void close() noexcept;
    void open(const char* const filePath) noexcept;
    void open(const CdFileId fileId) noexcept;
    void seekRelative(const int32_t offset) noexcept;
    void seekAbsolute(const int32_t offset) noexcept;
    void read(void* const pBuffer, const int32_t numBytes) noexcept;

    // Convenience overload
    template <class T>
    void read(T& value) noexcept { read(&value, sizeof(T)); }

private:
    GameFileReader(const GameFileReader& other) = delete;
    GameFileReader& operator = (const GameFileReader& other) = delete;
    GameFileReader& operator = (GameFileReader&& other) = delete;

    PsxCd_File      mCdFile;    // If reading from a file on-disk, this is the index of the file slot open in the 'psxcd' library
    FILE*           mpFile;     // If reading from a real file, this is the file being read
};

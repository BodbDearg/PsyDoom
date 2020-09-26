#pragma once

#include "InputStream.h"

#include <cstdio>

//------------------------------------------------------------------------------------------------------------------------------------------
// Simple stream that reads from a file
//------------------------------------------------------------------------------------------------------------------------------------------
class FileInputStream final : public InputStream {
public:
    inline FileInputStream(const char* const filePath) THROWS
        : mpFile(std::fopen(filePath, "rb"))
    {
        if (!mpFile)
            throw StreamException();
    }

    virtual ~FileInputStream() noexcept override {
        if (mpFile) {
            std::fclose(mpFile);
            mpFile = nullptr;
        }
    }

    virtual void readBytes(void* const pDstBytes, const size_t numBytes) THROWS override {
        if (numBytes > 0) {
            if (std::fread(pDstBytes, numBytes, 1, mpFile) != 1)
                throw StreamException();
        }
    }

    virtual void skipBytes(const size_t numBytes) THROWS override {
        if (numBytes > 0) {
            if (std::fseek(mpFile, (long) numBytes, SEEK_CUR) != 0)
                throw StreamException();
        }
    }

    virtual size_t tell() THROWS override {
        const long offset = std::ftell(mpFile);

        if (offset < 0)
            throw StreamException();

        return (size_t) offset;
    }

    virtual bool isAtEnd() noexcept override {
        const int charValue = std::getc(mpFile);
        std::ungetc(charValue, mpFile);
        return (charValue == EOF);
    }

private:
    inline FileInputStream(const FileInputStream& other) = delete;
    inline FileInputStream(FileInputStream&& other) = delete;
    inline FileInputStream& operator = (const FileInputStream& other) = delete;
    inline FileInputStream& operator = (FileInputStream&& other) = delete;

    FILE* mpFile;
};

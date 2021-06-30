#pragma once

#include "Macros.h"

#include <cstdint>
#include <vector>

class DiscReader;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a single entry in the filesystem
//------------------------------------------------------------------------------------------------------------------------------------------
struct IsoFileSysEntry {
    // Then the parent index is this, this entry is the root of the filesystem
    static constexpr uint16_t ROOT_PARENT_IDX = 0xFFFF;

    bool        bIsDirectory;       // If 'false' the entry is a file, otherwise it is a directory
    uint8_t     nameLen;            // How long the file/directory name is
    char        name[40];           // Null terminated name of the directory or file, including it's extension
    uint16_t    parentIdx;          // Index of the parent file system entry or 0xFFFF if the root
    uint16_t    firstChildIdx;      // Only used for directories: first child index in the list of file system entries
    uint16_t    numChildren;        // Only used for directories: how many things there are in the directory
    uint32_t    startLba;           // Only used for files: first CD-ROM logical block index containing the file's data
    uint32_t    size;               // Only used for files: size of the file in bytes
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains the ISO 9660 filesystem for a game disc and allows lookup of files.
// Note that the filesystem entries are cut down to just the attributes we are interested in.
//------------------------------------------------------------------------------------------------------------------------------------------
struct IsoFileSys {
    std::vector<IsoFileSysEntry>    entries;                // All the entries in the file system: the root entry is the first
    uint32_t                        logicalBlockSize;       // Size of a logical sector for the CD-ROM, normally 2,048 bytes

    bool build(DiscReader& discReader) noexcept;
    int32_t getEntryIndex(const char* const path) const noexcept;
    int32_t getEntryIndex(const IsoFileSysEntry& root, const char* const path) const noexcept;
    const IsoFileSysEntry* getEntry(const char* const path) const noexcept;
    const IsoFileSysEntry* getEntry(const IsoFileSysEntry& root, const char* const path) const noexcept;
};

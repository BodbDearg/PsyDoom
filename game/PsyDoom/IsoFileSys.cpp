#include "IsoFileSys.h"

#include "Asserts.h"
#include "ByteInputStream.h"
#include "DiscInfo.h"
#include "DiscReader.h"
#include "Endian.h"

#include <cstring>
#include <queue>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents most of an ISO 9660 directory record.
// Following this but not included are:
//
//  n bytes     :   name (size depends on name length)
//  0-1 byte    :   padding (if name length is even)
//  n bytes     :   system use area (whatever is leftover in directory record bytes)
//------------------------------------------------------------------------------------------------------------------------------------------
struct IsoDirRecord {
    uint8_t     recordLen;                  // Size in bytes of this directory record
    uint8_t     xaRecordLen;                // Size in bytes of the 'extended attribute' record that is at the start of the data block for this record
    uint8_t     lbaLittle[4];               // Which sector (logical block address) the data for the record starts at - in 32-bit little endian format
    uint8_t     lbaBig[4];                  // Which sector (logical block address) the data for the record starts at - in 32-bit big endian format
    uint8_t     sizeLittle[4];              // Size of the record's data - in 32-bit little endian format
    uint8_t     sizeBig[4];                 // Size of the record's data - in 32-bit big endian format
    uint8_t     dateYear;                   // Recording timestamp: year - 1900
    uint8_t     dateMonth;                  // Recording timestamp: month (1-12)
    uint8_t     dateDay;                    // Recording timestamp: day (1-31)
    uint8_t     dateHour;                   // Recording timestamp: hour (0-23)
    uint8_t     dateMinute;                 // Recording timestamp: minute (0-59)
    uint8_t     dateSecond;                 // Recording timestamp: second (0-59)
    int8_t      dateTimezone;               // Recording timestamp: timezone (offset from GMT in 15 minute intervals)
    uint8_t     flags;                      // If '0x2' is set then the record is a directory
    uint8_t     fileUnitSize;               // File unit size for the file section if recorded in interleaved mode: should be '0' for PSX discs?
    uint8_t     interleaveGapSize;          // Interleave gap size for the file section if recorded in interleaved mode: should be '0' for PSX discs?
    uint8_t     volumeSeqNumLittle[2];      // Which volume the directory data is recorded on - in 16-bit little endian format. Should always be '1' for PSX discs.
    uint8_t     volumeSeqNumBig[2];         // Which volume the directory data is recorded on - in 16-bit big endian format. Should always be '1' for PSX discs.
    uint8_t     nameLen;                    // Length of the record name which follows

    bool isDirectory() const noexcept {
        return (flags & 0x2);
    }

    uint32_t getLba() const noexcept {
        return (
            ((uint32_t) lbaLittle[0] << 0) |
            ((uint32_t) lbaLittle[1] << 8) |
            ((uint32_t) lbaLittle[2] << 16) |
            ((uint32_t) lbaLittle[3] << 24)
        );
    }

    uint32_t getSize() const noexcept {
        return (
            ((uint32_t) sizeLittle[0] << 0) |
            ((uint32_t) sizeLittle[1] << 8) |
            ((uint32_t) sizeLittle[2] << 16) |
            ((uint32_t) sizeLittle[3] << 24)
        );
    }
};

static_assert(sizeof(IsoDirRecord) == 33);

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a volume in the ISO 9960 file system, and it is found at sector 16 on the disc.
// Normally there is just one of these for the PlayStation.
//------------------------------------------------------------------------------------------------------------------------------------------
struct IsoVolDescriptor {
    uint8_t         volDescType;                        // 1 = primary volume descriptor
    char            standardId[5];                      // Normally 'CD001'
    uint8_t         volumeDescVersion;                  // 1 = standard
    uint8_t         _reserved1;
    char            systemId[32];                       // Normally "PLAYSTATION"
    char            volumeId[32];                       // Disc name
    char            _reserved2[8];
    uint32_t        volumeSpaceSizeLittle;              // Size of the volume in logical blocks (little endian)
    uint32_t        volumeSpaceSizeBig;                 // Size of the volume in logical blocks (big endian)
    char            _reserved3[32];
    uint16_t        volumeSetSizeLittle;                // Normally '1' (little endian)
    uint16_t        volumeSetSizeBig;                   // Normally '1' (big endian)
    uint16_t        volumeSeqNumLittle;                 // Normally '1' (little endian)
    uint16_t        volumeSeqNumBig;                    // Normally '1' (big endian)
    uint16_t        logicalBlockSizeLittle;             // Payload size for sectors, normally '2048' bytes (little endian)
    uint16_t        logicalBlockSizeBig;                // Payload size for sectors, normally '2048' bytes (big endian)
    uint32_t        pathTableSizeLittle;                // Size of the path table (little endian)
    uint32_t        pathTableSizeBig;                   // Size of the path table (big endian)
    uint32_t        pathTable1BlockNumLittle;           // Block number for a path table, '0' for none (little endian)
    uint32_t        pathTable2BlockNumLittle;           // Block number for a path table, '0' for none (little endian)
    uint32_t        pathTable3BlockNumLittle;           // Block number for a path table, '0' for none (big endian)
    uint32_t        pathTable4BlockNumLittle;           // Block number for a path table, '0' for none (big endian)
    IsoDirRecord    rootRecord;                         // Directory record for the root of the filesystem
    char            rootRecordName;                     // Ignore this for the root, should be '\0'
    char            volumeSetId[128];                   // Normally empty for PSX discs
    char            publisherId[128];                   // Company name that published the disc
    char            dataPreparerId[128];                // Can be whatever...
    char            applicationId[128];                 // Normally "PLAYSTATION"
    char            copyrightFilename[37];              // Name of a file
    char            abstractFilename[37];               // Name of a file
    char            bibliographicFilename[37];          // Name of a file
    char            volCreationTimestamp[17];           // YYYYMMDDHHMMSSFF
    char            volModificationTimestamp[17];       // YYYYMMDDHHMMSSFF
    char            volExpirationTimestamp[17];         // YYYYMMDDHHMMSSFF
    char            volEffectiveTimestamp[17];          // YYYYMMDDHHMMSSFF
    char            fileStructureVersion;               // Should be '1' for PSX discs
    char            _reserved4;
    char            applicationUseArea1[141];           // Normally zero filled for PSX
    char            cdxaIdSignature[8];                 // Normally "CD-XA001" for PSX
    uint8_t         cdxaFlags[2];                       // Normally all '0' for PSX
    char            cdxaStartupDir[8];                  // Normally zero filled for PSX
    char            _reserved5[8];
    char            applicationUseArea2[345];           // Normally zero filled for PSX
    char            _reserved6[653];
};

static_assert(sizeof(IsoVolDescriptor) == 2048);

//------------------------------------------------------------------------------------------------------------------------------------------
// Used to hold details on a directory which is pending reading
//------------------------------------------------------------------------------------------------------------------------------------------
struct DirToRead {
    uint32_t parentDirIndex;        // Index of the parent directory file system entry.
    uint32_t lba;                   // Logical block address of where the directory records for this directory are.
    uint32_t size;                  // Size of the directory records for this directory; includes padding and must to be a multiple of '2048'.
    uint32_t xaRecordSize;          // Extended attribute record size, normally '0'. If non zero skip this many bytes to get to the directory records for the dir.
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds context used by the directory reader
//------------------------------------------------------------------------------------------------------------------------------------------
struct DirReaderContext {
    IsoFileSys&             fs;                     // The filesystem being read
    const DirToRead&        dir;                    // Which directory is being read
    std::queue<DirToRead>&  dirsToRead;             // Which directories are left to read
    uint32_t                xaRecordBytesLeft;      // How many bytes left of the extended attribute record there are left to skip
    uint32_t                recordIdx;              // The index of the current record being read, within the directory
};

//------------------------------------------------------------------------------------------------------------------------------------------
// CD-ROM sector that the primary volume descriptor is found on for the ISO 9960 file system
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int32_t VOL_DESC_SECTOR = 16;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the specified character is a path separator
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isPathSeparator(const char c) noexcept {
    return ((c == '\\') || (c == '/'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the file system and disc reader for the process of reading files and directories.
// Reads the volume descriptor for the filesystem and creates the root filesystem entry.
// Also queues the first directory to be read.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool initFileSystemForReading(IsoFileSys& fs, std::queue<DirToRead>& dirsToRead, DiscReader& discReader) noexcept {
    // Default initialize the file system and prealloc some memory for entries
    fs.logicalBlockSize = 0;
    fs.entries.clear();
    fs.entries.reserve(2048);

    // Make sure the disc is open on the data track (01)
    if (!discReader.setTrackNum(1))
        return false;

    // Read the ISO volume descriptor (located at sector 16)
    const DiscTrack* pTrack = discReader.getOpenTrack();
    ASSERT(pTrack);

    if (!discReader.trackSeekAbs(pTrack->blockPayloadSize * VOL_DESC_SECTOR))
        return false;

    IsoVolDescriptor volDesc;

    if (!discReader.read(&volDesc, sizeof(IsoVolDescriptor)))
        return false;

    // Read the logical block size and verify it's within allowed limits
    fs.logicalBlockSize = Endian::littleToHost(volDesc.logicalBlockSizeLittle);

    const bool bInvalidLogicalBlockSize = (
        (fs.logicalBlockSize < IsoFileSys::MIN_LOGICAL_BLOCK_SIZE) ||
        (fs.logicalBlockSize > IsoFileSys::MAX_LOGICAL_BLOCK_SIZE)
    );

    if (bInvalidLogicalBlockSize)
        return false;

    // Create the filesystem entry for the volume root
    {
        IsoFileSysEntry& volRoot = fs.entries.emplace_back();
        volRoot.bIsDirectory = true;
        volRoot.parentIdx = IsoFileSysEntry::ROOT_PARENT_IDX;
    }

    // Queue the first directory to be read (children of the root)
    {
        DirToRead& dir = dirsToRead.emplace();
        dir.parentDirIndex = 0;
        dir.lba = volDesc.rootRecord.getLba();
        dir.size = volDesc.rootRecord.getSize();
        dir.xaRecordSize = volDesc.rootRecord.xaRecordLen;
    }

    // All good if we've got to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a single sector of file system data for a directory in the file system.
// Throws an exception if reading fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static void readFSDirSector(DirReaderContext& ctx, ByteInputStream& sectorBytes) THROWS {
    // Firstly skip past any extended attribute record bytes for this directory.
    // Those are located at the start of the directory data:
    if (ctx.xaRecordBytesLeft > 0) {
        const uint32_t numBytesToSkip = std::min((uint32_t) sectorBytes.size(), ctx.xaRecordBytesLeft);
        sectorBytes.skipBytes(numBytesToSkip);
        ctx.xaRecordBytesLeft -= numBytesToSkip;
    }

    // Continue reading records until we reach the end of the sector and can read no more.
    // Note: once reading is done any leftover bytes at the end of the sector are just zero padding bytes and are ignored.
    while (sectorBytes.checkCanRead<IsoDirRecord>()) {
        // Read the record firstly
        IsoDirRecord record;
        sectorBytes.read(record);

        // If the record says it's zero length then it means there are no more records in the sector.
        // The rest of the data will simply be zero padding bytes.
        const uint8_t recordLen = record.recordLen;

        if (recordLen == 0)
            break;
        
        // Otherwise sanity check some things about the directory record
        const bool bBadOrUnsupportedRecord = (
            (recordLen < sizeof(IsoDirRecord) + record.nameLen) ||      // Bad record length?
            (record.volumeSeqNumLittle[0] != 1) ||                      // Expect volume number to always be '1'
            (record.volumeSeqNumLittle[1] != 0) ||                      // Expect volume number to always be '1'
            (record.interleaveGapSize != 0) ||                          // Not supporting this whatever it is
            (record.fileUnitSize != 0)                                  // Not supporting this whatever it is
        );

        if (bBadOrUnsupportedRecord)
            throw "Bad filesystem record!";

        // Read the name of the record and null terminate.
        // Remove the version part of the name (';1' etc.) at the end of the string also.
        char recordName[256];
        sectorBytes.readArray(recordName, record.nameLen);
        recordName[record.nameLen] = 0;

        if (char* pVersionSep = std::strchr(recordName, ';'); pVersionSep) {
            *pVersionSep = 0;
        }

        // Skip the rest of the record bytes if there are any, we don't use them:
        sectorBytes.skipBytes(recordLen - sizeof(IsoDirRecord) - record.nameLen);

        // In the ISO 9960 filesystem the following two record indexes (within a directory) have the following meanings:
        //  0 - Parent directory record
        //  1 - This directory record (pointer to self)
        //
        // Don't make file system entries for these!
        if (ctx.recordIdx >= 2) {
            // Makeup the basic file system entry fields for this directory record
            IsoFileSysEntry& fsEntry = ctx.fs.entries.emplace_back();
            fsEntry.bIsDirectory = record.isDirectory();
            fsEntry.parentIdx = (uint16_t) ctx.dir.parentDirIndex;

            std::strncpy(fsEntry.name, recordName, C_ARRAY_SIZE(fsEntry.name) - 1);
            fsEntry.name[C_ARRAY_SIZE(fsEntry.name) - 1] = 0;
            fsEntry.nameLen = (uint8_t) std::strlen(fsEntry.name);

            // Are we dealing with a directory or a file?
            if (fsEntry.bIsDirectory) {
                // This is a directory: it will have it's children read at a later point
                DirToRead& queuedDir = ctx.dirsToRead.emplace();
                queuedDir.parentDirIndex = (uint32_t)(ctx.fs.entries.size() - 1);
                queuedDir.lba = record.getLba();
                queuedDir.size = record.getSize();
                queuedDir.xaRecordSize = record.xaRecordLen;
            } else {
                // This is just a file: record it's size and location
                fsEntry.startLba = record.getLba();
                fsEntry.size = record.getSize();
            }

            // The parent filesystem entry now has one more child underneath it
            ctx.fs.entries[ctx.dir.parentDirIndex].numChildren++;
        }

        // We're moving onto the next record in this directory...
        ctx.recordIdx++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the file system for a single directory at the front of the given queue of directories.
// May place additional child directories at the back of the queue for reading later.
// Returns 'false' if reading fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readFileSystemDir(IsoFileSys& fs, std::queue<DirToRead>& dirsToRead, DiscReader& discReader) noexcept {
    // Debug sanity checks
    ASSERT(!dirsToRead.empty());
    ASSERT(fs.logicalBlockSize > 0);

    // Get the directory to read.
    // Note that the size of the directory must be a multiple of the logical block size for the filesystem!
    DirToRead dir = dirsToRead.front();
    dirsToRead.pop();

    if (dir.size % fs.logicalBlockSize != 0)
        return false;

    // In the parent directory set the index of where it's first child will be found (if there are any)
    ASSERT(dir.parentDirIndex < fs.entries.size());
    fs.entries[dir.parentDirIndex].firstChildIdx = (uint16_t) fs.entries.size();

    // Seek to where the records for this directory are located
    if (!discReader.trackSeekAbs(dir.lba * fs.logicalBlockSize))
        return false;

    // Read all the sectors of filesystem entries for this directory
    DirReaderContext dirReaderCtx = { fs, dir, dirsToRead, dir.xaRecordSize, 0 };

    for (uint32_t sectorsLeft = dir.size / fs.logicalBlockSize; sectorsLeft > 0; --sectorsLeft) {
        // Get the raw sector bytes first
        std::byte sectorData[IsoFileSys::MAX_LOGICAL_BLOCK_SIZE];

        if (!discReader.read(sectorData, fs.logicalBlockSize))
            return false;

        // Read filesystem entries from this sector
        try {
            ByteInputStream byteStream(sectorData, fs.logicalBlockSize);
            readFSDirSector(dirReaderCtx, byteStream);
        } catch (...) {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the entire ISO 9960 filesystem from the given disc's data track (01).
// Returns 'true' if the filesystem was read successfully, without any errors.
//------------------------------------------------------------------------------------------------------------------------------------------
bool IsoFileSys::build(DiscReader& discReader) noexcept {
    // Initialize the filesystem in preparation for reading directories
    std::queue<DirToRead> dirsToRead;

    if (!initFileSystemForReading(*this, dirsToRead, discReader))
        return false;

    // Read all directories in the file system until there are no more left to process
    while (!dirsToRead.empty()) {
        if (!readFileSystemDir(*this, dirsToRead, discReader))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Lookup the index of the file system entry for the given path (case insensitive), relative to the root of the filesystem.
// Returns '-1' if the file system entry is not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t IsoFileSys::getEntryIndex(const char* const path) const noexcept {
    return (!entries.empty()) ? getEntryIndex(entries[0], path) : -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Lookup the index of the file system entry for the given path (case insensitive), relative to the give filesystem root.
// Returns '-1' if the file system entry is not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t IsoFileSys::getEntryIndex(const IsoFileSysEntry& root, const char* const path) const noexcept {
    // Skip root separators in the given path
    if (path[0] == '.') {
        if (isPathSeparator(path[1]))
            return getEntryIndex(root, path + 2);
    }
    else if (isPathSeparator(path[0])) {
        return getEntryIndex(root, path + 1);
    }

    // Try to match the path against all the entries in the given root
    for (int32_t childIdx = 0; childIdx < root.numChildren; ++childIdx) {
        const int32_t childEntryIdx = root.firstChildIdx + childIdx;
        const IsoFileSysEntry& childEntry = entries[childEntryIdx];

        for (uint32_t charIdx = 0; charIdx < C_ARRAY_SIZE(childEntry.name); ++charIdx) {
            // Do case insensitive comparison
            const char c1 = (char) std::toupper(childEntry.name[charIdx]);
            const char c2 = (char) std::toupper(path[charIdx]);

            // Is the filesystem name at the end?
            if (c1 == 0) {
                // If the search string is also at an end then they are matched:
                if (c2 == 0)
                    return childEntryIdx;

                // If the search string is on a path separator then we might be able to recurse into a directory.
                // If this is the case then chop off the part of the path that we have matched and recurse:
                if (isPathSeparator(c2)) {
                    if (childEntry.bIsDirectory)
                        return getEntryIndex(childEntry, path + charIdx + 1);
                }

                // Otherwise the paths are not matched: move onto 
                break;
            }

            // If the search string is not matching then this filesystem entry is not a match for the path
            if (c1 != c2)
                break;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience overload: lookup a file system entry directly by path
//------------------------------------------------------------------------------------------------------------------------------------------
const IsoFileSysEntry* IsoFileSys::getEntry(const char* const path) const noexcept {
    const int32_t entryIndex = getEntryIndex(path);
    return (entryIndex >= 0) ? &entries[entryIndex] : nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience overload: lookup a file system entry directly by path within a given parent
//------------------------------------------------------------------------------------------------------------------------------------------
const IsoFileSysEntry* IsoFileSys::getEntry(const IsoFileSysEntry& root, const char* const path) const noexcept {
    const int32_t entryIndex = getEntryIndex(root, path);
    return (entryIndex >= 0) ? &entries[entryIndex] : nullptr;
}

#include "DiscInfo.h"

#include "FileUtils.h"

#include <algorithm>
#include <cstring>
#include <regex>

// Regexes for matching the beginning of a command
static const std::regex gRegexCmdBeg_File = std::regex(R"(^FILE\b)", std::regex_constants::icase);
static const std::regex gRegexCmdBeg_Track = std::regex(R"(^TRACK\b)", std::regex_constants::icase);
static const std::regex gRegexCmdBeg_Index = std::regex(R"(^INDEX\b)", std::regex_constants::icase);
static const std::regex gRegexCmdBeg_Pregap = std::regex(R"(^PREGAP\b)", std::regex_constants::icase);
static const std::regex gRegexCmdBeg_Postgap = std::regex(R"(^POSTGAP\b)", std::regex_constants::icase);

// Regexes for parsing the individual bits of each command
static const std::regex gRegexCmd_File = std::regex(R"(FILE\s+\"(.*?)\"\s+BINARY\s*$)");
static const std::regex gRegexCmd_Track = std::regex(R"(TRACK\s+(\d+)\s+(.*?)\s*$)");
static const std::regex gRegexCmd_Index = std::regex(R"(INDEX\s+(\d+)\s+(\d+)\s*:\s*(\d+)\s*:\s*(\d+)\s*$)");

// Current parsing context for the .cue file parser
struct CueParseCtx {
    DiscInfo&       disc;           // The disc to save the results to
    const char*     cueBasePath;    // Path to which paths in the .cue file are relative: should include a trailing path separator if specified
    std::string     file;           // File to read the current track data from
    int32_t         fileSize;       // Total size in bytes of the file pointed to by 'file'
    std::string     line;           // The current .cue line
    std::string     errorMsg;       // Error message if something went wrong
};

static bool isNewline(const char c) noexcept {
    return ((c == '\r') || (c == '\n') || (c == '\v'));
}

static bool isSpace(const char c) noexcept {
    return ((c == ' ') || (c == '\t') || (c == '\v') || (c == '\f'));
}

static bool isNewlineOrSpace(const char c) noexcept {
    return (isNewline(c) || isSpace(c));
}

static const char* skipNewlinesAndSpace(const char* str) {
    for (char c = *str; c != 0; c = *++str) {
        if (!isNewlineOrSpace(c))
            break;
    }

    return str;
}

static const char* findNextNewline(const char* str) {
    for (char c = *str; c != 0; c = *++str) {
        if (isNewline(c))
            break;
    }

    return str;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a 'FILE' command. E.G: FILE "Final Doom.img" BINARY
//------------------------------------------------------------------------------------------------------------------------------------------
static bool parseCueCmd_File(CueParseCtx& ctx) noexcept {
    std::smatch matches;
    std::regex_search(ctx.line, matches, gRegexCmd_File);

    if (matches.size() != 2) {
        ctx.errorMsg = "Invalid 'FILE' command format: ";
        ctx.errorMsg += ctx.line;
        ctx.errorMsg += "\nExpected: FILE \"<PATH_TO_FILE>\" BINARY";
        return false;
    }

    ctx.file = ctx.cueBasePath;     // Note: should already include a trailing path separator if not empty (i.e base path is specified)
    ctx.file += matches[1].str();
    ctx.fileSize = (int32_t) FileUtils::getFileSize(ctx.file.c_str());

    if (ctx.fileSize < 0) {
        if (!FileUtils::fileExists(ctx.file.c_str())) {
            // This is probably the most likely error to happen: use a more specific message
            ctx.errorMsg = "File specified by 'FILE' command does not exist: ";
            ctx.errorMsg += ctx.line;
            return false;
        } else {
            ctx.errorMsg = "Invalid/unreadable file specified by a 'FILE' command - couldn't determine the file's size! Command was: ";
            ctx.errorMsg += ctx.line;
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a 'TRACK' command. E.G: TRACK 1 MODE2/2352
//------------------------------------------------------------------------------------------------------------------------------------------
static bool parseCueCmd_Track(CueParseCtx& ctx) noexcept {
    // Make sure the command is formatted validly
    std::smatch matches;
    std::regex_search(ctx.line, matches, gRegexCmd_Track);

    if (matches.size() != 3) {
        ctx.errorMsg = "Invalid 'TRACK' command format: ";
        ctx.errorMsg += ctx.line;
        ctx.errorMsg += "\nExpected: TRACK <TRACK_NUMBER> <MODE/FORMAT>";
        return false;
    }

    // There must be a file specified for the track
    if (ctx.file.empty()) {
        ctx.errorMsg = "'TRACK' command specified but no 'FILE' command specified before that! Track data file not defined.";
        return false;
    }

    // Fill in the basic track details
    DiscTrack& track = ctx.disc.tracks.emplace_back();
    track.sourceFilePath = ctx.file;
    track.sourceFileTotalSize = ctx.fileSize;

    try {
        track.trackNum = std::stoi(matches[1]);
    } catch (...) {
        ctx.errorMsg = "Invalid track number: ";
        ctx.errorMsg += matches[1];
        return false;
    }

    // These track details are not known yet
    track.fileOffset = -1;
    track.blockCount = -1;
    track.index0 = -1;
    track.index1 = 1;

    // Get the track mode uppercased
    std::string mode = matches[2];
    std::for_each(mode.c_str(), mode.c_str() + mode.size(), ::toupper);

    // Set these block/track parameters based on the mode.
    // See 'Bin2ISO : https://gist.github.com/ceritium/139577' for more details on some of these.
    if ((mode == "MODE1/2048") || (mode == "MODE2/2048")) {
        track.bIsData = true;
        track.blockSize = 2048;
        track.blockPayloadOffset = 0;
        track.blockPayloadSize = 2048;
    }
    else if (mode == "MODE1/2352") {
        track.bIsData = true;
        track.blockSize = 2352;
        track.blockPayloadOffset = 16;
        track.blockPayloadSize = 2048;
    }
    else if (mode == "MODE2/2352") {
        track.bIsData = true;
        track.blockSize = 2352;
        track.blockPayloadOffset = 24;
        track.blockPayloadSize = 2048;
    }
    else if (mode == "MODE2/2336") {
        track.bIsData = true;
        track.blockSize = 2352;
        track.blockPayloadOffset = 8;
        track.blockPayloadSize = 2048;
    }
    else if (mode == "MODE2/2324") {
        track.bIsData = true;
        track.blockSize = 2352;
        track.blockPayloadOffset = 24;
        track.blockPayloadSize = 2324;
    }
    else if (mode == "AUDIO") {
        track.bIsData = false;
        track.blockSize = 2352;
        track.blockPayloadOffset = 0;
        track.blockPayloadSize = 2352;
    }
    else {
        ctx.errorMsg = "Track mode not supported: ";
        ctx.errorMsg += mode;
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a 'INDEX' command. E.G: INDEX 0 05:01:39
//------------------------------------------------------------------------------------------------------------------------------------------
static bool parseCueCmd_Index(CueParseCtx& ctx) noexcept {
    // There must be a valid track to have an index
    if (ctx.disc.tracks.empty()) {
        ctx.errorMsg = "'INDEX' command issued without there being a track defined beforehand!";
        return false;
    }

    // Make sure the command is formatted validly
    std::smatch matches;
    std::regex_search(ctx.line, matches, gRegexCmd_Index);

    if (matches.size() != 5) {
        ctx.errorMsg = "Invalid 'INDEX' command format: ";
        ctx.errorMsg += ctx.line;
        ctx.errorMsg += "\nExpected: INDEX <INDEX_NUMBER> <MIN>:<SEC>:<FRAME>";
        return false;
    }

    try {
        // Get the index we are dealing with
        const int32_t indexNum = std::stoi(matches[1]);

        // Figure out which sector/lba this index refers to
        const int32_t min = std::stoi(matches[2]);
        const int32_t sec = std::stoi(matches[3]);
        const int32_t frame = std::stoi(matches[4]);

        const DiscPos discPos = { min, sec, frame };
        const int32_t indexLba = discPos.toLba();

        // Populate the track data
        DiscTrack& track = ctx.disc.tracks.back();

        if (indexNum == 0) {
            track.index0 = indexLba;
        } else if (indexNum == 1) {
            track.index1 = indexLba;
        }
    }
    catch (...) {
        ctx.errorMsg = "Failed to parse the index, minute, second or frame for an INDEX command. Command was: ";
        ctx.errorMsg += ctx.line;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse a single line in the .cue file: each command is restricted to one line
//------------------------------------------------------------------------------------------------------------------------------------------
static bool parseCueLine(CueParseCtx& ctx) noexcept {
    // Note: unsupported command lines or 'REM' lines are simply ignored
    if (std::regex_search(ctx.line, gRegexCmdBeg_File)) {
        return parseCueCmd_File(ctx);
    }
    else if (std::regex_search(ctx.line, gRegexCmdBeg_Track)) {
        return parseCueCmd_Track(ctx);
    }
    else if (std::regex_search(ctx.line, gRegexCmdBeg_Index)) {
        return parseCueCmd_Index(ctx);
    }
    else if (std::regex_search(ctx.line, gRegexCmdBeg_Pregap)) {
        ctx.errorMsg = "PREGAP commands are not supported in .cue files! Please rip without those.";
        return false;
    }
    else if (std::regex_search(ctx.line, gRegexCmdBeg_Postgap)) {
        ctx.errorMsg = "POSTGAP commands are not supported in .cue files! Please rip without those.";
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses all of the entries in the .cue file and does nothing else (no information inferrence)
//------------------------------------------------------------------------------------------------------------------------------------------
static bool parseCue(DiscInfo& disc, const char* const str, const char* const cueBasePath, std::string& errorMsg) noexcept {
    // Parse the .cue line by line
    CueParseCtx ctx = { disc };
    ctx.cueBasePath = cueBasePath;
    ctx.line.reserve(256);

    const char* const strEnd = str + std::strlen(str);
    const char* lineBeg = skipNewlinesAndSpace(str);

    while (lineBeg < strEnd) {
        // Figure out the line end and parse the entire line
        const char* const lineEnd = findNextNewline(lineBeg + 1);
        ctx.line.assign(lineBeg, lineEnd);

        if (!parseCueLine(ctx)) {
            errorMsg = std::move(ctx.errorMsg);
            return false;
        }

        lineBeg = skipNewlinesAndSpace(lineEnd);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Infer missing or not explicitly specified information in the .cue file.
// Note: this function assumes the track list has been sorted before calling.
//------------------------------------------------------------------------------------------------------------------------------------------
static void inferMissingCueInfo(DiscInfo& disc) noexcept {
    // Infer index 1 (data start) for the first track as the beginning of the file if not specified
    if (disc.tracks.size() > 0) {
        DiscTrack& firstTrack = disc.tracks[0];

        if (firstTrack.index1 < 0) {
            firstTrack.index1 = 0;
        }
    }

    // Infer missing index 0 (pre-gap start) for all tracks as being the same as index 1.
    // This means a zero sized pre-gap or no pre-gap.
    for (DiscTrack& track : disc.tracks) {
        if (track.index0 < 0) {
            track.index0 = track.index1;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine the file offsets and sizes (in blocks) for each track.
// Also validates the offsets and sizes and returns 'false' if that validation fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool determineTrackOffsetsAndSizes(DiscInfo& disc, std::string& errorMsg) noexcept {
    // Validate firstly that index '1' is defined for all tracks.
    // Note that if index 1 is defined, index 0 is also guaranteed to be defined.
    for (DiscTrack& track : disc.tracks) {
        if (track.index1 < 0) {
            errorMsg = "Track number ";
            errorMsg += std::to_string(track.trackNum);
            errorMsg += " does NOT define INDEX 1! Can't tell where the track data starts as a result.";
            return false;
        }
    }

    // Validate that the indexes for all the tracks are in range
    for (DiscTrack& track : disc.tracks) {
        const int64_t fileSizeInBlocks = track.sourceFileTotalSize / track.blockSize;

        // Show INDEX 1 errors first because oftentimes INDEX 0 is inferred from INDEX 1.
        // Error messages might be more relevant if we do this.
        if ((track.index1 < 0) || (track.index1 > fileSizeInBlocks)) {
            errorMsg = "Track number ";
            errorMsg += std::to_string(track.trackNum);
            errorMsg += " has an invalid INDEX 1 which falls outside of the track's data file!\nINDEX 1 sector index = ";
            errorMsg += std::to_string(track.index1);
            errorMsg += "; Track data file size (in sectors) = ";
            errorMsg += std::to_string(fileSizeInBlocks);
            return false;
        }

        if ((track.index0 < 0) || (track.index0 > fileSizeInBlocks)) {
            errorMsg = "Track number ";
            errorMsg += std::to_string(track.trackNum);
            errorMsg += " has an invalid INDEX 0 which falls outside of the track's data file!\nINDEX 0 sector index = ";
            errorMsg += std::to_string(track.index0);
            errorMsg += "; Track data file size (in sectors) = ";
            errorMsg += std::to_string(fileSizeInBlocks);
            return false;
        }
    }

    // Determine the file offset and size for each track
    for (size_t i = 0; i < disc.tracks.size(); ++i) {
        // Offset is where 'index 1' starts:
        DiscTrack& track = disc.tracks[i];
        track.fileOffset = track.index1 * track.blockSize;

        // End of track marker is where 'index 0' for the next track in the same file starts.
        // If there is no such track, then we instead make it the end of the file:
        int32_t endLba = (int32_t)(track.sourceFileTotalSize / track.blockSize);

        for (size_t j = i + 1; j < disc.tracks.size(); ++j) {
            DiscTrack& otherTrack = disc.tracks[j];

            if (track.sourceFilePath == otherTrack.sourceFilePath) {
                endLba = otherTrack.index0;
                break;
            }
        }

        track.blockCount = endLba - track.index1;
        track.trackPhysicalSize = track.blockCount * track.blockSize;
        track.trackPayloadSize = track.blockCount * track.blockPayloadSize;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a particular track in the disc
//------------------------------------------------------------------------------------------------------------------------------------------
DiscTrack* DiscInfo::getTrack(int32_t trackNum) noexcept {
    for (DiscTrack& track : tracks) {
        if (track.trackNum == trackNum)
            return &track;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get a particular track in the disc
//------------------------------------------------------------------------------------------------------------------------------------------
const DiscTrack* DiscInfo::getTrack(int32_t trackNum) const noexcept {
    for (const DiscTrack& track : tracks) {
        if (track.trackNum == trackNum)
            return &track;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse the .cue from the given string
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscInfo::parseFromCueStr(const char* const str, const char* const cueBasePath, std::string& errorMsg) noexcept {
    // Clear the current track list and parse the .cue firstly
    tracks.clear();

    if (!parseCue(*this, str, cueBasePath, errorMsg))
        return false;

    // Ensure the tracks are in sorted track number order for the .cue
    std::sort(
        tracks.begin(),
        tracks.end(),
        [](const DiscTrack& t1, const DiscTrack& t2) noexcept { return (t1.trackNum < t2.trackNum); }
    );

    // Infer various .cue information that wasn't explicitly specified
    inferMissingCueInfo(*this);

    // Determine the file offset and size of each track's data
    if (!determineTrackOffsetsAndSizes(*this, errorMsg))
        return false;

    // All good if we've reached here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse the .cue from the given file; if there is an error 'false' is returned and an error message potentially set
//------------------------------------------------------------------------------------------------------------------------------------------
bool DiscInfo::parseFromCueFile(const char* const filePath, std::string& errorMsg) noexcept {
    // Note: all paths specified in the .cue file will be relative to the .cue file itself.
    // Determine that base folder to which all paths are relative to here:
    std::string cueBasePath;
    FileUtils::getParentPath(filePath, cueBasePath);

    // Read the entire .cue file into ram and parse its contents
    FileData cueFile = FileUtils::getContentsOfFile(filePath, 8, std::byte(0));

    if (cueFile.bytes.get()) {
        return parseFromCueStr((char*) cueFile.bytes.get(), cueBasePath.c_str(), errorMsg);
    } else {
        errorMsg = "Failed to open/read the .cue file '";
        errorMsg += filePath;
        errorMsg += "'";

        return false;
    }
}

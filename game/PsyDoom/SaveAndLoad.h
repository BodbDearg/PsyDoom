#pragma once

#include "Macros.h"

#include <unordered_map>

class InputStream;
class OutputStream;
struct mobj_t;

// Enum representing the result of reading a save file
enum class ReadSaveResult : int32_t {
    OK,             // Read was OK
    BAD_FILE_ID,    // File ID was invalid
    BAD_VERSION,    // Save file format is not supported (invalid version)
    BAD_MAP_NUM,    // Invalid map number
    IO_ERROR        // General read error, not enough data, IO error etc.
};

// Enum representing the result of loading a save file
enum class LoadSaveResult : int32_t {
    OK,             // Load was OK
    BAD_MAP_HASH,   // Map file has changed, not the same map the game was saved with
    BAD_DATA        // Certain validation checks failed
};

BEGIN_NAMESPACE(SaveAndLoad)

extern std::unordered_map<mobj_t*, int32_t>     gMobjToIdx;
extern std::vector<mobj_t*>                     gMobjList;

bool save(OutputStream& out) noexcept;
ReadSaveResult read(InputStream& in) noexcept;
LoadSaveResult load() noexcept;

END_NAMESPACE(SaveAndLoad)

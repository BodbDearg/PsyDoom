#pragma once

#include "Macros.h"

#include <string>
#include <unordered_map>
#include <vector>

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
    BAD_MAP_DATA    // Certain map data validation checks failed
};

// Enum corresponding to a save file slot
enum class SaveFileSlot : uint8_t {
    SAVE1,
    SAVE2,
    SAVE3,
    QUICKSAVE,
    AUTOSAVE,
    NONE = 255,     // Indicates no slot
};

BEGIN_NAMESPACE(SaveAndLoad)

extern std::unordered_map<mobj_t*, int32_t>     gMobjToIdx;
extern std::vector<mobj_t*>                     gMobjList;
extern SaveFileSlot                             gCurSaveSlot;

bool save(OutputStream& out) noexcept;
ReadSaveResult read(InputStream& in) noexcept;
LoadSaveResult load() noexcept;
const char* getSaveFileName(const SaveFileSlot slot) noexcept;
std::string getSaveFilePath(const SaveFileSlot slot) noexcept;
void clearBufferedSave() noexcept;
int32_t getBufferedSaveMapNum() noexcept;

END_NAMESPACE(SaveAndLoad)

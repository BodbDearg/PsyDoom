//------------------------------------------------------------------------------------------------------------------------------------------
// A developer feature that automatically does an 'in-place' reload of the current map if all of the following criteria are true:
// 
//  (1) The game mode is single player.
//  (2) The current map being played is sourced from a real file on disc, via the file overrides mechanism.
//      Changes to files within a CD image are NOT monitored.
//  (3) The auto-reload feature is enabled.
//  (4) The file has been modified since we last checked.
//  (5) The platform is Windows. This feature is currently not supported on MacOS due to the '<filesystem>'
//      API not being available until later OS versions.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "DevMapAutoReloader.h"

#include "Cheats.h"
#include "Config.h"
#include "Doom/cdmaptbl.h"
#include "Doom/Game/g_game.h"
#include "ModMgr.h"
#include "ProgArgs.h"

#include <string>

// Is the auto reloader available on this platform?
#if __APPLE__
    #define ENABLE_MAP_AUTO_RELOADER 0
#else
    #define ENABLE_MAP_AUTO_RELOADER 1
    #include <filesystem>
#endif

BEGIN_NAMESPACE(DevMapAutoReloader)

#if ENABLE_MAP_AUTO_RELOADER
    static std::filesystem::path            gMapFilePath;
    static std::filesystem::file_time_type  gLastMapFileModifiedTime;
#endif

#if ENABLE_MAP_AUTO_RELOADER
//------------------------------------------------------------------------------------------------------------------------------------------
// Get the 'modified' timestamp for the current map file.
// Returns the default timestamp on an error.
//------------------------------------------------------------------------------------------------------------------------------------------
static std::filesystem::file_time_type queryMapFileModifiedTime() noexcept {
    try {
        return std::filesystem::last_write_time(gMapFilePath);
    } catch (...) {
        return {};
    }
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the map auto-reloader.
// This should be called when the map is initially loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
void init([[maybe_unused]] const CdFileId mapWadFile) noexcept {
    #if ENABLE_MAP_AUTO_RELOADER
        // Ignore if this feature is disabled
        if (!Config::gbEnableDevMapAutoReload)
            return;

        // Ignore the call if the current game mode is multiplayer
        if (gNetGame != gt_single)
            return;

        // No overrides for this wad file? The file watched must be a real file on disk.
        if (!ModMgr::areOverridesAvailableForFile(mapWadFile))
            return;

        // Make up the path to the overriden file
        gMapFilePath = ProgArgs::gDataDirPath;
        gMapFilePath.append(gCdMapTblFileNames[(uint32_t) mapWadFile]);

        // Get the current modified timestamp
        gLastMapFileModifiedTime = queryMapFileModifiedTime();
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the map auto-reloader.
// This should be called when the map is finished.
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    #if ENABLE_MAP_AUTO_RELOADER
        gMapFilePath.clear();
        gLastMapFileModifiedTime = {};
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This should be called periodically by the game loop.
// It will trigger a reload of the current map if appropriate.
//------------------------------------------------------------------------------------------------------------------------------------------
void update() noexcept {
    #if ENABLE_MAP_AUTO_RELOADER
        // If we are not watching anything then bail out now
        if (gMapFilePath.empty())
            return;

        // Check for the map file being modified and start an in-place reload of the map if it's changed
        const std::filesystem::file_time_type modifiedTime = queryMapFileModifiedTime();

        if (modifiedTime > gLastMapFileModifiedTime) {
            gLastMapFileModifiedTime = modifiedTime;
            Cheats::doInPlaceReloadCheat();
        }
    #endif
}

END_NAMESPACE(DevMapAutoReloader)

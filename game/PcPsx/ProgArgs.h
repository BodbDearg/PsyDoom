#pragma once

#include "Macros.h"
#include <cstdint>

class WadList;

BEGIN_NAMESPACE(ProgArgs)

extern const char*  gCueFileOverride;
extern bool         gbHeadlessMode;
extern const char*  gDataDirPath;
extern const char*  gPlayDemoFilePath;
extern const char*  gSaveDemoResultFilePath;
extern const char*  gCheckDemoResultFilePath;
extern bool         gbIsNetServer;
extern bool         gbIsNetClient;
extern uint16_t     gServerPort;
extern bool         gbNoMonsters;
extern bool         gbPistolStart;
extern bool         gbTurboMode;

void init(const int argc, const char** const argv) noexcept;
void shutdown() noexcept;
const char* getServerHost() noexcept;
void addWadArgsToList(WadList& wadList) noexcept;

END_NAMESPACE(ProgArgs)

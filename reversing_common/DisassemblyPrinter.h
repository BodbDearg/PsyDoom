#pragma once

#include "CpuOpcode.h"
#include <ostream>

//----------------------------------------------------------------------------------------------------------------------
// Module that prints the disassembly for a module.
// Uses the program elements defined, which identify what particular regions of the .EXE mean.
//----------------------------------------------------------------------------------------------------------------------
struct ExeFile;

namespace DisassemblyPrinter {
    void printExe(const ExeFile& exe, std::ostream& out) noexcept;
}

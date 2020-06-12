#include "DisassemblyPrinter.h"
#include "ExeFile.h"
#include "FatalErrors.h"
#include "ProgElems.h"
#include "PseudoCppPrinter.h"

#include <cstdio>
#include <fstream>

//------------------------------------------------------------------------------------------------------------------------------------------
// Entry point for 'DoomDisassemble'
//------------------------------------------------------------------------------------------------------------------------------------------
// Program purpose:
//
//  This program functions as a sort of primitive 'interactive' disassembler specifically (and ONLY!) for the 
//  following versions of PSXDOOM.EXE:
//
//      DOOM (Greatest Hits Version)    - US, NTSC-U (SLUS-00077) (Size: 428,032 bytes  MD5: fc9a10f36e6a4f6d5933c13ab77d3b06)
//      Final Doom                      - US, NTSC-U (SLUS-00331) (Size: 430,080 bytes  MD5: 0e31dffa3c367e5ffae9e0aa513bb095)
//
//  Basically you manually specify what each part of the EXE means, whether it is code and data, assign comments and
//  friendly names etc. and then the assembler can generate an annotated listing of the contents of the .EXE as well as
//  auto generated C++ files based on the diasassembly.
//
//  The 'interactive' part of this disassembly process comes from adjusting the input information and annotations based
//  on investigation, debugging and reverse engineering etc. This process is completed until more and more of the EXE is
//  known and understood, and this work can be the basis for translation to higher level C/C++ code.
//
//------------------------------------------------------------------------------------------------------------------------------------------
// Output location:
//
//  The program will attempt to output to whatever directory is the current working directory.
//------------------------------------------------------------------------------------------------------------------------------------------

// Number of 32-bit words in the above versions of PSX DOOM and Final DOOM.
// Used for very basic verification and also to tell whether we are dealing with DOOM or Final DOOM.
static constexpr uint32_t DOOM_NUM_PROG_WORDS = 106496;
static constexpr uint32_t FINAL_DOOM_NUM_PROG_WORDS = 107008;

int main(int argc, char* argv[]) noexcept {
    // *MUST* specify the path to PSXDOOM.EXE!
    if (argc != 2) {
        std::printf("Usage: DoomDisassemble <Path to the US/NTSC Playstation DOOM or Final Doom .EXE>\n");
        return 1;
    }

    // Disasemble the Japanese version of Destruction Derby if specified.
    // Using this to try and figure out some PSYQ functions, since it comes with debug symbols and was released around a similar time.
    const bool bIsDestructionDerby = (std::strstr(argv[1], "DEMOLISH.EXE") != 0);

    // Load the exe and verify that it is either DOOM or Final DOOM
    const char* const psxDoomExePath = argv[1];
    ExeFile exe;
    exe.loadFromFile(psxDoomExePath);

    // Verify that it is Final DOOM or DOOM if doing that
    if (!bIsDestructionDerby) {
        if (exe.sizeInWords != DOOM_NUM_PROG_WORDS && exe.sizeInWords != FINAL_DOOM_NUM_PROG_WORDS) {
            FATAL_ERROR_F("The given PSX DOOM .EXE file '%s' does not appear to be the US/NTSC version of PSX DOOM or Final DOOM!", psxDoomExePath);
        }
    }

    // Set the program elements for the .EXE and determine references to specific program words
    if (bIsDestructionDerby) {
        exe.setProgElems(gpProgramElems_DestructionDerby, gNumProgramElems_DestructionDerby);
        exe.assumedGpRegisterValue = gGpRegisterValue_DestructionDerby;
    } else {
        const bool bIsFinalDoom = (exe.sizeInWords == FINAL_DOOM_NUM_PROG_WORDS);

        if (bIsFinalDoom) {
            FATAL_ERROR("TODO: FINAL DOOM NOT SUPPORTED YET!");
        } else {
            exe.setProgElems(gpProgramElems_Doom, gNumProgramElems_Doom);
            exe.setJRInstHandlers(gpJRInstHandlers_Doom, gNumJRInstHandlers_Doom);
            exe.assumedGpRegisterValue = gGpRegisterValue_Doom;
        }
    }

    exe.determineWordReferences();

    // Start printing the disassembly
    try {
        std::fstream fileOut;
        const char* const pFileName = (bIsDestructionDerby) ? "disasm_dd_disasm.txt" : "disasm_doom_disasm.txt";
        fileOut.open(pFileName, std::fstream::out);
        DisassemblyPrinter::printExe(exe, fileOut);
    } catch (...) {
        FATAL_ERROR("Failed writing the disassembly to the output file!");
    }

    // Start printing the .cpp file
    try {
        std::fstream fileOut;
        fileOut.open("disasm_doom.cpp", std::fstream::out);
        PseudoCppPrinter::printCpp(exe, fileOut);
    } catch (...) {
        FATAL_ERROR("Failed writing the pseudo c++ code to the output file!");
    }

    return 0;
}

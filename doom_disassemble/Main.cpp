#include "ExeFile.h"
#include "FatalErrors.h"
#include <cstdio>

// TODO: TEMP - REMOVE
#include "CpuInstruction.h"
#include "PrintUtils.h"
#include <sstream>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
// Entry point for 'DoomDisassemble'
//----------------------------------------------------------------------------------------------------------------------
// Program purpose:
//
//  This program functions as a sort of primitive 'interactive' disassembler specifically (and ONLY!) for the 
//  following versions of PSXDOOM.EXE:
//
//      DOOM        - US, NTSC-U (SLUS-00077) (Size: 428,032 bytes  MD5: fc9a10f36e6a4f6d5933c13ab77d3b06)
//      Final Doom  - US, NTSC-U (SLUS-00331) (Size: 430,080 bytes  MD5: 0e31dffa3c367e5ffae9e0aa513bb095)
//
//  Basically you manually specify what each part of the EXE means, whether it is code and data, assign comments and
//  friendly names etc. and then the assembler can generate an annotated listing of the contents of the .EXE as well as
//  auto generated C++ files based on the diasassembly.
//
//  The 'interactive' part of this disassembly process comes from adjusting the input information and annotations based
//  on investigation, debugging and reverse engineering etc. This process is completed until more and more of the EXE is
//  known and understood, and this work can be the basis for translation to higher level C/C++ code.
//
//----------------------------------------------------------------------------------------------------------------------
// Output location:
//
//  The program will attempt to output to whatever directory is the current working directory.
//----------------------------------------------------------------------------------------------------------------------

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

    // Load the exe and verify that it is either DOOM or Final DOOM
    const char* const psxDoomExePath = argv[1];
    ExeFile exe;
    exe.loadFromFile(psxDoomExePath);

    // Verify that it is Final DOOM or DOOM
    if (exe.sizeInWords != DOOM_NUM_PROG_WORDS && exe.sizeInWords != FINAL_DOOM_NUM_PROG_WORDS) {
        FATAL_ERROR_F("The given PSX DOOM .EXE file '%s' does not appear to be the US/NTSC version of PSX DOOM or Final DOOM!", psxDoomExePath);
    }

    const bool bIsFinalDoom = (exe.sizeInWords == FINAL_DOOM_NUM_PROG_WORDS);

    // TODO: REMOVE - TEMP TEST
    {
        FILE* const pFile = std::fopen("disasm_doom_disasm.txt", "w");

        if (!pFile) {
            FATAL_ERROR_F("Can't open output file to write disassembly to!");
        }

        std::stringstream curLine;

        for (uint32_t progWord = 0; progWord < exe.sizeInWords; ++progWord) {
            curLine.str(std::string());

            // Print the address of the data
            const uint32_t addr = exe.baseAddress + progWord * 4;
            PrintUtils::printHexU32(addr, true, curLine);
            curLine << "    ";

            // Print the U32 value
            const uint32_t word = exe.words[progWord].value;
            PrintUtils::printHexU32(word, true, curLine);
            curLine << "  ";

            // Print the individual bytes in memory order (assuming little endian!)
            const uint8_t wordByte1 = (uint8_t)((word >> 0 ) & 0xFFu);
            const uint8_t wordByte2 = (uint8_t)((word >> 8 ) & 0xFFu);
            const uint8_t wordByte3 = (uint8_t)((word >> 16) & 0xFFu);
            const uint8_t wordByte4 = (uint8_t)((word >> 24) & 0xFFu);

            {
                PrintUtils::printHexU8(wordByte1, true, curLine); curLine.put(' ');
                PrintUtils::printHexU8(wordByte2, true, curLine); curLine.put(' ');
                PrintUtils::printHexU8(wordByte3, true, curLine); curLine.put(' ');
                PrintUtils::printHexU8(wordByte4, true, curLine);
            }

            // Print the ascii value of each individual byte.
            // Don't print control characters or tabs and newlines, replace them with space.
            curLine << "  ";

            {
                curLine.put((wordByte1 >= 32 && wordByte1 <= 126) ? (char) wordByte1 : ' ');
                curLine.put((wordByte2 >= 32 && wordByte2 <= 126) ? (char) wordByte2 : ' ');
                curLine.put((wordByte3 >= 32 && wordByte3 <= 126) ? (char) wordByte3 : ' ');
                curLine.put((wordByte4 >= 32 && wordByte4 <= 126) ? (char) wordByte4 : ' ');
            }
            
            // Print the instruction itself
            curLine << "      ";

            CpuInstruction inst;
            inst.decode(exe.words[progWord].value);

            if (inst.isNOP()) {
                curLine << "  nop";
            } else {
                if (!CpuOpcodeUtils::isBranchOrJumpOpcode(inst.opcode)) {
                    curLine << "  ";
                }

                inst.print(addr, curLine);
            }

            std::fprintf(pFile, "%s\n", curLine.str().c_str());
        }

        std::fflush(pFile);
        std::fclose(pFile);
    }

    return 0;
}

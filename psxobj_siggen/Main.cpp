#include "ObjFileData.h"
#include "ObjFileParser.h"
#include <cstdio>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
// Entry point for 'PSXObjSigGen'
//----------------------------------------------------------------------------------------------------------------------
// Program purpose:
//
//  This program parses the human readable textual output of the 'DUMPOBJ.EXE' tool included in the PlayStation SDK 
//  (PsyQ SDK) when invoked with the '/c' option (print code and data). Based on this dump output, it generates function
//  signatures for the contents of the .OBJ file as well as reference disassembly for code.
//
//  These signatures are then appended to specified output files, so the results of many invocations can be gathered
//  together to form one database of all this info. This database can then be used to try and identify the same
//  functions within a given PlayStation executable.
//
//  Basically this tool was created so we can identify PsyQ SDK constructs within a PlayStation executable.
//
//  The process to using this tool would be as follows:
//      (1) Dump the contents of all SDK .LIB files to .OBJ using the 'PSYLIB.EXE' tool and gather other SDK .OBJ files.
//          For more info, see: https://www.retroreversing.com/ps1-psylink
//      (2) Invoke 'DUMPOBJ.EXE' on every SDK .OBJ file to produce the textual output that is an input to this tool.
//      (3) Use this tool on all the dump output to build up the signature database.
//      (4) Run pattern matching on the .EXE using the generated signature database to identify PsyQ functions in an EXE.
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Read the file at the given path into the given string.
// Returns false on failure.
//----------------------------------------------------------------------------------------------------------------------
static bool readFileAsString(const char* const filePath, std::string& out) noexcept {
    std::FILE* const pFile = std::fopen(filePath, "rb");

    if (!pFile)
        return false;
    
    bool success = false;

    do {
        if (std::fseek(pFile, 0, SEEK_END) != 0)
            break;

        long size = std::ftell(pFile);

        if (size < 0 || size > INT32_MAX)
            break;

        if (std::fseek(pFile, 0, SEEK_SET) != 0)
            break;

        if (size > 0) {
            out.resize((uint32_t) size);

            if (std::fread(out.data(), (uint32_t) size, 1, pFile) != 1) {
                out.clear();
                break;
            }
        }

        success = true;
    } while (0);

    std::fclose(pFile);
    return success;
}

int main(int argc, char* argv[]) noexcept {
    if (argc < 2) {
        // TODO: PRINT ARGS
        return 1;
    }

    // Read the input file
    const char* const inputFilePath = argv[1];
    std::string inputFileStr;

    if (!readFileAsString(inputFilePath, inputFileStr)) {
        std::printf("Failed to read input file '%s'!\n", inputFilePath);
        return 1;
    }

    // Parse the input file
    ObjFile objFile;

    if (!ObjFileParser::parseObjFileDumpFromStr(inputFileStr, objFile)) {
        return 1;
    }

    return 0;
}

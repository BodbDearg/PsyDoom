#include "FatalErrors.h"
#include "FileUtils.h"
#include "FuncSignature.h"
#include "ObjFileData.h"
#include "ObjFileParser.h"
#include <cstdio>
#include <fstream>

//------------------------------------------------------------------------------------------------------------------------------------------
// Entry point for 'PSXObjSigGen'
//------------------------------------------------------------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) noexcept {
    if (argc != 3 && argc != 4) {        
        FATAL_ERROR_F("Usage: %s <INPUT_OBJ_FILE_DUMP> <OUT_SIGNATURE_FILE> [OUT_DISASM_FILE]\n", argv[0]);
    }

    // Read the input file
    const char* const inputFilePath = argv[1];
    std::string inputFileStr;

    if (!FileUtils::readFileAsString(inputFilePath, inputFileStr)) {
        FATAL_ERROR_F("Failed to read input file '%s'!\n", inputFilePath);
    }

    // Parse the input file
    ObjFile objFile;

    if (!ObjFileParser::parseObjFileDumpFromStr(inputFileStr, objFile)) {
        FATAL_ERROR_F("Failed to parse input file '%s'!\n", inputFilePath);
    }

    // Build a list of function signatures for export
    std::vector<FuncSignature> signatures;
    signatures.reserve(objFile.symbols.size());
    FuncSignatureUtils::buildSigList(objFile, signatures);

    // Print the signatures to the specified file
    try {
        std::fstream out;
        out.open(argv[2], std::fstream::out | std::fstream::app);
        FuncSignatureUtils::printSigList(signatures, out);
    } catch (...) {
        FATAL_ERROR_F("Failed appending the function signatures to the specified output file '%s'!", argv[2]);
    }

    // Optionally print disassembly to the specified file
    if (argc >= 4) {
        try {
            std::fstream out;
            out.open(argv[3], std::fstream::out | std::fstream::app);
            FuncSignatureUtils::printSigListDisassembly(signatures, out);
        } catch (...) {
            FATAL_ERROR_F("Failed appending function disassembly to the specified output file '%s'!", argv[3]);
        }
    }

    return 0;
}

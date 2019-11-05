#include "ObjFileData.h"
#include "ObjFileParser.h"
#include <algorithm>
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

//----------------------------------------------------------------------------------------------------------------------
// Describes a function signature for exporting to a file
//----------------------------------------------------------------------------------------------------------------------
struct FuncSignature {
    std::string             name;
    std::vector<uint32_t>   instructions;
    std::vector<uint32_t>   wildcardInstructionIndexes;
};

//----------------------------------------------------------------------------------------------------------------------
// Build the list of function signatures to export
//----------------------------------------------------------------------------------------------------------------------
static void buildFuncSignaturesList(ObjFile& objFile, std::vector<FuncSignature>& signatures) noexcept {
    // Sort symbols by section and then offset
    std::vector<ObjSymbol>& symbols = objFile.symbols;

    std::sort(
        symbols.begin(),
        symbols.end(),
        [](const ObjSymbol& s1, const ObjSymbol& s2) noexcept {
            if (s1.defSection != s2.defSection) {
                return (s1.defSection < s2.defSection);
            }

            if (s1.defOffset != s2.defOffset) {
                return (s1.defOffset < s2.defOffset);
            }

            return false;
        }
    );

    // Run through all symbols that we might need to export and get their code.
    // Will apply patches later!
    for (uint32_t symbolIdx = 0; symbolIdx < symbols.size(); ++symbolIdx) {
        // Ignore the symbol if external
        const ObjSymbol& symbol = symbols[symbolIdx];

        if (symbol.isExternal()) {
            continue;
        }

        // Ignore the symbol if not in the code section
        const ObjSection* pSection = objFile.getSectionWithNum(symbol.defSection);

        if ((!pSection) || (pSection->type != ObjSectionType::TEXT)) {
            continue;
        }

        // Determine the end offset where this symbol ends.
        // It's either at the end of the section data or at the start of the next symbol.
        const std::vector<std::byte>& sectionData = pSection->data;
        const uint32_t sectionEndOffset = (uint32_t) sectionData.size() * 4;
        uint32_t endOffset = sectionEndOffset;

        if ((size_t) symbolIdx + 1 < symbols.size()) {
            const ObjSymbol& nextSymbol = symbols[(size_t) symbolIdx + 1];

            if (!nextSymbol.isExternal()) {
                if (nextSymbol.defSection == symbol.defSection) {
                    endOffset = nextSymbol.defOffset;
                }
            }
        }

        // Assuming the start and end offsets are both aligned and in range
        const bool bInvalidOffsets = (
            (symbol.defOffset % 4 != 0) ||
            (endOffset % 4 != 0) ||
            (symbol.defOffset >= endOffset) ||
            (symbol.defOffset >= sectionEndOffset) ||
            (endOffset > sectionEndOffset)
        );

        if (bInvalidOffsets) {
            std::printf("Failed to generate signatures for function '%s' due to invalid start/end offsets!", symbol.name.c_str());
            std::exit(1);
        }

        // Alloc signature object and copy in it's instructions
        const uint32_t numInstructions = (endOffset - symbol.defOffset) / sizeof(uint32_t);

        FuncSignature& sig = signatures.emplace_back();
        sig.name = symbol.name;
        sig.instructions.resize(numInstructions);
        std::memcpy(sig.instructions.data(), sectionData.data() + symbol.defOffset, numInstructions * sizeof(uint32_t));

        // Apply any wildcards to instructions as indicated by patches
        for (const ObjPatch& patch : pSection->patches) {
            if (patch.targetOffset >= symbol.defOffset && patch.targetOffset < endOffset) {
                const uint32_t patchedInstructionIdx = (patch.targetOffset - symbol.defOffset) / sizeof(uint32_t);
                sig.wildcardInstructionIndexes.push_back(patchedInstructionIdx);
            }
        }
    }
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

    // Build a list of function signatures for export
    std::vector<FuncSignature> signatures;
    signatures.reserve(objFile.symbols.size());
    buildFuncSignaturesList(objFile, signatures);
    return 0;
}

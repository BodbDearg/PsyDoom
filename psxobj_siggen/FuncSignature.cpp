#include "FuncSignature.h"

#include "FatalErrors.h"
#include "ObjFileData.h"
#include "PrintUtils.h"
#include <algorithm>

void FuncSignatureUtils::buildSigList(ObjFile& objFile, std::vector<FuncSignature>& signatures) noexcept {
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
        const uint32_t sectionEndOffset = sectionData.size();
        uint32_t endOffset = sectionEndOffset;

        if ((size_t) symbolIdx + 1 < symbols.size()) {
            const ObjSymbol& nextSymbol = symbols[(size_t) symbolIdx + 1];

            if (!nextSymbol.isExternal()) {
                if (nextSymbol.defSection == symbol.defSection) {
                    endOffset = nextSymbol.defOffset;
                }
            }

            // If the next symbol is at the same location then skip exporting this symbol!
            if (nextSymbol.defOffset == symbol.defOffset)
                continue;
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
            FATAL_ERROR_F("Failed to generate signatures for function '%s' due to invalid start/end offsets!", symbol.name.c_str());
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

void FuncSignatureUtils::printSigList(const std::vector<FuncSignature>& signatures, std::ostream& out) noexcept {
    for (const FuncSignature& sig : signatures) {
        out << sig.name << "\n\t";
        out << sig.instructions.size() << "\n\t";
        out << sig.wildcardInstructionIndexes.size() << "\n\t";

        for (const uint32_t instruction : sig.instructions) {
            PrintUtils::printHexU32(instruction, true, out);
            out << " ";
        }

        out << "\n\t";

        for (const uint32_t index : sig.wildcardInstructionIndexes) {
            out << index;
            out << " ";
        }

        out << "\n";
    }
}

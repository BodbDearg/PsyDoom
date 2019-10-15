#include "DisassemblyPrinter.h"

#include "CpuInstruction.h"
#include "ExeFile.h"
#include "FatalErrors.h"
#include "PrintUtils.h"
#include <algorithm>

static void printAddressForLine(const uint32_t addr, std::ostream& out) noexcept {
    PrintUtils::printHexU32(addr, true, out);
    out << ":";
}

static void printProgWordReferences(const ExeWord& word, const ProgElem* const pInsideFunc, std::ostream& out) noexcept {
    if (word.bIsBranchTarget || word.bIsDataReferenced || word.bIsJumpTarget) {
        out << " <- ";
        uint32_t numWordReferences = 0;

        if (word.bIsJumpTarget) {
            out.put('J');
            ++numWordReferences;
        }

        if (word.bIsBranchTarget) {
            out.put('B');
            ++numWordReferences;
        }

        if (word.bIsDataReferenced) {
            out.put('R');
            ++numWordReferences;
        }

        if (pInsideFunc) {
            // Print 'I' if all references to the word are internal to the containing function
            if (word.startReferencingAddr != word.endReferencingAddr) {
                if (word.startReferencingAddr >= pInsideFunc->startAddr && word.endReferencingAddr <= pInsideFunc->endAddr) {
                    out.put('I');
                    ++numWordReferences;
                }
            }
        }

        for (uint32_t i = numWordReferences; i < 4; ++i) {
            out.put(' ');
        }
        
        out << "      ";
    } else {
        out << "              ";
    }
}

static void printProgElemName(const ProgElem& progElem, const char* const defaultNamePrefix, std::ostream& out) noexcept {
    if (progElem.name != nullptr && progElem.name[0] != 0) {
        out << progElem.name;
    } else {
        out << defaultNamePrefix;
        PrintUtils::printHexU32(progElem.startAddr, true, out);
    }
}

static void printProgInstruction(const ExeFile& exe, const ProgElem* const pParentFunc, const uint32_t instAddr, const uint32_t instWord, std::ostream& out) noexcept {
    // Firstly decode the instruction
    CpuInstruction inst;
    inst.decode(instWord);

    // Print 'nop' if the instruction is a nop, oth
    if (inst.isNOP()) {
        out << "  nop";
    } else {
        // If the instruction is NOT a branch or jump then indent it slightly.
        // This makes branches and jumps stand out more.
        // Control flow instructions are important for detecting function boundaries etc. !
        if (!CpuOpcodeUtils::isBranchOrJumpOpcode(inst.opcode)) {
            out << "  ";
        }

        inst.print(exe, instAddr, pParentFunc, out);
    }
}

static void printFunction(const ExeFile& exe, const ProgElem& progElem, std::ostream& out) noexcept {
    // Print the function title
    out << ";-----------------------------------------------------------------------------------------------------------------------\n";
    out << "; FUNC: ";
    printProgElemName(progElem, "unnamed_func_", out);
    out << "    (";
    PrintUtils::printHexU32(progElem.startAddr, true, out);
    out.put('-');
    PrintUtils::printHexU32(progElem.endAddr - 1, true, out);
    out << ")\n";
    out << ";-----------------------------------------------------------------------------------------------------------------------\n";

    // Validate that the function program element is correctly aligned
    const bool bIsFuncRangeAligned = (
        (progElem.startAddr % 4 == 0) &&
        (progElem.endAddr % 4 == 0) &&
        (progElem.endAddr > progElem.startAddr)
    );

    if (!bIsFuncRangeAligned) {
        FATAL_ERROR("Invalid function program element! The ranges of addresses specified are NOT 32-bit aligned or the function size is zero!");
    }

    // Figure out the start and end word for the function in the .EXE
    const uint32_t startWordIdx = (progElem.startAddr - exe.baseAddress) / 4;
    const uint32_t endWordIdx = (progElem.endAddr - exe.baseAddress) / 4;

    // Now print each instruction in the function
    for (uint32_t wordIdx = startWordIdx; wordIdx < endWordIdx; ++wordIdx) {
        // Print the address of this instruction and then print the instruction itself
        const uint32_t instAddr = exe.baseAddress + wordIdx * 4;
        printAddressForLine(instAddr , out);

        // Print the references to this exe word
        const ExeWord exeWord = exe.words[wordIdx];
        printProgWordReferences(exeWord, &progElem, out);

        // Print the instruction itself
        printProgInstruction(exe, &progElem, instAddr, exeWord.value, out);
        out.put('\n');
    }

    out.put('\n');
}

static void printArrayVariable(const ExeFile& exe, const ProgElem& progElem, std::ostream& out) noexcept {
    // Validate that the variable is correctly aligned, and that the size matches the variable type size
    const ProgElemType arrayElemType = progElem.arrayElemType;
    const uint32_t arrayElemTypeSize = getProgElemTypeSize(arrayElemType);

    const bool progElemAligned = (
        (arrayElemTypeSize > 0) &&
        (progElem.startAddr % arrayElemTypeSize == 0) &&
        (progElem.endAddr % arrayElemTypeSize == 0)
    );

    if (!progElemAligned) {
        FATAL_ERROR("Invalid array variable program element! The address range specified is not properly aligned for the type!");
    }

    // Get the number of array elements
    const uint32_t numArrayElems = (progElem.endAddr - progElem.startAddr) / arrayElemTypeSize; 

    // Print a comment above saying the address range because we don't show that:
    out << "\n; Range: ";
    PrintUtils::printHexU32(progElem.startAddr, true, out);
    out << " - ";
    PrintUtils::printHexU32(progElem.endAddr - 1, true, out);
    out.put('\n');

    // Print the type of the array variable and indent
    out << "    ";

    switch (arrayElemType) {
        case ProgElemType::INT32:       out << "i32";       break;
        case ProgElemType::UINT32:      out << "u32";       break;
        case ProgElemType::INT16:       out << "i16";       break;
        case ProgElemType::UINT16:      out << "u32";       break;
        case ProgElemType::INT8:        out << "i8";        break;
        case ProgElemType::UINT8:       out << "u8";        break;
        case ProgElemType::BOOL8:       out << "bool8";     break;
        case ProgElemType::CHAR8:       out << "string8";   break;
        case ProgElemType::PTR32:       out << "ptr32";     break;

        default:
            FATAL_ERROR("Unexpected case! We should never hit this!");
            break;
    }

    // Print the array size
    out.put('[');
    PrintUtils::printHexU32(numArrayElems, false, out);
    out << "] ";

    // Print the name of the array variable
    switch (arrayElemType) {
        case ProgElemType::INT32:       printProgElemName(progElem, "UNNAMED_array_i32_", out);     break;
        case ProgElemType::UINT32:      printProgElemName(progElem, "UNNAMED_array_u32_", out);     break;
        case ProgElemType::INT16:       printProgElemName(progElem, "UNNAMED_array_i16_", out);     break;
        case ProgElemType::UINT16:      printProgElemName(progElem, "UNNAMED_array_u16_", out);     break;
        case ProgElemType::INT8:        printProgElemName(progElem, "UNNAMED_array_i8_", out);      break;
        case ProgElemType::UINT8:       printProgElemName(progElem, "UNNAMED_array_u8_", out);      break;
        case ProgElemType::BOOL8:       printProgElemName(progElem, "UNNAMED_array_bool8_", out);   break;
        case ProgElemType::CHAR8:       printProgElemName(progElem, "UNNAMED_string8_", out);       break;
        case ProgElemType::PTR32:       printProgElemName(progElem, "UNNAMED_array_pt32_", out);    break;

        default:
            FATAL_ERROR("Unexpected case! We should never hit this!");
            break;
    }

    // Print array opening
    if (arrayElemType == ProgElemType::CHAR8) {
        out << " = \"";
    } else {
        out << " = {\n";
    }

    // Print each element in the array
    uint32_t arrayElemNum = 0;

    for (uint32_t addr = progElem.startAddr; addr < progElem.endAddr; addr += arrayElemTypeSize) {
        // Load the word for the variable
        const uint32_t wordIdx = (addr - exe.baseAddress) / 4;
        const ExeWord exeWord = exe.words[wordIdx];
    
        // Shift and/or mask the word's value if the size is less than 32-bits
        uint32_t elemVal = exeWord.value;

        if (arrayElemTypeSize != 4) {
            const uint32_t shift = 8 * (addr % 4);
            elemVal >>= shift;

            if (arrayElemTypeSize == 2) {
                elemVal &= 0xFFFFu;
            } else {
                elemVal &= 0xFFu;
            }
        }

        // Indent if not a string and if the first element in a line
        if (arrayElemType != ProgElemType::CHAR8) {
            const bool bDoIndent = (
                (progElem.arrayElemsPerLine <= 1) ||
                (arrayElemNum % progElem.arrayElemsPerLine == 0)
            );

            if (bDoIndent) {
                out << "        ";
            }
        }

        // Print the value itself
        switch (arrayElemType) {
            case ProgElemType::INT32:       PrintUtils::printHexI32((int32_t) elemVal, false, out);     break;
            case ProgElemType::UINT32:      PrintUtils::printHexU32(elemVal, false, out);               break;
            case ProgElemType::INT16:       PrintUtils::printHexI16((int16_t) elemVal, false, out);     break;
            case ProgElemType::UINT16:      PrintUtils::printHexU16((uint16_t) elemVal, false, out);    break;
            case ProgElemType::INT8:        PrintUtils::printHexI8((int8_t) elemVal, false, out);       break;
            case ProgElemType::UINT8:       PrintUtils::printHexU8((uint8_t) elemVal, false, out);      break;
            case ProgElemType::BOOL8:       PrintUtils::printBool((elemVal != 0), out);                 break;

            case ProgElemType::CHAR8:
                PrintUtils::printEscapedChar((char) elemVal, out);
                break;

            case ProgElemType::PTR32:
                exe.printNameOfElemAtAddr(elemVal, out);
                break;

            default:
                FATAL_ERROR("Unexpected case! We should never hit this!");
                break;
        }

        // Commas and newlines between the elements (if not a string)
        if (arrayElemType != ProgElemType::CHAR8) {
            if (addr + arrayElemTypeSize < progElem.endAddr) {
                out.put(',');
            }

            const bool bIsTimeForNewLine = (
                (progElem.arrayElemsPerLine <= 1) ||
                (arrayElemNum % progElem.arrayElemsPerLine == progElem.arrayElemsPerLine - 1)
            );

            if (bIsTimeForNewLine) {
                out.put('\n');
            } else {
                out.put(' ');
            }
        }

        ++arrayElemNum;
    }

    // Print the closing of the array
    if (arrayElemType == ProgElemType::CHAR8) {
        out << "\"\n";
    } else {
        out << "    }\n";
    }
}

static void printSingleVariable(const ExeFile& exe, const ProgElem& progElem, std::ostream& out) noexcept {
    // Validate that the variable is correctly aligned, and that the size matches the variable type size
    const uint32_t varTypeSize = getProgElemTypeSize(progElem.type);

    const bool progElemAligned = (
        (varTypeSize > 0) &&
        (progElem.startAddr % varTypeSize == 0) &&
        (progElem.endAddr % varTypeSize == 0)
    );

    if (!progElemAligned) {
        FATAL_ERROR("Invalid single variable program element! The address specified is not properly aligned for the type!");
    }

    const uint32_t progElemSize = progElem.endAddr - progElem.startAddr;

    if (progElemSize != varTypeSize) {
        FATAL_ERROR_F(
            "Invalid single variable program element! The size of the range specified ('%u') does not match the size of the type! (%u)",
            progElemSize,
            varTypeSize
        );
    }

    // Load the word for the variable
    const uint32_t wordIdx = (progElem.startAddr - exe.baseAddress) / 4;
    const ExeWord exeWord = exe.words[wordIdx];
    
    // Shift and/or mask the word's value if the size is less than 32-bits
    uint32_t varVal = exeWord.value;

    if (varTypeSize != 4) {
        const uint32_t shift = 8 * (progElem.startAddr % 4);
        varVal >>= shift;

        if (varTypeSize == 2) {
            varVal &= 0xFFFFu;
        } else {
            varVal &= 0xFFu;
        }
    }

    // Print the address of this variable and references to the exe word
    printAddressForLine(progElem.startAddr, out);
    printProgWordReferences(exeWord, &progElem, out);

    // Print the name and type of the single variable
    switch (progElem.type) {
        case ProgElemType::INT32:
            out << "i32 ";
            printProgElemName(progElem, "UNNAMED_i32_", out);
            break;

        case ProgElemType::UINT32:
            out << "u32 ";
            printProgElemName(progElem, "UNNAMED_u32_", out);
            break;

        case ProgElemType::INT16:
            out << "i16 ";
            printProgElemName(progElem, "UNNAMED_i16_", out);
            break;

        case ProgElemType::UINT16:
            out << "u16 ";
            printProgElemName(progElem, "UNNAMED_u16_", out);
            break;

        case ProgElemType::INT8:
            out << "i8 ";
            printProgElemName(progElem, "UNNAMED_i8_", out);
            break;

        case ProgElemType::UINT8:
            out << "u8 ";
            printProgElemName(progElem, "UNNAMED_u8_", out);
            break;

        case ProgElemType::BOOL8:
            out << "bool8 ";
            printProgElemName(progElem, "UNNAMED_bool8_", out);
            break;

        case ProgElemType::CHAR8:
            out << "char8 ";
            printProgElemName(progElem, "UNNAMED_char8_", out);
            break;

        case ProgElemType::PTR32:
            out << "ptr32 ";
            printProgElemName(progElem, "UNNAMED_pt32_", out);
            break;

        default:
            FATAL_ERROR("Unexpected case! We should never hit this!");
            break;
    }

    // Print the value itself
    out << " = ";

    switch (progElem.type) {
        case ProgElemType::INT32:       PrintUtils::printHexI32((int32_t) varVal, false, out);      break;
        case ProgElemType::UINT32:      PrintUtils::printHexU32(varVal, false, out);                break;
        case ProgElemType::INT16:       PrintUtils::printHexI16((int16_t) varVal, false, out);      break;
        case ProgElemType::UINT16:      PrintUtils::printHexU16((uint16_t) varVal, false, out);     break;
        case ProgElemType::INT8:        PrintUtils::printHexI8((int8_t) varVal, false, out);        break;
        case ProgElemType::UINT8:       PrintUtils::printHexU8((uint8_t) varVal, false, out);       break;
        case ProgElemType::BOOL8:       PrintUtils::printBool((varVal != 0), out);                  break;

        case ProgElemType::CHAR8:
            out.put('\'');
            PrintUtils::printEscapedChar((char) varVal, out);
            out.put('\'');
            break;

        case ProgElemType::PTR32:
            exe.printNameOfElemAtAddr(varVal, out);
            break;

        default:
            FATAL_ERROR("Unexpected case! We should never hit this!");
            break;
    }

    out << "\n";
}

//----------------------------------------------------------------------------------------------------------------------
// Verify the program element definition is SANE and raise a fatal error if not
//----------------------------------------------------------------------------------------------------------------------
static void validateProgElemRange(const ExeFile& exe, const ProgElem& progElem) noexcept {
    const uint32_t exeStart = exe.baseAddress;
    const uint32_t exeEnd = exeStart + exe.sizeInWords * 4;

    const bool bProgElemRangeInvalid = (
        (progElem.endAddr <= progElem.startAddr) ||
        (progElem.startAddr < exeStart) ||
        (progElem.endAddr < exeStart) ||
        (progElem.startAddr >= exeEnd) ||
        (progElem.endAddr > exeEnd)
    );

    if (bProgElemRangeInvalid) {
        FATAL_ERROR("Invalid program element! The range defined for the element is not valid for the .EXE");
    }
}

static void printProgElem(const ExeFile& exe, const ProgElem& progElem, std::ostream& out) noexcept {
    switch (progElem.type) {
        case ProgElemType::FUNCTION:
            printFunction(exe, progElem, out);
            break;

        case ProgElemType::INT32:
        case ProgElemType::UINT32:
        case ProgElemType::INT16:
        case ProgElemType::UINT16:
        case ProgElemType::INT8:
        case ProgElemType::UINT8:
        case ProgElemType::BOOL8:
        case ProgElemType::CHAR8:
        case ProgElemType::PTR32:
            printSingleVariable(exe, progElem, out);
            break;

        case ProgElemType::ARRAY:
            printArrayVariable(exe, progElem, out);
            break;
    }
}

static void printUncategorizedProgramRegion(
    const ExeFile& exe,
    const uint32_t startByteIdx,
    const uint32_t endByteIdx,
    std::ostream& out
) noexcept {
    // Print the region title
    out << "\n";
    out << "; -- UNCATEGORIZED REGION: ";
    PrintUtils::printHexU32(exe.baseAddress + startByteIdx, true, out);
    out.put('-');
    PrintUtils::printHexU32(exe.baseAddress + endByteIdx - 1, true, out);
    out.put('\n');

    // Continue until the entire region is done
    uint32_t curByteIdx = startByteIdx;

    while (curByteIdx < endByteIdx) {
        // Firstly print the address of whatever number of bytes we are going to print
        printAddressForLine(exe.baseAddress + curByteIdx, out);

        // Figure out how many bytes we can print on this line.
        // If the current byte is unaligned then print up until the NEXT aligned 32-bit word or the program end (whichever comes first).
        const uint32_t wordStartByteIdx = curByteIdx % 4;
        uint32_t numBytesToPrint = 4 - wordStartByteIdx;

        if (curByteIdx + numBytesToPrint > endByteIdx) {
            numBytesToPrint = endByteIdx - curByteIdx;
        }

        const uint32_t wordEndByteIdx = wordStartByteIdx + numBytesToPrint;

        // Firstly print the word references if it is a full word.
        // Otherwise print 14 spaces where the references section would be.
        const ExeWord exeWord = exe.words[curByteIdx / 4];

        if (numBytesToPrint == 4) {
            printProgWordReferences(exeWord, nullptr, out);
        } else {
            out << "              ";
        }

        // Print the 32-bit word fully if we have a whole word to print
        if (numBytesToPrint == 4) {
            PrintUtils::printHexU32(exeWord.value, true, out);
            out << "  ";
        }

        // Next print the individual hex values of the bytes
        for (uint32_t byteIdx = wordStartByteIdx; byteIdx < wordEndByteIdx; ++byteIdx) {
            const uint8_t byteVal = (uint8_t)(exeWord.value >> (8 * byteIdx));
            PrintUtils::printHexU8(byteVal, true, out);
            out.put(' ');
        }

        // Print the ascii value of each individual byte.
        // Don't print control characters or tabs and newlines, replace them with space.
        out.put(' ');

        for (uint32_t byteIdx = wordStartByteIdx; byteIdx < wordEndByteIdx; ++byteIdx) {
            const uint8_t byteVal = (uint8_t)(exeWord.value >> (8 * byteIdx));            
            out.put((byteVal >= 32 && byteVal <= 126) ? (char) byteVal : ' ');
        }

        // Lastly, if we have 4 bytes then try to decode a program instruction and then move onto a new line
        if (numBytesToPrint == 4) {
            out << "      ";
            printProgInstruction(exe, nullptr, exe.baseAddress + curByteIdx, exeWord.value, out);
        }

        out.put('\n');
        curByteIdx += numBytesToPrint;
    }

    out << "\n\n";
}

void DisassemblyPrinter::printExe(const ExeFile& exe, std::ostream& out) noexcept {
    // Firstly make sure all program elements are valid and in range for the exe
    for (const ProgElem& progElem : exe.progElems) {
        validateProgElemRange(exe, progElem);
    }

    // Continue until we have printed all program words
    uint32_t curProgByteIdx = 0;
    uint32_t curProgElemIdx = 0;

    const uint32_t numProgBytes = exe.sizeInWords * 4;
    const uint32_t numProgElems = (uint32_t) exe.progElems.size();
    const uint32_t exeBaseAddr = exe.baseAddress;

    while (curProgByteIdx < numProgBytes) {
        // Check if we are at a program element or are past due printing one.
        // If that is the case then print the program element.
        if (curProgElemIdx < numProgElems) {
            const uint32_t curByteAddr = exeBaseAddr + curProgByteIdx;
            const ProgElem& progElem = exe.progElems[curProgElemIdx];
            
            if (progElem.containsByteAtAddr(curByteAddr) || progElem.endAddr <= curByteAddr) {
                // Time to print this program element
                printProgElem(exe, progElem, out);
                ++curProgElemIdx;

                // Move on in the program bytes and ensure we don't go backwards
                if (progElem.endAddr > exeBaseAddr) {
                    const uint32_t progElemEndOffset = progElem.endAddr - exeBaseAddr;
                    curProgByteIdx = std::max(curProgByteIdx, progElemEndOffset);
                }

                // Need to examine where we are again
                continue;
            }
        }

        // Printing an unknown/uncategorized region of the EXE.
        // Figure out the size of it by seeing where the next program element starts.
        uint32_t endByteIdx = curProgByteIdx + 1;

        if (curProgElemIdx < numProgElems) {
            const uint32_t progElemStartByteIdx = exe.progElems[curProgElemIdx].startAddr - exe.baseAddress;
            endByteIdx = std::max(endByteIdx, progElemStartByteIdx);
        } else {
            endByteIdx = numProgBytes;
        }

        printUncategorizedProgramRegion(exe, curProgByteIdx, endByteIdx, out);
        curProgByteIdx = endByteIdx;
    }
}

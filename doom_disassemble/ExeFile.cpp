#include "ExeFile.h"

#include "CpuInstruction.h"
#include "FatalErrors.h"
#include "PrintUtils.h"
#include <algorithm>
#include <cstdio>

//----------------------------------------------------------------------------------------------------------------------
// The header for a playstation EXE, exactly 2048 bytes in size.
// Details taken from:
//  https://problemkaputt.de/psx-spx.htm#cdromfileformats
//----------------------------------------------------------------------------------------------------------------------

// The expected first 8 bytes of the .EXE header in little endian format.
// Should say 'PS-X EXE' when arranged in little endian format:
static constexpr uint64_t EXE_HEADER_MAGIC = 0x45584520582D5350u;

// The North America (US) PSX .EXE region signature
static constexpr const char* const EXE_NA_REGION_SIG = "Sony Computer Entertainment Inc. for North America area";

struct ExeHeader {
    // Should equal the magic value expected
    uint64_t magic;

    // Unknown/unused, should normally both be '0'
    uint32_t _unused1[2];

    // Initial value for the program counter register, i.e where the entry point is after loading
    // the .EXE file into the specified address in RAM. Normally >= 0x80010000.
    uint32_t initialPcReg;

    // Initial value of the GP/R28 ('Global Pointer') register.
    // This is normally '0'.
    uint32_t initialGpReg;

    // Where the .EXE file is loaded into the address space of the PlayStation.
    // This is the 'base address' of .EXE essentially and all locations in the program will be referenced relative to this.
    // This is USUALLY set to '0x80010000', with the first 64KB of the memory segment (0x80000000 to 0x8000FFFF) being
    // reserved for use by the BIOS.
    uint32_t destAddr;

    // Size of the actual EXE, excluding the header.
    // Must be in multiples of 2,048 bytes.
    uint32_t progSize;

    // Unknown/unused, should normally both be '0'
    uint32_t _unused2[2];

    // A region of memory to zero on startup, usually both '0'
    uint32_t memZeroFillStartAddr;
    uint32_t memZeroFillSize;

    // Initial base value for the SP/R29 (Stack pointer) register and an offset added to the base to give
    // the actual value of the stack pointer. Normally the base is set to '0x801FFFF0' for PlayStation
    // games and the offset is usually '0'.
    uint32_t initialSpRegBase;
    uint32_t initialSpRegOffset;

    // Reserved for the A(43h) bios function (should be zerofilled in exefile)
    uint32_t biosReserved[5];

    // An ASCII signature is expected to follow in addition to zero fill up until the end of 2,048 header bytes.
    // The ASCII marker should say one of the following:
    //
    //  "Sony Computer Entertainment Inc. for Japan area"
    //  "Sony Computer Entertainment Inc. for Europe area"
    //  "Sony Computer Entertainment Inc. for North America area"
    //
    char regionSigAndZeroFill[1972];
};

static_assert(sizeof(ExeHeader) == 2048);

//----------------------------------------------------------------------------------------------------------------------
// Read the raw 32-bit words for a PSX .EXE file including the header of the .EXE
//----------------------------------------------------------------------------------------------------------------------
static uint32_t readExeFileWords(const char* const path, std::unique_ptr<uint32_t[]>& words) noexcept {
    FILE* const pFile = std::fopen(path, "rb");
    
    if (!pFile) {
        FATAL_ERROR_F("Unable to open PSX .EXE file at path '%s'!", path);
    }

    if (std::fseek(pFile, 0, SEEK_END) != 0) {
        FATAL_ERROR_F("Failed to determine the size of PSX .EXE file '%s'! Seeking to the end of the file failed.", path);
    }

    const long offset = std::ftell(pFile);

    if (offset < 0) {
        FATAL_ERROR_F("Failed to determine the size of PSX .EXE file '%s'! A 'std::ftell' operation failed!", path);
    } else if (offset > INT32_MAX) {
        FATAL_ERROR_F("PSX .EXE file '%s' is too big to read! Greater than the range of a 32-bit integer! Size is: %dl", path, offset);
    } else if (offset == 0) {
        FATAL_ERROR_F("PSX .EXE file '%s' is zero sized!", path);
    } else if (offset % 4 != 0) {
        FATAL_ERROR_F("The size of PSX .EXE file '%s' is not a multiple of 4! Size was instead: %u, Invalid .EXE!", path, (uint32_t) offset);
    }

    if (std::fseek(pFile, 0, SEEK_SET) != 0) {
        FATAL_ERROR_F("Failed to read PSX .EXE file '%s'! Seeking to the beginning of the file failed.", path);
    }

    const uint32_t fileSize = (uint32_t) offset;
    const uint32_t fileSizeInWords = fileSize / sizeof(uint32_t);
    words.reset(new uint32_t[fileSizeInWords]);

    if (std::fread(words.get(), fileSize, 1, pFile) != 1) {
        FATAL_ERROR_F("Failed to read PSX .EXE file '%s'! Reading all of the file bytes failed.", path);
    }

    std::fclose(pFile);
    return fileSizeInWords;
}

ExeFile::ExeFile() noexcept 
    : baseAddress(0)
    , sizeInWords(0)
    , entryPointWordIdx(0)
    , words(nullptr)
    , progElems()
{
}

void ExeFile::loadFromFile(const char* const path) noexcept {
    // Read the raw 32-bit words of the .EXE file
    std::unique_ptr<uint32_t[]> exeFileWords;
    const uint32_t numExeWords = readExeFileWords(path, exeFileWords);
    
    // There must be > 2,048 bytes (> 512 32-bit words) in the .EXE file for it to be valid.
    // The first 2,048 bytes is the header of the .EXE.
    if (numExeWords <= 512) {
        FATAL_ERROR_F("PSX .EXE file '%s' is invalid because it is too small, does not contain a proper header and/or code. Size is: %u", path, numExeWords * 4);
    }

    // Validate the EXE header
    const uint32_t numExeBytes = numExeWords * sizeof(uint32_t);
    const ExeHeader& header = *reinterpret_cast<const ExeHeader*>(exeFileWords.get());

    const bool bIsValidHeader = (
        (header.magic == EXE_HEADER_MAGIC) &&       // Magic must be OK
        (header.initialPcReg % 4 == 0) &&           // These must be all 32-bit aligned
        (header.initialGpReg % 4 == 0) &&
        (header.destAddr % 4 == 0) &&
        (header.memZeroFillStartAddr % 4 == 0) &&
        (header.memZeroFillSize % 4 == 0) &&
        (header.initialSpRegBase % 4 == 0) &&
        (header.initialSpRegOffset % 4 == 0) &&     // Size must match file size plus header, and be a multiple of 2,048
        (header.progSize > 0) &&
        (header.progSize % 2048 == 0) &&
        (header.progSize < numExeBytes) &&
        (header.progSize + sizeof(ExeHeader) == numExeBytes) &&
        (header.initialPcReg >= header.destAddr) &&                 // Verify the entry point
        (header.initialPcReg - header.destAddr < numExeBytes) &&    
        (std::strcmp(EXE_NA_REGION_SIG, header.regionSigAndZeroFill) == 0)  // Verify North American version
    );

    if (!bIsValidHeader) {
        FATAL_ERROR_F("The header for PSX .EXE file '%s' is invalid or the .EXE does not match the description in the header!", path);
    }

    // Alloc program words and initialize
    const uint32_t numProgWords = header.progSize / sizeof(uint32_t);
    words.reset(new ExeWord[numProgWords]);
    
    {
        constexpr uint32_t EXE_HEADER_SIZE_IN_WORDS = sizeof(ExeHeader) / sizeof(uint32_t);

        const uint32_t* pSrcWord = exeFileWords.get() + EXE_HEADER_SIZE_IN_WORDS;
        const uint32_t* const pEndSrcWord = pSrcWord + numProgWords;
        ExeWord* pDstWord = words.get();

        while (pSrcWord < pEndSrcWord) {            
            *pDstWord = {};
            pDstWord->value = *pSrcWord;

            ++pSrcWord;
            ++pDstWord;
        }
    }

    // Save other program info
    baseAddress = header.destAddr;
    sizeInWords = numProgWords;
    entryPointWordIdx = (header.initialPcReg - header.destAddr) / sizeof(uint32_t);
}

void ExeFile::setProgElems(const ProgElem* const pElems, const uint32_t numElems) noexcept {
    // Save the elements
    assert(pElems || numElems == 0);
    progElems.resize(numElems);
    std::memcpy(progElems.data(), pElems, sizeof(ProgElem) * numElems);

    // Sort by start address, then by end address
    std::sort(
        progElems.begin(),
        progElems.end(),
        [](const ProgElem& elem1, const ProgElem& elem2) noexcept {
            if (elem1.startAddr != elem2.startAddr) {
                return (elem1.startAddr < elem2.startAddr);
            }

            return (elem1.endAddr < elem2.endAddr);
        }
    );
}

const ProgElem* ExeFile::findProgElemAtAddr(const uint32_t addr) const noexcept {
    auto iter = std::lower_bound(
        progElems.begin(),
        progElems.end(),
        addr,
        [](const ProgElem& elem, const uint32_t addr) noexcept {
            return (elem.endAddr <= addr);
        }
    );

    if (iter != progElems.end()) {
       const ProgElem& elem = *iter;

       if (addr >= elem.startAddr && addr < elem.endAddr) {
            return &elem;
       }
    }
    
    return nullptr;
}

void ExeFile::printNameOfElemAtAddr(const uint32_t addr, std::ostream& out) const noexcept {
    const ProgElem* const pElem = findProgElemAtAddr(addr);

    if (pElem) {
        pElem->printNameAtAddr(addr, out);
    } else {
        PrintUtils::printHexU32(addr, true, out);
    }
}

void ExeFile::determineWordReferences() noexcept {
    const uint32_t exeStartAddr = baseAddress;
    const uint32_t exeEndAddr = baseAddress + sizeInWords * 4;

    for (uint32_t wordIdx = 0; wordIdx < sizeInWords; ++wordIdx) {
        // Grab the program element at this word and the word itself
        const uint32_t word = words[wordIdx].value;
        const uint32_t wordAddr = baseAddress + wordIdx * 4;
        const ProgElem* const pProgElem = findProgElemAtAddr(wordAddr);

        // See if the word can have a data or instruction reference
        bool bCanHaveDataRef = false;
        bool bCanHaveInstRef = false;

        if (pProgElem == nullptr) {
            // If the region is undefined then we try to interpret as both code AND data
            bCanHaveDataRef = true;
            bCanHaveInstRef = true;
        } else {
            switch (pProgElem->type) {
                case ProgElemType::FUNCTION:
                    bCanHaveInstRef = true;
                    break;

                case ProgElemType::ARRAY: {
                    if (pProgElem->arrayElemType == ProgElemType::PTR32) {
                        bCanHaveDataRef = true;
                    }
                }   break;

                case ProgElemType::PTR32:
                    bCanHaveDataRef = true;
                    break;

                // A program element type that can't reference anything
                default: break;
            }
        }

        // Add data references other program words.
        // These references must be aligned references, don't detect other types of references.
        if (bCanHaveDataRef) {
            if (word % 4 == 0) {
                if (word >= exeStartAddr && word < exeEndAddr) {
                    const uint32_t referencedWordIdx = (word - baseAddress) / 4;
                    words[referencedWordIdx].bIsDataReferenced = true;
                }
            }
        }

        // Add code references to other program words.
        // Obviously code references absolutely HAVE to be aligned!
        if (bCanHaveInstRef) {
            CpuInstruction inst;

            if (inst.decode(word)) {
                const uint32_t thisInstAddr = baseAddress + wordIdx * 4;

                if (CpuOpcodeUtils::isBranchOpcode(inst.opcode)) {
                    // References to words via branch instructions                    
                    const uint32_t branchTgtAddr = inst.getBranchInstTargetAddr(thisInstAddr);

                    if (branchTgtAddr >= exeStartAddr && branchTgtAddr < exeEndAddr) {
                        const uint32_t branchTgtWordIdx = (branchTgtAddr - baseAddress) / 4;
                        words[branchTgtWordIdx].bIsBranchTarget = true;
                    }
                }
                else if (CpuOpcodeUtils::isFixedJumpOpcode(inst.opcode)) {
                    // References to words via fixed jump instructions
                    const uint32_t jumpTgtAddr = inst.getFixedJumpInstTargetAddr(thisInstAddr);

                    if (jumpTgtAddr >= exeStartAddr && jumpTgtAddr < exeEndAddr) {
                        const uint32_t jumpTgtWordIdx = (jumpTgtAddr - baseAddress) / 4;
                        words[jumpTgtWordIdx].bIsJumpTarget = true;
                    }
                }
            }
        }
    }
}

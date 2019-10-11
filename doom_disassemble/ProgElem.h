#pragma once

#include <cstdint>
#include <ostream>

//----------------------------------------------------------------------------------------------------------------------
// Data structures that hold manually identified elements of the program.
// Such elements are identified from examining the disassembly, and through debugging and reverse engineering.
//----------------------------------------------------------------------------------------------------------------------
enum class ProgElemType : uint8_t {
    FUNCTION,
    INT32,
    UINT32,
    INT16,
    UINT16,
    INT8,
    UINT8,
    BOOL8,
    CHAR8,
    ARRAY,
    PTR32
};

uint32_t getProgElemTypeSize(const ProgElemType type) noexcept;

struct ProgElem {
    uint32_t        startAddr;          // Where the element starts in the program (inclusive)
    uint32_t        endAddr;            // Where the element ends in the program (exclusive)
    const char*     name;               // The name of the element - must be a compile time constant string
    ProgElemType    type;               // What type of element this is
    ProgElemType    arrayElemType;      // If an array, what type the array elements are

    inline constexpr ProgElem() noexcept
        : startAddr(0)
        , endAddr(0)
        , name("")
        , type()
        , arrayElemType()
    {
    }

    inline constexpr ProgElem(
        uint32_t startAddr,
        uint32_t endAddr,
        const char* const name,
        ProgElemType type
    ) noexcept
        : startAddr(startAddr)
        , endAddr(endAddr)
        , type(type)
        , arrayElemType(type)
        , name(name)
    {
    }

    inline constexpr ProgElem(
        uint32_t startAddr,
        uint32_t endAddr,
        const char* const name,
        ProgElemType type,
        ProgElemType arrayElemType
    ) noexcept
        : startAddr(startAddr)
        , endAddr(endAddr)
        , type(type)
        , arrayElemType(arrayElemType)
        , name(name)
    {
    }

    constexpr ProgElem(const ProgElem& other) noexcept = default;

    // Tells if the program element fully contains the word which starts at the given address.
    // Will return 'true' if this is the case.
    inline bool containsWordAtAddr(const uint32_t wordAddr) const noexcept {
        return (wordAddr >= startAddr && wordAddr + 4 <= endAddr);
    }

    // Tells if the program element contains the byte at the given address
    inline bool containsByteAtAddr(const uint32_t byteAddr) const noexcept {
        return (byteAddr >= startAddr && byteAddr < endAddr);
    }

    // Print the name of the program element at the given address.
    // For some element types the address will be ignored, whereas for things like arrays it can be used to compute the element number.
    void printNameAtAddr(const uint32_t addr, std::ostream& out) const noexcept;
};

#pragma once

#include <cstdint>

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

inline uint32_t getProgElemTypeSize(const ProgElemType type) noexcept {
    switch (type) {
        case ProgElemType::FUNCTION:    return 0;   // N/A, depends on size of element memory region
        case ProgElemType::INT32:       return 4;
        case ProgElemType::UINT32:      return 4;
        case ProgElemType::INT16:       return 2;
        case ProgElemType::UINT16:      return 2;
        case ProgElemType::INT8:        return 1;
        case ProgElemType::UINT8:       return 1;
        case ProgElemType::BOOL8:       return 1;
        case ProgElemType::CHAR8:       return 1;
        case ProgElemType::ARRAY:       return 0;   // N/A, depends on size of element memory region
        case ProgElemType::PTR32:       return 4;
    }

    return 0;
}

struct ProgElem {
    uint32_t        startAddr;          // Where the element starts in the program (inclusive)
    uint32_t        endAddr;            // Where the element ends in the program (exclusive)
    const char*     name;               // The name of the element - must be a compile time constant string
    ProgElemType    type;               // What type of element this is
    ProgElemType    arrayElemType;      // If an array, what type the array elements are

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
};

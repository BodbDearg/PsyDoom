#include "ProgElem.h"

#include "PrintUtils.h"

//----------------------------------------------------------------------------------------------------------------------
// Print the name of a program element at the specified address.
// Just prints the program element name if the address is the start address, otherwise includes an offset into the elem.
//----------------------------------------------------------------------------------------------------------------------
static void printProgElemNameAtAddr(
    const ProgElem& progElem,
    const uint32_t addr,
    const char* const defaultNamePrefix,
    std::ostream& out
) {
    // Does the program element have a name?
    // If it doesn't then makeup a default one based on the prefix we were given...
    if (progElem.name != nullptr && progElem.name[0] != 0) {
        out << progElem.name;
    } else {
        out << defaultNamePrefix;
        PrintUtils::printHexU32(progElem.startAddr, true, out);
    }

    // If the address is not the start of the elem then show an offset too (plus the full address in brackets)
    if (addr != progElem.startAddr) {
        const int32_t addrOffsetInElem = (int32_t)(addr - progElem.startAddr);

        if (addrOffsetInElem >= 0) {
            out << " + ";
        }

        PrintUtils::printHexI32(addrOffsetInElem, false, out);
        out << " (";
        PrintUtils::printHexU32(addr, true, out);
        out.put(')');
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Print the name of a program element at the specified address, interpreting the element as an array.
// Prints always the hex index in the array of the given address.
//----------------------------------------------------------------------------------------------------------------------
static void printArrayProgElemNameAtAddr(
    const ProgElem& progElem,
    const uint32_t addr,
    const char* const defaultNamePrefix,
    std::ostream& out
) {
    // Figure out what array index the given address points to
    int32_t arrayIdx = 0;
    const int32_t elemSize = (int32_t) getProgElemTypeSize(progElem.arrayElemType);

    if (elemSize > 0) {
        const int32_t offsetInElem = (int32_t)(addr - progElem.startAddr);
        arrayIdx = offsetInElem / elemSize;
    }

    // Does the program element have a name?
    // If it doesn't then makeup a default one based on the prefix we were given...
    if (progElem.name != nullptr && progElem.name[0] != 0) {
        out << progElem.name;
    } else {
        out << defaultNamePrefix;
        PrintUtils::printHexU32(progElem.startAddr, true, out);
    }

    // Print the hex index into the array
    out.put('[');
    PrintUtils::printHexI32(arrayIdx, false, out);
    out.put(']');
}

uint32_t getProgElemTypeSize(const ProgElemType type) noexcept {
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

void ProgElem::printNameAtAddr(const uint32_t addr, std::ostream& out) const {
    switch (type) {
        case ProgElemType::FUNCTION:    printProgElemNameAtAddr(*this, addr, "UNNAMED_func_", out);     break;
        case ProgElemType::INT32:       printProgElemNameAtAddr(*this, addr, "UNNAMED_i32_", out);      break;
        case ProgElemType::UINT32:      printProgElemNameAtAddr(*this, addr, "UNNAMED_u32_", out);      break;
        case ProgElemType::INT16:       printProgElemNameAtAddr(*this, addr, "UNNAMED_i16_", out);      break;
        case ProgElemType::UINT16:      printProgElemNameAtAddr(*this, addr, "UNNAMED_u16_", out);      break;
        case ProgElemType::INT8:        printProgElemNameAtAddr(*this, addr, "UNNAMED_i8_", out);       break;
        case ProgElemType::UINT8:       printProgElemNameAtAddr(*this, addr, "UNNAMED_u8_", out);       break;
        case ProgElemType::BOOL8:       printProgElemNameAtAddr(*this, addr, "UNNAMED_bool8_", out);    break;
        case ProgElemType::CHAR8:       printProgElemNameAtAddr(*this, addr, "UNNAMED_char8_", out);    break;
        case ProgElemType::PTR32:       printProgElemNameAtAddr(*this, addr, "UNNAMED_ptr32_", out);    break;

        case ProgElemType::ARRAY: {
            switch (arrayElemType) {
                // These two cases should never occur!
                // But just print this if they do for some insane reason...
                case ProgElemType::FUNCTION:
                case ProgElemType::ARRAY:
                    printProgElemNameAtAddr(*this, addr, "unnamed_array_INVALID_TYPE_", out);
                    break;

                case ProgElemType::INT32:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_i32_", out);    break;
                case ProgElemType::UINT32:  printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_u32_", out);    break;
                case ProgElemType::INT16:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_i16_", out);    break;
                case ProgElemType::UINT16:  printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_u16_", out);    break;
                case ProgElemType::INT8:    printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_i8_", out);     break;
                case ProgElemType::UINT8:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_u8_", out);     break;
                case ProgElemType::BOOL8:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_bool8_", out);  break;
                case ProgElemType::CHAR8:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_string8_", out);      break;
                case ProgElemType::PTR32:   printArrayProgElemNameAtAddr(*this, addr, "UNNAMED_array_ptr32_", out);  break;
            }

        }   break;
    }    
}

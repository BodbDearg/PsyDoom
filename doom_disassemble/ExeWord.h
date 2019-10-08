#include <cstdint>
#include <cassert>

//----------------------------------------------------------------------------------------------------------------------
// Represents a single 32-bit word in the .EXE.
// The word can either be code or data and is stored in little endian format.
//----------------------------------------------------------------------------------------------------------------------
struct ExeWord {
    // The actual 32-bit value of the word.
    // This is the value that is read directly from the executable.
    uint32_t value; 

    // Whether each byte of data in the word has been categorized as some sort of code or data.
    // When the disassembler is given instructions as to what sort of information a region in the EXE represents, this
    // boolean will be set to 'true'. If multiple instructions claim the same program byte as something then a conflict/error
    // will be raised - each byte can only be classed as something once.
    bool bByteIsCategorized[4];
};

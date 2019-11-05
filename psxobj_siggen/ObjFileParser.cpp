#include "ObjFileParser.h"

#include "ObjFileData.h"
#include "TextStream.h"

struct ParseException {
    const char* msg;
    ParseException(const char* msg) noexcept : msg(msg) {}
};

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      Header : LNK version 2
//----------------------------------------------------------------------------------------------------------------------
static void parseHeaderInfo(TextStream& text, ObjFile& out) {
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("Header");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("LNK");
    line.consumeSpaceSeparatedTokenAhead("version");
    out.lnkVersion = (uint16_t) line.readDecimalUint();
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//  76 : Function end :
//      section 557c
//      offset $000000f4
//      end line 0
//----------------------------------------------------------------------------------------------------------------------
static void parseFunctionEndDirective(TextStream& text, [[maybe_unused]] ObjFile& out) {
    // Note: just ignoring this directive and skipping past it all
    {
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("76");
        line.consumeSpaceSeparatedTokenAhead(":");
        line.consumeSpaceSeparatedTokenAhead("Function");
        line.consumeSpaceSeparatedTokenAhead("end");
        line.consumeSpaceSeparatedTokenAhead(":");
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("section");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("offset");
        line.consumeSpaceSeparatedTokenAhead("$");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("end");
        line.consumeSpaceSeparatedTokenAhead("line");
        line.readHexUint();
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//  74 : Function start :
//      section 557c
//      offset $00000000
//      file 59a7
//      start line 0
//      frame reg 29
//      frame size 0
//      return pc reg 31
//      mask $00000000
//      mask offset 0
//      name _ExpAllocArea
//----------------------------------------------------------------------------------------------------------------------
static void parseFunctionStartDirective(TextStream& text, [[maybe_unused]] ObjFile& out) {
    // Note: just ignoring this directive and skipping past it all
    {
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("74");
        line.consumeSpaceSeparatedTokenAhead(":");
        line.consumeSpaceSeparatedTokenAhead("Function");
        line.consumeSpaceSeparatedTokenAhead("start");
        line.consumeSpaceSeparatedTokenAhead(":");
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("section");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("offset");
        line.consumeSpaceSeparatedTokenAhead("$");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("file");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("start");
        line.consumeSpaceSeparatedTokenAhead("line");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("frame");
        line.consumeSpaceSeparatedTokenAhead("reg");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("frame");
        line.consumeSpaceSeparatedTokenAhead("size");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("return");
        line.consumeSpaceSeparatedTokenAhead("pc");
        line.consumeSpaceSeparatedTokenAhead("reg");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("mask");
        line.consumeSpaceSeparatedTokenAhead("$");
        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("mask");
        line.consumeSpaceSeparatedTokenAhead("offset");

        if (line.peekChar() == '-') {
            line.skipChar();
        }

        line.readHexUint();
    }

    {
        text.skipAsciiWhiteSpace();
        TextStream line = text.readNextLineAsStream();
        line.consumeSpaceSeparatedTokenAhead("name");
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      48 : XBSS symbol number 2b '_que' size 1800 in section 6
//----------------------------------------------------------------------------------------------------------------------
static void parseXBssSymbolDirective(TextStream& text, ObjFile& out) {
    // Read symbol number
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("48");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("XBSS");
    line.consumeSpaceSeparatedTokenAhead("symbol");
    line.consumeSpaceSeparatedTokenAhead("number");

    ObjSymbol& symbol = out.symbols.emplace_back();
    symbol.number = line.readHexUint();

    // Read symbol name
    line.skipAsciiWhiteSpace();
    symbol.name = line.readDelimitedString('\'', '\'');

    // Read symbol size
    line.consumeSpaceSeparatedTokenAhead("size");
    symbol.defSize = line.readHexUint();

    // Read symbol section
    line.consumeSpaceSeparatedTokenAhead("in");
    line.consumeSpaceSeparatedTokenAhead("section");
    symbol.defSection = (uint16_t) line.readHexUint();

    // Ensure no leftovers
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      46 : Processor type 7
//----------------------------------------------------------------------------------------------------------------------
static void parseProcessorType(TextStream& text, ObjFile& out) {
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("46");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Processor");
    line.consumeSpaceSeparatedTokenAhead("type");
    out.processorType = (uint16_t) line.readDecimalUint();
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      28 : Define file number 9 as "C:\PSX\SRC\C2\SPRINTF.C"
//----------------------------------------------------------------------------------------------------------------------
static void parseFileNameDefinition(TextStream& text, [[maybe_unused]] ObjFile& out) {
    // Just ensure the format is as expected then ignore the rest - we don't use this info
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("28");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Define");
    line.consumeSpaceSeparatedTokenAhead("file");
    line.consumeSpaceSeparatedTokenAhead("number");
    line.readHexUint();
    line.consumeSpaceSeparatedTokenAhead("as");
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      16 : Section symbol number 1 '.rdata' in group 0 alignment 8
//----------------------------------------------------------------------------------------------------------------------
static void parseSectionDefinition(TextStream& text, ObjFile& out) {
    // Parse section number
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("16");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Section");
    line.consumeSpaceSeparatedTokenAhead("symbol");
    line.consumeSpaceSeparatedTokenAhead("number");

    const uint32_t sectionNumber = line.readHexUint();
    line.consumeAsciiWhiteSpaceAhead();

    // Parse section type
    struct KnownSectionType {
        const char*     asString;
        ObjSectionType  asType;
    };

    constexpr KnownSectionType KNOWN_SECTION_TYPES[] = {
        KnownSectionType { "'.rdata'", ObjSectionType::RDATA },
        KnownSectionType { "'.text'", ObjSectionType::TEXT },
        KnownSectionType { "'.data'", ObjSectionType::DATA },
        KnownSectionType { "'.sdata'", ObjSectionType::SDATA },
        KnownSectionType { "'.sbss'", ObjSectionType::SBSS },
        KnownSectionType { "'.bss'", ObjSectionType::BSS },
        KnownSectionType { "'.ctors'", ObjSectionType::CTORS },
        KnownSectionType { "'.dtors'", ObjSectionType::DTORS },
    };

    constexpr uint32_t NUM_KNOWN_SECTION_TYPES = sizeof(KNOWN_SECTION_TYPES) / sizeof(KnownSectionType);
    ObjSectionType sectionType = {};
    bool bFoundSectionType = false;
    
    for (uint32_t i = 0; i < NUM_KNOWN_SECTION_TYPES; ++i) {
        if (line.checkStringAhead(KNOWN_SECTION_TYPES[i].asString)) {
            line.consumeSpaceSeparatedTokenAhead(KNOWN_SECTION_TYPES[i].asString);
            sectionType = KNOWN_SECTION_TYPES[i].asType;
            bFoundSectionType = true;
            break;
        }
    }

    if (!bFoundSectionType) {
        throw ParseException("Unknown section type in section definition!");
    }

    // Parse group number
    line.consumeSpaceSeparatedTokenAhead("in");
    line.consumeSpaceSeparatedTokenAhead("group");
    const uint32_t groupNumber = line.readHexUint();

    // Parse alignment
    line.consumeSpaceSeparatedTokenAhead("alignment");
    const uint32_t alignment = line.readHexUint();

    // Ensure nothing is leftover
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();

    // Save the section
    ObjSection& section = out.sections.emplace_back();
    section.number = (uint16_t) sectionNumber;
    section.type = sectionType;
    section.group = (uint16_t) groupNumber;
    section.alignment = (uint16_t) alignment;
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      18 : Local symbol 'get_mode' at offset 1590 in section 2
//----------------------------------------------------------------------------------------------------------------------
static void parseLocalSymbolDirective(TextStream& text, ObjFile& out) {
    // Read symbol name
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("18");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Local");
    line.consumeSpaceSeparatedTokenAhead("symbol");

    ObjSymbol& symbol = out.symbols.emplace_back();
    symbol.name = line.readDelimitedString('\'', '\'');

    // Read symbol offset
    line.consumeSpaceSeparatedTokenAhead("at");
    line.consumeSpaceSeparatedTokenAhead("offset");
    symbol.defOffset = line.readHexUint();

    // Read symbol section
    line.consumeSpaceSeparatedTokenAhead("in");
    line.consumeSpaceSeparatedTokenAhead("section");
    symbol.defSection = (uint16_t) line.readHexUint();

    // Ensure no leftovers
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      14 : XREF symbol number c 'memchr'
//----------------------------------------------------------------------------------------------------------------------
static void parseXRefSymbolDirective(TextStream& text, ObjFile& out) {
    // Read symbol number
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("14");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("XREF");
    line.consumeSpaceSeparatedTokenAhead("symbol");
    line.consumeSpaceSeparatedTokenAhead("number");

    ObjSymbol& symbol = out.symbols.emplace_back();
    symbol.number = line.readHexUint();

    // Read symbol name
    line.skipAsciiWhiteSpace();
    symbol.name = line.readDelimitedString('\'', '\'');

    // Ensure no leftovers
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      12 : XDEF symbol number a 'sprintf' at offset 0 in section 2
//----------------------------------------------------------------------------------------------------------------------
static void parseXDefSymbolDirective(TextStream& text, ObjFile& out) {
    // Read symbol number
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("12");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("XDEF");
    line.consumeSpaceSeparatedTokenAhead("symbol");
    line.consumeSpaceSeparatedTokenAhead("number");

    ObjSymbol& symbol = out.symbols.emplace_back();
    symbol.number = line.readHexUint();

    // Read symbol name
    line.skipAsciiWhiteSpace();
    symbol.name = line.readDelimitedString('\'', '\'');

    // Read symbol offset
    line.consumeSpaceSeparatedTokenAhead("at");
    line.consumeSpaceSeparatedTokenAhead("offset");
    symbol.defOffset = line.readHexUint();

    // Read symbol section
    line.consumeSpaceSeparatedTokenAhead("in");
    line.consumeSpaceSeparatedTokenAhead("section");
    symbol.defSection = (uint16_t) line.readHexUint();

    // Ensure no leftovers
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like any of the following lines formats:
//      10 : Patch type 16 at offset c with (sectbase(2)+$1a0c)
//      10 : Patch type 74 at offset 70 with [2e]
//      10 : Patch type 84 at offset 168 with ($c+[2b])
//      10 : Patch type 82 at offset 14c with ($c+(sectbase(6)+$0))
//
// NOTE: in order to keep things simple, I ignore anything past the 'with' token.
// I don't need to know what something is being patched WITH, just WHAT is being patched for function signature matching.
//----------------------------------------------------------------------------------------------------------------------
static void parsePatchDirective(TextStream& text, ObjFile& out) {
    // Get the current section
    ObjSection* pSection = out.getSectionWithNum(out.curSectionNumber);

    if (!pSection) {
        throw ParseException("Invalid current section number!");
    }

    // Read patch type and target offset
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("10");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Patch");
    line.consumeSpaceSeparatedTokenAhead("type");

    ObjPatch& patch = pSection->patches.emplace_back();
    patch.type = (uint16_t) line.readDecimalUint();

    line.consumeSpaceSeparatedTokenAhead("at");
    line.consumeSpaceSeparatedTokenAhead("offset");
    patch.targetOffset = line.readHexUint();

    // Note: simply ignore the rest past this!
    line.consumeSpaceSeparatedTokenAhead("with");
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      8 : Uninitialised data, 11 bytes
//----------------------------------------------------------------------------------------------------------------------
static void parseUnitializedDataDirective(TextStream& text, ObjFile& out) {
    // Get the current section
    ObjSection* pSection = out.getSectionWithNum(out.curSectionNumber);

    if (!pSection) {
        throw ParseException("Invalid current section number!");
    }

    // Parse the uninitialized data directive and save the 'unitialized' bytes as zeros
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("8");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Uninitialised");
    line.consumeSpaceSeparatedTokenAhead("data");
    line.consumeSpaceSeparatedTokenAhead(",");    
    const uint32_t numUninitializedBytes = line.readDecimalUint();
    line.consumeSpaceSeparatedTokenAhead("bytes");
    line.ensureAtEnd();

    std::vector<std::byte>& sectionBytes = pSection->data;
    sectionBytes.insert(sectionBytes.end(), numUninitializedBytes, (std::byte) 0);
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      6 : Switch to section 2
//----------------------------------------------------------------------------------------------------------------------
static void parseSwitchToSectionDirective(TextStream& text, ObjFile& out) {
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("6");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("Switch");
    line.consumeSpaceSeparatedTokenAhead("to");
    line.consumeSpaceSeparatedTokenAhead("section");
    out.curSectionNumber = line.readHexUint();
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      2 : Code 37 bytes
//
//      0000:30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 
//      0010:00 00 00 00 30 31 32 33 34 35 36 37 38 39 61 62 
//      0020:63 64 65 66 00 
//----------------------------------------------------------------------------------------------------------------------
static void parseCodeDirective(TextStream& text, ObjFile& out) {
    // Get the current section
    ObjSection* pSection = out.getSectionWithNum(out.curSectionNumber);

    if (!pSection) {
        throw ParseException("Invalid current section number!");
    }
    
    // Get the number of bytes
    TextStream headerLine = text.readNextLineAsStream();
    headerLine.consumeSpaceSeparatedTokenAhead("2");
    headerLine.consumeSpaceSeparatedTokenAhead(":");
    headerLine.consumeSpaceSeparatedTokenAhead("Code");

    const uint32_t numCodeBytes = headerLine.readDecimalUint();
    headerLine.consumeSpaceSeparatedTokenAhead("bytes");
    headerLine.skipAsciiWhiteSpace();
    headerLine.ensureAtEnd();

    // Read the bytes
    std::vector<std::byte>& sectionBytes = pSection->data;
    sectionBytes.reserve(sectionBytes.size() + numCodeBytes);

    for (uint32_t numBytesLeft = numCodeBytes; numBytesLeft > 0;) {
        // Skip the offset at the start of the line first
        TextStream dataLine = text.readNextLineAsStream();
        dataLine.skipAsciiWhiteSpace();

        if (dataLine.isAtEnd())
            continue;

        dataLine.readHexUint();
        dataLine.consumeSpaceSeparatedTokenAhead(":");

        // Continue reading bytes in the line
        while ((!dataLine.isAtEnd()) && (numBytesLeft > 0)) {
            if (dataLine.skipAsciiWhiteSpace()) {
                continue;
            }

            uint32_t byte = (dataLine.readHexDigit() << 4);
            byte |= dataLine.readHexDigit();
            sectionBytes.push_back((std::byte) byte);
            --numBytesLeft;
        }

        dataLine.skipAsciiWhiteSpace();
        dataLine.ensureAtEnd();
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Parses text like:
//      0 : End of file
//----------------------------------------------------------------------------------------------------------------------
static void parseEndOfFileDirective(TextStream& text, [[maybe_unused]] ObjFile& out) {
    TextStream line = text.readNextLineAsStream();
    line.consumeSpaceSeparatedTokenAhead("0");
    line.consumeSpaceSeparatedTokenAhead(":");
    line.consumeSpaceSeparatedTokenAhead("End");
    line.consumeSpaceSeparatedTokenAhead("of");
    line.consumeSpaceSeparatedTokenAhead("file");
    line.skipAsciiWhiteSpace();
    line.ensureAtEnd();
}

bool ObjFileParser::parseObjFileDumpFromStr(const std::string& str, ObjFile& out) noexcept {
    TextStream text(str.c_str(), (uint32_t) str.size());

    try {
        // Main parsing loop: continue handling elements until we reach the file end!
        while (!text.isAtEnd()) {
            // If there is white space ahead then skip and retry again
            if (text.skipAsciiWhiteSpace()) {
                continue;
            }

            // Parsing various elements ahead
            if (text.checkStringAhead("Header")) {
                parseHeaderInfo(text, out);
            } else if (text.checkStringAhead("76")) {
                parseFunctionEndDirective(text, out);
            } else if (text.checkStringAhead("74")) {
                parseFunctionStartDirective(text, out);
            } else if (text.checkStringAhead("48")) {
                parseXBssSymbolDirective(text, out);
            } else if (text.checkStringAhead("46")) {
                parseProcessorType(text, out);
            } else if (text.checkStringAhead("28")) {
                parseFileNameDefinition(text, out);
            } else if (text.checkStringAhead("18")) {
                parseLocalSymbolDirective(text, out);
            } else if (text.checkStringAhead("16")) {
                parseSectionDefinition(text, out);
            } else if (text.checkStringAhead("14")) {
                parseXRefSymbolDirective(text, out);
            } else if (text.checkStringAhead("12")) {
                parseXDefSymbolDirective(text, out);
            } else if (text.checkStringAhead("10")) {
                parsePatchDirective(text, out);
            } else if (text.checkStringAhead("8")) {
                parseUnitializedDataDirective(text, out);
            } else if (text.checkStringAhead("6")) {
                parseSwitchToSectionDirective(text, out);
            } else if (text.checkStringAhead("2")) {
                parseCodeDirective(text, out);
            } else if (text.checkStringAhead("0")) {
                parseEndOfFileDirective(text, out);
                break;
            } else {
                throw ParseException("Unknown directive in obj file dump!");
            }
        }

        return true;
    } catch (ParseException e) {
        std::printf("\nParsing failed due to parse exception: %s", e.msg);
        return false;
    } catch (TextStreamException e) {
        std::printf("\nParsing failed due to text stream exception: %s", e.msg);
        return false;
    }
}

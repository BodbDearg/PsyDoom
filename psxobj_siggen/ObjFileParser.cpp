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

bool ObjFileParser::parseObjFileDumpFromStr(const std::string& str, ObjFile& out) noexcept {
    try {
        TextStream text(str.c_str(), (uint32_t) str.size());

        // Main parsing loop: continue handling elements until we reach the file end!
        while (!text.isAtEnd()) {
            // If there is white space ahead then skip and retry again
            if (text.skipAsciiWhiteSpace()) {
                continue;
            }

            // Parsing various elements ahead
            if (text.checkStringAhead("Header")) {
                parseHeaderInfo(text, out);
            } else if (text.checkStringAhead("46")) {
                parseProcessorType(text, out);
            } else if (text.checkStringAhead("16")) {
                parseSectionDefinition(text, out);
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

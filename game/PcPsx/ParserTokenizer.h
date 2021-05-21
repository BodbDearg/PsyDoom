#include "Macros.h"

#include <functional>

BEGIN_NAMESPACE(ParserTokenizer)

// Visitor function for a new line in a block of text: receives the index of the line visited
typedef std::function<void (const int32_t lineIdx)> VisitLineFunc;

// Visitor function for a token in a line: receives the index of the token in the line and the string for the token itself
typedef std::function<void (
    const int32_t tokenIdx,
    const char* const token,
    const size_t tokenLen
)> VisitTokenFunc;

bool getNextLine(const char* const pStr, const char* const pStrEnd, const char*& pBeg, const char*& pEnd) noexcept;
bool getNextToken(const char* const pStr, const char* const pStrEnd, const char*& pBeg, const char*& pEnd) noexcept;

void visitAllLineTokens(
const char* const pStrBeg,
    const char* const pStrEnd,
    const VisitLineFunc& onNewLine,
    const VisitTokenFunc& onToken
) noexcept;

END_NAMESPACE(ParserTokenizer)

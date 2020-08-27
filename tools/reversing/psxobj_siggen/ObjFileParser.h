#pragma once

#include <string>

struct ObjFile;

namespace ObjFileParser {
    bool parseObjFileDumpFromStr(const std::string& str, ObjFile& out) noexcept;
}

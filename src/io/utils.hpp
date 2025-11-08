#pragma once
#include <string>

namespace io {

inline std::string trim(const std::string& s) {
    const char* ws = " \t\n\r\f\v";
    size_t start = s.find_first_not_of(ws);
    size_t end   = s.find_last_not_of(ws);
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

}  // namespace io

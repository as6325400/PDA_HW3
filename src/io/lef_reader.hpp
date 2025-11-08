#pragma once
#include <string>
#include "../core/design.hpp"

namespace io {

class LefReader {
public:
  void read(const std::string& path, core::Design& d);
};

} 

#pragma once
#include <string>
#include "../core/design.hpp"

namespace io {

class DefWriter {
public:
  void write(const std::string& in_def,
             const core::Design& d,
             const std::string& out_def);
};

} // namespace io

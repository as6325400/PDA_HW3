#include "def_writer.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace io {

static void write_components_block(std::ostream& os, const core::Design& d) {

  os << "COMPONENTS " << d.instances.size() << " ;\n";
  for (const auto& it : d.instances) {
    const auto& inst = it.second;
    const std::string& macro  = inst.macro;
    const std::string orient  = inst.orient;
    const std::string status = inst.fixed ? "FIXED" : "PLACED";

    os << "  - " << inst.name << ' ' << macro << "\n"
       << "    + " << status << " ( " << inst.x << ' ' << inst.y
       << " ) " << orient << " ;\n";
  }
  os << "END COMPONENTS\n";
}

void DefWriter::write(const std::string& in_def,
                      const core::Design& d,
                      const std::string& out_def)
{
  std::ifstream fin(in_def);
  if (!fin.is_open())
    throw std::runtime_error("Cannot open input DEF: " + in_def);

  std::ofstream fout(out_def, std::ios::trunc);
  if (!fout.is_open())
    throw std::runtime_error("Cannot open output DEF: " + out_def);

  std::string line;
  bool in_components = false;
  bool replaced = false;

  while (std::getline(fin, line)) {
    const std::string trimmed = io::trim(line);

    if (!in_components && !trimmed.empty() && trimmed.rfind("COMPONENTS", 0) == 0) {
      in_components = true;
      write_components_block(fout, d);
      replaced = true;
      continue;
    }

    if (in_components) {
      if (trimmed.rfind("END COMPONENTS", 0) == 0) {
        in_components = false;
      }
      continue;
    }

    fout << line << '\n';
  }
}

} // namespace io

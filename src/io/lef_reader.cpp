#include "lef_reader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace core;

namespace io {

void LefReader::read(const std::string& path, Design& d) {
  std::ifstream fin(path);
  if (!fin.is_open())
    throw std::runtime_error("Cannot open LEF: " + path);

  std::string line;
  std::string current_macro;
  int dbu = 0;
  Site site;
  Macro macro;

  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token.empty()) continue;

    // ---- UNITS ----
    if (token == "DATABASE") {
      std::string tmp;
      iss >> tmp; // MICRONS
      iss >> dbu;
      d.units.lef_dbu_per_um = dbu;
    }

    // ---- SITE ----
    else if (token == "SITE") {
      iss >> site.name;
    }
    else if (token == "SIZE" && site.name != "" && current_macro.empty()) {
      double w, h;
      std::string by;
      iss >> w >> by >> h; // SIZE 0.1 BY 1.2
      site.w_dbu = static_cast<int>(w * d.units.lef_dbu_per_um);
      site.h_dbu = static_cast<int>(h * d.units.lef_dbu_per_um);
      d.upsertSite(site);
    }

    // ---- MACRO ----
    else if (token == "MACRO") {
      iss >> current_macro;
      macro = Macro{};
      macro.name = current_macro;
    }
    else if (token == "SIZE" && !current_macro.empty()) {
      double w, h;
      std::string by;
      iss >> w >> by >> h;
      macro.w_dbu = static_cast<int>(w * d.units.lef_dbu_per_um);
      macro.h_dbu = static_cast<int>(h * d.units.lef_dbu_per_um);
    }
    if (token == "SYMMETRY") {
      std::string sym;
      while (iss >> sym && sym != ";") {
        if (sym == "X") macro.sym_x = true;
        else if (sym == "Y") macro.sym_y = true;
        else if (sym == "R90") macro.sym_r90 = true;
      }
    }
    else if (token == "END" && !current_macro.empty()) {
      std::string endName;
      iss >> endName;
      if (endName == current_macro) {
        d.upsertMacro(macro);
        current_macro.clear();
      }
    }
  }
}
}

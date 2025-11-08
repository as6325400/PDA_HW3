#include "def_reader.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace core;

namespace io {

void DefReader::read(const std::string& path, Design& d) {
  std::ifstream fin(path);
  if (!fin.is_open())
    throw std::runtime_error("Cannot open DEF: " + path);

  std::string line;
  std::string current_macro;
  int dbu = 0;
  bool inComponents = false;
  Instance inst;

  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token.empty()) continue;

    // ---- UNITS ----
    else if (token == "UNITS") {
      std::string distance, microns;
      iss >> distance >> microns >> dbu;
      if (distance == "DISTANCE" && microns == "MICRONS") {
        d.units.def_dbu_per_um = dbu;
      }
    }

    // ---- DIE AREA ----
    else if (token == "DIEAREA") {
      char ch;
      int x1, y1, x2, y2;
      iss >> ch >> x1 >> y1 >> ch >> ch >> x2 >> y2;
      d.setDieArea(x1, y1, x2, y2);
    }

    else if (token == "ROW") {
      
      std::string rowline = line;
      while (rowline.find(';') == std::string::npos && std::getline(fin, line)) {
          rowline += " " + line;
      }

      std::istringstream rs(rowline);
      std::string kw, name, site, orient;
      int x0, y0;

      rs >> kw >> name >> site >> x0 >> y0 >> orient;  // ROW <name> <site> x y <orient>

      Row r;
      r.name = name;
      r.site = site;
      r.x0 = x0; r.y0 = y0;
      r.orient = orient;

      // DO <nx> BY <ny> STEP <sx> <sy> ;
      std::string t;
      while (rs >> t) {
          if (t == "DO") { rs >> r.nx; }
          else if (t == "BY") { rs >> r.ny; }
          else if (t == "STEP") { rs >> r.step_x >> r.step_y; }
          else if (t.back() == ';') { break; } // 收尾
      }

      d.addRow(r);
    }

    // ---- INSTANCE ----
    else if (token == "COMPONENTS") {
      inComponents = true;
      continue;
    }

    if (inComponents) {
      if (line.find("END COMPONENTS") != std::string::npos) {
        inComponents = false;
        continue;
      }

      std::string comp = line;
      comp = io::trim(comp);
      if (!comp.empty() && comp[0] == '-') {
        while (comp.find(';') == std::string::npos && std::getline(fin, line)) {
          comp += " " + line;
        }

        std::istringstream cs(comp);
        std::string dash;
        cs >> dash; // '-'

        core::Instance inst;
        cs >> inst.name >> inst.macro;

        std::string tok;
        while (cs >> tok) {
          if (tok == "+") {
            continue;
          } else if (tok == "PLACED" || tok == "FIXED") {
            
            if (tok == "FIXED") inst.fixed = true;

            // ( x y ) ORIENT
            char ch;
            // 吃 '(' x y ')'
            cs >> ch >> inst.x >> inst.y >> ch;
            std::string orient;
            // 吃 orient
            cs >> inst.orient;

          } else if (!tok.empty() && tok.back() == ';') {
            break; // 一筆結束
          }
        }
        d.upsertInstance(inst);
      }

      continue;
    }


    // ---- Nets ----
    else if (token == "NETS") {
      int n; iss >> n;
      bool inNets = true;

      while (std::getline(fin, line)) {
        std::string trimmed = io::trim(line);
        if (trimmed == "END NETS") break;

        if (!trimmed.empty() && trimmed[0] == '-') {
          std::string netline = trimmed;
          while (netline.find(';') == std::string::npos && std::getline(fin, line)) {
            netline += " " + io::trim(line);
          }

          std::istringstream ns(netline);
          std::string dash, netName;
          ns >> dash >> netName;

          core::Net net;
          net.name = netName;

          std::string tok;
          while (ns >> tok) {
            if (tok == "(") {
              char ch;
              std::string inst, pin; 
              ns >> inst >> pin >> ch;
              net.insts.push_back(inst);
            } else if (tok == ";") break;
          }


          d.upsertNet(net);
        }
      }
    }




    
  }
}
}
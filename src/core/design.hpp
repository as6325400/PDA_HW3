#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace core {

struct Units {
  int lef_dbu_per_um = 0;
  int def_dbu_per_um = 0;
};

struct Site {
  std::string name;
  int w_dbu = 0;
  int h_dbu = 0;
};

struct Macro {
  std::string name;
  std::string site;
  int w_dbu = 0;
  int h_dbu = 0;
  bool sym_x = false;
  bool sym_y = false;
  bool sym_r90 = false;
  
  bool canFlipX() const noexcept { return sym_x; }
  bool canFlipY() const noexcept { return sym_y; }
  bool canRotate90() const noexcept { return sym_r90; }

};

struct Row {
  std::string name;
  std::string site;
  std::string orient;
  int x0 = 0, y0 = 0;  // 原點 (DEF DBU)
  int nx = 0, ny = 0;  // DO, BY
  int step_x = 0, step_y = 0; // STEP
};

struct Instance {
  std::string name;
  std::string macro;
  int x = 0, y = 0;
  std::string orient;
  bool fixed = false;
  std::vector<std::string> nets;
};

struct Net {
  std::string name;
  std::vector<std::string> insts; // 同一條 net 連接的 instances
};



class Design {
public:
  Units units;

  // === LEF ===
  std::unordered_map<std::string, Site> sites;    // macro_name -> Site
  std::unordered_map<std::string, Macro> macros;  // macro_name -> Macro

  // === DEF ===
  int die_llx = 0;  // 左下角 x
  int die_lly = 0;  // 左下角 y
  int die_urx = 0;  // 右上角 x
  int die_ury = 0;  // 右上角 y
  std::vector<Row> rows;
  std::unordered_map<std::string, Instance> instances;
  std::unordered_map<std::string, Net> nets;

  // --- APIs ---
  void buildInstanceNetLists();
  void upsertSite(const Site& s);
  void upsertMacro(const Macro& m);
  void upsertInstance(const Instance& i);
  void upsertNet(const Net& n);

  void setDieArea(int llx, int lly, int urx, int ury) noexcept {
    die_llx = llx;
    die_lly = lly;
    die_urx = urx;
    die_ury = ury;
  }

  void addRow(Row &r){
    rows.push_back(r);
  }

};

} 
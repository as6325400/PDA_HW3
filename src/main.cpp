#include<bits/stdc++.h>
#include "core/design.hpp"
#include "io/lef_reader.hpp"
#include "io/def_reader.hpp"
#include "placer/detailed_placer.hpp"
#include "io/def_writer.hpp"

using namespace std;

int main(int argc, char* argv[]){
  
  if (argc < 4) {
    cerr << "Usage: " << argv[0] << " <input LEF> <input DEF> <output DEF>\n";
    return 1;
  }

  core::Design d;
  io::LefReader lef;
  io::DefReader def;

  lef.read(argv[1], d);
  def.read(argv[2], d);
  d.buildInstanceNetLists();

  io::DefWriter writer;
  writer.write(argv[2] /*input DEF*/, d, argv[3] /*output DEF*/);

  // cout << "lef DBU " << d.units.lef_dbu_per_um << '\n';
  
  // cout << "----------- Site ----------\n";

  // for (auto it:d.sites){
  //   auto site = it.second;
  //   cout << site.name << ' ' << site.w_dbu << ' ' << site.h_dbu << '\n';
  // }

  // cout << "----------- Site End ----------\n";

  // cout << "---------- Marco ----------\n";
  
  // for (auto mc:d.macros) {
  //   auto marco = mc.second;
  //   cout << marco.name << ' ' << marco.w_dbu << ' ' << marco.h_dbu << '\n';
  //   cout << "Can flip X " << marco.canFlipX();
  //   cout << " Can flip Y " << marco.canFlipY();
  //   cout << " Can Rotate90 " << marco.canRotate90();
  //   cout << '\n';
  // }

  // cout << "---------- Marco End ----------\n";

  // cout << "def DBU" << d.units.def_dbu_per_um << '\n';
  
  // cout << "---------- Die area ----------\n";

  // cout << d.die_llx << ' ' << d.die_lly << ' ' << d.die_urx << ' ' << d.die_ury << '\n';

  // cout << "---------- Die area End ----------\n";


  // cout << "---------- Row ----------\n";

  // for (auto row:d.rows) {
  //   cout << row.name << '\n';
  //   cout << row.nx << ' ' << row.ny << '\n';
  //   cout << row.orient << '\n';
  //   cout << row.site << '\n';
  //   cout << row.step_x << ' ' << row.step_y << '\n';
  //   cout << row.x0 << ' ' << row.y0 << '\n';
  // }

  // cout << "---------- Row End ----------\n";

  // cout << "---------- Instance ----------\n";

  // for (auto i:d.instances){
  //   auto inst = i.second;
  //   cout << inst.name << ' ';
  //   cout << inst.macro << '\n';
  //   cout << "fixed " << inst.fixed << '\n';
  //   cout << inst.orient << ' ' << inst.x << ' ' << inst.y << '\n';

  //   for(auto net:inst.nets){
  //     cout << net << ' ';
  //   }
  //   cout << '\n';

  // }

  // cout << "---------- Instance End ----------\n";

  // cout << "---------- Nets ----------\n";

  // for (auto it:d.nets){
  //   auto net = it.second;
  //   cout << net.name << '\n';
  //   for(auto inst : net.insts) {
  //     cout << inst << ' ';
  //   }
  //   cout << '\n';
  // }

  // cout << "---------- Nets ----------\n";

}
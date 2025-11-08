#include "design.hpp"

namespace core {


void Design::upsertSite(const Site& s) {
  sites[s.name] = s;
}

void Design::upsertMacro(const Macro& m) {
  macros[m.name] = m;
}

void Design::upsertInstance(const Instance& i) {
  instances[i.name] = i;
}

void Design::upsertNet(const Net& n) {
  nets[n.name] = n;
}

void Design::buildInstanceNetLists() {

  for (const auto& [netName, net] : nets) {
    for (const auto& instName : net.insts) {
      auto it = instances.find(instName);
      if (it == instances.end()) continue;
      it->second.nets.push_back(netName);
    }
  }
}

}

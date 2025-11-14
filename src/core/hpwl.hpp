#pragma once
#include <limits>
#include <algorithm>
#include "design.hpp"

namespace core {
  inline long long hpwl_counts(const Design& d){

    long long ans = 0;

    for (const auto &[netName, net]:d.nets){

      int x_min = std::numeric_limits<int>::max(), x_max = std::numeric_limits<int>::min();
      int y_min = std::numeric_limits<int>::max(), y_max = std::numeric_limits<int>::min();

      for (const auto &instName:net.insts){
        auto it = d.instances.find(instName);
        const auto &inst = it -> second;
        if (it == d.instances.end()) {
          // 如果想 debug，可以暫時印出來：
          std::cerr << "[WARN] net " << netName
                    << " has unknown inst " << instName << "\n";
          continue;
        }
        x_min = std::min(inst.x, x_min);
        y_min = std::min(inst.y, y_min);
        x_max = std::max(inst.x, x_max);
        y_max = std::max(inst.y, y_max);
      }

      for (const auto &pinName:net.pins){
        auto it = d.pins.find(pinName);
        const auto &pin = it -> second;
        x_min = std::min(pin.x, x_min);
        y_min = std::min(pin.y, y_min);
        x_max = std::max(pin.x, x_max);
        y_max = std::max(pin.y, y_max);
      }

      ans += (x_max - x_min) + (y_max - y_min);
    }
    return ans;
  }

}
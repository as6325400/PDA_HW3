#pragma once
#include <vector>
#include <string>
#include <limits>
#include <random>
#include "../core/design.hpp"

namespace placer {

class DetailedPlacer {
public:
  struct Params {
    int iters = 2000000;       // 嘗試次數
    int max_step_sites = 4;  // 一次最多走幾個 site
    unsigned seed = 12345;   // 隨機種子
  };

  DetailedPlacer();                       // 使用預設參數
  explicit DetailedPlacer(const Params&); // 自訂參數

  void run(core::Design& d);

private:
  struct Track {
    std::string site;
    std::string orient;
    int y = 0;
    int x0 = 0;
    int nx = 0;
    int step_x = 0;
    int x_end() const { return x0 + nx * step_x; }
  };

  Params params_;
  std::mt19937 rng_; // 在建構子初始化

  static std::vector<Track> expandTracks(const core::Design& d);
  static int  nearestTrack(const std::vector<Track>& ts, int y);
  static int  siteWidth(const core::Design& d, const std::string& siteName);
  static int  snapToGrid(int x, int x0, int step);
  static bool legalOnTrack(const core::Design& d,
                           const std::vector<Track>& ts, int tidx,
                           const std::string& instName, int w, int newX);
  static long long deltaHpwlIfMove(const core::Design& d,
                                   const std::string& instName,
                                   int nx, int ny);
  bool tryShiftOnTrack(core::Design& d,
                       const std::vector<Track>& ts, int tidx,
                       const std::string& instName,
                       int step_sites);
};

} // namespace placer

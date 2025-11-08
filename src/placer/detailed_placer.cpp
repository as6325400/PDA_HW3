#include "detailed_placer.hpp"
#include <algorithm>
#include <cmath>
#include <climits>
#include <unordered_set>

using namespace core;

namespace placer {


DetailedPlacer::DetailedPlacer()
  : params_(), rng_(params_.seed) {}

DetailedPlacer::DetailedPlacer(const Params& p)
  : params_(p), rng_(params_.seed) {}
// ---------- 小工具 ----------

std::vector<DetailedPlacer::Track>
DetailedPlacer::expandTracks(const Design& d) {
  std::vector<Track> ts;
  ts.reserve(d.rows.size());

  for (const auto& r : d.rows) {
    for (int j = 0; j < std::max(1, r.ny); ++j) {
      Track t;
      t.site   = r.site;
      t.orient = r.orient;
      t.y      = r.y0 + j * r.step_y;
      t.x0     = r.x0;
      t.nx     = std::max(1, r.nx);
      t.step_x = r.step_x;
      if (t.step_x <= 0) continue; // 防呆
      ts.push_back(t);
    }
  }

  // Y 由小到大排序，方便找最近列
  std::sort(ts.begin(), ts.end(), [](const Track& a, const Track& b){
    if (a.y != b.y) return a.y < b.y;
    return a.x0 < b.x0;
  });
  return ts;
}

int DetailedPlacer::nearestTrack(const std::vector<Track>& ts, int y) {
  if (ts.empty()) return -1;
  int best = 0;
  int bestd = std::abs(ts[0].y - y);
  for (int i = 1; i < (int)ts.size(); ++i) {
    int d = std::abs(ts[i].y - y);
    if (d < bestd) { bestd = d; best = i; }
  }
  return best;
}

int DetailedPlacer::siteWidth(const Design& d, const std::string& siteName) {
  auto it = d.sites.find(siteName);
  if (it == d.sites.end()) return 0;
  return it->second.w_dbu;
}

int DetailedPlacer::snapToGrid(int x, int x0, int step) {
  if (step <= 0) return x;
  // 以 x0 為原點，四捨五入到最近的 step 倍
  const double k = double(x - x0) / double(step);
  const int    q = (int)std::llround(k);
  return x0 + q * step;
}

bool DetailedPlacer::legalOnTrack(const Design& d,
                                  const std::vector<Track>& ts, int tidx,
                                  const std::string& instName, int w, int newX)
{
  if (tidx < 0 || tidx >= (int)ts.size()) return false;
  const auto& t = ts[tidx];

  // 必須落在 [x0, x_end - w] 之內
  if (newX < t.x0) return false;
  if (newX + w > t.x_end()) return false;

  // 與同一列上的其他 instance 不可重疊
  const int newL = newX;
  const int newR = newX + w;

  for (const auto& kv : d.instances) {
    const auto& name = kv.first;
    const auto& inst = kv.second;
    if (name == instName) continue;
    if (inst.y != t.y) continue;      // 只看同一列
    // 固定或可動都不能重疊
    auto itM = d.macros.find(inst.macro);
    if (itM == d.macros.end()) continue; // 沒找到 macro，略過
    const int w2 = itM->second.w_dbu;

    const int L2 = inst.x;
    const int R2 = inst.x + w2;
    // 半開區間 [L,R)：不重疊條件為 R<=L2 或 R2<=L
    const bool overlap = !(newR <= L2 || R2 <= newL);
    if (overlap) return false;
  }
  return true;
}

static inline long long hpwl_of_net_with_override(
  const Design& d, const Net& net,
  const std::string& overrideInst, int ox, int oy)
{
  if (net.insts.size() <= 1u) return 0;

  int xmin = INT_MAX, xmax = INT_MIN;
  int ymin = INT_MAX, ymax = INT_MIN;

  for (const auto& in : net.insts) {
    auto itI = d.instances.find(in);
    if (itI == d.instances.end()) continue;

    int px = itI->second.x;
    int py = itI->second.y;
    if (in == overrideInst) { px = ox; py = oy; }

    xmin = std::min(xmin, px);
    xmax = std::max(xmax, px);
    ymin = std::min(ymin, py);
    ymax = std::max(ymax, py);
  }

  if (xmin == INT_MAX) return 0;
  return (long long)(xmax - xmin) + (long long)(ymax - ymin);
}

long long DetailedPlacer::deltaHpwlIfMove(const Design& d,
                                          const std::string& instName,
                                          int nx, int ny)
{
  auto it = d.instances.find(instName);
  if (it == d.instances.end()) return 0;
  const auto& inst = it->second;

  long long delta = 0;
  for (const auto& netName : inst.nets) {
    auto itN = d.nets.find(netName);
    if (itN == d.nets.end()) continue;
    const Net& net = itN->second;

    long long before = hpwl_of_net_with_override(d, net, instName, inst.x, inst.y);
    long long after  = hpwl_of_net_with_override(d, net, instName, nx, ny);
    delta += (after - before);
  }
  return delta;
}

bool DetailedPlacer::tryShiftOnTrack(Design& d,
                                     const std::vector<Track>& ts, int tidx,
                                     const std::string& instName,
                                     int step_sites)
{
  if (step_sites == 0) return false;

  auto itI = d.instances.find(instName);
  if (itI == d.instances.end()) return false;
  Instance& inst = itI->second;

  // 只處理可動元件
  if (inst.fixed) return false;

  const auto itM = d.macros.find(inst.macro);
  if (itM == d.macros.end()) return false;
  const int w = itM->second.w_dbu;

  const Track& t = ts[tidx];
  const int sx = siteWidth(d, t.site);
  if (sx <= 0) return false;

  // 目標座標：在同一列左右移動 step_sites 個 site
  int targetX = inst.x + step_sites * sx;
  // 對齊 grid
  targetX = snapToGrid(targetX, t.x0, t.step_x);

  // 邊界裁切
  targetX = std::max(targetX, t.x0);
  targetX = std::min(targetX, t.x_end() - w);

  // 合法性檢查（不重疊 & 在範圍內）
  if (!legalOnTrack(d, ts, tidx, instName, w, targetX)) return false;

  // 計算 delta HPWL（只重算 inst 相關 nets）
  const long long delta = deltaHpwlIfMove(d, instName, targetX, t.y);
  if (delta < 0) {
    // 接受移動
    inst.x = targetX;
    inst.y = t.y; // 保持在這條列
    return true;
  }
  return false;
}

// ---------- 主流程 ----------

void DetailedPlacer::run(Design& d) {
  // 準備 tracks（把每個 ROW 的 BY>1 展開）
  auto tracks = expandTracks(d);
  if (tracks.empty()) return;

  // 初步對齊所有可動 instances 到最近列 & site 格點（不合法就 clamp）
  for (auto& kv : d.instances) {
    auto& inst = kv.second;
    if (inst.fixed) continue;

    // 找該 instance 最近的 track
    int tidx = nearestTrack(tracks, inst.y);
    if (tidx < 0) continue;
    const auto& t = tracks[tidx];

    // 取得 cell 寬度與 site 寬
    auto itM = d.macros.find(inst.macro);
    if (itM == d.macros.end()) continue;
    const int w = itM->second.w_dbu;
    const int sx = siteWidth(d, t.site);
    if (sx <= 0) continue;

    // snap + clamp
    int nx = snapToGrid(inst.x, t.x0, t.step_x);
    nx = std::max(nx, t.x0);
    nx = std::min(nx, t.x_end() - w);

    // 若重疊，往右邊掃到第一個可行位置（O(N) 但夠用了）
    int tryX = nx;
    const int limit = t.x_end() - w;
    bool placed = false;
    while (tryX <= limit) {
      if (legalOnTrack(d, tracks, tidx, inst.name, w, tryX)) {
        inst.x = tryX;
        inst.y = t.y;
        placed = true;
        break;
      }
      tryX += sx;
    }
    if (!placed) {
      // 放不下就回到原位（極少數情況），不處理
    }
  }

  // 迭代改善：隨機挑 cell，嘗試小幅左右移動（貪婪接受）
  std::uniform_int_distribution<int> coin(0, 1);
  std::uniform_int_distribution<int> step(1, std::max(1, params_.max_step_sites));

  // 收集 movable instances
  std::vector<std::string> movables;
  movables.reserve(d.instances.size());
  for (auto& kv : d.instances) if (!kv.second.fixed) movables.push_back(kv.first);
  if (movables.empty()) return;

  std::uniform_int_distribution<int> pick(0, (int)movables.size() - 1);

  for (int it = 0; it < params_.iters; ++it) {
    const std::string& name = movables[pick(rng_)];

    auto itI = d.instances.find(name);
    if (itI == d.instances.end()) continue;
    auto& inst = itI->second;

    int tidx = nearestTrack(tracks, inst.y);
    if (tidx < 0) continue;

    // 正右負左
    int dir = coin(rng_) ? 1 : -1;
    int steps = step(rng_) * dir;

    tryShiftOnTrack(d, tracks, tidx, name, steps);
  }
}

} // namespace placer

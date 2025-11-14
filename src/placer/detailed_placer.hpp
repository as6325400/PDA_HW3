#pragma once
#include "../core/design.hpp"
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <map>
#include <set>
#include <iostream>

using namespace std;

// ==========================================
// 1. Min-Cost Max-Flow (MCMF) Solver
// ==========================================
class MinCostMaxFlow {
public:
    struct Edge {
        int to;
        int capacity;
        int flow;
        long long cost; // 使用 long long 避免 HPWL 加總溢位
        int rev; // 反向邊在 adjacency list 中的索引
    };

    vector<vector<Edge>> adj;
    vector<long long> dist;
    vector<int> parent_edge;
    vector<int> parent_node;
    int n;
    int s, t;

    MinCostMaxFlow(int num_nodes, int source, int sink) : n(num_nodes), s(source), t(sink) {
        adj.resize(n);
        dist.resize(n);
        parent_edge.resize(n);
        parent_node.resize(n);
    }

    void add_edge(int u, int v, int cap, long long cost) {
        Edge a = {v, cap, 0, cost, (int)adj[v].size()};
        Edge b = {u, 0, 0, -cost, (int)adj[u].size()}; // 反向邊容量為 0，成本為負
        adj[u].push_back(a);
        adj[v].push_back(b);
    }

    // 使用 SPFA 尋找最短路徑 (在有負權邊但無負環的圖中適用，這裡其實只有正權，也可以用 Dijkstra)
    bool spfa() {
        fill(dist.begin(), dist.end(), numeric_limits<long long>::max());
        vector<bool> in_queue(n, false);
        queue<int> q;

        dist[s] = 0;
        q.push(s);
        in_queue[s] = true;

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            in_queue[u] = false;

            for (int i = 0; i < adj[u].size(); ++i) {
                Edge &e = adj[u][i];
                if (e.capacity - e.flow > 0 && dist[e.to] > dist[u] + e.cost) {
                    dist[e.to] = dist[u] + e.cost;
                    parent_node[e.to] = u;
                    parent_edge[e.to] = i;
                    if (!in_queue[e.to]) {
                        q.push(e.to);
                        in_queue[e.to] = true;
                    }
                }
            }
        }
        return dist[t] != numeric_limits<long long>::max();
    }

    pair<int, long long> solve() {
        int total_flow = 0;
        long long total_cost = 0;

        while (spfa()) {
            int push = numeric_limits<int>::max();
            int curr = t;
            while (curr != s) {
                int prev = parent_node[curr];
                int edge_idx = parent_edge[curr];
                push = min(push, adj[prev][edge_idx].capacity - adj[prev][edge_idx].flow);
                curr = prev;
            }

            total_flow += push;
            curr = t;
            while (curr != s) {
                int prev = parent_node[curr];
                int edge_idx = parent_edge[curr];
                adj[prev][edge_idx].flow += push;
                int rev_idx = adj[prev][edge_idx].rev;
                adj[curr][rev_idx].flow -= push;
                total_cost += (long long)push * adj[prev][edge_idx].cost;
                curr = prev;
            }
        }
        return {total_flow, total_cost};
    }
};

// ==========================================
// 2. Detailed Placer Class
// ==========================================
class DetailedPlacer {
public:
    core::Design& design;

    DetailedPlacer(core::Design& d) : design(d) {}

    // 一個簡單的座標結構
    struct Pos { int x, y; };

    // 計算 HPWL 的輔助函數
    long long compute_net_hpwl(const core::Net& net, const string& moving_inst_name, int new_x, int new_y) {
        long long min_x = numeric_limits<long long>::max();
        long long min_y = numeric_limits<long long>::max();
        long long max_x = numeric_limits<long long>::min();
        long long max_y = numeric_limits<long long>::min();

        // 根據 spec，pin 位置簡化為 instance 的左下角座標
        // 遍歷這個 net 連接的所有 instance
        for (const string& inst_name : net.insts) {
            int cur_x, cur_y;
            if (inst_name == moving_inst_name) {
                // 如果是我們正在嘗試移動的單元，使用新的測試座標
                cur_x = new_x;
                cur_y = new_y;
            } else {
                // 否則使用它目前在 Design 中的座標
                // 注意：這裡需要能快速從 design 中找到 instance。
                // 假設 design.instances 是一個 map<string, Instance>
                const auto& other_inst = design.instances.at(inst_name);
                cur_x = other_inst.x;
                cur_y = other_inst.y;
            }
            min_x = min(min_x, (long long)cur_x);
            max_x = max(max_x, (long long)cur_x);
            min_y = min(min_y, (long long)cur_y);
            max_y = max(max_y, (long long)cur_y);
        }

        // 如果 net 只有一個 pin 或沒有 pin，HPWL 為 0
        if (min_x == numeric_limits<long long>::max()) return 0;

        return (max_x - min_x) + (max_y - min_y);
    }

    // 計算將單元 inst 放到位置 (site_x, site_y) 的總成本
    long long calculate_cost(const string& inst_name, int site_x, int site_y) {
        long long total_cost = 0;
        const auto& inst = design.instances.at(inst_name);
        // 遍歷該單元連接的所有 net
        for (const string& net_name : inst.nets) {
             // 假設 design.nets 是一個 map<string, Net>
            const auto& net = design.nets.at(net_name);
            total_cost += compute_net_hpwl(net, inst_name, site_x, site_y);
        }
        return total_cost;
    }

    // =====================================================
    // 核心演算法：給定一個區域的單元和空位，進行 MCMF 最佳化
    // modules: 要在這個區域內重新排列的單元名稱列表 (C_i)
    // sites: 這個區域內可用的合法位置座標列表 (P_j)
    // =====================================================
    void solveRegion(const vector<string>& modules, const vector<Pos>& sites) {
        int k = modules.size(); // 單元數量
        int m = sites.size();   // 位置數量

        if (k == 0 || m == 0) return;
        if (k > m) {
            cerr << "Error: More modules than sites in region!" << endl;
            return;
        }

        // 節點編號：
        // Source S = 0
        // Sink T = k + m + 1
        // Modules C_i = 1 ... k
        // Sites P_j = k + 1 ... k + m
        int S = 0;
        int T = k + m + 1;
        MinCostMaxFlow mcmf(T + 1, S, T);

        // 1. 建立從 Source 到每個 Module 的邊
        for (int i = 0; i < k; ++i) {
            mcmf.add_edge(S, i + 1, 1, 0); // 容量 1，成本 0
        }

        // 2. 建立從每個 Site 到 Sink 的邊
        for (int j = 0; j < m; ++j) {
            mcmf.add_edge(k + 1 + j, T, 1, 0); // 容量 1，成本 0
        }

        // 3. 建立從每個 Module 到每個 Site 的邊，並計算成本
        // 這是最耗時的部分，複雜度 O(k * m * avg_nets_per_cell * avg_pins_per_net)
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < m; ++j) {
                long long cost = calculate_cost(modules[i], sites[j].x, sites[j].y);
                mcmf.add_edge(i + 1, k + 1 + j, 1, cost);
            }
        }

        // 4. 求解 MCMF
        // cout << "Running MCMF for region with " << k << " modules and " << m << " sites..." << endl;
        pair<int, long long> result = mcmf.solve();

        if (result.first != k) {
            cerr << "Warning: MCMF did not find a perfect matching! Flow: " << result.first << "/" << k << endl;
            // 在實際應用中，這裡可能需要 fallback 策略，或者這意味著輸入有問題
        }

        // 5. 解析結果並更新單元位置
        // 檢查從 Module 節點出發的所有邊，找出有流量的邊
        for (int i = 0; i < k; ++i) {
            int u = i + 1; // Module C_i 的節點編號
            for (const auto& e : mcmf.adj[u]) {
                // 如果這條邊有流量 (flow == 1)，且是指向 Site 節點的
                if (e.flow == 1 && e.to > k && e.to <= k + m) {
                    int site_idx = e.to - (k + 1); // 取得 Site 的索引 j
                    
                    // 更新設計中單元的座標
                    // 注意：這裡直接修改了 design 的全域狀態
                    design.instances.at(modules[i]).x = sites[site_idx].x;
                    design.instances.at(modules[i]).y = sites[site_idx].y;
                    
                    // cout << "Moved " << modules[i] << " to (" << sites[site_idx].x << ", " << sites[site_idx].y << ")" << endl;
                    break; 
                }
            }
        }
    }
};
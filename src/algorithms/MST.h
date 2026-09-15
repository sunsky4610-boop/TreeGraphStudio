// 替换 MST.h 文件

#pragma once

#include "../core/Graph.h"
#include <functional>
#include <vector>
#include <algorithm>

template<typename GraphType>
class Kruskal {
public:
    using EdgeWeight = int;

    struct EdgeInfo {
        int from, to;
        EdgeWeight weight;
        bool selected;
    };

    explicit Kruskal(const GraphType* graph)
        : m_graph(graph), m_unionFind(0) {}

    void initialize() {
        m_edges.clear();
        m_selectedEdges.clear();
        m_totalWeight = 0;
        m_step = 0;

        // 收集所有边
        for (const auto& edge : m_graph->getEdges()) {
            m_edges.push_back({edge->from(), edge->to(), edge->weight(), false});
        }

        std::sort(m_edges.begin(), m_edges.end(),
            [](const EdgeInfo& a, const EdgeInfo& b) {
                return a.weight < b.weight;
            });

        int maxNodeId = 0;
        const auto& allNodeIds = m_graph->getAllNodeIds();
        for (int id : allNodeIds) {
            maxNodeId = std::max(maxNodeId, id);
        }
        
        // 创建足够大的并查集
        m_unionFind = UnionFind(maxNodeId + 1);
    }

    bool step() {
        if (m_step >= m_edges.size()) {
            return false;
        }

        auto& edge = m_edges[m_step];
        bool wasConnected = m_unionFind.connected(edge.from, edge.to);

        if (!wasConnected) {
            edge.selected = true;
            m_unionFind.unionNodes(edge.from, edge.to);
            m_selectedEdges.push_back(edge);
            m_totalWeight += edge.weight;

            // 选中边回调
            if (m_onEdgeSelected) {
                m_onEdgeSelected(edge.from, edge.to, edge.weight);
            }
            
            // 新增：选中边对应的节点回调
            if (m_onNodeConnected) {
                m_onNodeConnected(edge.from);
                m_onNodeConnected(edge.to);
            }
        } else {
            // 新增：被拒绝的边回调
            if (m_onEdgeRejected) {
                m_onEdgeRejected(edge.from, edge.to, edge.weight);
            }
        }

        // 新增：每次步进都调用进度回调
        if (m_onStepProgress) {
            m_onStepProgress(m_step + 1, m_edges.size());
        }

        m_step++;
        return m_step < m_edges.size();
    }

    void runAll() {
        while (step()) {}
    }

    const std::vector<EdgeInfo>& getSelectedEdges() const {
        return m_selectedEdges;
    }

    EdgeWeight getTotalWeight() const { return m_totalWeight; }

    // 增强回调注册
    void setOnEdgeSelected(std::function<void(int, int, EdgeWeight)> cb) {
        m_onEdgeSelected = cb;
    }
    void setOnEdgeRejected(std::function<void(int, int, EdgeWeight)> cb) {
        m_onEdgeRejected = cb;
    }
    
    // 新增：节点连接回调
    void setOnNodeConnected(std::function<void(int)> cb) {
        m_onNodeConnected = cb;
    }
    
    // 新增：进度回调
    void setOnStepProgress(std::function<void(int, int)> cb) {
        m_onStepProgress = cb;
    }

private:
    class UnionFind {
    public:
        explicit UnionFind(int size) : parent(size), rank(size, 0) {
            for (int i = 0; i < size; ++i) parent[i] = i;
        }

        int find(int x) {
            if (parent[x] != x) {
                parent[x] = find(parent[x]);
            }
            return parent[x];
        }

        void unionNodes(int x, int y) {
            int rootX = find(x);
            int rootY = find(y);
            if (rootX == rootY) return;

            if (rank[rootX] < rank[rootY]) {
                parent[rootX] = rootY;
            } else if (rank[rootX] > rank[rootY]) {
                parent[rootY] = rootX;
            } else {
                parent[rootY] = rootX;
                rank[rootX]++;
            }
        }

        bool connected(int x, int y) {
            return find(x) == find(y);
        }

    private:
        std::vector<int> parent;
        std::vector<int> rank;
    };

    const GraphType* m_graph;
    std::vector<EdgeInfo> m_edges;
    std::vector<EdgeInfo> m_selectedEdges;
    EdgeWeight m_totalWeight{0};
    size_t m_step{0};
    UnionFind m_unionFind;

    std::function<void(int, int, EdgeWeight)> m_onEdgeSelected;
    std::function<void(int, int, EdgeWeight)> m_onEdgeRejected;
    std::function<void(int)> m_onNodeConnected;  // 新增
    std::function<void(int, int)> m_onStepProgress;  // 新增
};
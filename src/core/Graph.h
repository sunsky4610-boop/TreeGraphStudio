#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <queue>
#include <stack>
#include <algorithm>
#include <unordered_set>
#include <QDebug>
#include <set>

// Node.h和Edge.h的内容合并到这里，避免循环依赖
template<typename DataType = std::string>
class Node {
public:
    explicit Node(int id, const DataType& data = DataType{})
        : m_id(id), m_data(data) {}

    int id() const { return m_id; }
    DataType& data() { return m_data; }
    const DataType& data() const { return m_data; }
    void setData(const DataType& data) { m_data = data; }
    void setPosition(double x, double y) { m_x = x; m_y = y; }
    double x() const { return m_x; }
    double y() const { return m_y; }

private:
    int m_id;
    DataType m_data;
    double m_x{0}, m_y{0};
};

template<typename WeightType = int>
class Edge {
public:
    Edge(int from, int to, WeightType weight = WeightType{})
        : m_from(from), m_to(to), m_weight(weight) {}

    int from() const { return m_from; }
    int to() const { return m_to; }
    WeightType weight() const { return m_weight; }
    void setWeight(WeightType w) { m_weight = w; }
    bool operator<(const Edge& other) const {
        return m_weight < other.m_weight;
    }
private:
    int m_from, m_to;
    WeightType m_weight;
};

/**
 * @brief 图基类 - 模板编程核心
 */
template<typename NodeData = std::string, typename EdgeWeight = int, bool Directed = false>
class Graph {
public:
    using NodeType = Node<NodeData>;
    using EdgeType = Edge<EdgeWeight>;

    virtual ~Graph() = default;

    // 节点操作
    int addNode(const NodeData& data = NodeData{}) {
        int id = m_nodes.size();
        m_nodes.emplace_back(std::make_unique<NodeType>(id, data));
        m_adjList[id] = {};
        return id;
    }

    bool removeNode(int nodeId) {
        if (!hasNode(nodeId)) return false;

        // 先收集所有要删除的边（只收集一次）
        auto edgesToRemove = getEdgesToNode(nodeId);
        
        // 创建要删除的边ID集合，避免重复
        std::set<std::pair<int, int>> edgesToDelete;
        for (const auto& edge : edgesToRemove) {
            int from = edge->from();
            int to = edge->to();
            if (!Directed) {
                // 无向图：规范化边表示（较小ID在前）
                if (from > to) std::swap(from, to);
            }
            edgesToDelete.insert({from, to});
        }

        // 删除所有相关的边
        for (const auto& [from, to] : edgesToDelete) {
            // 对于有向图，只删除指定方向的边
            // 对于无向图，removeEdge会处理两个方向
            removeEdge(from, to);
        }

        // 删除节点
        m_nodes[nodeId].reset();
        m_adjList.erase(nodeId);

        // 从邻接表中删除对该节点的引用
        for (auto& [id, neighbors] : m_adjList) {
            neighbors.erase(
                std::remove(neighbors.begin(), neighbors.end(), nodeId),
                neighbors.end()
            );
        }

        return true;
    }

    std::vector<int> getAllNodeIds() const {
        std::vector<int> ids;
        for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
            if (m_nodes[i]) {
                ids.push_back(i);
            }
        }
        return ids;
    }

    // 边操作
    bool addEdge(int from, int to, EdgeWeight weight = EdgeWeight{}) {
        if (!hasNode(from) || !hasNode(to)) return false;
        
        // 允许有向图创建平行边（相同方向不同权重）
        // 只有完全相同的三元组(from, to, weight)才拒绝
        bool exists = std::any_of(m_edges.begin(), m_edges.end(),
            [from, to, weight](const auto& e) {
                return e->from() == from && e->to() == to && e->weight() == weight;
            });
        
        if (exists) return false;

        m_edges.emplace_back(std::make_unique<EdgeType>(from, to, weight));
        m_adjList[from].push_back(to);

        if (!Directed) {
            // 对于无向图，也添加反向边
            m_adjList[to].push_back(from);
        }
        return true;
    }

    bool removeEdge(int from, int to) {
        auto it = std::find_if(m_edges.begin(), m_edges.end(),
            [from, to](const auto& e) {
                return e->from() == from && e->to() == to ||
                       (!Directed && e->from() == to && e->to() == from);
            });

        if (it == m_edges.end()) return false;

        m_edges.erase(it);

        // 从邻接表中删除
        if (m_adjList.find(from) != m_adjList.end()) {
            m_adjList[from].erase(
                std::remove(m_adjList[from].begin(), m_adjList[from].end(), to),
                m_adjList[from].end()
            );
        }

        if (!Directed) {
            if (m_adjList.find(to) != m_adjList.end()) {
                m_adjList[to].erase(
                    std::remove(m_adjList[to].begin(), m_adjList[to].end(), from),
                    m_adjList[to].end()
                );
            }
        }
        return true;
    }

    // 查询操作
    bool hasNode(int nodeId) const {
        return nodeId >= 0 && nodeId < static_cast<int>(m_nodes.size()) && m_nodes[nodeId];
    }

    bool hasEdge(int from, int to) const {
        return std::any_of(m_edges.begin(), m_edges.end(),
            [from, to, this](const auto& e) {
                return (e->from() == from && e->to() == to) ||
                       (!Directed && e->from() == to && e->to() == from);
            });
    }

    const NodeType* getNode(int id) const {
        return hasNode(id) ? m_nodes[id].get() : nullptr;
    }

    NodeType* getNode(int id) {
        return hasNode(id) ? m_nodes[id].get() : nullptr;
    }

    std::vector<const EdgeType*> getEdges() const {
        std::vector<const EdgeType*> result;
        for (const auto& e : m_edges) result.push_back(e.get());
        return result;
    }

    std::vector<const EdgeType*> getEdgesToNode(int nodeId) const {
        std::vector<const EdgeType*> result;
        for (const auto& e : m_edges) {
            if (e->from() == nodeId || e->to() == nodeId) {
                result.push_back(e.get());
            }
        }
        return result;
    }

    std::vector<int> getNeighbors(int nodeId) const {
        auto it = m_adjList.find(nodeId);
        if (it == m_adjList.end()) {
            return {};
        }
        return it->second;
    }

    bool isDirected() const { return Directed; }

    int nodeCount() const {
        return std::count_if(m_nodes.begin(), m_nodes.end(),
            [](const auto& n) { return n != nullptr; });
    }

    int edgeCount() const { return m_edges.size(); }

protected:
    std::vector<std::unique_ptr<NodeType>> m_nodes;
    std::vector<std::unique_ptr<EdgeType>> m_edges;
    std::unordered_map<int, std::vector<int>> m_adjList;
};

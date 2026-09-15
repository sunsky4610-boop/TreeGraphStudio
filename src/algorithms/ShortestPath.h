#pragma once

#include "../core/Graph.h"
#include <functional>
#include <limits>
#include <queue>
#include <unordered_set>

/**
 * @brief Dijkstra最短路径算法 - 单源最短路径
 * @tparam GraphType 图类型
 */
template<typename GraphType>
class Dijkstra {
public:
    using DistanceType = int;
    using EdgeWeight = int;

    struct DistanceInfo {
        DistanceType distance;
        int predecessor;
        bool visited;
    };

    explicit Dijkstra(const GraphType* graph)
        : m_graph(graph), m_startNode(-1), m_currentNode(-1) {
        reset();
    }

    void reset() {
        m_distances.clear();
        m_priorityQueue = {};
        m_finished = false;
        m_currentNode = -1;
    }

    void initialize(int startNode) {
        reset();
        if (!m_graph || !m_graph->hasNode(startNode)) return;

        m_startNode = startNode;
        m_priorityQueue = {};

        // 验证图的基本完整性
        const auto& allNodeIds = m_graph->getAllNodeIds();
        if (allNodeIds.empty()) return;

        for (int nodeId : allNodeIds) {
            m_distances[nodeId] = {std::numeric_limits<DistanceType>::max(), -1, false};
        }

        m_distances[startNode] = {0, -1, false};
        m_priorityQueue.push({0, startNode});
        m_currentNode = startNode;
    }

    // 执行一步算法
    bool step() {
        if (m_priorityQueue.empty()) {
            m_finished = true;
            return false;
        }

        DistanceType dist{};
        int node{-1};
        // 跳过已访问节点和过时条目：用循环而非递归，避免过期条目较多时递归层层累积
        while (true) {
            if (m_priorityQueue.empty()) {
                m_finished = true;
                return false;
            }
            auto [d, u] = m_priorityQueue.top();
            m_priorityQueue.pop();
            if (!m_distances[u].visited && d <= m_distances[u].distance) {
                dist = d;
                node = u;
                break;
            }
        }

        m_distances[node].visited = true;
        m_currentNode = node;

        if (m_onNodeVisited) {
            m_onNodeVisited(node);
        }

        // 使用确认的距离
        DistanceType confirmedDist = m_distances[node].distance;

        for (int neighbor : m_graph->getNeighbors(node)) {
            auto edge = findEdge(node, neighbor);
            if (!edge) {
                // 这不应该发生，但为安全起见
                continue;
            }

            DistanceType newDist = confirmedDist + edge->weight();

            if (newDist < m_distances[neighbor].distance) {
                m_distances[neighbor].distance = newDist;
                m_distances[neighbor].predecessor = node;
                m_priorityQueue.push({newDist, neighbor});

                if (m_onEdgeRelaxed) {
                    m_onEdgeRelaxed(node, neighbor, newDist);
                }
            }
        }

        return !m_priorityQueue.empty();
    }

    void runAll() {
        while (step()) {}
    }

    // 获取当前处理的节点
    int getCurrentNode() const {
        return m_currentNode;
    }

    // 获取结果
    std::unordered_map<int, DistanceInfo> getDistances() const {
        return m_distances;
    }

    std::vector<int> getPath(int targetNode) const {
        std::vector<int> path;
        if (m_distances.count(targetNode) == 0) return path;

        int current = targetNode;
        while (current != -1) {
            path.insert(path.begin(), current);
            current = m_distances.at(current).predecessor;
        }
        return path;
    }

    // 回调注册
    void setOnNodeVisited(std::function<void(int)> cb) { m_onNodeVisited = cb; }
    void setOnEdgeRelaxed(std::function<void(int, int, DistanceType)> cb) { m_onEdgeRelaxed = cb; }

private:
    const typename GraphType::EdgeType* findEdge(int from, int to) const {
        if (!m_graph) return nullptr;

        const auto& edges = m_graph->getEdges();
        for (const auto& edge : edges) {
            if (!edge) continue;

            // 有向图：严格匹配方向
            if (m_graph->isDirected()) {
                if (edge->from() == from && edge->to() == to) {
                    return edge;
                }
            }
            // 无向图：允许双向
            else {
                if ((edge->from() == from && edge->to() == to) ||
                    (edge->from() == to && edge->to() == from)) {
                    return edge;
                }
            }
        }
        return nullptr;
    }

    const GraphType* m_graph;
    int m_startNode;
    int m_currentNode;
    std::unordered_map<int, DistanceInfo> m_distances;
    std::priority_queue<std::pair<DistanceType, int>,
                       std::vector<std::pair<DistanceType, int>>,
                       std::greater<>> m_priorityQueue;
    bool m_finished{false};

    std::function<void(int)> m_onNodeVisited;
    std::function<void(int, int, DistanceType)> m_onEdgeRelaxed;
};
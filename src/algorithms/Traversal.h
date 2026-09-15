#pragma once

#include "../core/Graph.h"
#include <functional>
#include <set>
#include <deque>
#include <vector>

template<typename GraphType>
class TraversalAlgorithm {
public:
    enum State { READY, RUNNING, PAUSED, FINISHED };
    using NodeCallback = std::function<void(int)>;
    using EdgeCallback = std::function<void(int, int)>;

    explicit TraversalAlgorithm(const GraphType* graph)
        : m_graph(graph), m_state(READY) {}

    virtual ~TraversalAlgorithm() = default;
    virtual void initialize(int startNode) = 0;
    virtual bool step() = 0;

    void runAll() {
        while (step()) {}
    }

    void reset() {
        m_state = READY;
        m_visited.clear();
        m_visitedOrder.clear();  // 清除访问顺序记录
        m_currentNode = -1;
    }

    State getState() const { return m_state; }
    
    // 返回按访问顺序的节点列表
    const std::vector<int>& getVisitedNodes() const { 
        return this->m_visitedOrder;  // 从基类获取
    }
    
    // 返回用于快速查找的集合
    const std::set<int>& getVisitedSet() const { return this->m_visited; }

    int getCurrentNode() const { return this->m_currentNode; }
    const std::deque<int>& getFrontier() const { return this->m_frontier; }

    void setOnNodeVisited(NodeCallback cb) { this->m_onNodeVisited = cb; }
    void setOnEdgeProcessed(EdgeCallback cb) { this->m_onEdgeProcessed = cb; }

protected:
    const GraphType* m_graph;
    State m_state;
    std::set<int> m_visited;              // 用于快速查找
    std::vector<int> m_visitedOrder;      // 记录访问顺序
    std::deque<int> m_frontier;
    int m_currentNode{-1};
    NodeCallback m_onNodeVisited;
    EdgeCallback m_onEdgeProcessed;
};

/**
 * @brief BFS实现
 */
template<typename GraphType>
class BFS : public TraversalAlgorithm<GraphType> {
public:
    using State = typename TraversalAlgorithm<GraphType>::State;

    explicit BFS(const GraphType* graph)
        : TraversalAlgorithm<GraphType>(graph) {}

    void initialize(int startNode) override {
        this->reset();
        if (!this->m_graph->hasNode(startNode)) return;

        this->m_state = State::RUNNING;
        this->m_frontier.clear();
        this->m_currentNode = -1;
        this->m_frontier.push_back(startNode);
    }

    bool step() override {
        if (this->m_state != State::RUNNING) {
            this->m_state = State::FINISHED;
            return false;
        }

        if (this->m_frontier.empty()) {
            this->m_state = State::FINISHED;
            return false;
        }

        // 只处理一次队首节点
        this->m_currentNode = this->m_frontier.front();
        this->m_frontier.pop_front();

        // 如果节点已访问，直接返回状态但不处理
        if (this->m_visited.count(this->m_currentNode)) {
            bool hasMore = !this->m_frontier.empty();
            if (!hasMore) {
                this->m_state = State::FINISHED;
            }
            return hasMore;
        }

        // 标记为已访问
        this->m_visited.insert(this->m_currentNode);
        this->m_visitedOrder.push_back(this->m_currentNode);
        
        if (this->m_onNodeVisited) {
            this->m_onNodeVisited(this->m_currentNode);
        }

        // 添加未访问的邻居到队列（按ID排序以确保确定性顺序）
        auto neighbors = this->m_graph->getNeighbors(this->m_currentNode);
        std::sort(neighbors.begin(), neighbors.end());  // ← 添加这行！
        for (int neighbor : neighbors) {
            if (!this->m_visited.count(neighbor)) {
                this->m_frontier.push_back(neighbor);
                if (this->m_onEdgeProcessed) {
                    this->m_onEdgeProcessed(this->m_currentNode, neighbor);
                }
            }
        }

        // 检查是否还有更多节点
        bool hasMore = !this->m_frontier.empty();
        if (!hasMore) {
            this->m_state = State::FINISHED;
        }
        return hasMore;
    }
};

/**
 * @brief DFS实现
 */
template<typename GraphType>
class DFS : public TraversalAlgorithm<GraphType> {
public:
    using State = typename TraversalAlgorithm<GraphType>::State;

    explicit DFS(const GraphType* graph)
        : TraversalAlgorithm<GraphType>(graph) {}

    void initialize(int startNode) override {
        this->reset();
        if (!this->m_graph || !this->m_graph->hasNode(startNode)) return;

        this->m_state = State::RUNNING;
        this->m_frontier.clear();
        this->m_currentNode = -1;
        this->m_frontier.push_back(startNode);
    }

    bool step() override {
        if (this->m_state != State::RUNNING) {
            this->m_state = State::FINISHED;
            return false;
        }

        if (this->m_frontier.empty()) {
            this->m_state = State::FINISHED;
            return false;
        }

        // 只处理一次栈顶节点
        this->m_currentNode = this->m_frontier.back();
        this->m_frontier.pop_back();

        // 如果节点已访问，直接返回状态但不处理
        if (this->m_visited.count(this->m_currentNode)) {
            bool hasMore = !this->m_frontier.empty();
            if (!hasMore) {
                this->m_state = State::FINISHED;
            }
            return hasMore;
        }

        // 标记为已访问
        this->m_visited.insert(this->m_currentNode);
        this->m_visitedOrder.push_back(this->m_currentNode);
        
        if (this->m_onNodeVisited) {
            this->m_onNodeVisited(this->m_currentNode);
        }

        // 获取邻居节点并排序（确保确定性）
        auto neighbors = this->m_graph->getNeighbors(this->m_currentNode);
        std::sort(neighbors.begin(), neighbors.end());  // ← 先按ID升序
        std::reverse(neighbors.begin(), neighbors.end());  // ← 再逆序（栈是LIFO）
        for (int neighbor : neighbors) {
            if (!this->m_visited.count(neighbor)) {
                this->m_frontier.push_back(neighbor);
                if (this->m_onEdgeProcessed) {
                    this->m_onEdgeProcessed(this->m_currentNode, neighbor);
                }
            }
        }

        // 检查是否还有更多节点
        bool hasMore = !this->m_frontier.empty();
        if (!hasMore) {
            this->m_state = State::FINISHED;
        }
        return hasMore;
    }
};
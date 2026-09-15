#pragma once

#include "Graph.h"
#include <unordered_set>
#include <queue>
#include <functional>
#include <QMessageBox>

/**
 * @brief 树类 - 继承自图，确保无环
 * @tparam DataType 节点数据类型
 */
template<typename DataType = std::string>
class Tree : public Graph<DataType, int, false> {
public:
    using BaseGraph = Graph<DataType, int, false>;
    using ErrorCallback = std::function<void(const QString&)>;
    
    Tree() {
        // 设置默认错误回调
        m_errorCallback = [](const QString& msg) {
            QMessageBox::warning(nullptr, "树结构错误", msg);
        };
    }
    
    void setErrorCallback(ErrorCallback cb) { m_errorCallback = cb; }

    /**
     * @brief 添加边（树专用）- 自动检查环和多父节点
     * @param parent 父节点ID
     * @param child 子节点ID
     * @param weight 边权重
     * @return 成功返回true，失败返回false并触发错误回调
     */
    bool addEdge(int parent, int child, int weight = 1) {
        // 检查子节点是否已存在父节点（树中每个节点只能有一个父节点）
        for (const auto& edge : this->getEdges()) {
            if (edge->to() == child) {
                if (m_errorCallback) {
                    m_errorCallback(QString("节点 %1 已存在父节点，树的节点只能有一个父节点").arg(child));
                }
                return false;
            }
        }

        // 检查是否会形成环（从child到parent是否存在路径）
        if (hasPath(child, parent)) {
            if (m_errorCallback) {
                m_errorCallback(QString("添加边 %1→%2 会形成环，这不是合法的树结构").arg(parent).arg(child));
            }
            return false;
        }

        // 调用基类方法实际添加边
        return BaseGraph::addEdge(parent, child, weight);
    }

private:
    /**
     * @brief 检查从from到to是否存在路径（用于环检测）
     */
    bool hasPath(int from, int to) {
        if (from == to) return true;
        std::queue<int> q;
        q.push(from);
        std::unordered_set<int> visited;
        while (!q.empty()) {
            int curr = q.front(); q.pop();
            if (curr == to) return true;
            visited.insert(curr);
            for (int neighbor : this->getNeighbors(curr)) {
                if (!visited.count(neighbor)) q.push(neighbor);
            }
        }
        return false;
    }

    ErrorCallback m_errorCallback; // 错误回调函数
};
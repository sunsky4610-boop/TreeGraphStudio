#include "GraphCanvas.h"
#include "../core/Graph.h"
#include "../core/Tree.h"
#include "../algorithms/Traversal.h"
#include "../algorithms/ShortestPath.h"
#include "../algorithms/MST.h"
#include "../utils/JsonSerializer.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QToolTip>
#include <cmath>
#include <functional>
#include <queue>

// 析构函数
GraphCanvas::AlgorithmWrapper::~AlgorithmWrapper() {
    if (algoPtr) {
        switch (type) {
            case BFS_T:
                if (directed) delete static_cast<::BFS<Graph<std::string, int, true>>*>(algoPtr);  // 有向与无向是不同类型，按 directed 分别释放
                else          delete static_cast<::BFS<Graph<std::string, int, false>>*>(algoPtr);
                break;
            case DFS_T:
                if (directed) delete static_cast<::DFS<Graph<std::string, int, true>>*>(algoPtr);
                else          delete static_cast<::DFS<Graph<std::string, int, false>>*>(algoPtr);
                break;
            case DIJKSTRA_T:
                if (directed) delete static_cast<::Dijkstra<Graph<std::string, int, true>>*>(algoPtr);
                else          delete static_cast<::Dijkstra<Graph<std::string, int, false>>*>(algoPtr);
                break;
            case KRUSKAL_T:
                // Kruskal 仅支持无向图/树，不存在有向实例
                delete static_cast<::Kruskal<Graph<std::string, int, false>>*>(algoPtr);
                break;
        }
        algoPtr = nullptr;
    }
}

GraphCanvas::~GraphCanvas() {
    deleteAlgorithm();
    cleanupGraphData();
}

void GraphCanvas::deleteAlgorithm() {
    if (m_algorithm) {
        delete static_cast<AlgorithmWrapper*>(m_algorithm);
        m_algorithm = nullptr;
    }
}

// ============================================
// 构造函数

GraphCanvas::GraphCanvas(QWidget* parent)
    : QGraphicsView(parent),
      m_scene(new QGraphicsScene(this)),
      m_animationTimer(new QTimer(this)) {

    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    // 整屏刷新视口：默认 MinimalViewportUpdate 在 Y 轴翻转 + 子图元变换下算不全重绘区域，拖动节点会留残影
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCursor(Qt::ArrowCursor);

    scale(1, -1);

    connect(m_animationTimer, &QTimer::timeout, this, &GraphCanvas::updateAnimation);

    setGraphType(UNDIRECTED_GRAPH);
    loadExample();
}

// ============================================
// 图类型设置

void GraphCanvas::setGraphType(GraphType type) {
    m_graphType = type;
    cleanupGraphData();

    switch (type) {
        case TREE:
            m_graphData = new Tree<std::string>();
            static_cast<Tree<std::string>*>(m_graphData)->setErrorCallback(
                [this](const QString& msg) {
                    QMessageBox::warning(this, "树结构错误", msg);
                });
            break;
        case DIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, true>();
            break;
        case UNDIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, false>();
            break;
    }

    clear();
}

void GraphCanvas::cleanupGraphData() {
    if (m_graphData) {
        switch (m_graphType) {
            case TREE:
                delete static_cast<Tree<std::string>*>(m_graphData);
                break;
            case DIRECTED_GRAPH:
                delete static_cast<Graph<std::string, int, true>*>(m_graphData);
                break;
            case UNDIRECTED_GRAPH:
                delete static_cast<Graph<std::string, int, false>*>(m_graphData);
                break;
        }
        m_graphData = nullptr;
    }
}

// 画布管理

void GraphCanvas::clear() {
    // 先停止正在运行的算法
    if (m_algorithmState == RUNNING) {
        m_animationTimer->stop();
        m_algorithmState = IDLE;
        deleteAlgorithm();
        emit algorithmStateChanged(IDLE);
    }

    // 清除场景
    m_scene->clear();

    // 清除所有UI状态
    m_nodeItems.clear();
    m_edgeItems.clear();
    m_nodeNames.clear();
    m_nameToId.clear();
    m_nextNodeId = 0;
    m_highlightedNodes.clear();
    m_highlightedEdges.clear();
    m_rejectedEdges.clear();
    m_edgeStartNode = -1;
    m_hoveredNode = -1;
    m_hoveredEdge = -1;
    m_isDraggingNode = false;
    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_startNode = -1;
    m_targetNode = -1;
    m_selectedNode = -1;
    clearStepHistory();
    m_algorithmState = IDLE;

    // 删除并重新创建图数据结构
    cleanupGraphData();
    switch (m_graphType) {
        case TREE:
            m_graphData = new Tree<std::string>();
            static_cast<Tree<std::string>*>(m_graphData)->setErrorCallback(
                [this](const QString& msg) {
                    QMessageBox::warning(this, "树结构错误", msg);
                });
            break;
        case DIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, true>();
            break;
        case UNDIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, false>();
            break;
    }

    // 重置坐标轴
    m_xAxis = nullptr;
    m_yAxis = nullptr;
    m_axisLabels.clear();

    emit graphModified();

    if (m_showAxes) {
        updateAxes();
    }
}

void GraphCanvas::loadExample() {
    clear();

    // 系统自动分配
    createNode(100, 100, "A");
    createNode(200, 100, "B");
    createNode(150, 200, "C");
    createNode(300, 150, "D");

    // 需要获取实际创建的ID来创建边
    // 修改为按名称查找ID
    int idA = getNodeIdByName("A");
    int idB = getNodeIdByName("B");
    int idC = getNodeIdByName("C");
    int idD = getNodeIdByName("D");

    if (idA != -1 && idB != -1) createEdge(idA, idB, 3);
    if (idA != -1 && idC != -1) createEdge(idA, idC, 2);
    if (idB != -1 && idD != -1) createEdge(idB, idD, 4);

    updateNodeColors();
    updateEdgeColors();
    updateAxes();

}

// ============================================
// 图数据操作

int GraphCanvas::addNodeToGraph(const std::string& data) {
    int newId = -1;
    switch (m_graphType) {
        case TREE:
            newId = static_cast<Tree<std::string>*>(m_graphData)->addNode(data);
            break;
        case DIRECTED_GRAPH:
            newId = static_cast<Graph<std::string, int, true>*>(m_graphData)->addNode(data);
            break;
        case UNDIRECTED_GRAPH:
            newId = static_cast<Graph<std::string, int, false>*>(m_graphData)->addNode(data);
            break;
    }
    return newId;
}

bool GraphCanvas::addEdgeToGraph(int from, int to, int weight) {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->addEdge(from, to, weight);
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->addEdge(from, to, weight);
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->addEdge(from, to, weight);
    }
    return false;
}

void GraphCanvas::removeEdgeFromGraph(int from, int to) {
    switch (m_graphType) {
        case TREE:
            static_cast<Tree<std::string>*>(m_graphData)->removeEdge(from, to);
            break;
        case DIRECTED_GRAPH:
            static_cast<Graph<std::string, int, true>*>(m_graphData)->removeEdge(from, to);
            break;
        case UNDIRECTED_GRAPH:
            static_cast<Graph<std::string, int, false>*>(m_graphData)->removeEdge(from, to);
            break;
    }
}

void GraphCanvas::removeNodeFromGraph(int nodeId) {
    switch (m_graphType) {
        case TREE:
            static_cast<Tree<std::string>*>(m_graphData)->removeNode(nodeId);
            break;
        case DIRECTED_GRAPH:
            static_cast<Graph<std::string, int, true>*>(m_graphData)->removeNode(nodeId);
            break;
        case UNDIRECTED_GRAPH:
            static_cast<Graph<std::string, int, false>*>(m_graphData)->removeNode(nodeId);
            break;
    }
}

std::vector<int> GraphCanvas::getGraphNeighbors(int nodeId) const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->getNeighbors(nodeId);
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->getNeighbors(nodeId);
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->getNeighbors(nodeId);
    }
    return {};
}

void* GraphCanvas::getNodeObject(int nodeId) const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->getNode(nodeId);
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->getNode(nodeId);
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->getNode(nodeId);
    }
    return nullptr;
}

void GraphCanvas::setNodePosition(int nodeId, double x, double y) {
    if (auto* node = static_cast<Node<std::string>*>(getNodeObject(nodeId))) {
        node->setPosition(x, y);
    }
}

std::pair<double, double> GraphCanvas::getNodePosition(int nodeId) const {
    if (auto* node = static_cast<Node<std::string>*>(getNodeObject(nodeId))) {
        return {node->x(), node->y()};
    }
    return {0, 0};
}

std::string GraphCanvas::getNodeData(int nodeId) const {
    if (auto* node = static_cast<Node<std::string>*>(getNodeObject(nodeId))) {
        return node->data();
    }
    return "";
}

void GraphCanvas::setNodeData(int nodeId, const std::string& data) {
    if (auto* node = static_cast<Node<std::string>*>(getNodeObject(nodeId))) {
        node->setData(data);
    }
}

std::vector<std::tuple<int, int, int>> GraphCanvas::getAllEdges() const {
    std::vector<std::tuple<int, int, int>> edges;

    switch (m_graphType) {
        case TREE: {
            auto treeEdges = static_cast<Tree<std::string>*>(m_graphData)->getEdges();
            for (const auto& edge : treeEdges) {
                edges.emplace_back(edge->from(), edge->to(), edge->weight());
            }
            break;
        }
        case DIRECTED_GRAPH: {
            auto graphEdges = static_cast<Graph<std::string, int, true>*>(m_graphData)->getEdges();
            for (const auto& edge : graphEdges) {
                edges.emplace_back(edge->from(), edge->to(), edge->weight());
            }
            break;
        }
        case UNDIRECTED_GRAPH: {
            auto graphEdges = static_cast<Graph<std::string, int, false>*>(m_graphData)->getEdges();
            for (const auto& edge : graphEdges) {
                edges.emplace_back(edge->from(), edge->to(), edge->weight());
            }
            break;
        }
    }

    return edges;
}

bool GraphCanvas::isDirectedGraph() const {
    switch (m_graphType) {
        case TREE:
            return false;
        case DIRECTED_GRAPH:
            return true;
        case UNDIRECTED_GRAPH:
            return false;
    }
    return false;
}

int GraphCanvas::getNodeCount() const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->nodeCount();
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->nodeCount();
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->nodeCount();
    }
    return 0;
}

int GraphCanvas::getEdgeCount() const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->edgeCount();
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->edgeCount();
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->edgeCount();
    }
    return 0;
}

bool GraphCanvas::hasNode(int nodeId) const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->hasNode(nodeId);
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->hasNode(nodeId);
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->hasNode(nodeId);
    }
    return false;
}

bool GraphCanvas::hasEdge(int from, int to) const {
    switch (m_graphType) {
        case TREE:
            return static_cast<Tree<std::string>*>(m_graphData)->hasEdge(from, to);
        case DIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, true>*>(m_graphData)->hasEdge(from, to);
        case UNDIRECTED_GRAPH:
            return static_cast<Graph<std::string, int, false>*>(m_graphData)->hasEdge(from, to);
    }
    return false;
}

const Graph<std::string, int, false>* GraphCanvas::getCurrentGraph() const {
    if (m_graphType == TREE) {
        return static_cast<Tree<std::string>*>(m_graphData);
    } else if (m_graphType == UNDIRECTED_GRAPH) {
        return static_cast<Graph<std::string, int, false>*>(m_graphData);
    }
    return nullptr;
}

// ============================================
// 节点/边创建

void GraphCanvas::createNode(double x, double y, const std::string& label) {
    //先添加到图数据结构，获取真实ID
    std::string displayLabel = label.empty() ? "Node" + std::to_string(m_nextNodeId) : label;
    int actualId = addNodeToGraph(displayLabel);

    if (actualId == -1) {
        QMessageBox::warning(this, "错误", "节点创建失败！");
        return;
    }

    NodeItem item;
    item.id = actualId;
    item.originalPos = QPointF(x, y);

    QPen nodePen(Qt::black, 2);
    nodePen.setJoinStyle(Qt::RoundJoin);
    item.shape = m_scene->addEllipse(-25, -25, 50, 50, nodePen, QBrush(Qt::lightGray));
    item.shape->setPos(x, y);
    item.shape->setZValue(10);

    item.label = new QGraphicsSimpleTextItem(QString::fromStdString(displayLabel));
    item.label->setPos(-item.label->boundingRect().width()/2, -35);
    item.label->setBrush(Qt::black);
    item.label->setFont(QFont("Arial", 10, QFont::Bold));
    item.label->setZValue(20);
    item.label->setParentItem(item.shape);
    item.label->setTransform(QTransform::fromScale(1, -1));

    item.coordinateLabel = new QGraphicsSimpleTextItem(QString("(%1, %2)").arg(x).arg(y));
    item.coordinateLabel->setPos(-item.coordinateLabel->boundingRect().width()/2, 30);
    item.coordinateLabel->setBrush(Qt::darkGray);
    item.coordinateLabel->setFont(QFont("Arial", 8));
    item.coordinateLabel->setVisible(m_showCoordinates);
    item.coordinateLabel->setZValue(20);
    item.coordinateLabel->setParentItem(item.shape);
    item.coordinateLabel->setTransform(QTransform::fromScale(1, -1));

    m_nodeItems[actualId] = item;

    setNodePosition(actualId, x, y);
    setNodeData(actualId, displayLabel);

    // 保存节点名称映射
    m_nodeNames[actualId] = QString::fromStdString(displayLabel);
    m_nameToId[m_nodeNames[actualId]] = actualId;

    m_nextNodeId++;
    emit graphModified();
}

void GraphCanvas::createEdge(int from, int to, int weight) {
    if (!hasNode(from) || !hasNode(to)) {
        QMessageBox::warning(this, "错误", "起点或终点节点不存在！");
        return;
    }

    if (!addEdgeToGraph(from, to, weight)) {
        return;
    }

    auto item = std::make_unique<EdgeItem>();
    item->from = from;
    item->to = to;
    item->isHovered = false;

    auto [fromX, fromY] = getNodePosition(from);
    auto [toX, toY] = getNodePosition(to);

    QPointF fromEdge = getNodeEdgePoint(from, QPointF(toX, toY));
    QPointF toEdge = getNodeEdgePoint(to, QPointF(fromX, fromY));

    QPen edgePen(Qt::black, 3);
    edgePen.setCapStyle(Qt::RoundCap);
    item->line = m_scene->addLine(
        fromEdge.x(), fromEdge.y(),
        toEdge.x(), toEdge.y(),
        edgePen
    );
    item->line->setZValue(5);

    if (m_graphType == DIRECTED_GRAPH) {
        drawArrow(item.get(), fromEdge.x(), fromEdge.y(), toEdge.x(), toEdge.y());
    }

    auto midX = (fromEdge.x() + toEdge.x()) / 2;
    auto midY = (fromEdge.y() + toEdge.y()) / 2;
    item->weight = new QGraphicsSimpleTextItem(QString::number(weight));
    item->weight->setPos(midX, midY);

    // 修复权值文字的变换
    QTransform transform;
    transform.translate(-item->weight->boundingRect().width()/2,
                       -item->weight->boundingRect().height()/2);
    transform.scale(1, -1);  // 反转Y轴，使文字正立
    item->weight->setTransform(transform);

    item->weight->setBrush(Qt::darkRed);
    item->weight->setFont(QFont("Arial", 9, QFont::Bold));
    item->weight->setVisible(m_showWeights);
    item->weight->setZValue(15);
    m_scene->addItem(item->weight);

    m_edgeItems.push_back(std::move(item));
    emit graphModified();
}

QPointF GraphCanvas::getNodeEdgePoint(int nodeId, const QPointF& target) const {
    if (!m_nodeItems.count(nodeId)) return target;

    auto [nodeX, nodeY] = getNodePosition(nodeId);
    double dx = target.x() - nodeX;
    double dy = target.y() - nodeY;
    double distance = std::sqrt(dx*dx + dy*dy);

    if (distance == 0) return QPointF(nodeX, nodeY);

    double scale = 25.0 / distance;
    return QPointF(nodeX + dx * scale, nodeY + dy * scale);
}

void GraphCanvas::drawArrow(EdgeItem* edgeItem, double fromX, double fromY, double toX, double toY) {
    double angle = std::atan2(toY - fromY, toX - fromX);
    double arrowTipX = toX;
    double arrowTipY = toY;
    double arrowLength = 15;
    double arrowAngle = M_PI / 6;

    QPointF arrowP1(arrowTipX, arrowTipY);
    QPointF arrowP2(arrowTipX - arrowLength * cos(angle - arrowAngle),
                    arrowTipY - arrowLength * sin(angle - arrowAngle));
    QPointF arrowP3(arrowTipX - arrowLength * cos(angle + arrowAngle),
                    arrowTipY - arrowLength * sin(angle + arrowAngle));

    QPolygonF arrowHead;
    arrowHead << arrowP1 << arrowP2 << arrowP3;

    QPen arrowPen(Qt::black, 2);
    arrowPen.setJoinStyle(Qt::RoundJoin);
    edgeItem->arrow = m_scene->addPolygon(arrowHead, arrowPen, QBrush(Qt::black));
    edgeItem->arrow->setZValue(6);
}

// ============================================
// 模式设置

void GraphCanvas::setMode(Mode mode) {
    m_mode = mode;
    m_edgeStartNode = -1;
    m_hoveredNode = -1;
    m_hoveredEdge = -1;

    for (auto& [id, item] : m_nodeItems) {
        if (item.shape) {
            item.shape->setBrush(QBrush(Qt::lightGray));
        }
    }

    for (auto& edgeItem : m_edgeItems) {
        if (edgeItem->line) {
            QPen pen(Qt::black, 3);
            pen.setCapStyle(Qt::RoundCap);
            edgeItem->line->setPen(pen);
        }
    }

    switch(m_mode) {
        case SELECT:
            setCursor(Qt::ArrowCursor);
            break;
        case ADD_NODE:
            setCursor(Qt::CrossCursor);
            break;
        case ADD_EDGE:
            setCursor(Qt::PointingHandCursor);
            break;
    }

    updateNodeColors();
    updateEdgeColors();
}

void GraphCanvas::setShowWeights(bool show) {
    m_showWeights = show;
    updateAllEdges();
}

void GraphCanvas::setShowCoordinates(bool show) {
    m_showCoordinates = show;
    updateNodeLabels();
}

void GraphCanvas::setShowAxes(bool show) {
    m_showAxes = show;
    updateAxes();
}

void GraphCanvas::updateAllEdges() {
    for (auto& edgeItem : m_edgeItems) {
        if (edgeItem->weight) {
            edgeItem->weight->setVisible(m_showWeights);
        }
    }
}

void GraphCanvas::updateNodeColors() {
    for (auto& [id, item] : m_nodeItems) {
        if (!item.shape) continue;
        if (m_highlightedNodes.count(id) && !item.isDragging) {
            item.shape->setBrush(QBrush(QColor(255, 255, 0, 200)));
        } else if (m_edgeStartNode == id) {
            item.shape->setBrush(QBrush(Qt::green));
        } else if (m_hoveredNode == id && m_mode == SELECT && !item.isDragging) {
            item.shape->setBrush(QBrush(QColor(0, 255, 255, 200)));
        } else if (item.isDragging) {
            item.shape->setBrush(QBrush(QColor(255, 165, 0, 200)));
        } else {
            item.shape->setBrush(QBrush(QColor(211, 211, 211, 200)));
        }
    }
}

void GraphCanvas::updateEdgeColors() {
    for (size_t i = 0; i < m_edgeItems.size(); ++i) {
        if (!m_edgeItems[i] || !m_edgeItems[i]->line) continue;

        QPen pen;
        int from = m_edgeItems[i]->from;
        int to = m_edgeItems[i]->to;

        bool isHighlighted = false;
        bool isRejected = false;

        // 1. 检查是否被拒边（最高优先级）
        for (const auto& rejectedEdge : m_rejectedEdges) {
            if ((rejectedEdge.first == from && rejectedEdge.second == to) ||
                (!isDirectedGraph() && rejectedEdge.first == to && rejectedEdge.second == from)) {
                isRejected = true;
                break;
            }
        }

        // 2. 检查是否悬停
        if (m_hoveredEdge == static_cast<int>(i) && m_mode == SELECT) {
            pen = QPen(QColor(0, 0, 255, 200), 4);  // 蓝色悬停
            isHighlighted = true;
        }
        // 3. 检查是否被选中（MST边）
        else if (!isRejected) {
            for (const auto& highlightedEdge : m_highlightedEdges) {
                if ((highlightedEdge.first == from && highlightedEdge.second == to) ||
                    (!isDirectedGraph() && highlightedEdge.first == to && highlightedEdge.second == from)) {
                    isHighlighted = true;
                    break;
                }
            }

            if (isHighlighted) {
                pen = QPen(QColor(255, 255, 0, 200), 3);  // 黄色选中
            } else {
                pen = QPen(QColor(0, 0, 0, 150), 3);      // 默认黑色
            }
        }

        // 被拒边样式（红色虚线）
        if (isRejected) {
            pen = QPen(QColor(255, 0, 0, 150), 3, Qt::DashLine);  // 红色虚线
        }

        pen.setCapStyle(Qt::RoundCap);
        m_edgeItems[i]->line->setPen(pen);

        // 更新箭头颜色
        if (m_edgeItems[i]->arrow) {
            if (isRejected) {
                m_edgeItems[i]->arrow->setBrush(QBrush(QColor(255, 0, 0, 150)));
            } else {
                m_edgeItems[i]->arrow->setBrush(isHighlighted ?
                    QBrush(QColor(255, 255, 0, 200)) : QBrush(Qt::black));
            }
        }
    }
}

void GraphCanvas::updateNodeLabels() {
    for (auto& [id, item] : m_nodeItems) {
        if (item.coordinateLabel) {
            item.coordinateLabel->setVisible(m_showCoordinates);
        }
    }
}

void GraphCanvas::updateAxes() {
    if (m_xAxis) {
        m_scene->removeItem(m_xAxis);
        delete m_xAxis;
        m_xAxis = nullptr;
    }
    if (m_yAxis) {
        m_scene->removeItem(m_yAxis);
        delete m_yAxis;
        m_yAxis = nullptr;
    }
    for (auto label : m_axisLabels) {
        m_scene->removeItem(label);
        delete label;
    }
    m_axisLabels.clear();

    if (!m_showAxes) return;

    QRectF sceneRect = m_scene->sceneRect();
    if (sceneRect.isNull()) {
        sceneRect = QRectF(-100, -100, 600, 600);
    }

    QPen axisPen(Qt::gray, 1, Qt::DashLine);
    m_xAxis = m_scene->addLine(sceneRect.left(), 0, sceneRect.right(), 0, axisPen);
    m_xAxis->setZValue(1);

    m_yAxis = m_scene->addLine(0, sceneRect.bottom(), 0, sceneRect.top(), axisPen);
    m_yAxis->setZValue(1);

    for (int x = -500; x <= 500; x += 50) {
        if (x != 0) {
            QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(x));
            label->setPos(x - 10, 5);
            label->setBrush(Qt::gray);
            label->setFont(QFont("Arial", 7));
            label->setZValue(1);
            m_scene->addItem(label);
            label->setTransform(QTransform::fromScale(1, -1));
            m_axisLabels.append(label);
        }
    }

    for (int y = -500; y <= 500; y += 50) {
        if (y != 0) {
            QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(QString::number(y));
            label->setPos(5, y - 7);
            label->setBrush(Qt::gray);
            label->setFont(QFont("Arial", 7));
            label->setZValue(1);
            m_scene->addItem(label);
            label->setTransform(QTransform::fromScale(1, -1));
            m_axisLabels.append(label);
        }
    }

    QGraphicsSimpleTextItem* originLabel = new QGraphicsSimpleTextItem("(0,0)");
    originLabel->setPos(5, 5);
    originLabel->setBrush(Qt::darkGray);
    originLabel->setFont(QFont("Arial", 8, QFont::Bold));
    originLabel->setZValue(1);
    m_scene->addItem(originLabel);
    originLabel->setTransform(QTransform::fromScale(1, -1));
    m_axisLabels.append(originLabel);
}

// ============================================
// 鼠标事件

int GraphCanvas::findNodeAt(const QPointF& pos) const {
    for (const auto& [id, item] : m_nodeItems) {
        if (item.shape && item.shape->contains(item.shape->mapFromScene(pos))) {
            return id;
        }
    }
    return -1;
}

int GraphCanvas::findEdgeAt(const QPointF& pos) const {
    for (size_t i = 0; i < m_edgeItems.size(); ++i) {
        if (!m_edgeItems[i] || !m_edgeItems[i]->line) continue;
        auto* line = m_edgeItems[i]->line;
        QPointF p1 = line->line().p1();
        QPointF p2 = line->line().p2();
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double length = std::sqrt(dx*dx + dy*dy);
        if (length > 0) {
            double t = std::max(0.0, std::min(1.0, ((pos.x() - p1.x()) * dx + (pos.y() - p1.y()) * dy) / (length * length)));
            double projX = p1.x() + t * dx;
            double projY = p1.y() + t * dy;
            double distance = std::sqrt((pos.x() - projX) * (pos.x() - projX) + (pos.y() - projY) * (pos.y() - projY));
            if (distance < 10) return i;
        }
    }
    return -1;
}

void GraphCanvas::mousePressEvent(QMouseEvent* event) {
    QGraphicsView::mousePressEvent(event);
    auto pos = mapToScene(event->pos());

    if (event->button() == Qt::LeftButton) {
        int nodeId = findNodeAt(pos);
        int edgeIndex = findEdgeAt(pos);

        switch(m_mode) {
            case ADD_NODE:
                if (nodeId == -1 && edgeIndex == -1) {
                    createNode(pos.x(), pos.y(), "N" + std::to_string(m_nextNodeId));
                }
                break;

            case ADD_EDGE:
                if (nodeId != -1) {
                    if (m_edgeStartNode == -1) {
                        m_edgeStartNode = nodeId;
                        if (m_nodeItems.count(m_edgeStartNode) && m_nodeItems[m_edgeStartNode].shape) {
                            m_nodeItems[m_edgeStartNode].shape->setBrush(QBrush(Qt::green));
                        }
                    } else if (m_edgeStartNode != nodeId) {
                        if (hasEdge(m_edgeStartNode, nodeId)) {
                            QMessageBox::information(this, "提示", "边已存在！");
                            if (m_nodeItems.count(m_edgeStartNode) && m_nodeItems[m_edgeStartNode].shape) {
                                m_nodeItems[m_edgeStartNode].shape->setBrush(QBrush(Qt::lightGray));
                            }
                            m_edgeStartNode = -1;
                            return;
                        }
                        bool ok;
                        int weight = QInputDialog::getInt(this, "边权重", "请输入权重:", 1, 1, 100, 1, &ok);
                        if (ok) {
                            createEdge(m_edgeStartNode, nodeId, weight);
                        }
                        if (m_nodeItems.count(m_edgeStartNode) && m_nodeItems[m_edgeStartNode].shape) {
                            m_nodeItems[m_edgeStartNode].shape->setBrush(QBrush(Qt::lightGray));
                        }
                        m_edgeStartNode = -1;
                    }
                }
                break;

            case SELECT:
                if (nodeId != -1) {
                    auto it = m_nodeItems.find(nodeId);
                    if (it != m_nodeItems.end() && it->second.shape) {
                        m_dragItem = it->second.shape;
                        m_dragOffset = pos - it->second.shape->scenePos();
                        it->second.isDragging = true;
                        m_isDraggingNode = true;
                        it->second.originalPos = it->second.shape->scenePos();
                        updateNodeColors();
                    }

                    setSelectedNode(nodeId);
                    emit nodeSelected(nodeId);
                } else {
                    setSelectedNode(-1);
                }
                break;
        }
    }
    // 添加右键点击处理
    else if (event->button() == Qt::RightButton) {
        int nodeId = findNodeAt(pos);
        int edgeIndex = findEdgeAt(pos);

        if (nodeId != -1) {
            showNodeContextMenu(nodeId, event->globalPos());
        } else if (edgeIndex != -1) {
            showEdgeContextMenu(edgeIndex, event->globalPos());
        }
    }
}

void GraphCanvas::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);
    auto pos = mapToScene(event->pos());
    int oldHoveredNode = m_hoveredNode;
    int oldHoveredEdge = m_hoveredEdge;
    m_hoveredNode = findNodeAt(pos);
    m_hoveredEdge = findEdgeAt(pos);

    if (m_hoveredNode != -1) {
        emit nodeHovered(m_hoveredNode, getNodeInfo(m_hoveredNode));
        QToolTip::showText(event->globalPos(), getNodeInfo(m_hoveredNode), this);
    } else if (m_hoveredEdge != -1) {
        emit edgeHovered(m_edgeItems[m_hoveredEdge]->from,
                        m_edgeItems[m_hoveredEdge]->to,
                        getEdgeInfo(m_hoveredEdge));
        QToolTip::showText(event->globalPos(), getEdgeInfo(m_hoveredEdge), this);
    } else {
        QToolTip::hideText();
    }

    if (oldHoveredNode != m_hoveredNode || oldHoveredEdge != m_hoveredEdge) {
        updateNodeColors();
        updateEdgeColors();
    }

    if (m_dragItem && event->buttons() & Qt::LeftButton && m_isDraggingNode) {
        for (auto& [nodeId, nodeItem] : m_nodeItems) {
            if (nodeItem.shape && nodeItem.shape == m_dragItem) {
                QPointF newPos = pos - m_dragOffset;
                nodeItem.shape->setPos(newPos);
                setNodePosition(nodeId, newPos.x(), newPos.y());

                if (nodeItem.coordinateLabel) {
                    nodeItem.coordinateLabel->setText(QString("(%1, %2)").arg(newPos.x()).arg(newPos.y()));
                }

                for (auto& edgeItem : m_edgeItems) {
                    if (!edgeItem || !edgeItem->line) continue;
                    bool fromThis = edgeItem->from == nodeId;
                    bool toThis = edgeItem->to == nodeId;
                    if (fromThis || toThis) {
                        int otherNodeId = fromThis ? edgeItem->to : edgeItem->from;
                        auto [otherX, otherY] = getNodePosition(otherNodeId);

                        QPointF thisEdge = getNodeEdgePoint(nodeId, QPointF(otherX, otherY));
                        QPointF otherEdge = getNodeEdgePoint(otherNodeId, QPointF(newPos.x(), newPos.y()));
                        edgeItem->line->setLine(thisEdge.x(), thisEdge.y(), otherEdge.x(), otherEdge.y());

                        if (m_graphType == DIRECTED_GRAPH && edgeItem->arrow) {
                            m_scene->removeItem(edgeItem->arrow);
                            delete edgeItem->arrow;  // 旧箭头已移出场景，需手动释放，否则拖动时每帧泄漏
                            edgeItem->arrow = nullptr;

                            // 【关键修复】确保箭头方向始终从 edgeItem->from 指向 edgeItem->to
                            QPointF fromPos, toPos;
                            if (nodeId == edgeItem->from) {
                                // 拖动的是起点节点
                                fromPos = thisEdge;
                                toPos = otherEdge;
                            } else {
                                // 拖动的是终点节点
                                fromPos = otherEdge;
                                toPos = thisEdge;
                            }
                            drawArrow(edgeItem.get(), fromPos.x(), fromPos.y(),
                                      toPos.x(), toPos.y());
                        }
                    }

                    if (edgeItem->weight) {
                        QLineF line = edgeItem->line->line();
                        QPointF midPoint = line.pointAt(0.5);

                        edgeItem->weight->setPos(midPoint);

                        // 修复权值文字的变换
                        QTransform transform;
                        transform.translate(-edgeItem->weight->boundingRect().width()/2,
                                           -edgeItem->weight->boundingRect().height()/2);
                        transform.scale(1, -1);  // 反转Y轴，使文字正立
                        edgeItem->weight->setTransform(transform);
                    }
                }
                break;
            }
        }
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent* event) {
    QGraphicsView::mouseReleaseEvent(event);
    if (m_isDraggingNode) {
        for (auto& [id, nodeItem] : m_nodeItems) {
            if (nodeItem.isDragging) {
                QPointF pos = nodeItem.shape->scenePos();
                setNodePosition(id, pos.x(), pos.y());
                if (nodeItem.coordinateLabel) {
                    nodeItem.coordinateLabel->setText(QString("(%1, %2)").arg(pos.x()).arg(pos.y()));
                }
                nodeItem.isDragging = false;
            }
        }
        m_isDraggingNode = false;
        updateNodeColors();
    }
    m_dragItem = nullptr;
}

void GraphCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    auto pos = mapToScene(event->pos());
    int edgeIndex = findEdgeAt(pos);
    if (edgeIndex != -1) {
        onWeightEditRequested(edgeIndex);
    }
}

void GraphCanvas::leaveEvent(QEvent* event) {
    QGraphicsView::leaveEvent(event);
    if (m_hoveredNode != -1 || m_hoveredEdge != -1) {
        m_hoveredNode = -1;
        m_hoveredEdge = -1;
        updateNodeColors();
        updateEdgeColors();
        QToolTip::hideText();
    }
}

void GraphCanvas::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (m_showAxes) {
        updateAxes();
    }
}

// ============================================
// 右键菜单

void GraphCanvas::showEdgeContextMenu(int edgeIndex, const QPoint& screenPos) {
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(m_edgeItems.size())) return;
    if (!m_edgeItems[edgeIndex]) return;
    QMenu menu;
    QAction* editWeightAction = menu.addAction("编辑权重");
    QAction* deleteEdgeAction = menu.addAction("删除边");
    QAction* selectedAction = menu.exec(screenPos);
    if (selectedAction == editWeightAction) {
        onWeightEditRequested(edgeIndex);
    } else if (selectedAction == deleteEdgeAction) {
        onEdgeDeleteRequested(edgeIndex);
    }
}

void GraphCanvas::showNodeContextMenu(int nodeId, const QPoint& screenPos) {
    if (!m_nodeItems.count(nodeId)) return;
    QMenu menu;
    QAction* renameAction = menu.addAction("重命名节点");
    QAction* deleteAction = menu.addAction("删除节点");
    QAction* selectedAction = menu.exec(screenPos);
    if (selectedAction == renameAction) {
        onNodeRenameRequested(nodeId);
    } else if (selectedAction == deleteAction) {
        onNodeDeleteRequested(nodeId);
    }
}

// ============================================
// 菜单响应

void GraphCanvas::onEdgeDeleteRequested(int edgeIndex) {
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(m_edgeItems.size())) return;
    if (!m_edgeItems[edgeIndex]) return;
    int from = m_edgeItems[edgeIndex]->from;
    int to = m_edgeItems[edgeIndex]->to;

    removeEdgeFromGraph(from, to);

    EdgeItem* edgeToDelete = m_edgeItems[edgeIndex].get();
    if (edgeToDelete->line) {
        m_scene->removeItem(edgeToDelete->line);
        delete edgeToDelete->line;
    }
    if (edgeToDelete->arrow) {
        m_scene->removeItem(edgeToDelete->arrow);
        delete edgeToDelete->arrow;
    }
    if (edgeToDelete->weight) {
        m_scene->removeItem(edgeToDelete->weight);
        delete edgeToDelete->weight;
    }
    m_edgeItems.erase(m_edgeItems.begin() + edgeIndex);
    emit graphModified();
}

void GraphCanvas::onNodeDeleteRequested(int nodeId) {
    deleteNodeAndEdges(nodeId);
    emit graphModified();
}

void GraphCanvas::onNodeRenameRequested(int nodeId) {
    if (!m_nodeItems.count(nodeId) || !m_nodeItems[nodeId].label) return;

    bool ok;
    QString currentName = m_nodeItems[nodeId].label->text();
    QString newName = QInputDialog::getText(this, "重命名节点",
                                          "请输入新节点名:",
                                          QLineEdit::Normal,
                                          currentName, &ok);

    if (ok && !newName.isEmpty()) {
        setNodeName(nodeId, newName);
        emit graphModified();
    }
}

void GraphCanvas::onWeightEditRequested(int edgeIndex) {
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(m_edgeItems.size())) return;
    if (!m_edgeItems[edgeIndex] || !m_edgeItems[edgeIndex]->weight) return;

    bool ok;
    int currentWeight = m_edgeItems[edgeIndex]->weight->text().toInt();
    int newWeight = QInputDialog::getInt(this, "编辑权重", "请输入新的权重:",
                                        currentWeight, 1, 100, 1, &ok);
    if (ok) {
        int from = m_edgeItems[edgeIndex]->from;
        int to = m_edgeItems[edgeIndex]->to;

        removeEdgeFromGraph(from, to);
        addEdgeToGraph(from, to, newWeight);

        m_edgeItems[edgeIndex]->weight->setText(QString::number(newWeight));
        emit graphModified();
    }
}

// ============================================
// 信息获取

QString GraphCanvas::getNodeInfo(int nodeId) const {
    if (!hasNode(nodeId)) return "";

    auto [x, y] = getNodePosition(nodeId);
    auto neighbors = getGraphNeighbors(nodeId);
    int degree = static_cast<int>(neighbors.size());
    std::string data = getNodeData(nodeId);

    return QString("节点 %1: %2\n"
                   "横坐标: %3\n"
                   "纵坐标: %4\n"
                   "度数: %5\n"
                   "邻近节点: %6")
            .arg(nodeId)
            .arg(QString::fromStdString(data))
            .arg(x, 0, 'f', 2)
            .arg(y, 0, 'f', 2)
            .arg(degree)
            .arg([&neighbors](){
                QStringList neighborStrs;
                for (int n : neighbors) {
                    neighborStrs << QString::number(n);
                }
                return neighborStrs.join(", ");
            }());
}

QString GraphCanvas::getEdgeInfo(int edgeIndex) const {
    if (edgeIndex < 0 || edgeIndex >= static_cast<int>(m_edgeItems.size()))
        return "";
    int from = m_edgeItems[edgeIndex]->from;
    int to = m_edgeItems[edgeIndex]->to;

    int weight = 0;
    auto edges = getAllEdges();
    for (const auto& [f, t, w] : edges) {
        if ((f == from && t == to) || (f == to && t == from)) {
            weight = w;
            break;
        }
    }

    auto [fromX, fromY] = getNodePosition(from);
    auto [toX, toY] = getNodePosition(to);

    QString typeStr;
    switch (m_graphType) {
        case TREE: typeStr = "树"; break;
        case DIRECTED_GRAPH: typeStr = "有向"; break;
        case UNDIRECTED_GRAPH: typeStr = "无向"; break;
    }

    return QString("边: %1 → %2\n"
                   "权重: %3\n"
                   "起点坐标: %4\n"
                   "终点坐标: %5\n"
                   "类型: %6")
            .arg(from)
            .arg(to)
            .arg(weight)
            .arg(fromX, 0, 'f', 1)
            .arg(fromY, 0, 'f', 1)
            .arg(toX, 0, 'f', 1)
            .arg(toY, 0, 'f', 1)
            .arg(typeStr);
}

void GraphCanvas::saveStepToHistory() {
    StepInfo step;
    step.visitedNodes = m_highlightedNodes;
    step.visitedEdges = m_highlightedEdges;
    step.description = QString("第 %1 步").arg(m_currentStep);
    m_stepHistory.push_back(step);
}

void GraphCanvas::restoreStepFromHistory(int stepIndex) {
    if (stepIndex >= 0 && stepIndex < static_cast<int>(m_stepHistory.size())) {
        const StepInfo& step = m_stepHistory[stepIndex];
        m_highlightedNodes = step.visitedNodes;
        m_highlightedEdges = step.visitedEdges;
        updateNodeColors();
        updateEdgeColors();
    }
}

void GraphCanvas::clearStepHistory() {
    m_stepHistory.clear();
}

QString GraphCanvas::getAlgorithmInfo() const {
    if (!m_algorithm) return "";
    QString info;
    info += QString("当前算法: %1\n").arg(m_currentAlgorithm);
    info += QString("执行步数: %1\n").arg(m_algorithmStepCount);
    info += QString("已访问节点: %1\n").arg(m_highlightedNodes.size());
    info += QString("已访问边: %1").arg(m_highlightedEdges.size());
    return info;
}

// 算法控制
void GraphCanvas::startAlgorithm(int startNode, AlgorithmType algoType) {
    // ==================== 算法支持性检查 ====================
    // 只有Kruskal不支持有向图
    if (algoType == MST && m_graphType == DIRECTED_GRAPH) {
        QMessageBox::warning(this, "错误",
            "最小生成树(Kruskal)算法不支持有向图！\n"
            "有向图请使用BFS、DFS或最短路径算法。");
        return;
    }

    // 检查起始节点（MST不需要）
    if (algoType != MST) {
        if (!hasNode(startNode)) {
            QMessageBox::warning(this, "错误", "起始节点不存在！");
            return;
        }
    }

    // 通用检查
    if (getNodeCount() == 0) {
        QMessageBox::warning(this, "错误", "图中没有节点！");
        return;
    }

    // 提示有向图特性
    if (m_graphType == DIRECTED_GRAPH && algoType != MST) {
        QMessageBox::information(this, "提示",
            "你正在有向图上运行算法。\n"
            "遍历将遵循边的方向进行。");
    }

    // 【修复】在算法开始前清空历史记录（在类内部调用）
    resetAlgorithm();       // 这会清除大部分状态
    clearStepHistory();     // 清空步骤历史
    m_currentStep = 0;      // 重置当前步骤

    m_startNode = startNode;

    QString algoName;

    // 创建算法包装器
    auto* wrapper = new AlgorithmWrapper();
    m_algorithm = wrapper;
    m_currentAlgorithmType = algoType;

    // 根据图类型创建算法实例
    if (m_graphType == DIRECTED_GRAPH) {
        // 有向图算法
        wrapper->directed = true;  // 有向图实例，后续强转必须用 Graph<...,true>
        auto* graph = static_cast<Graph<std::string, int, true>*>(m_graphData);

        switch(algoType) {
            case BFS: {
                wrapper->type = AlgorithmWrapper::BFS_T;
                auto* bfs = new ::BFS<Graph<std::string, int, true>>(graph);
                bfs->initialize(startNode);
                bfs->setOnNodeVisited([this](int id) { onNodeVisited(id); });
                wrapper->algoPtr = bfs;
                algoName = "BFS遍历(有向)";
                break;
            }
            case DFS: {
                wrapper->type = AlgorithmWrapper::DFS_T;
                auto* dfs = new ::DFS<Graph<std::string, int, true>>(graph);
                dfs->initialize(startNode);
                dfs->setOnNodeVisited([this](int id) { onNodeVisited(id); });
                wrapper->algoPtr = dfs;
                algoName = "DFS遍历(有向)";
                break;
            }
            case SHORTEST_PATH: {
                // 最短路径需要特殊处理，使用专门的 startShortestPath 方法
                delete wrapper;
                m_algorithm = nullptr;
                resetAlgorithm();
                return;
            }
            case MST: {
                // 不支持的组合，前面已检查
                delete wrapper;
                m_algorithm = nullptr;
                return;
            }
        }
    } else {
        // 无向图/树算法
        wrapper->directed = false;  // 无向图/树实例，后续强转用 Graph<...,false>
        Graph<std::string, int, false>* graphBase = nullptr;
        if (m_graphType == TREE) {
            graphBase = static_cast<Tree<std::string>*>(m_graphData);
        } else {
            graphBase = static_cast<Graph<std::string, int, false>*>(m_graphData);
        }

        if (!graphBase) {
            QMessageBox::warning(this, "错误", "无法获取图对象！");
            delete wrapper;
            m_algorithm = nullptr;
            return;
        }

        switch(algoType) {
            case BFS: {
                wrapper->type = AlgorithmWrapper::BFS_T;
                auto* bfs = new ::BFS<Graph<std::string, int, false>>(graphBase);
                bfs->initialize(startNode);
                bfs->setOnNodeVisited([this](int id) { onNodeVisited(id); });
                wrapper->algoPtr = bfs;
                algoName = "BFS遍历";
                break;
            }
            case DFS: {
                wrapper->type = AlgorithmWrapper::DFS_T;
                auto* dfs = new ::DFS<Graph<std::string, int, false>>(graphBase);
                dfs->initialize(startNode);
                dfs->setOnNodeVisited([this](int id) { onNodeVisited(id); });
                wrapper->algoPtr = dfs;
                algoName = "DFS遍历";
                break;
            }
            case SHORTEST_PATH: {
                delete wrapper;
                m_algorithm = nullptr;
                resetAlgorithm();
                return;
            }
            case MST: {
                wrapper->type = AlgorithmWrapper::KRUSKAL_T;
                auto* kruskal = new Kruskal<Graph<std::string, int, false>>(graphBase);
                kruskal->initialize();

                kruskal->setOnEdgeSelected([this, kruskal](int from, int to, int weight) {
                    m_highlightedEdges.push_back({from, to});
                    m_highlightedNodes.insert(from);
                    m_highlightedNodes.insert(to);

                    updateNodeColors();
                    updateEdgeColors();

                    emit algorithmStatusUpdated(QString("MST构建中: 已选择 %1 条边，总权重 %2")
                                            .arg(m_highlightedEdges.size())
                                            .arg(kruskal->getTotalWeight()));
                });

                kruskal->setOnEdgeRejected([this](int from, int to, int weight) {
                    m_rejectedEdges.push_back({from, to});
                    updateEdgeColors();

                    QTimer::singleShot(1000, [this, from, to]() {
                        m_rejectedEdges.erase(
                            std::remove_if(m_rejectedEdges.begin(), m_rejectedEdges.end(),
                                [from, to](const auto& p) {
                                    return p.first == from && p.second == to;
                                }),
                            m_rejectedEdges.end()
                        );
                        updateEdgeColors();
                    });

                    emit algorithmProgressUpdated(QString("【MST】拒绝边 %1-%2（权重%3，会形成环）")
                                                .arg(getNodeName(from))
                                                .arg(getNodeName(to))
                                                .arg(weight));
                });

                kruskal->setOnStepProgress([this](int current, int total) {
                    emit algorithmProgressUpdated(QString("【MST】检查进度: %1/%2 条边").arg(current).arg(total));
                });

                wrapper->algoPtr = kruskal;
                algoName = "最小生成树";
                break;
            }
        }
    }

    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_currentAlgorithm = algoName;
    m_algorithmState = RUNNING;
    clearStepHistory();

    StepInfo initialStep;
    initialStep.visitedNodes = {};
    initialStep.visitedEdges = {};
    initialStep.description = "初始状态";
    m_stepHistory.push_back(initialStep);

    emit algorithmStarted(algoName, startNode);
    emit algorithmStateChanged(RUNNING);

    int interval = 1100 - m_animationSpeed * 100;
    m_animationTimer->start(interval);
}

void GraphCanvas::startShortestPath(int startNode, int targetNode) {
    if (!hasNode(startNode) || !hasNode(targetNode)) {
        QMessageBox::warning(this, "错误", "起点或终点节点不存在！");
        return;
    }

    if (getNodeCount() == 0) {
        QMessageBox::warning(this, "错误", "图中没有节点！");
        return;
    }

    if (m_graphType == DIRECTED_GRAPH) {
        QMessageBox::information(this, "提示",
            "你正在有向图上运行最短路径算法。\n"
            "算法将遵循边的方向计算路径。");
    }

    // 【修复】在算法开始前清空历史记录（在类内部调用）
    resetAlgorithm();
    clearStepHistory();
    m_currentStep = 0;

    m_startNode = startNode;
    m_targetNode = targetNode;

    auto* wrapper = new AlgorithmWrapper();
    m_algorithm = wrapper;
    m_currentAlgorithmType = SHORTEST_PATH;

    if (m_graphType == DIRECTED_GRAPH) {
        // 有向图最短路径
        wrapper->directed = true;  // 有向图实例，后续强转必须用 Graph<...,true>
        auto* graph = static_cast<Graph<std::string, int, true>*>(m_graphData);
        wrapper->type = AlgorithmWrapper::DIJKSTRA_T;
        auto* dijkstra = new Dijkstra<Graph<std::string, int, true>>(graph);
        dijkstra->initialize(startNode);

        dijkstra->setOnEdgeRelaxed([this, dijkstra](int from, int to, int distance) {
            auto path = dijkstra->getPath(to);
            m_highlightedEdges.clear();
            for (size_t i = 0; i + 1 < path.size(); ++i) {
                m_highlightedEdges.push_back({path[i], path[i+1]});
            }
            updateEdgeColors();
            m_highlightedNodes.insert(to);
            updateNodeColors();
        });

        dijkstra->setOnNodeVisited([this](int id) {
            m_highlightedNodes.insert(id);
            updateNodeColors();
        });

        wrapper->algoPtr = dijkstra;
    } else {
        // 无向图最短路径
        wrapper->directed = false;  // 无向图/树实例，后续强转用 Graph<...,false>
        Graph<std::string, int, false>* graphBase = nullptr;
        if (m_graphType == TREE) {
            graphBase = static_cast<Tree<std::string>*>(m_graphData);
        } else {
            graphBase = static_cast<Graph<std::string, int, false>*>(m_graphData);
        }

        wrapper->type = AlgorithmWrapper::DIJKSTRA_T;
        auto* dijkstra = new Dijkstra<Graph<std::string, int, false>>(graphBase);
        dijkstra->initialize(startNode);

        dijkstra->setOnEdgeRelaxed([this, dijkstra](int from, int to, int distance) {
            auto path = dijkstra->getPath(to);
            m_highlightedEdges.clear();
            for (size_t i = 0; i + 1 < path.size(); ++i) {
                m_highlightedEdges.push_back({path[i], path[i+1]});
            }
            updateEdgeColors();
            m_highlightedNodes.insert(to);
            updateNodeColors();
        });

        dijkstra->setOnNodeVisited([this](int id) {
            m_highlightedNodes.insert(id);
            updateNodeColors();
        });

        wrapper->algoPtr = dijkstra;
    }

    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_currentAlgorithm = QString("最短路径 (%1 → %2)")
        .arg(getNodeName(startNode))
        .arg(getNodeName(targetNode));
    m_algorithmState = RUNNING;
    clearStepHistory();

    // 创建初始步骤（在清空历史之后）
    StepInfo initialStep;
    initialStep.visitedNodes = {startNode};
    initialStep.visitedEdges = {};
    initialStep.description = QString("初始状态：从 %1 到 %2")
        .arg(getNodeName(startNode))
        .arg(getNodeName(targetNode));
    m_stepHistory.push_back(initialStep);

    m_highlightedNodes.insert(startNode);
    updateNodeColors();

    emit algorithmStarted(m_currentAlgorithm, startNode);
    emit algorithmStateChanged(RUNNING);

    int interval = 1100 - m_animationSpeed * 100;
    m_animationTimer->start(interval);
}

void GraphCanvas::stepAlgorithm() {
    if (!m_algorithm || m_algorithmState != RUNNING) return;

    auto* wrapper = static_cast<AlgorithmWrapper*>(m_algorithm);
    bool hasMoreSteps = false;
    QString stepInfo;
    bool nodeProcessed = false;  // 标记是否处理了节点

    // 记录步骤前的访问集合大小
    size_t nodesBeforeStep = m_highlightedNodes.size();

    switch(wrapper->type) {
        case AlgorithmWrapper::BFS_T: {
            // 有向图/无向图是不同的模板实例，按创建时记录的 directed 选择匹配类型
            auto process = [&](auto* bfs) {
                size_t nodesBefore = bfs->getVisitedNodes().size();
                hasMoreSteps = bfs->step();
                size_t nodesAfter = bfs->getVisitedNodes().size();
                if (nodesAfter > nodesBefore && !bfs->getVisitedNodes().empty()) {
                    int nodeId = bfs->getVisitedNodes().back();
                    stepInfo = QString("访问节点 %1 (%2)").arg(getNodeName(nodeId)).arg(nodeId);
                    nodeProcessed = true;
                }
            };
            if (wrapper->directed)
                process(static_cast<::BFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
            else
                process(static_cast<::BFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
            break;
        }

    case AlgorithmWrapper::DFS_T: {
            // 按 directed 选择匹配的模板实例
            auto process = [&](auto* dfs) {
                size_t nodesBefore = dfs->getVisitedNodes().size();
                hasMoreSteps = dfs->step();
                size_t nodesAfter = dfs->getVisitedNodes().size();
                if (nodesAfter > nodesBefore && !dfs->getVisitedNodes().empty()) {
                    int nodeId = dfs->getVisitedNodes().back();
                    stepInfo = QString("访问节点 %1 (%2)").arg(getNodeName(nodeId)).arg(nodeId);
                    nodeProcessed = true;
                }
            };
            if (wrapper->directed)
                process(static_cast<::DFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
            else
                process(static_cast<::DFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
            break;
        }
        case AlgorithmWrapper::DIJKSTRA_T: {
            // 按 directed 选择匹配的模板实例
            auto process = [&](auto* dijkstra) {
                hasMoreSteps = dijkstra->step();
                int nodeId = dijkstra->getCurrentNode();
                if (nodeId != -1) {
                    QString nodeName = getNodeName(nodeId);
                    stepInfo = QString("处理节点 %1 (%2)").arg(nodeName).arg(nodeId);
                    nodeProcessed = true;

                    // 实时显示完整最短路径
                    if (m_targetNode != -1) {
                        auto path = dijkstra->getPath(m_targetNode);
                        if (!path.empty()) {
                            QStringList pathNames;
                            for (int id : path) {
                                pathNames.append(getNodeName(id));
                            }
                            stepInfo += "\n当前最短路径: " + pathNames.join(" → ");

                            // 高亮路径上的所有边
                            m_highlightedEdges.clear();
                            for (size_t i = 0; i + 1 < path.size(); ++i) {
                                m_highlightedEdges.push_back({path[i], path[i+1]});
                            }
                        }
                    }
                }
            };
            if (wrapper->directed)
                process(static_cast<::Dijkstra<Graph<std::string, int, true>>*>(wrapper->algoPtr));
            else
                process(static_cast<::Dijkstra<Graph<std::string, int, false>>*>(wrapper->algoPtr));
            break;
        }
        case AlgorithmWrapper::KRUSKAL_T: {
            auto* kruskal = static_cast<::Kruskal<Graph<std::string, int, false>>*>(wrapper->algoPtr);
            hasMoreSteps = kruskal->step();
            auto& edges = kruskal->getSelectedEdges();
            if (!edges.empty()) {
                auto& edge = edges.back();
                stepInfo = QString("选择边 %1-%2 (权重: %3)")
                    .arg(getNodeName(edge.from))
                    .arg(getNodeName(edge.to))
                    .arg(edge.weight);
                nodeProcessed = true;  // 标记处理了节点
            }
            break;
        }
    }

    // 检查是否有新节点被访问
    size_t nodesAfterStep = m_highlightedNodes.size();
    bool newNodeVisited = (nodesAfterStep > nodesBeforeStep);

    // 只要有新节点被访问就记录步骤，不管hasMoreSteps
    if (nodeProcessed && newNodeVisited) {
        m_algorithmStepCount++;
        m_currentStep++;

        StepInfo currentStep;
        currentStep.visitedNodes = m_highlightedNodes;
        currentStep.visitedEdges = m_highlightedEdges;
        currentStep.description = stepInfo;
        m_stepHistory.push_back(currentStep);

        emit algorithmStep(m_algorithmStepCount, stepInfo);
    }

    // 处理算法结束
    if (!hasMoreSteps) {
        m_animationTimer->stop();
        m_algorithmState = PAUSED;

        QString resultText;
        auto* wrapper = static_cast<AlgorithmWrapper*>(m_algorithm);

        switch(wrapper->type) {
            case AlgorithmWrapper::BFS_T: {
                resultText = QString("BFS遍历完成，共访问 %1 个节点").arg(m_highlightedNodes.size());
                // 按 directed 选择匹配的模板实例，取算法内部记录的遍历顺序
                auto collect = [&](auto* bfs) {
                    const auto& visitedOrder = bfs->getVisitedNodes();
                    QStringList visitOrder;
                    for (int nodeId : visitedOrder) {
                        visitOrder.append(getNodeName(nodeId));
                    }
                    resultText += "\n遍历顺序: " + visitOrder.join(" → ");
                    if (static_cast<int>(visitedOrder.size()) < getNodeCount()) {
                        resultText += QString("\n提示：图不连通，另有 %1 个节点从起点不可达、未被遍历")
                                            .arg(getNodeCount() - static_cast<int>(visitedOrder.size()));
                    }
                };
                if (wrapper->directed)
                    collect(static_cast<::BFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                else
                    collect(static_cast<::BFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                break;
            }
            case AlgorithmWrapper::DFS_T: {
                resultText = QString("DFS遍历完成，共访问 %1 个节点").arg(m_highlightedNodes.size());
                // 按 directed 选择匹配的模板实例
                auto collect = [&](auto* dfs) {
                    const auto& visitedOrder = dfs->getVisitedNodes();
                    QStringList visitOrder;
                    for (int nodeId : visitedOrder) {
                        visitOrder.append(getNodeName(nodeId));
                    }
                    resultText += "\n遍历顺序: " + visitOrder.join(" → ");
                    if (static_cast<int>(visitedOrder.size()) < getNodeCount()) {
                        resultText += QString("\n提示：图不连通，另有 %1 个节点从起点不可达、未被遍历")
                                            .arg(getNodeCount() - static_cast<int>(visitedOrder.size()));
                    }
                };
                if (wrapper->directed)
                    collect(static_cast<::DFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                else
                    collect(static_cast<::DFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                break;
            }
            case AlgorithmWrapper::KRUSKAL_T: {
                auto* kDone = static_cast<Kruskal<Graph<std::string, int, false>>*>(wrapper->algoPtr);
                resultText = QString("最小生成树构建完成，总权重: %1").arg(kDone->getTotalWeight());
                int kSelected = static_cast<int>(kDone->getSelectedEdges().size());
                if (kSelected < getNodeCount() - 1) {
                    resultText += QString("\n提示：图不连通，得到的是最小生成森林（含 %1 个连通分量）")
                                      .arg(getNodeCount() - kSelected);
                }
                break;
            }
            case AlgorithmWrapper::DIJKSTRA_T:
                // 最短路径已在步骤中处理；按 directed 选择匹配的模板实例
                {
                    auto collect = [&](auto* d) {
                        const auto dMap = d->getDistances();
                        bool reachable = (m_targetNode != -1) && dMap.count(m_targetNode)
                                         && dMap.at(m_targetNode).visited;
                        if (reachable) {
                            auto path = d->getPath(m_targetNode);
                            QStringList pathNames;
                            for (int id : path) {
                                pathNames.append(getNodeName(id));
                            }
                            resultText = QString("最短路径计算完成\n路径: %1\n总距离: %2")
                                .arg(pathNames.join(" → "))
                                .arg(dMap.at(m_targetNode).distance);
                        } else {
                            resultText = QString("目标节点 %1 不可达：不存在从起点到达它的路径")
                                            .arg(getNodeName(m_targetNode));
                        }
                    };
                    if (wrapper->directed)
                        collect(static_cast<::Dijkstra<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                    else
                        collect(static_cast<::Dijkstra<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                }
                break;
        }

        emit algorithmFinished(QString("算法执行完成，共 %1 步\n%2")
                            .arg(m_algorithmStepCount).arg(resultText));
        emit algorithmStateChanged(PAUSED);
    }

    updateNodeColors();
    updateEdgeColors();
}

void GraphCanvas::pauseAlgorithm() {
    if (m_algorithmState == RUNNING) {
        m_animationTimer->stop();
        m_algorithmState = PAUSED;
        emit algorithmPaused();
        emit algorithmStateChanged(PAUSED);
    }
}

void GraphCanvas::resumeAlgorithm() {
    if (m_algorithmState == PAUSED) {
        m_algorithmState = RUNNING;
        int interval = 1100 - m_animationSpeed * 100;
        m_animationTimer->start(interval);
        emit algorithmResumed();
        emit algorithmStateChanged(RUNNING);
    }
}

void GraphCanvas::previousStep() {
    if (m_currentStep > 0) {
        m_currentStep--;
        restoreStepFromHistory(m_currentStep);
        m_algorithmStepCount--;
        updateNodeColors();
        updateEdgeColors();

        QString stepDesc = m_stepHistory[m_currentStep].description;
        emit algorithmStep(m_algorithmStepCount, QString("回退到 %1").arg(stepDesc));
    }
}

void GraphCanvas::nextStep() {
    if (m_algorithm && m_algorithmState == PAUSED) {
        if (m_currentStep < static_cast<int>(m_stepHistory.size()) - 1) {
            m_currentStep++;
            restoreStepFromHistory(m_currentStep);
            m_algorithmStepCount++;
            updateNodeColors();
            updateEdgeColors();

            QString stepDesc = m_stepHistory[m_currentStep].description;
            emit algorithmStep(m_algorithmStepCount, stepDesc);
        } else {
            stepAlgorithm();
        }
    }
}

void GraphCanvas::resetAlgorithm() {
    m_animationTimer->stop();
    m_algorithmState = IDLE;
    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_highlightedNodes.clear();
    m_highlightedEdges.clear();
    m_rejectedEdges.clear();  // 清除被拒绝的边
    m_targetNode = -1;
    clearStepHistory();

    deleteAlgorithm();

    for (auto& [id, item] : m_nodeItems) {
        if (item.shape) {
            item.shape->setBrush(QBrush(Qt::lightGray));
        }
    }

    for (auto& edgeItem : m_edgeItems) {
        if (edgeItem->line) {
            QPen pen(Qt::black, 3);
            pen.setCapStyle(Qt::RoundCap);
            edgeItem->line->setPen(pen);
        }
    }

    emit algorithmStateChanged(IDLE);
}

void GraphCanvas::runAll() {
    if (!m_algorithm || m_algorithmState != RUNNING) return;

    auto* wrapper = static_cast<AlgorithmWrapper*>(m_algorithm);

    bool hasMoreSteps = true;
    while (hasMoreSteps) {
        m_algorithmStepCount++;
        m_currentStep++;

        StepInfo currentStep;
        currentStep.visitedNodes = m_highlightedNodes;
        currentStep.visitedEdges = m_highlightedEdges;
        currentStep.description = QString("第 %1 步").arg(m_algorithmStepCount);
        m_stepHistory.push_back(currentStep);

        switch(wrapper->type) {
            case AlgorithmWrapper::BFS_T: {
                auto doStep = [&](auto* algo) { hasMoreSteps = algo->step(); };
                if (wrapper->directed)
                    doStep(static_cast<::BFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                else
                    doStep(static_cast<::BFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                break;
            }
            case AlgorithmWrapper::DFS_T: {
                auto doStep = [&](auto* algo) { hasMoreSteps = algo->step(); };
                if (wrapper->directed)
                    doStep(static_cast<::DFS<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                else
                    doStep(static_cast<::DFS<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                break;
            }
            case AlgorithmWrapper::DIJKSTRA_T: {
                auto doStep = [&](auto* algo) { hasMoreSteps = algo->step(); };
                if (wrapper->directed)
                    doStep(static_cast<::Dijkstra<Graph<std::string, int, true>>*>(wrapper->algoPtr));
                else
                    doStep(static_cast<::Dijkstra<Graph<std::string, int, false>>*>(wrapper->algoPtr));
                break;
            }
            case AlgorithmWrapper::KRUSKAL_T: {
                // Kruskal 仅支持无向图/树，不存在有向实例
                auto* kruskal = static_cast<::Kruskal<Graph<std::string, int, false>>*>(wrapper->algoPtr);
                hasMoreSteps = kruskal->step();
                break;
            }
        }

        updateNodeColors();
        updateEdgeColors();
    }

    m_animationTimer->stop();
    m_algorithmState = IDLE;

    emit algorithmFinished(QString("算法执行完成，共 %1 步").arg(m_algorithmStepCount));
    emit algorithmStateChanged(IDLE);
}

void GraphCanvas::onNodeVisited(int nodeId) {
    m_highlightedNodes.insert(nodeId);
}

void GraphCanvas::updateAnimation() {
    if (m_algorithm && m_algorithmState == RUNNING) {
        stepAlgorithm();
    }
}

// ============================================
// 文件IO

bool GraphCanvas::saveGraph(const QString& filePath) {
    QJsonObject json;

    // 根据图类型调用相应的序列化方法
    switch (m_graphType) {
        case TREE:
        case UNDIRECTED_GRAPH:
            json = JsonSerializer::graphToJson(
                static_cast<Graph<std::string, int, false>*>(m_graphData)
            );
            break;
        case DIRECTED_GRAPH:
            json = JsonSerializer::graphToJson(
                static_cast<Graph<std::string, int, true>*>(m_graphData)
            );
            break;
    }

    return JsonSerializer::saveToFile(json, filePath);
}

bool GraphCanvas::loadGraph(const QString& filePath) {
    auto json = JsonSerializer::loadFromFile(filePath);
    if (json.isEmpty()) return false;

    // 检查图类型是否匹配
    QString graphType = json["type"].toString();
    bool isDirectedFile = (graphType == "directed");
    bool isCurrentDirected = (m_graphType == DIRECTED_GRAPH);

    if (isDirectedFile != isCurrentDirected) {
        QMessageBox::warning(this, "错误",
            QString("文件类型(%1)与当前图类型不匹配！\n请切换图类型后再加载。")
            .arg(isDirectedFile ? "有向图" : "无向图"));
        return false;
    }

    // 停止算法和动画
    m_animationTimer->stop();
    m_algorithmState = IDLE;
    deleteAlgorithm();

    // 完全清除场景和所有UI状态（包括坐标轴）
    m_scene->clear();
    m_nodeItems.clear();
    m_edgeItems.clear();
    m_nodeNames.clear();
    m_nameToId.clear();
    m_nextNodeId = 0;
    m_highlightedNodes.clear();
    m_highlightedEdges.clear();
    m_rejectedEdges.clear();
    m_edgeStartNode = -1;
    m_hoveredNode = -1;
    m_hoveredEdge = -1;
    m_isDraggingNode = false;
    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_startNode = -1;
    m_targetNode = -1;
    m_selectedNode = -1;
    clearStepHistory();

    // 清除坐标轴
    m_xAxis = nullptr;
    m_yAxis = nullptr;
    m_axisLabels.clear();

    // 【关键修复】清除并重新创建图数据结构
    cleanupGraphData();
    switch (m_graphType) {
        case TREE:
            m_graphData = new Tree<std::string>();
            static_cast<Tree<std::string>*>(m_graphData)->setErrorCallback(
                [this](const QString& msg) {
                    QMessageBox::warning(this, "树结构错误", msg);
                });
            break;
        case DIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, true>();
            break;
        case UNDIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, false>();
            break;
    }

    // 从JSON加载到当前图数据结构
    if (m_graphType == DIRECTED_GRAPH) {
        Graph<std::string, int, true>* graph =
            static_cast<Graph<std::string, int, true>*>(m_graphData);
        JsonSerializer::jsonToGraph(json, graph);
        rebuildCanvasFromGraph(graph);
    } else {
        Graph<std::string, int, false>* graph =
            static_cast<Graph<std::string, int, false>*>(m_graphData);
        JsonSerializer::jsonToGraph(json, graph);
        rebuildCanvasFromGraph(graph);
    }

    // 重建坐标轴（如果需要）
    if (m_showAxes) {
        updateAxes();
    }

    emit graphModified();

    return true;
}

// 从图数据结构重建画布
template<bool Directed>
void GraphCanvas::rebuildCanvasFromGraph(Graph<std::string, int, Directed>* graph) {
    // 不再调用 m_scene->clear()，因为 loadGraph() 已调用 clear()
    // 直接重建UI元素

    m_nodeItems.clear();
    m_edgeItems.clear();
    m_nodeNames.clear();
    m_nameToId.clear();
    m_nextNodeId = 0;

    // 获取所有节点ID
    auto nodeIds = graph->getAllNodeIds();

    // 重建节点
    for (int id : nodeIds) {
        auto* node = graph->getNode(id);
        if (!node) continue;

        double x = node->x();
        double y = node->y();
        std::string label = node->data();

        NodeItem item;
        item.id = id;
        item.originalPos = QPointF(x, y);

        QPen nodePen(Qt::black, 2);
        nodePen.setJoinStyle(Qt::RoundJoin);
        item.shape = m_scene->addEllipse(-25, -25, 50, 50, nodePen, QBrush(Qt::lightGray));
        item.shape->setPos(x, y);
        item.shape->setZValue(10);

        item.label = new QGraphicsSimpleTextItem(QString::fromStdString(label));
        item.label->setPos(-item.label->boundingRect().width()/2, -35);
        item.label->setBrush(Qt::black);
        item.label->setFont(QFont("Arial", 10, QFont::Bold));
        item.label->setZValue(20);
        item.label->setParentItem(item.shape);
        item.label->setTransform(QTransform::fromScale(1, -1));

        item.coordinateLabel = new QGraphicsSimpleTextItem(QString("(%1, %2)").arg(x).arg(y));
        item.coordinateLabel->setPos(-item.coordinateLabel->boundingRect().width()/2, 30);
        item.coordinateLabel->setBrush(Qt::darkGray);
        item.coordinateLabel->setFont(QFont("Arial", 8));
        item.coordinateLabel->setVisible(m_showCoordinates);
        item.coordinateLabel->setZValue(20);
        item.coordinateLabel->setParentItem(item.shape);
        item.coordinateLabel->setTransform(QTransform::fromScale(1, -1));

        m_nodeItems[id] = item;
        m_nodeNames[id] = QString::fromStdString(label);
        m_nameToId[m_nodeNames[id]] = id;

        if (id >= m_nextNodeId) {
            m_nextNodeId = id + 1;
        }
    }

    // 重建边（仅UI，不修改图数据结构）
    auto edges = graph->getEdges();
    for (const auto* edge : edges) {
        int from = edge->from();
        int to = edge->to();
        int weight = edge->weight();

        // 手动创建UI元素，不调用 createEdge
        auto item = std::make_unique<EdgeItem>();
        item->from = from;
        item->to = to;
        item->isHovered = false;

        auto [fromX, fromY] = getNodePosition(from);
        auto [toX, toY] = getNodePosition(to);

        QPointF fromEdge = getNodeEdgePoint(from, QPointF(toX, toY));
        QPointF toEdge = getNodeEdgePoint(to, QPointF(fromX, fromY));

        QPen edgePen(Qt::black, 3);
        edgePen.setCapStyle(Qt::RoundCap);
        item->line = m_scene->addLine(
            fromEdge.x(), fromEdge.y(),
            toEdge.x(), toEdge.y(),
            edgePen
        );
        item->line->setZValue(5);

        if (m_graphType == DIRECTED_GRAPH) {
            drawArrow(item.get(), fromEdge.x(), fromEdge.y(), toEdge.x(), toEdge.y());
        }

        auto midX = (fromEdge.x() + toEdge.x()) / 2;
        auto midY = (fromEdge.y() + toEdge.y()) / 2;
        item->weight = new QGraphicsSimpleTextItem(QString::number(weight));
        item->weight->setPos(midX, midY);

        QTransform transform;
        transform.translate(-item->weight->boundingRect().width()/2,
                           -item->weight->boundingRect().height()/2);
        transform.scale(1, -1);
        item->weight->setTransform(transform);

        item->weight->setBrush(Qt::darkRed);
        item->weight->setFont(QFont("Arial", 9, QFont::Bold));
        item->weight->setVisible(m_showWeights);
        item->weight->setZValue(15);
        m_scene->addItem(item->weight);

        m_edgeItems.push_back(std::move(item));
    }
}

// 显式实例化模板（必须！）
template void GraphCanvas::rebuildCanvasFromGraph<false>(Graph<std::string, int, false>* graph);
template void GraphCanvas::rebuildCanvasFromGraph<true>(Graph<std::string, int, true>* graph);

// ============================================
// 其他

void GraphCanvas::paintEvent(QPaintEvent* event) {
    QGraphicsView::paintEvent(event);
}

void GraphCanvas::setSelectedNode(int nodeId) {
    if (nodeId == m_selectedNode) return;

    if (m_selectedNode != -1 && m_nodeItems.count(m_selectedNode)) {
        m_nodeItems[m_selectedNode].shape->setBrush(QBrush(Qt::lightGray));
    }

    m_selectedNode = nodeId;

    if (m_selectedNode != -1 && m_nodeItems.count(m_selectedNode)) {
        m_nodeItems[m_selectedNode].shape->setBrush(QBrush(QColor(0, 255, 0, 200)));
    }

    updateNodeColors();
}

// ============================================
// 新增函数实现

QString GraphCanvas::getNodeName(int nodeId) const {
    auto it = m_nodeNames.find(nodeId);
    if (it != m_nodeNames.end()) {
        return it->second;
    }
    return QString("Node%1").arg(nodeId);
}

void GraphCanvas::setNodeName(int nodeId, const QString& name) {
    if (!hasNode(nodeId)) return;

    // 检查名称是否已被使用
    if (m_nameToId.find(name) != m_nameToId.end() && m_nameToId[name] != nodeId) {
        QMessageBox::warning(nullptr, "错误", QString("名称 '%1' 已被其他节点使用！").arg(name));
        return;
    }

    // 更新映射
    QString oldName = m_nodeNames[nodeId];
    m_nodeNames[nodeId] = name;
    m_nameToId.erase(oldName);
    m_nameToId[name] = nodeId;

    // 更新显示标签
    if (m_nodeItems.count(nodeId) && m_nodeItems[nodeId].label) {
        m_nodeItems[nodeId].label->setText(name);
    }

    // 更新图数据结构中的节点数据
    setNodeData(nodeId, name.toStdString());
}

int GraphCanvas::getNodeIdByName(const QString& name) const {
    auto it = m_nameToId.find(name);
    if (it != m_nameToId.end()) {
        return it->second;
    }
    return -1;
}

QStringList GraphCanvas::getSubtreeNodes(int rootNodeId) {
    QStringList result;
    if (m_graphType != TREE) return result;

    // 使用队列进行广度优先搜索
    std::queue<int> q;
    q.push(rootNodeId);

    while (!q.empty()) {
        int current = q.front();
        q.pop();
        result.append(getNodeName(current));

        // 获取子节点
        auto children = getTreeChildren(current);
        for (int child : children) {
            q.push(child);
        }
    }

    return result;
}

QStringList GraphCanvas::getChildren(int parentNodeId) {
    QStringList result;
    if (m_graphType != TREE) return result;

    auto children = getTreeChildren(parentNodeId);
    for (int child : children) {
        result.append(getNodeName(child));
    }

    return result;
}

void GraphCanvas::deleteNodeAndEdges(int nodeId) {
    if (!m_nodeItems.count(nodeId)) return;

    // 先删除与该节点相连的所有边
    QVector<int> edgesToDelete;
    for (int i = 0; i < static_cast<int>(m_edgeItems.size()); ++i) {
        if (m_edgeItems[i]->from == nodeId || m_edgeItems[i]->to == nodeId) {
            edgesToDelete.append(i);
        }
    }

    // 从后往前删除边，避免索引变化
    for (int i = edgesToDelete.size() - 1; i >= 0; --i) {
        onEdgeDeleteRequested(edgesToDelete[i]);
    }

    // 如果是树结构，递归删除子树
    if (m_graphType == TREE) {
        std::queue<int> nodesToDelete;
        nodesToDelete.push(nodeId);

        while (!nodesToDelete.empty()) {
            int current = nodesToDelete.front();
            nodesToDelete.pop();

            // 获取子节点
            auto children = getTreeChildren(current);
            for (int child : children) {
                nodesToDelete.push(child);
            }

            // 删除节点
            if (current != nodeId) {
                removeNodeFromGraph(current);

                // 从场景中删除节点图形项
                if (m_nodeItems.count(current)) {
                    NodeItem& item = m_nodeItems[current];
                    // label/coordinateLabel 是 shape 的子图元，delete shape 时级联释放，不能再单独 delete
                    if (item.shape) {
                        m_scene->removeItem(item.shape);
                        delete item.shape;
                    }

                    // 从映射表中删除
                    QString nodeName = m_nodeNames[current];
                    m_nodeItems.erase(current);
                    m_nodeNames.erase(current);
                    m_nameToId.erase(nodeName);
                }
            }
        }
    }

    // 从图数据结构中删除主节点
    removeNodeFromGraph(nodeId);

    // 从场景中删除节点图形项
    NodeItem& item = m_nodeItems[nodeId];
    // label/coordinateLabel 是 shape 的子图元，delete shape 时级联释放，不能再单独 delete
    if (item.shape) {
        m_scene->removeItem(item.shape);
        delete item.shape;
    }

    // 从映射表中删除
    QString nodeName = m_nodeNames[nodeId];
    m_nodeItems.erase(nodeId);
    m_nodeNames.erase(nodeId);
    m_nameToId.erase(nodeName);
}

std::vector<int> GraphCanvas::getTreeChildren(int nodeId) const {
    std::vector<int> children;
    if (m_graphType != TREE) return children;

    for (const auto& edge : m_edgeItems) {
        if (edge->from == nodeId) {
            children.push_back(edge->to);
        }
    }

    return children;
}

// 修复权值文字的方法
void GraphCanvas::updateEdgeWeightPosition(EdgeItem* edgeItem) {
    if (!edgeItem || !edgeItem->line || !edgeItem->weight) return;

    QLineF line = edgeItem->line->line();
    QPointF midPoint = line.pointAt(0.5);

    edgeItem->weight->setPos(midPoint);

    QTransform transform;
    transform.translate(-edgeItem->weight->boundingRect().width()/2,
                       -edgeItem->weight->boundingRect().height()/2);
    transform.scale(1, -1);
    edgeItem->weight->setTransform(transform);
}

void GraphCanvas::fixAllEdgeWeightsTransform() {
    for (auto& edgeItem : m_edgeItems) {
        updateEdgeWeightPosition(edgeItem.get());
    }
}

bool GraphCanvas::loadGraphQuietly(const QString& filePath) {
    auto json = JsonSerializer::loadFromFile(filePath);
    if (json.isEmpty()) {
        return false; // 加载失败，不弹窗
    }

    // 检查图类型是否匹配
    QString graphType = json["type"].toString();
    bool isDirectedFile = (graphType == "directed");
    bool isCurrentDirected = (m_graphType == DIRECTED_GRAPH);

    if (isDirectedFile != isCurrentDirected) {
        return false; // 类型不匹配，不弹窗
    }

    // 停止算法和动画
    m_animationTimer->stop();
    m_algorithmState = IDLE;
    deleteAlgorithm();

    // 完全清除场景和所有UI状态
    m_scene->clear();
    m_nodeItems.clear();
    m_edgeItems.clear();
    m_nodeNames.clear();
    m_nameToId.clear();
    m_nextNodeId = 0;
    m_highlightedNodes.clear();
    m_highlightedEdges.clear();
    m_rejectedEdges.clear();
    m_edgeStartNode = -1;
    m_hoveredNode = -1;
    m_hoveredEdge = -1;
    m_isDraggingNode = false;
    m_algorithmStepCount = 0;
    m_currentStep = 0;
    m_startNode = -1;
    m_targetNode = -1;
    m_selectedNode = -1;
    clearStepHistory();

    // 清除坐标轴
    m_xAxis = nullptr;
    m_yAxis = nullptr;
    m_axisLabels.clear();

    // 清除并重新创建图数据结构
    cleanupGraphData();
    switch (m_graphType) {
        case TREE:
            m_graphData = new Tree<std::string>();
            static_cast<Tree<std::string>*>(m_graphData)->setErrorCallback(
                [this](const QString& msg) {
                    QMessageBox::warning(this, "树结构错误", msg);
                });
            break;
        case DIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, true>();
            break;
        case UNDIRECTED_GRAPH:
            m_graphData = new Graph<std::string, int, false>();
            break;
    }

    // 从JSON加载到当前图数据结构
    if (m_graphType == DIRECTED_GRAPH) {
        Graph<std::string, int, true>* graph =
            static_cast<Graph<std::string, int, true>*>(m_graphData);
        JsonSerializer::jsonToGraph(json, graph);
        rebuildCanvasFromGraph(graph);
    } else {
        Graph<std::string, int, false>* graph =
            static_cast<Graph<std::string, int, false>*>(m_graphData);
        JsonSerializer::jsonToGraph(json, graph);
        rebuildCanvasFromGraph(graph);
    }

    // 重建坐标轴（如果需要）
    if (m_showAxes) {
        updateAxes();
    }

    emit graphModified();
    return true;
}
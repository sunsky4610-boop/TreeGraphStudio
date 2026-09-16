#pragma once

#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QTimer>
#include <QElapsedTimer>
#include <QColor>
#include <QPainter>
#include <QVariantAnimation>
#include <QMenu>
#include <QInputDialog>
#include <QApplication>
#include <QCursor>
#include <QMessageBox>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <map>
#include <QSet>
#include <QQueue>

// 包含 Graph 头文件
#include "../core/Graph.h"
#include "../core/Tree.h"

// 前向声明算法类模板
template<typename GraphType> class BFS;
template<typename GraphType> class DFS;
template<typename GraphType> class Dijkstra;
template<typename GraphType> class Kruskal;

class QWheelEvent;

class GraphCanvas : public QGraphicsView {
    Q_OBJECT

public:
    enum Mode { SELECT, ADD_NODE, ADD_EDGE };
    enum GraphType { TREE, DIRECTED_GRAPH, UNDIRECTED_GRAPH };
    enum AlgorithmState { IDLE, RUNNING, PAUSED };
    enum AlgorithmType { BFS, DFS, SHORTEST_PATH, MST };

    explicit GraphCanvas(QWidget* parent = nullptr);
    ~GraphCanvas();

    // 图管理
    void setGraphType(GraphType type);
    void clear();
    // 重置视图到标准数学坐标系(Y 轴朝上)，用于"重置视图"以及新建/加载后自愈文字方向
    void resetViewTransform();
    void setDarkMode(bool dark);
    void centerGraphAnimated(bool fitAll = true);
    void ensureGraphVisibleAnimated();
    void loadExample();
    
    // 获取图信息
    int getNodeCount() const;
    int getEdgeCount() const;
    bool hasNode(int nodeId) const;  // 保持数字ID用于内部操作
    bool hasEdge(int from, int to) const;  // 保持数字ID用于内部操作
    
    // 算法控制
    void startAlgorithm(int startNode, AlgorithmType algoType);
    void startShortestPath(int startNode, int targetNode);  // 新增：专门的路径算法
    void stepAlgorithm();
    void pauseAlgorithm();
    void resumeAlgorithm();
    void previousStep();
    void nextStep();
    void resetAlgorithm();
    void runAll();

    // 编辑控制
    void setMode(Mode mode);
    void setShowWeights(bool show);
    void setShowCoordinates(bool show);
    void setShowAxes(bool show);

    // 速度控制
    void setAnimationSpeed(int speed) { m_animationSpeed = speed; }
    int animationSpeed() const { return m_animationSpeed; }

    // 文件IO
    bool saveGraph(const QString& filePath);
    bool loadGraph(const QString& filePath);

    // 获取算法信息
    QString getAlgorithmInfo() const;

    // 获取算法状态
    AlgorithmState getAlgorithmState() const { return m_algorithmState; }

    // 获取当前图对象（用于练习系统）
    const ::Graph<std::string, int, false>* getCurrentGraph() const;

    // 获取节点信息
    QString getNodeInfo(int nodeId) const;
    QString getEdgeInfo(int edgeIndex) const;
    
    // 节点名称处理
    QString getNodeName(int nodeId) const;
    void setNodeName(int nodeId, const QString& name);
    int getNodeIdByName(const QString& name) const;
    
    // 获取子树节点（树结构专用）
    QStringList getSubtreeNodes(int rootNodeId);
    QStringList getChildren(int parentNodeId);

    // 获取当前图类型
    GraphType getGraphType() const { return m_graphType; }

    // 模板化的图对象获取函数
    template<bool Directed>
    const Graph<std::string, int, Directed>* getGraphTyped() const {
        if (m_graphData) {
            return static_cast<const Graph<std::string, int, Directed>*>(m_graphData);
        }
        return nullptr;
    }

    // 静默加载图（不弹出错误提示，返回bool）
    bool loadGraphQuietly(const QString& filePath);


signals:
    // 使用数字ID传递信号
    void nodeSelected(int nodeId);
    void graphModified();
    void algorithmStarted(const QString& algoName, int startNode);
    void algorithmStep(int step, const QString& info);
    void algorithmPaused();
    void algorithmResumed();
    void algorithmFinished(const QString& result);
    void nodeHovered(int nodeId, const QString& nodeInfo);
    void edgeHovered(int from, int to, const QString& edgeInfo);
    void algorithmStateChanged(AlgorithmState state);
    void algorithmStatusUpdated(const QString& message);
    void algorithmProgressUpdated(const QString& info);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onNodeVisited(int nodeId);
    void updateAnimation();
    void onWeightEditRequested(int edgeIndex);
    void onEdgeDeleteRequested(int edgeIndex);
    void onNodeDeleteRequested(int nodeId);
    void onNodeRenameRequested(int nodeId);

private:
    // 结构体定义
    struct NodeItem {
        QGraphicsEllipseItem* shape{nullptr};
        QGraphicsSimpleTextItem* label{nullptr};
        QGraphicsSimpleTextItem* coordinateLabel{nullptr};
        int id;
        bool isDragging{false};
        QPointF originalPos;
        // —— L2 视觉动画 ——
        QColor targetColor{QColor(211, 211, 211, 220)};  // 逻辑目标填充色
        QColor dispColor{QColor(211, 211, 211, 220)};    // 当前显示色（每帧向目标插值）
        float spawnT{1.0f};        // 创建弹出进度 0→1，1 表示完成
        float hoverT{0.0f};        // 悬停放大进度 0→1
        bool wasHighlight{false}; // 上一帧是否高亮（用于边沿触发涟漪）
    };

    struct EdgeItem {
        QGraphicsLineItem* line{nullptr};
        QGraphicsPolygonItem* arrow{nullptr};
        QGraphicsSimpleTextItem* weight{nullptr};
        int from;
        int to;
        bool isHovered{false};
        // —— L2 视觉动画 ——
        QColor targetColor{QColor(0, 0, 0, 150)};
        QColor dispColor{QColor(0, 0, 0, 150)};
        QColor targetArrowColor{QColor(Qt::black)};
        QColor dispArrowColor{QColor(Qt::black)};
        float targetWidth{3.0f};
        float dispWidth{3.0f};
        Qt::PenStyle targetStyle{Qt::SolidLine};
    };

    // 访问节点时向外扩散的涟漪环
    struct Ripple {
        QGraphicsEllipseItem* ring{nullptr};
        float t{0.0f};
    };

    struct StepInfo {
        std::set<int> visitedNodes;
        std::vector<std::pair<int, int>> visitedEdges;
        QString description;
    };

    // 算法包装器
    struct AlgorithmWrapper {
        enum Type { BFS_T, DFS_T, DIJKSTRA_T, KRUSKAL_T };
        Type type;
        void* algoPtr;
        // 记录算法绑定的图是否有向：Graph<...,true> 与 Graph<...,false> 是不同类型，
        // 后续所有 static_cast 必须与此标记一致，否则属于未定义行为
        bool directed{false};
        
        ~AlgorithmWrapper();
    };

    // UI组件
    QGraphicsScene* m_scene;
    GraphType m_graphType{UNDIRECTED_GRAPH};
    Mode m_mode{SELECT};
    bool m_showWeights{true};
    bool m_showCoordinates{false};
    bool m_showAxes{false};

    // 图数据指针
    void* m_graphData{nullptr};
    AlgorithmWrapper* m_algorithm{nullptr};
    
    AlgorithmState m_algorithmState{IDLE};
    QTimer* m_animationTimer;
    QTimer* m_animClock{nullptr};   // L2：60fps 视觉动画时钟
    QElapsedTimer m_frameTimer;     // 帧间隔计时（帧率无关）
    qint64 m_lastFrameMs{0};
    std::vector<Ripple> m_ripples; // 正在扩散的涟漪
    std::set<int> m_highlightedNodes;
    std::vector<std::pair<int, int>> m_highlightedEdges;
    
    // 被拒绝的边
    std::vector<std::pair<int, int>> m_rejectedEdges;
    
    int m_animationSpeed{5};
    int m_algorithmStepCount{0};
    int m_currentStep{0};
    QString m_currentAlgorithm;
    std::vector<StepInfo> m_stepHistory;
    int m_startNode{-1};
    int m_targetNode{-1};
    AlgorithmType m_currentAlgorithmType{BFS};

    // 坐标轴
    QGraphicsLineItem* m_xAxis{nullptr};
    QGraphicsLineItem* m_yAxis{nullptr};
    QList<QGraphicsSimpleTextItem*> m_axisLabels;

    // 编辑相关
    std::unordered_map<int, NodeItem> m_nodeItems;
    std::vector<std::unique_ptr<EdgeItem>> m_edgeItems;
    int m_nextNodeId{0};
    int m_edgeStartNode{-1};
    QGraphicsItem* m_dragItem{nullptr};
    QPointF m_dragOffset;
    int m_hoveredNode{-1};
    int m_hoveredEdge{-1};
    bool m_isDraggingNode{false};
    bool m_isPanning{false};
    QPoint m_lastPanPos;
    QVariantAnimation* m_centerAnimation{nullptr};
    bool m_darkMode{false};
    int m_selectedNode{-1};

    // 名称-ID映射
    std::unordered_map<int, QString> m_nodeNames;
    std::unordered_map<QString, int> m_nameToId;

    // 核心方法
    void createNode(double x, double y, const std::string& label = "");
    void createEdge(int from, int to, int weight);
    void updateAllEdges();
    void updateNodeColors();
    void updateEdgeColors();
    void tickVisualEffects();                          // 60fps 视觉动画推进
    void spawnRipple(int nodeId);                      // 在节点处生成涟漪
    static QColor lerpColor(const QColor& a, const QColor& b, float t);
    void updateNodeLabels();
    void updateAxes();
    int findNodeAt(const QPointF& pos) const;
    int findEdgeAt(const QPointF& pos) const;
    QPointF getNodeEdgePoint(int nodeId, const QPointF& target) const;
    void drawArrow(EdgeItem* edgeItem, double fromX, double fromY, double toX, double toY);
    void showEdgeContextMenu(int edgeIndex, const QPoint& screenPos);
    void showNodeContextMenu(int nodeId, const QPoint& screenPos);
    void saveStepToHistory();
    void restoreStepFromHistory(int stepIndex);
    void clearStepHistory();
    void setSelectedNode(int nodeId);
    
    // 图数据操作
    int addNodeToGraph(const std::string& data);
    bool addEdgeToGraph(int from, int to, int weight);
    void removeEdgeFromGraph(int from, int to);
    void removeNodeFromGraph(int nodeId);
    std::vector<int> getGraphNeighbors(int nodeId) const;
    void* getNodeObject(int nodeId) const;
    void setNodePosition(int nodeId, double x, double y);
    std::pair<double, double> getNodePosition(int nodeId) const;
    std::string getNodeData(int nodeId) const;
    void setNodeData(int nodeId, const std::string& data);
    std::vector<std::tuple<int, int, int>> getAllEdges() const;
    bool isDirectedGraph() const;
    
    void cleanupGraphData();
    void deleteAlgorithm();
    
    // 辅助方法
    void deleteNodeAndEdges(int nodeId);
    std::vector<int> getTreeChildren(int nodeId) const;
    QString generateNodeName(int nodeId) const;
    
    // 修复权值文字的方法
    void updateEdgeWeightPosition(EdgeItem* edgeItem);
    void fixAllEdgeWeightsTransform();

    template<bool Directed>
    void rebuildCanvasFromGraph(Graph<std::string, int, Directed>* graph);
};

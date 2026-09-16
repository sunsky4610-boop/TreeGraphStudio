#include "GraphPracticeWidget.h"
#include "utils/PathUtils.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>

GraphPracticeWidget::GraphPracticeWidget(QWidget* parent)
    : QWidget(parent) {
    
    setupUI();
    refreshFileList();
}

void GraphPracticeWidget::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 左侧文件列表
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);

    auto* intro = new QLabel("内置练习库\n选择一个场景，阅读目标后加载到主画布逐步运行。", leftPanel);
    intro->setObjectName("practiceIntro");
    intro->setWordWrap(true);
    leftLayout->addWidget(intro);
    
    auto* dirLayout = new QHBoxLayout();
    m_selectDirBtn = new QPushButton("📁 选择目录", leftPanel);
    connect(m_selectDirBtn, &QPushButton::clicked, this, &GraphPracticeWidget::onSelectDirectory);
    
    auto* refreshBtn = new QPushButton("🔄 刷新", leftPanel);
    connect(refreshBtn, &QPushButton::clicked, this, &GraphPracticeWidget::refreshFileList);
    
    dirLayout->addWidget(m_selectDirBtn);
    dirLayout->addWidget(refreshBtn);
    dirLayout->addStretch();
    
    m_fileList = new QListWidget(leftPanel);
    m_fileList->setMinimumWidth(300);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &GraphPracticeWidget::onFileDoubleClicked);
    connect(m_fileList, &QListWidget::itemSelectionChanged, this, &GraphPracticeWidget::onFileSelectionChanged);
    
    leftLayout->addLayout(dirLayout);
    leftLayout->addWidget(m_fileList);
    
    // 右侧预览和加载
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    
    auto* previewGroup = new QGroupBox("图信息预览", rightPanel);
    auto* previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewLabel = new QLabel("请选择一个图文件", previewGroup);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setAlignment(Qt::AlignTop);
    m_previewLabel->setStyleSheet("padding: 10px;");
    
    previewLayout->addWidget(m_previewLabel);
    
    m_loadBtn = new QPushButton("加载到主窗口", rightPanel);
    m_loadBtn->setEnabled(false);
    m_loadBtn->setStyleSheet("padding: 10px; font-weight: bold;");
    connect(m_loadBtn, &QPushButton::clicked, this, &GraphPracticeWidget::onLoadSelectedGraph);
    
    rightLayout->addWidget(previewGroup);
    rightLayout->addWidget(m_loadBtn);
    rightLayout->addStretch();
    
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
}

void GraphPracticeWidget::onSelectDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择图文件目录",
        m_currentDirectory.isEmpty() ? QDir::homePath() : m_currentDirectory
    );
    
    if (!dir.isEmpty()) {
        m_currentDirectory = dir;
        refreshFileList();
    }
}

void GraphPracticeWidget::refreshFileList() {
    m_fileList->clear();
    m_graphInfoCache.clear();
    
    if (m_currentDirectory.isEmpty()) {
        m_currentDirectory = ":/learning/graphs";
    }
    
    QDir dir(m_currentDirectory);
    // 设置不区分大小写的过滤器
    dir.setNameFilters(QStringList() << "*.graph" << "*.GRAPH");
    
    QFileInfoList fileList = dir.entryInfoList(QDir::Files, QDir::Name);
    
    if (fileList.isEmpty()) {
        m_fileList->addItem("未找到.graph文件\n请在\"" + m_currentDirectory + "\"目录下添加图文件");
        m_fileList->item(0)->setFlags(Qt::NoItemFlags);
        return;
    }
    
    // 使用toLower()统一转换为小写比较
    for (const QFileInfo& info : fileList) {
        if (info.suffix().toLower() == "graph") {
            QString displayText = formatFileInfo(info);
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, info.absoluteFilePath());
            m_fileList->addItem(item);
        }
    }
}

QString GraphPracticeWidget::formatFileInfo(const QFileInfo& info) {
    QJsonObject graphInfo = peekGraphInfo(info.absoluteFilePath());
    
    QString name = info.fileName();
    if (name == "traversal.graph") name = "01 · 图遍历训练";
    else if (name == "shortest-path.graph") name = "02 · 最短路径训练";
    else if (name == "minimum-spanning-tree.graph") name = "03 · 最小生成树训练";
    QString type = graphInfo.value("type").toString();
    int nodeCount = graphInfo.value("nodeCount").toInt();
    int edgeCount = graphInfo.value("edgeCount").toInt();
    
    QString typeStr = (type == "directed") ? "有向图" : "无向图";
    
    return QString("%1\n└─ %2 | 节点:%3 | 边:%4")
           .arg(name)
           .arg(typeStr)
           .arg(nodeCount)
           .arg(edgeCount);
}

QJsonObject GraphPracticeWidget::peekGraphInfo(const QString& filePath) {
    if (m_graphInfoCache.contains(filePath)) {
        return m_graphInfoCache[filePath];
    }
    
    QJsonObject info;
    info["type"] = "unknown";
    info["nodeCount"] = 0;
    info["edgeCount"] = 0;
    
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return info;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject json = doc.object();
    
    // 提取基本信息
    info["type"] = json.value("type").toString();
    
    QJsonArray nodes = json.value("nodes").toArray();
    QJsonArray edges = json.value("edges").toArray();
    
    info["nodeCount"] = nodes.size();
    info["edgeCount"] = edges.size();
    
    m_graphInfoCache[filePath] = info;
    return info;
}

void GraphPracticeWidget::onFileSelectionChanged() {
    QListWidgetItem* currentItem = m_fileList->currentItem();
    if (!currentItem || currentItem->flags() == Qt::NoItemFlags) {
        m_previewLabel->setText("请选择一个图文件");
        m_loadBtn->setEnabled(false);
        return;
    }
    
    QString filePath = currentItem->data(Qt::UserRole).toString();
    QJsonObject info = peekGraphInfo(filePath);
    
    const QString fileName = QFileInfo(filePath).fileName();
    QString title;
    QString algorithm;
    QString mission;
    if (fileName == "traversal.graph" || fileName.contains("BFS", Qt::CaseInsensitive)) {
        title = "图遍历：比较 BFS 与 DFS";
        algorithm = "BFS / DFS";
        mission = "先从 N0 运行 BFS，观察逐层扩散；重置后运行 DFS，比较访问顺序和待访问结构。";
    } else if (fileName == "shortest-path.graph" || fileName.contains("dijkstra", Qt::CaseInsensitive)) {
        title = "加权图：寻找最低代价路径";
        algorithm = "Dijkstra 最短路径";
        mission = "选择 N0 为起点、N6 为终点，逐步观察距离松弛，并判断为什么局部最短边不一定属于最终路径。";
    } else {
        title = "连通网：构造最小生成树";
        algorithm = "Kruskal 最小生成树";
        mission = "按边权从小到大观察选边过程，重点辨认会形成环而被拒绝的边，并核对最终总权重。";
    }

    QString previewText = QString(
        "<h2>%1</h2>"
        "<p><b>推荐算法：</b>%2</p>"
        "<p><b>练习目标：</b>%3</p>"
        "<hr/><p><b>图类型：</b>%4　 <b>节点：</b>%5　 <b>边：</b>%6</p>"
        "<p style='color:#8b5cf6'><b>操作：</b>加载后选择对应算法，使用“下一步”观察每一次状态变化。</p>"
    ).arg(title).arg(algorithm).arg(mission)
     .arg(info.value("type").toString() == "directed" ? "有向图" : "无向图")
     .arg(info.value("nodeCount").toInt()).arg(info.value("edgeCount").toInt());
    
    m_previewLabel->setText(previewText);
    m_loadBtn->setEnabled(info.value("nodeCount").toInt() > 0);
}

void GraphPracticeWidget::onLoadSelectedGraph() {
    QListWidgetItem* currentItem = m_fileList->currentItem();
    if (!currentItem) return;
    
    QString filePath = currentItem->data(Qt::UserRole).toString();
    emit loadGraphRequest(filePath);
}

void GraphPracticeWidget::onFileDoubleClicked(QListWidgetItem* item) {
    if (!item || item->flags() == Qt::NoItemFlags) return;
    
    QString filePath = item->data(Qt::UserRole).toString();
    emit loadGraphRequest(filePath);
}

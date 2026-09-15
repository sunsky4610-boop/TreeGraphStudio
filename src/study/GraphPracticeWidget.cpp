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
        m_currentDirectory = PathUtils::getLearningResourcePath("graphs");
        QDir().mkpath(m_currentDirectory);
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
    
    QString previewText = QString(
        "<b>文件名:</b> %1<br/>"
        "<b>类型:</b> %2<br/>"
        "<b>节点数量:</b> %3<br/>"
        "<b>边数量:</b> %4<br/>"
        "<b>路径:</b> %5<br/><br/>"
        "<i>双击文件或点击\"加载到主窗口\"按钮在主界面中打开此图</i>"
    ).arg(QFileInfo(filePath).fileName())
     .arg(info.value("type").toString() == "directed" ? "有向图" : "无向图")
     .arg(info.value("nodeCount").toInt())
     .arg(info.value("edgeCount").toInt())
     .arg(filePath);
    
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
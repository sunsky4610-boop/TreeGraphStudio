#include "MainWindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStatusBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QInputDialog>
#include <QDateTime>
#include <QCheckBox>
#include <QMetaObject>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include "HelpWindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
      m_studyWindow(nullptr) {
    setupUI();
    m_canvas->loadExample();
}

void MainWindow::setupUI() {
    // 中心组件
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==================== 左侧面板 ====================
    auto* leftPanel = new QGroupBox("工具箱", this);
    leftPanel->setObjectName("leftSidebar");
    leftPanel->setFixedWidth(240);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(14, 16, 14, 14);
    leftLayout->setSpacing(8);

    // 图操作组
    auto* graphOpGroup = new QGroupBox("图操作", leftPanel);
    auto* graphOpLayout = new QVBoxLayout(graphOpGroup);

    // 创建按钮并保存到成员变量
    m_newGraphBtn = new QPushButton("新建图", graphOpGroup);
    m_newTreeBtn = new QPushButton("新建树", graphOpGroup);
    auto* clearBtn = new QPushButton("清空画布", graphOpGroup);

    // 设置按钮可选中
    m_newGraphBtn->setCheckable(true);
    m_newTreeBtn->setCheckable(true);

    // 默认选中新建图按钮
    m_newGraphBtn->setChecked(true);

    // 连接按钮信号（直接连接到槽函数）
    connect(m_newGraphBtn, &QPushButton::clicked, this, &MainWindow::onNewGraph);
    connect(m_newTreeBtn, &QPushButton::clicked, this, &MainWindow::onNewTree);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onReset);

    // 添加互斥逻辑
    connect(m_newGraphBtn, &QPushButton::clicked, [this]() {
        m_newTreeBtn->setChecked(false);
        updateGraphTypeButtons();
    });

    connect(m_newTreeBtn, &QPushButton::clicked, [this]() {
        m_newGraphBtn->setChecked(false);
        updateGraphTypeButtons();
    });

    graphOpLayout->addWidget(m_newGraphBtn);
    graphOpLayout->addWidget(m_newTreeBtn);
    graphOpLayout->addWidget(clearBtn);
    leftLayout->addWidget(graphOpGroup);



    // 创建按钮组
    m_graphTypeButtonGroup = new QButtonGroup(this);
    m_graphTypeButtonGroup->addButton(m_newGraphBtn, 0);
    m_graphTypeButtonGroup->addButton(m_newTreeBtn, 1);

    // 图类型选择
    auto* graphTypeGroup = new QGroupBox("图类型", leftPanel);
    auto* graphTypeLayout = new QHBoxLayout(graphTypeGroup);
    auto* undirectedBtn = new QRadioButton("无向", graphTypeGroup);
    auto* directedBtn = new QRadioButton("有向", graphTypeGroup);
    undirectedBtn->setChecked(true);
    graphTypeLayout->addWidget(undirectedBtn);
    graphTypeLayout->addWidget(directedBtn);
    graphTypeLayout->setSpacing(15);
    graphTypeLayout->setContentsMargins(10, 10, 10, 10);

    m_graphTypeGroup = new QButtonGroup(this);
    m_graphTypeGroup->addButton(undirectedBtn, 0);
    m_graphTypeGroup->addButton(directedBtn, 1);

    // 连接图类型切换信号
    connect(m_graphTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::onGraphTypeChanged);

    leftLayout->addWidget(graphTypeGroup);

    // 编辑模式组
    auto* modeGroup = new QGroupBox("编辑模式", leftPanel);
    auto* modeLayout = new QVBoxLayout(modeGroup);
    modeLayout->setSpacing(8);
    auto* selectModeBtn = new QPushButton("选择模式", modeGroup);
    auto* addNodeModeBtn = new QPushButton("添加节点", modeGroup);
    auto* addEdgeModeBtn = new QPushButton("添加边", modeGroup);

    selectModeBtn->setCheckable(true);
    addNodeModeBtn->setCheckable(true);
    addEdgeModeBtn->setCheckable(true);
    selectModeBtn->setChecked(true);

    modeLayout->addWidget(selectModeBtn);
    modeLayout->addWidget(addNodeModeBtn);
    modeLayout->addWidget(addEdgeModeBtn);

    // 创建模式按钮组并连接信号
    auto* modeButtonGroup = new QButtonGroup(this);
    modeButtonGroup->addButton(selectModeBtn, 0);
    modeButtonGroup->addButton(addNodeModeBtn, 1);
    modeButtonGroup->addButton(addEdgeModeBtn, 2);

    connect(modeButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [this](int id) {
        switch(id) {
            case 0: m_canvas->setMode(GraphCanvas::SELECT); break;
            case 1: m_canvas->setMode(GraphCanvas::ADD_NODE); break;
            case 2: m_canvas->setMode(GraphCanvas::ADD_EDGE); break;
        }
    });

    leftLayout->addWidget(modeGroup);

    leftLayout->addStretch();

    // 显示设置放在左侧工具箱底部，避免悬浮在画布上遮挡内容
    auto* showGroup = new QGroupBox("显示设置", leftPanel);
    auto* showLayout = new QVBoxLayout(showGroup);

    // 权重显示按钮
    m_showWeightsBtn = new QPushButton("隐藏权重", showGroup);
    m_showWeightsBtn->setCheckable(true);
    m_showWeightsBtn->setChecked(true);

    // 坐标显示复选框
    auto* showCoordsCheck = new QCheckBox("显示节点坐标", showGroup);
    showCoordsCheck->setChecked(false);

    // 坐标轴显示复选框
    auto* showAxesCheck = new QCheckBox("显示坐标轴", showGroup);
    showAxesCheck->setChecked(false);

    connect(m_showWeightsBtn, &QPushButton::toggled, [this](bool checked) {
        if (m_canvas) {
            m_canvas->setShowWeights(checked);
            m_showWeightsBtn->setText(checked ? "隐藏权重" : "显示权重");
            m_infoPanel->append(QString("[%1] %2")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                               .arg(checked ? "显示边权重" : "隐藏边权重"));
        }
    });

    connect(showCoordsCheck, &QCheckBox::toggled, [this](bool checked) {
        if (m_canvas) {
            m_canvas->setShowCoordinates(checked);
            m_infoPanel->append(QString("[%1] %2节点坐标")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                               .arg(checked ? "显示" : "隐藏"));
        }
    });

    connect(showAxesCheck, &QCheckBox::toggled, [this](bool checked) {
        if (m_canvas) {
            m_canvas->setShowAxes(checked);
            m_infoPanel->append(QString("[%1] %2坐标轴")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                               .arg(checked ? "显示" : "隐藏"));
        }
    });

    showLayout->addWidget(m_showWeightsBtn);
    showLayout->addWidget(showCoordsCheck);
    showLayout->addWidget(showAxesCheck);
    leftLayout->addWidget(showGroup);

    // 学习系统入口与显示设置一起固定在左侧底部
    m_studyBtn = new QPushButton("打开学习系统", leftPanel);
    m_studyBtn->setObjectName("primaryButton");
    connect(m_studyBtn, &QPushButton::clicked, this, &MainWindow::onStudySystemClicked);
    leftLayout->addWidget(m_studyBtn);

    // ==================== 中央画布 ====================
    m_canvas = new GraphCanvas(this);
    m_canvas->setMinimumWidth(400);

    // 工作区保留统一留白，让画布与两侧栏形成清晰、克制的层级
    auto* canvasShell = new QWidget(centralWidget);
    canvasShell->setObjectName("workspace");
    auto* canvasLayout = new QVBoxLayout(canvasShell);
    canvasLayout->setContentsMargins(14, 14, 14, 14);
    canvasLayout->setSpacing(0);
    canvasLayout->addWidget(m_canvas);

    // 连接画布信号
    connect(m_canvas, &GraphCanvas::nodeSelected, this, &MainWindow::onNodeSelected);
    connect(m_canvas, &GraphCanvas::graphModified, [this]() {
        m_statusLabel->setText(QString("就绪 | 节点: %1 | 边: %2")
                          .arg(m_canvas->getNodeCount())
                          .arg(m_canvas->getEdgeCount()));
    });

    // 连接算法信号
    connect(m_canvas, &GraphCanvas::algorithmStarted, this, [this](const QString& algoName, int startNode) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        m_infoPanel->append(QString("[%1] 开始执行 %2，起始节点: %3")
                        .arg(time).arg(algoName).arg(startNode));
        m_infoPanel->append("----------------------------------------");

        updateAlgorithmButtons();
    });

    connect(m_canvas, &GraphCanvas::algorithmStep, this, [this](int step, const QString& info) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        m_infoPanel->append(QString("[%1] 步骤 %2: %3")
                           .arg(time).arg(step).arg(info));
    });

    connect(m_canvas, &GraphCanvas::algorithmPaused, this, [this]() {
        m_infoPanel->append(QString("[%1] 算法已暂停")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        updateAlgorithmButtons();
    });

    connect(m_canvas, &GraphCanvas::algorithmResumed, this, [this]() {
        m_infoPanel->append(QString("[%1] 算法继续执行")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        updateAlgorithmButtons();
    });

    connect(m_canvas, &GraphCanvas::algorithmFinished, this, [this](const QString& result) {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        m_infoPanel->append(QString("[%1] %2")
                           .arg(time).arg(result));
        m_infoPanel->append("========================================");

        updateAlgorithmButtons();
    });

    connect(m_canvas, &GraphCanvas::algorithmStateChanged, this, &MainWindow::updateAlgorithmButtons);

    // 连接算法状态信号
    connect(m_canvas, &GraphCanvas::algorithmStatusUpdated, this, [this](const QString& message) {
        m_statusLabel->setText(message);
    });

    // 连接算法进度信号
    connect(m_canvas, &GraphCanvas::algorithmProgressUpdated, this, [this](const QString& info) {
        m_infoPanel->append(QString("[%1] %2")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                        .arg(info));
    });

    // 连接悬停信号
    connect(m_canvas, &GraphCanvas::nodeHovered, this, [this](int nodeId, const QString& nodeInfo) {
        m_statusLabel->setText(QString("悬停节点 %1 | 坐标: %2")
                              .arg(nodeId).arg(nodeInfo.split("\n")[2] + ", " + nodeInfo.split("\n")[3]));
    });

    connect(m_canvas, &GraphCanvas::edgeHovered, this, [this](int from, int to, const QString& edgeInfo) {
        m_statusLabel->setText(QString("悬停边 %1 → %2 | 权重: %3")
                              .arg(from).arg(to).arg(edgeInfo.split("\n")[2]));
    });

    // ==================== 右侧面板 ====================
    auto* rightPanel = new QGroupBox("控制面板", this);
    rightPanel->setObjectName("rightSidebar");
    rightPanel->setFixedWidth(360);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(14, 16, 14, 14);
    rightLayout->setSpacing(8);

    // 算法选择
    auto* algoGroup = new QGroupBox("算法选择", rightPanel);
    auto* algoLayout = new QVBoxLayout(algoGroup);
    m_algorithmCombo = new QComboBox(algoGroup);
    m_algorithmCombo->addItems({"BFS遍历", "DFS遍历", "最短路径", "最小生成树"});
    connect(m_algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    algoLayout->addWidget(m_algorithmCombo);
    rightLayout->addWidget(algoGroup);

    // 速度控制
    auto* speedGroup = new QGroupBox("动画速度", rightPanel);
    auto* speedLayout = new QVBoxLayout(speedGroup);
    auto* speedSliderLayout = new QHBoxLayout();
    m_speedSlider = new QSlider(Qt::Horizontal, speedGroup);
    m_speedSlider->setRange(1, 10);
    m_speedSlider->setValue(5);

    speedSliderLayout->addWidget(new QLabel("慢"));
    speedSliderLayout->addWidget(m_speedSlider);
    speedSliderLayout->addWidget(new QLabel("快"));
    speedLayout->addLayout(speedSliderLayout);
    rightLayout->addWidget(speedGroup);

    // 在UI完全创建后连接滑块信号
    QMetaObject::invokeMethod(this, [this]() {
        connect(m_speedSlider, &QSlider::valueChanged, [this](int value) {
            if (m_canvas) {
                m_canvas->setAnimationSpeed(value);
                m_infoPanel->append(QString("[%1] 动画速度设置为: %2")
                                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                   .arg(value));
            }
        });
    }, Qt::QueuedConnection);

    // 算法控制按钮组
    auto* algoCtrlGroup = new QGroupBox("算法控制", rightPanel);
    auto* algoCtrlLayout = new QGridLayout(algoCtrlGroup);

    // 第一行：运行、暂停、继续
    m_runButton = new QPushButton("运行", algoCtrlGroup);
    m_runButton->setObjectName("primaryButton");
    m_pauseButton = new QPushButton("暂停", algoCtrlGroup);
    m_resumeButton = new QPushButton("继续", algoCtrlGroup);

    // 第二行：上一步、下一步、重置
    m_prevStepButton = new QPushButton("上一步", algoCtrlGroup);
    m_nextStepButton = new QPushButton("下一步", algoCtrlGroup);
    m_resetButton = new QPushButton("重置", algoCtrlGroup);

    algoCtrlLayout->addWidget(m_runButton, 0, 0);
    algoCtrlLayout->addWidget(m_pauseButton, 0, 1);
    algoCtrlLayout->addWidget(m_resumeButton, 0, 2);
    algoCtrlLayout->addWidget(m_prevStepButton, 1, 0);
    algoCtrlLayout->addWidget(m_nextStepButton, 1, 1);
    algoCtrlLayout->addWidget(m_resetButton, 1, 2);

    // 设置按钮最小尺寸
    QList<QPushButton*> algoButtons = {m_runButton, m_pauseButton, m_resumeButton,
                                       m_prevStepButton, m_nextStepButton, m_resetButton};
    for (auto* btn : algoButtons) {
        btn->setMinimumWidth(65);
    }

    // 连接算法控制按钮信号
    connect(m_runButton, &QPushButton::clicked, this, &MainWindow::onRunAlgorithm);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseAlgorithm);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeAlgorithm);
    connect(m_prevStepButton, &QPushButton::clicked, this, &MainWindow::onPreviousStep);
    connect(m_nextStepButton, &QPushButton::clicked, this, &MainWindow::onNextStep);
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onResetAlgorithm);

    rightLayout->addWidget(algoCtrlGroup);

    // 信息面板
    auto* infoGroup = new QGroupBox("操作说明与进度", rightPanel);
    auto* infoLayout = new QVBoxLayout(infoGroup);

    // 信息显示区域
    m_infoPanel = new QTextEdit(rightPanel);
    m_infoPanel->setObjectName("activityLog");
    m_infoPanel->setReadOnly(true);
    m_infoPanel->setPlainText(
        "==== TreeGraph Studio 使用说明 ====\n"
        "1. 选择图类型（有向/无向）\n"
        "2. 选择编辑模式:\n"
        "   - 选择模式: 点击节点运行动画\n"
        "   - 添加节点: 点击空白处添加\n"
        "   - 添加边: 依次点击两个节点\n"
        "3. 悬停功能:\n"
        "   - 鼠标悬停在节点上显示详细信息\n"
        "   - 鼠标悬停在边上显示详细信息\n"
        "4. 右键菜单:\n"
        "   - 右键节点: 删除节点/重命名\n"
        "   - 右键边: 编辑权重/删除边\n"
        "5. 双击边编辑权重\n"
        "6. 算法控制:\n"
        "   - 运行: 开始自动执行算法\n"
        "   - 暂停: 暂停算法执行\n"
        "   - 继续: 继续执行算法\n"
        "   - 上一步: 回退到上一步\n"
        "   - 下一步: 执行下一步\n"
        "   - 重置: 重置节点颜色\n"
        "================================\n"
    );

    // 按钮行布局
    auto* buttonLayout = new QHBoxLayout();

    // 清空按钮
    auto* clearInfoBtn = new QPushButton("清空", infoGroup);

    // 新增：扩展按钮
    auto* expandInfoBtn = new QPushButton("新窗口", infoGroup);

    buttonLayout->addWidget(clearInfoBtn);
    buttonLayout->addWidget(expandInfoBtn);

    connect(clearInfoBtn, &QPushButton::clicked, [this]() {
        m_infoPanel->clear();
        m_infoPanel->append(QString("[%1] 操作记录已清空")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        m_infoPanel->append("==== TreeGraph Studio ====");
    });

    // 新增：连接扩展按钮
    connect(expandInfoBtn, &QPushButton::clicked, [this]() {
        HelpWindow* helpWindow = new HelpWindow(this);
        helpWindow->setAttribute(Qt::WA_DeleteOnClose);

        // 实时绑定主窗口信息面板，自动同步
        helpWindow->setInfoPanel(m_infoPanel);

        helpWindow->show();
    });

    infoLayout->addWidget(m_infoPanel, 1);
    infoLayout->addLayout(buttonLayout);
    rightLayout->addWidget(infoGroup, 1);

    // ==================== 添加到主布局 ====================
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(canvasShell, 1);
    mainLayout->addWidget(rightPanel);

    // ==================== 菜单和状态栏 ====================
    createMenuBar();
    m_statusLabel = new QLabel("就绪 | 节点: 4 | 边: 3 | 无向图", this);
    m_statusLabel->setMinimumWidth(400);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->setStyleSheet("QStatusBar::item { border: 0px }");

    // 初始更新按钮状态
    updateAlgorithmButtons();
}

void MainWindow::createMenuBar() {
    auto* fileMenu = menuBar()->addMenu("文件(&F)");
    auto* saveAction = fileMenu->addAction("保存(&S)");
    auto* loadAction = fileMenu->addAction("加载(&L)");
    fileMenu->addSeparator();
    auto* exitAction = fileMenu->addAction("退出(&X)");

    connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoad);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto* viewMenu = menuBar()->addMenu("视图(&V)");
    auto* zoomInAction = viewMenu->addAction("放大(&I)");
    auto* zoomOutAction = viewMenu->addAction("缩小(&O)");
    auto* resetViewAction = viewMenu->addAction("重置视图(&R)");
    viewMenu->addSeparator();
    auto* darkModeAction = viewMenu->addAction("深色模式(&D)");
    darkModeAction->setCheckable(true);

    connect(zoomInAction, &QAction::triggered, [this](){
        if (m_canvas) {
            m_canvas->scale(1.2, 1.2);
            m_infoPanel->append(QString("[%1] 视图放大")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        }
    });

    connect(zoomOutAction, &QAction::triggered, [this](){
        if (m_canvas) {
            m_canvas->scale(0.8, 0.8);
            m_infoPanel->append(QString("[%1] 视图缩小")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        }
    });

    connect(resetViewAction, &QAction::triggered, [this](){
        if (m_canvas) {
            m_canvas->resetViewTransform();
            m_canvas->centerGraphAnimated(true);
            m_infoPanel->append(QString("[%1] 视图重置")
                               .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        }
    });

    connect(darkModeAction, &QAction::toggled, this, [this](bool checked) {
        applyTheme(checked);
        m_infoPanel->append(QString("[%1] 已切换至%2模式")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                           .arg(checked ? "深色" : "浅色"));
    });
    darkModeAction->setChecked(QSettings().value("appearance/darkMode", false).toBool());

    auto* helpMenu = menuBar()->addMenu("帮助(&H)");
    auto* helpAction = helpMenu->addAction("详细帮助(&H)");
    auto* aboutAction = helpMenu->addAction("关于(&A)");

    // 连接详细帮助
    connect(helpAction, &QAction::triggered, [this](){
        HelpWindow* helpWindow = new HelpWindow(this);
        helpWindow->setAttribute(Qt::WA_DeleteOnClose);
        helpWindow->showManual();
        helpWindow->show();
    });

    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::about(this, "关于",
            "TreeGraph Studio\n"
            "C++课程设计项目 - 图算法教学系统\n\n"
            "功能特点:\n"
            "1. 支持有向/无向图、树的创建和编辑\n"
            "2. 可视化图算法执行过程\n"
            "3. 悬停显示节点/边详细信息\n"
            "4. 右键菜单快捷操作\n"
            "5. 练习系统和学习进度跟踪\n"
            "6. 坐标轴显示和节点坐标定位\n"
            "7. 完善的算法控制功能\n\n"
            "作者: 学生项目\n"
        );
    });
}

void MainWindow::applyTheme(bool dark) {
    QFile qss(dark ? ":/themes/dark.qss" : ":/themes/light.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(qss.readAll()));
    }
    if (m_canvas) m_canvas->setDarkMode(dark);
    QSettings().setValue("appearance/darkMode", dark);

#ifdef Q_OS_WIN
    // 保留原生窗口按钮，同时让标题栏、文字与边线真正跟随主题
    HWND hwnd = reinterpret_cast<HWND>(winId());
    BOOL immersiveDark = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, 20, &immersiveDark, sizeof(immersiveDark));
    const COLORREF captionColor = dark ? RGB(32, 32, 36) : RGB(251, 251, 252);
    const COLORREF textColor = dark ? RGB(236, 235, 239) : RGB(38, 37, 42);
    const COLORREF borderColor = dark ? RGB(52, 51, 58) : RGB(222, 221, 226);
    DwmSetWindowAttribute(hwnd, 35, &captionColor, sizeof(captionColor));
    DwmSetWindowAttribute(hwnd, 36, &textColor, sizeof(textColor));
    DwmSetWindowAttribute(hwnd, 34, &borderColor, sizeof(borderColor));
#endif
}

void MainWindow::updateAlgorithmButtons() {
    if (!m_canvas) return;

    GraphCanvas::AlgorithmState state = m_canvas->getAlgorithmState();

    switch(state) {
        case GraphCanvas::IDLE:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_resumeButton->setEnabled(false);
            m_prevStepButton->setEnabled(false);
            m_nextStepButton->setEnabled(false);
            m_resetButton->setEnabled(false);
            m_runButton->setText("运行");
            break;

        case GraphCanvas::RUNNING:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(true);
            m_resumeButton->setEnabled(false);
            m_prevStepButton->setEnabled(true);
            m_nextStepButton->setEnabled(true);
            m_resetButton->setEnabled(true);
            m_runButton->setText("停止");
            break;

        case GraphCanvas::PAUSED:
            m_runButton->setEnabled(true);
            m_pauseButton->setEnabled(false);
            m_resumeButton->setEnabled(true);
            m_prevStepButton->setEnabled(true);
            m_nextStepButton->setEnabled(true);
            m_resetButton->setEnabled(true);
            m_runButton->setText("继续运行");
            break;
    }
}

void MainWindow::onNewGraph() {
    if (!m_canvas) return;

    m_canvas->setGraphType(GraphCanvas::UNDIRECTED_GRAPH);
    m_canvas->clear();

    // 更新按钮状态
    m_newGraphBtn->setChecked(true);
    m_newTreeBtn->setChecked(false);



    m_statusLabel->setText("就绪 | 节点: 0 | 边: 0 | 无向图");
    m_infoPanel->append(QString("[%1] 已创建无向图")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    updateAlgorithmButtons();
}

void MainWindow::onNewTree() {
    if (!m_canvas) return;

    m_canvas->setGraphType(GraphCanvas::TREE);
    m_canvas->clear();

    // 更新按钮状态
    m_newTreeBtn->setChecked(true);
    m_newGraphBtn->setChecked(false);



    m_statusLabel->setText("就绪 | 节点: 0 | 边: 0 | 树");
    m_infoPanel->append(QString("[%1] 已创建树")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    updateAlgorithmButtons();
}

void MainWindow::onSave() {
    if (!m_canvas) return;

    QString fileName = QFileDialog::getSaveFileName(this, "保存图", "", "Graph Files (*.graph)");
    if (fileName.isEmpty()) return;
    if (m_canvas->saveGraph(fileName)) {
        m_infoPanel->append(QString("[%1] 保存成功: %2")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                           .arg(fileName));
    } else {
        QMessageBox::warning(this, "错误", "保存失败！");
    }
}

void MainWindow::onLoad() {
    if (!m_canvas) return;

    QString fileName = QFileDialog::getOpenFileName(this, "加载图", "", "Graph Files (*.graph)");
    if (fileName.isEmpty()) return;

    m_statusLabel->setText("正在加载文件...");
    QApplication::processEvents(); // 刷新UI

    if (m_canvas->loadGraph(fileName)) {
        m_canvas->centerGraphAnimated(true);
        m_infoPanel->append(QString("[%1] 加载成功: %2")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                           .arg(fileName));
        m_statusLabel->setText(QString("就绪 | 节点: %1 | 边: %2").arg(m_canvas->getNodeCount()).arg(m_canvas->getEdgeCount()));
    } else {
        QMessageBox::warning(this, "错误", "加载失败！");
        m_statusLabel->setText("加载失败");
    }
}

void MainWindow::onGraphTypeChanged(int type) {
    if (!m_canvas) return;

    if (type == 0) {
        // 无向图
        m_canvas->setGraphType(GraphCanvas::UNDIRECTED_GRAPH);

        // 更新按钮状态
        m_newGraphBtn->setChecked(true);
        m_newTreeBtn->setChecked(false);



        m_statusLabel->setText("就绪 | 节点: 0 | 边: 0 | 无向图");
        m_infoPanel->append(QString("[%1] 切换到无向图模式")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    } else {
        // 有向图
        m_canvas->setGraphType(GraphCanvas::DIRECTED_GRAPH);

        // 更新按钮状态（有向图使用新建图按钮）
        m_newGraphBtn->setChecked(true);
        m_newTreeBtn->setChecked(false);



        m_statusLabel->setText("就绪 | 节点: 0 | 边: 0 | 有向图");
        m_infoPanel->append(QString("[%1] 切换到有向图模式（带箭头）")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    }

    m_canvas->clear();
    updateAlgorithmButtons();
}

void MainWindow::onRunAlgorithm() {
    if (!m_canvas) return;

    GraphCanvas::AlgorithmState state = m_canvas->getAlgorithmState();

    // 用户开始观察算法时，如果图形没有完整出现在视口中，自动平滑展示全图
    if (state == GraphCanvas::IDLE) m_canvas->ensureGraphVisibleAnimated();

    if (state == GraphCanvas::RUNNING) {
        // 如果正在运行，则暂停
        m_canvas->pauseAlgorithm();
    } else {
        if (state == GraphCanvas::IDLE) {
            int nodeCount = m_canvas->getNodeCount();
            if (nodeCount == 0) {
                QMessageBox::warning(this, "错误", "图中没有节点！");
                return;
            }

            // 获取算法类型
            QString algoText = m_algorithmCombo->currentText();
            GraphCanvas::AlgorithmType algoType;

            if (algoText == "BFS遍历") {
                algoType = GraphCanvas::BFS;
            } else if (algoText == "DFS遍历") {
                algoType = GraphCanvas::DFS;
            } else if (algoText == "最短路径") {
                algoType = GraphCanvas::SHORTEST_PATH;
            } else if (algoText == "最小生成树") {
                algoType = GraphCanvas::MST;
            } else {
                algoType = GraphCanvas::BFS;
            }

            // 对于最短路径算法，需要起点和终点
            int startNodeId = -1;
            int targetNodeId = -1;

            if (algoType == GraphCanvas::SHORTEST_PATH) {
                // 获取当前所有节点的名称列表
                QStringList nodeNames;
                for (int i = 0; i < nodeCount; ++i) {
                    if (m_canvas->hasNode(i)) {
                        nodeNames.append(m_canvas->getNodeName(i));
                    }
                }

                if (nodeNames.size() < 2) {
                    QMessageBox::warning(this, "错误", "最短路径需要至少两个节点！");
                    return;
                }

                // 先选择起点
                bool ok;
                QString startNodeName = QInputDialog::getItem(this, "选择起点",
                                                              "请选择起点节点:",
                                                              nodeNames, 0, false, &ok);
                if (!ok || startNodeName.isEmpty()) return;

                startNodeId = m_canvas->getNodeIdByName(startNodeName);
                if (startNodeId == -1) {
                    QMessageBox::warning(this, "错误", QString("节点 '%1' 不存在！").arg(startNodeName));
                    return;
                }

                // 再选择终点（排除起点）
                QStringList targetNodeNames = nodeNames;
                targetNodeNames.removeAll(startNodeName);

                QString targetNodeName = QInputDialog::getItem(this, "选择终点",
                                                               "请选择终点节点:",
                                                               targetNodeNames, 0, false, &ok);
                if (!ok || targetNodeName.isEmpty()) return;

                targetNodeId = m_canvas->getNodeIdByName(targetNodeName);
                if (targetNodeId == -1) {
                    QMessageBox::warning(this, "错误", QString("节点 '%1' 不存在！").arg(targetNodeName));
                    return;
                }

                m_infoPanel->append(QString("[%1] 最短路径算法：从 %2 到 %3")
                                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                   .arg(startNodeName).arg(targetNodeName));
            } else if (algoType != GraphCanvas::MST) {
                // 其他算法只需要起点
                if (m_lastSelectedNode != -1 && m_canvas->hasNode(m_lastSelectedNode)) {
                    startNodeId = m_lastSelectedNode;
                    QString nodeName = m_canvas->getNodeName(startNodeId);
                    m_infoPanel->append(QString("[%1] 使用选中的节点 %2 (%3) 作为起始节点")
                                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                       .arg(nodeName).arg(startNodeId));
                } else {
                    // 弹窗询问起始节点
                    QStringList nodeNames;
                    for (int i = 0; i < nodeCount; ++i) {
                        if (m_canvas->hasNode(i)) {
                            nodeNames.append(m_canvas->getNodeName(i));
                        }
                    }

                    bool ok;
                    QString nodeName = QInputDialog::getItem(this, "选择起始节点",
                                                            "请选择起始节点:",
                                                            nodeNames, 0, false, &ok);
                    if (!ok || nodeName.isEmpty()) return;

                    startNodeId = m_canvas->getNodeIdByName(nodeName);
                    if (startNodeId == -1) {
                        QMessageBox::warning(this, "错误", QString("节点 '%1' 不存在！").arg(nodeName));
                        return;
                    }
                }
            }

            // 调用算法
            if (algoType == GraphCanvas::SHORTEST_PATH) {
                m_canvas->startShortestPath(startNodeId, targetNodeId);
            } else {
                m_canvas->startAlgorithm(startNodeId, algoType);
            }
        } else if (state == GraphCanvas::PAUSED) {
            m_canvas->resumeAlgorithm();
        }
    }
    updateAlgorithmButtons();
}

void MainWindow::onPauseAlgorithm() {
    if (!m_canvas) return;

    m_canvas->pauseAlgorithm();
    updateAlgorithmButtons();
}

void MainWindow::onResumeAlgorithm() {
    if (!m_canvas) return;

    m_canvas->resumeAlgorithm();
    updateAlgorithmButtons();
}

void MainWindow::onPreviousStep() {
    if (!m_canvas) return;

    m_canvas->previousStep();
    updateAlgorithmButtons();
}

void MainWindow::onNextStep() {
    if (!m_canvas) return;

    m_canvas->nextStep();
    updateAlgorithmButtons();
}

void MainWindow::onResetAlgorithm() {
    if (!m_canvas) return;

    m_canvas->resetAlgorithm();
    m_infoPanel->append(QString("[%1] 算法已重置")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    updateAlgorithmButtons();
}

void MainWindow::onReset() {
    if (!m_canvas) return;

    m_canvas->clear();
    m_infoPanel->append(QString("[%1] 画布已清空")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    m_statusLabel->setText("就绪 | 节点: 0 | 边: 0");
    updateAlgorithmButtons();
}

void MainWindow::onAlgorithmChanged(int index) {
    QString algoName = m_algorithmCombo->itemText(index);

    // 检查当前算法状态
    if (m_canvas->getAlgorithmState() != GraphCanvas::IDLE) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "切换算法",
            "当前算法正在执行，切换算法将重置当前进度。确定要切换吗？",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_canvas->resetAlgorithm();
        } else {
            // 恢复原来的选择
            m_algorithmCombo->blockSignals(true);
            m_algorithmCombo->setCurrentIndex(m_algorithmCombo->currentIndex());
            m_algorithmCombo->blockSignals(false);
            return;
        }
    }

    m_infoPanel->append(QString("[%1] 已选择算法: %2")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                       .arg(algoName));
}

void MainWindow::onNodeSelected(int nodeId) {

    // 获取节点名称
    QString nodeName = m_canvas->getNodeName(nodeId);
    m_statusLabel->setText(QString("就绪 | 选中节点: %1 (%2)").arg(nodeName).arg(nodeId));
    m_lastSelectedNode = nodeId; // 记录选中的节点

    // 仅显示提示信息，不自动开始算法
    m_infoPanel->append(QString("[%1] 已选中节点 %2 (%3)，请点击运行按钮开始算法")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                       .arg(nodeName).arg(nodeId));
}

void MainWindow::onStudySystemClicked() {
    if (!m_studyWindow) {
        m_studyWindow = new StudyWindow(this);
        m_studyWindow->setAttribute(Qt::WA_DeleteOnClose);

        // 窗口销毁时自动置空指针（防止野指针）
        connect(m_studyWindow, &QObject::destroyed, this, [this]() {
            m_studyWindow = nullptr;
        });

        // 加载图后不关闭窗口
        connect(m_studyWindow, &StudyWindow::loadGraphRequest,
                this, [this](const QString& filePath) {
                    if (m_canvas->loadGraph(filePath)) {
                        m_canvas->centerGraphAnimated(true);
                        m_infoPanel->append(QString("[%1] 从学习系统加载图: %2")
                                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                           .arg(QFileInfo(filePath).fileName()));
                        m_statusLabel->setText(QString("就绪 | 节点: %1 | 边: %2")
                                              .arg(m_canvas->getNodeCount())
                                              .arg(m_canvas->getEdgeCount()));
                    } else {
                        QMessageBox::warning(this, "错误", "加载图文件失败！");
                    }
                });
    }

    m_studyWindow->show();
    m_studyWindow->raise();
    m_studyWindow->activateWindow();
}

void MainWindow::updateGraphTypeButtons() {
    // 图/树按钮的选中态统一由全局 QSS 的 QPushButton:checked 呈现，无需再手动设置内联样式
}

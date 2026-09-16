#include "StudyWindow.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

StudyWindow::StudyWindow(QWidget* parent)
    : QDialog(parent) {
    
    setWindowTitle("学习系统 - TreeGraph Studio");
    setMinimumSize(900, 700);
    resize(1000, 750);
    
    setupUI();
}

void StudyWindow::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 标题
    auto* titleLabel = new QLabel("TreeGraph Studio 学习系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("studyTitle");
    mainLayout->addWidget(titleLabel);
    
    // 标签页
    m_tabWidget = new QTabWidget(this);
    
    // 算法学习标签页（文档）
    m_docViewer = new DocumentViewer(this);
    m_tabWidget->addTab(m_docViewer, "📄 算法学习");
    
    // 算法实践标签页（图文件）
    m_graphPractice = new GraphPracticeWidget(this);
    m_tabWidget->addTab(m_graphPractice, "🕸️ 算法实践");
    
    // 视频学习标签页
    m_videoPlayer = new VideoPlayerWidget(this);
    m_tabWidget->addTab(m_videoPlayer, "🎬 视频学习");
    
    // 连接图加载信号
    connect(m_graphPractice, &GraphPracticeWidget::loadGraphRequest,
            this, &StudyWindow::loadGraphRequest);
    
    mainLayout->addWidget(m_tabWidget);
    
    // 底部按钮
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    auto* closeBtn = new QPushButton("关闭", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeBtn);
    
    mainLayout->addLayout(buttonLayout);
}

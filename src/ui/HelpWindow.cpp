#include "HelpWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollBar>
#include <QTimer>  

HelpWindow::HelpWindow(QWidget* parent) : QDialog(parent),
    m_textEdit(nullptr),
    m_infoPanel(nullptr),
    m_scrollTimer(nullptr)  //初始化为nullptr
{
    setWindowTitle("详细操作说明");
    setMinimumSize(800, 600);

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont("Consolas", 10));
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);

    // 创建滚动定时器
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(true);
    m_scrollTimer->setInterval(100);

    connect(m_scrollTimer, &QTimer::timeout, this, [this]() {
        if (m_textEdit) {
            QScrollBar* vScrollBar = m_textEdit->verticalScrollBar();
            if (vScrollBar) {
                vScrollBar->setValue(vScrollBar->maximum());
            }
        }
    });

    QPushButton* closeBtn = new QPushButton("关闭", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

    layout->addWidget(m_textEdit);
    layout->addWidget(closeBtn);
}

void HelpWindow::setInfoPanel(QTextEdit* infoPanel) {
    m_infoPanel = infoPanel;
    if (!m_infoPanel) return;

    // 初始同步
    QString text = m_infoPanel->toPlainText();
    m_textEdit->setPlainText(text);
    
    // 滚动到底部
    if (m_textEdit) {
        QTimer::singleShot(50, this, [this]() {
            QScrollBar* vScrollBar = m_textEdit->verticalScrollBar();
            if (vScrollBar) {
                vScrollBar->setValue(vScrollBar->maximum());
            }
        });
    }

    // 连接信号实现实时同步
    connect(m_infoPanel, &QTextEdit::textChanged, 
            this, &HelpWindow::syncWithInfoPanel);
}

void HelpWindow::syncWithInfoPanel() {
    if (m_infoPanel && m_textEdit) {
        // 保存当前滚动位置
        int oldScrollPos = m_textEdit->verticalScrollBar()->value();
        bool atBottom = oldScrollPos >= m_textEdit->verticalScrollBar()->maximum() - 10;
        
        // 更新文本
        m_textEdit->setPlainText(m_infoPanel->toPlainText());
        
        // 如果之前已经在底部，就滚动到底部
        if (atBottom) {
            m_scrollTimer->start();  // 延迟滚动，确保文本已更新
        }
    }
}
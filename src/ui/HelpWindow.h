#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QTimer>

class HelpWindow : public QDialog {
    Q_OBJECT

public:
    explicit HelpWindow(QWidget* parent = nullptr);
    
    // 绑定主窗口信息面板，实现实时同步
    void setInfoPanel(QTextEdit* infoPanel);

private slots:
    void syncWithInfoPanel();  // 同步槽函数

private:
    QTextEdit* m_textEdit;
    QTextEdit* m_infoPanel;  // 保存主窗口面板指针
    QTimer* m_scrollTimer;   // 自动滚动定时器
};
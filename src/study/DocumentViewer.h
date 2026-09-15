#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QFile>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

/**
 * @brief 文档查看器 - 支持MD/TXT格式，带搜索导航
 */
class DocumentViewer : public QWidget {
    Q_OBJECT

public:
    explicit DocumentViewer(QWidget* parent = nullptr);
    ~DocumentViewer() = default;

    bool loadDocument(const QString& filePath);
    void clear();

public slots:
    void zoomIn();
    void zoomOut();
    void searchText(const QString& text);  // 修改为执行搜索

private slots:
    void onOpenDocument();
    void updateStatus();
    void onPreviousResult();  // 上一个结果
    void onNextResult();      // 下一个结果

private:
    void setupUI();
    void setupToolBar();
    QString markdownToHtml(const QString& markdown);
    QString enhancedMarkdownToHtml(const QString& markdown);
    void renderDocument(const QString& content, const QString& fileExt);
    
    // 搜索相关函数
    void highlightSearchResults();
    void navigateToResult(int index);

    // UI组件
    QPushButton* m_openBtn;
    QPushButton* m_zoomInBtn;
    QPushButton* m_zoomOutBtn;
    QLineEdit* m_searchEdit;
    
    // 搜索导航按钮
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QLabel* m_searchInfoLabel;
    
    QTextEdit* m_textEdit;
    QLabel* m_statusLabel;
    
    // 数据
    QString m_currentFilePath;
    int m_fontSize;
    
    // 搜索结果
    QVector<QTextCursor> m_searchResults;
    int m_currentSearchIndex;

    // 搜索结果高亮颜色
    const QColor MATCH_HIGHLIGHT_COLOR = QColor(255, 255, 0, 150);      // 黄色：所有匹配
    const QColor CURRENT_MATCH_COLOR = QColor(255, 165, 0, 200);        // 橙色：当前匹配
};
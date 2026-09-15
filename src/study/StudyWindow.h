#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include "DocumentViewer.h"
#include "GraphPracticeWidget.h"
#include "VideoPlayerWidget.h" 

/**
 * @brief 学习系统主窗口
 */
class StudyWindow : public QDialog {
    Q_OBJECT

public:
    explicit StudyWindow(QWidget* parent = nullptr);
    ~StudyWindow() = default;

    DocumentViewer* documentViewer() const { return m_docViewer; }
    GraphPracticeWidget* graphPracticeWidget() const { return m_graphPractice; }
    VideoPlayerWidget* videoPlayer() const { return m_videoPlayer; }

signals:
    void loadGraphRequest(const QString& filePath);

private:
    void setupUI();

    QTabWidget* m_tabWidget;
    DocumentViewer* m_docViewer;
    GraphPracticeWidget* m_graphPractice;
    VideoPlayerWidget* m_videoPlayer;
};
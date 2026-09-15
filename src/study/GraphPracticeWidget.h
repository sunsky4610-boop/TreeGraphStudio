#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "JsonSerializer.h"

/**
 * @brief 图实践组件 - 浏览和加载图文件
 */
class GraphPracticeWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphPracticeWidget(QWidget* parent = nullptr);
    ~GraphPracticeWidget() = default;

signals:
    // 请求加载图文件到主窗口
    void loadGraphRequest(const QString& filePath);

public slots:
    // 刷新文件列表
    void refreshFileList();

private slots:
    // 选择图文件目录
    void onSelectDirectory();
    
    // 加载选中的图
    void onLoadSelectedGraph();
    
    // 文件列表双击
    void onFileDoubleClicked(QListWidgetItem* item);
    
    // 选中文件变化
    void onFileSelectionChanged();

private:
    void setupUI();
    QString formatFileInfo(const QFileInfo& info);
    QJsonObject peekGraphInfo(const QString& filePath);
    
    // UI组件
    QListWidget* m_fileList;
    QPushButton* m_selectDirBtn;
    QPushButton* m_loadBtn;
    QLabel* m_previewLabel;
    
    // 数据
    QString m_currentDirectory;
    QMap<QString, QJsonObject> m_graphInfoCache; // 缓存图信息
};
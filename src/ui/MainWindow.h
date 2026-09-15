#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QButtonGroup>
#include <QDateTime>
#include "GraphCanvas.h"
#include "../study/StudyWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onNewGraph();
    void onNewTree();
    void onSave();
    void onLoad();
    void onRunAlgorithm();
    void onPauseAlgorithm();
    void onResumeAlgorithm();
    void onPreviousStep();
    void onNextStep();
    void onResetAlgorithm();
    void onReset();
    void onAlgorithmChanged(int index);
    void onNodeSelected(int nodeId);

    // 学习系统入口
    void onStudySystemClicked();
    
    void onGraphTypeChanged(int type);

private:
    void setupUI();
    void createMenuBar();
    void updateAlgorithmButtons();
    void updateGraphTypeButtons();

    QButtonGroup* m_graphTypeButtonGroup;

    GraphCanvas* m_canvas;
    QComboBox* m_algorithmCombo;
    QSlider* m_speedSlider;
    QPushButton* m_runButton;
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    QPushButton* m_prevStepButton;
    QPushButton* m_nextStepButton;
    QPushButton* m_resetButton;
    QTextEdit* m_infoPanel;
    QLabel* m_statusLabel;
    
    // 学习系统按钮
    QPushButton* m_studyBtn;
    
    // 学习系统窗口指针
    StudyWindow* m_studyWindow;
    
    QButtonGroup* m_graphTypeGroup;
    QPushButton* m_showWeightsBtn;
    QPushButton* m_newGraphBtn;
    QPushButton* m_newTreeBtn;
    int m_lastSelectedNode{-1};

    bool m_isRunning{false};

};
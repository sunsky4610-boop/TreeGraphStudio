#include "DocumentViewer.h"
#include "utils/PathUtils.h"
#include <QRegularExpression> 
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QScrollBar>
#include <QCoreApplication>
#include <QDebug>  // 添加调试

DocumentViewer::DocumentViewer(QWidget* parent)
    : QWidget(parent), m_fontSize(11), m_currentSearchIndex(-1) {
    
    setupUI();
    onCourseChanged(0);
}

void DocumentViewer::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 工具栏
    setupToolBar();
    mainLayout->addWidget(m_openBtn->parentWidget());
    
    // 文档显示区
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont("Consolas", m_fontSize));
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_textEdit->setObjectName("learningDocument");
    
    m_textEdit->setAcceptRichText(true);
    m_textEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QTextDocument* doc = m_textEdit->document();
    doc->setDocumentMargin(20);
    
    mainLayout->addWidget(m_textEdit);
    
    // 状态栏
    m_statusLabel = new QLabel("就绪 | 未加载文档", this);
    m_statusLabel->setObjectName("learningStatus");
    mainLayout->addWidget(m_statusLabel);
}

void DocumentViewer::setupToolBar() {
    auto* toolBar = new QWidget(this);
    auto* layout = new QHBoxLayout(toolBar);
    layout->setContentsMargins(5, 5, 5, 5);
    
    m_openBtn = new QPushButton("📂 打开文档", toolBar);
    m_openBtn->setStyleSheet("padding: 5px 15px;");
    connect(m_openBtn, &QPushButton::clicked, this, &DocumentViewer::onOpenDocument);

    m_courseCombo = new QComboBox(toolBar);
    m_courseCombo->setMinimumWidth(230);
    m_courseCombo->addItem("BFS · 广度优先搜索", ":/learning/documents/BFS.md");
    m_courseCombo->addItem("DFS · 深度优先搜索", ":/learning/documents/DFS.md");
    m_courseCombo->addItem("Dijkstra · 最短路径", ":/learning/documents/Dijkstra.md");
    m_courseCombo->addItem("Kruskal · 最小生成树", ":/learning/documents/Kruskal.md");
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DocumentViewer::onCourseChanged);
    
    m_zoomInBtn = new QPushButton("A+", toolBar);
    m_zoomInBtn->setToolTip("放大字体");
    m_zoomInBtn->setMaximumWidth(40);
    connect(m_zoomInBtn, &QPushButton::clicked, this, &DocumentViewer::zoomIn);
    
    m_zoomOutBtn = new QPushButton("A-", toolBar);
    m_zoomOutBtn->setToolTip("缩小字体");
    m_zoomOutBtn->setMaximumWidth(40);
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &DocumentViewer::zoomOut);
    
    // 搜索框
    m_searchEdit = new QLineEdit(toolBar);
    m_searchEdit->setPlaceholderText("🔍 搜索...");
    m_searchEdit->setMaximumWidth(200);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DocumentViewer::searchText);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &DocumentViewer::onNextResult);
    
    // 搜索结果导航按钮
    m_prevBtn = new QPushButton("⬆️ 上一个", toolBar);
    m_prevBtn->setMaximumWidth(80);
    m_prevBtn->setEnabled(false);
    connect(m_prevBtn, &QPushButton::clicked, this, &DocumentViewer::onPreviousResult);
    
    m_nextBtn = new QPushButton("⬇️ 下一个", toolBar);
    m_nextBtn->setMaximumWidth(80);
    m_nextBtn->setEnabled(false);
    connect(m_nextBtn, &QPushButton::clicked, this, &DocumentViewer::onNextResult);
    
    // 搜索结果信息标签
    m_searchInfoLabel = new QLabel("", toolBar);
    m_searchInfoLabel->setStyleSheet("color: #666; font-size: 12px;");
    m_searchInfoLabel->setMaximumWidth(120);
    
    layout->addWidget(m_openBtn);
    layout->addWidget(m_courseCombo);
    layout->addStretch();
    layout->addWidget(new QLabel("字体:", toolBar));
    layout->addWidget(m_zoomOutBtn);
    layout->addWidget(m_zoomInBtn);
    layout->addSpacing(20);
    
    // 搜索区域
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_prevBtn);
    layout->addWidget(m_nextBtn);
    layout->addWidget(m_searchInfoLabel);
}

void DocumentViewer::onOpenDocument() {
    QString defaultDir = PathUtils::getLearningResourcePath("documents");
    
    // 不区分大小写的文件过滤器
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "打开学习文档",
        defaultDir,
        "支持格式 (*.md *.MD *.txt *.TXT);;"
        "Markdown (*.md *.MD);;"
        "文本文件 (*.txt *.TXT)"
    );
    
    if (filePath.isEmpty()) return;
    
    loadDocument(filePath);
}

bool DocumentViewer::loadDocument(const QString& filePath) {
    m_currentFilePath = filePath;
    
    QString fileExt = QFileInfo(filePath).suffix().toLower();
    if (fileExt != "md" && fileExt != "txt") {
        QMessageBox::warning(this, "不支持的格式", 
            "仅支持 Markdown (.md) 和文本 (.txt) 文件。\n\n"
            "如需查看Word文档，请先转换为Markdown格式：\n"
            "1. 在Word中打开文档\n"
            "2. 另存为 → 选择 .txt 或复制内容到.md文件\n"
            "3. 重新加载转换后的文件");
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "错误", QString("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    
    QString content = file.readAll();
    file.close();
    
    renderDocument(content, fileExt);
    updateStatus();
    return true;
}

void DocumentViewer::renderDocument(const QString& content, const QString& fileExt) {
    if (fileExt == "md") {
        // 直接调用 markdownToHtml
        QString html = markdownToHtml(content);
        m_textEdit->setHtml(html);
    } else {
        m_textEdit->setPlainText(content);
    }
}

// Markdown解析器，支持表格、代码块、嵌套列表等
QString DocumentViewer::markdownToHtml(const QString& markdown) {
    QString html = markdown;
    
    // ==================== 阶段0: 预处理 - 转义HTML特殊字符（除了代码块内） ====================
    
    // 阶段1: 处理围栏代码块（```code```）- 最高优先级
    {
        // 更精确的正则，支持指定语言
        QRegularExpression re("```(\\w+)?\\n(.*?)\\n```", 
            QRegularExpression::DotMatchesEverythingOption | 
            QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        
        QVector<QPair<int, int>> positions;
        QVector<QString> codes;
        QVector<QString> languages;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            languages.append(match.captured(1)); // 语言类型
            QString code = match.captured(2).toHtmlEscaped();
            codes.append(code);
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            int start = positions[i].first;
            int end = positions[i].second;
            
            // 生成语法高亮类名（如有需要后续可扩展）
            QString langClass = languages[i].isEmpty() ? "" : 
                QString(" class='language-%1'").arg(languages[i]);
            
            QString replacement = QString(
                "<pre style='background: #f5f5f5; padding: 12px; border-radius: 5px; "
                "font-family: 'Consolas', 'Monaco', monospace; font-size: 11px; "
                "white-space: pre-wrap; overflow-x: auto; border: 1px solid #ddd; margin: 10px 0;'>"
                "<code%1>%2</code></pre>"
            ).arg(langClass).arg(codes[i]);
            
            html = html.replace(start, end - start, replacement);
        }
    }
    
    // 阶段2: 处理缩进代码块（4个空格或tab）
    {
        QRegularExpression re("(?:\\n|^)((?:[ ]{4}|\\t)(?:.*\\n)+)+");
        re.setPatternOptions(QRegularExpression::MultilineOption);
        
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        QVector<QPair<int, int>> positions;
        QVector<QString> codes;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            
            // 移除缩进并转义HTML
            QString code = match.captured(1);
            code.remove(QRegularExpression("^(?:[ ]{4}|\\t)", QRegularExpression::MultilineOption));
            codes.append(code.toHtmlEscaped());
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            int start = positions[i].first;
            int end = positions[i].second;
            
            QString replacement = QString(
                "<pre style='background: #f8f8f8; padding: 10px; border-radius: 5px; "
                "font-family: monospace; font-size: 11px; "
                "white-space: pre-wrap; border: 1px solid #e0e0e0; margin: 8px 0;'>%1</pre>"
            ).arg(codes[i]);
            
            html = html.replace(start, end - start, replacement);
        }
    }
    
    // 阶段3: 处理行内代码
    {
        // 改进正则，支持反引号内的转义
        QRegularExpression re("`+([^`]+?)`+");
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        
        QVector<QPair<int, int>> positions;
        QVector<QString> codes;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            QString code = match.captured(1).toHtmlEscaped();
            codes.append(code);
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            int start = positions[i].first;
            int end = positions[i].second;
            QString replacement = QString(
                "<code style='background: #e8e8e8; padding: 2px 6px; border-radius: 3px; "
                "font-family: monospace; font-size: 0.9em; color: #d63384; "
                "border: 1px solid #ddd;'>%1</code>"
            ).arg(codes[i]);
            html = html.replace(start, end - start, replacement);
        }
    }
    
    // 阶段4: 处理表格（保持原有逻辑，但增强样式）
    {
        QRegularExpression re("((?:\\|[^\\n]*)+\\n(?:\\|?\\s*[:-]+[-:]*\\|?\\s*)+\\n(?:\\|[^\\n]*\\n)+)");
        re.setPatternOptions(QRegularExpression::MultilineOption);
        
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        QVector<QPair<int, int>> positions;
        QVector<QString> tables;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            
            QStringList lines = match.captured(1).split('\n', Qt::SkipEmptyParts);
            if (lines.size() >= 3) {
                QString tableHtml = "<table style='border-collapse: collapse; width: 100%; margin: 15px 0; border: 1px solid #ddd; font-size: 13px;'>";
                
                // 表头
                QString header = lines[0].trimmed();
                if (header.startsWith('|')) header = header.mid(1);
                if (header.endsWith('|')) header.chop(1);
                QStringList headers = header.split('|');
                
                tableHtml += "<thead><tr style='background: #f5f5f5;'>";
                for (const QString& h : headers) {
                    tableHtml += QString("<th style='border: 1px solid #ddd; padding: 10px 8px; text-align: left; font-weight: bold;'>%1</th>")
                        .arg(h.trimmed());
                }
                tableHtml += "</tr></thead><tbody>";
                
                // 数据行
                for (int i = 2; i < lines.size(); ++i) {
                    QString row = lines[i].trimmed();
                    if (row.startsWith('|')) row = row.mid(1);
                    if (row.endsWith('|')) row.chop(1);
                    QStringList cells = row.split('|');
                    
                    QString rowStyle = (i % 2 == 0) ? "background: #fafafa;" : "background: #fff;";
                    tableHtml += QString("<tr style='%1'>").arg(rowStyle);
                    for (const QString& cell : cells) {
                        tableHtml += QString("<td style='border: 1px solid #ddd; padding: 8px;'>%1</td>")
                            .arg(cell.trimmed());
                    }
                    tableHtml += "</tr>";
                }
                
                tableHtml += "</tbody></table>";
                tables.append(tableHtml);
            } else {
                tables.append(match.captured(1));
            }
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            html = html.replace(positions[i].first, positions[i].second - positions[i].first, tables[i]);
        }
    }
    
    // 阶段5: 处理标题（改进样式）
    html.replace(QRegularExpression("^# (.*)$", QRegularExpression::MultilineOption), 
                 "<h1 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; margin-top: 20px;'>\\1</h1>");
    html.replace(QRegularExpression("^## (.*)$", QRegularExpression::MultilineOption), 
                 "<h2 style='color: #34495e; margin-top: 18px;'>\\1</h2>");
    html.replace(QRegularExpression("^### (.*)$", QRegularExpression::MultilineOption), 
                 "<h3 style='color: #7f8c8d; margin-top: 15px;'>\\1</h3>");
    
    // 阶段6: 处理文本格式（增强）
    // 加粗（排除代码块内）
    html.replace(QRegularExpression("\\*\\*(.+?)\\*\\*(?![^<]*</code>)"), "<strong style='color: #e74c3c;'>\\1</strong>");
    // 斜体
    html.replace(QRegularExpression("\\*(.+?)\\*(?![^<]*</code>)"), "<em>\\1</em>");
    
    // 阶段7: 处理图片（保持原有）
    {
        QRegularExpression re("!\\[(.*?)\\]\\((.*?)\\)");
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        
        QVector<QPair<int, int>> positions;
        QVector<QStringList> captures;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            
            QStringList caps;
            caps << match.captured(1) << match.captured(2);
            captures.append(caps);
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            int start = positions[i].first;
            int end = positions[i].second;
            QString alt = captures[i][0];
            QString src = captures[i][1];
            
            if (!src.startsWith("http") && !src.startsWith(":/") && !QFileInfo(src).isAbsolute()) {
                if (!m_currentFilePath.isEmpty()) {
                    QDir docDir = QFileInfo(m_currentFilePath).absoluteDir();
                    src = docDir.absoluteFilePath(src);
                }
            }
            
            QString replacement = QString(
                "<img src='%1' alt='%2' style='max-width: 100%%; height: auto; "
                "border: 1px solid #ddd; border-radius: 5px; margin: 10px 0; display: block;'/>"
            ).arg(src, alt);
            
            html = html.replace(start, end - start, replacement);
        }
    }
    
    // 阶段8: 处理链接（排除图片）
    {
        // 先替换图片，再处理链接，且排除已处理的图片
        QRegularExpression re("(?<!\\!)\\[(.*?)\\]\\((.*?)\\)");
        QRegularExpressionMatchIterator matches = re.globalMatch(html);
        
        QVector<QPair<int, int>> positions;
        QVector<QStringList> captures;
        
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            positions.append(qMakePair(match.capturedStart(), match.capturedEnd()));
            
            QStringList caps;
            caps << match.captured(1) << match.captured(2);
            captures.append(caps);
        }
        
        for (int i = positions.size() - 1; i >= 0; --i) {
            int start = positions[i].first;
            int end = positions[i].second;
            QString text = captures[i][0];
            QString url = captures[i][1];
            
            QString replacement = QString(
                "<a href='%1' style='color: #3498db; text-decoration: none; "
                "font-weight: bold; border-bottom: 1px dotted #3498db;'>%2</a>"
            ).arg(url, text);
            
            html = html.replace(start, end - start, replacement);
        }
    }
    
    // 阶段9: 处理段落和换行
    // 先将连续空行转换为段落分隔
    html.replace(QRegularExpression("\\n{2,}"), "</p><p style='margin: 10px 0; line-height: 1.6;'>");
    
    // 确保整个内容在段落中
    if (!html.trimmed().isEmpty() && !html.startsWith("<h1>") && !html.startsWith("<h2>") && 
        !html.startsWith("<h3>") && !html.startsWith("<pre>") && !html.startsWith("<table>")) {
        html = "<p style='margin: 10px 0; line-height: 1.6;'>" + html + "</p>";
    }
    
    // 处理单行换行（Markdown中的<br>）
    html.replace(QRegularExpression("(?<!<br/>)\\n"), "<br/>");
    
    return html;
}

QString DocumentViewer::enhancedMarkdownToHtml(const QString& markdown) {
    return markdownToHtml(markdown);
}

void DocumentViewer::zoomIn() {
    m_fontSize = qMin(m_fontSize + 1, 24);
    QFont font = m_textEdit->font();
    font.setPointSize(m_fontSize);
    m_textEdit->setFont(font);
    updateStatus();
}

void DocumentViewer::zoomOut() {
    m_fontSize = qMax(m_fontSize - 1, 8);
    QFont font = m_textEdit->font();
    font.setPointSize(m_fontSize);
    m_textEdit->setFont(font);
    updateStatus();
}

void DocumentViewer::searchText(const QString& text) {
    m_searchResults.clear();
    m_currentSearchIndex = -1;
    
    QList<QTextEdit::ExtraSelection> extraSelections;
    m_textEdit->setExtraSelections(extraSelections);
    
    if (text.isEmpty()) {
        m_prevBtn->setEnabled(false);
        m_nextBtn->setEnabled(false);
        m_searchInfoLabel->setText("");
        return;
    }
    
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = m_textEdit->document()->find(text, cursor);
        
        if (!cursor.isNull()) {
            m_searchResults.append(cursor);
        }
    }
    
    if (!m_searchResults.isEmpty()) {
        highlightSearchResults();
        navigateToResult(0);
    } else {
        m_prevBtn->setEnabled(false);
        m_nextBtn->setEnabled(false);
        m_searchInfoLabel->setText("未找到");
    }
}

void DocumentViewer::highlightSearchResults() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    for (int i = 0; i < m_searchResults.size(); ++i) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = m_searchResults[i];
        
        if (i == m_currentSearchIndex) {
            selection.format.setBackground(CURRENT_MATCH_COLOR);
            selection.format.setForeground(Qt::white);
            selection.format.setFontWeight(QFont::Bold);
        } else {
            selection.format.setBackground(MATCH_HIGHLIGHT_COLOR);
            selection.format.setForeground(Qt::black);
        }
        
        extraSelections.append(selection);
    }
    
    m_textEdit->setExtraSelections(extraSelections);
}

void DocumentViewer::navigateToResult(int index) {
    if (index < 0 || index >= m_searchResults.size()) {
        return;
    }
    
    m_currentSearchIndex = index;
    
    QTextCursor cursor = m_searchResults[index];
    cursor.clearSelection();
    
    m_textEdit->setTextCursor(cursor);
    
    m_textEdit->ensureCursorVisible();
    QRect cursorRect = m_textEdit->cursorRect(cursor);
    QScrollBar* vScrollBar = m_textEdit->verticalScrollBar();
    if (vScrollBar && cursorRect.top() > 0) {
        int adjust = cursorRect.top() - 10;
        vScrollBar->setValue(vScrollBar->value() + adjust);
    }
    
    highlightSearchResults();
    
    m_prevBtn->setEnabled(index > 0);
    m_nextBtn->setEnabled(index < m_searchResults.size() - 1);
    m_searchInfoLabel->setText(QString("%1/%2").arg(index + 1).arg(m_searchResults.size()));
}

void DocumentViewer::onPreviousResult() {
    if (m_currentSearchIndex > 0) {
        navigateToResult(m_currentSearchIndex - 1);
    }
}

void DocumentViewer::onNextResult() {
    if (m_currentSearchIndex >= 0 && m_currentSearchIndex < m_searchResults.size() - 1) {
        navigateToResult(m_currentSearchIndex + 1);
    }
    else if (m_currentSearchIndex == -1 && !m_searchResults.isEmpty()) {
        navigateToResult(0);
    }
}

void DocumentViewer::clear() {
    m_textEdit->clear();
    m_currentFilePath.clear();
    updateStatus();
}

void DocumentViewer::updateStatus() {
    if (m_currentFilePath.isEmpty()) {
        m_statusLabel->setText("就绪 | 未加载文档");
        return;
    }
    
    QString fileName = QFileInfo(m_currentFilePath).fileName();
    int wordCount = m_textEdit->toPlainText().length();
    
    m_statusLabel->setText(QString("已加载: %1 | 字数: %2 | 字体大小: %3")
                          .arg(fileName).arg(wordCount).arg(m_fontSize));
}

void DocumentViewer::onCourseChanged(int index) {
    if (index >= 0) loadDocument(m_courseCombo->itemData(index).toString());
}

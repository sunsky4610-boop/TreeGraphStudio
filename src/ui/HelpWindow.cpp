#include "HelpWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollBar>
#include <QTimer>
#include <QTextCursor>

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

void HelpWindow::showManual() {
    m_infoPanel = nullptr;  // 静态手册，不绑定实时日志
    setWindowTitle("详细操作说明");
    m_textEdit->setFont(QFont("Microsoft YaHei UI", 10));
    m_textEdit->setHtml(QString::fromUtf8(R"HTML(
<h2><font color="#2563eb">TreeGraph Studio 详细操作说明</font></h2>
<p><i>树 / 图算法可视化教学系统</i></p>
<hr>
<h3><font color="#2563eb">一、界面布局</font></h3>
<ul>
<li><b>左侧工具箱</b>：新建图 / 新建树 / 清空画布、图类型(有向 / 无向)、三种编辑模式。</li>
<li><b>中间画布</b>：点阵背景，是搭建图、编辑节点边和播放算法动画的区域。</li>
<li><b>右侧控制面板</b>：算法选择、动画速度、算法控制，以及<b>操作说明与进度</b>实时日志。</li>
<li><b>画布右下角浮动卡片</b>：显示设置、学习系统入口。</li>
<li><b>顶部菜单</b>：文件 / 视图 / 帮助；<b>底部状态栏</b>显示当前节点、边数量与状态。</li>
</ul>
<h3><font color="#2563eb">二、新建与图类型</font></h3>
<ul>
<li><b>新建图</b>：创建一张空白图；<b>新建树</b>：创建一棵空白树(添加边时自动维护父子关系，不允许成环)；<b>清空画布</b>：删除全部内容。</li>
<li>左栏单选切换<b>有向 / 无向</b>，切换会清空当前画布。</li>
<li>有向边带箭头，方向由"添加边"时点击两个节点的先后顺序决定。</li>
</ul>
<h3><font color="#2563eb">三、三种编辑模式</font></h3>
<ol>
<li><b>选择模式</b>：单击节点设为算法起始点（紫罗兰色）；按住节点拖动可调整位置；鼠标悬停节点 / 边可查看名称、坐标、度、权重等信息。</li>
<li><b>添加节点</b>：在画布空白处单击即新增一个节点(自动命名 N0、N1……)。</li>
<li><b>添加边</b>：先单击起点节点、再单击终点节点，随后在弹窗中输入权重(默认 1)。</li>
</ol>
<h3><font color="#2563eb">四、右键菜单与双击</font></h3>
<ul>
<li><b>右键节点</b>：重命名 / 删除节点(删除节点会一并删除与之相连的边)。</li>
<li><b>右键边</b>：编辑权重 / 删除边。</li>
<li><b>双击边</b>：快速编辑该边的权重。</li>
</ul>
<h3><font color="#2563eb">五、文件保存与加载</font></h3>
<ul>
<li><b>文件 → 保存</b>：把当前图保存为 <b>.graph</b> 文件。</li>
<li><b>文件 → 加载</b>：打开 .graph 文件；文件的有向 / 无向类型需与当前画布一致，加载树文件前请先切换到"新建树"。</li>
</ul>
<h3><font color="#2563eb">六、算法演示(右栏)</font></h3>
<ul>
<li><b>算法选择</b>：BFS 遍历、DFS 遍历、最短路径(Dijkstra)、最小生成树(Kruskal)。
  <ul>
  <li>BFS / DFS：先在"选择模式"下点选一个起点，再运行。</li>
  <li>最短路径：选择起点与终点后运行。</li>
  <li>最小生成树：仅<b>无向图</b>可用，有向图会提示无法运行。</li>
  </ul>
</li>
<li><b>运行</b>：自动按步播放；<b>暂停 / 继续</b>：控制自动播放；<b>上一步 / 下一步</b>：手动逐帧查看；<b>重置</b>：清除高亮、回到初始状态。</li>
<li><b>动画速度</b>：拖动滑块调整播放快慢。</li>
</ul>
<p><b>颜色含义：</b></p>
<ul>
<li><font color="#d9a400"><b>黄色</b></font>：算法正在访问 / 处理的节点或边。</li>
<li><font color="#8b5cf6"><b>紫罗兰色</b></font>：起始节点 / 当前选中节点。</li>
<li><font color="#1c9ed0"><b>青色</b></font>：鼠标悬停的节点或边。</li>
<li><font color="#e04040"><b>红色虚线</b></font>：被算法拒绝的边(例如 Kruskal 中会形成环的边)。</li>
<li><font color="#e8920a"><b>橙色</b></font>：正在拖动的节点。</li>
</ul>
<h3><font color="#2563eb">七、显示设置(画布右下角卡片)</font></h3>
<ul>
<li>显示 / 隐藏边权重、节点坐标、坐标轴。</li>
</ul>
<h3><font color="#2563eb">八、学习系统</font></h3>
<ul>
<li>点击画布右下角"<b>打开学习系统</b>"，可查看学习文档、算法演示视频和图练习。</li>
<li>在"图练习"中双击文件，或选中文件后点"<b>加载到主窗口</b>"，可把示例图载入画布，再选择算法运行。</li>
</ul>
<h3><font color="#2563eb">九、视图缩放</font></h3>
<ul>
<li><b>鼠标滚轮</b>：以光标位置为中心放大 / 缩小(约 0.35× ~ 4×)。</li>
<li><b>视图菜单</b>：放大 / 缩小 / 重置视图。</li>
<li>在"选择模式"下按住节点拖动，可自由调整图的布局。</li>
</ul>
<h3><font color="#2563eb">十、操作说明与进度(右栏)</font></h3>
<ul>
<li>实时记录每一步操作和算法的遍历顺序。</li>
<li><b>清空</b>：清除日志；<b>新窗口</b>：把日志放大到独立窗口查看。</li>
</ul>
<hr>
<p><b>快速上手：</b>新建图 → 添加节点 → 添加边搭好结构 → 选择算法与起点 → 点击"运行"，即可观看完整的算法执行过程。</p>
)HTML"));
    m_textEdit->moveCursor(QTextCursor::Start);
}

void HelpWindow::setInfoPanel(QTextEdit* infoPanel) {
    m_infoPanel = infoPanel;
    setWindowTitle("操作说明与进度");
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

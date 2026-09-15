# TreeGraph Studio · 树与图算法可视化教学系统

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5.15.x-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D4.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![GitHub stars](https://img.shields.io/github/stars/sunsky4610-boop/TreeGraphStudio?style=social)

> 让抽象的图论"看得见、跟得上、能动手"——一个用 C++17 与 Qt5 实现的树 / 图算法可视化教学桌面软件。

你可以用鼠标画出无向图、有向图和树，然后**单步动画**观看 BFS、DFS、Dijkstra、Kruskal 是如何一步步运行的；软件同时内置教程文档、示例图和教学视频，构成一个"编辑 → 演示 → 学习 → 练习"的闭环。

---

## 目录

- [功能特性](#-功能特性)
- [支持的算法](#-支持的算法)
- [效果预览](#-效果预览)
- [系统要求](#-系统要求)
- [构建与运行](#-构建与运行)
- [快速上手](#-快速上手)
- [项目结构](#-项目结构)
- [核心设计](#-核心设计)
- [.graph 文件格式](#-graph-文件格式)
- [常见问题](#-常见问题)
- [后续规划](#-后续规划)
- [许可证](#-许可证)

---

## ✨ 功能特性

### 图结构编辑
- 支持 **无向图 / 有向图 / 树** 三种结构，一键切换
- 添加节点、添加边、选择三种编辑模式；节点可拖拽移动、双击重命名、右键删除
- 边权重可视化，范围 1–100；有向图自动绘制方向箭头
- 可选坐标轴与节点坐标显示，悬停查看度数 / 权重 / 坐标等信息

### 算法可视化
- **逐步执行**：运行、暂停、继续、上一步、下一步、重置，并记录完整步骤历史
- **动画与配色**：访问过的节点、松弛的边、被选中 / 被拒绝的生成树边用不同颜色区分
- 1–10 级速度调节，实时输出算法状态、遍历顺序、最短路径与总权重
- 对"图不连通""目标不可达""有向图不能求 MST"等情况给出明确提示

### 内置学习系统
- **文档学习**：Markdown / TXT 阅读器，支持标题导航、关键字搜索、字号缩放
- **图实践**：浏览内置 `.graph` 示例（显示节点 / 边数与类型），一键加载到画布练习
- **视频教程**：基于 Qt Multimedia，自带 WMV 支持；MP4/MKV 需另装解码器（见[常见问题](#-常见问题)）
- 支持自行导入文档（.md/.txt）、视频与 `.graph` 案例

### 数据持久化
- 自定义 `.graph`（JSON）格式保存完整图结构，加载时校验图类型

---

## 🧮 支持的算法

| 算法 | 类型 | 时间复杂度 | 空间复杂度 | 适用图 | 可视化要点 |
|------|------|-----------|-----------|--------|-----------|
| **BFS** 广度优先 | 遍历 | O(V+E) | O(V) | 有向 / 无向 / 树 | 队列驱动，按层扩散、逐层点亮 |
| **DFS** 深度优先 | 遍历 | O(V+E) | O(V) | 有向 / 无向 / 树 | 显式栈驱动，一条路走到底再回溯 |
| **Dijkstra** | 最短路径 | O(E log V) | O(V) | 有向 / 无向（非负权） | 最小堆 + 松弛，动态显示当前最短路径与总距离 |
| **Kruskal** | 最小生成树 | O(E log E) | O(V) | 仅无向 / 树 | 边按权排序 + 并查集，成环边被标红拒绝 |

> 算法层与界面层解耦：所有算法都实现统一的 `initialize() / step() / runAll()` 步进接口，
> 通过 `std::function` 回调驱动界面刷新，因此可以方便地暂停、单步与回放，也易于扩展新算法。

---

## 📸 效果预览

<!-- 准备好截图后，把图片放到 screenshots/ 目录并取消下面注释即可
### 主界面与算法动画
![主界面](screenshots/main.png)
![BFS 演示](screenshots/bfs.gif)
![Dijkstra 演示](screenshots/dijkstra.gif)
-->

*（截图 / 动图待补充）*

---

## 🖥️ 系统要求

- **操作系统**：Windows 10 / 11
- **Qt**：Qt 5.15.x（在 5.15.2 MSVC2019 64-bit 上开发验证；5.14 及以上的 Qt5 均可），需包含 `Core / Widgets / Gui / Multimedia / MultimediaWidgets`
- **编译器（二选一）**：
  - MSVC：Visual Studio 2019/2022/2026 的「使用 C++ 的桌面开发」工作负载（选 Qt 的 **MSVC 2019 64-bit** 组件，向后兼容）
  - MinGW：Qt 安装器自带的 MinGW 64-bit 工具链
- **构建工具**：CMake ≥ 3.16（Qt 安装器可一并安装 CMake 与 Ninja）

---

## 🔧 构建与运行

### 方式一：Qt Creator（推荐，最简单）

1. 克隆仓库：
   ```bash
   git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
   cd TreeGraphStudio
   ```
2. 打开 Qt Creator →「文件 → 打开文件或项目」→ 选择根目录的 `CMakeLists.txt`；
3. 在「Configure Project」页面勾选与你安装的 Qt 对应的套件（如 **Desktop Qt 5.15.2 MSVC2019 64bit**），点击 Configure；
4. 点击左下角锤子（或 `Ctrl+B`）构建；
5. 构建产物位于 `build-.../bin/` 目录。**把整个 `learning resources` 文件夹复制到该 `bin` 目录旁边**，教程、示例图和视频才会出现；
6. 点击绿色三角（或 `Ctrl+R`）运行。

### 方式二：命令行（以 MSVC + Ninja 为例）

先在「x64 Native Tools Command Prompt for VS」中执行（以保证 cl.exe 在 PATH 中）：

```bash
git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
cd TreeGraphStudio
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"
cmake --build build
```

可执行文件生成在 `build/bin/`。MinGW 把 `CMAKE_PREFIX_PATH` 换成对应的 `mingw81_64` 路径、生成器改用 MinGW Makefiles 即可。

> **部署提示**：构建脚本会自动把 Qt 的 `mediaservice` 媒体插件复制到可执行文件目录。
> 若要把 exe 拷到没有 Qt 的机器上运行，用 `windeployqt TreeGraphStudio.exe` 收集依赖 DLL。
>
> **路径提示**：请把项目放在纯英文路径下编译，中文 / 空格路径可能导致多媒体插件加载异常。

---

## 🚀 快速上手

1. **选类型**：左侧面板选择「无向 / 有向 / 树」；
2. **画图**：点「添加节点」后在画布空白处点击放置节点；点「添加边」后依次点击两个节点并输入权重；
3. **跑算法**：右侧选择 BFS / DFS / 最短路径 / 最小生成树，指定起点（最短路径还要指定终点），点「运行」；
4. **单步观察**：用「上一步 / 下一步」逐帧观看，或拖动速度条调节节奏；
5. **保存**：菜单「文件 → 保存」导出 `.graph`，下次可直接加载继续。

### 交互速查

| 操作 | 效果 |
|------|------|
| 单击节点 | 选中 / 设为算法起点 |
| 拖拽节点 | 移动位置，相连的边与箭头实时跟随 |
| 双击节点 / 边 | 重命名节点 / 编辑边权重 |
| 右键节点 / 边 | 删除、重命名、编辑权重等菜单 |

---

## 🏗️ 项目结构

```
TreeGraphStudio/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 程序入口
├── resources.qrc               # Qt 资源文件
├── src/
│   ├── core/                   # 核心数据结构（模板）
│   │   ├── Graph.h             # 图基类 Graph<NodeData, EdgeWeight, Directed>
│   │   ├── Tree.h              # 树：环检测 + 单父节点约束
│   │   ├── Node.h / Edge.h     # 节点 / 边
│   ├── algorithms/             # 算法层（与界面解耦，统一步进接口）
│   │   ├── Traversal.h         # BFS / DFS
│   │   ├── ShortestPath.h      # Dijkstra（最小堆 + 松弛）
│   │   └── MST.h               # Kruskal（并查集）
│   ├── ui/                     # 界面层
│   │   ├── MainWindow.*        # 主窗口与面板
│   │   ├── GraphCanvas.*       # QGraphicsView 画布：编辑、动画、步骤回放
│   │   └── HelpWindow.*        # 帮助窗口
│   ├── study/                  # 内置学习系统
│   │   ├── StudyWindow.*
│   │   ├── DocumentViewer.*    # Markdown/TXT 阅读
│   │   ├── GraphPracticeWidget.*
│   │   └── VideoPlayerWidget.* # Qt Multimedia 播放
│   └── utils/
│       ├── JsonSerializer.h    # .graph 序列化
│       └── PathUtils.h         # 资源路径查找
└── learning resources/         # 内置教学资源
    ├── documents/              # 算法教程（md/txt）
    ├── graphs/                 # 示例图（.graph）
    └── videos/                 # 教学视频（wmv）
```

---

## 🧠 核心设计

- **统一步进接口**：每个算法提供 `initialize(start)` 初始化、`step()` 推进一步并返回是否结束、`runAll()` 一次跑完；界面层用定时器以固定间隔调用 `step()`，从而用同一套代码支持自动播放与单步回放。
- **回调驱动渲染**：算法在"访问节点 / 松弛边 / 选中或拒绝边"时通过 `std::function` 回调通知画布，算法本身不依赖任何 Qt 控件，便于单独测试与扩展。
- **模板化图结构**：`Graph<数据类型, 权重类型, 是否有向>` 在编译期区分有向 / 无向，邻接表 + 智能指针管理内存；`Tree` 继承无向图并加入成环与多父节点校验。
- **统一算法包装**：界面层通过 `AlgorithmWrapper` 在运行时切换不同算法与图类型，并维护步骤历史（`StepInfo`）以支持前进 / 后退。

---

## 📁 .graph 文件格式

`.graph` 本质是 UTF-8 编码的 JSON：

```json
{
  "type": "undirected",
  "nodes": [
    { "id": 0, "label": "A", "x": 120.0, "y": 100.0 },
    { "id": 1, "label": "B", "x": 260.0, "y": 180.0 }
  ],
  "edges": [
    { "from": 0, "to": 1, "weight": 5 }
  ]
}
```

`type` 取值 `undirected`（无向图）/ `directed`（有向图）/ `tree`（树）。

---

## ❓ 常见问题

**Q1：构建时报 "Could NOT find Qt5" / "Qt5 Multimedia not found"？**
确认安装 Qt 时勾选了对应编译器的预编译库（含 Multimedia 模块），并在 Qt Creator 里选对了套件；命令行构建则检查 `CMAKE_PREFIX_PATH` 是否指向正确的 Qt 版本目录。

**Q2：MSVC 编译时界面中文乱码 / 出现 C4819、C2065？**
本项目已在 CMake 中为 MSVC 开启 `/utf-8` 并定义 `_USE_MATH_DEFINES`，使用仓库自带的 `CMakeLists.txt` 即可，无需额外设置。

**Q3：视频无法播放，提示格式不支持？**
Windows 下自带支持 WMV；MP4 / MKV 等格式请安装 [K-Lite Codec Pack Basic](https://codecguide.com)（仅需 Basic 版）。

**Q4：为什么有向图不能用最小生成树？**
最小生成树（Kruskal）只定义在无向图上，软件会直接拦截并提示；有向图请使用 BFS / DFS / 最短路径。

**Q5：Dijkstra 能用负权边吗？**
不能。Dijkstra 要求边权非负，本软件的权重输入也被限定为 1–100 的正整数；需要负权场景可改用 Bellman-Ford（在后续规划中）。

**Q6：遍历结束提示"另有节点不可达"是什么意思？**
说明你的图不连通（有向图则是从起点沿方向无法到达），BFS / DFS 只会遍历起点所在的连通分量，这是正常现象。

**Q7：如何添加自己的算法？**
在 `src/algorithms/` 下新增实现统一步进接口的算法类，再在 `GraphCanvas` 的创建与步进分支中注册即可，算法层不依赖界面、可独立编写单元测试。

---

## 🧭 后续规划

- [ ] Prim、Bellman-Ford、拓扑排序、Floyd 等更多算法
- [ ] 撤销 / 重做、随机图与网格图生成器
- [ ] 大规模图（1000+ 节点）渲染性能优化
- [ ] 跨平台（Linux / macOS）多媒体适配
- [ ] 算法单元测试与多语言支持

欢迎通过 [Issues](https://github.com/sunsky4610-boop/TreeGraphStudio/issues) 提交 bug 与建议，也欢迎 Pull Request。

---

## 📄 许可证

本项目基于 [MIT License](LICENSE) 开源，可自由使用、修改与分发，保留版权声明即可。

---

**如果这个项目对你学习图论有帮助，欢迎点一个 Star ⭐，这是对作者最大的鼓励。**

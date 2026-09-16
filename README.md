# TreeGraph Studio · 树与图算法可视化教学系统

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5.15.2-7C3AED.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D4.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![GitHub stars](https://img.shields.io/github/stars/sunsky4610-boop/TreeGraphStudio?style=social)

> 让抽象的图论"看得见、跟得上、能动手"——用 C++17 与 Qt5 实现的树 / 图算法可视化教学桌面软件。

用鼠标画出无向图、有向图或树，再以**单步动画**观看 BFS、DFS、Dijkstra、Kruskal 如何逐步运行；软件内置教程文档与任务式练习图，形成“编辑 → 演示 → 学习 → 练习”的完整闭环。

## 直接下载运行

不想配置开发环境的用户，可在 GitHub Releases 下载 `TreeGraphStudio-Windows-x64.zip`：

1. 完整解压 ZIP，不能只从压缩包内直接打开 EXE；
2. 双击 `TreeGraphStudio.exe`；
3. Windows 首次提示安全确认时，选择“更多信息 → 仍要运行”。

发布包已包含 Qt 运行库，Windows 10/11 x64 无需安装 Qt。

## 目录

- [功能特性](#功能特性)
- [支持的算法](#支持的算法)
- [效果预览](#效果预览)
- [环境要求与构建](#环境要求与构建)
- [快速上手](#快速上手)
- [项目结构与设计](#项目结构与设计)
- [.graph 数据格式](#graph-数据格式)
- [常见问题](#常见问题)
- [参与与许可](#参与与许可)

---

## 功能特性

- **图结构编辑**：无向图 / 有向图 / 树一键切换；添加节点、添加边、选择三种模式；节点可拖拽、双击重命名、右键删除；边权重 1–100 可调，有向图自动绘制箭头；可选坐标轴与坐标显示。
- **算法可视化**：运行 / 暂停 / 继续 / 上一步 / 下一步 / 重置，完整步骤历史可回放；用颜色区分访问节点、松弛边、被选中与被拒绝的生成树边；1–10 级调速；实时输出遍历顺序、最短路径与总权重；对"图不连通""目标不可达""有向图不能求 MST"给出明确提示。
- **内置学习系统**：BFS、DFS、Dijkstra、Kruskal 四套内置课程；三套任务式 `.graph` 练习可一键载入；支持搜索、字号缩放及自行导入文档、视频与案例。
- **数据持久化**：自定义 `.graph`（JSON）格式保存完整图结构，加载时自动校验图类型。

## 支持的算法

| 算法 | 类型 | 时间复杂度 | 空间复杂度 | 适用图 | 可视化要点 |
|------|------|-----------|-----------|--------|-----------|
| **BFS** 广度优先 | 遍历 | O(V+E) | O(V) | 有向 / 无向 / 树 | 队列驱动，按层扩散、逐层点亮 |
| **DFS** 深度优先 | 遍历 | O(V+E) | O(V) | 有向 / 无向 / 树 | 显式栈驱动，走到底再回溯 |
| **Dijkstra** | 最短路径 | O(E log V) | O(V) | 有向 / 无向（非负权） | 最小堆 + 松弛，动态显示最短路径与总距离 |
| **Kruskal** | 最小生成树 | O(E log E) | O(V) | 仅无向 / 树 | 边按权排序 + 并查集，成环边标红拒绝 |

## 效果预览

<!-- 截图准备好后放入 screenshots/ 并取消注释
![主界面](screenshots/main.png)
![算法动画](screenshots/algorithm.gif)
-->

*（界面截图 / 动图待补充）*

---

## 环境要求与构建

### 一、开发环境（本项目实际验证版本）

| 组件 | 版本 / 选择 | 说明 |
|------|------------|------|
| 操作系统 | Windows 10 / 11 | 主要面向 Windows 桌面 |
| **Qt** | **5.15.2**（5.15.x、5.14+ 均可） | 安装时勾选 **MSVC 2019 64-bit** 预编译库 |
| Qt 模块 | Core、Widgets、Gui、Multimedia、MultimediaWidgets | 装 Qt 主库即包含，CMake 会自动查找 |
| 编译器 | **MSVC**（本项目用 Visual Studio 2026 的 v143 工具链，兼容 Qt 的 MSVC2019 套件；VS2019/2022 亦可）；或 MinGW 64-bit | 二选一，需支持 **C++17** |
| 构建系统 | **CMake ≥ 3.16**（Qt 自带 3.30.x） | 工程用 CMake 管理 |
| 生成器 | **Ninja**（Qt 自带）；或 NMake / JOM | Qt Creator 会自动选择 |
| IDE | **Qt Creator**（随 Qt 安装） | 推荐，配置最省心 |

### 二、Qt 在线安装器该勾哪些（新手照勾）

1. **Qt → Qt 5.15.2 → MSVC 2019 64-bit**（用 MinGW 则改勾 MinGW 64-bit）；
2. **Developer and Designer Tools** 下勾选：
   - **Qt Creator**（IDE）
   - **CMake**（构建工具）
   - **Ninja**（生成器）
   - **Debugging Tools for Windows / CDB Debugger Support**（调试用，可选）
3. 编译器单独装：用 MSVC 就安装 Visual Studio 并勾选「**使用 C++ 的桌面开发**」工作负载。

### 三、方式 A：Qt Creator 构建（推荐）

1. 获取源码：
   ```bash
   git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
   cd TreeGraphStudio
   ```
2. Qt Creator →「文件 → 打开文件或项目」→ 选择根目录 `CMakeLists.txt`；
3. 在配置页勾选套件 **Desktop Qt 5.15.2 MSVC2019 64bit**，点 Configure；
4. `Ctrl+B` 构建，可执行文件生成在 `build-.../bin/`；
5. `Ctrl+R` 运行。教程与示例图已经编译进程序，不需要额外复制资源目录。

### 四、方式 B：命令行（MSVC + Ninja）

在「x64 Native Tools Command Prompt for VS」中（保证 `cl.exe` 在 PATH）：

```bash
git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
cd TreeGraphStudio
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"
cmake --build build
```

产物位于 `build/bin/`。MinGW 把 `CMAKE_PREFIX_PATH` 换成对应 `mingw*_64` 目录即可。

### 五、运行与部署提示

- 构建脚本会**自动复制 Qt 的 `mediaservice` 媒体插件**到可执行文件目录，无需手动处理；
- 想把 exe 拷到没装 Qt 的电脑运行，用 `windeployqt TreeGraphStudio.exe` 收集依赖 DLL；
- 视频学习页支持用户自行导入本地视频；格式兼容性取决于 Windows 解码器；
- 请使用**纯英文路径**编译与运行，中文 / 空格路径可能导致多媒体插件加载异常；
- MSVC 下的 UTF-8 编码与 `M_PI` 等问题已在 `CMakeLists.txt` 中通过 `/utf-8` 与 `_USE_MATH_DEFINES` 处理，直接构建即可。

---

## 快速上手

1. **选类型**：左侧选择「无向 / 有向 / 树」；
2. **画图**：「添加节点」后在空白处点击放置；「添加边」后依次点两个节点并输入权重；
3. **跑算法**：右侧选算法，指定起点（最短路径还需终点），点「运行」；
4. **单步观察**：用「上一步 / 下一步」逐帧观看，或拖动速度条；
5. **保存**：菜单「文件 → 保存」导出 `.graph`。

| 交互 | 效果 |
|------|------|
| 单击节点 | 选中 / 设为算法起点 |
| 拖拽节点 | 移动，相连边与箭头实时跟随 |
| 双击节点 / 边 | 重命名 / 编辑权重 |
| 右键节点 / 边 | 删除、重命名、编辑权重 |

---

## 项目结构与设计

```
TreeGraphStudio/
├── CMakeLists.txt          # CMake 构建脚本（Qt5 依赖、C++17、MSVC 编码处理、插件复制）
├── main.cpp                # 程序入口
├── resources.qrc           # Qt 资源清单
├── src/
│   ├── core/               # 核心数据结构（模板）：Graph / Tree / Node / Edge
│   ├── algorithms/         # 算法层：Traversal(BFS/DFS)、ShortestPath(Dijkstra)、MST(Kruskal)
│   ├── ui/                 # 界面层：MainWindow 主窗口、GraphCanvas 画布、HelpWindow
│   ├── study/              # 学习系统：文档阅读 / 示例图实践 / 视频播放
│   └── utils/              # 工具：JsonSerializer 序列化、PathUtils 路径
├── learning resources/     # 内置教学资源源文件：documents 文档、graphs 示例图
├── resources/              # 主题样式、应用图标与 Windows 资源
├── LICENSE                 # MIT 许可证
└── README.md
```

**设计要点**

- **统一步进接口**：每个算法都实现 `initialize(start)` / `step()`（推进一步并返回是否结束）/ `runAll()`；界面用定时器按固定间隔调用 `step()`，于是自动播放与单步回放共用同一套逻辑。
- **算法与界面解耦**：算法在"访问节点 / 松弛边 / 选中或拒绝边"时通过 `std::function` 回调通知画布，算法层不依赖任何 Qt 控件，可独立测试、便于扩展。
- **模板化图结构**：`Graph<NodeData, EdgeWeight, Directed>` 在编译期区分有向 / 无向，邻接表 + 智能指针管理内存；`Tree` 加入成环与多父节点校验。
- **统一算法包装**：界面层通过 `AlgorithmWrapper` 在运行时切换图类型与算法，并用步骤历史支持前进 / 后退。

## .graph 数据格式

`.graph` 为 UTF-8 编码的 JSON：

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

## 常见问题

**Q1：构建报 "Could NOT find Qt5" / "Qt5 Multimedia not found"？**
确认安装 Qt 时勾选了对应编译器的预编译库（含 Multimedia），Qt Creator 里选对套件；命令行构建检查 `CMAKE_PREFIX_PATH` 是否指向正确的 Qt 目录。

**Q2：MSVC 编译中文乱码 / C4819 / C2065？**
仓库的 `CMakeLists.txt` 已为 MSVC 开启 `/utf-8` 并定义 `_USE_MATH_DEFINES`，直接用自带脚本构建即可，无需额外设置。

**Q3：视频提示格式不支持？**
视频模块依赖系统解码器。当前发布版不附带示例视频，可自行导入本地视频；若格式无法播放，请转换为系统支持的格式。

**Q4：有向图为什么不能用最小生成树？**
MST（Kruskal）只定义在无向图上，软件会拦截并提示；有向图请使用 BFS / DFS / 最短路径。

**Q5：Dijkstra 支持负权边吗？**
不支持，Dijkstra 要求边权非负（本软件权重限定 1–100）；负权场景需 Bellman-Ford（规划中）。

**Q6：提示"另有节点不可达"？**
说明图不连通（有向图则是从起点沿方向无法到达），遍历只覆盖起点所在的连通分量，属正常现象。

**Q7：如何扩展新算法？**
在 `src/algorithms/` 下实现统一步进接口的新算法类，再到 `GraphCanvas` 的创建与步进分支注册即可。

---

## 参与与许可

- **作者 / 维护**：张川（GitHub：[@sunsky4610-boop](https://github.com/sunsky4610-boop)）
- **问题反馈**：欢迎在 [Issues](https://github.com/sunsky4610-boop/TreeGraphStudio/issues) 提交 bug 与建议，也欢迎 Pull Request
- **后续规划**：Prim / Bellman-Ford / 拓扑排序 / Floyd 等算法、撤销重做与随机图生成、千节点级渲染优化、跨平台适配、单元测试与多语言
- **许可证**：本项目基于 [MIT License](LICENSE) 开源，可自由使用、修改与分发，保留版权声明即可

**如果它对你学习图论有帮助，欢迎点一个 Star ⭐，这是对作者最大的鼓励。**

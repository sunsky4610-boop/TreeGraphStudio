# TreeGraph Studio

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5.15.2-7C3AED.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D4.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)
![GitHub stars](https://img.shields.io/github/stars/sunsky4610-boop/TreeGraphStudio?style=social)

> An interactive desktop application that makes graph algorithms visible, traceable, and easier to learn.

TreeGraph Studio is a C++17 and Qt-based educational tool for building graphs and watching algorithms execute step by step. Create undirected graphs, directed graphs, or trees with the mouse, then explore BFS, DFS, Dijkstra, and Kruskal through animated execution, detailed progress messages, built-in lessons, and guided practice graphs.

## Download and Run

Download `TreeGraphStudio-Windows-x64.zip` from the [latest release](https://github.com/sunsky4610-boop/TreeGraphStudio/releases/latest):

1. Extract the entire ZIP archive. Do not run the executable from inside the archive.
2. Launch `TreeGraphStudio.exe`.
3. If Windows displays a security prompt on first launch, select **More info → Run anyway**.

The release package includes the required Qt runtime libraries. Qt does not need to be installed on Windows 10 or Windows 11 x64.

## Contents

- [Features](#features)
- [Supported Algorithms](#supported-algorithms)
- [Preview](#preview)
- [Requirements and Build Instructions](#requirements-and-build-instructions)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [.graph File Format](#graph-file-format)
- [FAQ](#faq)
- [Contributing and License](#contributing-and-license)

## Features

- **Graph editing:** Switch between undirected graphs, directed graphs, and trees. Add, move, rename, and delete nodes; add or edit weighted edges; display arrows, coordinates, and axes when needed.
- **Algorithm visualization:** Run, pause, resume, step backward, step forward, or reset an algorithm. A complete step history supports replay, while restrained visual states distinguish visited nodes, relaxed edges, selected minimum-spanning-tree edges, and rejected edges.
- **Smooth canvas interaction:** Pan the graph by dragging empty canvas space, double-click to center the whole graph, and use animated fit-to-view transitions. Nodes respond to hover with a subtle scale animation.
- **Built-in learning system:** Read lessons for BFS, DFS, Dijkstra, and Kruskal, then load three guided `.graph` exercises directly into the main canvas. External documents, videos, and graph examples can also be imported.
- **Light and dark themes:** A polished interface with persistent theme preferences and native Windows title-bar styling.
- **Data persistence:** Save and load complete graph structures in a JSON-based `.graph` format with graph-type validation.

## Supported Algorithms

| Algorithm | Category | Time Complexity | Space Complexity | Supported Graphs | Visualization Focus |
|---|---|---:|---:|---|---|
| **BFS** | Traversal | O(V + E) | O(V) | Directed, undirected, tree | Queue-driven, level-by-level expansion |
| **DFS** | Traversal | O(V + E) | O(V) | Directed, undirected, tree | Stack-driven exploration and backtracking |
| **Dijkstra** | Shortest path | O(E log V) | O(V) | Directed or undirected, non-negative weights | Priority queue, edge relaxation, final path and distance |
| **Kruskal** | Minimum spanning tree | O(E log E) | O(V) | Undirected graph or tree | Sorted edges, disjoint sets, accepted and rejected edges |

## Preview

Screenshots and an animated demonstration are planned for a future update.

<!--
![Main window](screenshots/main.png)
![Algorithm animation](screenshots/algorithm.gif)
-->

## Requirements and Build Instructions

### Verified Development Environment

| Component | Version or Choice | Notes |
|---|---|---|
| Operating system | Windows 10 or 11 | Primary desktop target |
| **Qt** | **5.15.2**; Qt 5.14+ should work | Install the kit matching your compiler |
| Qt modules | Core, Widgets, Gui, Multimedia, MultimediaWidgets | Located automatically by CMake |
| Compiler | MSVC 2019/2022-compatible toolchain or MinGW 64-bit | Must support C++17 |
| Build system | **CMake 3.16+** | Project configuration and build |
| Generator | Ninja, NMake, or JOM | Ninja is recommended |
| IDE | Qt Creator | Optional but recommended |

### Qt Installer Components

For an MSVC build, select:

1. **Qt → Qt 5.15.2 → MSVC 2019 64-bit**;
2. **Developer and Designer Tools → Qt Creator, CMake, and Ninja**;
3. The **Desktop development with C++** workload in Visual Studio.

Choose the matching MinGW 64-bit Qt kit instead if you prefer MinGW.

### Build with Qt Creator

1. Clone the repository:

   ```bash
   git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
   cd TreeGraphStudio
   ```

2. In Qt Creator, choose **File → Open File or Project** and open `CMakeLists.txt`.
3. Select a compatible desktop Qt kit and configure the project.
4. Build with `Ctrl+B` and run with `Ctrl+R`.

Lessons and practice graphs are embedded in the application and do not require a separate resource directory.

### Build from the Command Line

Run the following from an x64 Native Tools Command Prompt for Visual Studio:

```bat
git clone https://github.com/sunsky4610-boop/TreeGraphStudio.git
cd TreeGraphStudio
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64"
cmake --build build
```

The executable is generated under `build/bin/`. Change `CMAKE_PREFIX_PATH` if you use a different Qt kit.

### Deployment Notes

- The build copies the required Qt media-service plugin into the executable directory.
- Use `windeployqt TreeGraphStudio.exe` when preparing a standalone package manually.
- The video-learning page can open local media files; codec support depends on the Windows environment.
- Build and run the project from an ASCII-only path when possible, because some multimedia plugins can fail on paths containing non-ASCII characters.
- The CMake configuration enables UTF-8 source handling and `_USE_MATH_DEFINES` for MSVC.

## Quick Start

1. **Choose a graph type:** undirected graph, directed graph, or tree.
2. **Build a graph:** select **Add Node** and click the canvas, then select **Add Edge** and choose two nodes.
3. **Choose an algorithm:** select a start node; Dijkstra also requires a target node.
4. **Run or inspect step by step:** use the playback controls or adjust the animation-speed slider.
5. **Save your work:** use **File → Save** to export a `.graph` file.

| Interaction | Result |
|---|---|
| Click a node | Select it or use it as the algorithm start node |
| Drag a node | Move it while connected edges and arrows follow |
| Double-click a node or edge | Rename the node or edit the edge weight |
| Right-click a node or edge | Open editing and deletion actions |
| Drag empty canvas space | Pan the whole view |
| Double-click empty canvas space | Smoothly center and fit the graph |

## Architecture

```text
TreeGraphStudio/
├── CMakeLists.txt          # Qt 5 dependencies, C++17, build and deployment rules
├── main.cpp                # Application entry point
├── resources.qrc           # Embedded Qt resources
├── src/
│   ├── core/               # Graph, Tree, Node, and Edge data structures
│   ├── algorithms/         # BFS, DFS, Dijkstra, and Kruskal
│   ├── ui/                 # MainWindow, GraphCanvas, and HelpWindow
│   ├── study/              # Lessons, practice graphs, and video playback
│   └── utils/              # JSON serialization and path utilities
├── learning resources/     # Source lessons and practice graphs
├── resources/              # Themes, application icons, and Windows resources
├── LICENSE
└── README.md
```

### Design Highlights

- **Unified stepping model:** Algorithms expose initialization and step-based execution so automatic playback and manual navigation share the same state model.
- **Separated algorithm and UI layers:** Algorithm events are sent to the canvas through callbacks, keeping the algorithm implementation independent from Qt widgets.
- **Template-based graph structures:** `Graph<NodeData, EdgeWeight, Directed>` distinguishes directed and undirected graphs at compile time, while `Tree` adds cycle and multiple-parent validation.
- **Replayable state history:** The UI wrapper stores algorithm steps to support backward and forward navigation.

## .graph File Format

`.graph` files contain UTF-8 encoded JSON:

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

The `type` field is `undirected`, `directed`, or `tree`.

## FAQ

**Why does CMake report that Qt5 or Qt5 Multimedia cannot be found?**
Install the Qt kit matching your compiler and make sure `CMAKE_PREFIX_PATH` points to that kit.

**Why does MSVC report encoding errors such as C4819?**
The repository already enables `/utf-8` and `_USE_MATH_DEFINES`. Use the supplied CMake configuration instead of compiling individual source files directly.

**Why can a video fail to play?**
Video playback depends on codecs available in Windows. The current release does not bundle sample videos; imported files may need to be converted to a supported format.

**Why is Kruskal unavailable for directed graphs?**
A conventional minimum spanning tree is defined for undirected graphs. Use BFS, DFS, or Dijkstra for directed graphs.

**Does Dijkstra support negative edge weights?**
No. The editor limits weights to 1–100 because Dijkstra requires non-negative edge weights. Bellman–Ford is a possible future addition.

**What does an unreachable-node message mean?**
The graph is disconnected, or the node cannot be reached from the selected start node in a directed graph. Traversal correctly covers only the reachable component.

**How can I add another algorithm?**
Implement the step-based interface under `src/algorithms/`, then register the new algorithm in the creation and stepping branches of `GraphCanvas`.

## Contributing and License

- **Author and maintainer:** Zhang Chuan (GitHub: [@sunsky4610-boop](https://github.com/sunsky4610-boop))
- **Issues and pull requests:** Bug reports, suggestions, and contributions are welcome.
- **Roadmap:** Prim, Bellman–Ford, topological sorting, Floyd–Warshall, undo/redo, random graph generation, large-graph rendering optimization, cross-platform validation, unit tests, and internationalization.
- **License:** Distributed under the [MIT License](LICENSE). You may use, modify, and redistribute the project while retaining the copyright notice.

If TreeGraph Studio helps you learn graph algorithms, consider starring the repository.

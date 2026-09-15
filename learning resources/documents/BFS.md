# BFS广度优先搜索算法详解：从入门到精通

## 📋 目录
1. [算法初识：什么是BFS](#一算法初识什么是bfs)
2. [核心思想：层层扩散的搜索策略](#二核心思想层层扩散的搜索策略)
3. [数据结构：队列的关键作用](#三数据结构队列的关键作用)
4. [算法实现：从伪代码到真代码](#四算法实现从伪代码到真代码)
5. [复杂度分析：时空效率](#五复杂度分析时空效率)
6. [经典应用：图论中的多面手](#六经典应用场景)
7. [高级变种：双向BFS与A\*算法](#七高级变种双向bfs与启发式搜索)
8. [实战练习：经典题目推荐](#八实战练习经典题目推荐)

---

## 一、算法初识：什么是BFS

**BFS**（Breadth-First Search，广度优先搜索）是一种图遍历算法，它从起点开始，像"涟漪"一样逐层向外扩展，先访问所有相邻节点，再访问更远层的节点。

### 核心特点
- **完备性**：只要目标节点可达，BFS一定能找到
- **最短路径**：在无权图中，BFS找到的第一条路径就是最短路径
- **空间换时间**：使用队列记住待访问节点

### 生活类比
想象你在一个迷宫中寻找出口：
- **DFS（深度优先）**：一条路走到黑，碰壁再回溯
- **BFS（广度优先）**：同时派出多个探路者，向四面八方均匀扩散

---

## 二、核心思想：层层扩散的搜索策略

### 算法流程图
```
起始节点 → 访问所有邻居 → 访问所有邻居的邻居 → ... → 找到目标
  [Layer0]      [Layer1]              [Layer2]
```

### 关键步骤
1. **初始化**：将起点加入队列，标记已访问
2. **循环处理**：当队列不为空时
   - 取出队首节点
   - 检查是否为目标节点
   - 将所有未访问的邻居加入队列
3. **结束条件**：找到目标或遍历完整个图

---

## 三、数据结构：队列的关键作用

### 为什么必须用队列？
- **FIFO特性**：保证先访问的节点，其邻居也先被处理
- **层序控制**：天然实现"当前层→下一层"的遍历顺序

### 访问标记数组
```python
visited = [False] * n  # 防止重复访问，避免死循环
```

---

## 四、算法实现：从伪代码到真代码

### 伪代码
```plaintext
BFS(graph, start, target):
    queue = [start]
    visited = {start}
    
    while queue 不为空:
        node = queue.pop_front()
        
        if node == target:
            return True
        
        for neighbor in graph[node]:
            if neighbor 不在 visited 中:
                visited.add(neighbor)
                queue.push_back(neighbor)
    
    return False
```

### Python实现（邻接表）
```python
from collections import deque

def bfs(graph, start, target):
    """
    graph: 邻接表字典 {node: [neighbors]}
    start: 起始节点
    target: 目标节点
    """
    if start == target:
        return 0
    
    queue = deque([start])
    visited = {start}
    distance = {start: 0}  # 记录距离
    
    while queue:
        node = queue.popleft()
        
        for neighbor in graph[node]:
            if neighbor not in visited:
                visited.add(neighbor)
                distance[neighbor] = distance[node] + 1
                
                if neighbor == target:
                    return distance[neighbor]
                
                queue.append(neighbor)
    
    return -1  # 无法到达

# 示例用法
graph = {
    'A': ['B', 'C'],
    'B': ['A', 'D', 'E'],
    'C': ['A', 'F'],
    'D': ['B'],
    'E': ['B', 'F'],
    'F': ['C', 'E']
}

print(bfs(graph, 'A', 'F'))  # 输出: 2 (A→C→F)
```

### C++实现（邻接矩阵）
```cpp
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int bfs(const vector<vector<int>>& graph, int start, int target) {
    int n = graph.size();
    vector<bool> visited(n, false);
    vector<int> distance(n, -1);
    
    queue<int> q;
    q.push(start);
    visited[start] = true;
    distance[start] = 0;
    
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        
        if (node == target) {
            return distance[node];
        }
        
        for (int neighbor = 0; neighbor < n; ++neighbor) {
            if (graph[node][neighbor] == 1 && !visited[neighbor]) {
                visited[neighbor] = true;
                distance[neighbor] = distance[node] + 1;
                q.push(neighbor);
            }
        }
    }
    
    return -1;
}

// 示例
int main() {
    vector<vector<int>> graph = {
        {0, 1, 1, 0, 0, 0},
        {1, 0, 0, 1, 1, 0},
        {1, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1},
        {0, 0, 1, 0, 1, 0}
    };
    
    cout << bfs(graph, 0, 5) << endl;  // 输出: 2
    return 0;
}
```

### Java实现（泛型图）
```java
import java.util.*;

public class BFS {
    public static <T> int bfs(Map<T, List<T>> graph, T start, T target) {
        if (start.equals(target)) return 0;
        
        Queue<T> queue = new LinkedList<>();
        Set<T> visited = new HashSet<>();
        Map<T, Integer> distance = new HashMap<>();
        
        queue.offer(start);
        visited.add(start);
        distance.put(start, 0);
        
        while (!queue.isEmpty()) {
            T node = queue.poll();
            
            for (T neighbor : graph.getOrDefault(node, new ArrayList<>())) {
                if (!visited.contains(neighbor)) {
                    visited.add(neighbor);
                    distance.put(neighbor, distance.get(node) + 1);
                    
                    if (neighbor.equals(target)) {
                        return distance.get(neighbor);
                    }
                    
                    queue.offer(neighbor);
                }
            }
        }
        
        return -1;
    }
    
    public static void main(String[] args) {
        Map<String, List<String>> graph = new HashMap<>();
        graph.put("A", Arrays.asList("B", "C"));
        graph.put("B", Arrays.asList("A", "D", "E"));
        graph.put("C", Arrays.asList("A", "F"));
        graph.put("D", Collections.singletonList("B"));
        graph.put("E", Arrays.asList("B", "F"));
        graph.put("F", Arrays.asList("C", "E"));
        
        System.out.println(bfs(graph, "A", "F"));  // 输出: 2
    }
}
```

---

## 五、复杂度分析：时空效率

### 时间复杂度
- **邻接表**：O(V + E)  
  V=顶点数，E=边数
- **邻接矩阵**：O(V²)  
  需要扫描整行找邻居

### 空间复杂度
- **队列**：O(V) 最坏情况存储所有顶点
- **visited数组**：O(V)
- **总计**：O(V)

---

## 六、经典应用场景

### 1. 无权图最短路径
```python
# 迷宫问题：找出最少步数到达出口
def maze_bfs(maze, start, end):
    # 0=通路, 1=墙壁
    directions = [(0,1), (1,0), (0,-1), (-1,0)]
    queue = deque([(start[0], start[1], 0)])
    visited = {start}
    
    while queue:
        x, y, steps = queue.popleft()
        if (x, y) == end:
            return steps
        
        for dx, dy in directions:
            nx, ny = x + dx, y + dy
            if (0 <= nx < len(maze) and 0 <= ny < len(maze[0]) and 
                maze[nx][ny] == 0 and (nx, ny) not in visited):
                visited.add((nx, ny))
                queue.append((nx, ny, steps + 1))
    
    return -1
```

### 2. 社交网络六度空间
找出两个人之间最少需要几个中间人介绍

### 3. 网页爬虫
按深度逐层抓取网页，避免深度递归

### 4. 连通块计数
```python
def count_islands(grid):
    if not grid: return 0
    
    rows, cols = len(grid), len(grid[0])
    visited = set()
    islands = 0
    
    def bfs(r, c):
        queue = deque([(r, c)])
        visited.add((r, c))
        while queue:
            row, col = queue.popleft()
            for dr, dc in [(1,0), (-1,0), (0,1), (0,-1)]:
                nr, nc = row + dr, col + dc
                if (0 <= nr < rows and 0 <= nc < cols and 
                    grid[nr][nc] == 1 and (nr, nc) not in visited):
                    visited.add((nr, nc))
                    queue.append((nr, nc))
    
    for r in range(rows):
        for c in range(cols):
            if grid[r][c] == 1 and (r, c) not in visited:
                bfs(r, c)
                islands += 1
    
    return islands
```

---

## 七、高级变种：双向BFS与启发式搜索

### 双向BFS（Bidirectional BFS）
**适用场景**：知道起点和终点，图很大时

**思想**：从起点和终点同时进行BFS，直到两边相遇

```python
def bidirectional_bfs(graph, start, target):
    if start == target: return 0
    
    # 两个队列和两个访问集合
    queue_start = deque([start])
    queue_target = deque([target])
    visited_start = {start}
    visited_target = {target}
    distance_start = {start: 0}
    distance_target = {target: 0}
    
    while queue_start and queue_target:
        # 从起点扩展
        node = queue_start.popleft()
        for neighbor in graph[node]:
            if neighbor in visited_target:
                return distance_start[node] + distance_target[neighbor] + 1
            if neighbor not in visited_start:
                visited_start.add(neighbor)
                distance_start[neighbor] = distance_start[node] + 1
                queue_start.append(neighbor)
        
        # 从终点扩展
        node = queue_target.popleft()
        for neighbor in graph[node]:
            if neighbor in visited_start:
                return distance_target[node] + distance_start[neighbor] + 1
            if neighbor not in visited_target:
                visited_target.add(neighbor)
                distance_target[neighbor] = distance_target[node] + 1
                queue_target.append(neighbor)
    
    return -1
```

**复杂度**：O(b^(d/2))，比单向BFS的O(b^d)指数级提升

### 0-1 BFS
**适用场景**：边权只有0或1的图

**优化**：使用双端队列，权为0从队首入队，权为1从队尾入队

```python
from collections import deque

def bfs_01(graph, start, target):
    # graph: {node: [(neighbor, weight)]}
    n = len(graph)
    dist = [float('inf')] * n
    dist[start] = 0
    dq = deque([start])
    
    while dq:
        node = dq.popleft()
        for neighbor, weight in graph[node]:
            if dist[node] + weight < dist[neighbor]:
                dist[neighbor] = dist[node] + weight
                if weight == 0:
                    dq.appendleft(neighbor)
                else:
                    dq.append(neighbor)
    
    return dist[target]
```

---

## 八、实战练习：经典题目推荐

### 基础级
1. **[LeetCode 102] 二叉树的层序遍历**
   - 练习队列的基本使用

2. **[LeetCode 200] 岛屿数量**
   - 二维网格中的BFS应用

3. **[LeetCode 994] 腐烂的橘子**
   - 多源BFS经典题

### 进阶级
4. **[LeetCode 127] 单词接龙**
   - 字符串图中的最短路径

5. **[LeetCode 752] 打开转盘锁**
   - 状态空间BFS

6. **[Codeforces 173B] 篮球场**
   - 01 BFS应用

### 大师级
7. **[LeetCode 815] 公交路线**
   - 反向建图 + BFS优化

8. **[Luogu P1144] 最短路计数**
   - BFS + 动态规划

9. **[Kattis] shortestpath1**
   - 无权图最短路径模板

---

## 📌 总结要点

| 特性 | BFS | DFS |
|------|-----|-----|
| **数据结构** | 队列 | 栈/递归 |
| **路径特性** | 最短路径（无权图） | 不一定最短 |
| **空间复杂度** | O(V) | O(V)（递归栈） |
| **适用场景** | 最短路径、连通性 | 拓扑排序、回溯 |

### 记忆口诀
> **BFS三要素**：队列存节点，标记防重复，逐层找目标

---

## 🎓 学习建议

1. **先画图**：手动模拟BFS过程，理解层次遍历
2. **写模板**：熟练掌握邻接表和邻接矩阵两种实现
3. **做变形**：从基础题开始，逐步挑战状态空间BFS
4. **对比学习**：与DFS对比，理解各自优劣

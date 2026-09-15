# Dijkstra算法详解：加权图最短路径的王者

## 📋 目录
1. [算法初识：什么是Dijkstra](#一算法初识什么是dijkstra)
2. [核心思想：贪心策略与松弛操作](#二核心思想贪心策略与松弛操作)
3. [数据结构：优先队列的关键作用](#三数据结构优先队列的关键作用)
4. [算法实现：从伪代码到真代码](#四算法实现从伪代码到真代码)
5. [复杂度分析：不同实现的对比](#五复杂度分析不同实现的对比)
6. [经典应用：现实世界的路径规划](#六经典应用场景)
7. [高级变种：A\*与堆优化](#七高级变种astar与堆优化)
8. [实战练习：经典题目推荐](#八实战练习经典题目推荐)

---

## 一、算法初识：什么是Dijkstra

**Dijkstra算法**（迪杰斯特拉算法）是荷兰计算机科学家Edsger Dijkstra在1956年提出的，用于解决**带权有向图或无向图**中单源最短路径问题的经典算法。

### 核心特点
- **加权图专用**：处理边带权重的图，BFS是无权图版本
- **非负权重**：要求所有边权必须 ≥ 0（不能处理负权边）
- **单源最短**：计算从起点到所有其他节点的最短路径
- **贪心策略**：每次选择当前距离最短的节点进行扩展

### 生活类比
想象你在规划旅行路线：
- **BFS**：假设所有路段耗时相同，按"换乘次数最少"规划
- **Dijkstra**：考虑每条路的实际耗时（高速、国道、山路不同），按"总时间最短"规划

---

## 二、核心思想：贪心策略与松弛操作

### 算法流程图
```
初始化距离 → 选择未处理的最小节点 → 松弛其邻居 → 标记为已处理 → 重复直到所有节点处理完
```

### 关键概念

**1. 距离数组 `dist`**：
- `dist[v]` 表示从起点到节点v的当前最短距离估计值
- 初始时 `dist[start] = 0`，其余为 `∞`

**2. 松弛操作（Relaxation）**：
```python
# 如果通过u到v的距离比当前dist[v]更短，则更新
if dist[u] + weight(u, v) < dist[v]:
    dist[v] = dist[u] + weight(u, v)
```

**3. 贪心选择**：
每次从未处理的节点中选择 `dist` 值最小的节点，认为它的最短路径已确定

### 完整步骤
1. **初始化**：`dist[start] = 0`，其余为 `∞`，所有节点未访问
2. **选择**：从未访问节点中选 `dist` 最小的节点 `u`
3. **标记**：将 `u` 标记为已访问（最短路径已确定）
4. **松弛**：对 `u` 的每个邻居 `v`，执行松弛操作
5. **重复**：步骤2-4直到所有节点访问完或目标节点已访问

---

## 三、数据结构：优先队列的关键作用

### 为什么必须用优先队列？
- **高效选择**：O(log V) 时间获取最小距离节点
- **动态更新**：距离更新后可重新调整堆序
- **避免扫描**：不需要每次线性扫描所有未访问节点

### 三种实现对比

| 实现方式 | 时间复杂度 | 适用场景 |
|----------|------------|----------|
| **数组扫描** | O(V²) | 稠密图（E ≈ V²） |
| **二叉堆** | O(E log V) | 稀疏图（E << V²） |
| **斐波那契堆** | O(E + V log V) | 理论最优，实现复杂 |

---

## 四、算法实现：从伪代码到真代码

### 伪代码
```plaintext
Dijkstra(graph, start):
    dist = array of size V filled with INF
    dist[start] = 0
    priority_queue = [(0, start)]  # (距离, 节点)
    visited = empty set
    
    while priority_queue not empty:
        d, u = pop_min(priority_queue)
        
        if u in visited: continue
        visited.add(u)
        
        if u == target: break  # 可选：提前终止
        
        for each neighbor v of u with weight w:
            if v not in visited:
                new_dist = d + w
                if new_dist < dist[v]:
                    dist[v] = new_dist
                    push(priority_queue, (new_dist, v))
    
    return dist
```

### Python实现（邻接表 + heapq）
```python
import heapq
from collections import defaultdict

def dijkstra(graph, start, target=None):
    """
    graph: 邻接表字典 {node: [(neighbor, weight)]}
    start: 起始节点
    target: 目标节点（可选，提前终止）
    return: 距离字典 {node: shortest_distance}
    """
    # 初始化距离字典
    dist = {node: float('inf') for node in graph}
    dist[start] = 0
    
    # 优先队列: (距离, 节点)
    pq = [(0, start)]
    visited = set()
    
    while pq:
        current_dist, u = heapq.heappop(pq)
        
        # 跳过已访问节点（处理过时条目）
        if u in visited:
            continue
        
        # 提前终止：找到目标即可返回
        if u == target:
            return dist
        
        visited.add(u)
        
        # 松弛操作
        for v, weight in graph[u]:
            if v not in visited:
                new_dist = current_dist + weight
                if new_dist < dist[v]:
                    dist[v] = new_dist
                    heapq.heappush(pq, (new_dist, v))
    
    return dist

# 示例用法
graph = {
    'A': [('B', 4), ('C', 2)],
    'B': [('A', 4), ('C', 1), ('D', 5)],
    'C': [('A', 2), ('B', 1), ('D', 8), ('E', 10)],
    'D': [('B', 5), ('C', 8), ('E', 2), ('F', 6)],
    'E': [('C', 10), ('D', 2), ('F', 3)],
    'F': [('D', 6), ('E', 3)]
}

distances = dijkstra(graph, 'A')
print(distances)
# 输出: {'A': 0, 'B': 3, 'C': 2, 'D': 8, 'E': 10, 'F': 13}
```

### C++实现（邻接表 + priority_queue）
```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <climits>
using namespace std;

class Dijkstra {
private:
    struct Edge {
        int to;
        int weight;
    };
    
    struct Node {
        int dist;
        int vertex;
        
        bool operator>(const Node& other) const {
            return dist > other.dist;
        }
    };

public:
    unordered_map<int, int> shortestPath(
        const unordered_map<int, vector<Edge>>& graph, 
        int start, 
        int target = -1
    ) {
        unordered_map<int, int> dist;
        unordered_map<int, bool> visited;
        priority_queue<Node, vector<Node>, greater<Node>> pq;
        
        // 初始化所有节点距离
        for (const auto& pair : graph) {
            dist[pair.first] = INT_MAX;
        }
        dist[start] = 0;
        pq.push({0, start});
        
        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();
            
            int u = current.vertex;
            int d = current.dist;
            
            if (visited[u]) continue;
            visited[u] = true;
            
            if (u == target && target != -1) break;
            
            if (d > dist[u]) continue; // 过时条目
            
            for (const Edge& edge : graph.at(u)) {
                int v = edge.to;
                int weight = edge.weight;
                
                if (!visited[v]) {
                    int newDist = d + weight;
                    if (newDist < dist[v]) {
                        dist[v] = newDist;
                        pq.push({newDist, v});
                    }
                }
            }
        }
        
        return dist;
    }
};

// 示例
int main() {
    unordered_map<int, vector<Edge>> graph = {
        {0, {{1, 4}, {2, 2}}},
        {1, {{0, 4}, {2, 1}, {3, 5}}},
        {2, {{0, 2}, {1, 1}, {3, 8}, {4, 10}}},
        {3, {{1, 5}, {2, 8}, {4, 2}, {5, 6}}},
        {4, {{2, 10}, {3, 2}, {5, 3}}},
        {5, {{3, 6}, {4, 3}}}
    };
    
    Dijkstra dijkstra;
    auto distances = dijkstra.shortestPath(graph, 0);
    
    for (const auto& pair : distances) {
        cout << "到节点" << pair.first << "的最短距离: " << pair.second << endl;
    }
    return 0;
}
```

### Java实现（邻接表 + PriorityQueue）
```java
import java.util.*;

class Dijkstra {
    static class Edge {
        int to;
        int weight;
        
        Edge(int to, int weight) {
            this.to = to;
            this.weight = weight;
        }
    }
    
    static class Node implements Comparable<Node> {
        int dist;
        int vertex;
        
        Node(int dist, int vertex) {
            this.dist = dist;
            this.vertex = vertex;
        }
        
        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.dist, other.dist);
        }
    }
    
    public Map<Integer, Integer> shortestPath(
        Map<Integer, List<Edge>> graph, 
        int start, 
        Integer target
    ) {
        Map<Integer, Integer> dist = new HashMap<>();
        Set<Integer> visited = new HashSet<>();
        PriorityQueue<Node> pq = new PriorityQueue<>();
        
        // 初始化
        for (int node : graph.keySet()) {
            dist.put(node, Integer.MAX_VALUE);
        }
        dist.put(start, 0);
        pq.offer(new Node(0, start));
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            int u = current.vertex;
            int d = current.dist;
            
            if (visited.contains(u)) continue;
            visited.add(u);
            
            if (target != null && u == target) break;
            
            for (Edge edge : graph.get(u)) {
                int v = edge.to;
                if (!visited.contains(v)) {
                    int newDist = d + edge.weight;
                    if (newDist < dist.get(v)) {
                        dist.put(v, newDist);
                        pq.offer(new Node(newDist, v));
                    }
                }
            }
        }
        
        return dist;
    }
    
    public static void main(String[] args) {
        Map<Integer, List<Edge>> graph = new HashMap<>();
        graph.put(0, Arrays.asList(new Edge(1, 4), new Edge(2, 2)));
        graph.put(1, Arrays.asList(new Edge(0, 4), new Edge(2, 1), new Edge(3, 5)));
        graph.put(2, Arrays.asList(new Edge(0, 2), new Edge(1, 1), new Edge(3, 8), new Edge(4, 10)));
        graph.put(3, Arrays.asList(new Edge(1, 5), new Edge(2, 8), new Edge(4, 2), new Edge(5, 6)));
        graph.put(4, Arrays.asList(new Edge(2, 10), new Edge(3, 2), new Edge(5, 3)));
        graph.put(5, Arrays.asList(new Edge(3, 6), new Edge(4, 3)));
        
        Dijkstra dijkstra = new Dijkstra();
        Map<Integer, Integer> distances = dijkstra.shortestPath(graph, 0, null);
        
        distances.forEach((node, distance) -> 
            System.out.println("到节点" + node + "的最短距离: " + distance)
        );
    }
}
```

### 路径还原实现
```python
def dijkstra_with_path(graph, start, target):
    """
    返回最短距离和路径
    """
    dist = {node: float('inf') for node in graph}
    dist[start] = 0
    
    pq = [(0, start)]
    visited = set()
    prev = {node: None for node in graph}  # 记录前驱节点
    
    while pq:
        current_dist, u = heapq.heappop(pq)
        
        if u in visited:
            continue
        
        if u == target:
            break
        
        visited.add(u)
        
        for v, weight in graph[u]:
            if v not in visited:
                new_dist = current_dist + weight
                if new_dist < dist[v]:
                    dist[v] = new_dist
                    prev[v] = u  # 更新前驱
                    heapq.heappush(pq, (new_dist, v))
    
    # 重建路径
    path = []
    current = target
    while current is not None:
        path.append(current)
        current = prev[current]
    path.reverse()
    
    return dist[target], path if dist[target] != float('inf') else None

# 示例
distance, path = dijkstra_with_path(graph, 'A', 'F')
print(f"最短距离: {distance}, 路径: {path}")
# 输出: 最短距离: 13, 路径: ['A', 'C', 'B', 'D', 'F']
```

---

## 五、复杂度分析：不同实现的对比

### 时间复杂度
| 实现方式 | 建堆 | 每次弹出 | 每次插入 | 总复杂度 | 适用场景 |
|----------|------|----------|----------|----------|----------|
| **数组扫描** | O(V) | O(V) | O(1) | **O(V²)** | 稠密图 |
| **二叉堆** | O(V) | O(log V) | O(log V) | **O(E log V)** | 稀疏图 |
| **斐波那契堆** | O(V) | O(log V) | O(1)均摊 | **O(E + V log V)** | 理论最优 |

- **V**：顶点数量
- **E**：边的数量
- **稀疏图**：E ≈ V（如道路网络）
- **稠密图**：E ≈ V²（如社交网络）

### 空间复杂度
- **距离数组**：O(V)
- **优先队列**：O(V)
- **访问标记**：O(V)
- **总计**：**O(V + E)**

---

## 六、经典应用场景

### 1. 地图导航与路径规划
```python
# 城市道路网络（带距离和通行时间）
road_network = {
    '市中心': [('火车站', 5.2), ('机场', 25.8), ('大学城', 12.0)],
    '火车站': [('市中心', 5.2), ('大学城', 8.5), ('高新区', 15.3)],
    # ... 更多道路
}

# 计算最短驾车距离
distances = dijkstra(road_network, '市中心')
print(f"从市中心到机场: {distances['机场']:.1f}公里")
```

### 2. 网络路由协议
OSPF（开放最短路径优先）协议使用Dijkstra算法计算数据包转发路径

### 3. 项目管理关键路径
```python
# 任务依赖图（权重为任务持续时间）
project_tasks = {
    '需求分析': [('设计', 5), ('原型', 3)],
    '设计': [('前端', 8), ('后端', 10)],
    '原型': [('测试', 2)],
    # ... 更多任务
}

# 计算项目最短完成时间
durations = dijkstra(project_tasks, '需求分析')
```

### 4. 游戏AI路径寻找
即时战略游戏中的单位移动路径计算

---

## 七、高级变种：A\*与堆优化

### 1. A\*算法（启发式Dijkstra）
当知道目标节点时，使用启发函数加速搜索

```python
import heapq

def a_star(graph, start, target, heuristic):
    """
    heuristic: 启发函数 h(v) = 从v到target的估算距离
    """
    dist = {node: float('inf') for node in graph}
    dist[start] = 0
    
    # f(v) = g(v) + h(v)，其中g(v)是实际距离
    pq = [(heuristic(start, target), 0, start)]
    visited = set()
    
    while pq:
        f_score, g_score, u = heapq.heappop(pq)
        
        if u in visited:
            continue
        
        if u == target:
            return g_score
        
        visited.add(u)
        
        for v, weight in graph[u]:
            if v not in visited:
                new_g = g_score + weight
                if new_g < dist[v]:
                    dist[v] = new_g
                    f = new_g + heuristic(v, target)
                    heapq.heappush(pq, (f, new_g, v))
    
    return float('inf')

# 曼哈顿距离启发式（网格地图）
def manhattan(a, b):
    return abs(a[0] - b[0]) + abs(a[1] - b[1])

# 欧几里得距离启发式
def euclidean(a, b):
    return ((a[0] - b[0])**2 + (a[1] - b[1])**2)**0.5
```

### 2. 多源Dijkstra
```python
def multi_source_dijkstra(graph, sources):
    """
    从多个源点同时开始，计算每个节点的最近源点
    """
    dist = {node: float('inf') for node in graph}
    pq = []
    
    for source in sources:
        dist[source] = 0
        heapq.heappush(pq, (0, source))
    
    while pq:
        d, u = heapq.heappop(pq)
        
        if d > dist[u]:
            continue
        
        for v, weight in graph[u]:
            new_dist = d + weight
            if new_dist < dist[v]:
                dist[v] = new_dist
                heapq.heappush(pq, (new_dist, v))
    
    return dist
```

### 3. 带状态压缩的Dijkstra
```python
# 示例：带油量限制的最短路径
def dijkstra_with_fuel(graph, start, target, max_fuel):
    # 状态: (节点, 当前油量)
    dist = defaultdict(lambda: float('inf'))
    dist[(start, max_fuel)] = 0
    
    pq = [(0, start, max_fuel)]
    
    while pq:
        cost, u, fuel = heapq.heappop(pq)
        
        if (cost, u, fuel) != (dist[(u, fuel)], u, fuel):
            continue
        
        if u == target:
            return cost
        
        # 加油（如果在加油站）
        if u in gas_stations:
            new_fuel = max_fuel
            new_cost = cost + fuel_price[u] * (max_fuel - fuel)
            if new_cost < dist[(u, new_fuel)]:
                dist[(u, new_fuel)] = new_cost
                heapq.heappush(pq, (new_cost, u, new_fuel))
        
        # 行驶到邻居
        for v, w in graph[u]:
            if fuel >= w:
                new_fuel = fuel - w
                new_cost = cost
                if new_cost < dist[(v, new_fuel)]:
                    dist[(v, new_fuel)] = new_cost
                    heapq.heappush(pq, (new_cost, v, new_fuel))
    
    return -1
```

---

## 八、实战练习：经典题目推荐

### 基础级
1. **[LeetCode 743] 网络延迟时间**
   - Dijkstra基础模板题

2. **[LeetCode 1514] 概率最大的路径**
   - 将概率转换为负对数权重

3. **[LeetCode 1631] 最小体力消耗路径**
   - 二维网格中的Dijkstra应用

### 进阶级
4. **[LeetCode 1786] 从第一个节点出发到最后一个节点的受限路径数**
   - Dijkstra + 动态规划

5. **[LeetCode 1830] 使字符串有序的最少操作次数**
   - 状态图最短路径

6. **[Codeforces 20C] Dijkstra?**
   - 路径还原与长整型处理

### 大师级
7. **[LeetCode 1368] 使网格图至少有一条有效路径的最小代价**
   - 0-1 BFS与Dijkstra结合

8. **[Luogu P4779] 单源最短路径（标准版）**
   - 高性能Dijkstra模板

9. **[Kattis] shortestpath2**
   - 多次查询优化

---

## 📌 总结要点

| 特性 | Dijkstra | BFS | DFS |
|------|----------|-----|-----|
| **适用图** | 加权非负图 | 无权图 | 任意图 |
| **最短路径** | ✅ 保证最短 | ✅ 保证最短 | ❌ 不保证 |
| **核心思想** | 贪心 + 松弛 | 层次遍历 | 深度遍历 |
| **时间复杂度** | O(E log V) | O(V + E) | O(V + E) |
| **数据结构** | 优先队列 | 普通队列 | 栈/递归 |
| **负权边** | ❌ 不支持 | ✅ 支持 | ✅ 支持 |

### 记忆口诀
> **Dijkstra三步走**：选最近点，标记确定，松弛邻居

---

## ⚠️ 重要注意事项

1. **负权边问题**：Dijkstra不能处理负权边，需要用**Bellman-Ford**或**SPFA**
   ```python
   # 检测到负权环
   if dist[v] > dist[u] + weight and i == V - 1:
       return "存在负权环"
   ```

2. **提前终止**：如果只求到某个目标点的距离，目标出队后即可终止

3. **过时条目**：优先队列中可能包含过时距离值，需要检查跳过

4. **稠密图优化**：当E ≈ V²时，数组实现O(V²)可能比二叉堆更快

---

## 🎓 学习建议

1. **理解贪心**：先掌握贪心思想，再理解为什么Dijkstra有效
2. **手动模拟**：用纸笔画图，手动执行算法过程
3. **对比BFS**：将BFS的队列换成优先队列，理解两者联系
4. **注意边界**：负权边、不连通图、大数处理等特殊情况
5. **实现优化**：掌握至少两种实现（二叉堆和数组扫描）

Dijkstra是算法世界的"导航仪"，掌握好它，你就能在加权图的世界中畅行无阻！
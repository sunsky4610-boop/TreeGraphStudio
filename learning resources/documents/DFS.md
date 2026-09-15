# DFS深度优先搜索算法详解：从入门到精通

## 📋 目录
1. [算法初识：什么是DFS](#一算法初识什么是dfs)
2. [核心思想：深入到底的搜索策略](#二核心思想深入到底的搜索策略)
3. [数据结构：递归与栈的关键作用](#三数据结构递归与栈的关键作用)
4. [算法实现：从伪代码到真代码](#四算法实现从伪代码到真代码)
5. [复杂度分析：时空效率](#五复杂度分析时空效率)
6. [经典应用：图论中的探索者](#六经典应用场景)
7. [高级变种：回溯与剪枝](#七高级变种回溯与剪枝)
8. [实战练习：经典题目推荐](#八实战练习经典题目推荐)

---

## 一、算法初识：什么是DFS

**DFS**（Depth-First Search，深度优先搜索）是一种图遍历算法，它从起点开始，沿着一条路径一直走到底，直到无法继续才回溯到上一个分叉点，尝试其他路径。

### 核心特点
- **探索性**：优先深入探索，而不是广泛搜索
- **回溯机制**：走不通时自动回退到上一步
- **内存高效**：通常只需记录当前路径
- **不保证最短路径**：在无权图中找到的路径不一定最短

### 生活类比
想象你在一个迷宫中寻找出口：
- **DFS（深度优先）**：选择一条路一直走，遇到死胡同才返回上一个岔路口
- **BFS（广度优先）**：同时派出多个探路者，向四面八方均匀扩散

---

## 二、核心思想：深入到底的搜索策略

### 算法流程图
```
起始节点 → 访问第一个邻居 → 访问邻居的第一个邻居 → ... → 无法继续
    ↑                                               ↓
    ←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←←← 回溯
```

### 关键步骤
1. **访问节点**：标记当前节点为已访问
2. **深入探索**：递归访问第一个未访问的邻居
3. **回溯处理**：当当前节点无未访问邻居时，返回上一层
4. **重复探索**：继续访问其他未探索的分支

---

## 三、数据结构：递归与栈的关键作用

### 为什么用递归？
- **天然匹配**：递归的调用栈自动保存路径信息
- **代码简洁**：无需手动管理栈结构
- **回溯方便**：函数返回即自动回溯

### 手动栈实现
当递归深度过大时，可手动实现栈避免栈溢出

```python
# 递归隐式使用系统调用栈
def dfs_recursive(node):
    visited.add(node)
    process(node)
    for neighbor in graph[node]:
        if neighbor not in visited:
            dfs_recursive(neighbor)
    # 函数返回即回溯

# 手动显式栈
def dfs_iterative(start):
    stack = [start]
    visited = {start}
    
    while stack:
        node = stack.pop()
        process(node)
        
        for neighbor in reversed(graph[node]):  # 逆序入栈保持遍历顺序
            if neighbor not in visited:
                visited.add(neighbor)
                stack.append(neighbor)
```

---

## 四、算法实现：从伪代码到真代码

### 伪代码
```plaintext
DFS(graph, node, visited):
    visited.add(node)
    process(node)  # 处理当前节点
    
    for neighbor in graph[node]:
        if neighbor 不在 visited 中:
            DFS(graph, neighbor, visited)  # 递归深入
```

### Python实现（递归版）
```python
def dfs_recursive(graph, start, target, visited=None):
    """
    graph: 邻接表字典 {node: [neighbors]}
    start: 起始节点
    target: 目标节点
    """
    if visited is None:
        visited = set()
    
    visited.add(start)
    
    if start == target:
        return True
    
    for neighbor in graph[start]:
        if neighbor not in visited:
            if dfs_recursive(graph, neighbor, target, visited):
                return True
    
    return False

# 示例用法
graph = {
    'A': ['B', 'C'],
    'B': ['A', 'D', 'E'],
    'C': ['A', 'F'],
    'D': ['B'],
    'E': ['B', 'F'],
    'F': ['C', 'E']
}

print(dfs_recursive(graph, 'A', 'F'))  # 输出: True
```

### Python实现（迭代版+路径记录）
```python
def dfs_iterative(graph, start, target):
    """
    手动栈实现，记录完整路径
    """
    stack = [(start, [start])]  # (节点, 路径)
    visited = set()
    
    while stack:
        node, path = stack.pop()
        
        if node == target:
            return path
        
        if node not in visited:
            visited.add(node)
            
            for neighbor in reversed(graph[node]):
                if neighbor not in visited:
                    stack.append((neighbor, path + [neighbor]))
    
    return None

# 示例
path = dfs_iterative(graph, 'A', 'F')
print(path)  # 输出: ['A', 'C', 'F'] 或 ['A', 'B', 'E', 'F']
```

### C++实现（邻接矩阵）
```cpp
#include <iostream>
#include <vector>
using namespace std;

class DFS {
private:
    vector<vector<int>> graph;
    vector<bool> visited;
    
    void dfs_recursive(int node) {
        visited[node] = true;
        cout << node << " ";
        
        for (int neighbor = 0; neighbor < graph.size(); ++neighbor) {
            if (graph[node][neighbor] == 1 && !visited[neighbor]) {
                dfs_recursive(neighbor);
            }
        }
    }
    
public:
    DFS(const vector<vector<int>>& g) : graph(g) {
        visited.assign(graph.size(), false);
    }
    
    void traverse(int start) {
        dfs_recursive(start);
    }
};

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
    
    DFS dfs(graph);
    dfs.traverse(0);  // 输出: 0 1 3 2 5 4 （具体顺序可能不同）
    return 0;
}
```

### Java实现（泛型图+回溯）
```java
import java.util.*;

public class DFS {
    private Map<String, List<String>> graph;
    private Set<String> visited;
    
    public DFS(Map<String, List<String>> graph) {
        this.graph = graph;
        this.visited = new HashSet<>();
    }
    
    /**
     * 深度优先搜索并记录路径
     * @return 是否找到目标
     */
    public boolean search(String start, String target, List<String> path) {
        visited.add(start);
        path.add(start);
        
        if (start.equals(target)) {
            return true;
        }
        
        for (String neighbor : graph.getOrDefault(start, new ArrayList<>())) {
            if (!visited.contains(neighbor)) {
                if (search(neighbor, target, path)) {
                    return true;
                }
            }
        }
        
        // 回溯：移除当前节点
        path.remove(path.size() - 1);
        return false;
    }
    
    public static void main(String[] args) {
        Map<String, List<String>> graph = new HashMap<>();
        graph.put("A", Arrays.asList("B", "C"));
        graph.put("B", Arrays.asList("A", "D", "E"));
        graph.put("C", Arrays.asList("A", "F"));
        graph.put("D", Collections.singletonList("B"));
        graph.put("E", Arrays.asList("B", "F"));
        graph.put("F", Arrays.asList("C", "E"));
        
        DFS dfs = new DFS(graph);
        List<String> path = new ArrayList<>();
        
        if (dfs.search("A", "F", path)) {
            System.out.println("找到路径: " + path);  // 输出: [A, C, F]
        }
    }
}
```

---

## 五、复杂度分析：时空效率

### 时间复杂度
- **邻接表**：O(V + E)  
  每个节点访问一次，每条边检查一次
- **邻接矩阵**：O(V²)  
  需要扫描整行找邻居

### 空间复杂度
- **递归栈**：O(H) H为图的高度/深度
- **visited数组**：O(V)
- **最坏情况**：O(V)（链状图）

⚠️ **注意**：递归深度过大可能导致栈溢出，此时应使用迭代版

---

## 六、经典应用场景

### 1. 拓扑排序
```python
def topological_sort(graph):
    """
    有向无环图(DAG)的拓扑排序
    """
    visited = set()
    stack = []  # 存储拓扑顺序
    
    def dfs(node):
        visited.add(node)
        for neighbor in graph.get(node, []):
            if neighbor not in visited:
                dfs(neighbor)
        stack.append(node)  # 后序遍历
    
    for node in graph:
        if node not in visited:
            dfs(node)
    
    return stack[::-1]  # 反转得到拓扑序

# 示例：课程安排
courses = {
    'CS101': ['CS201', 'CS202'],
    'CS201': ['CS301'],
    'CS202': ['CS301'],
    'CS301': []
}
print(topological_sort(courses))  # 输出: ['CS101', 'CS202', 'CS201', 'CS301']
```

### 2. 连通分量计数
```python
def count_connected_components(graph):
    visited = set()
    components = 0
    
    def dfs(node):
        visited.add(node)
        for neighbor in graph[node]:
            if neighbor not in visited:
                dfs(neighbor)
    
    for node in graph:
        if node not in visited:
            dfs(node)
            components += 1
    
    return components

# 示例
graph = {
    'A': ['B'],
    'B': ['A'],
    'C': ['D'],
    'D': ['C'],
    'E': []
}
print(count_connected_components(graph))  # 输出: 3
```

### 3. 回溯算法（全排列）
```python
def permutations(nums):
    result = []
    visited = [False] * len(nums)
    
    def dfs(path):
        if len(path) == len(nums):
            result.append(path[:])
            return
        
        for i in range(len(nums)):
            if not visited[i]:
                visited[i] = True
                path.append(nums[i])
                dfs(path)
                # 回溯
                path.pop()
                visited[i] = False
    
    dfs([])
    return result

print(permutations([1, 2, 3]))
# 输出: [[1,2,3],[1,3,2],[2,1,3],[2,3,1],[3,1,2],[3,2,1]]
```

### 4. 二分图检测
```python
def is_bipartite(graph):
    color = {}
    
    def dfs(node, c=0):
        color[node] = c
        for neighbor in graph[node]:
            if neighbor in color:
                if color[neighbor] == c:
                    return False
            else:
                if not dfs(neighbor, 1 - c):
                    return False
        return True
    
    for node in graph:
        if node not in color:
            if not dfs(node):
                return False
    
    return True
```

---

## 七、高级变种：回溯与剪枝

### 1. 记忆化搜索（Memoization）
```python
def dfs_with_memo(node, memo):
    if node in memo:
        return memo[node]
    
    result = 0
    for neighbor in graph[node]:
        result += dfs_with_memo(neighbor, memo)
    
    memo[node] = result
    return result
```

### 2. 剪枝优化
```python
def dfs_with_pruning(path, current, target, best):
    # 剪枝条件1：当前路径已不如最优解
    if current >= best['value']:
        return
    
    # 剪枝条件2：不可能达到目标
    if not can_reach_target(path):
        return
    
    if is_complete(path):
        if current < best['value']:
            best['value'] = current
            best['path'] = path[:]
        return
    
    for choice in get_choices(path):
        if is_valid(path, choice):
            path.append(choice)
            dfs_with_pruning(path, current + cost(choice), target, best)
            path.pop()  # 回溯
```

### 3. 迭代加深DFS（IDDFS）
结合BFS和DFS的优点，限制深度逐步增加

```python
def iddfs(graph, start, target, max_depth):
    for depth in range(max_depth + 1):
        visited = set()
        if dfs_depth_limited(start, target, depth, visited):
            return True
    return False

def dfs_depth_limited(node, target, depth, visited):
    if depth == 0:
        return node == target
    if depth > 0:
        visited.add(node)
        for neighbor in graph[node]:
            if neighbor not in visited:
                if dfs_depth_limited(neighbor, target, depth - 1, visited):
                    return True
    return False
```

---

## 八、实战练习：经典题目推荐

### 基础级
1. **[LeetCode 104] 二叉树的最大深度**
   - 练习递归DFS基础

2. **[LeetCode 136] 只出现一次的数字**
   - 理解DFS遍历思想

3. **[LeetCode 206] 反转链表**
   - 递归DFS的应用

### 进阶级
4. **[LeetCode 78] 子集**
   - 回溯算法入门

5. **[LeetCode 79] 单词搜索**
   - 二维网格DFS

6. **[LeetCode 207] 课程表**
   - 拓扑排序应用

### 大师级
7. **[LeetCode 51] N皇后**
   - 复杂回溯剪枝

8. **[LeetCode 301] 删除无效的括号**
   - 高级DFS + 剪枝

9. **[LeetCode 685] 冗余连接II**
   - DFS在并查集中的应用

---

## 📌 总结要点

| 特性 | DFS | BFS |
|------|-----|-----|
| **数据结构** | 栈/递归 | 队列 |
| **路径特性** | 不保证最短 | 保证最短（无权图）|
| **空间复杂度** | O(H) H=深度 | O(V) |
| **适用场景** | 回溯、拓扑排序、连通性 | 最短路径、层次遍历 |
| **代码风格** | 递归简洁 | 迭代稳健 |

### 记忆口诀
> **DFS三要素**：一路走到底，碰壁就回溯，递归最简洁

---

## 🎓 学习建议

1. **理解递归**：先掌握递归思想，再学DFS事半功倍
2. **画图模拟**：手动画出递归树，理解回溯过程
3. **从简到繁**：先从二叉树遍历开始，再学图DFS
4. **掌握模板**：熟记回溯三步：做选择→递归→撤销选择
5. **注意边界**：时刻警惕递归深度和栈溢出问题

DFS是算法世界的"探险家"，掌握好它，你就能游刃有余地解决回溯、搜索、图论等一大类问题！
# Robot Navigation System (C++)

A comprehensive C++ implementation of grid-based robot navigation featuring multiple pathfinding algorithms, movement-cost modeling, and fully rendered console visualizations. The system operates on a 15×15 environment with obstacles, supports negative edge costs, and includes complete path reconstruction with step-by-step analysis.

---

## Key Features
- **Pathfinding Algorithms**
  - Breadth-First Search (BFS)
  - Depth-First Search (DFS) – Iterative (Stack)
  - Depth-First Search (DFS) – Recursive
  - Bellman-Ford – Supports negative weights and detects negative cycles  
- **Grid Visualization**
  - ANSI-colored output for walls, paths, start/end points
  - Coordinate-based path trace
- **Movement Modeling**
  - Six-directional movement with individual costs (positive & negative)
- **Robust Diagnostics**
  - Cycle detection
  - Unreachable state handling
  - Fully validated path reconstruction

---

## Project Overview
The system loads or auto-generates a 15×15 grid and executes the selected algorithm to navigate from **S (Start)** to **G (Goal)**. Each algorithm updates parent nodes to backtrack the optimal or explored path, and the final output includes:
- Total path cost  
- Number of steps  
- Sequential path coordinates  
- Visual map rendering  

---

## Build & Run
```bash
g++ main.cpp -o navigation
./navigation

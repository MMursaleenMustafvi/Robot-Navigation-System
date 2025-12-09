#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// --- COLORS (ANSI Codes) ---
const string RED_COLOR = "\033[1;31m";    // Wall / Error
const string BLUE_COLOR = "\033[1;34m";   // Path
const string YELLOW_COLOR = "\033[1;33m"; // Start / Goal
const string RESET_COLOR = "\033[0m";     // Reset

// --- CONSTANTS ---
const int ROWS = 15;
const int COLS = 15;
const int MAX_NODES = 225; 
const int INF = 999999;    

// Directions & Costs
int dRow[] = {-1, 1, 0, 0, -1, -1};
int dCol[] = {0, 0, 1, -1, 1, -1};
int dCost[] = {2, -1, 2, -1, 3, -3};

struct Point {
    int r;
    int c;
};

// Global Data
char grid[ROWS][COLS + 1]; 
Point startPos, goalPos;
Point parent[ROWS][COLS]; 
bool visited[ROWS][COLS]; // Global visited for Recursion

// --- DATA STRUCTURES ---

struct Queue {
    Point arr[2000]; 
    int front, rear;
    void init() { front = 0; rear = 0; }
    bool isEmpty() { return front == rear; }
    void push(Point p) { arr[rear++] = p; }
    Point pop() { return arr[front++]; }
};

struct Stack {
    Point arr[2000]; 
    int top;
    void init() { top = -1; }
    bool isEmpty() { return top == -1; }
    void push(Point p) { arr[++top] = p; }
    Point pop() { return arr[top--]; }
};

// --- UTILITY ---

void createGridFile() {
    ofstream outFile("grid.txt");
    outFile << "00000000000000G" << endl;
    outFile << "010000001100000" << endl;
    outFile << "110011001100011" << endl;
    outFile << "000000000110000" << endl;
    outFile << "000011111100011" << endl;
    outFile << "011000000000011" << endl;
    outFile << "011000110011000" << endl;
    outFile << "011011111011000" << endl;
    outFile << "000011111011000" << endl;
    outFile << "010011111011001" << endl;
    outFile << "010000001000001" << endl;
    outFile << "010110001000001" << endl;
    outFile << "010110000011001" << endl;
    outFile << "000110010011001" << endl;
    outFile << "S00110010011001" << endl;
    outFile.close();
}

bool loadGrid() {
    ifstream inFile("grid.txt");
    if (!inFile) return false;
    for (int i = 0; i < ROWS; i++) {
        string line;
        if (getline(inFile, line)) {
            for (int j = 0; j < COLS; j++) {
                grid[i][j] = line[j];
                if (grid[i][j] == 'S') { startPos.r = i; startPos.c = j; }
                if (grid[i][j] == 'G') { goalPos.r = i; goalPos.c = j; }
            }
        }
    }
    inFile.close();
    return true;
}

bool isValid(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return false;
    if (grid[r][c] == '1') return false; 
    return true;
}

void resetGlobals() {
    for(int i=0; i<ROWS; i++) {
        for(int j=0; j<COLS; j++) {
            parent[i][j] = {-1, -1};
            visited[i][j] = false;
        }
    }
}

// --- DISPLAY ---

void printSeparator() {
    cout << " +---------------------------------------------------------+" << endl;
}

void printHeader(string title) {
    cout << "\n";
    printSeparator();
    cout << " | " << title << endl;
    printSeparator();
}

void printResult(Point path[], int length, int totalCost, bool success, string message = "") {
    if (!success) {
        cout << " | STATUS: " << RED_COLOR << "FAILED" << RESET_COLOR << endl;
        if(message != "") cout << " | Reason: " << message << endl;
        printSeparator();
        return;
    }

    cout << " | STATUS: " << BLUE_COLOR << "SUCCESS" << RESET_COLOR << endl;
    cout << " | Total Cost: " << totalCost << endl;
    cout << " | Steps Taken: " << length -1 << endl;
    cout << " | Path: ";
    for (int i = 0; i < length; i++) {
        cout << "(" << path[i].r + 1 << "," << path[i].c + 1 << ")";
        if (i < length - 1) cout << " -> ";
        if ((i+1) % 5 == 0 && i != length-1) cout << "\n |       ";
    }
    cout << endl;
    printSeparator();

    // Visual Grid
    char dispGrid[ROWS][COLS];
    for(int i=0; i<ROWS; i++) {
        for(int j=0; j<COLS; j++) {
            dispGrid[i][j] = (grid[i][j] == '1') ? '1' : '.';
        }
    }
    
    for (int i = 1; i < length - 1; i++) {
        dispGrid[path[i].r][path[i].c] = '*';
    }

    cout << "\n   [ VISUAL GRID ]\n";
    cout << "   (" << RED_COLOR << "1=Wall" << RESET_COLOR << ", " 
         << BLUE_COLOR << "*=Path" << RESET_COLOR << ", " 
         << YELLOW_COLOR << "S=Start , G=Ends" << RESET_COLOR << ")\n\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "   ";
        for (int j = 0; j < COLS; j++) {
            char cell = dispGrid[i][j];
            
            if (i == startPos.r && j == startPos.c) {
                cout << YELLOW_COLOR << "S" << RESET_COLOR << " ";
            } else if (i == goalPos.r && j == goalPos.c) {
                cout << YELLOW_COLOR << "G" << RESET_COLOR << " ";
            } else if (cell == '1') {
                cout << RED_COLOR << "1" << RESET_COLOR << " ";
            } else if (cell == '*') {
                cout << BLUE_COLOR << "*" << RESET_COLOR << " ";
            } else {
                cout << "0 ";
            }
        }
        cout << endl;
    }
    cout << endl;
}

void reconstructPath(string algoName, int finalCost) {
    printHeader(algoName);
    Point path[2000];
    int length = 0;
    Point curr = goalPos;

    if (parent[curr.r][curr.c].r == -1) {
        printResult(path, 0, 0, false, "Goal unreachable");
        return;
    }

    while (curr.r != -1) {
        if (length >= 1000) {
             printResult(path, 0, 0, false, "Loop detected in path reconstruction");
             return;
        }
        path[length++] = curr;
        Point p = parent[curr.r][curr.c];
        if (curr.r == startPos.r && curr.c == startPos.c) break;
        curr = p;
    }

    // Reverse
    for (int i = 0; i < length / 2; i++) {
        Point temp = path[i];
        path[i] = path[length - 1 - i];
        path[length - 1 - i] = temp;
    }

    // Calculate actual cost from path
    int actualCost = 0;
    for (int i = 0; i < length - 1; i++) {
        Point from = path[i];
        Point to = path[i + 1];
        for (int k = 0; k < 6; k++) {
            if (from.r + dRow[k] == to.r && from.c + dCol[k] == to.c) {
                actualCost += dCost[k];
                break;
            }
        }
    }

    printResult(path, length, actualCost, true);
}

// --- ALGORITHMS ---

// 1. BFS
void runBFS() {
    resetGlobals();
    Queue q; q.init();
    q.push(startPos);
    visited[startPos.r][startPos.c] = true;

    while (!q.isEmpty()) {
        Point curr = q.pop();
        if (curr.r == goalPos.r && curr.c == goalPos.c) break;

        for (int k = 0; k < 6; k++) {
            int nr = curr.r + dRow[k];
            int nc = curr.c + dCol[k];
            if (isValid(nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[nr][nc] = curr;
                q.push({nr, nc});
            }
        }
    }
    reconstructPath("BFS (Breadth First Search)", 0); 
}

// 2. DFS Iterative (Stack)
void runDFSIterative() {
    resetGlobals();
    Stack s; s.init();
    s.push(startPos);

    while (!s.isEmpty()) {
        Point curr = s.pop();
        
        if (!visited[curr.r][curr.c]) {
            visited[curr.r][curr.c] = true;
            if (curr.r == goalPos.r && curr.c == goalPos.c) break;

            for (int k = 0; k < 6; k++) {
                int nr = curr.r + dRow[k];
                int nc = curr.c + dCol[k];
                if (isValid(nr, nc) && !visited[nr][nc]) {
                    parent[nr][nc] = curr;
                    s.push({nr, nc});
                }
            }
        }
    }
    reconstructPath("DFS (Iterative / Stack)", 0);
}

// 3. DFS Recursive
bool dfsRecursiveHelper(Point curr) {
    visited[curr.r][curr.c] = true;
    
    if (curr.r == goalPos.r && curr.c == goalPos.c) return true;

    for (int k = 0; k < 6; k++) {
        int nr = curr.r + dRow[k];
        int nc = curr.c + dCol[k];

        if (isValid(nr, nc) && !visited[nr][nc]) {
            parent[nr][nc] = curr;
            if (dfsRecursiveHelper({nr, nc})) return true;
        }
    }
    return false;
}

void runDFSRecursive() {
    resetGlobals();
    dfsRecursiveHelper(startPos);
    reconstructPath("DFS (Recursive)", 0);
}

// 4. Bellman-Ford (With Negative Cycle Detection)
void runBellmanFord() {
    resetGlobals();
    int dist[ROWS][COLS];
    
    for(int i=0; i<ROWS; i++) 
        for(int j=0; j<COLS; j++) dist[i][j] = INF;

    dist[startPos.r][startPos.c] = 0;
    int V = ROWS * COLS; 

    // Step 1: Relaxation (V-1 times)
    for (int i = 0; i < V - 1; i++) {
        bool changed = false;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (dist[r][c] == INF) continue; 

                for (int k = 0; k < 6; k++) {
                    int nr = r + dRow[k];
                    int nc = c + dCol[k];
                    int w = dCost[k];

                    if (isValid(nr, nc)) {
                        if (dist[r][c] + w < dist[nr][nc]) {
                            dist[nr][nc] = dist[r][c] + w;
                            parent[nr][nc] = {r, c};
                            changed = true;
                        }
                    }
                }
            }
        }
        if (!changed) break; 
    }

    // Step 2: Negative Cycle Detection (V-th Iteration)
    bool negativeCycle = false;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (dist[r][c] == INF) continue;

            for (int k = 0; k < 6; k++) {
                int nr = r + dRow[k];
                int nc = c + dCol[k];
                int w = dCost[k];

                if (isValid(nr, nc)) {
                    if (dist[r][c] + w < dist[nr][nc]) {
                        negativeCycle = true;
                        // To highlight where cycle is
                        cout << RED_COLOR << " [!] Cycle detected at (" 
                             << nr+1 << "," << nc+1 << ")" << RESET_COLOR << endl;
                    }
                }
            }
        }
    }

    if (negativeCycle) {
        printHeader("Bellman-Ford Algorithm");
        Point empty[1];
        printResult(empty, 0, 0, false, "Negative Cost Cycle Detected! Algorithm Stopped.");
    } else {
        int finalCost = dist[goalPos.r][goalPos.c];
        if (finalCost == INF) {
             printHeader("Bellman-Ford Algorithm");
             Point empty[1];
             printResult(empty, 0, 0, false, "Destination Unreachable");
        } else {
             reconstructPath("Bellman-Ford Algorithm", finalCost);
        }
    }
}

//main

int main() {
    createGridFile(); 
    if (!loadGrid()) {
        cout << "Error: Unable to load grid.txt!" << endl;
        return 1;
    }

    string menuColor = "\033[1;36m"; 
    int choice;
    do {
        cout << "\n";
        cout << menuColor << "  #############################################" << RESET_COLOR << "\n";
        cout << menuColor << "  #         ROBOT NAVIGATION SYSTEM           #" << RESET_COLOR << "\n";
        cout << menuColor << "  #        Grid Size: 15x15 (Nodes)           #" << RESET_COLOR << "\n";
        cout << menuColor << "  #############################################" << RESET_COLOR << "\n";
        cout << "  1. Run BFS (Breadth First Search)\n";
        cout << "  2. Run DFS (Iterative using Stack)\n";
        cout << "  3. Run DFS (Recursive)\n";
        cout << "  4. Run Bellman-Ford (Shortest Path)\n";
        cout << "  5. Check Dijkstra\n";
        cout << "  6. Check Kruskal\n";
        cout << "  7. Check Prim\n";
        cout << "  0. Exit\n";
        cout << menuColor << "  #############################################" << RESET_COLOR << "\n";
        cout << "  Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: runBFS(); break;
            case 2: runDFSIterative(); break;
            case 3: runDFSRecursive(); break;
            case 4: runBellmanFord(); break;
            case 5: 
                printHeader("Dijkstra's Algorithm");
                cout << " | STATUS: NOT APPLICABLE\n | REASON: Negative Edge Costs Exist.\n";
                printSeparator(); break;
            case 6: 
                printHeader("Kruskal's Algorithm");
                cout << " | STATUS: NOT APPLICABLE\n | REASON: Target is Pathfinding, not MST.\n";
                printSeparator(); break;
            case 7: 
                printHeader("Prim's Algorithm");
                cout << " | STATUS: NOT APPLICABLE\n | REASON: Target is Pathfinding, not MST.\n";
                printSeparator(); break;
            case 0: cout << "\n  Exiting...\n"; break;
            default: cout << "\n  Invalid Choice.\n";
        }

    } while (choice != 0);

    return 0;
}
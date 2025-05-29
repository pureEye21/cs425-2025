#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n));
    
    // Initialize the next hop table
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                nextHop[i][j] = -1; // No hop needed for self
            } else if (graph[i][j] != INF && graph[i][j] != 0) {
                nextHop[i][j] = j; // Direct neighbor
            } else {
                nextHop[i][j] = -1; // No path known yet
            }
        }
    }
    
    // Print initial tables
    cout << "--- DVR Initial Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
    
    bool changed;
    int iteration = 1;
    
    do {
        changed = false;
        
        // Each node shares its distance vector with neighbors
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && graph[i][j] != INF && graph[i][j] != 0) { // For each neighbor
                    for (int k = 0; k < n; ++k) { // For each destination
                        // If there's a better path through neighbor j
                        if (dist[i][k] > graph[i][j] + dist[j][k] && dist[j][k] != INF) {
                            dist[i][k] = graph[i][j] + dist[j][k];
                            nextHop[i][k] = j;
                            changed = true;
                        }
                    }
                }
            }
        }
        
        if (changed) {
            cout << "--- DVR Iteration " << iteration++ << " ---\n";
            for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
        }
        
    } while (changed);
    
    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
            hop = prev[hop];
        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;
        
        // Dijkstra's algorithm for LSR
        for (int count = 0; count < n; ++count) {
            // Find vertex with minimum distance
            int u = -1;
            int minDist = INF;
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && dist[v] < minDist) {
                    minDist = dist[v];
                    u = v;
                }
            }
            
            // If we can't find a vertex, break
            if (u == -1) break;
            
            // Mark the vertex as visited
            visited[u] = true;
            
            // Update distances of adjacent vertices
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && graph[u][v] != 0 && graph[u][v] != INF && 
                    dist[u] + graph[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                    prev[v] = u;
                }
            }
        }
        
        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}
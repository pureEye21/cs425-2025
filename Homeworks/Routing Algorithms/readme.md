# Routing Protocols Simulation

## Overview
This project implements and simulates Distance Vector Routing (DVR) and Link State Routing (LSR) algorithms for computer networks. The simulation takes a network topology as input in the form of an adjacency matrix and computes the optimal routing tables for each node in the network.

## Algorithms Implemented
1. **Distance Vector Routing (DVR)** - Based on the Bellman-Ford algorithm, where each node shares its routing table with neighbors until convergence.
2. **Link State Routing (LSR)** - Based on Dijkstra's algorithm, where each node computes the shortest path to all other nodes.

## Files
- `routing_sim.cpp`: Main source code implementing the DVR and LSR algorithms
- `input1.txt`: Sample input file with network topology
- `README.md`: This documentation file

## Requirements
- C++ compiler with C++11 support
- Standard C++ libraries (iostream, vector, queue, etc.)

## Compilation
To compile the program, use:
```bash
g++ -o routing_sim routing_sim.cpp -std=c++11
```

## Usage
Run the executable with an input file as an argument:
```bash
./routing_sim input1.txt
```

## Input Format
The input file should contain:
- First line: Integer `n` representing the number of nodes in the network
- Next `n` lines: Each line contains `n` integers representing the adjacency matrix
  - `0` indicates no direct connection between nodes (or self)
  - `9999` (INF) indicates an unreachable link
  - Any other value represents the cost of the link

Example input file:
```
4
0 10 100 30
10 0 20 40
100 20 0 10
30 40 10 0
```

## Output Format
The program outputs the routing tables for each node:
- For DVR: Initial tables, tables after each iteration, and final tables
- For LSR: Final routing tables

Each table shows:
- Destination node
- Cost to reach the destination
- Next hop node

## Implementation Details

### Distance Vector Routing
The implementation follows these steps:
1. Initialize distance and next hop tables based on direct connections
2. Iteratively update distance vectors by sharing information with neighbors
3. Continue until no further updates are made (convergence)

#### Key Code Components:
- **Initialization**:
  ```cpp
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
  ```

- **Bellman-Ford Update Logic**:
  ```cpp
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
  ```

- **Convergence Check**:
  The `changed` boolean flag is used to track if any updates were made in an iteration. The algorithm continues until no further updates are made, indicating convergence.

### Link State Routing
The implementation uses Dijkstra's algorithm:
1. For each source node, compute shortest paths to all other nodes
2. Track both distances and previous nodes in the path
3. Generate routing tables with next hop information

#### Key Code Components:
- **Dijkstra's Algorithm Core**:
  ```cpp
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
  ```

- **Path Reconstruction for Next Hop**:
  ```cpp
  // Trace back the path to find the next hop
  int hop = i;
  while (prev[hop] != src && prev[hop] != -1)
      hop = prev[hop];
  cout << (prev[hop] == -1 ? -1 : hop) << endl;
  ```
  This code reconstructs the path from source to destination to determine the next hop. It follows the `prev` array backwards until it finds the first node after the source.

## Notes
- The constant `INF` (9999) represents an unreachable link
- The simulation handles symmetric and asymmetric link costs
- For nodes with no path to a destination, the next hop is shown as "-1"

## Function Explanations

### `void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop)`
Prints the routing table for a specific node in the DVR algorithm.
- `node`: The node whose routing table is being printed
- `table`: Matrix containing the distance/cost values
- `nextHop`: Matrix containing the next hop node information

### `void simulateDVR(const vector<vector<int>>& graph)`
Simulates the Distance Vector Routing algorithm.
- Initializes the distance and next hop tables
- Implements the Bellman-Ford algorithm for distributed routing
- Iterates until convergence
- Prints routing tables after each iteration where changes occur

### `void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev)`
Prints the routing table for a specific node in the LSR algorithm.
- `src`: Source node whose routing table is being printed
- `dist`: Vector containing the shortest distances to all nodes
- `prev`: Vector containing the previous node in the shortest path

### `void simulateLSR(const vector<vector<int>>& graph)`
Simulates the Link State Routing algorithm.
- For each node, runs Dijkstra's algorithm to compute shortest paths
- Handles path reconstruction to determine next hops
- Prints the final routing tables

### `vector<vector<int>> readGraphFromFile(const string& filename)`
Reads the network topology from a file.
- Parses the adjacency matrix from the input file
- Returns a 2D vector representing the graph

### `int main(int argc, char *argv[])`
The main function that:
- Processes command-line arguments
- Reads the network topology from the specified file
- Runs both routing simulations and displays results

## Time Complexity Analysis

### Distance Vector Routing
- Time Complexity: O(n³ × k), where n is the number of nodes and k is the number of iterations until convergence
- Space Complexity: O(n²) for storing the distance and next hop tables

### Link State Routing
- Time Complexity: O(n³) for running Dijkstra's algorithm for each node
- Space Complexity: O(n²) for storing the graph and O(n) for auxiliary data structures

## Potential Improvements
- Implementation of split horizon and poison reverse mechanisms for DVR to prevent routing loops
- Priority queue implementation for Dijkstra's algorithm to improve LSR efficiency
- Support for dynamic topology changes during simulation

#include "Graph.h"

Graph::Graph() : cityLookup(100), cityCount(0) {}

void Graph::addLocation(std::string name) {
    if (cityLookup.contains(name)) return;
    cityLookup.insert(name, cityCount);
    cities.push_back(name);
    
    // Add empty list for this new city
    CustomLinkedList<Edge> edges;
    adjList.push_back(edges);
    
    cityCount++;
}

void Graph::addRoute(std::string from, std::string to, int weight) {
    if (!cityLookup.contains(from) || !cityLookup.contains(to)) {
        std::cout << "One or both locations do not exist." << std::endl;
        return;
    }
    
    int u = cityLookup.get(from);
    int v = cityLookup.get(to);
    
    // Add edge u -> v
    adjList[u].append(Edge(v, weight));
    // Add edge v -> u (Undirected graph assumed for roads, or typical map)
    // If directed, remove this line. Prompt says "Roads", usually 2-way but "blockRoad" implies directionality might exist?
    // Let's assume undirected for simplicity unless specified.
    adjList[v].append(Edge(u, weight));
}

void Graph::blockRoad(std::string from, std::string to) {
    if (!cityLookup.contains(from) || !cityLookup.contains(to)) return;
    
    int u = cityLookup.get(from);
    int v = cityLookup.get(to);
    
    // Iterate through list of u to find v and update weight to INF or delete
    // Since Linked List implementation doesn't allow easy arbitrary access/modification without traversal:
    // We didn't impl 'find' or iterator in CustomLinkedList.
    // I need to traverse manually.
    Node<Edge>* curr = adjList[u].getHead();
    while (curr) {
        if (curr->data.toIndex == v) {
            curr->data.weight = INF; // Soft block
            std::cout << "Road blocked between " << from << " and " << to << std::endl;
            // Also block v -> u
             Node<Edge>* curr2 = adjList[v].getHead();
             while (curr2) {
                 if (curr2->data.toIndex == u) {
                     curr2->data.weight = INF;
                     break;
                 }
                 curr2 = curr2->next;
             }
            return;
        }
        curr = curr->next;
    }
}

struct DijkstraNode {
    int u;
    int dist;
    
    // MinHeap needs < operator. We want SMALLER dist to have HIGHER priority (be at root).
    // MinHeap implementation puts smallest value at root.
    // So if dist A < dist B, then A < B should be true.
    bool operator<(const DijkstraNode& other) const {
        return dist < other.dist;
    }
};

void Graph::runDijkstra(std::string startCity, std::string endCity) {
     if (!cityLookup.contains(startCity) || !cityLookup.contains(endCity)) {
        std::cout << "Invalid cities." << std::endl;
        return;
    }
    
    int start = cityLookup.get(startCity);
    int end = cityLookup.get(endCity);
    
    CustomVector<int> dist;
    CustomVector<int> parent;
    // Resize vectors manually or push_back checks
    // We don't have 'resize' public. push_back INF for all cities.
    // Or assume max cities.
    // Let's just create array pointers since CustomVector is limited?
    // No, let's use CustomVector.
    
    // Initialize
    for(int i=0; i<cityCount; i++) {
        dist.push_back(INF);
        parent.push_back(-1);
    }
    
    dist[start] = 0;
    
    MinHeap<DijkstraNode> pq;
    pq.insert({start, 0});
    
    while (!pq.isEmpty()) {
        DijkstraNode top = pq.extractMin();
        int u = top.u;
        int d = top.dist;
        
        if (d > dist[u]) continue;
        if (u == end) break; // Found destination
        
        Node<Edge>* curr = adjList[u].getHead();
        while (curr) {
            int v = curr->data.toIndex;
            int weight = curr->data.weight;
            
            if (dist[u] + weight < dist[v] && weight != INF) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.insert({v, dist[v]});
            }
            curr = curr->next;
        }
    }
    
    if (dist[end] == INF) {
        std::cout << "No route found from " << startCity << " to " << endCity << std::endl;
    } else {
        std::cout << "Shortest Path Cost: " << dist[end] << std::endl;
        std::cout << "Path: ";
        // Reconstruct path
        CustomStack<int> pathStack;
        int curr = end;
        while (curr != -1) {
            pathStack.push(curr);
            curr = parent[curr];
        }
        
        while (!pathStack.isEmpty()) {
            std::cout << cities[pathStack.pop()];
            if (!pathStack.isEmpty()) std::cout << " -> ";
        }
        std::cout << std::endl;
    }
}

#include "Graph.h"

Graph::Graph() : cityLookup(100), cityCount(0) {}

void Graph::addLocation(string name) {
    if (cityLookup.contains(name)) return;
    cityLookup.insert(name, cityCount);
    cities.push_back(name);
    
    // Add empty list for this new city
    CustomLinkedList<Edge> edges;
    adjList.push_back(edges);
    
    cityCount++;
}

void Graph::addRoute(string from, string to, int weight) {
    if (!cityLookup.contains(from) || !cityLookup.contains(to)) {
        cout << "One or both locations do not exist." << endl;
        return;
    }
    
    int u = cityLookup.get(from);
    int v = cityLookup.get(to);
    
    adjList[u].append(Edge(v, weight));
    adjList[v].append(Edge(u, weight));
}

void Graph::blockRoad(string from, string to) {
    if (!cityLookup.contains(from) || !cityLookup.contains(to)) return;
    
    int u = cityLookup.get(from);
    int v = cityLookup.get(to);
    
    Node<Edge>* curr = adjList[u].getHead();
    while (curr) {
        if (curr->data.toIndex == v) {
            curr->data.weight = INF; 
            cout << "Road blocked between " << from << " and " << to << endl;
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
    
    bool operator<(const DijkstraNode& other) const {
        return dist < other.dist;
    }
};

void Graph::runDijkstra(string startCity, string endCity) {
     if (!cityLookup.contains(startCity) || !cityLookup.contains(endCity)) {
        cout << "Invalid cities." << endl;
        return;
    }
    
    int start = cityLookup.get(startCity);
    int end = cityLookup.get(endCity);
    
    CustomVector<int> dist;
    CustomVector<int> parent;
    
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
        if (u == end) break; 
        
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
        cout << "No route found from " << startCity << " to " << endCity << endl;
    } else {
        cout << "Shortest Path Cost: " << dist[end] << endl;
        cout << "Path: ";
        CustomStack<int> pathStack;
        int curr = end;
        while (curr != -1) {
            pathStack.push(curr);
            curr = parent[curr];
        }
        
        while (!pathStack.isEmpty()) {
            cout << cities[pathStack.pop()];
            if (!pathStack.isEmpty()) cout << " -> ";
        }
        cout << endl;
    }
}

void Graph::loadGraph(string filename) {
    ifstream inFile(filename);
    if (!inFile) {
        // Fallback to default if no file
        addLocation("A"); addLocation("B"); addLocation("C"); addLocation("D"); addLocation("E");
        addRoute("A", "B", 10); addRoute("A", "C", 5);
        addRoute("B", "D", 20); addRoute("C", "B", 8);
        addRoute("C", "E", 15); addRoute("D", "E", 5);
        return;
    }
    
    string type;
    while (inFile >> type) {
        if (type == "CITY") {
            string name;
            inFile >> name;
            addLocation(name);
        } else if (type == "ROUTE") {
            string u, v;
            int w;
            inFile >> u >> v >> w;
            addRoute(u, v, w);
        }
    }
    inFile.close();
}

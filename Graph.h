#ifndef GRAPH_H
#define GRAPH_H

#include "structures.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct Edge {
    int toIndex;
    int weight;
    
    Edge(int t, int w) : toIndex(t), weight(w) {}
    bool operator==(const Edge& other) const {
        return toIndex == other.toIndex;
    }
};

class Graph {
private:
    HashMap<string, int> cityLookup;
    CustomVector<string> cities;
    CustomVector<CustomLinkedList<Edge>> adjList;
    int cityCount;
    
    const int INF = 1000000000;

public:
    Graph();
    void addLocation(string name);
    void addRoute(string from, string to, int weight);
    void blockRoad(string from, string to);
    void runDijkstra(string startCity, string endCity);
    
    // File I/O
    void loadGraph(string filename);
};

#endif // GRAPH_H

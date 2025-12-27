#ifndef GRAPH_H
#define GRAPH_H

#include "structures.h"
#include <string>
#include <iostream>

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
    HashMap<std::string, int> cityLookup;
    CustomVector<std::string> cities;
    CustomVector<CustomLinkedList<Edge>> adjList;
    int cityCount;
    
    const int INF = 1000000000;

public:
    Graph();
    void addLocation(std::string name);
    void addRoute(std::string from, std::string to, int weight);
    void blockRoad(std::string from, std::string to);
    void runDijkstra(std::string startCity, std::string endCity);
};

#endif // GRAPH_H

#ifndef GRAPH_H
#define GRAPH_H

#include "structures.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

struct Edge
{
    int toIndex;
    int weight;
    bool blocked; // Track if road is blocked

    Edge(int t, int w) : toIndex(t), weight(w), blocked(false) {}
    bool operator==(const Edge &other) const
    {
        return toIndex == other.toIndex;
    }
};

class Graph
{
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
    void unblockRoad(string from, string to, int weight); // For undo
    int getRoadWeight(string from, string to);            // Get current weight
    void runDijkstra(string startCity, string endCity);

    // Validation helpers
    bool cityExists(string name) const;
    CustomVector<string> getAllCities() const;
    int getCityCount() const;

    // File I/O
    void loadGraph(string filename);
    void saveGraph(string filename);
};

#endif // GRAPH_H

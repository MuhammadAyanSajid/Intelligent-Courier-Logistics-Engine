#ifndef GRAPH_H
#define GRAPH_H

#include "structures.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

// Route result structure for multiple paths (simple data container - keep as struct)
struct RouteResult
{
    CustomVector<string> path;
    int totalCost;
    bool valid;

    RouteResult() : totalCost(0), valid(false) {}
};

// Edge structure (simple data container - keep as struct)
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

// Zone class for organizing destinations
class Zone
{
private:
    string name;
    CustomVector<string> cities;

public:
    Zone() : name("") {}
    Zone(string n) : name(n) {}

    // Getters
    string getName() const { return name; }
    const CustomVector<string> &getCities() const { return cities; }

    // Setters
    void setName(string n) { name = n; }

    void addCity(string city)
    {
        if (!cities.contains(city))
        {
            cities.push_back(city);
        }
    }

    bool containsCity(string city) const
    {
        return cities.contains(city);
    }

    int getCityCount() const { return cities.getSize(); }
};

class Graph
{
private:
    HashMap<string, int> cityLookup;
    CustomVector<string> cities;
    CustomVector<CustomLinkedList<Edge>> adjList;
    HashMap<string, Zone> zones;        // Zone management
    HashMap<string, string> cityToZone; // City to zone mapping
    int cityCount;

    const int INF = 1000000000;

    // Helper for K-shortest paths
    RouteResult dijkstraWithExclusion(int start, int end,
                                      CustomVector<int> &excludedEdgesFrom,
                                      CustomVector<int> &excludedEdgesTo);

public:
    Graph();
    void addLocation(string name);
    void addRoute(string from, string to, int weight);
    void blockRoad(string from, string to);
    void unblockRoad(string from, string to, int weight); // For undo
    int getRoadWeight(string from, string to);            // Get current weight
    void runDijkstra(string startCity, string endCity);

    // K-Shortest Paths (Yen's Algorithm)
    CustomVector<RouteResult> findKShortestPaths(string startCity, string endCity, int k = 3);
    void displayAlternativeRoutes(string startCity, string endCity, int k = 3);

    // Zone Management
    void addZone(string zoneName);
    void assignCityToZone(string city, string zoneName);
    string getCityZone(string city);
    CustomVector<string> getCitiesInZone(string zoneName);
    CustomVector<string> getAllZones();
    void autoAssignZones(); // Auto-assign based on city names or position

    // Validation helpers
    bool cityExists(string name) const;
    CustomVector<string> getAllCities() const;
    int getCityCount() const;
    bool isRoadBlocked(string from, string to);

    // File I/O
    void loadGraph(string filename);
    void saveGraph(string filename);
};

#endif // GRAPH_H

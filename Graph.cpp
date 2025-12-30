#include "Graph.h"

Graph::Graph() : cityLookup(100), zones(20), cityToZone(100), cityCount(0) {}

void Graph::addLocation(string name)
{
    if (cityLookup.contains(name))
        return;
    cityLookup.insert(name, cityCount);
    cities.push_back(name);

    // Add empty list for this new city
    CustomLinkedList<Edge> edges;
    adjList.push_back(edges);

    cityCount++;
}

void Graph::addRoute(string from, string to, int weight)
{
    if (!cityLookup.contains(from) || !cityLookup.contains(to))
    {
        cout << "One or both locations do not exist." << endl;
        return;
    }

    int u = cityLookup.get(from);
    int v = cityLookup.get(to);

    // Check if edge already exists to avoid duplicates
    Node<Edge> *curr = adjList[u].getHead();
    while (curr)
    {
        if (curr->data.toIndex == v)
        {
            // Update weight if route exists
            curr->data.weight = weight;
            curr->data.blocked = false;
            // Update reverse edge too
            Node<Edge> *curr2 = adjList[v].getHead();
            while (curr2)
            {
                if (curr2->data.toIndex == u)
                {
                    curr2->data.weight = weight;
                    curr2->data.blocked = false;
                    break;
                }
                curr2 = curr2->next;
            }
            return;
        }
        curr = curr->next;
    }

    adjList[u].append(Edge(v, weight));
    adjList[v].append(Edge(u, weight));
}

void Graph::blockRoad(string from, string to)
{
    if (!cityLookup.contains(from) || !cityLookup.contains(to))
    {
        cout << "Invalid cities: " << from << " or " << to << " not found." << endl;
        return;
    }

    int u = cityLookup.get(from);
    int v = cityLookup.get(to);

    bool found = false;
    Node<Edge> *curr = adjList[u].getHead();
    while (curr)
    {
        if (curr->data.toIndex == v)
        {
            curr->data.blocked = true;
            curr->data.weight = INF;
            found = true;
            break;
        }
        curr = curr->next;
    }

    // Block reverse direction too
    Node<Edge> *curr2 = adjList[v].getHead();
    while (curr2)
    {
        if (curr2->data.toIndex == u)
        {
            curr2->data.blocked = true;
            curr2->data.weight = INF;
            break;
        }
        curr2 = curr2->next;
    }

    if (found)
    {
        cout << "Road blocked between " << from << " and " << to << endl;
    }
    else
    {
        cout << "No road exists between " << from << " and " << to << endl;
    }
}

void Graph::unblockRoad(string from, string to, int weight)
{
    if (!cityLookup.contains(from) || !cityLookup.contains(to))
        return;

    int u = cityLookup.get(from);
    int v = cityLookup.get(to);

    Node<Edge> *curr = adjList[u].getHead();
    while (curr)
    {
        if (curr->data.toIndex == v)
        {
            curr->data.blocked = false;
            curr->data.weight = weight;
            break;
        }
        curr = curr->next;
    }

    Node<Edge> *curr2 = adjList[v].getHead();
    while (curr2)
    {
        if (curr2->data.toIndex == u)
        {
            curr2->data.blocked = false;
            curr2->data.weight = weight;
            break;
        }
        curr2 = curr2->next;
    }

    cout << "Road unblocked between " << from << " and " << to << " (weight: " << weight << ")" << endl;
}

int Graph::getRoadWeight(string from, string to)
{
    if (!cityLookup.contains(from) || !cityLookup.contains(to))
        return -1;

    int u = cityLookup.get(from);
    int v = cityLookup.get(to);

    Node<Edge> *curr = adjList[u].getHead();
    while (curr)
    {
        if (curr->data.toIndex == v && !curr->data.blocked)
        {
            return curr->data.weight;
        }
        curr = curr->next;
    }
    return -1; // No route or blocked
}

bool Graph::cityExists(string name) const
{
    return cityLookup.contains(name);
}

CustomVector<string> Graph::getAllCities() const
{
    return cities;
}

int Graph::getCityCount() const
{
    return cityCount;
}

struct DijkstraNode
{
    int u;
    int dist;

    bool operator<(const DijkstraNode &other) const
    {
        return dist < other.dist;
    }
};

void Graph::runDijkstra(string startCity, string endCity)
{
    if (!cityLookup.contains(startCity) || !cityLookup.contains(endCity))
    {
        cout << "Invalid cities." << endl;
        return;
    }

    int start = cityLookup.get(startCity);
    int end = cityLookup.get(endCity);

    CustomVector<int> dist;
    CustomVector<int> parent;

    for (int i = 0; i < cityCount; i++)
    {
        dist.push_back(INF);
        parent.push_back(-1);
    }

    dist[start] = 0;

    MinHeap<DijkstraNode> pq;
    pq.insert({start, 0});

    while (!pq.isEmpty())
    {
        DijkstraNode top = pq.extractMin();
        int u = top.u;
        int d = top.dist;

        if (d > dist[u])
            continue;
        if (u == end)
            break;

        Node<Edge> *curr = adjList[u].getHead();
        while (curr)
        {
            int v = curr->data.toIndex;
            int weight = curr->data.weight;

            if (dist[u] + weight < dist[v] && weight != INF)
            {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.insert({v, dist[v]});
            }
            curr = curr->next;
        }
    }

    if (dist[end] == INF)
    {
        cout << "No route found from " << startCity << " to " << endCity << endl;
    }
    else
    {
        cout << "Shortest Path Cost: " << dist[end] << endl;
        cout << "Path: ";
        CustomStack<int> pathStack;
        int curr = end;
        while (curr != -1)
        {
            pathStack.push(curr);
            curr = parent[curr];
        }

        while (!pathStack.isEmpty())
        {
            cout << cities[pathStack.pop()];
            if (!pathStack.isEmpty())
                cout << " -> ";
        }
        cout << endl;
    }
}

void Graph::loadGraph(string filename)
{
    ifstream inFile(filename);
    if (!inFile)
    {
        // Fallback to default if no file
        addLocation("A");
        addLocation("B");
        addLocation("C");
        addLocation("D");
        addLocation("E");
        addRoute("A", "B", 10);
        addRoute("A", "C", 5);
        addRoute("B", "D", 20);
        addRoute("C", "B", 8);
        addRoute("C", "E", 15);
        addRoute("D", "E", 5);
        return;
    }

    string type;
    while (inFile >> type)
    {
        if (type == "CITY")
        {
            string name;
            inFile >> name;
            addLocation(name);
        }
        else if (type == "ROUTE")
        {
            string u, v;
            int w;
            inFile >> u >> v >> w;
            addRoute(u, v, w);
        }
    }
    inFile.close();
    cout << "Graph loaded: " << cityCount << " cities." << endl;
}

void Graph::saveGraph(string filename)
{
    ofstream outFile(filename);
    if (!outFile)
    {
        cout << "Error saving graph to " << filename << endl;
        return;
    }

    // Save cities
    for (int i = 0; i < cityCount; i++)
    {
        outFile << "CITY " << cities[i] << endl;
    }

    // Save routes (avoiding duplicates by only saving u < v)
    for (int u = 0; u < cityCount; u++)
    {
        Node<Edge> *curr = adjList[u].getHead();
        while (curr)
        {
            int v = curr->data.toIndex;
            if (u < v && !curr->data.blocked)
            { // Only save unblocked, avoid duplicate
                outFile << "ROUTE " << cities[u] << " " << cities[v] << " " << curr->data.weight << endl;
            }
            curr = curr->next;
        }
    }

    // Save zones
    CustomVector<string> zoneNames = getAllZones();
    for (int i = 0; i < zoneNames.getSize(); i++)
    {
        outFile << "ZONE " << zoneNames[i] << endl;
        CustomVector<string> zoneCities = getCitiesInZone(zoneNames[i]);
        for (int j = 0; j < zoneCities.getSize(); j++)
        {
            outFile << "ZONECITY " << zoneNames[i] << " " << zoneCities[j] << endl;
        }
    }

    outFile.close();
}

// ============== K-Shortest Paths (Yen's Algorithm) ==============

RouteResult Graph::dijkstraWithExclusion(int start, int end,
                                         CustomVector<int> &excludedEdgesFrom,
                                         CustomVector<int> &excludedEdgesTo)
{
    RouteResult result;
    result.valid = false;

    CustomVector<int> dist;
    CustomVector<int> parent;

    for (int i = 0; i < cityCount; i++)
    {
        dist.push_back(INF);
        parent.push_back(-1);
    }

    dist[start] = 0;

    MinHeap<DijkstraNode> pq;
    pq.insert({start, 0});

    while (!pq.isEmpty())
    {
        DijkstraNode top = pq.extractMin();
        int u = top.u;
        int d = top.dist;

        if (d > dist[u])
            continue;
        if (u == end)
            break;

        Node<Edge> *curr = adjList[u].getHead();
        while (curr)
        {
            int v = curr->data.toIndex;
            int weight = curr->data.weight;

            // Check if this edge is excluded
            bool excluded = false;
            for (int i = 0; i < excludedEdgesFrom.getSize(); i++)
            {
                if (excludedEdgesFrom[i] == u && excludedEdgesTo[i] == v)
                {
                    excluded = true;
                    break;
                }
            }

            if (!excluded && dist[u] + weight < dist[v] && weight != INF)
            {
                dist[v] = dist[u] + weight;
                parent[v] = u;
                pq.insert({v, dist[v]});
            }
            curr = curr->next;
        }
    }

    if (dist[end] != INF)
    {
        result.valid = true;
        result.totalCost = dist[end];

        // Reconstruct path
        CustomStack<int> pathStack;
        int curr = end;
        while (curr != -1)
        {
            pathStack.push(curr);
            curr = parent[curr];
        }

        while (!pathStack.isEmpty())
        {
            result.path.push_back(cities[pathStack.pop()]);
        }
    }

    return result;
}

CustomVector<RouteResult> Graph::findKShortestPaths(string startCity, string endCity, int k)
{
    CustomVector<RouteResult> kPaths;

    if (!cityLookup.contains(startCity) || !cityLookup.contains(endCity))
    {
        return kPaths;
    }

    int start = cityLookup.get(startCity);
    int end = cityLookup.get(endCity);

    // Find first shortest path
    CustomVector<int> emptyFrom, emptyTo;
    RouteResult firstPath = dijkstraWithExclusion(start, end, emptyFrom, emptyTo);

    if (!firstPath.valid)
    {
        return kPaths; // No path exists
    }

    kPaths.push_back(firstPath);

    // Candidate paths for next shortest
    CustomVector<RouteResult> candidates;

    // Yen's Algorithm: Find k-1 more paths
    for (int i = 1; i < k; i++)
    {
        RouteResult &prevPath = kPaths[kPaths.getSize() - 1];

        // For each node in the previous path (except the last)
        for (int j = 0; j < prevPath.path.getSize() - 1; j++)
        {
            string spurNode = prevPath.path[j];
            int spurIndex = cityLookup.get(spurNode);

            // Root path from start to spur node
            CustomVector<string> rootPath;
            for (int r = 0; r <= j; r++)
            {
                rootPath.push_back(prevPath.path[r]);
            }

            // Exclude edges that share the root path
            CustomVector<int> excludedFrom, excludedTo;
            for (int p = 0; p < kPaths.getSize(); p++)
            {
                RouteResult &existingPath = kPaths[p];
                if (existingPath.path.getSize() > j)
                {
                    // Check if root path matches
                    bool matches = true;
                    for (int r = 0; r <= j && r < existingPath.path.getSize(); r++)
                    {
                        if (existingPath.path[r] != rootPath[r])
                        {
                            matches = false;
                            break;
                        }
                    }
                    if (matches && j + 1 < existingPath.path.getSize())
                    {
                        // Exclude the edge from spur to next node in existing path
                        int fromIdx = cityLookup.get(existingPath.path[j]);
                        int toIdx = cityLookup.get(existingPath.path[j + 1]);
                        excludedFrom.push_back(fromIdx);
                        excludedTo.push_back(toIdx);
                    }
                }
            }

            // Find spur path from spur node to end
            RouteResult spurPath = dijkstraWithExclusion(spurIndex, end, excludedFrom, excludedTo);

            if (spurPath.valid)
            {
                // Combine root path + spur path
                RouteResult totalPath;
                totalPath.valid = true;

                // Add root path (except spur node, which is included in spur path)
                for (int r = 0; r < j; r++)
                {
                    totalPath.path.push_back(rootPath[r]);
                }

                // Add spur path
                for (int s = 0; s < spurPath.path.getSize(); s++)
                {
                    totalPath.path.push_back(spurPath.path[s]);
                }

                // Calculate total cost
                totalPath.totalCost = 0;
                for (int c = 0; c < totalPath.path.getSize() - 1; c++)
                {
                    int w = getRoadWeight(totalPath.path[c], totalPath.path[c + 1]);
                    if (w > 0)
                        totalPath.totalCost += w;
                }

                // Check if this path is already in candidates or kPaths
                bool duplicate = false;
                for (int c = 0; c < candidates.getSize(); c++)
                {
                    if (candidates[c].path.getSize() == totalPath.path.getSize())
                    {
                        bool same = true;
                        for (int x = 0; x < totalPath.path.getSize(); x++)
                        {
                            if (candidates[c].path[x] != totalPath.path[x])
                            {
                                same = false;
                                break;
                            }
                        }
                        if (same)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                }

                if (!duplicate)
                {
                    candidates.push_back(totalPath);
                }
            }
        }

        if (candidates.isEmpty())
            break;

        // Find the shortest candidate
        int bestIdx = 0;
        for (int c = 1; c < candidates.getSize(); c++)
        {
            if (candidates[c].totalCost < candidates[bestIdx].totalCost)
            {
                bestIdx = c;
            }
        }

        kPaths.push_back(candidates[bestIdx]);

        // Remove selected candidate
        CustomVector<RouteResult> newCandidates;
        for (int c = 0; c < candidates.getSize(); c++)
        {
            if (c != bestIdx)
            {
                newCandidates.push_back(candidates[c]);
            }
        }
        candidates = newCandidates;
    }

    return kPaths;
}

void Graph::displayAlternativeRoutes(string startCity, string endCity, int k)
{
    cout << "\n--- ALTERNATIVE ROUTES (" << startCity << " -> " << endCity << ") ---\n";

    CustomVector<RouteResult> routes = findKShortestPaths(startCity, endCity, k);

    if (routes.isEmpty())
    {
        cout << "No routes found between " << startCity << " and " << endCity << endl;
        return;
    }

    for (int i = 0; i < routes.getSize(); i++)
    {
        cout << "\nRoute " << (i + 1);
        if (i == 0)
            cout << " (SHORTEST)";
        cout << ":\n";
        cout << "  Cost: " << routes[i].totalCost << "\n";
        cout << "  Path: ";
        for (int j = 0; j < routes[i].path.getSize(); j++)
        {
            cout << routes[i].path[j];
            if (j < routes[i].path.getSize() - 1)
                cout << " -> ";
        }
        cout << endl;
    }
}

bool Graph::isRoadBlocked(string from, string to)
{
    if (!cityLookup.contains(from) || !cityLookup.contains(to))
        return true;

    int u = cityLookup.get(from);
    int v = cityLookup.get(to);

    Node<Edge> *curr = adjList[u].getHead();
    while (curr)
    {
        if (curr->data.toIndex == v)
        {
            return curr->data.blocked;
        }
        curr = curr->next;
    }
    return true; // No edge = effectively blocked
}

// ============== Zone Management ==============

void Graph::addZone(string zoneName)
{
    if (!zones.contains(zoneName))
    {
        zones.insert(zoneName, Zone(zoneName));
        cout << "Zone '" << zoneName << "' created." << endl;
    }
}

void Graph::assignCityToZone(string city, string zoneName)
{
    if (!cityLookup.contains(city))
    {
        cout << "City '" << city << "' does not exist." << endl;
        return;
    }

    if (!zones.contains(zoneName))
    {
        addZone(zoneName);
    }

    Zone z = zones.get(zoneName);
    z.addCity(city);
    zones.insert(zoneName, z);
    cityToZone.insert(city, zoneName);
    cout << "City '" << city << "' assigned to zone '" << zoneName << "'." << endl;
}

string Graph::getCityZone(string city)
{
    if (cityToZone.contains(city))
    {
        return cityToZone.get(city);
    }
    return "Unassigned";
}

CustomVector<string> Graph::getCitiesInZone(string zoneName)
{
    CustomVector<string> result;
    if (zones.contains(zoneName))
    {
        Zone z = zones.get(zoneName);
        result = z.getCities();
    }
    return result;
}

CustomVector<string> Graph::getAllZones()
{
    return zones.getAllKeys();
}

void Graph::autoAssignZones()
{
    // Create default zones
    addZone("North");
    addZone("South");
    addZone("East");
    addZone("West");
    addZone("Central");

    // Simple auto-assignment based on city name first letter
    // This is a basic heuristic - can be customized
    for (int i = 0; i < cityCount; i++)
    {
        string city = cities[i];
        if (city.empty())
            continue;

        char first = city[0];
        if (first >= 'A' && first <= 'E')
        {
            assignCityToZone(city, "North");
        }
        else if (first >= 'F' && first <= 'J')
        {
            assignCityToZone(city, "East");
        }
        else if (first >= 'K' && first <= 'O')
        {
            assignCityToZone(city, "Central");
        }
        else if (first >= 'P' && first <= 'T')
        {
            assignCityToZone(city, "South");
        }
        else
        {
            assignCityToZone(city, "West");
        }
    }
    cout << "Auto-assigned all cities to zones based on naming convention." << endl;
}

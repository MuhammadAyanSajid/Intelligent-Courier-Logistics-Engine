#include "CourierSystem.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>

using namespace std;


#define PRINT_SUCCESS(msg) cout << "\n" << BRIGHT_GREEN << "[SUCCESS] " << RESET << msg
#define PRINT_ERROR(msg) cout << "\n" << BRIGHT_RED << "[ERROR] " << RESET << msg
#define PRINT_WARNING(msg) cout << "\n" << BRIGHT_YELLOW << "[WARNING] " << RESET << msg
#define PRINT_INFO(msg) cout << "\n" << BRIGHT_CYAN << "[INFO] " << RESET << msg


string CourierSystem::getCurrentTimestamp() const {
time_t now = time(0);
tm* timeinfo = localtime(&now);
ostringstream oss;
    oss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}


void CourierSystem::logOperation(OperationType type, int parcelID, const string& prevState, 
                                 const string& newState, int riderID, int srcID, int destID) {
    OperationLog log;
    log.type = type;
    log.timestamp = getCurrentTimestamp();
    log.parcelID = parcelID;
    log.previousState = prevState;
    log.newState = newState;
    log.riderID = riderID;
    log.srcID = srcID;
    log.destID = destID;
    operationHistory.push(log);
}


bool CourierSystem::isEdgeBlocked(int srcID, int destID) const {
    for(int i = 0; i < blockedEdges.size(); i++) {
        if(blockedEdges[i].srcID == srcID && blockedEdges[i].destID == destID && blockedEdges[i].isBlocked) {
            return true;
        }
        if(blockedEdges[i].srcID == destID && blockedEdges[i].destID == srcID && blockedEdges[i].isBlocked) {
            return true;
        }
    }
    return false;
}


Parcel* CourierSystem::findParcel(int trackingID) {
    Parcel** ptr = parcelMap.get(trackingID);
    return ptr ? *ptr : nullptr;
}


bool CourierSystem::validateCityID(int cityID) const {
    const Vector<Graph<City>::GraphNode>& nodes = cityMap.getNodes();
    for(int i = 0; i < nodes.size(); i++) {
        if(nodes[i].id == cityID) {
            return true;
        }
    }
    return false;
}

bool CourierSystem::validateRoute(int srcID, int destID) const {
    return validateCityID(srcID) && validateCityID(destID) && srcID != destID;
}

bool CourierSystem::validateParcelData(int weight, int srcID, int destID) const {
    return weight > 0 && validateRoute(srcID, destID);
}


bool CourierSystem::isParcelInQueue(const Queue<Parcel>& queue, int parcelID) const {
    Queue<Parcel> temp = queue;
    while(!temp.empty()) {
        if(temp.front().trackingID == parcelID) {
            return true;
        }
        temp.pop();
    }
    return false;
}

bool CourierSystem::isParcelInHeap(const MinHeap<Parcel>& heap, int parcelID) const {
    
    
    MinHeap<Parcel> temp = heap;
    Vector<Parcel> items;
    while(!temp.empty()) {
        items.push_back(temp.top());
        temp.pop();
    }
    for(int i = 0; i < items.size(); i++) {
        if(items[i].trackingID == parcelID) {
            return true;
        }
    }
    return false;
}

void CourierSystem::removeParcelFromQueue(Queue<Parcel>& queue, int parcelID) {
    Queue<Parcel> temp;
    while(!queue.empty()) {
        Parcel p = queue.front();
        queue.pop();
        if(p.trackingID != parcelID) {
            temp.push(p);
        }
    }
    queue = temp;
}

void CourierSystem::removeParcelFromHeap(MinHeap<Parcel>& heap, int parcelID) {
    MinHeap<Parcel> newHeap;
    while(!heap.empty()) {
        Parcel p = heap.top();
        heap.pop();
        if(p.trackingID != parcelID) {
            newHeap.push(p);
        }
    }
    heap = newHeap;
}

CourierSystem::CourierSystem() {
    nextTrackingID = 1001;
    nextRiderID = 1;
    initializeAdmins(); 
}

void CourierSystem::loadData() {
    CSVUtils::loadCities("cities.csv", cityMap);
    CSVUtils::loadRoutes("routes.csv", cityMap);
    CSVUtils::loadRiders("riders.csv", riders, nextRiderID);
    CSVUtils::loadParcels("parcels.csv", allParcels, parcelMap, nextTrackingID);
    
    
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].status == Status::Pending) {
            pickupQueue.push(allParcels[i]);
        } else if(allParcels[i].status == Status::InWarehouse) {
            warehouseQueue.push(allParcels[i]);
        } else if(allParcels[i].status == Status::InTransit) {
            transitQueue.push(allParcels[i]);
        }
    }
}

void CourierSystem::addParcel(string sender, string receiver, Priority priority, int weight, int srcID, int destID) {
    
    if (!validateParcelData(weight, srcID, destID)) {
        PRINT_ERROR("Invalid parcel data. Check weight (>0) and city IDs.\n");
        return;
    }
    
    if (priority < Priority::Overnight || priority > Priority::Normal) {
        PRINT_ERROR("Invalid priority. Use 1 (Overnight), 2 (Two-Day), or 3 (Normal).\n");
        return;
    }
    
    Parcel newParcel(nextTrackingID++, sender, receiver, priority, weight, srcID, destID);
    
    
    pickupQueue.push(newParcel);
    
    
    allParcels.push_back(newParcel);
    
    
    parcelMap.insert(newParcel.trackingID, &allParcels[allParcels.size() - 1]);
    
    
    CSVUtils::saveParcel("parcels.csv", newParcel);
    
    
    logOperation(OperationType::AddParcel, newParcel.trackingID, "None", "Pending", -1);
    
    PRINT_SUCCESS("Parcel Added! Tracking ID: " << BRIGHT_YELLOW << newParcel.trackingID << RESET << "\n");
}

bool CourierSystem::removeParcel(int trackingID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
        PRINT_ERROR("Parcel ID not found.\n");
        return false;
    }
    
    
    bool removedFromQueue = false;
    if (parcel->status == Status::Pending) {
        removedFromQueue = pickupQueue.remove(*parcel);
    } else if (parcel->status == Status::InWarehouse) {
        removedFromQueue = warehouseQueue.remove(*parcel);
    } else if (parcel->status == Status::InTransit) {
        removedFromQueue = transitQueue.remove(*parcel);
        
        for(int i = 0; i < riders.size(); i++) {
            for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
                if(riders[i].assignedParcels[j] == trackingID) {
                    riders[i].currentLoad -= parcel->weight;
                    
                    for(int k = j; k < riders[i].assignedParcels.size() - 1; k++) {
                        riders[i].assignedParcels[k] = riders[i].assignedParcels[k+1];
                    }
                    riders[i].assignedParcels.pop_back();
                    break;
                }
            }
        }
    }
    
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::Returned;
    parcel->history.push_back(getCurrentTimestamp() + " - Parcel Withdrawn");
    
    
    saveData();
    
    
    logOperation(OperationType::RemoveParcel, trackingID, prevState, "Withdrawn", -1);
    
    if (removedFromQueue || parcel->status == Status::Delivered) {
        PRINT_SUCCESS("Parcel " << trackingID << " withdrawn.\n");
    } else {
        PRINT_WARNING("Parcel " << trackingID << " marked as withdrawn but may not have been in queue.\n");
    }
    return true;
}

void CourierSystem::addCity(string name) {
    if (name.empty()) {
        PRINT_ERROR("City name cannot be empty.\n");
        return;
    }
    
    
    int maxID = 0;
    Vector<Graph<City>::GraphNode>& nodes = cityMap.getNodes();
    for(int i=0; i<nodes.size(); i++) {
        if(nodes[i].id > maxID) maxID = nodes[i].id;
    }
    int newID = maxID + 1;
    
    cityMap.addNode(newID, City(newID, name));
    CSVUtils::saveCity("cities.csv", newID, name);
    
    
    recalculateActiveRoutes();
    
    PRINT_SUCCESS("City '" << name << "' added with ID: " << newID << "\n");
}

void CourierSystem::addRoute(int srcID, int destID, int distance) {
    if (!validateRoute(srcID, destID)) {
        PRINT_ERROR("Invalid route. Check that both city IDs exist and are different.\n");
        return;
    }
    
    if (distance <= 0) {
        PRINT_ERROR("Distance must be greater than 0.\n");
        return;
    }
    
    cityMap.addEdge(srcID, destID, distance);
    cityMap.addEdge(destID, srcID, distance); 
    CSVUtils::saveRoute("routes.csv", srcID, destID, distance);
    
    
    recalculateActiveRoutes();
    
    PRINT_SUCCESS("Route added between " << srcID << " and " << destID << "\n");
}

int CourierSystem::calculateShortestPath(int srcID, int destID, Vector<int>& path, bool avoidBlocked) {
    const int MAX_ID = 1000;
    int dist[MAX_ID];
    int parent[MAX_ID];
    bool visited[MAX_ID];
    
    for(int i=0; i<MAX_ID; i++) {
        dist[i] = 999999; 
        parent[i] = -1;
        visited[i] = false;
    }
    
    dist[srcID] = 0;
    MinHeap<DijkstraNode> pq;
    pq.push({srcID, 0});
    
    while(!pq.empty()) {
        DijkstraNode current = pq.top();
        pq.pop();
        
        int u = current.id;
        int currentDist = current.dist;
        
        
        if (visited[u]) continue;
        
        
        if (currentDist > dist[u]) continue;
        
        visited[u] = true;
        
        
        if (u == destID) break;
        
        LinkedList<Graph<City>::Edge>* neighbors = cityMap.getNeighbors(u);
        if (!neighbors) continue;
        
        for(auto& edge : *neighbors) {
            int v = edge.destID;
            int weight = edge.weight;
            
            
            if (avoidBlocked && isEdgeBlocked(u, v)) {
                continue;
            }
            
            
            if (avoidBlocked && this->isPathOverloaded(u, v)) {
                continue;
            }
            
            
            int newDist = dist[u] + weight;
            if (newDist < dist[v]) {
                dist[v] = newDist;
                parent[v] = u;
                pq.push({v, dist[v]});
            }
        }
    }
    
    if (dist[destID] == 999999) return -1; 
    
    
    path.clear();
    int curr = destID;
    while(curr != -1) {
        path.push_back(curr);
        curr = parent[curr];
    }
    
    
    Vector<int> reversedPath;
    for(int i=path.size()-1; i>=0; i--) {
        reversedPath.push_back(path[i]);
    }
    path = reversedPath;
    
    return dist[destID];
}

void CourierSystem::displayRoute(int srcID, int destID) {
    Vector<int> path;
    
    
    int distance = calculateShortestPath(srcID, destID, path, true);
    
    if (distance == -1 || path.size() == 0) {
        distance = calculateShortestPath(srcID, destID, path, false);
    }
    
    if (distance == -1 || path.size() == 0) {
        PRINT_ERROR("No route found between these cities.\n");
        return;
    }
    
    int titleWidth = 30;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_CYAN BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 28; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Optimal Route   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    cout << BRIGHT_GREEN << "Total Distance: " << BRIGHT_YELLOW << distance << " km" << RESET << "\n";
    cout << BRIGHT_BLUE << "Path: " << RESET;
    for(int i=0; i<path.size(); i++) {
        City* c = cityMap.getNodeData(path[i]);
        if(c) cout << BRIGHT_CYAN << c->name << RESET;
        else cout << path[i];
        
        if(i < path.size()-1) cout << BRIGHT_YELLOW << " -> " << RESET;
    }
    cout << "\n";
    
    
    if(path.size() > 2) {
        cout << BRIGHT_MAGENTA << "Route Details: " << path.size() << " cities, " << (path.size()-1) << " segments" << RESET << "\n";
    }
}

void CourierSystem::viewAllRoutes(int srcID, int destID) {
    if (!validateRoute(srcID, destID)) {
cout << "\n[ERROR] Invalid city IDs.\n";
        return;
    }
    
    
    Vector<Vector<int>> allPaths;
    Vector<int> distances;
    bool visited[1000];
    for(int i = 0; i < 1000; i++) visited[i] = false;
    Vector<int> currentPath;
    
    
function<void(int, int, int)> dfs = [&](int current, int target, int currentDist) {
        if(current == target) {
            
            Vector<int> path = currentPath;
            path.push_back(current);
            allPaths.push_back(path);
            distances.push_back(currentDist);
            return;
        }
        
        if(visited[current] || currentPath.size() > 10) return; 
        
        visited[current] = true;
        currentPath.push_back(current);
        
        LinkedList<Graph<City>::Edge>* neighbors = cityMap.getNeighbors(current);
        if(neighbors) {
            for(auto& edge : *neighbors) {
                if(!visited[edge.destID] && !isEdgeBlocked(current, edge.destID) && !isPathOverloaded(current, edge.destID)) {
                    dfs(edge.destID, target, currentDist + edge.weight);
                }
            }
        }
        
        currentPath.pop_back();
        visited[current] = false;
    };
    
    dfs(srcID, destID, 0);
    
    if(allPaths.size() == 0) {
cout << "\n[INFO] No routes found between these cities.\n";
        return;
    }
    
    
    for(int i = 0; i < allPaths.size() - 1; i++) {
        for(int j = i + 1; j < allPaths.size(); j++) {
            if(distances[i] > distances[j]) {
                
                int tempDist = distances[i];
                distances[i] = distances[j];
                distances[j] = tempDist;
                Vector<int> tempPath = allPaths[i];
                allPaths[i] = allPaths[j];
                allPaths[j] = tempPath;
            }
        }
    }
    
cout << "\n=== All Available Routes ===\n";
cout << "Found " << allPaths.size() << " route(s) between ";
    City* src = cityMap.getNodeData(srcID);
    City* dest = cityMap.getNodeData(destID);
cout << (src ? src->name : to_string(srcID)) << " and " << (dest ? dest->name : to_string(destID)) << "\n\n";
    
    for(int i = 0; i < allPaths.size() && i < 20; i++) { 
cout << "Route " << (i+1) << " - Distance: " << distances[i] << " km\n";
cout << "Path: ";
        for(int j = 0; j < allPaths[i].size(); j++) {
            City* c = cityMap.getNodeData(allPaths[i][j]);
            if(c) cout << c->name;
            else cout << allPaths[i][j];
            
            if(j < allPaths[i].size() - 1) cout << " -> ";
        }
cout << "\n";
cout << "Cities: " << allPaths[i].size() << ", Segments: " << (allPaths[i].size() - 1) << "\n\n";
    }
    
    if(allPaths.size() > 20) {
cout << "... and " << (allPaths.size() - 20) << " more route(s)\n";
    }
}

void CourierSystem::findKShortestPaths(int srcID, int destID, int k, Vector<Vector<int>>& paths, Vector<int>& distances) {
    paths.clear();
    distances.clear();
    
    
    Vector<BlockedEdge> originalBlockedEdges = blockedEdges;
    
    
    auto isEdgeBlockedLocal = [&](int s, int d, const Vector<BlockedEdge>& blockedList) -> bool {
        for(int i = 0; i < blockedList.size(); i++) {
            if(blockedList[i].isBlocked && 
               ((blockedList[i].srcID == s && blockedList[i].destID == d) ||
                (blockedList[i].srcID == d && blockedList[i].destID == s))) {
                return true;
            }
        }
        return false;
    };
    
    
    auto calculateWithLocalBlocked = [&](int src, int dest, Vector<int>& path, const Vector<BlockedEdge>& localBlocked) -> int {
        const int MAX_ID = 1000;
        int dist[MAX_ID];
        int parent[MAX_ID];
        bool visited[MAX_ID];
        
        for(int i=0; i<MAX_ID; i++) {
            dist[i] = 999999;
            parent[i] = -1;
            visited[i] = false;
        }
        
        dist[src] = 0;
        MinHeap<DijkstraNode> pq;
        pq.push({src, 0});
        
        while(!pq.empty()) {
            DijkstraNode current = pq.top();
            pq.pop();
            
            int u = current.id;
            int currentDist = current.dist;
            
            if (visited[u]) continue;
            if (currentDist > dist[u]) continue;
            
            visited[u] = true;
            if (u == dest) break;
            
            LinkedList<Graph<City>::Edge>* neighbors = cityMap.getNeighbors(u);
            if (!neighbors) continue;
            
            for(auto& edge : *neighbors) {
                int v = edge.destID;
                int weight = edge.weight;
                
                
                if (isEdgeBlockedLocal(u, v, localBlocked)) {
                    continue;
                }
                
                if (this->isPathOverloaded(u, v)) {
                    continue;
                }
                
                int newDist = dist[u] + weight;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    parent[v] = u;
                    pq.push({v, dist[v]});
                }
            }
        }
        
        if (dist[dest] == 999999) return -1;
        
        path.clear();
        int curr = dest;
        while(curr != -1) {
            path.push_back(curr);
            curr = parent[curr];
        }
        
        Vector<int> reversedPath;
        for(int i=path.size()-1; i>=0; i--) {
            reversedPath.push_back(path[i]);
        }
        path = reversedPath;
        
        return dist[dest];
    };
    
    
    Vector<int> firstPath;
    int firstDist = calculateWithLocalBlocked(srcID, destID, firstPath, originalBlockedEdges);
    if (firstDist == -1) {
        return;
    }
    
    paths.push_back(firstPath);
    distances.push_back(firstDist);
    
    
    
    for(int pathIdx = 1; pathIdx < k; pathIdx++) {
        int bestDist = 999999;
        Vector<int> bestPath;
        
        
        for(int p = 0; p < paths.size(); p++) {
            for(int i = 0; i < paths[p].size() - 1; i++) {
                int blockedSrc = paths[p][i];
                int blockedDest = paths[p][i+1];
                
                
                Vector<BlockedEdge> tempBlocked = originalBlockedEdges;
                
                
                bool found = false;
                for(int j = 0; j < tempBlocked.size(); j++) {
                    if((tempBlocked[j].srcID == blockedSrc && tempBlocked[j].destID == blockedDest) ||
                       (tempBlocked[j].srcID == blockedDest && tempBlocked[j].destID == blockedSrc)) {
                        tempBlocked[j].isBlocked = true;
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    tempBlocked.push_back(BlockedEdge(blockedSrc, blockedDest, true));
                }
                
                
                Vector<int> newPath;
                int newDist = calculateWithLocalBlocked(srcID, destID, newPath, tempBlocked);
                
                if (newDist != -1 && newDist < bestDist) {
                    
                    bool isDifferent = true;
                    for(int j = 0; j < paths.size(); j++) {
                        if (paths[j].size() == newPath.size()) {
                            bool same = true;
                            for(int m = 0; m < newPath.size(); m++) {
                                if (paths[j][m] != newPath[m]) {
                                    same = false;
                                    break;
                                }
                            }
                            if (same) {
                                isDifferent = false;
                                break;
                            }
                        }
                    }
                    
                    if (isDifferent) {
                        bestDist = newDist;
                        bestPath = newPath;
                    }
                }
            }
        }
        
        if (bestDist < 999999) {
            paths.push_back(bestPath);
            distances.push_back(bestDist);
        } else {
            break; 
        }
    }
    
    
    blockedEdges = originalBlockedEdges;
}

void CourierSystem::calculateAlternativeRoutes(int srcID, int destID, int k, Vector<Vector<int>>& routes, Vector<int>& distances) {
    findKShortestPaths(srcID, destID, k, routes, distances);
}

void CourierSystem::displayAlternativeRoutes(int srcID, int destID, int count) {
    Vector<Vector<int>> routes;
    Vector<int> distances;
    
    calculateAlternativeRoutes(srcID, destID, count, routes, distances);
    
    if (routes.size() == 0) {
cout << "\n[ERROR] No routes found between these cities.\n";
        return;
    }
    
cout << "\n=== Alternative Routes (" << routes.size() << " found) ===\n";
    for(int i = 0; i < routes.size(); i++) {
cout << "\nRoute " << (i+1) << " - Distance: " << distances[i] << " km\n";
cout << "Path: ";
        for(int j = 0; j < routes[i].size(); j++) {
            City* c = cityMap.getNodeData(routes[i][j]);
            if(c) cout << c->name;
            else cout << routes[i][j];
            
            if(j < routes[i].size()-1) cout << " -> ";
        }
cout << "\n";
    }
}

void CourierSystem::blockRoute(int srcID, int destID) {
    
    for(int i = 0; i < blockedEdges.size(); i++) {
        if((blockedEdges[i].srcID == srcID && blockedEdges[i].destID == destID) ||
           (blockedEdges[i].srcID == destID && blockedEdges[i].destID == srcID)) {
            blockedEdges[i].isBlocked = true;
cout << "\n[SUCCESS] Route already in blocked list, marked as blocked.\n";
            recalculateActiveRoutes();
            return;
        }
    }
    
    
    blockedEdges.push_back(BlockedEdge(srcID, destID, true));
    logOperation(OperationType::BlockRoute, -1, "Open", "Blocked", -1, srcID, destID);
    
cout << "\n[SUCCESS] Route between " << srcID << " and " << destID << " blocked.\n";
    recalculateActiveRoutes();
}

void CourierSystem::unblockRoute(int srcID, int destID) {
    for(int i = 0; i < blockedEdges.size(); i++) {
        if((blockedEdges[i].srcID == srcID && blockedEdges[i].destID == destID) ||
           (blockedEdges[i].srcID == destID && blockedEdges[i].destID == srcID)) {
            blockedEdges[i].isBlocked = false;
            logOperation(OperationType::UnblockRoute, -1, "Blocked", "Open", -1, srcID, destID);
cout << "\n[SUCCESS] Route between " << srcID << " and " << destID << " unblocked.\n";
            recalculateActiveRoutes();
            return;
        }
    }
cout << "\n[INFO] Route not found in blocked list.\n";
}

void CourierSystem::displayBlockedRoutes() {
    Table t;
    t.addHeader("Source ID");
    t.addHeader("Destination ID");
    t.addHeader("Status");
    
    bool hasBlocked = false;
    for(int i = 0; i < blockedEdges.size(); i++) {
        if(blockedEdges[i].isBlocked) {
            hasBlocked = true;
            City* src = cityMap.getNodeData(blockedEdges[i].srcID);
            City* dest = cityMap.getNodeData(blockedEdges[i].destID);
string srcName = src ? src->name : to_string(blockedEdges[i].srcID);
string destName = dest ? dest->name : to_string(blockedEdges[i].destID);
            t.addRow({srcName, destName, "Blocked"});
        }
    }
    
    if (!hasBlocked) {
cout << "\n[INFO] No blocked routes.\n";
        return;
    }
    
cout << "\n=== Blocked Routes ===\n";
    t.print();
}

void CourierSystem::recalculateActiveRoutes() {
    int count = 0;
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].status == Status::InTransit) {
            Vector<int> newPath;
            
            int startCity = (allParcels[i].currentCityID != -1) ? allParcels[i].currentCityID : allParcels[i].sourceCityID;
            int dist = calculateShortestPath(startCity, allParcels[i].destCityID, newPath, true);
            if(dist != -1) {
                
                allParcels[i].currentRoute = newPath;
                allParcels[i].history.push_back(getCurrentTimestamp() + " - Route recalculated due to network change");
                count++;
            }
        }
    }
    if(count > 0) {
cout << "\n[INFO] Recalculated routes for " << count << " parcels in transit.\n";
    }
}


void CourierSystem::markPathAsOverloaded(int srcID, int destID, int load) {
    if (!validateRoute(srcID, destID)) {
cout << "\n[ERROR] Invalid route.\n";
        return;
    }
    
    
    for(int i = 0; i < overloadedEdges.size(); i++) {
        if((overloadedEdges[i].srcID == srcID && overloadedEdges[i].destID == destID) ||
           (overloadedEdges[i].srcID == destID && overloadedEdges[i].destID == srcID)) {
            overloadedEdges[i].currentLoad = load;
            if(load >= overloadedEdges[i].maxCapacity) {
cout << "\n[WARNING] Path is at or over capacity! Consider blocking.\n";
            }
            recalculateActiveRoutes();
            return;
        }
    }
    
    
    overloadedEdges.push_back(OverloadedEdge(srcID, destID, load, 100));
cout << "\n[SUCCESS] Path marked as overloaded (Load: " << load << "/100).\n";
    recalculateActiveRoutes();
}

void CourierSystem::unmarkPathAsOverloaded(int srcID, int destID) {
    for(int i = 0; i < overloadedEdges.size(); i++) {
        if((overloadedEdges[i].srcID == srcID && overloadedEdges[i].destID == destID) ||
           (overloadedEdges[i].srcID == destID && overloadedEdges[i].destID == srcID)) {
            
            for(int j = i; j < overloadedEdges.size() - 1; j++) {
                overloadedEdges[j] = overloadedEdges[j+1];
            }
            overloadedEdges.pop_back();
cout << "\n[SUCCESS] Path overload status removed.\n";
            recalculateActiveRoutes();
            return;
        }
    }
cout << "\n[INFO] Path not found in overloaded list.\n";
}

bool CourierSystem::isPathOverloaded(int srcID, int destID) const {
    for(int i = 0; i < overloadedEdges.size(); i++) {
        if((overloadedEdges[i].srcID == srcID && overloadedEdges[i].destID == destID) ||
           (overloadedEdges[i].srcID == destID && overloadedEdges[i].destID == srcID)) {
            return overloadedEdges[i].currentLoad >= overloadedEdges[i].maxCapacity;
        }
    }
    return false;
}

void CourierSystem::displayOverloadedRoutes() {
    Table t;
    t.addHeader("Source ID");
    t.addHeader("Destination ID");
    t.addHeader("Current Load");
    t.addHeader("Max Capacity");
    t.addHeader("Status");
    
    bool hasOverloaded = false;
    for(int i = 0; i < overloadedEdges.size(); i++) {
        hasOverloaded = true;
        City* src = cityMap.getNodeData(overloadedEdges[i].srcID);
        City* dest = cityMap.getNodeData(overloadedEdges[i].destID);
string srcName = src ? src->name : to_string(overloadedEdges[i].srcID);
string destName = dest ? dest->name : to_string(overloadedEdges[i].destID);
string status = (overloadedEdges[i].currentLoad >= overloadedEdges[i].maxCapacity) ? "OVERLOADED" : "High Load";
        
        t.addRow({
            srcName,
            destName,
to_string(overloadedEdges[i].currentLoad),
to_string(overloadedEdges[i].maxCapacity),
            status
        });
    }
    
    if (!hasOverloaded) {
cout << "\n[INFO] No overloaded routes.\n";
        return;
    }
    
cout << "\n=== Overloaded Routes ===\n";
    t.print();
}


Vector<Parcel> CourierSystem::getParcelsByWeightCategory(WeightCategory category) {
    Vector<Parcel> result;
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].getWeightCategory() == category) {
            result.push_back(allParcels[i]);
        }
    }
    return result;
}

Vector<Parcel> CourierSystem::getParcelsByDestination(int destCityID) {
    Vector<Parcel> result;
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].destCityID == destCityID) {
            result.push_back(allParcels[i]);
        }
    }
    return result;
}

void CourierSystem::sortParcelsByWeight(Vector<Parcel>& parcels) {
    
    
    for(int i = 0; i < parcels.size(); i++) {
        for(int j = 0; j < parcels.size() - i - 1; j++) {
            if(parcels[j].weight > parcels[j+1].weight) {
                Parcel temp = parcels[j];
                parcels[j] = parcels[j+1];
                parcels[j+1] = temp;
            }
        }
    }
}

void CourierSystem::sortParcelsByDestination(Vector<Parcel>& parcels) {
    
    for(int i = 0; i < parcels.size(); i++) {
        for(int j = 0; j < parcels.size() - i - 1; j++) {
            if(parcels[j].destCityID > parcels[j+1].destCityID) {
                Parcel temp = parcels[j];
                parcels[j] = parcels[j+1];
                parcels[j+1] = temp;
            }
        }
    }
}

void CourierSystem::displayParcelsByWeight() {
    Table t;
    t.addHeader("Tracking ID");
    t.addHeader("Weight (kg)");
    t.addHeader("Category");
    t.addHeader("Priority");
    t.addHeader("Status");
    
    Vector<Parcel> sorted = allParcels;
    sortParcelsByWeight(sorted);
    
    for(int i = 0; i < sorted.size(); i++) {
        t.addRow({
to_string(sorted[i].trackingID),
to_string(sorted[i].weight),
            sorted[i].getWeightCategoryStr(),
            sorted[i].getPriorityStr(),
            sorted[i].getStatusStr()
        });
    }
    
cout << "\n=== Parcels Sorted by Weight ===\n";
    t.print();
}

void CourierSystem::displayParcelsByDestination() {
    Table t;
    t.setHeaderColor(BRIGHT_BLUE BOLD);
    t.setBorderColor(CYAN);
    t.setRowColor(WHITE);
    t.addHeader("Tracking ID");
    t.addHeader("Destination City");
    t.addHeader("Priority");
    t.addHeader("Status");
    
    Vector<Parcel> sorted = allParcels;
    sortParcelsByDestination(sorted);
    
    for(int i = 0; i < sorted.size(); i++) {
        City* dest = cityMap.getNodeData(sorted[i].destCityID);
string destName = dest ? dest->name : to_string(sorted[i].destCityID);
        t.addRow({
to_string(sorted[i].trackingID),
            destName,
            sorted[i].getPriorityStr(),
            sorted[i].getStatusStr()
        });
    }
    
    int titleWidth = 45;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_BLUE BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 43; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Parcels Sorted by Destination   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
}

void CourierSystem::sortParcelsByPriority(Vector<Parcel>& parcels) {
    
    for(int i = 0; i < parcels.size(); i++) {
        for(int j = 0; j < parcels.size() - i - 1; j++) {
            if(parcels[j].priority > parcels[j+1].priority) {
                Parcel temp = parcels[j];
                parcels[j] = parcels[j+1];
                parcels[j+1] = temp;
            }
        }
    }
}

void CourierSystem::displayParcelsByPriority() {
    Table t;
    t.setHeaderColor(BRIGHT_GREEN BOLD);
    t.setBorderColor(GREEN);
    t.setRowColor(WHITE);
    t.addHeader("Tracking ID");
    t.addHeader("Priority");
    t.addHeader("Weight (kg)");
    t.addHeader("Status");
    t.addHeader("Fragile");
    
    Vector<Parcel> sorted = allParcels;
    sortParcelsByPriority(sorted);
    
    for(int i = 0; i < sorted.size(); i++) {
        t.addRow({
to_string(sorted[i].trackingID),
            sorted[i].getPriorityStr(),
to_string(sorted[i].weight),
            sorted[i].getStatusStr(),
            sorted[i].isFragile ? "Yes" : "No"
        });
    }
    
    int titleWidth = 45;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_GREEN BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 43; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Parcels Sorted by Priority   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
}

Vector<Parcel> CourierSystem::getFragileParcels() {
    Vector<Parcel> result;
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].isFragile) {
            result.push_back(allParcels[i]);
        }
    }
    return result;
}

void CourierSystem::markParcelAsFragile(int trackingID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID not found.\n";
        return;
    }
    
    parcel->isFragile = true;
    parcel->history.push_back(getCurrentTimestamp() + " - Marked as Fragile");
    
    
    if(parcel->status == Status::InWarehouse) {
        warehouseQueue.remove(*parcel);
        warehouseQueue.push(*parcel); 
    }
    
    saveData();
cout << "\n[SUCCESS] Parcel " << trackingID << " marked as fragile.\n";
}


void CourierSystem::processNextParcel() {
    if (pickupQueue.empty()) {
cout << "\n[INFO] No pending parcels to process.\n";
        return;
    }
    
    Parcel top = pickupQueue.front();
    pickupQueue.pop();
    
    Parcel* parcel = findParcel(top.trackingID);
    if (!parcel) return;
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::InWarehouse;
    parcel->history.push_back(getCurrentTimestamp() + " - Processed at Warehouse");
    
    
    warehouseQueue.push(*parcel);
    
    
    saveData();
    
    logOperation(OperationType::ProcessParcel, top.trackingID, prevState, "In Warehouse", -1);
cout << "\n[SUCCESS] Parcel " << top.trackingID << " (" << top.getPriorityStr() << ") moved to Warehouse.\n";
}

void CourierSystem::processParcelByID(int parcelID) {
    Parcel* parcel = findParcel(parcelID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID not found.\n";
        return;
    }
    
    
    if (parcel->status != Status::Pending) {
cout << "\n[ERROR] Parcel is not in Pending status. Current status: " << parcel->getStatusStr() << "\n";
        return;
    }
    
    
    if (!pickupQueue.remove(*parcel)) {
cout << "\n[WARNING] Parcel not found in pickup queue, but proceeding...\n";
    }
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::InWarehouse;
    parcel->history.push_back(getCurrentTimestamp() + " - Processed at Warehouse");
    
    
    warehouseQueue.push(*parcel);
    
    
    saveData();
    
    logOperation(OperationType::ProcessParcel, parcelID, prevState, "In Warehouse", -1);
cout << "\n[SUCCESS] Parcel " << parcelID << " (" << parcel->getPriorityStr() << ") moved to Warehouse.\n";
}

void CourierSystem::assignRider() {
    if (warehouseQueue.empty()) {
cout << "\n[INFO] No parcels waiting in Warehouse.\n";
        return;
    }
    
    Parcel top = warehouseQueue.top();
    warehouseQueue.pop();
    
    Parcel* parcel = findParcel(top.trackingID);
    if (!parcel) return;
    
    
    int riderID = findAvailableRider(parcel->weight, parcel->sourceCityID);
    if (riderID == -1) {
cout << "\n[WARNING] No available rider found. Parcel remains in warehouse.\n";
        warehouseQueue.push(*parcel); 
        return;
    }
    
    
    if (parcel->isFragile) {
        Rider* rider = nullptr;
        for(int i = 0; i < riders.size(); i++) {
            if(riders[i].riderID == riderID) {
                rider = &riders[i];
                break;
            }
        }
        if(rider && rider->currentLoad > rider->capacity * 0.8) {
cout << "\n[INFO] Fragile parcel assigned to rider with sufficient capacity buffer.\n";
        }
    }
    
    assignRiderToParcel(parcel->trackingID, riderID);
}

void CourierSystem::assignRiderToParcel(int parcelID, int riderID) {
    Parcel* parcel = findParcel(parcelID);
    if (!parcel) {
cout << "\n[ERROR] Parcel not found.\n";
        return;
    }
    
    
    Rider* rider = nullptr;
    for(int i = 0; i < riders.size(); i++) {
        if(riders[i].riderID == riderID) {
            rider = &riders[i];
            break;
        }
    }
    
    if (!rider) {
cout << "\n[ERROR] Rider not found.\n";
        return;
    }
    
    if (!rider->canCarry(parcel->weight)) {
cout << "\n[ERROR] Rider capacity exceeded. Cannot assign parcel.\n";
        return;
    }
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::InTransit;
    
    
    Vector<int> route;
    int dist = calculateShortestPath(parcel->sourceCityID, parcel->destCityID, route, true);
    if(dist != -1) {
        parcel->currentRoute = route;
        parcel->currentCityID = parcel->sourceCityID;
    }
    
    parcel->history.push_back(getCurrentTimestamp() + " - Assigned to Rider " + rider->name);
    
    
    rider->currentLoad += parcel->weight;
    rider->assignedParcels.push_back(parcelID);
    
    
    transitQueue.push(*parcel);
    
    
    saveData();
    
    logOperation(OperationType::AssignRider, parcelID, prevState, "In Transit", riderID);
cout << "\n[SUCCESS] Parcel " << parcelID << " assigned to Rider " << rider->name << ".\n";
}

int CourierSystem::findAvailableRider(int requiredCapacity, int cityID) {
    int bestRider = -1;
    int bestAvailableCapacity = -1;
    int bestDistance = -1; 
    
    
    for(int i = 0; i < riders.size(); i++) {
        if(riders[i].currentCityID == cityID && riders[i].canCarry(requiredCapacity)) {
            int available = riders[i].getAvailableCapacity();
            if(bestRider == -1 || available > bestAvailableCapacity) {
                bestRider = riders[i].riderID;
                bestAvailableCapacity = available;
                bestDistance = 0; 
            }
        }
    }
    
    
    if(bestRider == -1) {
        for(int i = 0; i < riders.size(); i++) {
            if(riders[i].canCarry(requiredCapacity)) {
                
                LinkedList<Graph<City>::Edge>* neighbors = cityMap.getNeighbors(cityID);
                if(neighbors) {
                    for(auto& edge : *neighbors) {
                        if(edge.destID == riders[i].currentCityID && !isEdgeBlocked(cityID, edge.destID)) {
                            
                            int available = riders[i].getAvailableCapacity();
                            int distance = edge.weight;
                            
                            
                            if(bestRider == -1 || 
                               (bestDistance == -1 || distance < bestDistance) ||
                               (distance == bestDistance && available > bestAvailableCapacity)) {
                                bestRider = riders[i].riderID;
                                bestAvailableCapacity = available;
                                bestDistance = distance;
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        
        if(bestRider == -1) {
            for(int i = 0; i < riders.size(); i++) {
                if(riders[i].canCarry(requiredCapacity)) {
                    int available = riders[i].getAvailableCapacity();
                    if(bestRider == -1 || available > bestAvailableCapacity) {
                        bestRider = riders[i].riderID;
                        bestAvailableCapacity = available;
                    }
                }
            }
            
            
            if(bestRider != -1) {
                for(int i = 0; i < riders.size(); i++) {
                    if(riders[i].riderID == bestRider) {
                        if(riders[i].currentCityID != cityID) {
cout << "[INFO] Rider " << riders[i].name << " is moving from city " << riders[i].currentCityID 
     << " to city " << cityID << " for pickup.\n";
                            riders[i].currentCityID = cityID;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    return bestRider;
}

void CourierSystem::completeDelivery() {
    if (transitQueue.empty()) {
cout << "\n[INFO] No parcels currently in Transit.\n";
        return;
    }
    
    Parcel top = transitQueue.front();
    transitQueue.pop();
    
    Parcel* parcel = findParcel(top.trackingID);
    if (!parcel) return;
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::Delivered;
    parcel->currentCityID = parcel->destCityID; 
    
    
    City* destCity = cityMap.getNodeData(parcel->destCityID);
string destName = destCity ? destCity->name : to_string(parcel->destCityID);
    parcel->history.push_back(getCurrentTimestamp() + " - Delivered to " + parcel->receiverName + " at " + destName);
    
    
    int assignedRiderID = -1;
    for(int i = 0; i < riders.size(); i++) {
        for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
            if(riders[i].assignedParcels[j] == parcel->trackingID) {
                assignedRiderID = riders[i].riderID;
                riders[i].currentLoad -= parcel->weight;
                riders[i].currentCityID = parcel->destCityID; 
                
                for(int k = j; k < riders[i].assignedParcels.size() - 1; k++) {
                    riders[i].assignedParcels[k] = riders[i].assignedParcels[k+1];
                }
                riders[i].assignedParcels.pop_back();
                break;
            }
        }
    }
    
    
    saveData();
    
    logOperation(OperationType::CompleteDelivery, top.trackingID, prevState, "Delivered", assignedRiderID);
cout << "\n[SUCCESS] Parcel " << top.trackingID << " Delivered to " << destName << "!\n";
}

void CourierSystem::completeDeliveryByID(int parcelID) {
    Parcel* parcel = findParcel(parcelID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID not found.\n";
        return;
    }
    
    
    if (parcel->status != Status::InTransit) {
cout << "\n[ERROR] Parcel is not in Transit status. Current status: " << parcel->getStatusStr() << "\n";
        return;
    }
    
    
    if (!transitQueue.remove(*parcel)) {
cout << "\n[WARNING] Parcel not found in transit queue, but proceeding...\n";
    }
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::Delivered;
    parcel->currentCityID = parcel->destCityID; 
    
    
    City* destCity = cityMap.getNodeData(parcel->destCityID);
string destName = destCity ? destCity->name : to_string(parcel->destCityID);
    parcel->history.push_back(getCurrentTimestamp() + " - Delivered to " + parcel->receiverName + " at " + destName);
    
    
    int assignedRiderID = -1;
    for(int i = 0; i < riders.size(); i++) {
        for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
            if(riders[i].assignedParcels[j] == parcelID) {
                assignedRiderID = riders[i].riderID;
                riders[i].currentLoad -= parcel->weight;
                riders[i].currentCityID = parcel->destCityID; 
                
                for(int k = j; k < riders[i].assignedParcels.size() - 1; k++) {
                    riders[i].assignedParcels[k] = riders[i].assignedParcels[k+1];
                }
                riders[i].assignedParcels.pop_back();
                break;
            }
        }
    }
    
    
    saveData();
    
    logOperation(OperationType::CompleteDelivery, parcelID, prevState, "Delivered", assignedRiderID);
cout << "\n[SUCCESS] Parcel " << parcelID << " Delivered to " << destName << "!\n";
}

void CourierSystem::recordDispatch(int trackingID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID " << trackingID << " not found.\n";
        return;
    }
    if (parcel->status != Status::InWarehouse && parcel->status != Status::InTransit) {
cout << "\n[WARNING] Parcel is not in Warehouse or Transit status. Current: " << parcel->getStatusStr() << "\n";
    }
    parcel->history.push_back(getCurrentTimestamp() + " - Dispatched from warehouse");
    saveData();
cout << "\n[SUCCESS] Dispatch recorded for parcel " << trackingID << ".\n";
}

void CourierSystem::recordLoading(int trackingID, int cityID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID " << trackingID << " not found.\n";
        return;
    }
    if (!validateCityID(cityID)) {
cout << "\n[ERROR] Invalid City ID: " << cityID << "\n";
        return;
    }
    City* city = cityMap.getNodeData(cityID);
string cityName = city ? city->name : to_string(cityID);
    parcel->history.push_back(getCurrentTimestamp() + " - Loaded at " + cityName);
    parcel->currentCityID = cityID; 
    saveData();
cout << "\n[SUCCESS] Loading recorded for parcel " << trackingID << " at " << cityName << ".\n";
}

void CourierSystem::recordUnloading(int trackingID, int cityID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID " << trackingID << " not found.\n";
        return;
    }
    if (!validateCityID(cityID)) {
cout << "\n[ERROR] Invalid City ID: " << cityID << "\n";
        return;
    }
    City* city = cityMap.getNodeData(cityID);
string cityName = city ? city->name : to_string(cityID);
    parcel->history.push_back(getCurrentTimestamp() + " - Unloaded at " + cityName);
    parcel->currentCityID = cityID; 
    saveData();
cout << "\n[SUCCESS] Unloading recorded for parcel " << trackingID << " at " << cityName << ".\n";
}

void CourierSystem::recordDeliveryAttempt(int trackingID, bool success) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) return;
    
    parcel->deliveryAttempts++;
    if (success) {
        parcel->status = Status::Delivered;
        parcel->history.push_back(getCurrentTimestamp() + " - Delivery successful (Attempt " + to_string(parcel->deliveryAttempts) + ")");
    } else {
        parcel->history.push_back(getCurrentTimestamp() + " - Delivery attempt failed (Attempt " + to_string(parcel->deliveryAttempts) + ")");
        if (parcel->deliveryAttempts >= 3) {
            returnToSender(trackingID);
        }
    }
}

void CourierSystem::returnToSender(int trackingID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID " << trackingID << " not found.\n";
        return;
    }
    
    if (parcel->status == Status::Delivered) {
cout << "\n[ERROR] Cannot return delivered parcel to sender.\n";
        return;
    }
    
string prevState = parcel->getStatusStr();
    parcel->status = Status::Returned;
    
    
    removeParcelFromQueue(transitQueue, trackingID);
    
    parcel->history.push_back(getCurrentTimestamp() + " - Returned to sender after " + to_string(parcel->deliveryAttempts) + " failed attempts");
    
    
    for(int i = 0; i < riders.size(); i++) {
        for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
            if(riders[i].assignedParcels[j] == trackingID) {
                riders[i].currentLoad -= parcel->weight;
                for(int k = j; k < riders[i].assignedParcels.size() - 1; k++) {
                    riders[i].assignedParcels[k] = riders[i].assignedParcels[k+1];
                }
                riders[i].assignedParcels.pop_back();
                break;
            }
        }
    }
    
    saveData();
    logOperation(OperationType::ReturnToSender, trackingID, prevState, "Returned", -1);
cout << "\n[SUCCESS] Parcel " << trackingID << " returned to sender.\n";
}


void CourierSystem::addRider(string name, int capacity, int cityID) {
    Rider newRider(nextRiderID++, name, capacity, cityID);
    riders.push_back(newRider);
    
    
    CSVUtils::saveRider("riders.csv", newRider.riderID, name, capacity, cityID);
    
cout << "\n[SUCCESS] Rider '" << name << "' added with ID: " << newRider.riderID << "\n";
}

void CourierSystem::displayRiders() {
    Table t;
    t.addHeader("Rider ID");
    t.addHeader("Name");
    t.addHeader("Capacity (kg)");
    t.addHeader("Current Load (kg)");
    t.addHeader("Available (kg)");
    t.addHeader("Location");
    
    for(int i = 0; i < riders.size(); i++) {
        City* city = cityMap.getNodeData(riders[i].currentCityID);
string cityName = city ? city->name : to_string(riders[i].currentCityID);
        t.addRow({
to_string(riders[i].riderID),
            riders[i].name,
to_string(riders[i].capacity),
to_string(riders[i].currentLoad),
to_string(riders[i].getAvailableCapacity()),
            cityName
        });
    }
    
cout << "\n=== All Riders ===\n";
    t.print();
}

void CourierSystem::displayRiderLoad(int riderID) {
    for(int i = 0; i < riders.size(); i++) {
        if(riders[i].riderID == riderID) {
            Table t;
            t.addHeader("Parcel ID");
            t.addHeader("Weight (kg)");
            t.addHeader("Status");
            
            for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
                Parcel* parcel = findParcel(riders[i].assignedParcels[j]);
                if(parcel) {
                    t.addRow({
to_string(parcel->trackingID),
to_string(parcel->weight),
                        parcel->getStatusStr()
                    });
                }
            }
            
cout << "\n=== Rider " << riders[i].name << " Load ===\n";
cout << "Total Capacity: " << riders[i].capacity << " kg\n";
cout << "Current Load: " << riders[i].currentLoad << " kg\n";
cout << "Available: " << riders[i].getAvailableCapacity() << " kg\n";
            t.print();
            return;
        }
    }
cout << "\n[ERROR] Rider not found.\n";
}

void CourierSystem::displayAllRiderLoads() {
    for(int i = 0; i < riders.size(); i++) {
        displayRiderLoad(riders[i].riderID);
    }
}


void CourierSystem::displayQueueStatus() {
cout << "\n=== Queue Status ===\n";
    
    
    int pickupCount = 0;
    Queue<Parcel> tempPickup = pickupQueue;
    while(!tempPickup.empty()) {
        tempPickup.pop();
        pickupCount++;
    }
cout << "Pickup Queue: " << pickupCount << " parcel(s)\n";
    
    
    int warehouseCount = 0;
    MinHeap<Parcel> tempWarehouse = warehouseQueue;
    while(!tempWarehouse.empty()) {
        tempWarehouse.pop();
        warehouseCount++;
    }
cout << "Warehouse Queue: " << warehouseCount << " parcel(s)\n";
    
    
    int transitCount = 0;
    Queue<Parcel> tempTransit = transitQueue;
    while(!tempTransit.empty()) {
        tempTransit.pop();
        transitCount++;
    }
cout << "Transit Queue: " << transitCount << " parcel(s)\n";
}


void CourierSystem::detectMissingParcels() {
    Vector<int> missingParcels;
    
    
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].status == Status::InTransit) {
            
            bool inQueue = false;
            Queue<Parcel> temp = transitQueue;
            while(!temp.empty()) {
                if(temp.front().trackingID == allParcels[i].trackingID) {
                    inQueue = true;
                    break;
                }
                temp.pop();
            }
            
            if(!inQueue) {
                allParcels[i].status = Status::Missing;
                missingParcels.push_back(allParcels[i].trackingID);
            }
        }
    }
    
    if(missingParcels.size() == 0) {
cout << "\n[INFO] No missing parcels detected.\n";
        return;
    }
    
cout << "\n[ALERT] " << missingParcels.size() << " missing parcel(s) detected!\n";
    Table t;
    t.addHeader("Tracking ID");
    t.addHeader("Status");
    t.addHeader("Last Known Location");
    
    for(int i = 0; i < missingParcels.size(); i++) {
        Parcel* parcel = findParcel(missingParcels[i]);
        if(parcel) {
            City* city = cityMap.getNodeData(parcel->sourceCityID);
string cityName = city ? city->name : to_string(parcel->sourceCityID);
            t.addRow({
to_string(parcel->trackingID),
                parcel->getStatusStr(),
                cityName
            });
            parcel->history.push_back(getCurrentTimestamp() + " - Marked as Missing");
        }
    }
    t.print();
}


void CourierSystem::undoLastOperation() {
    if(operationHistory.empty()) {
cout << "\n[INFO] No operations to undo.\n";
        return;
    }
    
    OperationLog log = operationHistory.top();
    operationHistory.pop();
    
cout << "\n[INFO] Undoing operation: " << log.timestamp << "\n";
    
    bool success = false;
    
    switch(log.type) {
        case OperationType::AddParcel: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel) {
                
                parcelMap.remove(log.parcelID);
                removeParcelFromQueue(pickupQueue, log.parcelID);
                
                removeParcelFromQueue(transitQueue, log.parcelID);
                removeParcelFromHeap(warehouseQueue, log.parcelID);
                
                
                for(int i = 0; i < allParcels.size(); i++) {
                    if(allParcels[i].trackingID == log.parcelID) {
                        for(int j = i; j < allParcels.size() - 1; j++) {
                            allParcels[j] = allParcels[j+1];
                        }
                        allParcels.pop_back();
                        break;
                    }
                }
                nextTrackingID--; 
                success = true;
            } else {
cout << "[ERROR] Parcel not found for undo operation.\n";
            }
            break;
        }
        case OperationType::RemoveParcel: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel) {
                
                if(parcel->status != Status::Returned) {
cout << "[WARNING] Parcel status is " << parcel->getStatusStr() << ", expected Returned. Proceeding anyway...\n";
                }
                
                
                removeParcelFromQueue(pickupQueue, log.parcelID);
                removeParcelFromQueue(transitQueue, log.parcelID);
                removeParcelFromHeap(warehouseQueue, log.parcelID);
                
                
                parcel->status = Status::Pending;
                if(!isParcelInQueue(pickupQueue, log.parcelID)) {
                    pickupQueue.push(*parcel);
                    
                    if(!parcel->history.empty() && 
                       parcel->history.back().find("Withdrawn") != string::npos) {
                        parcel->history.pop_back();
                    }
                    success = true;
                } else {
cout << "[WARNING] Parcel already in pickup queue. State restored anyway.\n";
                    success = true;
                }
            } else {
cout << "[ERROR] Parcel not found for undo operation.\n";
            }
            break;
        }
        case OperationType::ProcessParcel: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel) {
                
                if(parcel->status != Status::InWarehouse) {
cout << "[WARNING] Parcel status is " << parcel->getStatusStr() << ", expected In Warehouse. Proceeding anyway...\n";
                }
                
                
                if(isParcelInHeap(warehouseQueue, log.parcelID)) {
                    removeParcelFromHeap(warehouseQueue, log.parcelID);
                }
                
                
                removeParcelFromQueue(pickupQueue, log.parcelID);
                
                
                parcel->status = Status::Pending;
                if(!isParcelInQueue(pickupQueue, log.parcelID)) {
                    pickupQueue.push(*parcel);
                    
                    if(!parcel->history.empty() && 
                       parcel->history.back().find("Warehouse") != string::npos) {
                        parcel->history.pop_back();
                    }
                    success = true;
                } else {
cout << "[WARNING] Parcel already in pickup queue. State restored anyway.\n";
                    success = true;
                }
            } else {
cout << "[ERROR] Parcel not found for undo operation.\n";
            }
            break;
        }
        case OperationType::AssignRider: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel && log.riderID != -1) {
                
                if(parcel->status != Status::InTransit) {
cout << "[WARNING] Parcel status is " << parcel->getStatusStr() << ", expected In Transit. Proceeding anyway...\n";
                }
                
                
                Rider* rider = nullptr;
                for(int i = 0; i < riders.size(); i++) {
                    if(riders[i].riderID == log.riderID) {
                        rider = &riders[i];
                        break;
                    }
                }
                
                if(rider) {
                    
                    bool foundInRider = false;
                    for(int j = 0; j < rider->assignedParcels.size(); j++) {
                        if(rider->assignedParcels[j] == log.parcelID) {
                            foundInRider = true;
                            for(int k = j; k < rider->assignedParcels.size() - 1; k++) {
                                rider->assignedParcels[k] = rider->assignedParcels[k+1];
                            }
                            rider->assignedParcels.pop_back();
                            break;
                        }
                    }
                    
                    if(foundInRider) {
                        rider->currentLoad -= parcel->weight;
                        if(rider->currentLoad < 0) rider->currentLoad = 0; 
                    }
                    
                    
                    removeParcelFromQueue(transitQueue, log.parcelID);
                    
                    
                    parcel->status = Status::InWarehouse;
                    
                    parcel->currentRoute.clear();
                    parcel->currentCityID = parcel->sourceCityID; 
                    
                    
                    if(!isParcelInHeap(warehouseQueue, log.parcelID)) {
                        warehouseQueue.push(*parcel);
                        
                        if(!parcel->history.empty() && 
                           parcel->history.back().find("Assigned to Rider") != string::npos) {
                            parcel->history.pop_back();
                        }
                        success = true;
                    } else {
cout << "[WARNING] Parcel already in warehouse queue. State restored anyway.\n";
                        success = true;
                    }
                } else {
cout << "[ERROR] Rider not found for undo operation.\n";
                }
            } else {
cout << "[ERROR] Invalid parcel or rider ID for undo operation.\n";
            }
            break;
        }
        case OperationType::CompleteDelivery: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel) {
                
                if(parcel->status != Status::Delivered) {
cout << "[WARNING] Parcel status is " << parcel->getStatusStr() << ", expected Delivered. Proceeding anyway...\n";
                }
                
                
                if(log.riderID != -1) {
                    bool riderFound = false;
                    for(int i = 0; i < riders.size(); i++) {
                        if(riders[i].riderID == log.riderID) {
                            riderFound = true;
                            
                            bool alreadyAssigned = false;
                            for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
                                if(riders[i].assignedParcels[j] == log.parcelID) {
                                    alreadyAssigned = true;
                                    break;
                                }
                            }
                            if(!alreadyAssigned) {
                                riders[i].currentLoad += parcel->weight;
                                riders[i].assignedParcels.push_back(log.parcelID);
                                
                                riders[i].currentCityID = parcel->destCityID;
                            }
                            break;
                        }
                    }
                    if(!riderFound) {
cout << "[WARNING] Rider ID " << log.riderID << " not found. Continuing undo...\n";
                    }
                }
                
                
                removeParcelFromQueue(transitQueue, log.parcelID);
                removeParcelFromQueue(pickupQueue, log.parcelID);
                removeParcelFromHeap(warehouseQueue, log.parcelID);
                
                
                parcel->status = Status::InTransit;
                parcel->currentCityID = parcel->destCityID; 
                
                
                if(!isParcelInQueue(transitQueue, log.parcelID)) {
                    transitQueue.push(*parcel);
                    
                    if(!parcel->history.empty() && 
                       parcel->history.back().find("Delivered") != string::npos) {
                        parcel->history.pop_back();
                    }
                    success = true;
                } else {
cout << "[WARNING] Parcel already in transit queue. State restored anyway.\n";
                    success = true;
                }
            } else {
cout << "[ERROR] Parcel not found for undo operation.\n";
            }
            break;
        }
        case OperationType::BlockRoute: {
            if(log.srcID != -1 && log.destID != -1) {
                unblockRoute(log.srcID, log.destID);
                success = true;
            } else {
cout << "[ERROR] Invalid route IDs for undo operation.\n";
            }
            break;
        }
        case OperationType::UnblockRoute: {
            if(log.srcID != -1 && log.destID != -1) {
                blockRoute(log.srcID, log.destID);
                success = true;
            } else {
cout << "[ERROR] Invalid route IDs for undo operation.\n";
            }
            break;
        }
        case OperationType::ReturnToSender: {
            Parcel* parcel = findParcel(log.parcelID);
            if(parcel) {
                
                if(parcel->status != Status::Returned) {
cout << "[WARNING] Parcel status is " << parcel->getStatusStr() << ", expected Returned. Proceeding anyway...\n";
                }
                
                
                removeParcelFromQueue(pickupQueue, log.parcelID);
                removeParcelFromQueue(transitQueue, log.parcelID);
                removeParcelFromHeap(warehouseQueue, log.parcelID);
                
                
                parcel->status = Status::InTransit;
                
                
                if(!isParcelInQueue(transitQueue, log.parcelID)) {
                    transitQueue.push(*parcel);
                    
                    if(!parcel->history.empty() && 
                       parcel->history.back().find("Returned to sender") != string::npos) {
                        parcel->history.pop_back();
                    }
                    success = true;
                } else {
cout << "[WARNING] Parcel already in transit queue. State restored anyway.\n";
                    success = true;
                }
            } else {
cout << "[ERROR] Parcel not found for undo operation.\n";
            }
            break;
        }
    }
    
    if(success) {
        saveData();
cout << "[SUCCESS] Operation undone successfully.\n";
    } else {
        
        operationHistory.push(log);
cout << "[ERROR] Undo operation failed. Operation restored to history.\n";
    }
}

void CourierSystem::displayOperationLog() {
    if(operationHistory.empty()) {
cout << "\n[INFO] No operations logged.\n";
        return;
    }
    
    
    Stack<OperationLog> temp = operationHistory;
    Vector<OperationLog> logs;
    
    while(!temp.empty()) {
        logs.push_back(temp.top());
        temp.pop();
    }
    
    Table t;
    t.addHeader("Timestamp");
    t.addHeader("Operation");
    t.addHeader("Parcel ID");
    t.addHeader("Previous State");
    t.addHeader("New State");
    
    for(int i = logs.size() - 1; i >= 0; i--) {
string opType;
        switch(logs[i].type) {
            case OperationType::AddParcel: opType = "Add Parcel"; break;
            case OperationType::RemoveParcel: opType = "Remove Parcel"; break;
            case OperationType::ProcessParcel: opType = "Process Parcel"; break;
            case OperationType::AssignRider: opType = "Assign Rider"; break;
            case OperationType::CompleteDelivery: opType = "Complete Delivery"; break;
            case OperationType::BlockRoute: opType = "Block Route"; break;
            case OperationType::UnblockRoute: opType = "Unblock Route"; break;
            case OperationType::ReturnToSender: opType = "Return to Sender"; break;
        }
        
        t.addRow({
            logs[i].timestamp,
            opType,
            logs[i].parcelID == -1 ? "N/A" : to_string(logs[i].parcelID),
            logs[i].previousState,
            logs[i].newState
        });
    }
    
cout << "\n=== Operation Log ===\n";
    t.print();
}

void CourierSystem::saveOperationLog(const string& filename) {
ofstream file(filename);
    if(!file.is_open()) {
cout << "\n[ERROR] Cannot open file for writing.\n";
        return;
    }
    
    if(operationHistory.empty()) {
        file << "Timestamp,Operation,ParcelID,PreviousState,NewState,RiderID,SrcID,DestID\n";
        file.close();
cout << "\n[INFO] Operation log is empty. File created with header only.\n";
        return;
    }
    
    
    Vector<OperationLog> logs;
    
    
    Stack<OperationLog> temp = operationHistory;
    
    
    while(!temp.empty()) {
        logs.push_back(temp.top());
        temp.pop();
    }
    
    file << "Timestamp,Operation,ParcelID,PreviousState,NewState,RiderID,SrcID,DestID\n";
    
    
    for(int i = logs.size() - 1; i >= 0; i--) {
string opType;
        switch(logs[i].type) {
            case OperationType::AddParcel: opType = "AddParcel"; break;
            case OperationType::RemoveParcel: opType = "RemoveParcel"; break;
            case OperationType::ProcessParcel: opType = "ProcessParcel"; break;
            case OperationType::AssignRider: opType = "AssignRider"; break;
            case OperationType::CompleteDelivery: opType = "CompleteDelivery"; break;
            case OperationType::BlockRoute: opType = "BlockRoute"; break;
            case OperationType::UnblockRoute: opType = "UnblockRoute"; break;
            case OperationType::ReturnToSender: opType = "ReturnToSender"; break;
            default: opType = "Unknown"; break;
        }
        
        
string prevState = logs[i].previousState;
string newState = logs[i].newState;
replace(prevState.begin(), prevState.end(), ',', ';');
replace(newState.begin(), newState.end(), ',', ';');
        
        file << logs[i].timestamp << "," << opType << "," << logs[i].parcelID << ","
             << prevState << "," << newState << ","
             << logs[i].riderID << "," << logs[i].srcID << "," << logs[i].destID << "\n";
    }
    
    file.close();
cout << "\n[SUCCESS] Operation log saved to " << filename << " (" << logs.size() << " entries)\n";
}

void CourierSystem::loadOperationLog(const string& filename) {
ifstream file(filename);
    if(!file.is_open()) {
cout << "\n[ERROR] Cannot open file for reading.\n";
        return;
    }
    
string line;
    if(!getline(file, line)) { 
        file.close();
cout << "\n[ERROR] Empty or invalid log file.\n";
        return;
    }
    
    int count = 0;
    while(getline(file, line)) {
        if(line.empty()) continue;
        
        auto tokens = CSVUtils::split(line, ',');
        if(tokens.size() < 5) continue;
        
        
        OperationType opType;
string opStr = tokens[1];
        if(opStr == "AddParcel") opType = OperationType::AddParcel;
        else if(opStr == "RemoveParcel") opType = OperationType::RemoveParcel;
        else if(opStr == "ProcessParcel") opType = OperationType::ProcessParcel;
        else if(opStr == "AssignRider") opType = OperationType::AssignRider;
        else if(opStr == "CompleteDelivery") opType = OperationType::CompleteDelivery;
        else if(opStr == "BlockRoute") opType = OperationType::BlockRoute;
        else if(opStr == "UnblockRoute") opType = OperationType::UnblockRoute;
        else if(opStr == "ReturnToSender") opType = OperationType::ReturnToSender;
        else continue;
        
        int parcelID = (tokens[2] != "N/A" && !tokens[2].empty()) ? stoi(tokens[2]) : -1;
        int riderID = tokens.size() > 5 ? stoi(tokens[5]) : -1;
        int srcID = tokens.size() > 6 ? stoi(tokens[6]) : -1;
        int destID = tokens.size() > 7 ? stoi(tokens[7]) : -1;
        
        
        switch(opType) {
            case OperationType::AddParcel:
                
                break;
            case OperationType::RemoveParcel:
                if(parcelID != -1) removeParcel(parcelID);
                break;
            case OperationType::ProcessParcel:
                if(parcelID != -1) {
                    Parcel* p = findParcel(parcelID);
                    if(p && p->status == Status::Pending) {
                        
                        processParcelByID(parcelID);
                    } else if(p) {
cout << "[INFO] Skipping ProcessParcel for ID " << parcelID << " - status is " << p->getStatusStr() << "\n";
                    }
                }
                break;
            case OperationType::AssignRider:
                if(parcelID != -1 && riderID != -1) {
                    Parcel* p = findParcel(parcelID);
                    if(p && p->status == Status::InWarehouse) {
                        assignRiderToParcel(parcelID, riderID);
                    } else if(p) {
cout << "[INFO] Skipping AssignRider for ID " << parcelID << " - status is " << p->getStatusStr() << "\n";
                    }
                }
                break;
            case OperationType::CompleteDelivery:
                if(parcelID != -1) {
                    Parcel* p = findParcel(parcelID);
                    if(p && p->status == Status::InTransit) {
                        
                        completeDeliveryByID(parcelID);
                    } else if(p) {
cout << "[INFO] Skipping CompleteDelivery for ID " << parcelID << " - status is " << p->getStatusStr() << "\n";
                    }
                }
                break;
            case OperationType::BlockRoute:
                if(srcID != -1 && destID != -1) {
                    if(!isEdgeBlocked(srcID, destID)) {
                        blockRoute(srcID, destID);
                    }
                }
                break;
            case OperationType::UnblockRoute:
                if(srcID != -1 && destID != -1) {
                    if(isEdgeBlocked(srcID, destID)) {
                        unblockRoute(srcID, destID);
                    }
                }
                break;
            case OperationType::ReturnToSender:
                if(parcelID != -1) {
                    Parcel* p = findParcel(parcelID);
                    if(p && p->status != Status::Returned) {
                        returnToSender(parcelID);
                    } else if(p) {
cout << "[INFO] Skipping ReturnToSender for ID " << parcelID << " - already returned\n";
                    }
                }
                break;
        }
        count++;
    }
    
    file.close();
cout << "\n[SUCCESS] Replayed " << count << " operations from " << filename << "\n";
}


void CourierSystem::trackParcel(int trackingID) {
    Parcel* parcel = findParcel(trackingID);
    if (!parcel) {
cout << "\n[ERROR] Parcel ID not found.\n";
        return;
    }
    
    Table t;
    t.addHeader("Attribute");
    t.addHeader("Value");
    
    t.addRow({"Tracking ID", to_string(parcel->trackingID)});
    t.addRow({"Sender", parcel->senderName});
    t.addRow({"Receiver", parcel->receiverName});
    t.addRow({"Priority", parcel->getPriorityStr()});
    t.addRow({"Weight", to_string(parcel->weight) + " kg (" + parcel->getWeightCategoryStr() + ")"});
    t.addRow({"Fragile", parcel->isFragile ? "Yes" : "No"});
    t.addRow({"Status", parcel->getStatusStr()});
    t.addRow({"Delivery Attempts", to_string(parcel->deliveryAttempts)});
    
    City* src = cityMap.getNodeData(parcel->sourceCityID);
    City* dest = cityMap.getNodeData(parcel->destCityID);
    City* current = cityMap.getNodeData(parcel->currentCityID);
    
    t.addRow({"Origin", src ? src->name : to_string(parcel->sourceCityID)});
    t.addRow({"Destination", dest ? dest->name : to_string(parcel->destCityID)});
    t.addRow({"Current Location", current ? current->name : (parcel->currentCityID != -1 ? to_string(parcel->currentCityID) : "Unknown")});
    
    int titleWidth = 35;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_MAGENTA BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 33; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Parcel Tracking Info   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
    
    
    if(parcel->currentRoute.size() > 0) {
cout << "\n--- Assigned Route Path ---\n";
cout << "Route: ";
        for(int i = 0; i < parcel->currentRoute.size(); i++) {
            City* c = cityMap.getNodeData(parcel->currentRoute[i]);
            if(c) cout << c->name;
            else cout << parcel->currentRoute[i];
            
            if(i < parcel->currentRoute.size() - 1) cout << " -> ";
        }
cout << "\n";
cout << "Total Route: " << parcel->currentRoute.size() << " cities, " << (parcel->currentRoute.size() - 1) << " segments\n";
        
        
        if(parcel->status == Status::InTransit && parcel->currentCityID != -1) {
            int currentIndex = -1;
            for(int i = 0; i < parcel->currentRoute.size(); i++) {
                if(parcel->currentRoute[i] == parcel->currentCityID) {
                    currentIndex = i;
                    break;
                }
            }
            if(currentIndex != -1) {
cout << "Progress: At city " << (currentIndex + 1) << " of " << parcel->currentRoute.size();
                if(currentIndex < parcel->currentRoute.size() - 1) {
                    City* next = cityMap.getNodeData(parcel->currentRoute[currentIndex + 1]);
cout << " (Next: " << (next ? next->name : to_string(parcel->currentRoute[currentIndex + 1])) << ")";
                }
cout << "\n";
            }
        }
    }
    
    
    if(parcel->status == Status::InTransit) {
        for(int i = 0; i < riders.size(); i++) {
            for(int j = 0; j < riders[i].assignedParcels.size(); j++) {
                if(riders[i].assignedParcels[j] == trackingID) {
                    City* riderCity = cityMap.getNodeData(riders[i].currentCityID);
cout << "\nAssigned Rider: " << riders[i].name << " (Location: " << (riderCity ? riderCity->name : to_string(riders[i].currentCityID)) << ")\n";
                    break;
                }
            }
        }
    }
    
cout << "\n--- Complete History Timeline ---\n";
    bool hasHistory = false;
    for(auto& event : parcel->history) {
        hasHistory = true;
cout << "  • " << event << "\n";
    }
    if(!hasHistory) {
cout << "  No history available.\n";
    }
}

void CourierSystem::displayCities() {
    Table t;
    t.addHeader("ID");
    t.addHeader("City Name");
    
    Vector<Graph<City>::GraphNode>& nodes = cityMap.getNodes();
    for(int i=0; i<nodes.size(); i++) {
        t.addRow({to_string(nodes[i].id), nodes[i].data.name});
    }
    
cout << "\n=== Available Cities ===\n";
    t.print();
}

void CourierSystem::displayAllParcels() {
    Table t;
    t.setHeaderColor(BRIGHT_MAGENTA BOLD);
    t.setBorderColor(MAGENTA);
    t.setRowColor(WHITE);
    t.addHeader("ID");
    t.addHeader("Sender");
    t.addHeader("Receiver");
    t.addHeader("Priority");
    t.addHeader("Weight");
    t.addHeader("Status");
    
    for(int i=0; i<allParcels.size(); i++) {
        Parcel& p = allParcels[i];
        t.addRow({
to_string(p.trackingID),
            p.senderName,
            p.receiverName,
            p.getPriorityStr(),
to_string(p.weight) + " kg",
            p.getStatusStr()
        });
    }
    
    cout << BRIGHT_MAGENTA BOLD;
    cout << "\n┏━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    cout <<   "┃       All Parcels       ┃\n";
    cout <<   "┗━━━━━━━━━━━━━━━━━━━━━━━━━┛\n" << RESET;
    t.print();
}


void CourierSystem::saveData() {
    CSVUtils::saveAllParcels("parcels.csv", allParcels);
    CSVUtils::saveAllRiders("riders.csv", riders);
    saveAdmins(); 
cout << "\n[SUCCESS] All data saved to files.\n";
}


void CourierSystem::displayDeliveryStatistics() {
    int total = allParcels.size();
    if (total == 0) {
        PRINT_INFO("No parcels in system.\n");
        return;
    }
    
    int delivered = 0, inTransit = 0, pending = 0, returned = 0;
    int overnight = 0, twoDay = 0, normal = 0;
    
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].status == Status::Delivered) delivered++;
        else if(allParcels[i].status == Status::InTransit) inTransit++;
        else if(allParcels[i].status == Status::Pending) pending++;
        else if(allParcels[i].status == Status::Returned) returned++;
        
        if(allParcels[i].priority == Priority::Overnight) overnight++;
        else if(allParcels[i].priority == Priority::TwoDay) twoDay++;
        else if(allParcels[i].priority == Priority::Normal) normal++;
    }
    
    Table t;
    t.setHeaderColor(BRIGHT_CYAN BOLD);
    t.setBorderColor(CYAN);
    t.setRowColor(WHITE);
    t.addHeader("Metric");
    t.addHeader("Count");
    t.addHeader("Percentage");
    
    t.addRow({"Total Parcels", to_string(total), "100%"});
    t.addRow({"Delivered", to_string(delivered), to_string((delivered * 100) / total) + "%"});
    t.addRow({"In Transit", to_string(inTransit), to_string((inTransit * 100) / total) + "%"});
    t.addRow({"Pending", to_string(pending), to_string((pending * 100) / total) + "%"});
    t.addRow({"Returned", to_string(returned), to_string((returned * 100) / total) + "%"});
    t.addRow({"", "", ""});
    t.addRow({"Overnight Priority", to_string(overnight), to_string((overnight * 100) / total) + "%"});
    t.addRow({"Two-Day Priority", to_string(twoDay), to_string((twoDay * 100) / total) + "%"});
    t.addRow({"Normal Priority", to_string(normal), to_string((normal * 100) / total) + "%"});
    
    int titleWidth = 35;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_CYAN BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 33; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Delivery Statistics   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
    
    cout << BRIGHT_GREEN << "\n[SUCCESS] Success Rate: " << BRIGHT_YELLOW << getDeliverySuccessRate() << "%" << RESET << "\n";
}

void CourierSystem::displayRiderStatistics(int riderID) {
    Rider* rider = nullptr;
    for(int i = 0; i < riders.size(); i++) {
        if(riders[i].riderID == riderID) {
            rider = &riders[i];
            break;
        }
    }
    
    if (!rider) {
cout << "\n[ERROR] Rider not found.\n";
        return;
    }
    
    int totalAssigned = 0, delivered = 0;
    int totalWeight = 0;
    
    for(int i = 0; i < rider->assignedParcels.size(); i++) {
        Parcel* p = findParcel(rider->assignedParcels[i]);
        if(p) {
            totalAssigned++;
            totalWeight += p->weight;
            if(p->status == Status::Delivered) delivered++;
        }
    }
    
    Table t;
    t.setHeaderColor(BRIGHT_YELLOW BOLD);
    t.setBorderColor(YELLOW);
    t.setRowColor(WHITE);
    t.addHeader("Metric");
    t.addHeader("Value");
    
    t.addRow({"Rider Name", rider->name});
    t.addRow({"Capacity", to_string(rider->capacity) + " kg"});
    t.addRow({"Current Load", to_string(rider->currentLoad) + " kg"});
    t.addRow({"Available", to_string(rider->getAvailableCapacity()) + " kg"});
    t.addRow({"Total Assigned", to_string(totalAssigned)});
    t.addRow({"Delivered", to_string(delivered)});
    t.addRow({"Total Weight Carried", to_string(totalWeight) + " kg"});
    
    City* city = cityMap.getNodeData(rider->currentCityID);
string cityName = city ? city->name : to_string(rider->currentCityID);
    t.addRow({"Current Location", cityName});
    
    int titleWidth = 30;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_YELLOW BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 28; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   Rider Statistics   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
}

double CourierSystem::getAverageDeliveryTime(Priority priority) {
    
    
    switch(priority) {
        case Priority::Overnight: return 12.0; 
        case Priority::TwoDay: return 36.0; 
        case Priority::Normal: return 72.0; 
        default: return 0.0;
    }
}

int CourierSystem::getDeliverySuccessRate() {
    int total = allParcels.size();
    if (total == 0) return 0;
    
    int delivered = 0;
    for(int i = 0; i < allParcels.size(); i++) {
        if(allParcels[i].status == Status::Delivered) {
            delivered++;
        }
    }
    
    return (delivered * 100) / total;
}


void CourierSystem::initializeAdmins() {
    
    bool superAdminExists = false;
    for(int i = 0; i < admins.size(); i++) {
        if(admins[i].username == "arslan" && admins[i].isSuperAdmin) {
            superAdminExists = true;
            break;
        }
    }
    
    if(!superAdminExists) {
        admins.push_back(Admin("arslan", "1122", true));
    }
}

bool CourierSystem::authenticateAdmin(const string& username, const string& password) {
    for(int i = 0; i < admins.size(); i++) {
        if(admins[i].username == username && admins[i].password == password) {
            return true;
        }
    }
    return false;
}

void CourierSystem::addAdmin(string username, string password) {
    
    for(int i = 0; i < admins.size(); i++) {
        if(admins[i].username == username) {
cout << "\n[ERROR] Admin with username '" << username << "' already exists.\n";
            return;
        }
    }
    
    if(username.empty() || password.empty()) {
cout << "\n[ERROR] Username and password cannot be empty.\n";
        return;
    }
    
    
    admins.push_back(Admin(username, password, false));
    saveAdmins();
cout << "\n[SUCCESS] Admin '" << username << "' added successfully.\n";
}

bool CourierSystem::removeAdmin(string username) {
    
    if(username == "arslan") {
        PRINT_ERROR("Cannot remove super admin.\n");
        return false;
    }
    
    for(int i = 0; i < admins.size(); i++) {
        if(admins[i].username == username) {
            
            for(int j = i; j < admins.size() - 1; j++) {
                admins[j] = admins[j+1];
            }
            admins.pop_back();
            saveAdmins();
            PRINT_SUCCESS("Admin '" << username << "' removed successfully.\n");
            return true;
        }
    }
    
    PRINT_ERROR("Admin with username '" << username << "' not found.\n");
    return false;
}

void CourierSystem::displayAdmins() {
    Table t;
    t.setHeaderColor(BRIGHT_RED BOLD);
    t.setBorderColor(RED);
    t.setRowColor(WHITE);
    t.addHeader("Username");
    t.addHeader("Type");
    
    for(int i = 0; i < admins.size(); i++) {
        string type = admins[i].isSuperAdmin ? "Super Admin" : "Admin";
        t.addRow({admins[i].username, type});
    }
    
    int titleWidth = 25;
    int totalWidth = titleWidth + 2;
    cout << BRIGHT_RED BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 23; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "   All Admins   ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    t.print();
}

void CourierSystem::loadAdmins() {
    CSVUtils::loadAdmins("admins.csv", admins);
    
    initializeAdmins();
}

void CourierSystem::saveAdmins() {
    CSVUtils::saveAllAdmins("admins.csv", admins);
}



#ifndef COURIER_SYSTEM_H
#define COURIER_SYSTEM_H

#include "Utils.h"
#include "DataStructures.h"
#include "Models.h"
#include <limits>
#include <algorithm>
#include <ctime>
#include <sstream>

using namespace std;

class CourierSystem
{
private:
    Graph<City> cityMap;

    
    Queue<Parcel> pickupQueue;      
    MinHeap<Parcel> warehouseQueue; 
    Queue<Parcel> transitQueue;     

    
    HashTable<int, Parcel *> parcelMap;

    
    Vector<Parcel> allParcels;

    
    Vector<Rider> riders;
    int nextRiderID;

    
    Vector<Admin> admins;

    
    Vector<BlockedEdge> blockedEdges;

    
    struct OverloadedEdge
    {
        int srcID;
        int destID;
        int currentLoad;
        int maxCapacity;
        OverloadedEdge() : srcID(-1), destID(-1), currentLoad(0), maxCapacity(100) {}
        OverloadedEdge(int s, int d, int load, int cap) : srcID(s), destID(d), currentLoad(load), maxCapacity(cap) {}
    };
    Vector<OverloadedEdge> overloadedEdges;

    
    Stack<OperationLog> operationHistory;

    int nextTrackingID;

    
    string getCurrentTimestamp() const;

    
    void logOperation(OperationType type, int parcelID, const string &prevState,
                      const string &newState, int riderID = -1, int srcID = -1, int destID = -1);

    
    bool isEdgeBlocked(int srcID, int destID) const;

    
    Parcel *findParcel(int trackingID);

    
    bool validateCityID(int cityID) const;
    bool validateRoute(int srcID, int destID) const;
    bool validateParcelData(int weight, int srcID, int destID) const;

    
    bool isParcelInQueue(const Queue<Parcel> &queue, int parcelID) const;
    bool isParcelInHeap(const MinHeap<Parcel> &heap, int parcelID) const;
    void removeParcelFromQueue(Queue<Parcel> &queue, int parcelID);
    void removeParcelFromHeap(MinHeap<Parcel> &heap, int parcelID);

public:
    CourierSystem();

    void loadData();
    void saveData(); 

    
    void addParcel(string sender, string receiver, Priority priority, int weight, int srcID, int destID);
    bool removeParcel(int trackingID); 
    void addCity(string name);
    void addRoute(int srcID, int destID, int distance);

    
    Vector<Parcel> getParcelsByWeightCategory(WeightCategory category);
    Vector<Parcel> getParcelsByDestination(int destCityID);
    Vector<Parcel> getFragileParcels();
    void sortParcelsByWeight(Vector<Parcel> &parcels);
    void sortParcelsByDestination(Vector<Parcel> &parcels);
    void sortParcelsByPriority(Vector<Parcel> &parcels);
    void displayParcelsByWeight();
    void displayParcelsByDestination();
    void displayParcelsByPriority();
    void markParcelAsFragile(int trackingID);

    
    void displayRoute(int srcID, int destID);
    void viewAllRoutes(int srcID, int destID); 
    int calculateShortestPath(int srcID, int destID, Vector<int> &path, bool avoidBlocked = true);
    void calculateAlternativeRoutes(int srcID, int destID, int k, Vector<Vector<int>> &routes, Vector<int> &distances);
    void displayAlternativeRoutes(int srcID, int destID, int count);

    
    void blockRoute(int srcID, int destID);
    void unblockRoute(int srcID, int destID);
    void displayBlockedRoutes();
    void recalculateActiveRoutes(); 

    
    void processNextParcel();                            
    void processParcelByID(int parcelID);                
    void assignRider();                                  
    void assignRiderToParcel(int parcelID, int riderID); 
    int findAvailableRider(int requiredCapacity, int cityID);
    void completeDelivery();                 
    void completeDeliveryByID(int parcelID); 
    void recordDispatch(int trackingID);
    void recordLoading(int trackingID, int cityID);
    void recordUnloading(int trackingID, int cityID);
    void recordDeliveryAttempt(int trackingID, bool success);
    void returnToSender(int trackingID);

    
    void addRider(string name, int capacity, int cityID);
    void displayRiders();
    void displayRiderLoad(int riderID);
    void displayAllRiderLoads();

    
    void displayQueueStatus();

    
    void detectMissingParcels();

    
    void undoLastOperation();
    void displayOperationLog();
    void saveOperationLog(const string &filename);
    void loadOperationLog(const string &filename);

    
    void trackParcel(int trackingID);
    void displayAllParcels();
    void displayCities();

    
    void displayDeliveryStatistics();
    void displayRiderStatistics(int riderID);
    double getAverageDeliveryTime(Priority priority);
    int getDeliverySuccessRate();

    
    void initializeAdmins(); 
    bool authenticateAdmin(const string &username, const string &password);
    void addAdmin(string username, string password);
    bool removeAdmin(string username);
    void displayAdmins();
    void loadAdmins();
    void saveAdmins();

    
    void markPathAsOverloaded(int srcID, int destID, int load);
    void unmarkPathAsOverloaded(int srcID, int destID);
    bool isPathOverloaded(int srcID, int destID) const;
    void displayOverloadedRoutes();

private:
    
    struct DijkstraNode
    {
        int id;
        int dist;
        bool operator<(const DijkstraNode &other) const
        {
            return dist > other.dist; 
        }
        bool operator>(const DijkstraNode &other) const
        {
            return dist < other.dist;
        }
    };

    
    void findKShortestPaths(int srcID, int destID, int k, Vector<Vector<int>> &paths, Vector<int> &distances);
};

#endif

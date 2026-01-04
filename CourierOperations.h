#ifndef COURIER_OPERATIONS_H
#define COURIER_OPERATIONS_H

#include "structures.h"
#include "Parcel.h"
#include <string>

using namespace std;

// Action types for undo functionality
enum ActionType
{
    ACTION_NONE,
    ACTION_ADD_PARCEL,
    ACTION_DISPATCH,
    ACTION_BLOCK_ROAD,
    ACTION_STATUS_CHANGE,
    ACTION_ADD_TO_WAREHOUSE,
    ACTION_PICKUP,
    ACTION_ASSIGN_RIDER,
    ACTION_DELIVERY_ATTEMPT,
    ACTION_MARK_DELIVERED,
    ACTION_RETURN_TO_SENDER
};

// Rider/Courier class
class Rider
{
private:
    int riderID;
    string name;
    int currentLoad;             // Current weight being carried (kg)
    int maxCapacity;             // Maximum weight capacity (kg)
    int assignedParcels;         // Number of parcels currently assigned
    int maxParcels;              // Maximum parcels that can be carried
    string currentZone;          // Zone the rider is assigned to
    bool available;              // Is rider available for new assignments
    CustomVector<int> parcelIDs; // IDs of parcels assigned to this rider

public:
    Rider() : riderID(0), currentLoad(0), maxCapacity(50),
              assignedParcels(0), maxParcels(10), available(true) {}

    Rider(int id, string n, int cap = 50, int maxP = 10)
        : riderID(id), name(n), currentLoad(0), maxCapacity(cap),
          assignedParcels(0), maxParcels(maxP), currentZone("Unassigned"),
          available(true) {}

    // Getters
    int getRiderID() const { return riderID; }
    string getName() const { return name; }
    int getCurrentLoad() const { return currentLoad; }
    int getMaxCapacity() const { return maxCapacity; }
    int getAssignedParcels() const { return assignedParcels; }
    int getMaxParcels() const { return maxParcels; }
    string getCurrentZone() const { return currentZone; }
    bool isAvailable() const { return available; }
    const CustomVector<int> &getParcelIDs() const { return parcelIDs; }

    // Setters
    void setRiderID(int id) { riderID = id; }
    void setName(string n) { name = n; }
    void setCurrentZone(string zone) { currentZone = zone; }
    void setAvailable(bool avail) { available = avail; }

    // Check if rider can take a parcel
    bool canTakeParcel(int parcelWeight) const
    {
        return available &&
               (currentLoad + parcelWeight <= maxCapacity) &&
               (assignedParcels < maxParcels);
    }

    // Assign a parcel to this rider
    bool assignParcel(int parcelID, int weight)
    {
        if (!canTakeParcel(weight))
            return false;
        parcelIDs.push_back(parcelID);
        currentLoad += weight;
        assignedParcels++;
        return true;
    }

    // Remove a parcel (delivered or returned)
    void removeParcel(int parcelID, int weight)
    {
        for (int i = 0; i < parcelIDs.getSize(); i++)
        {
            if (parcelIDs[i] == parcelID)
            {
                // Shift remaining elements
                for (int j = i; j < parcelIDs.getSize() - 1; j++)
                {
                    parcelIDs[j] = parcelIDs[j + 1];
                }
                parcelIDs.pop_back();
                break;
            }
        }
        currentLoad -= weight;
        if (currentLoad < 0)
            currentLoad = 0;
        assignedParcels--;
        if (assignedParcels < 0)
            assignedParcels = 0;
    }

    // Get remaining capacity
    int getRemainingCapacity() const
    {
        return maxCapacity - currentLoad;
    }

    // For priority comparison (riders with more capacity get priority)
    bool operator<(const Rider &other) const
    {
        // Lower load = higher priority (more available capacity)
        return currentLoad < other.currentLoad;
    }

    bool operator==(const Rider &other) const
    {
        return riderID == other.riderID;
    }
};

// Action class that stores full state for reversal
class Action
{
private:
    ActionType type;
    int parcelID;
    string prevStatus; // Previous status for status changes
    string data1;      // Flexible data (e.g., city1 for road block)
    string data2;      // Flexible data (e.g., city2 for road block)
    int roadWeight;    // Original road weight before blocking

    // For parcel actions - store full parcel data
    string destination;
    int weight;
    int priority;
    int riderID;      // For rider assignment undo
    int attemptCount; // For delivery attempt undo

public:
    Action(ActionType t, int id = -1, string d1 = "", string d2 = "")
        : type(t), parcelID(id), prevStatus(""), data1(d1), data2(d2),
          roadWeight(0), destination(""), weight(0), priority(3),
          riderID(-1), attemptCount(0) {}

    Action() : type(ACTION_NONE), parcelID(-1), roadWeight(0), weight(0),
               priority(3), riderID(-1), attemptCount(0) {}

    // Getters
    ActionType getType() const { return type; }
    int getParcelID() const { return parcelID; }
    string getPrevStatus() const { return prevStatus; }
    string getData1() const { return data1; }
    string getData2() const { return data2; }
    int getRoadWeight() const { return roadWeight; }
    string getDestination() const { return destination; }
    int getWeight() const { return weight; }
    int getPriority() const { return priority; }
    int getRiderID() const { return riderID; }
    int getAttemptCount() const { return attemptCount; }

    // Setters
    void setType(ActionType t) { type = t; }
    void setParcelID(int id) { parcelID = id; }
    void setPrevStatus(string status) { prevStatus = status; }
    void setData1(string d) { data1 = d; }
    void setData2(string d) { data2 = d; }
    void setRoadWeight(int w) { roadWeight = w; }
    void setDestination(string dest) { destination = dest; }
    void setWeight(int w) { weight = w; }
    void setPriority(int p) { priority = p; }
    void setRiderID(int id) { riderID = id; }
    void setAttemptCount(int count) { attemptCount = count; }

    // Store full parcel state
    void storeParcelState(const Parcel &p)
    {
        parcelID = p.getParcelID();
        destination = p.getDestination();
        weight = p.getWeight();
        priority = p.getPriority();
        prevStatus = p.getStatus();
        riderID = p.getAssignedRiderID();
        attemptCount = p.getDeliveryAttempts();
    }

    // Reconstruct parcel from stored state
    Parcel reconstructParcel() const
    {
        Parcel p(parcelID, destination, weight, priority);
        p.setStatus(prevStatus);
        p.setAssignedRiderID(riderID);
        p.setDeliveryAttempts(attemptCount);
        return p;
    }
};

class CourierOperations
{
private:
    CustomQueue<Parcel> pickupQueue;    // Parcels waiting to be picked up
    CustomQueue<Parcel> warehouseQueue; // Parcels in warehouse waiting to be loaded
    CustomQueue<Parcel> transitQueue;   // Parcels currently in transit
    CustomStack<Action> auditLog;
    HashMap<int, Rider> riders;          // Rider management
    HashMap<int, Parcel> missingParcels; // Track potentially missing parcels
    int nextRiderID;

public:
    CourierOperations();

    // ============== Pickup Queue Operations ==============
    void addToPickupQueue(Parcel p);
    Parcel processPickup();
    bool hasPickupPending();
    int getPickupQueueSize();
    Parcel peekPickupQueue();

    // ============== Warehouse Queue Operations ==============
    void addToWarehouse(Parcel p);
    Parcel loadFromWarehouse();
    bool hasPendingParcels();
    Parcel peekWarehouse();
    int getWarehouseSize();

    // ============== Transit Queue Operations ==============
    void addToTransit(Parcel p);
    Parcel completeTransit();
    bool hasTransitParcels();
    int getTransitQueueSize();
    Parcel peekTransitQueue();

    // ============== Rider Management ==============
    int addRider(string name, int capacity = 50, int maxParcels = 10);
    Rider getRider(int id);
    bool riderExists(int id);
    Rider *findBestRider(int parcelWeight, string zone = "");
    bool assignParcelToRider(int parcelID, int weight, int riderID);
    void releaseParcelFromRider(int parcelID, int weight, int riderID);
    CustomVector<Rider> getAllRiders();
    int getRiderCount();

    // ============== Missing Parcel Detection ==============
    void reportMissing(Parcel p, string reason);
    void resolveMissing(int parcelID);
    bool isParcelMissing(int parcelID);
    CustomVector<Parcel> getMissingParcels();
    int getMissingCount();

    // ============== Audit Log Operations ==============
    void logAction(Action act);
    Action getLastAction();  // Peek without pop
    Action undoLastAction(); // Pop for undo processing
    bool hasUndoableActions();
    int getAuditLogSize();

    // Get action type as string for display
    static string actionTypeToString(ActionType type);
};

#endif // COURIER_OPERATIONS_H
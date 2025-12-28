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
    ACTION_ADD_TO_WAREHOUSE
};

// Enhanced Action struct that stores full state for reversal
struct Action
{
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

    Action(ActionType t, int id = -1, string d1 = "", string d2 = "")
        : type(t), parcelID(id), prevStatus(""), data1(d1), data2(d2),
          roadWeight(0), destination(""), weight(0), priority(3) {}

    Action() : type(ACTION_NONE), parcelID(-1), roadWeight(0), weight(0), priority(3) {}

    // Store full parcel state
    void storeParcelState(const Parcel &p)
    {
        parcelID = p.parcelID;
        destination = p.destination;
        weight = p.weight;
        priority = p.priority;
        prevStatus = p.status;
    }

    // Reconstruct parcel from stored state
    Parcel reconstructParcel() const
    {
        Parcel p(parcelID, destination, weight, priority);
        p.status = prevStatus;
        return p;
    }
};

class CourierOperations
{
private:
    CustomQueue<Parcel> warehouseQueue;
    CustomStack<Action> auditLog;

public:
    CourierOperations();

    // Warehouse Queue Operations
    void addToWarehouse(Parcel p);
    Parcel loadFromWarehouse();
    bool hasPendingParcels();
    Parcel peekWarehouse();
    int getWarehouseSize();

    // Audit Log Operations
    void logAction(Action act);
    Action getLastAction();  // Peek without pop
    Action undoLastAction(); // Pop for undo processing
    bool hasUndoableActions();
    int getAuditLogSize();

    // Get action type as string for display
    static string actionTypeToString(ActionType type);
};

#endif // COURIER_OPERATIONS_H

#ifndef COURIER_OPERATIONS_H
#define COURIER_OPERATIONS_H

#include "structures.h"
#include "Parcel.h"
#include <string>

struct Action {
    std::string type; // "DISPATCH", "BLOCK_ROAD", etc.
    int parcelID;     // Involved parcel
    std::string prevStatus; // For undoing status change
    // For road blocking undo? 
    // Usually we'd store road info. Let's make it generic-ish.
    std::string data1; // e.g. From city / Old Status
    std::string data2; // e.g. To city
    
    Action(std::string t, int id = -1, std::string d1 = "", std::string d2 = "") 
        : type(t), parcelID(id), data1(d1), data2(d2) {}
    
    Action() : type("NONE") {} // Default constructor for Node<T>
};

class CourierOperations {
private:
    CustomQueue<Parcel> warehouseQueue;
    CustomStack<Action> auditLog;
    
public:
    CourierOperations();
    void addToWarehouse(Parcel p);
    Parcel loadFromWarehouse();
    bool hasPendingParcels();
    
    void logAction(Action act);
    Action undoLastAction(); 
    // Returns action so Main can perform the reverse logic (or we delegate reverse logic here if we have access to managers)
    // To keep it coupled correctly, maybe pass Managers? 
    // For now, let's just expose the stack operation. 
    // Actually, "Implement undoLastAction(): Pops from the stack and reverses the state." 
    // This implies this class might need access to ParcelManager/Graph OR Main handles it.
    // I'll return the Action and Main can switch on it to reverse, OR this class knows how.
};

#endif // COURIER_OPERATIONS_H

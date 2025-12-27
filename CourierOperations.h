#ifndef COURIER_OPERATIONS_H
#define COURIER_OPERATIONS_H

#include "structures.h"
#include "Parcel.h"
#include <string>

using namespace std;

struct Action {
    string type; 
    int parcelID;     
    string prevStatus; 
    string data1; 
    string data2; 
    
    Action(string t, int id = -1, string d1 = "", string d2 = "") 
        : type(t), parcelID(id), data1(d1), data2(d2) {}
    
    Action() : type("NONE") {}
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
};

#endif // COURIER_OPERATIONS_H

#ifndef PARCEL_MANAGER_H
#define PARCEL_MANAGER_H

#include "structures.h"
#include "Parcel.h"
#include <iostream>

class ParcelManager {
private:
    MinHeap<Parcel> priorityQueue;
    HashMap<int, Parcel> parcelMap;

public:
    ParcelManager();
    void addParcel(Parcel p);
    void dispatchNext();
    Parcel peekHighPriority();
    Parcel getParcel(int id);
    bool hasParcels();
};

#endif // PARCEL_MANAGER_H

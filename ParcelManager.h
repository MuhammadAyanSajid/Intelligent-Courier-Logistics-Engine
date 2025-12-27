#ifndef PARCEL_MANAGER_H
#define PARCEL_MANAGER_H

#include "structures.h"
#include "Parcel.h"
#include <iostream>
#include <fstream>

using namespace std;

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
    
    // File Handling
    void saveParcels(string filename);
    void loadParcels(string filename);
};

#endif // PARCEL_MANAGER_H

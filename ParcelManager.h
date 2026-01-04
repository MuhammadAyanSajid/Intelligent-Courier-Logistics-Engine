#ifndef PARCEL_MANAGER_H
#define PARCEL_MANAGER_H

#include "structures.h"
#include "Parcel.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class ParcelManager
{
private:
    MinHeap<Parcel> priorityQueue;
    HashMap<int, Parcel> parcelMap;

public:
    ParcelManager();
    void addParcel(Parcel p);
    bool parcelExists(int id);     // Check if parcel ID already exists
    bool canReuseParcelId(int id); // Check if parcel ID can be reused (delivered/returned)
    void updateParcel(Parcel p);   // Updates state in Map (and Heap if needed?)
    Parcel dispatchNext();         // Returns the dispatched parcel for tracking
    void removeParcel(int id);     // Remove parcel from map after dispatch
    Parcel peekHighPriority();
    Parcel getParcel(int id);
    bool hasParcels();
    CustomVector<Parcel> getAllParcels(); // Get all parcels for web API

    // Re-insert a parcel (for undo)
    void reinsertParcel(Parcel p);

    // File Handling
    void saveParcels(string filename);
    void loadParcels(string filename);
};

#endif // PARCEL_MANAGER_H

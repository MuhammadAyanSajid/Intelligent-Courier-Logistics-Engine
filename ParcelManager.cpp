#include "ParcelManager.h"

ParcelManager::ParcelManager() : parcelMap(100) {}

void ParcelManager::addParcel(Parcel p) {
    priorityQueue.insert(p);
    parcelMap.insert(p.parcelID, p);
    cout << "Parcel " << p.parcelID << " added to system (Priority: " << p.priority << ")." << endl;
}

void ParcelManager::dispatchNext() {
    if (priorityQueue.isEmpty()) {
        cout << "No parcels to dispatch." << endl;
        return;
    }

    Parcel p = priorityQueue.extractMin();
    cout << "Dispatching Parcel " << p.parcelID << " to " << p.destination << endl;
}

Parcel ParcelManager::peekHighPriority() {
    return priorityQueue.peek();
}

Parcel ParcelManager::getParcel(int id) {
    return parcelMap.get(id);
}

bool ParcelManager::hasParcels() {
    return !priorityQueue.isEmpty();
}

void ParcelManager::updateParcel(Parcel p) {
    // Update the copy in Map so it can be saved/retrieved correctly
    parcelMap.insert(p.parcelID, p);
    // Note: Heap order is based on Priority/Weight. If those changed, heap property might violate.
    // But usually we just update Status. Status doesn't affect Heap order.
    // If Priority changed, we should re-insert to Heap, but that's complex without 'remove'.
    // For this project, assume status updates only.
}

void ParcelManager::saveParcels(string filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cout << "Error opening file for saving: " << filename << endl;
        return;
    }
    
    CustomVector<Parcel> all = parcelMap.getAllValues();
    for(int i=0; i<all.getSize(); i++) {
        Parcel p = all[i];
        // Format: ID Dest Weight Priority Status
        // Note: Destination and Status should ideally be single words or handled carefully.
        // For simplicity, we assume they are safe strings (no spaces) or we use delimiters.
        // Just writing them with space separation:
        outFile << p.parcelID << " " << p.destination << " " << p.weight << " " << p.priority << " " << p.status << endl;
    }
    
    outFile.close();
    cout << "Parcels saved: " << all.getSize() << " records." << endl;
}

void ParcelManager::loadParcels(string filename) {
    ifstream inFile(filename);
    if (!inFile) {
        cout << "No saved parcels found." << endl;
        return;
    }
    
    int id, w, p;
    string dest, status;
    // Format: ID Dest Weight Priority Status
    // string status might be multi-word? Let's assume single word or simple format.
    
    while (inFile >> id >> dest >> w >> p >> status) {
        Parcel newP(id, dest, w, p);
        newP.status = status;
        addParcel(newP);
    }
    inFile.close();
    cout << "Parcels loaded from " << filename << endl;
}

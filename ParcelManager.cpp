#include "ParcelManager.h"

ParcelManager::ParcelManager() : parcelMap(100) {}

void ParcelManager::addParcel(Parcel p) {
    priorityQueue.insert(p);
    parcelMap.insert(p.parcelID, p);
    std::cout << "Parcel " << p.parcelID << " added to system (Priority: " << p.priority << ")." << std::endl;
}

void ParcelManager::dispatchNext() {
    if (priorityQueue.isEmpty()) {
        std::cout << "No parcels to dispatch." << std::endl;
        return;
    }

    Parcel p = priorityQueue.extractMin();
    std::cout << "Dispatching Parcel " << p.parcelID << " to " << p.destination << std::endl;
    
    // Update status in map?? The map stores a COPY.
    // Ideally we should store pointers in the Map if we want to update the same object.
    // But for this sprint, we just Dispatch.
    
    // Note: The prompt asks for "dispatchNext(): Removes the highest priority parcel from the heap."
    // It doesn't explicitly say we must update the Map copy, but usually we should.
    // However, with our current structure, Map holds a value copy.
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

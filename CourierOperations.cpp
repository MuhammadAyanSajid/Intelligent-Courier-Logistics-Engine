#include "CourierOperations.h"
#include <iostream>

CourierOperations::CourierOperations() {}

void CourierOperations::addToWarehouse(Parcel p) {
    warehouseQueue.enqueue(p);
    std::cout << "Parcel " << p.parcelID << " added to warehouse loading queue." << std::endl;
}

Parcel CourierOperations::loadFromWarehouse() {
    if (warehouseQueue.isEmpty()) return Parcel();
    return warehouseQueue.dequeue();
}

bool CourierOperations::hasPendingParcels() {
    return !warehouseQueue.isEmpty();
}

void CourierOperations::logAction(Action act) {
    auditLog.push(act);
}

Action CourierOperations::undoLastAction() {
    if (auditLog.isEmpty()) {
        std::cout << "Nothing to undo." << std::endl;
        return Action("NONE");
    }
    
    Action last = auditLog.pop();
    std::cout << "Undoing action: " << last.type << std::endl;
    return last;
}

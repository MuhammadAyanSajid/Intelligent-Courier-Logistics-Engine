#include "CourierOperations.h"
#include <iostream>

CourierOperations::CourierOperations() {}

void CourierOperations::addToWarehouse(Parcel p) {
    warehouseQueue.enqueue(p);
    cout << "Parcel " << p.parcelID << " added to warehouse loading queue." << endl;
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
        cout << "Nothing to undo." << endl;
        return Action("NONE");
    }
    
    Action last = auditLog.pop();
    cout << "Undoing action: " << last.type << endl;
    return last;
}

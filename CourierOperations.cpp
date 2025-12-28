#include "CourierOperations.h"
#include <iostream>

CourierOperations::CourierOperations() {}

// ============== Warehouse Queue Operations ==============

void CourierOperations::addToWarehouse(Parcel p)
{
    warehouseQueue.enqueue(p);
    cout << "Parcel " << p.parcelID << " added to warehouse loading queue." << endl;
}

Parcel CourierOperations::loadFromWarehouse()
{
    if (warehouseQueue.isEmpty())
    {
        cout << "Warehouse queue is empty." << endl;
        return Parcel();
    }
    Parcel p = warehouseQueue.dequeue();
    cout << "Parcel " << p.parcelID << " loaded from warehouse." << endl;
    return p;
}

bool CourierOperations::hasPendingParcels()
{
    return !warehouseQueue.isEmpty();
}

Parcel CourierOperations::peekWarehouse()
{
    return warehouseQueue.peek();
}

int CourierOperations::getWarehouseSize()
{
    return warehouseQueue.getSize();
}

// ============== Audit Log Operations ==============

void CourierOperations::logAction(Action act)
{
    auditLog.push(act);
    cout << "[AUDIT] Logged: " << actionTypeToString(act.type);
    if (act.parcelID > 0)
        cout << " (Parcel #" << act.parcelID << ")";
    cout << endl;
}

Action CourierOperations::getLastAction()
{
    return auditLog.peek();
}

Action CourierOperations::undoLastAction()
{
    if (auditLog.isEmpty())
    {
        cout << "Nothing to undo." << endl;
        return Action();
    }

    Action last = auditLog.pop();
    cout << "[UNDO] Reversing action: " << actionTypeToString(last.type) << endl;
    return last;
}

bool CourierOperations::hasUndoableActions()
{
    return !auditLog.isEmpty();
}

int CourierOperations::getAuditLogSize()
{
    return auditLog.getSize();
}

string CourierOperations::actionTypeToString(ActionType type)
{
    switch (type)
    {
    case ACTION_ADD_PARCEL:
        return "ADD_PARCEL";
    case ACTION_DISPATCH:
        return "DISPATCH";
    case ACTION_BLOCK_ROAD:
        return "BLOCK_ROAD";
    case ACTION_STATUS_CHANGE:
        return "STATUS_CHANGE";
    case ACTION_ADD_TO_WAREHOUSE:
        return "ADD_TO_WAREHOUSE";
    default:
        return "NONE";
    }
}

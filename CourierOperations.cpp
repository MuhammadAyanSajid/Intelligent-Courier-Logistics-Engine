#include "CourierOperations.h"
#include <iostream>

CourierOperations::CourierOperations() : riders(50), missingParcels(50), nextRiderID(1) {}

// ============== Pickup Queue Operations ==============

void CourierOperations::addToPickupQueue(Parcel p)
{
    pickupQueue.enqueue(p);
    cout << "Parcel " << p.getParcelID() << " added to pickup queue." << endl;
}

Parcel CourierOperations::processPickup()
{
    if (pickupQueue.isEmpty())
    {
        cout << "Pickup queue is empty." << endl;
        return Parcel();
    }
    Parcel p = pickupQueue.dequeue();
    cout << "Parcel " << p.getParcelID() << " picked up." << endl;
    return p;
}

bool CourierOperations::hasPickupPending()
{
    return !pickupQueue.isEmpty();
}

int CourierOperations::getPickupQueueSize()
{
    return pickupQueue.getSize();
}

Parcel CourierOperations::peekPickupQueue()
{
    return pickupQueue.peek();
}

// ============== Warehouse Queue Operations ==============

void CourierOperations::addToWarehouse(Parcel p)
{
    warehouseQueue.enqueue(p);
    cout << "Parcel " << p.getParcelID() << " added to warehouse loading queue." << endl;
}

Parcel CourierOperations::loadFromWarehouse()
{
    if (warehouseQueue.isEmpty())
    {
        cout << "Warehouse queue is empty." << endl;
        return Parcel();
    }
    Parcel p = warehouseQueue.dequeue();
    cout << "Parcel " << p.getParcelID() << " loaded from warehouse." << endl;
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

// ============== Transit Queue Operations ==============

void CourierOperations::addToTransit(Parcel p)
{
    transitQueue.enqueue(p);
    cout << "Parcel " << p.getParcelID() << " added to transit queue." << endl;
}

Parcel CourierOperations::completeTransit()
{
    if (transitQueue.isEmpty())
    {
        cout << "Transit queue is empty." << endl;
        return Parcel();
    }
    Parcel p = transitQueue.dequeue();
    cout << "Parcel " << p.getParcelID() << " completed transit." << endl;
    return p;
}

bool CourierOperations::hasTransitParcels()
{
    return !transitQueue.isEmpty();
}

int CourierOperations::getTransitQueueSize()
{
    return transitQueue.getSize();
}

Parcel CourierOperations::peekTransitQueue()
{
    return transitQueue.peek();
}

// ============== Rider Management ==============

int CourierOperations::addRider(string name, int capacity, int maxParcels)
{
    Rider r(nextRiderID, name, capacity, maxParcels);
    riders.insert(nextRiderID, r);
    cout << "Rider " << name << " (ID: " << nextRiderID << ") added to system." << endl;
    return nextRiderID++;
}

Rider CourierOperations::getRider(int id)
{
    return riders.get(id);
}

bool CourierOperations::riderExists(int id)
{
    return riders.contains(id);
}

Rider *CourierOperations::findBestRider(int parcelWeight, string zone)
{
    CustomVector<Rider> allRiders = riders.getAllValues();
    Rider *bestRider = nullptr;
    int bestScore = -1;

    for (int i = 0; i < allRiders.getSize(); i++)
    {
        Rider &r = allRiders[i];
        if (!r.canTakeParcel(parcelWeight))
            continue;

        // Calculate score (higher = better)
        int score = r.getRemainingCapacity();

        // Bonus for matching zone
        if (!zone.empty() && r.getCurrentZone() == zone)
        {
            score += 100;
        }

        // Prefer riders with fewer parcels (for load balancing)
        score += (r.getMaxParcels() - r.getAssignedParcels()) * 5;

        if (score > bestScore)
        {
            bestScore = score;
            // Need to get the actual rider from the map for modification
            if (riders.contains(r.getRiderID()))
            {
                bestRider = &allRiders[i];
            }
        }
    }

    return bestRider;
}

bool CourierOperations::assignParcelToRider(int parcelID, int weight, int riderID)
{
    if (!riders.contains(riderID))
    {
        cout << "Rider " << riderID << " not found." << endl;
        return false;
    }

    Rider r = riders.get(riderID);
    if (!r.canTakeParcel(weight))
    {
        cout << "Rider " << riderID << " cannot take this parcel (capacity/limit exceeded)." << endl;
        return false;
    }

    r.assignParcel(parcelID, weight);
    riders.insert(riderID, r); // Update in map
    cout << "Parcel " << parcelID << " assigned to Rider " << r.getName() << " (ID: " << riderID << ")." << endl;
    return true;
}

void CourierOperations::releaseParcelFromRider(int parcelID, int weight, int riderID)
{
    if (!riders.contains(riderID))
        return;

    Rider r = riders.get(riderID);
    r.removeParcel(parcelID, weight);
    riders.insert(riderID, r); // Update in map
    cout << "Parcel " << parcelID << " released from Rider " << r.getName() << "." << endl;
}

CustomVector<Rider> CourierOperations::getAllRiders()
{
    return riders.getAllValues();
}

int CourierOperations::getRiderCount()
{
    return riders.getSize();
}

// ============== Missing Parcel Detection ==============

void CourierOperations::reportMissing(Parcel p, string reason)
{
    p.updateStatus(STATUS_MISSING);
    p.addToHistory("[" + getCurrentTimestamp() + "] Reported missing: " + reason);
    missingParcels.insert(p.getParcelID(), p);
    cout << "[ALERT] Parcel " << p.getParcelID() << " reported MISSING: " << reason << endl;
}

void CourierOperations::resolveMissing(int parcelID)
{
    if (missingParcels.contains(parcelID))
    {
        missingParcels.remove(parcelID);
        cout << "Parcel " << parcelID << " resolved - removed from missing list." << endl;
    }
}

bool CourierOperations::isParcelMissing(int parcelID)
{
    return missingParcels.contains(parcelID);
}

CustomVector<Parcel> CourierOperations::getMissingParcels()
{
    return missingParcels.getAllValues();
}

int CourierOperations::getMissingCount()
{
    return missingParcels.getSize();
}

// ============== Audit Log Operations ==============

void CourierOperations::logAction(Action act)
{
    auditLog.push(act);
    cout << "[AUDIT] Logged: " << actionTypeToString(act.getType());
    if (act.getParcelID() > 0)
        cout << " (Parcel #" << act.getParcelID() << ")";
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
    cout << "[UNDO] Reversing action: " << actionTypeToString(last.getType()) << endl;
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
    case ACTION_PICKUP:
        return "PICKUP";
    case ACTION_ASSIGN_RIDER:
        return "ASSIGN_RIDER";
    case ACTION_DELIVERY_ATTEMPT:
        return "DELIVERY_ATTEMPT";
    case ACTION_MARK_DELIVERED:
        return "MARK_DELIVERED";
    case ACTION_RETURN_TO_SENDER:
        return "RETURN_TO_SENDER";
    default:
        return "NONE";
    }
}

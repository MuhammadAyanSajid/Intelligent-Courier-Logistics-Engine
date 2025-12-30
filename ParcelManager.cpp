#include "ParcelManager.h"

ParcelManager::ParcelManager() : parcelMap(100) {}

bool ParcelManager::parcelExists(int id)
{
    return parcelMap.contains(id);
}

void ParcelManager::addParcel(Parcel p)
{
    // Check for duplicate ID
    if (parcelMap.contains(p.getParcelID()))
    {
        cout << "Error: Parcel ID " << p.getParcelID() << " already exists!" << endl;
        return;
    }
    priorityQueue.insert(p);
    parcelMap.insert(p.getParcelID(), p);
    cout << "Parcel " << p.getParcelID() << " added to system (Priority: " << p.getPriority() << ")." << endl;
}

Parcel ParcelManager::dispatchNext()
{
    if (priorityQueue.isEmpty())
    {
        cout << "No parcels to dispatch." << endl;
        return Parcel();
    }

    Parcel p = priorityQueue.extractMin();
    cout << "Dispatching Parcel " << p.getParcelID() << " to " << p.getDestination() << endl;
    return p; // Return for tracking/undo
}

void ParcelManager::removeParcel(int id)
{
    parcelMap.remove(id);
}

void ParcelManager::reinsertParcel(Parcel p)
{
    // Used for undo - re-add to both heap and map
    priorityQueue.insert(p);
    parcelMap.insert(p.getParcelID(), p);
}

Parcel ParcelManager::peekHighPriority()
{
    return priorityQueue.peek();
}

Parcel ParcelManager::getParcel(int id)
{
    return parcelMap.get(id);
}

bool ParcelManager::hasParcels()
{
    return !priorityQueue.isEmpty();
}

CustomVector<Parcel> ParcelManager::getAllParcels()
{
    return parcelMap.getAllValues();
}

void ParcelManager::updateParcel(Parcel p)
{
    // Update the copy in Map so it can be saved/retrieved correctly
    parcelMap.insert(p.getParcelID(), p);
    // Note: Heap order is based on Priority/Weight. If those changed, heap property might violate.
    // But usually we just update Status. Status doesn't affect Heap order.
    // If Priority changed, we should re-insert to Heap, but that's complex without 'remove'.
    // For this project, assume status updates only.
}

// CSV-style format: ID|Destination|Weight|Priority|Status
// Using | as delimiter to handle spaces in status
void ParcelManager::saveParcels(string filename)
{
    ofstream outFile(filename);
    if (!outFile)
    {
        cout << "Error opening file for saving: " << filename << endl;
        return;
    }

    CustomVector<Parcel> all = parcelMap.getAllValues();
    for (int i = 0; i < all.getSize(); i++)
    {
        Parcel p = all[i];
        // Format: ID|Dest|Weight|Priority|Status (pipe-delimited)
        outFile << p.getParcelID() << "|"
                << p.getDestination() << "|"
                << p.getWeight() << "|"
                << p.getPriority() << "|"
                << p.getStatus() << endl;
    }

    outFile.close();
    cout << "Parcels saved: " << all.getSize() << " records." << endl;
}

void ParcelManager::loadParcels(string filename)
{
    ifstream inFile(filename);
    if (!inFile)
    {
        cout << "No saved parcels found. Starting fresh." << endl;
        return;
    }

    string line;
    int loadedCount = 0;

    while (getline(inFile, line))
    {
        if (line.empty())
            continue;

        // Parse pipe-delimited format: ID|Dest|Weight|Priority|Status
        int id = 0, w = 0, p = 0;
        string dest = "", status = "";

        // Manual parsing using stringstream and getline with delimiter
        stringstream ss(line);
        string token;
        int fieldNum = 0;

        while (getline(ss, token, '|'))
        {
            switch (fieldNum)
            {
            case 0:
                id = stoi(token);
                break;
            case 1:
                dest = token;
                break;
            case 2:
                w = stoi(token);
                break;
            case 3:
                p = stoi(token);
                break;
            case 4:
                status = token;
                break;
            }
            fieldNum++;
        }

        if (fieldNum >= 5 && id > 0)
        {
            Parcel newP(id, dest, w, p);
            newP.setStatus(status);
            // Add silently without duplicate messages during load
            if (!parcelMap.contains(newP.getParcelID()))
            {
                priorityQueue.insert(newP);
                parcelMap.insert(newP.getParcelID(), newP);
                loadedCount++;
            }
        }
    }

    inFile.close();
    cout << "Parcels loaded: " << loadedCount << " records from " << filename << endl;
}

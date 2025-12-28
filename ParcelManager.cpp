#include "ParcelManager.h"

ParcelManager::ParcelManager() : parcelMap(100) {}

bool ParcelManager::parcelExists(int id)
{
    return parcelMap.contains(id);
}

void ParcelManager::addParcel(Parcel p)
{
    // Check for duplicate ID
    if (parcelMap.contains(p.parcelID))
    {
        cout << "Error: Parcel ID " << p.parcelID << " already exists!" << endl;
        return;
    }
    priorityQueue.insert(p);
    parcelMap.insert(p.parcelID, p);
    cout << "Parcel " << p.parcelID << " added to system (Priority: " << p.priority << ")." << endl;
}

Parcel ParcelManager::dispatchNext()
{
    if (priorityQueue.isEmpty())
    {
        cout << "No parcels to dispatch." << endl;
        return Parcel();
    }

    Parcel p = priorityQueue.extractMin();
    cout << "Dispatching Parcel " << p.parcelID << " to " << p.destination << endl;
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
    parcelMap.insert(p.parcelID, p);
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

void ParcelManager::updateParcel(Parcel p)
{
    // Update the copy in Map so it can be saved/retrieved correctly
    parcelMap.insert(p.parcelID, p);
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
        outFile << p.parcelID << "|"
                << p.destination << "|"
                << p.weight << "|"
                << p.priority << "|"
                << p.status << endl;
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
            newP.status = status;
            // Add silently without duplicate messages during load
            if (!parcelMap.contains(newP.parcelID))
            {
                priorityQueue.insert(newP);
                parcelMap.insert(newP.parcelID, newP);
                loadedCount++;
            }
        }
    }

    inFile.close();
    cout << "Parcels loaded: " << loadedCount << " records from " << filename << endl;
}

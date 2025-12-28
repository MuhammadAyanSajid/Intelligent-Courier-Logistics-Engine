#include <iostream>
#include <string>
#include "structures.h"
#include "Parcel.h"
#include "ParcelManager.h"
#include "Graph.h"
#include "CourierOperations.h"
#include "ValidationUtils.h"

using namespace std;

// Display formatted header
void displayHeader()
{
    cout << "\n";
    cout << "+==================================================================+\n";
    cout << "|       INTELLIGENT COURIER LOGISTICS ENGINE v2.0                 |\n";
    cout << "|               DSA Lab Project - Final Sprint                    |\n";
    cout << "+==================================================================+\n";
}

// Display menu
void displayMenu()
{
    cout << "\n+-------------------- MAIN MENU --------------------+\n";
    cout << "|  1. Add New Parcel                                 |\n";
    cout << "|  2. View High Priority Parcel                      |\n";
    cout << "|  3. Process/Dispatch Next Parcel (Show Route)      |\n";
    cout << "|  4. Search Parcel by ID                            |\n";
    cout << "|  5. Block a Road (Simulate Traffic/Closure)        |\n";
    cout << "|  6. Undo Last Operation                            |\n";
    cout << "|  7. Add to Warehouse Loading Queue                 |\n";
    cout << "|  8. View System Status                             |\n";
    cout << "|  9. View All Cities/Routes                         |\n";
    cout << "|  0. Save & Exit                                    |\n";
    cout << "+----------------------------------------------------+\n";
}

// Display parcel info formatted
void displayParcel(const Parcel &p)
{
    string priorityStr;
    switch (p.priority)
    {
    case 1:
        priorityStr = "Overnight (HIGH)";
        break;
    case 2:
        priorityStr = "2-Day (MEDIUM)";
        break;
    case 3:
        priorityStr = "Normal (LOW)";
        break;
    default:
        priorityStr = "Unknown";
    }

    cout << "+------------------- PARCEL INFO -------------------+\n";
    cout << "| ID:          " << p.parcelID << "\n";
    cout << "| Destination: " << p.destination << "\n";
    cout << "| Weight:      " << p.weight << " kg\n";
    cout << "| Priority:    " << priorityStr << "\n";
    cout << "| Status:      " << p.status << "\n";
    cout << "+----------------------------------------------------+\n";
}

int main()
{
    displayHeader();

    // 1. Initialize Graph & Load Data
    Graph cityMap;
    cout << "\n[INIT] Loading map data...\n";
    cityMap.loadGraph("map_data.txt");

    // 2. Initialize Managers
    ParcelManager pm;
    CourierOperations ops;

    // Load Parcels
    cout << "[INIT] Loading parcel data...\n";
    pm.loadParcels("parcels.txt");

    // Track warehouse location (configurable)
    string warehouseCity = "A";
    cout << "[INIT] Warehouse set to: " << warehouseCity << "\n";

    int choice;
    do
    {
        displayMenu();
        choice = Utils::getIntInput("Enter choice (0-9): ", 0, 9);

        switch (choice)
        {
        case 1:
        { // Add New Parcel
            cout << "\n── ADD NEW PARCEL ──\n";
            int id = Utils::getIntInput("Enter Parcel ID (1-99999): ", 1, 99999);

            // Check for duplicate
            if (pm.parcelExists(id))
            {
                cout << "Error: Parcel ID " << id << " already exists!\n";
                break;
            }

            string dest = Utils::getValidCity("Enter Destination City: ", cityMap);
            int w = Utils::getIntInput("Enter Weight (1-1000 kg): ", 1, 1000);

            cout << "Priority Levels:\n";
            cout << "  1 = Overnight (High Priority)\n";
            cout << "  2 = 2-Day Delivery (Medium)\n";
            cout << "  3 = Normal Delivery (Low)\n";
            int p = Utils::getIntInput("Enter Priority (1-3): ", 1, 3);

            Parcel newP(id, dest, w, p);
            pm.addParcel(newP);

            // Log action for undo
            Action act(ACTION_ADD_PARCEL, id);
            act.storeParcelState(newP);
            ops.logAction(act);

            // Auto-save
            pm.saveParcels("parcels.txt");
            break;
        }

        case 2:
        { // View High Priority
            cout << "\n── HIGH PRIORITY PARCEL ──\n";
            if (pm.hasParcels())
            {
                Parcel p = pm.peekHighPriority();
                displayParcel(p);
            }
            else
            {
                cout << "No parcels in queue.\n";
            }
            break;
        }

        case 3:
        { // Dispatch Next
            cout << "\n── DISPATCH PARCEL ──\n";
            if (!pm.hasParcels())
            {
                cout << "No parcels to dispatch.\n";
                break;
            }

            // Get parcel BEFORE dispatch
            Parcel p = pm.peekHighPriority();
            string prevStatus = p.status;

            // Confirm dispatch
            displayParcel(p);
            if (!Utils::getYesNoInput("Dispatch this parcel?"))
            {
                cout << "Dispatch cancelled.\n";
                break;
            }

            // Dispatch (remove from heap)
            Parcel dispatched = pm.dispatchNext();

            // Calculate and show route
            cout << "\nCalculating optimal route from " << warehouseCity
                 << " to " << dispatched.destination << "...\n";
            cityMap.runDijkstra(warehouseCity, dispatched.destination);

            // Update status
            dispatched.updateStatus("In Transit");
            pm.updateParcel(dispatched);

            // Log action with full state for undo
            Action act(ACTION_DISPATCH, dispatched.parcelID);
            act.destination = dispatched.destination;
            act.weight = dispatched.weight;
            act.priority = dispatched.priority;
            act.prevStatus = prevStatus;
            ops.logAction(act);

            // Remove from HashMap since it's dispatched
            pm.removeParcel(dispatched.parcelID);
            cout << "\nParcel " << dispatched.parcelID << " is now In Transit.\n";

            // Auto-save
            pm.saveParcels("parcels.txt");
            break;
        }

        case 4:
        { // Search Parcel
            cout << "\n── SEARCH PARCEL ──\n";
            int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.parcelID != 0)
            {
                displayParcel(p);

                // Show history
                cout << "\n─── TRACKING HISTORY ───\n";
                Node<string> *curr = p.history.getHead();
                int eventNum = 1;
                while (curr)
                {
                    cout << "  " << eventNum++ << ". " << curr->data << "\n";
                    curr = curr->next;
                }
                if (p.history.getSize() == 0)
                {
                    cout << "  (No history recorded)\n";
                }
            }
            else
            {
                cout << "Parcel ID " << id << " not found.\n";
            }
            break;
        }

        case 5:
        { // Block Road
            cout << "\n── BLOCK ROAD ──\n";
            string from = Utils::getValidCity("Block road FROM city: ", cityMap);
            string to = Utils::getValidCity("Block road TO city: ", cityMap);

            if (from == to)
            {
                cout << "Error: Cannot block road to same city.\n";
                break;
            }

            // Get current weight before blocking (for undo)
            int originalWeight = cityMap.getRoadWeight(from, to);
            if (originalWeight < 0)
            {
                cout << "No direct road exists between " << from << " and " << to << ".\n";
                break;
            }

            cityMap.blockRoad(from, to);

            // Log for undo
            Action act(ACTION_BLOCK_ROAD, -1, from, to);
            act.roadWeight = originalWeight;
            ops.logAction(act);

            // Auto-save
            cityMap.saveGraph("map_data.txt");
            break;
        }

        case 6:
        { // Undo
            cout << "\n── UNDO LAST OPERATION ──\n";
            if (!ops.hasUndoableActions())
            {
                cout << "Nothing to undo.\n";
                break;
            }

            Action act = ops.undoLastAction();

            switch (act.type)
            {
            case ACTION_ADD_PARCEL:
            {
                // Remove the parcel that was added
                pm.removeParcel(act.parcelID);
                cout << "Removed parcel " << act.parcelID << " (ADD undone).\n";
                break;
            }
            case ACTION_DISPATCH:
            {
                // Re-insert the dispatched parcel
                Parcel restored = act.reconstructParcel();
                pm.reinsertParcel(restored);
                cout << "Restored parcel " << act.parcelID << " to queue (DISPATCH undone).\n";
                break;
            }
            case ACTION_BLOCK_ROAD:
            {
                // Unblock the road
                cityMap.unblockRoad(act.data1, act.data2, act.roadWeight);
                cout << "Unblocked road " << act.data1 << " <-> " << act.data2 << ".\n";
                cityMap.saveGraph("map_data.txt");
                break;
            }
            default:
                cout << "Unknown action type, cannot undo.\n";
            }

            // Auto-save after undo
            pm.saveParcels("parcels.txt");
            break;
        }

        case 7:
        { // Add to Warehouse Queue
            cout << "\n── WAREHOUSE LOADING QUEUE ──\n";
            int id = Utils::getIntInput("Enter Parcel ID to add to loading queue: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.parcelID != 0)
            {
                ops.addToWarehouse(p);
                p.updateStatus("Queued for Loading");
                pm.updateParcel(p);

                Action act(ACTION_ADD_TO_WAREHOUSE, id);
                ops.logAction(act);

                // Auto-save
                pm.saveParcels("parcels.txt");
            }
            else
            {
                cout << "Parcel ID " << id << " not found.\n";
            }
            break;
        }

        case 8:
        { // System Status
            cout << "\n+================== SYSTEM STATUS ==================+\n";
            cout << "| Cities in Network:        " << cityMap.getCityCount() << "\n";
            cout << "| Parcels in Priority Queue: " << (pm.hasParcels() ? "Yes" : "Empty") << "\n";
            cout << "| Warehouse Queue Size:     " << ops.getWarehouseSize() << "\n";
            cout << "| Audit Log Actions:        " << ops.getAuditLogSize() << "\n";
            cout << "| Warehouse Location:       " << warehouseCity << "\n";
            cout << "+===================================================+\n";
            break;
        }

        case 9:
        { // View Cities/Routes
            cout << "\n── CITY NETWORK ──\n";
            CustomVector<string> cities = cityMap.getAllCities();
            cout << "Cities: ";
            for (int i = 0; i < cities.getSize(); i++)
            {
                cout << cities[i];
                if (i < cities.getSize() - 1)
                    cout << ", ";
            }
            cout << "\n\nUse the map_data.txt file to see all routes.\n";
            break;
        }

        case 0:
        { // Save & Exit
            cout << "\n── SAVING DATA ──\n";
            pm.saveParcels("parcels.txt");
            cityMap.saveGraph("map_data.txt");
            cout << "\nThank you for using the Courier Logistics Engine!\n";
            break;
        }

        default:
            cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);

    return 0;
}

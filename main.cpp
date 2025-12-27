#include <iostream>
#include <string>
#include "structures.h"
#include "Parcel.h"
#include "ParcelManager.h"
#include "Graph.h"
#include "CourierOperations.h"
#include "ValidationUtils.h"

using namespace std;

// No need for clearInput helper since ValidationUtils handles it

int main() {
    // 1. Initialize Graph & Load Data
    Graph cityMap;
    cityMap.loadGraph("map_data.txt"); // Will load default if missing

    // 2. Initialize Managers
    ParcelManager pm;
    CourierOperations ops;
    
    // Load Parcels
    pm.loadParcels("parcels.txt");

    int choice;
    do {
        cout << "\n=== Intelligent Courier Logistics Engine ===\n";
        cout << "1. Add Parcel\n";
        cout << "2. View High Priority Parcel\n";
        cout << "3. Process/Dispatch Next Parcel (Show Route)\n";
        cout << "4. Search Parcel by ID\n";
        cout << "5. Block a Road\n";
        cout << "6. Undo Last Operation\n";
        cout << "7. Add to Loading Queue (Ops)\n";
        cout << "0. Save & Exit\n";
        
        choice = Utils::getIntInput("Enter choice: ", 0, 7);

        switch (choice) {
            case 1: {
                int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
                string dest = Utils::getAlphabeticStringInput("Enter Destination (City Name): ");
                int w = Utils::getIntInput("Enter Weight: ", 1, 1000);
                int p = Utils::getIntInput("Enter Priority (1-3): ", 1, 3);
                
                Parcel newP(id, dest, w, p);
                pm.addParcel(newP);
                ops.logAction(Action("ADD_PARCEL", id));
                break;
            }
            case 2: {
                if (pm.hasParcels()) {
                    Parcel p = pm.peekHighPriority();
                    cout << "Next Priority Parcel:\n";
                    cout << "ID: " << p.parcelID << ", Dest: " << p.destination 
                              << ", Priority: " << p.priority << ", Status: " << p.status << endl;
                } else {
                    cout << "No parcels in queue.\n";
                }
                break;
            }
            case 3: {
                if (pm.hasParcels()) {
                    Parcel p = pm.peekHighPriority(); 
                    pm.dispatchNext(); 
                    
                    cout << "Calculating route for Parcel " << p.parcelID << " to " << p.destination << "...\n";
                    cityMap.runDijkstra("A", p.destination); // Assuming A is warehouse
                    
                    p.updateStatus("In Transit"); 
                    pm.updateParcel(p); // Sync change to manager
                    ops.logAction(Action("DISPATCH", p.parcelID, "Loaded")); 
                }
                break;
            }
            case 4: {
                int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
                Parcel p = pm.getParcel(id);
                if (p.parcelID != 0) { 
                    cout << "Parcel Found:\n";
                    cout << "ID: " << p.parcelID << ", Dest: " << p.destination << ", Status: " << p.status << "\n";
                    cout << "History:\n";
                    Node<string>* curr = p.history.getHead();
                    while (curr) {
                        cout << " - " << curr->data << "\n";
                        curr = curr->next;
                    }
                } else {
                    cout << "Parcel not found.\n";
                }
                break;
            }
            case 5: {
                string from = Utils::getStringInput("Block road from: ");
                string to = Utils::getStringInput("Block road to: ");
                cityMap.blockRoad(from, to);
                ops.logAction(Action("BLOCK_ROAD", -1, from, to));
                break;
            }
            case 6: {
                Action act = ops.undoLastAction();
                if (act.type != "NONE") {
                    cout << "Undo successful (logically).\n";
                    // Real logic needed here if we want deep state reversal
                }
                break;
            }
            case 7: {
                 int id = Utils::getIntInput("Enter Parcel ID to load to truck: ", 1, 99999);
                 Parcel p = pm.getParcel(id);
                 if (p.parcelID != 0) {
                     ops.addToWarehouse(p);
                 } else {
                     cout << "Parcel not found." << endl;
                 }
                 break;
            }
            case 0:
                pm.saveParcels("parcels.txt");
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}

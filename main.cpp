#include <iostream>
#include <string>
#include "structures.h"
#include "Parcel.h"
#include "ParcelManager.h"
#include "Graph.h"
#include "CourierOperations.h"

// Helper to clear input buffer
void clearInput() {
    std::cin.clear();
    char c;
    while((c = getchar()) != '\n' && c != EOF);
}

int main() {
    // 1. Initialize Graph
    Graph cityMap;
    cityMap.addLocation("A");
    cityMap.addLocation("B");
    cityMap.addLocation("C");
    cityMap.addLocation("D");
    cityMap.addLocation("E");
    
    cityMap.addRoute("A", "B", 10);
    cityMap.addRoute("A", "C", 5);
    cityMap.addRoute("B", "D", 20);
    cityMap.addRoute("C", "B", 8); // Directed or undirected logic handled in Graph
    cityMap.addRoute("C", "E", 15);
    cityMap.addRoute("D", "E", 5);

    // 2. Initialize Managers
    ParcelManager pm;
    CourierOperations ops;
    
    // Seed some initial parcels logic if needed? 
    // Or just let user add them.

    int choice;
    do {
        std::cout << "\n=== Intelligent Courier Logistics Engine ===\n";
        std::cout << "1. Add Parcel\n";
        std::cout << "2. View High Priority Parcel\n";
        std::cout << "3. Process/Dispatch Next Parcel (Show Route)\n";
        std::cout << "4. Search Parcel by ID\n";
        std::cout << "5. Block a Road\n";
        std::cout << "6. Undo Last Operation\n";
        std::cout << "7. Add to Loading Queue (Ops)\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            clearInput();
            continue;
        }

        switch (choice) {
            case 1: {
                int id, w, p;
                std::string dest;
                std::cout << "Enter Parcel ID: "; std::cin >> id;
                std::cout << "Enter Destination (A-E): "; std::cin >> dest;
                std::cout << "Enter Weight: "; std::cin >> w;
                std::cout << "Enter Priority (1-3): "; std::cin >> p;
                
                Parcel newP(id, dest, w, p);
                pm.addParcel(newP);
                ops.logAction(Action("ADD_PARCEL", id));
                break;
            }
            case 2: {
                if (pm.hasParcels()) {
                    Parcel p = pm.peekHighPriority();
                    std::cout << "Next Priority Parcel:\n";
                    std::cout << "ID: " << p.parcelID << ", Dest: " << p.destination 
                              << ", Priority: " << p.priority << ", Status: " << p.status << std::endl;
                } else {
                    std::cout << "No parcels in queue.\n";
                }
                break;
            }
            case 3: {
                // Dispatch logic: Pop from Heap, Find Route
                if (pm.hasParcels()) {
                    Parcel p = pm.peekHighPriority(); // Peek first to get info
                    pm.dispatchNext(); // Removes it
                    
                    std::cout << "Calculating route for Parcel " << p.parcelID << " to " << p.destination << "...\n";
                    // Assume starting point is "A" (Warehouse)
                    cityMap.runDijkstra("A", p.destination);
                    
                    p.updateStatus("In Transit"); // Update local object
                    // In a real system we'd update the persistent record.
                    
                    ops.logAction(Action("DISPATCH", p.parcelID, "Loaded")); // Track prev status
                }
                break;
            }
            case 4: {
                int id;
                std::cout << "Enter Parcel ID: "; std::cin >> id;
                Parcel p = pm.getParcel(id);
                if (p.parcelID != 0) { // Assuming 0 is invalid/default
                    std::cout << "Parcel Found:\n";
                    std::cout << "ID: " << p.parcelID << ", Dest: " << p.destination << ", Status: " << p.status << "\n";
                    std::cout << "History:\n";
                    Node<std::string>* curr = p.history.getHead();
                    while (curr) {
                        std::cout << " - " << curr->data << "\n";
                        curr = curr->next;
                    }
                } else {
                    std::cout << "Parcel not found.\n";
                }
                break;
            }
            case 5: {
                std::string from, to;
                std::cout << "Block road from: "; std::cin >> from;
                std::cout << "Block road to: "; std::cin >> to;
                cityMap.blockRoad(from, to);
                ops.logAction(Action("BLOCK_ROAD", -1, from, to));
                break;
            }
            case 6: {
                Action act = ops.undoLastAction();
                if (act.type == "ADD_PARCEL") {
                    std::cout << "Undo Add Parcel " << act.parcelID << ": functionality not fully impl (would remove from heap).\n";
                } else if (act.type == "DISPATCH") {
                     std::cout << "Undo Dispatch Parcel " << act.parcelID << ": would re-add to heap.\n";
                     // pm.addParcel(...) - Need to retrieve parcel data. 
                } else if (act.type == "BLOCK_ROAD") {
                    std::cout << "Undo Block Road " << act.data1 << "-" << act.data2 << ": Need unblock logic.\n";
                    // restore route
                    cityMap.addRoute(act.data1, act.data2, 10); // Dummy weight restore?
                }
                break;
            }
            case 7: {
                 // Warehouse Ops
                 // Demo: load pending
                 int id; 
                 std::cout << "Enter Parcel ID to load to truck: "; std::cin >> id;
                 Parcel p = pm.getParcel(id);
                 if (p.parcelID != 0) {
                     ops.addToWarehouse(p);
                 }
                 break;
            }
            case 0:
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}

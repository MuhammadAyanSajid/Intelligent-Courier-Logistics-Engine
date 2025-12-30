
#include "structures.h"
#include "Parcel.h"
#include "ParcelManager.h"
#include "Graph.h"
#include "CourierOperations.h"
#include "ValidationUtils.h"
#include "WebServer.h"
#include <cstring>

using namespace std;

// Display formatted header
void displayHeader()
{
    cout << "\n";
    cout << "+==================================================================+\n";
    cout << "|       INTELLIGENT COURIER LOGISTICS ENGINE v3.0                 |\n";
    cout << "|               DSA Lab Project - Complete Edition                |\n";
    cout << "+==================================================================+\n";
}

// Display main menu
void displayMenu()
{
    cout << "\n+==================== MAIN MENU ====================+\n";
    cout << "|                                                    |\n";
    cout << "|  --- PARCEL MANAGEMENT ---                         |\n";
    cout << "|   1. Add New Parcel                                |\n";
    cout << "|   2. View High Priority Parcel                     |\n";
    cout << "|   3. Search Parcel by ID                           |\n";
    cout << "|   4. Dispatch Parcel (Show Route)                  |\n";
    cout << "|                                                    |\n";
    cout << "|  --- DELIVERY OPERATIONS ---                       |\n";
    cout << "|   5. Record Delivery Attempt                       |\n";
    cout << "|   6. Mark Parcel as Delivered                      |\n";
    cout << "|   7. Return Parcel to Sender                       |\n";
    cout << "|                                                    |\n";
    cout << "|  --- QUEUE MANAGEMENT ---                          |\n";
    cout << "|   8. Pickup Queue Operations                       |\n";
    cout << "|   9. Warehouse Queue Operations                    |\n";
    cout << "|  10. Transit Queue Operations                      |\n";
    cout << "|                                                    |\n";
    cout << "|  --- ROUTING & ZONES ---                           |\n";
    cout << "|  11. Find Alternative Routes                       |\n";
    cout << "|  12. Block/Unblock Road                            |\n";
    cout << "|  13. Zone Management                               |\n";
    cout << "|                                                    |\n";
    cout << "|  --- RIDER MANAGEMENT ---                          |\n";
    cout << "|  14. Manage Riders                                 |\n";
    cout << "|  15. Assign Parcel to Rider                        |\n";
    cout << "|                                                    |\n";
    cout << "|  --- SYSTEM ---                                    |\n";
    cout << "|  16. Missing Parcel Detection                      |\n";
    cout << "|  17. Undo Last Operation                           |\n";
    cout << "|  18. View System Status                            |\n";
    cout << "|  19. View All Cities/Routes                        |\n";
    cout << "|   0. Save & Exit                                   |\n";
    cout << "+----------------------------------------------------+\n";
}

// Display parcel info formatted
void displayParcel(const Parcel &p)
{
    cout << "+------------------- PARCEL INFO -------------------+\n";
    cout << "| ID:              " << p.getParcelID() << "\n";
    cout << "| Destination:     " << p.getDestination() << "\n";
    cout << "| Zone:            " << p.getZone() << "\n";
    cout << "| Weight:          " << p.getWeight() << " kg (" << getWeightCategoryName(p.getWeightCat()) << ")\n";
    cout << "| Priority:        " << p.getPriorityString() << "\n";
    cout << "| Status:          " << p.getStatus() << "\n";
    cout << "| Delivery Attempts: " << p.getDeliveryAttempts() << "/" << p.getMaxAttempts() << "\n";
    cout << "| Assigned Rider:  " << (p.getAssignedRiderID() > 0 ? to_string(p.getAssignedRiderID()) : "None") << "\n";
    cout << "+----------------------------------------------------+\n";
}

// Display rider info
void displayRider(const Rider &r)
{
    cout << "+------------------- RIDER INFO --------------------+\n";
    cout << "| ID:              " << r.getRiderID() << "\n";
    cout << "| Name:            " << r.getName() << "\n";
    cout << "| Current Load:    " << r.getCurrentLoad() << "/" << r.getMaxCapacity() << " kg\n";
    cout << "| Assigned Parcels: " << r.getAssignedParcels() << "/" << r.getMaxParcels() << "\n";
    cout << "| Zone:            " << r.getCurrentZone() << "\n";
    cout << "| Available:       " << (r.isAvailable() ? "Yes" : "No") << "\n";
    cout << "+----------------------------------------------------+\n";
}

// Function to run CLI mode
void runCLI(Graph &cityMap, ParcelManager &pm, CourierOperations &ops, string &warehouseCity);

int main(int argc, char *argv[])
{
    // Check for --web flag
    bool webMode = false;
    int webPort = 8080;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--web") == 0)
        {
            webMode = true;
        }
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
        {
            webPort = atoi(argv[++i]);
        }
    }

    displayHeader();

    // 1. Initialize Graph & Load Data
    Graph cityMap;
    cout << "\n[INIT] Loading map data...\n";
    cityMap.loadGraph("map_data.txt");

    // Auto-assign zones if not already done
    if (cityMap.getAllZones().isEmpty())
    {
        cout << "[INIT] Auto-assigning zones...\n";
        cityMap.autoAssignZones();
    }

    // 2. Initialize Managers
    ParcelManager pm;
    CourierOperations ops;

    // Add default riders
    cout << "[INIT] Setting up riders...\n";
    ops.addRider("Ali", 50, 10);
    ops.addRider("Ahmed", 60, 12);
    ops.addRider("Sara", 45, 8);

    // Load Parcels
    cout << "[INIT] Loading parcel data...\n";
    pm.loadParcels("parcels.txt");

    // Track warehouse location (configurable)
    string warehouseCity = "Karachi";
    cout << "[INIT] Warehouse set to: " << warehouseCity << "\n";

    // Check mode
    if (webMode)
    {
        // Start Web Server
        cout << "\n[MODE] Starting Web Interface...\n";
        WebServer server(pm, ops, cityMap, warehouseCity);
        server.start(webPort);
        return 0;
    }

    // CLI Mode
    runCLI(cityMap, pm, ops, warehouseCity);
    return 0;
}

void runCLI(Graph &cityMap, ParcelManager &pm, CourierOperations &ops, string &warehouseCity)
{
    int choice;
    do
    {
        displayMenu();
        choice = Utils::getIntInput("Enter choice (0-19): ", 0, 19);

        switch (choice)
        {
        case 1:
        { // Add New Parcel
            cout << "\n-- ADD NEW PARCEL --\n";
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

            string sender = Utils::getAlphanumericInput("Enter Sender Address: ");

            Parcel newP(id, dest, w, p, sender);

            // Assign zone based on destination
            string zone = cityMap.getCityZone(dest);
            newP.setZone(zone);

            pm.addParcel(newP);

            // Log action for undo
            Action act(ACTION_ADD_PARCEL, id);
            act.storeParcelState(newP);
            ops.logAction(act);

            cout << "\nParcel created with weight category: " << getWeightCategoryName(newP.getWeightCat()) << "\n";

            // Auto-save
            pm.saveParcels("parcels.txt");
            break;
        }

        case 2:
        { // View High Priority
            cout << "\n-- HIGH PRIORITY PARCEL --\n";
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
        { // Search Parcel
            cout << "\n-- SEARCH PARCEL --\n";
            int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.getParcelID() != 0)
            {
                displayParcel(p);

                // Show history
                cout << "\n--- TRACKING HISTORY ---\n";
                Node<string> *curr = p.getHistory().getHead();
                int eventNum = 1;
                while (curr)
                {
                    cout << "  " << eventNum++ << ". " << curr->data << "\n";
                    curr = curr->next;
                }
                if (p.getHistory().getSize() == 0)
                {
                    cout << "  (No history recorded)\n";
                }
            }
            else
            {
                // Check if it's in missing parcels
                if (ops.isParcelMissing(id))
                {
                    cout << "[ALERT] Parcel " << id << " is marked as MISSING!\n";
                }
                else
                {
                    cout << "Parcel ID " << id << " not found.\n";
                }
            }
            break;
        }

        case 4:
        { // Dispatch Parcel
            cout << "\n-- DISPATCH PARCEL --\n";
            if (!pm.hasParcels())
            {
                cout << "No parcels to dispatch.\n";
                break;
            }

            // Get parcel BEFORE dispatch
            Parcel p = pm.peekHighPriority();
            string prevStatus = p.getStatus();

            // Confirm dispatch
            displayParcel(p);
            if (!Utils::getYesNoInput("Dispatch this parcel?"))
            {
                cout << "Dispatch cancelled.\n";
                break;
            }

            // Dispatch (remove from heap)
            Parcel dispatched = pm.dispatchNext();

            // Show alternative routes
            cout << "\nCalculating routes from " << warehouseCity
                 << " to " << dispatched.getDestination() << "...\n";
            cityMap.displayAlternativeRoutes(warehouseCity, dispatched.getDestination(), 3);

            // Update status and add to transit
            dispatched.updateStatus(STATUS_IN_TRANSIT);
            pm.updateParcel(dispatched);
            ops.addToTransit(dispatched);

            // Log action with full state for undo
            Action act(ACTION_DISPATCH, dispatched.getParcelID());
            act.setDestination(dispatched.getDestination());
            act.setWeight(dispatched.getWeight());
            act.setPriority(dispatched.getPriority());
            act.setPrevStatus(prevStatus);
            ops.logAction(act);

            // Remove from HashMap since it's dispatched
            pm.removeParcel(dispatched.getParcelID());
            cout << "\nParcel " << dispatched.getParcelID() << " is now In Transit.\n";

            // Auto-save
            pm.saveParcels("parcels.txt");
            break;
        }

        case 5:
        { // Record Delivery Attempt
            cout << "\n-- RECORD DELIVERY ATTEMPT --\n";
            int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.getParcelID() == 0)
            {
                cout << "Parcel ID " << id << " not found.\n";
                break;
            }

            if (!p.canAttemptDelivery())
            {
                cout << "Cannot attempt delivery. Status: " << p.getStatus() << "\n";
                break;
            }

            string reason = Utils::getLineInput("Enter failure reason: ");

            Action act(ACTION_DELIVERY_ATTEMPT, p.getParcelID());
            act.storeParcelState(p);

            bool canRetry = p.recordDeliveryAttempt(reason);
            pm.updateParcel(p);
            ops.logAction(act);

            if (!canRetry)
            {
                cout << "\n[ALERT] Max delivery attempts reached!\n";
                cout << "Parcel will be returned to sender: " << p.getSenderAddress() << "\n";
            }
            else
            {
                cout << "\nAttempt recorded. " << (p.getMaxAttempts() - p.getDeliveryAttempts())
                     << " attempts remaining.\n";
            }

            pm.saveParcels("parcels.txt");
            break;
        }

        case 6:
        { // Mark as Delivered
            cout << "\n-- MARK PARCEL DELIVERED --\n";
            int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.getParcelID() == 0)
            {
                cout << "Parcel ID " << id << " not found.\n";
                break;
            }

            if (p.getStatus() == STATUS_DELIVERED)
            {
                cout << "Parcel is already delivered.\n";
                break;
            }

            Action act(ACTION_MARK_DELIVERED, p.getParcelID());
            act.storeParcelState(p);

            p.markDelivered();

            // Release from rider if assigned
            if (p.getAssignedRiderID() > 0)
            {
                ops.releaseParcelFromRider(p.getParcelID(), p.getWeight(), p.getAssignedRiderID());
                p.setAssignedRiderID(-1);
            }

            pm.updateParcel(p);
            ops.logAction(act);

            cout << "\nParcel " << id << " marked as DELIVERED!\n";
            pm.saveParcels("parcels.txt");
            break;
        }

        case 7:
        { // Return to Sender
            cout << "\n-- RETURN PARCEL TO SENDER --\n";
            int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(id);

            if (p.getParcelID() == 0)
            {
                cout << "Parcel ID " << id << " not found.\n";
                break;
            }

            // Validate parcel can be returned
            if (p.getStatus() == STATUS_DELIVERED)
            {
                cout << "Error: Cannot return an already delivered parcel.\n";
                break;
            }
            if (p.getStatus() == STATUS_RETURNED)
            {
                cout << "Error: Parcel is already marked for return.\n";
                break;
            }

            string reason = Utils::getLineInput("Enter return reason: ");

            Action act(ACTION_RETURN_TO_SENDER, p.getParcelID());
            act.storeParcelState(p);

            p.returnToSender(reason);

            // Release from rider if assigned
            if (p.getAssignedRiderID() > 0)
            {
                ops.releaseParcelFromRider(p.getParcelID(), p.getWeight(), p.getAssignedRiderID());
                p.setAssignedRiderID(-1);
            }

            pm.updateParcel(p);
            ops.logAction(act);

            cout << "\nParcel " << id << " marked for return to: " << p.getSenderAddress() << "\n";
            pm.saveParcels("parcels.txt");
            break;
        }

        case 8:
        { // Pickup Queue Operations
            cout << "\n-- PICKUP QUEUE --\n";
            cout << "1. Add parcel to pickup queue\n";
            cout << "2. Process next pickup\n";
            cout << "3. View pickup queue status\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 3);

            if (subChoice == 1)
            {
                int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
                Parcel p = pm.getParcel(id);
                if (p.getParcelID() != 0)
                {
                    // Validate parcel status before adding to queue
                    if (p.getStatus() == STATUS_DELIVERED || p.getStatus() == STATUS_RETURNED)
                    {
                        cout << "Error: Cannot add " << p.getStatus() << " parcel to pickup queue.\n";
                        break;
                    }
                    p.updateStatus(STATUS_PENDING_PICKUP);
                    ops.addToPickupQueue(p);
                    pm.updateParcel(p);

                    Action act(ACTION_PICKUP, id);
                    act.storeParcelState(p);
                    ops.logAction(act);
                }
                else
                {
                    cout << "Parcel not found.\n";
                }
            }
            else if (subChoice == 2)
            {
                if (ops.hasPickupPending())
                {
                    Parcel p = ops.processPickup();
                    p.updateStatus(STATUS_PICKED_UP);
                    pm.updateParcel(p);
                    displayParcel(p);
                }
                else
                {
                    cout << "Pickup queue is empty.\n";
                }
            }
            else
            {
                cout << "Pickup Queue Size: " << ops.getPickupQueueSize() << "\n";
                if (ops.hasPickupPending())
                {
                    cout << "Next pickup: Parcel #" << ops.peekPickupQueue().getParcelID() << "\n";
                }
            }
            pm.saveParcels("parcels.txt");
            break;
        }

        case 9:
        { // Warehouse Queue Operations
            cout << "\n-- WAREHOUSE QUEUE --\n";
            cout << "1. Add parcel to warehouse\n";
            cout << "2. Load from warehouse\n";
            cout << "3. View warehouse status\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 3);

            if (subChoice == 1)
            {
                int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
                Parcel p = pm.getParcel(id);
                if (p.getParcelID() != 0)
                {
                    // Validate parcel status before adding to warehouse
                    if (p.getStatus() == STATUS_DELIVERED || p.getStatus() == STATUS_RETURNED)
                    {
                        cout << "Error: Cannot add " << p.getStatus() << " parcel to warehouse.\n";
                        break;
                    }
                    p.updateStatus(STATUS_IN_WAREHOUSE);
                    ops.addToWarehouse(p);
                    pm.updateParcel(p);

                    Action act(ACTION_ADD_TO_WAREHOUSE, id);
                    ops.logAction(act);
                }
                else
                {
                    cout << "Parcel not found.\n";
                }
            }
            else if (subChoice == 2)
            {
                if (ops.hasPendingParcels())
                {
                    Parcel p = ops.loadFromWarehouse();
                    p.updateStatus(STATUS_QUEUED_LOADING);
                    pm.updateParcel(p);
                    displayParcel(p);
                }
                else
                {
                    cout << "Warehouse queue is empty.\n";
                }
            }
            else
            {
                cout << "Warehouse Queue Size: " << ops.getWarehouseSize() << "\n";
                if (ops.hasPendingParcels())
                {
                    cout << "Next in warehouse: Parcel #" << ops.peekWarehouse().getParcelID() << "\n";
                }
            }
            pm.saveParcels("parcels.txt");
            break;
        }

        case 10:
        { // Transit Queue Operations
            cout << "\n-- TRANSIT QUEUE --\n";
            cout << "1. View transit queue\n";
            cout << "2. Complete transit (arrive at destination)\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 2);

            if (subChoice == 1)
            {
                cout << "Transit Queue Size: " << ops.getTransitQueueSize() << "\n";
                if (ops.hasTransitParcels())
                {
                    cout << "Next to arrive: Parcel #" << ops.peekTransitQueue().getParcelID() << "\n";
                }
            }
            else
            {
                if (ops.hasTransitParcels())
                {
                    Parcel p = ops.completeTransit();
                    p.updateStatus(STATUS_OUT_FOR_DELIVERY);
                    pm.updateParcel(p);
                    cout << "Parcel " << p.getParcelID() << " arrived at destination area.\n";
                    displayParcel(p);
                }
                else
                {
                    cout << "Transit queue is empty.\n";
                }
            }
            pm.saveParcels("parcels.txt");
            break;
        }

        case 11:
        { // Find Alternative Routes
            cout << "\n-- FIND ALTERNATIVE ROUTES --\n";
            string from = Utils::getValidCity("From city: ", cityMap);
            string to = Utils::getValidCity("To city: ", cityMap);

            if (from == to)
            {
                cout << "Error: Source and destination cannot be the same city.\n";
                break;
            }

            int k = Utils::getIntInput("Number of routes to find (1-5): ", 1, 5);

            cityMap.displayAlternativeRoutes(from, to, k);
            break;
        }

        case 12:
        { // Block/Unblock Road
            cout << "\n-- ROAD MANAGEMENT --\n";
            cout << "1. Block a road\n";
            cout << "2. Unblock a road\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 2);

            string from = Utils::getValidCity("From city: ", cityMap);
            string to = Utils::getValidCity("To city: ", cityMap);

            if (from == to)
            {
                cout << "Error: Cannot modify road to same city.\n";
                break;
            }

            if (subChoice == 1)
            {
                int originalWeight = cityMap.getRoadWeight(from, to);
                if (originalWeight < 0)
                {
                    cout << "No direct road exists between " << from << " and " << to << ".\n";
                    break;
                }

                cityMap.blockRoad(from, to);

                Action act(ACTION_BLOCK_ROAD, -1, from, to);
                act.setRoadWeight(originalWeight);
                ops.logAction(act);
            }
            else
            {
                if (!cityMap.isRoadBlocked(from, to))
                {
                    cout << "Road is not blocked.\n";
                    break;
                }
                int weight = Utils::getIntInput("Enter road weight: ", 1, 1000);
                cityMap.unblockRoad(from, to, weight);
            }

            cityMap.saveGraph("map_data.txt");
            break;
        }

        case 13:
        { // Zone Management
            cout << "\n-- ZONE MANAGEMENT --\n";
            cout << "1. View all zones\n";
            cout << "2. Assign city to zone\n";
            cout << "3. View cities in zone\n";
            cout << "4. Auto-assign all zones\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 4);

            if (subChoice == 1)
            {
                CustomVector<string> zones = cityMap.getAllZones();
                cout << "\nExisting Zones:\n";
                for (int i = 0; i < zones.getSize(); i++)
                {
                    cout << "  - " << zones[i] << "\n";
                }
                if (zones.isEmpty())
                {
                    cout << "  (No zones defined)\n";
                }
            }
            else if (subChoice == 2)
            {
                string city = Utils::getValidCity("Enter city: ", cityMap);
                string zone = Utils::getAlphabeticStringInput("Enter zone name (letters only): ");
                cityMap.assignCityToZone(city, zone);
            }
            else if (subChoice == 3)
            {
                string zone = Utils::getAlphabeticStringInput("Enter zone name (letters only): ");
                CustomVector<string> cities = cityMap.getCitiesInZone(zone);
                cout << "\nCities in zone '" << zone << "':\n";
                for (int i = 0; i < cities.getSize(); i++)
                {
                    cout << "  - " << cities[i] << "\n";
                }
                if (cities.isEmpty())
                {
                    cout << "  (No cities in this zone)\n";
                }
            }
            else
            {
                cityMap.autoAssignZones();
            }
            cityMap.saveGraph("map_data.txt");
            break;
        }

        case 14:
        { // Manage Riders
            cout << "\n-- RIDER MANAGEMENT --\n";
            cout << "1. Add new rider\n";
            cout << "2. View all riders\n";
            cout << "3. View rider details\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 3);

            if (subChoice == 1)
            {
                string name = Utils::getAlphabeticStringInput("Enter rider name (letters only): ");
                int capacity = Utils::getIntInput("Max weight capacity (kg): ", 10, 200);
                int maxParcels = Utils::getIntInput("Max parcels: ", 1, 50);
                int id = ops.addRider(name, capacity, maxParcels);
                cout << "Rider added with ID: " << id << "\n";
            }
            else if (subChoice == 2)
            {
                CustomVector<Rider> riders = ops.getAllRiders();
                cout << "\n--- ALL RIDERS ---\n";
                for (int i = 0; i < riders.getSize(); i++)
                {
                    Rider r = riders[i];
                    cout << "  [" << r.getRiderID() << "] " << r.getName()
                         << " - Load: " << r.getCurrentLoad() << "/" << r.getMaxCapacity() << " kg"
                         << ", Parcels: " << r.getAssignedParcels() << "/" << r.getMaxParcels()
                         << (r.isAvailable() ? " (Available)" : " (Busy)") << "\n";
                }
                if (riders.isEmpty())
                {
                    cout << "  (No riders in system)\n";
                }
            }
            else
            {
                int id = Utils::getIntInput("Enter rider ID: ", 1, 99999);
                if (ops.riderExists(id))
                {
                    displayRider(ops.getRider(id));
                }
                else
                {
                    cout << "Rider not found.\n";
                }
            }
            break;
        }

        case 15:
        { // Assign Parcel to Rider
            cout << "\n-- ASSIGN PARCEL TO RIDER --\n";
            int parcelID = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
            Parcel p = pm.getParcel(parcelID);

            if (p.getParcelID() == 0)
            {
                cout << "Parcel not found.\n";
                break;
            }

            // Validate parcel status
            if (p.getStatus() == STATUS_DELIVERED)
            {
                cout << "Error: Cannot assign a delivered parcel to a rider.\n";
                break;
            }
            if (p.getStatus() == STATUS_RETURNED)
            {
                cout << "Error: Cannot assign a returned parcel to a rider.\n";
                break;
            }

            if (p.getAssignedRiderID() > 0)
            {
                cout << "Parcel already assigned to Rider " << p.getAssignedRiderID() << "\n";
                break;
            }

            cout << "1. Auto-assign best rider\n";
            cout << "2. Manually select rider\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 2);

            int riderID = -1;
            if (subChoice == 1)
            {
                Rider *best = ops.findBestRider(p.getWeight(), p.getZone());
                if (best != nullptr)
                {
                    riderID = best->getRiderID();
                    cout << "Best rider found: " << best->getName() << " (ID: " << riderID << ")\n";
                }
                else
                {
                    cout << "No suitable rider available.\n";
                    break;
                }
            }
            else
            {
                riderID = Utils::getIntInput("Enter Rider ID: ", 1, 99999);
                if (!ops.riderExists(riderID))
                {
                    cout << "Error: Rider ID " << riderID << " not found.\n";
                    break;
                }
            }

            if (ops.assignParcelToRider(parcelID, p.getWeight(), riderID))
            {
                p.setAssignedRiderID(riderID);
                p.updateStatus(STATUS_OUT_FOR_DELIVERY);
                pm.updateParcel(p);

                Action act(ACTION_ASSIGN_RIDER, parcelID);
                act.storeParcelState(p);
                act.setRiderID(riderID);
                ops.logAction(act);

                pm.saveParcels("parcels.txt");
            }
            break;
        }

        case 16:
        { // Missing Parcel Detection
            cout << "\n-- MISSING PARCEL DETECTION --\n";
            cout << "1. Report parcel as missing\n";
            cout << "2. View missing parcels\n";
            cout << "3. Resolve missing parcel\n";
            int subChoice = Utils::getIntInput("Enter choice: ", 1, 3);

            if (subChoice == 1)
            {
                int id = Utils::getIntInput("Enter Parcel ID: ", 1, 99999);
                Parcel p = pm.getParcel(id);
                if (p.getParcelID() != 0)
                {
                    string reason = Utils::getLineInput("Enter reason/details: ");
                    ops.reportMissing(p, reason);
                    pm.updateParcel(p);
                    pm.saveParcels("parcels.txt");
                }
                else
                {
                    cout << "Parcel not found in system.\n";
                }
            }
            else if (subChoice == 2)
            {
                CustomVector<Parcel> missing = ops.getMissingParcels();
                cout << "\n--- MISSING PARCELS (" << missing.getSize() << ") ---\n";
                for (int i = 0; i < missing.getSize(); i++)
                {
                    Parcel p = missing[i];
                    cout << "  [" << p.getParcelID() << "] " << p.getDestination()
                         << " - Last status: " << p.getStatus() << "\n";
                }
                if (missing.isEmpty())
                {
                    cout << "  (No missing parcels)\n";
                }
            }
            else
            {
                int id = Utils::getIntInput("Enter Parcel ID to resolve: ", 1, 99999);
                ops.resolveMissing(id);
            }
            break;
        }

        case 17:
        { // Undo
            cout << "\n-- UNDO LAST OPERATION --\n";
            if (!ops.hasUndoableActions())
            {
                cout << "Nothing to undo.\n";
                break;
            }

            Action act = ops.undoLastAction();

            switch (act.getType())
            {
            case ACTION_ADD_PARCEL:
            {
                pm.removeParcel(act.getParcelID());
                cout << "Removed parcel " << act.getParcelID() << " (ADD undone).\n";
                break;
            }
            case ACTION_DISPATCH:
            {
                Parcel restored = act.reconstructParcel();
                pm.reinsertParcel(restored);
                cout << "Restored parcel " << act.getParcelID() << " to queue (DISPATCH undone).\n";
                break;
            }
            case ACTION_BLOCK_ROAD:
            {
                cityMap.unblockRoad(act.getData1(), act.getData2(), act.getRoadWeight());
                cout << "Unblocked road " << act.getData1() << " <-> " << act.getData2() << ".\n";
                cityMap.saveGraph("map_data.txt");
                break;
            }
            case ACTION_ASSIGN_RIDER:
            {
                Parcel p = pm.getParcel(act.getParcelID());
                if (p.getParcelID() != 0)
                {
                    ops.releaseParcelFromRider(act.getParcelID(), p.getWeight(), act.getRiderID());
                    p.setAssignedRiderID(-1);
                    p.setStatus(act.getPrevStatus());
                    pm.updateParcel(p);
                    cout << "Parcel " << act.getParcelID() << " unassigned from rider.\n";
                }
                break;
            }
            case ACTION_DELIVERY_ATTEMPT:
            {
                Parcel p = pm.getParcel(act.getParcelID());
                if (p.getParcelID() != 0)
                {
                    p.setDeliveryAttempts(act.getAttemptCount());
                    p.setStatus(act.getPrevStatus());
                    pm.updateParcel(p);
                    cout << "Delivery attempt reversed for parcel " << act.getParcelID() << ".\n";
                }
                break;
            }
            case ACTION_MARK_DELIVERED:
            case ACTION_RETURN_TO_SENDER:
            {
                Parcel restored = act.reconstructParcel();
                pm.updateParcel(restored);
                cout << "Parcel " << act.getParcelID() << " status restored to: " << act.getPrevStatus() << "\n";
                break;
            }
            default:
                cout << "Unknown action type, cannot undo.\n";
            }

            pm.saveParcels("parcels.txt");
            break;
        }

        case 18:
        { // System Status
            cout << "\n+================== SYSTEM STATUS ==================+\n";
            cout << "| Cities in Network:        " << cityMap.getCityCount() << "\n";
            cout << "| Zones Defined:            " << cityMap.getAllZones().getSize() << "\n";
            cout << "| Parcels in Priority Queue: " << (pm.hasParcels() ? "Yes" : "Empty") << "\n";
            cout << "|                                                    \n";
            cout << "| QUEUE STATUS:                                      \n";
            cout << "|   Pickup Queue:           " << ops.getPickupQueueSize() << " parcels\n";
            cout << "|   Warehouse Queue:        " << ops.getWarehouseSize() << " parcels\n";
            cout << "|   Transit Queue:          " << ops.getTransitQueueSize() << " parcels\n";
            cout << "|                                                    \n";
            cout << "| RIDERS:                                            \n";
            cout << "|   Total Riders:           " << ops.getRiderCount() << "\n";
            cout << "|                                                    \n";
            cout << "| ALERTS:                                            \n";
            cout << "|   Missing Parcels:        " << ops.getMissingCount() << "\n";
            cout << "|   Audit Log Actions:      " << ops.getAuditLogSize() << "\n";
            cout << "|                                                    \n";
            cout << "| Warehouse Location:       " << warehouseCity << "\n";
            cout << "+===================================================+\n";
            break;
        }

        case 19:
        { // View Cities/Routes
            cout << "\n-- CITY NETWORK --\n";
            CustomVector<string> allCities = cityMap.getAllCities();
            cout << "Cities (" << allCities.getSize() << "): ";
            for (int i = 0; i < allCities.getSize(); i++)
            {
                cout << allCities[i];
                string zone = cityMap.getCityZone(allCities[i]);
                if (zone != "Unassigned")
                {
                    cout << "(" << zone << ")";
                }
                if (i < allCities.getSize() - 1)
                    cout << ", ";
            }
            cout << "\n\nUse the map_data.txt file to see all routes.\n";
            break;
        }

        case 0:
        { // Save & Exit
            cout << "\n-- SAVING DATA --\n";
            pm.saveParcels("parcels.txt");
            cityMap.saveGraph("map_data.txt");
            cout << "\nThank you for using the Courier Logistics Engine!\n";
            break;
        }

        default:
            cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);
}

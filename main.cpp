#include <iostream>
#include <limits>
#include <conio.h>  
#include "CourierSystem.h"
#include "Utils.h"

using namespace std;


void initializeConsole() {
    enableUTF8();
    
#ifdef _WIN32
    SetConsoleTitleA("SwiftEx Courier System");
#endif
}


string getMaskedPassword() {
    string password = "";
    char ch;
    
    while(true) {
        ch = _getch(); 
        
        if(ch == '\r' || ch == '\n') { 
            break;
        } else if(ch == '\b' || ch == 127) { 
            if(password.length() > 0) {
                password.pop_back();
cout << "\b \b"; 
            }
        } else {
            password += ch;
cout << "*"; 
        }
    }
    
    return password;
}


bool authenticate(CourierSystem& system) {
    clearScreen();
    
    cout << BRIGHT_CYAN BOLD;
    cout << "\n┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    cout <<   "┃     SwiftEx Courier System Login   ┃\n";
    cout <<   "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n" << RESET;
    
    int attempts = 0;
    const int maxAttempts = 3;
    
    while(attempts < maxAttempts) {
        string username, password;
        
        cout << BRIGHT_GREEN << "\nUsername: " << RESET;
        cin >> username;
        
        cout << BRIGHT_GREEN << "Password: " << RESET;
        password = getMaskedPassword();
        cout << "\n";
        
        if(system.authenticateAdmin(username, password)) {
            cout << BRIGHT_GREEN BOLD << "\n[SUCCESS] Authentication successful!\n" << RESET;
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
            return true;
        } else {
            attempts++;
            cout << BRIGHT_RED << "\n[ERROR] Invalid username or password. Attempts remaining: " 
                 << (maxAttempts - attempts) << RESET << "\n";
            if(attempts >= maxAttempts) {
                cout << BRIGHT_RED BOLD << "\n[ERROR] Maximum login attempts reached. Access denied.\n" << RESET;
                return false;
            }
        }
    }
    
    return false;
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printMainMenu() {
    Table t;
    t.addHeader("No.");
    t.addHeader("Main Menu");
    
    t.addRow({"1", "Parcel Management"});
    t.addRow({"2", "Routing & Navigation"});
    t.addRow({"3", "Operations"});
    t.addRow({"4", "Rider Management"});
    t.addRow({"5", "System & Reports"});

    t.addRow({"0", "Exit & Save"});
    
    cout << "\n┏━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    cout <<   "┃  SwiftEx Courier Engine   ┃\n";
    cout <<   "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    t.print();
cout << "Select Option: ";
}

void printParcelMenu() {
    clearScreen();
    Table t;
    t.setHeaderColor(BRIGHT_MAGENTA BOLD);
    t.setBorderColor(MAGENTA);
    t.setRowColor(WHITE);
    
    t.addHeader("No.");
    t.addHeader("Parcel Management");
    
    t.addRow({"1", "Add New Parcel"});
    t.addRow({"2", "Remove/Withdraw Parcel"});
    t.addRow({"3", "Track Parcel"});
    t.addRow({"4", "View All Parcels"});
    t.addRow({"5", "View Parcels by Weight"});
    t.addRow({"6", "View Parcels by Destination"});
    t.addRow({"7", "View Parcels by Priority"});
    t.addRow({"8", "Mark Parcel as Fragile"});
    t.addRow({"0", "Back to Main Menu"});
    
    t.print();
    cout << BRIGHT_YELLOW << "\nSelect Option: " << RESET;
}

void printRoutingMenu() {
    clearScreen();
    Table t;
    t.setHeaderColor(BRIGHT_BLUE BOLD);
    t.setBorderColor(CYAN);
    t.setRowColor(WHITE);
    
    t.addHeader("No.");
    t.addHeader("Routing & Navigation");
    
    t.addRow({"1", "View All Cities"});
    t.addRow({"2", "Find Shortest Route"});
    t.addRow({"3", "View Alternative Routes"});
    t.addRow({"4", "View All Routes"});
    t.addRow({"5", "Block Route"});
    t.addRow({"6", "Unblock Route"});
    t.addRow({"7", "View Blocked Routes"});
    t.addRow({"8", "Mark Path as Overloaded"});
    t.addRow({"9", "Unmark Path as Overloaded"});
    t.addRow({"10", "View Overloaded Routes"});
    t.addRow({"11", "Add New City"});
    t.addRow({"12", "Add New Route"});
    t.addRow({"0", "Back to Main Menu"});
    
    t.print();
    cout << BRIGHT_YELLOW << "\nSelect Option: " << RESET;
}

void printOperationsMenu() {
    clearScreen();
    Table t;
    t.setHeaderColor(BRIGHT_GREEN BOLD);
    t.setBorderColor(GREEN);
    t.setRowColor(WHITE);
    
    t.addHeader("No.");
    t.addHeader("Operations");
    
    t.addRow({"1", "Process Next Order (Pickup -> Warehouse)"});
    t.addRow({"2", "Assign Rider (Warehouse -> Transit)"});
    t.addRow({"3", "Complete Delivery (Transit -> Delivered)"});
    t.addRow({"4", "View Queue Status"});
    t.addRow({"5", "Detect Missing Parcels"});
    t.addRow({"6", "Record Dispatch"});
    t.addRow({"7", "Record Loading"});
    t.addRow({"8", "Record Unloading"});
    t.addRow({"9", "Record Delivery Attempt"});
    t.addRow({"10", "Return to Sender"});
    t.addRow({"0", "Back to Main Menu"});
    
    t.print();
    cout << BRIGHT_YELLOW << "\nSelect Option: " << RESET;
}

void printRiderMenu() {
    clearScreen();
    Table t;
    t.setHeaderColor(BRIGHT_YELLOW BOLD);
    t.setBorderColor(YELLOW);
    t.setRowColor(WHITE);
    
    t.addHeader("No.");
    t.addHeader("Rider Management");
    
    t.addRow({"1", "Add New Rider"});
    t.addRow({"2", "View All Riders"});
    t.addRow({"3", "View Rider Load"});
    t.addRow({"4", "View All Rider Loads"});
    t.addRow({"0", "Back to Main Menu"});
    
    t.print();
    cout << BRIGHT_YELLOW << "\nSelect Option: " << RESET;
}

void printSystemMenu() {
    clearScreen();
    Table t;
    t.setHeaderColor(BRIGHT_CYAN BOLD);
    t.setBorderColor(CYAN);
    t.setRowColor(WHITE);
    
    t.addHeader("No.");
    t.addHeader("System & Reports");
    
    t.addRow({"1", "View Operation Log"});
    t.addRow({"2", "Save Operation Log to File"});
    t.addRow({"3", "Load & Replay Operation Log"});
    t.addRow({"4", "Undo Last Operation"});
    t.addRow({"5", "View Delivery Statistics"});
    t.addRow({"6", "View Rider Statistics"});
    t.addRow({"7", "Save All Data"});
    t.addRow({"0", "Back to Main Menu"});
    
    t.print();
    cout << BRIGHT_YELLOW << "\nSelect Option: " << RESET;
}


 

void handleParcelMenu(CourierSystem& system) {
    int choice;
    while (true) {
        printParcelMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) break;
        
        switch(choice) {
            case 1: { 
string sender, receiver;
                int priorityInt, srcID, destID, weight;
                
cout << "Enter Sender Name: ";
cin >> sender;
cout << "Enter Receiver Name: ";
cin >> receiver;
                
cout << "Select Priority (1: Overnight, 2: Two-Day, 3: Normal): ";
cin >> priorityInt;
                Priority p = (priorityInt == 1) ? Priority::Overnight : 
                             (priorityInt == 2) ? Priority::TwoDay : Priority::Normal;
                             
cout << "Enter Weight (kg): ";
cin >> weight;
                
                system.displayCities();
cout << "Select Origin City ID: ";
cin >> srcID;
cout << "Select Destination City ID: ";
cin >> destID;
                
                system.addParcel(sender, receiver, p, weight, srcID, destID);
                break;
            }
            case 2: { 
                int id;
cout << "Enter Tracking ID to withdraw: ";
cin >> id;
                system.removeParcel(id);
                break;
            }
            case 3: { 
                int id;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
                system.trackParcel(id);
                break;
            }
            case 4:
                system.displayAllParcels();
                break;
            case 5:
                system.displayParcelsByWeight();
                break;
            case 6:
                system.displayParcelsByDestination();
                break;
            case 7:
                system.displayParcelsByPriority();
                break;
            case 8: { 
                int id;
cout << "Enter Tracking ID to mark as fragile: ";
cin >> id;
                system.markParcelAsFragile(id);
                break;
            }
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        }
        
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
    }
}

void handleRoutingMenu(CourierSystem& system) {
    int choice;
    while (true) {
        printRoutingMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) break;
        
        switch(choice) {
            case 1:
                system.displayCities();
                break;
            case 2: { 
                int s, d;
                system.displayCities();
cout << "Enter Source ID: ";
cin >> s;
cout << "Enter Destination ID: ";
cin >> d;
                system.displayRoute(s, d);
                break;
            }
            case 3: { 
                int s, d, k;
                system.displayCities();
cout << "Enter Source ID: ";
cin >> s;
cout << "Enter Destination ID: ";
cin >> d;
cout << "Enter number of alternative routes to find (max 5): ";
cin >> k;
                if (k > 5) k = 5;
                if (k < 1) k = 1;
                system.displayAlternativeRoutes(s, d, k);
                break;
            }
            case 4: { 
                int s, d;
                system.displayCities();
cout << "Enter Source ID: ";
cin >> s;
cout << "Enter Destination ID: ";
cin >> d;
                system.viewAllRoutes(s, d);
                break;
            }
            case 5: { 
                int s, d;
                system.displayCities();
cout << "Enter Source City ID: ";
cin >> s;
cout << "Enter Destination City ID: ";
cin >> d;
                system.blockRoute(s, d);
                break;
            }
            case 6: { 
                int s, d;
                system.displayCities();
cout << "Enter Source City ID: ";
cin >> s;
cout << "Enter Destination City ID: ";
cin >> d;
                system.unblockRoute(s, d);
                break;
            }
            case 7:
                system.displayBlockedRoutes();
                break;
            case 8: { 
                int s, d, load;
                system.displayCities();
cout << "Enter Source City ID: ";
cin >> s;
cout << "Enter Destination City ID: ";
cin >> d;
cout << "Enter Current Load: ";
cin >> load;
                system.markPathAsOverloaded(s, d, load);
                break;
            }
            case 9: { 
                int s, d;
                system.displayCities();
cout << "Enter Source City ID: ";
cin >> s;
cout << "Enter Destination City ID: ";
cin >> d;
                system.unmarkPathAsOverloaded(s, d);
                break;
            }
            case 10:
                system.displayOverloadedRoutes();
                break;
            case 11: { 
string name;
cout << "Enter New City Name: ";
cin >> name;
                system.addCity(name);
                break;
            }
            case 12: { 
                int s, d, dist;
                system.displayCities();
cout << "Enter Source ID: ";
cin >> s;
cout << "Enter Destination ID: ";
cin >> d;
cout << "Enter Distance (km): ";
cin >> dist;
                system.addRoute(s, d, dist);
                break;
            }
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        }
        
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
    }
}

void handleOperationsMenu(CourierSystem& system) {
    int choice;
    while (true) {
        printOperationsMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) break;
        
        switch(choice) {
            case 1:
                system.processNextParcel();
                break;
            case 2:
                system.assignRider();
                break;
            case 3:
                system.completeDelivery();
                break;
            case 4:
                system.displayQueueStatus();
                break;
            case 5:
                system.detectMissingParcels();
                break;
            case 6: { 
                int id;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
                system.recordDispatch(id);
                break;
            }
            case 7: { 
                int id, cityID;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
                system.displayCities();
cout << "Enter City ID where loading: ";
cin >> cityID;
                system.recordLoading(id, cityID);
                break;
            }
            case 8: { 
                int id, cityID;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
                system.displayCities();
cout << "Enter City ID where unloading: ";
cin >> cityID;
                system.recordUnloading(id, cityID);
                break;
            }
            case 9: { 
                int id;
                int success;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
cout << "Was delivery successful? (1=Yes, 0=No): ";
cin >> success;
                system.recordDeliveryAttempt(id, success == 1);
                break;
            }
            case 10: { 
                int id;
                cout << BRIGHT_CYAN << "Enter Tracking ID: " << RESET;
cin >> id;
                system.returnToSender(id);
                break;
            }
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        }
        
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
    }
}

void handleRiderMenu(CourierSystem& system) {
    int choice;
    while (true) {
        printRiderMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) break;
        
        switch(choice) {
            case 1: { 
string name;
                int capacity, cityID;
cout << "Enter Rider Name: ";
cin >> name;
cout << "Enter Capacity (kg): ";
cin >> capacity;
                system.displayCities();
cout << "Enter Current City ID: ";
cin >> cityID;
                system.addRider(name, capacity, cityID);
                break;
            }
            case 2:
                system.displayRiders();
                break;
            case 3: { 
                int riderID;
                system.displayRiders();
cout << "Enter Rider ID: ";
cin >> riderID;
                system.displayRiderLoad(riderID);
                break;
            }
            case 4:
                system.displayAllRiderLoads();
                break;
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        }
        
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
    }
}

void handleSystemMenu(CourierSystem& system) {
    int choice;
    while (true) {
        printSystemMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) break;
        
        switch(choice) {
            case 1:
                system.displayOperationLog();
                break;
            case 2: { 
string filename;
cout << "Enter filename (e.g., operations.log): ";
cin >> filename;
                system.saveOperationLog(filename);
                break;
            }
            case 3: { 
string filename;
cout << "Enter filename to replay (e.g., operations.log): ";
cin >> filename;
                system.loadOperationLog(filename);
                break;
            }
            case 4:
                system.undoLastOperation();
                break;
            case 5:
                system.displayDeliveryStatistics();
                break;
            case 6: { 
                int riderID;
                system.displayRiders();
cout << "Enter Rider ID: ";
cin >> riderID;
                system.displayRiderStatistics(riderID);
                break;
            }
            case 7:
                system.saveData();
                break;
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        }
        
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
    }
}


 

int main() {
    
    initializeConsole();
    
    CourierSystem system;
    
    clearScreen();
    int titleWidth = 40;
    int totalWidth = titleWidth + 2;
    
    cout << BRIGHT_CYAN BOLD;
    cout << "\n";
    cout << "┏";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┓\n";
    cout << "┃";
    int titleLen = 37; 
    int padding = (totalWidth - titleLen) / 2;
    for(int i = 0; i < padding; i++) cout << " ";
    cout << "     Loading Data from Files...     ";
    for(int i = 0; i < padding - 1; i++) cout << " ";
    cout << "┃\n";
    cout << "┗";
    for(int i = 0; i < totalWidth - 2; i++) cout << "━";
    cout << "┛\n" << RESET;
    
    
    system.loadData();
    cout << BRIGHT_GREEN << "[SUCCESS] Data loaded successfully!\n" << RESET;
    
    
    if(!authenticate(system)) {
        cout << BRIGHT_RED BOLD << "\n[ERROR] Access denied. Exiting program...\n" << RESET;
        return 1;
    }
    
    int choice;
    while (true) {
        clearScreen();
        printMainMenu();
        if (!(cin >> choice)) {
            clearInput();
            continue;
        }
        
        if (choice == 0) {
            clearScreen();
            int titleWidth = 40;
            int totalWidth = titleWidth + 2;
            
            cout << BRIGHT_CYAN BOLD;
            cout << "\n";
            cout << "┏";
            for(int i = 0; i < totalWidth - 2; i++) cout << "━";
            cout << "┓\n";
            cout << "┃";
            int titleLen = 37; 
            int padding = (totalWidth - titleLen) / 2;
            for(int i = 0; i < padding; i++) cout << " ";
            cout << "        Saving All Data...           ";
            for(int i = 0; i < padding - 1; i++) cout << " ";
            cout << "┃\n";
            cout << "┗";
            for(int i = 0; i < totalWidth - 2; i++) cout << "━";
            cout << "┛\n" << RESET;
            system.saveData();
            
            cout << BRIGHT_GREEN BOLD;
            cout << "\n";
            cout << "┏";
            for(int i = 0; i < totalWidth - 2; i++) cout << "━";
            cout << "┓\n";
            cout << "┃";
            titleLen = 39; 
            padding = (totalWidth - titleLen) / 2;
            for(int i = 0; i < padding; i++) cout << " ";
            cout << "  Thank you for using SwiftEx        ";
            for(int i = 0; i < padding - 1; i++) cout << " ";
            cout << "┃\n";
            cout << "┃";
            titleLen = 39; 
            padding = (totalWidth - titleLen) / 2;
            for(int i = 0; i < padding; i++) cout << " ";
            cout << "     Intelligent Courier System!      ";
            for(int i = 0; i < padding - 1; i++) cout << " ";
            cout << "┃\n";
            cout << "┗";
            for(int i = 0; i < totalWidth - 2; i++) cout << "━";
            cout << "┛\n" << RESET;
            break;
        }
        
        switch(choice) {
            case 1:
                handleParcelMenu(system);
                break;
            case 2:
                handleRoutingMenu(system);
                break;
            case 3:
                handleOperationsMenu(system);
                break;
            case 4:
                handleRiderMenu(system);
                break;
            case 5:
                handleSystemMenu(system);
                break;
            default:
        cout << BRIGHT_RED << "[ERROR] Invalid Option!" << RESET << "\n";
        cout << BRIGHT_YELLOW << "\nPress Enter to continue..." << RESET;
        cin.ignore();
        cin.get();
        clearScreen();
        }
    }
    
    return 0;
}

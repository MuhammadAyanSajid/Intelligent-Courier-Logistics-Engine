#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <cstdlib>
#include <windows.h>  
#include "DataStructures.h"
#include "Models.h"

using namespace std;


#ifdef _WIN32
    #define RESET   "\033[0m"
    #define BLACK   "\033[30m"
    #define RED     "\033[31m"
    #define GREEN   "\033[32m"
    #define YELLOW  "\033[33m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN    "\033[36m"
    #define WHITE   "\033[37m"
    #define BOLD    "\033[1m"
    #define BRIGHT_RED     "\033[91m"
    #define BRIGHT_GREEN   "\033[92m"
    #define BRIGHT_YELLOW  "\033[93m"
    #define BRIGHT_BLUE    "\033[94m"
    #define BRIGHT_MAGENTA "\033[95m"
    #define BRIGHT_CYAN    "\033[96m"
#else
    #define RESET   "\033[0m"
    #define BLACK   "\033[30m"
    #define RED     "\033[31m"
    #define GREEN   "\033[32m"
    #define YELLOW  "\033[33m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN    "\033[36m"
    #define WHITE   "\033[37m"
    #define BOLD    "\033[1m"
    #define BRIGHT_RED     "\033[91m"
    #define BRIGHT_GREEN   "\033[92m"
    #define BRIGHT_YELLOW  "\033[93m"
    #define BRIGHT_BLUE    "\033[94m"
    #define BRIGHT_MAGENTA "\033[95m"
    #define BRIGHT_CYAN    "\033[96m"
#endif


inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


inline void enableUTF8() {
#ifdef _WIN32
    
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    wcscpy(cfi.FaceName, L"Consolas");
    cfi.dwFontSize.Y = 16;
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
#endif
}




class Table {
private:
    vector<string> headers;
    vector<vector<string>> rows;
    vector<int> columnWidths;
    string headerColor;
    string borderColor;
    string rowColor;
    
    
    const string TOP_LEFT = "┏";
    const string TOP_RIGHT = "┓";
    const string BOTTOM_LEFT = "┗";
    const string BOTTOM_RIGHT = "┛";
    const string HORIZONTAL = "━";
    const string VERTICAL = "┃";
    const string TOP_T = "┳";
    const string BOTTOM_T = "┻";
    const string LEFT_T = "┣";
    const string RIGHT_T = "┫";
    const string CROSS = "╋";

public:
    Table() : headerColor(CYAN BOLD), borderColor(BRIGHT_CYAN), rowColor(WHITE) {}
    
    void setHeaderColor(const string& color) { headerColor = color; }
    void setBorderColor(const string& color) { borderColor = color; }
    void setRowColor(const string& color) { rowColor = color; }
    
    void addHeader(const string& header) {
        headers.push_back(header);
        
        columnWidths.push_back(calculateWidth(header));
    }

    void addRow(const vector<string>& row) {
        if (row.size() != headers.size()) return;
        rows.push_back(row);
        for (size_t i = 0; i < row.size(); ++i) {
            int width = calculateWidth(row[i]);
            if (width > columnWidths[i]) {
                columnWidths[i] = width;
            }
        }
    }
    
    
    
    
    int calculateWidth(const string& str) {
        int width = 0;
        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];
            if ((c & 0x80) == 0) {
                
                width++;
                i++;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < str.length()) {
                
                unsigned int code = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F);
                if (code >= 0x2500 && code <= 0x257F) {
                    
                    width += 1;
                } else {
                    
                    width += 2;
                }
                i += 3;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < str.length()) {
                
                width += 1;
                i += 2;
            } else if ((c & 0xF8) == 0xF0 && i + 3 < str.length()) {
                
                width += 2;
                i += 4;
            } else {
                
                width++; 
                i++;
            }
        }
        return width;
    }

    
    string repeatString(const string& str, int count) {
        string result;
        for (int i = 0; i < count; ++i) {
            result += str;
        }
        return result;
    }

    void print() {
        if (headers.empty()) return;
        
        
        cout << borderColor << TOP_LEFT;
        for (size_t i = 0; i < headers.size(); ++i) {
            cout << repeatString(HORIZONTAL, columnWidths[i] + 2);
            if (i < headers.size() - 1) cout << TOP_T;
        }
        cout << TOP_RIGHT << RESET << "\n";

        
        cout << borderColor << VERTICAL << RESET;
        for (size_t i = 0; i < headers.size(); ++i) {
            cout << " " << headerColor << left << setw(columnWidths[i]) << headers[i] 
                 << borderColor << " " << VERTICAL << RESET;
        }
        cout << "\n";

        
        cout << borderColor << LEFT_T;
        for (size_t i = 0; i < headers.size(); ++i) {
            cout << repeatString(HORIZONTAL, columnWidths[i] + 2);
            if (i < headers.size() - 1) cout << CROSS;
        }
        cout << RIGHT_T << RESET << "\n";

        
        for (const auto& row : rows) {
            cout << borderColor << VERTICAL << RESET;
            for (size_t i = 0; i < row.size(); ++i) {
                cout << " " << rowColor << left << setw(columnWidths[i]) << row[i] 
                     << borderColor << " " << VERTICAL << RESET;
            }
            cout << "\n";
        }

        
        cout << borderColor << BOTTOM_LEFT;
        for (size_t i = 0; i < headers.size(); ++i) {
            cout << repeatString(HORIZONTAL, columnWidths[i] + 2);
            if (i < headers.size() - 1) cout << BOTTOM_T;
        }
        cout << BOTTOM_RIGHT << RESET << "\n";
    }
};




namespace CSVUtils {

    inline vector<string> split(const string& s, char delimiter) {
vector<string> tokens;
string token;
istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    inline void loadCities(const string& filename, Graph<City>& graph) {
ifstream file(filename);
        if (!file.is_open()) return;

string line;
getline(file, line); 

        while (getline(file, line)) {
            auto tokens = split(line, ',');
            if (tokens.size() >= 2) {
                int id = stoi(tokens[0]);
string name = tokens[1];
                graph.addNode(id, City(id, name));
            }
        }
        file.close();
    }

    inline void loadRoutes(const string& filename, Graph<City>& graph) {
ifstream file(filename);
        if (!file.is_open()) return;

string line;
getline(file, line); 

        while (getline(file, line)) {
            auto tokens = split(line, ',');
            if (tokens.size() >= 3) {
                int src = stoi(tokens[0]);
                int dest = stoi(tokens[1]);
                int dist = stoi(tokens[2]);
                graph.addEdge(src, dest, dist);
                
                
                graph.addEdge(dest, src, dist); 
            }
        }
        file.close();
    }
    
    inline void saveCity(const string& filename, int id, const string& name) {
ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << id << "," << name << "\n";
            file.close();
        }
    }
    
    inline void saveRoute(const string& filename, int src, int dest, int dist) {
ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << src << "," << dest << "," << dist << "\n";
            file.close();
        }
    }
    
    
    inline void loadRiders(const string& filename, Vector<Rider>& riders, int& nextRiderID) {
ifstream file(filename);
        if (!file.is_open()) {
            
ofstream createFile(filename);
            if (createFile.is_open()) {
                createFile << "RiderID,Name,Capacity,CurrentCityID\n";
                createFile.close();
            }
            return;
        }
        
string line;
        if (!getline(file, line)) { 
            file.close();
            return;
        }
        
        int maxID = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;
            auto tokens = split(line, ',');
            if (tokens.size() >= 4) {
                int id = stoi(tokens[0]);
string name = tokens[1];
                int capacity = stoi(tokens[2]);
                int cityID = stoi(tokens[3]);
                riders.push_back(Rider(id, name, capacity, cityID));
                if (id > maxID) maxID = id;
            }
        }
        nextRiderID = maxID + 1;
        file.close();
    }
    
    
    inline void saveAllRiders(const string& filename, const Vector<Rider>& riders) {
ofstream file(filename);
        if (!file.is_open()) return;
        
        file << "RiderID,Name,Capacity,CurrentCityID\n";
        for (int i = 0; i < riders.size(); i++) {
            file << riders[i].riderID << "," << riders[i].name << ","
                 << riders[i].capacity << "," << riders[i].currentCityID << "\n";
        }
        file.close();
    }
    
    
    inline void saveRider(const string& filename, int id, const string& name, int capacity, int cityID) {
ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << id << "," << name << "," << capacity << "," << cityID << "\n";
            file.close();
        }
    }
    
    
    inline void loadParcels(const string& filename, Vector<Parcel>& parcels, HashTable<int, Parcel*>& parcelMap, int& nextTrackingID) {
ifstream file(filename);
        if (!file.is_open()) {
            
ofstream createFile(filename);
            if (createFile.is_open()) {
                createFile << "TrackingID,Sender,Receiver,Priority,Weight,Status,SourceCityID,DestCityID,DeliveryAttempts,History,IsFragile,CurrentCityID\n";
                createFile.close();
            }
            return;
        }
        
string line;
        if (!getline(file, line)) { 
            file.close();
            return;
        }
        
        int maxID = 1000;
        while (getline(file, line)) {
            if (line.empty()) continue;
            auto tokens = split(line, ','); 
            if (tokens.size() >= 8) {
                int trackingID = stoi(tokens[0]);
string sender = tokens[1];
string receiver = tokens[2];
                Priority priority = static_cast<Priority>(stoi(tokens[3]));
                int weight = stoi(tokens[4]);
                Status status = static_cast<Status>(stoi(tokens[5]));
                int srcID = stoi(tokens[6]);
                int destID = stoi(tokens[7]);
                int attempts = tokens.size() > 8 ? stoi(tokens[8]) : 0;
                bool fragile = tokens.size() > 10 ? (stoi(tokens[10]) != 0) : false;
                int currentCity = tokens.size() > 11 ? stoi(tokens[11]) : srcID;
                
                Parcel p(trackingID, sender, receiver, priority, weight, srcID, destID);
                p.status = status;
                p.deliveryAttempts = attempts;
                p.isFragile = fragile;
                p.currentCityID = currentCity;
                
                
                if (tokens.size() > 9 && !tokens[9].empty()) {
string historyStr = tokens[9];
istringstream histStream(historyStr);
string event;
                    while (getline(histStream, event, '|')) {
                        if (!event.empty()) {
                            p.history.push_back(event);
                        }
                    }
                }
                
                parcels.push_back(p);
                parcelMap.insert(trackingID, &parcels[parcels.size() - 1]);
                if (trackingID > maxID) maxID = trackingID;
            }
        }
        nextTrackingID = maxID + 1;
        file.close();
    }
    
    
    inline void saveAllParcels(const string& filename, const Vector<Parcel>& parcels) {
ofstream file(filename);
        if (!file.is_open()) return;
        
        file << "TrackingID,Sender,Receiver,Priority,Weight,Status,SourceCityID,DestCityID,DeliveryAttempts,History,IsFragile,CurrentCityID\n";
        for (int i = 0; i < parcels.size(); i++) {
            const Parcel& p = parcels[i];
            file << p.trackingID << "," << p.senderName << "," << p.receiverName << ","
                 << static_cast<int>(p.priority) << "," << p.weight << ","
                 << static_cast<int>(p.status) << "," << p.sourceCityID << ","
                 << p.destCityID << "," << p.deliveryAttempts << ",";
            
            
            bool first = true;
            for (auto& event : p.history) {
                if (!first) file << "|";
                file << event;
                first = false;
            }
            file << "," << (p.isFragile ? 1 : 0) << "," << p.currentCityID << "\n";
        }
        file.close();
    }
    
    
    inline void saveParcel(const string& filename, const Parcel& parcel) {
ifstream checkFile(filename);
        bool fileExists = checkFile.is_open();
        bool hasHeader = false;
        if (fileExists) {
string firstLine;
            if (getline(checkFile, firstLine)) {
                hasHeader = (firstLine.find("TrackingID") != string::npos);
            }
        }
        checkFile.close();
        
ofstream file(filename, ios::app);
        if (file.is_open()) {
            if (!hasHeader) {
                file << "TrackingID,Sender,Receiver,Priority,Weight,Status,SourceCityID,DestCityID,DeliveryAttempts,History,IsFragile,CurrentCityID\n";
            }
            file << parcel.trackingID << "," << parcel.senderName << "," << parcel.receiverName << ","
                 << static_cast<int>(parcel.priority) << "," << parcel.weight << ","
                 << static_cast<int>(parcel.status) << "," << parcel.sourceCityID << ","
                 << parcel.destCityID << "," << parcel.deliveryAttempts << ",";
            
            bool first = true;
            for (auto& event : parcel.history) {
                if (!first) file << "|";
                file << event;
                first = false;
            }
            file << "," << (parcel.isFragile ? 1 : 0) << "," << parcel.currentCityID << "\n";
            file.close();
        }
    }
    
    
    inline void loadAdmins(const string& filename, Vector<Admin>& admins) {
ifstream file(filename);
        if (!file.is_open()) {
            
            return;
        }
        
string line;
        if (!getline(file, line)) { 
            file.close();
            return;
        }
        
        while (getline(file, line)) {
            if (line.empty()) continue;
            auto tokens = split(line, ',');
            if (tokens.size() >= 3) {
string username = tokens[0];
string password = tokens[1];
                bool isSuperAdmin = (stoi(tokens[2]) != 0);
                admins.push_back(Admin(username, password, isSuperAdmin));
            }
        }
        file.close();
    }
    
    
    inline void saveAllAdmins(const string& filename, const Vector<Admin>& admins) {
ofstream file(filename);
        if (!file.is_open()) return;
        
        file << "Username,Password,IsSuperAdmin\n";
        for (int i = 0; i < admins.size(); i++) {
            file << admins[i].username << "," << admins[i].password << ","
                 << (admins[i].isSuperAdmin ? 1 : 0) << "\n";
        }
        file.close();
    }
}

#endif

#ifndef MODELS_H
#define MODELS_H

#include <string>
#include "DataStructures.h"

using namespace std;

enum class Priority {
    Overnight = 1,
    TwoDay = 2,
    Normal = 3
};

enum class Status {
    Pending,
    PickedUp,
    InWarehouse,
    InTransit,
    Delivered,
    Returned,
    Missing
};

enum class WeightCategory {
    Light = 1,    
    Medium = 2,   
    Heavy = 3     
};

struct City {
    int id;
string name;
    
    City() : id(-1), name("") {}
    City(int i, string n) : id(i), name(n) {}
};

struct Parcel {
    int trackingID;
string senderName;
string receiverName;
    Priority priority;
    int weight; 
    Status status;
    int sourceCityID;
    int destCityID;
    int currentCityID; 
    Vector<int> currentRoute; 
    LinkedList<string> history;
    int deliveryAttempts; 
    bool isFragile; 

    Parcel() : trackingID(-1), weight(0), status(Status::Pending), sourceCityID(-1), destCityID(-1),
               currentCityID(-1), deliveryAttempts(0), isFragile(false) {}
    
    Parcel(int id, string s, string r, Priority p, int w, int src, int dest)
        : trackingID(id), senderName(s), receiverName(r), priority(p), weight(w), 
          status(Status::Pending), sourceCityID(src), destCityID(dest), currentCityID(src),
          deliveryAttempts(0), isFragile(false) {
        history.push_back("Parcel Created");
    }
    
    
    bool operator==(const Parcel& other) const {
        return trackingID == other.trackingID;
    }

    
    
    WeightCategory getWeightCategory() const {
        if (weight < 5) return WeightCategory::Light;
        if (weight < 20) return WeightCategory::Medium;
        return WeightCategory::Heavy;
    }
    
string getWeightCategoryStr() const {
        switch(getWeightCategory()) {
            case WeightCategory::Light: return "Light (0-5kg)";
            case WeightCategory::Medium: return "Medium (5-20kg)";
            case WeightCategory::Heavy: return "Heavy (20+kg)";
            default: return "Unknown";
        }
    }

    
    
    
    bool operator<(const Parcel& other) const {
        
        if (priority != other.priority) {
            return priority < other.priority;
        }
        
        
        if (isFragile != other.isFragile) {
            return !isFragile; 
        }
        
        WeightCategory thisCat = getWeightCategory();
        WeightCategory otherCat = other.getWeightCategory();
        if (thisCat != otherCat) {
            return thisCat < otherCat; 
        }
        
        if (destCityID != other.destCityID) {
            return destCityID < other.destCityID;
        }
        
        return trackingID < other.trackingID;
    }
    
    bool operator>(const Parcel& other) const {
        return other < *this;
    }
    
string getPriorityStr() const {
        switch(priority) {
            case Priority::Overnight: return "Overnight";
            case Priority::TwoDay: return "Two-Day";
            case Priority::Normal: return "Normal";
            default: return "Unknown";
        }
    }
    
string getStatusStr() const {
        switch(status) {
            case Status::Pending: return "Pending";
            case Status::PickedUp: return "Picked Up";
            case Status::InWarehouse: return "In Warehouse";
            case Status::InTransit: return "In Transit";
            case Status::Delivered: return "Delivered";
            case Status::Returned: return "Returned";
            case Status::Missing: return "Missing";
            default: return "Unknown";
        }
    }
};

struct Rider {
    int riderID;
string name;
    int capacity; 
    int currentLoad; 
    int currentCityID;
    Vector<int> assignedParcels; 
    
    Rider() : riderID(-1), capacity(0), currentLoad(0), currentCityID(-1) {}
    Rider(int id, string n, int cap, int city) 
        : riderID(id), name(n), capacity(cap), currentLoad(0), currentCityID(city) {}
    
    
    bool canCarry(int weight) const {
        return (currentLoad + weight) <= capacity;
    }
    
    
    int getAvailableCapacity() const {
        return capacity - currentLoad;
    }
};


enum class OperationType {
    AddParcel,
    RemoveParcel,
    ProcessParcel,
    AssignRider,
    CompleteDelivery,
    BlockRoute,
    UnblockRoute,
    ReturnToSender
};

struct OperationLog {
    OperationType type;
string timestamp;
    int parcelID;
string previousState;
string newState;
    int riderID; 
    int srcID, destID; 
    
    OperationLog() : type(OperationType::AddParcel), parcelID(-1), riderID(-1), srcID(-1), destID(-1) {}
};


struct BlockedEdge {
    int srcID;
    int destID;
    bool isBlocked;
    
    BlockedEdge() : srcID(-1), destID(-1), isBlocked(false) {}
    BlockedEdge(int s, int d, bool blocked) : srcID(s), destID(d), isBlocked(blocked) {}
    
    bool operator==(const BlockedEdge& other) const {
        return (srcID == other.srcID && destID == other.destID) ||
               (srcID == other.destID && destID == other.srcID); 
    }
};


struct Admin {
    string username;
    string password;
    bool isSuperAdmin;
    
    Admin() : username(""), password(""), isSuperAdmin(false) {}
    Admin(string user, string pass, bool superAdmin = false) 
        : username(user), password(pass), isSuperAdmin(superAdmin) {}
    
    bool operator==(const Admin& other) const {
        return username == other.username;
    }
};

#endif

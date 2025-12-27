#ifndef PARCEL_H
#define PARCEL_H

#include <string>
#include "structures.h"

using namespace std;

struct Parcel {
    int parcelID;
    string destination;
    int weight;
    int priority; // 1 = Overnight, 2 = 2-Day, 3 = Normal
    string status;
    CustomLinkedList<string> history;

    Parcel() : parcelID(0), weight(0), priority(3), status("Unknown") {}

    Parcel(int id, string dest, int w, int p) 
        : parcelID(id), destination(dest), weight(w), priority(p), status("Loaded") {
        history.append("Loaded at " + destination); // Initial history
    }

    // Overload < operator for MinHeap
    bool operator<(const Parcel& other) const {
        if (priority != other.priority) {
            return priority < other.priority; // Lower val = Higher priority
        }
        return weight > other.weight; 
    }
    
    void updateStatus(string newStatus) {
        status = newStatus;
        history.append("Status changed to: " + newStatus); 
    }
};

#endif // PARCEL_H

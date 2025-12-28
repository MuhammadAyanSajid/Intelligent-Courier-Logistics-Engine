#ifndef PARCEL_H
#define PARCEL_H

#include <string>
#include <ctime>
#include "structures.h"

using namespace std;

// Helper function to get current timestamp as string
inline string getCurrentTimestamp()
{
    time_t now = time(0);
    char buffer[20];
    strftime(buffer, 20, "%Y-%m-%d %H:%M", localtime(&now));
    return string(buffer);
}

struct Parcel
{
    int parcelID;
    string destination;
    int weight;
    int priority; // 1 = Overnight, 2 = 2-Day, 3 = Normal
    string status;
    CustomLinkedList<string> history;

    Parcel() : parcelID(0), weight(0), priority(3), status("Unknown") {}

    Parcel(int id, string dest, int w, int p)
        : parcelID(id), destination(dest), weight(w), priority(p), status("Loaded")
    {
        history.append("[" + getCurrentTimestamp() + "] Created - Destination: " + destination);
    }

    // Overload < operator for MinHeap (lower priority number = higher priority)
    bool operator<(const Parcel &other) const
    {
        if (priority != other.priority)
        {
            return priority < other.priority; // Lower val = Higher priority
        }
        return weight > other.weight; // Heavier parcels have higher priority as tie-breaker
    }

    // Equality operator for comparisons
    bool operator==(const Parcel &other) const
    {
        return parcelID == other.parcelID;
    }

    void updateStatus(string newStatus)
    {
        status = newStatus;
        history.append("[" + getCurrentTimestamp() + "] " + newStatus);
    }
};

#endif // PARCEL_H

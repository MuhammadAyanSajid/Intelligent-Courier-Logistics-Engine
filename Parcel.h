#ifndef PARCEL_H
#define PARCEL_H

#include <string>
#include <ctime>
#include "structures.h"

using namespace std;

// Weight category constants
enum WeightCategory
{
    WEIGHT_LIGHT = 1,  // 0-5 kg
    WEIGHT_MEDIUM = 2, // 6-20 kg
    WEIGHT_HEAVY = 3   // 21+ kg
};

// Parcel status constants
const string STATUS_CREATED = "Created";
const string STATUS_PENDING_PICKUP = "Pending Pickup";
const string STATUS_PICKED_UP = "Picked Up";
const string STATUS_IN_WAREHOUSE = "In Warehouse";
const string STATUS_QUEUED_LOADING = "Queued for Loading";
const string STATUS_IN_TRANSIT = "In Transit";
const string STATUS_OUT_FOR_DELIVERY = "Out for Delivery";
const string STATUS_DELIVERY_ATTEMPTED = "Delivery Attempted";
const string STATUS_DELIVERED = "Delivered";
const string STATUS_RETURNED = "Returned to Sender";
const string STATUS_MISSING = "Missing";

// Helper function to get current timestamp as string
inline string getCurrentTimestamp()
{
    time_t now = time(0);
    char buffer[20];
    strftime(buffer, 20, "%Y-%m-%d %H:%M", localtime(&now));
    return string(buffer);
}

// Helper function to get weight category from weight
inline WeightCategory getWeightCategory(int weight)
{
    if (weight <= 5)
        return WEIGHT_LIGHT;
    else if (weight <= 20)
        return WEIGHT_MEDIUM;
    else
        return WEIGHT_HEAVY;
}

// Helper function to get weight category name
inline string getWeightCategoryName(WeightCategory cat)
{
    switch (cat)
    {
    case WEIGHT_LIGHT:
        return "Light (0-5 kg)";
    case WEIGHT_MEDIUM:
        return "Medium (6-20 kg)";
    case WEIGHT_HEAVY:
        return "Heavy (21+ kg)";
    default:
        return "Unknown";
    }
}

class Parcel
{
private:
    int parcelID;
    string destination;
    string zone; // Destination zone (North, South, East, West, Central)
    int weight;
    WeightCategory weightCat; // Weight category
    int priority;             // 1 = Overnight, 2 = 2-Day, 3 = Normal
    string status;
    int deliveryAttempts; // Number of delivery attempts made
    int maxAttempts;      // Maximum attempts before return to sender
    string senderAddress; // For return-to-sender functionality
    int assignedRiderID;  // ID of rider assigned to this parcel (-1 if none)
    CustomLinkedList<string> history;

public:
    Parcel() : parcelID(0), weight(0), weightCat(WEIGHT_LIGHT), priority(3),
               status("Unknown"), deliveryAttempts(0), maxAttempts(3),
               assignedRiderID(-1) {}

    Parcel(int id, string dest, int w, int p, string sender = "Warehouse")
        : parcelID(id), destination(dest), weight(w), priority(p),
          status(STATUS_CREATED), deliveryAttempts(0), maxAttempts(3),
          senderAddress(sender), assignedRiderID(-1)
    {
        weightCat = getWeightCategory(w);
        zone = "Unassigned"; // Will be set based on destination
        history.append("[" + getCurrentTimestamp() + "] Created - Destination: " + destination);
    }

    // Getters
    int getParcelID() const { return parcelID; }
    string getDestination() const { return destination; }
    string getZone() const { return zone; }
    int getWeight() const { return weight; }
    WeightCategory getWeightCat() const { return weightCat; }
    int getPriority() const { return priority; }
    string getStatus() const { return status; }
    int getDeliveryAttempts() const { return deliveryAttempts; }
    int getMaxAttempts() const { return maxAttempts; }
    string getSenderAddress() const { return senderAddress; }
    int getAssignedRiderID() const { return assignedRiderID; }
    const CustomLinkedList<string> &getHistory() const { return history; }

    // Setters
    void setParcelID(int id) { parcelID = id; }
    void setDestination(string dest) { destination = dest; }
    void setWeight(int w)
    {
        weight = w;
        weightCat = getWeightCategory(w);
    }
    void setPriority(int p) { priority = p; }
    void setStatus(string s) { status = s; }
    void setDeliveryAttempts(int attempts) { deliveryAttempts = attempts; }
    void setSenderAddress(string addr) { senderAddress = addr; }
    void setAssignedRiderID(int id) { assignedRiderID = id; }
    void addToHistory(string entry) { history.append(entry); }

    // Overload < operator for MinHeap (lower priority number = higher priority)
    bool operator<(const Parcel &other) const
    {
        if (priority != other.priority)
        {
            return priority < other.priority; // Lower val = Higher priority
        }
        // Secondary: weight category (heavier = higher priority)
        if (weightCat != other.weightCat)
        {
            return weightCat > other.weightCat;
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

    // Record a delivery attempt
    bool recordDeliveryAttempt(string reason = "Recipient not available")
    {
        deliveryAttempts++;
        history.append("[" + getCurrentTimestamp() + "] Delivery Attempt #" +
                       to_string(deliveryAttempts) + " Failed: " + reason);

        if (deliveryAttempts >= maxAttempts)
        {
            updateStatus(STATUS_RETURNED);
            history.append("[" + getCurrentTimestamp() + "] Max attempts reached. Returning to sender.");
            return false; // Indicates parcel should be returned
        }
        updateStatus(STATUS_DELIVERY_ATTEMPTED);
        return true; // Can still attempt delivery
    }

    // Mark as delivered
    void markDelivered()
    {
        updateStatus(STATUS_DELIVERED);
        history.append("[" + getCurrentTimestamp() + "] Successfully delivered to " + destination);
    }

    // Return to sender
    void returnToSender(string reason)
    {
        updateStatus(STATUS_RETURNED);
        history.append("[" + getCurrentTimestamp() + "] Returned to sender: " + reason);
    }

    // Set zone based on destination
    void setZone(string z)
    {
        zone = z;
        history.append("[" + getCurrentTimestamp() + "] Assigned to zone: " + zone);
    }

    // Check if parcel can still be delivered
    bool canAttemptDelivery() const
    {
        return deliveryAttempts < maxAttempts &&
               status != STATUS_DELIVERED &&
               status != STATUS_RETURNED;
    }

    // Get priority as string
    string getPriorityString() const
    {
        switch (priority)
        {
        case 1:
            return "Overnight (HIGH)";
        case 2:
            return "2-Day (MEDIUM)";
        case 3:
            return "Normal (LOW)";
        default:
            return "Unknown";
        }
    }
};

#endif // PARCEL_H

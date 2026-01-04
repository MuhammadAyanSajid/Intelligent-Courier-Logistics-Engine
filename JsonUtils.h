#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <string>
#include <sstream>
#include "Parcel.h"
#include "CourierOperations.h"
#include "Graph.h"
#include "structures.h"

using namespace std;

namespace JsonUtils
{
    // Escape special characters for JSON strings
    inline string escapeJson(const string &str)
    {
        string result;
        for (char c : str)
        {
            switch (c)
            {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
            }
        }
        return result;
    }

    // Convert Parcel to JSON
    inline string parcelToJson(const Parcel &p)
    {
        stringstream ss;
        ss << "{";
        ss << "\"parcelID\":" << p.getParcelID() << ",";
        ss << "\"destination\":\"" << escapeJson(p.getDestination()) << "\",";
        ss << "\"zone\":\"" << escapeJson(p.getZone()) << "\",";
        ss << "\"weight\":" << p.getWeight() << ",";
        ss << "\"weightCategory\":\"" << escapeJson(getWeightCategoryName(p.getWeightCat())) << "\",";
        ss << "\"priority\":" << p.getPriority() << ",";
        ss << "\"priorityString\":\"" << escapeJson(p.getPriorityString()) << "\",";
        ss << "\"status\":\"" << escapeJson(p.getStatus()) << "\",";
        ss << "\"deliveryAttempts\":" << p.getDeliveryAttempts() << ",";
        ss << "\"maxAttempts\":" << p.getMaxAttempts() << ",";
        ss << "\"senderAddress\":\"" << escapeJson(p.getSenderAddress()) << "\",";
        ss << "\"assignedRiderID\":" << p.getAssignedRiderID() << ",";
        ss << "\"history\":[";
        const CustomLinkedList<string> &history = p.getHistory();
        Node<string> *curr = history.getHead();
        bool first = true;
        while (curr)
        {
            if (!first)
                ss << ",";
            ss << "\"" << escapeJson(curr->data) << "\"";
            first = false;
            curr = curr->next;
        }
        ss << "]}";
        return ss.str();
    }

    // Convert Rider to JSON
    inline string riderToJson(const Rider &r)
    {
        stringstream ss;
        ss << "{";
        ss << "\"riderID\":" << r.getRiderID() << ",";
        ss << "\"name\":\"" << escapeJson(r.getName()) << "\",";
        ss << "\"currentLoad\":" << r.getCurrentLoad() << ",";
        ss << "\"maxCapacity\":" << r.getMaxCapacity() << ",";
        ss << "\"assignedParcels\":" << r.getAssignedParcels() << ",";
        ss << "\"maxParcels\":" << r.getMaxParcels() << ",";
        ss << "\"currentZone\":\"" << escapeJson(r.getCurrentZone()) << "\",";
        ss << "\"available\":" << (r.isAvailable() ? "true" : "false") << ",";
        ss << "\"parcelIDs\":[";
        const CustomVector<int> &ids = r.getParcelIDs();
        for (int i = 0; i < ids.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << ids[i];
        }
        ss << "]}";
        return ss.str();
    }

    // Convert RouteResult to JSON
    inline string routeToJson(const RouteResult &route)
    {
        stringstream ss;
        ss << "{\"totalCost\":" << route.totalCost << ",";
        ss << "\"valid\":" << (route.valid ? "true" : "false") << ",";
        ss << "\"path\":[";
        for (int i = 0; i < route.path.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << "\"" << escapeJson(route.path[i]) << "\"";
        }
        ss << "]}";
        return ss.str();
    }

    // Convert Edge to JSON (for graph visualization)
    inline string edgeToJson(const string &from, const string &to, int weight, bool blocked)
    {
        stringstream ss;
        ss << "{\"from\":\"" << escapeJson(from) << "\",";
        ss << "\"to\":\"" << escapeJson(to) << "\",";
        ss << "\"weight\":" << weight << ",";
        ss << "\"blocked\":" << (blocked ? "true" : "false") << "}";
        return ss.str();
    }

    // Convert CustomVector<Parcel> to JSON array
    inline string parcelsToJson(const CustomVector<Parcel> &parcels)
    {
        stringstream ss;
        ss << "[";
        for (int i = 0; i < parcels.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << parcelToJson(parcels[i]);
        }
        ss << "]";
        return ss.str();
    }

    // Convert CustomVector<Rider> to JSON array
    inline string ridersToJson(const CustomVector<Rider> &riders)
    {
        stringstream ss;
        ss << "[";
        for (int i = 0; i < riders.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << riderToJson(riders[i]);
        }
        ss << "]";
        return ss.str();
    }

    // Convert CustomVector<RouteResult> to JSON array
    inline string routesToJson(const CustomVector<RouteResult> &routes)
    {
        stringstream ss;
        ss << "[";
        for (int i = 0; i < routes.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << routeToJson(routes[i]);
        }
        ss << "]";
        return ss.str();
    }

    // Convert CustomVector<string> to JSON array
    inline string stringsToJson(const CustomVector<string> &strings)
    {
        stringstream ss;
        ss << "[";
        for (int i = 0; i < strings.getSize(); i++)
        {
            if (i > 0)
                ss << ",";
            ss << "\"" << escapeJson(strings[i]) << "\"";
        }
        ss << "]";
        return ss.str();
    }

    // Create success response JSON
    inline string successResponse(const string &message, const string &data = "")
    {
        stringstream ss;
        ss << "{\"success\":true,\"message\":\"" << escapeJson(message) << "\"";
        if (!data.empty())
            ss << ",\"data\":" << data;
        ss << "}";
        return ss.str();
    }

    // Create error response JSON
    inline string errorResponse(const string &message)
    {
        return "{\"success\":false,\"error\":\"" + escapeJson(message) + "\"}";
    }

    // Simple JSON parser - get string value by key
    inline string getJsonValue(const string &json, const string &key)
    {
        string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == string::npos)
            return "";

        pos += searchKey.length();

        // Skip whitespace
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
            pos++;

        if (pos >= json.length())
            return "";

        // Check if value is a string (starts with quote)
        if (json[pos] == '"')
        {
            pos++; // Skip opening quote
            string value;
            while (pos < json.length() && json[pos] != '"')
            {
                if (json[pos] == '\\' && pos + 1 < json.length())
                {
                    pos++; // Skip escape character
                }
                value += json[pos++];
            }
            return value;
        }
        else
        {
            // Value is a number or boolean
            string value;
            while (pos < json.length() && json[pos] != ',' && json[pos] != '}' && json[pos] != ' ')
            {
                value += json[pos++];
            }
            return value;
        }
    }

    // Get integer value from JSON
    inline int getJsonInt(const string &json, const string &key, int defaultVal = 0)
    {
        string val = getJsonValue(json, key);
        if (val.empty())
            return defaultVal;
        try
        {
            return stoi(val);
        }
        catch (...)
        {
            return defaultVal;
        }
    }

    // Get boolean value from JSON
    inline bool getJsonBool(const string &json, const string &key, bool defaultVal = false)
    {
        string val = getJsonValue(json, key);
        if (val == "true")
            return true;
        if (val == "false")
            return false;
        return defaultVal;
    }
}

#endif
#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <iostream>
#include <string>
#include "Graph.h"

using namespace std;

namespace Utils
{

    // Clears the input buffer safely with EOF check
    void clearInputBuffer()
    {
        cin.clear();
        // Check for EOF before consuming characters
        if (cin.eof())
        {
            cin.clear();
            return;
        }
        int ch;
        while ((ch = cin.get()) != '\n' && ch != EOF)
        {
            continue;
        }
    }

    // Get a validated integer within a range
    int getIntInput(string prompt, int min, int max)
    {
        int value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
            {
                cout << "\nEnd of input reached. Using default: " << min << endl;
                return min;
            }
            if (cin >> value)
            {
                if (value >= min && value <= max)
                {
                    clearInputBuffer(); // Consume trailing newline
                    return value;
                }
                else
                {
                    cout << "Error: Valid range is " << min << " to " << max << ".\n";
                }
            }
            else
            {
                cout << "Error: Invalid input. Please enter a number.\n";
                clearInputBuffer();
            }
        }
    }

    // Get a non-empty string (single word)
    string getStringInput(string prompt)
    {
        string value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
            {
                cout << "\nEnd of input.\n";
                return "";
            }
            cin >> value;
            clearInputBuffer();
            if (!value.empty())
            {
                return value;
            }
            cout << "Error: Input cannot be empty.\n";
        }
    }

    // Get a full line of input (can have spaces)
    string getLineInput(string prompt)
    {
        string value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return "";
            getline(cin, value);
            if (!value.empty())
            {
                return value;
            }
            cout << "Error: Input cannot be empty.\n";
        }
    }

    // Check if string is alphabetic
    bool isAlphabetic(const string &s)
    {
        if (s.empty())
            return false;
        for (size_t i = 0; i < s.length(); i++)
        {
            char c = s[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
            {
                return false;
            }
        }
        return true;
    }

    // Get a string strictly containing only alphabets
    string getAlphabeticStringInput(string prompt)
    {
        string value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return "";
            cin >> value;       // Read word
            clearInputBuffer(); // Consume rest of line if any

            if (!value.empty() && isAlphabetic(value))
            {
                return value;
            }
            cout << "Error: Input must contain only alphabetic characters (A-Z, a-z).\n";
        }
    }

    // Get a valid city name that exists in the graph
    string getValidCity(const string &prompt, Graph &graph)
    {
        // Show available cities
        CustomVector<string> cities = graph.getAllCities();
        cout << "Available cities: ";
        for (int i = 0; i < cities.getSize(); i++)
        {
            cout << cities[i];
            if (i < cities.getSize() - 1)
                cout << ", ";
        }
        cout << endl;

        string value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return "";
            cin >> value;
            clearInputBuffer();

            if (graph.cityExists(value))
            {
                return value;
            }
            cout << "Error: City '" << value << "' not found in the map.\n";
        }
    }

    // Validate yes/no input
    bool getYesNoInput(string prompt)
    {
        string value;
        while (true)
        {
            cout << prompt << " (y/n): ";
            cin >> value;
            clearInputBuffer();

            if (value == "y" || value == "Y" || value == "yes" || value == "Yes")
            {
                return true;
            }
            if (value == "n" || value == "N" || value == "no" || value == "No")
            {
                return false;
            }
            cout << "Error: Please enter 'y' or 'n'.\n";
        }
    }

    // Helper to get ANY integer (no range restriction)
    int getIntInputSimple(string prompt)
    {
        int value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return 0;
            if (cin >> value)
            {
                clearInputBuffer();
                return value;
            }
            else
            {
                cout << "Error: Please enter a valid number.\n";
                clearInputBuffer();
            }
        }
    }

    // Check if string is alphanumeric (letters, numbers, spaces allowed)
    bool isAlphanumeric(const string &s)
    {
        if (s.empty())
            return false;
        for (size_t i = 0; i < s.length(); i++)
        {
            char c = s[i];
            bool valid = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == ' ' || c == '-' || c == ',' || c == '.';
            if (!valid)
            {
                return false;
            }
        }
        return true;
    }

    // Get alphanumeric string input (for addresses)
    string getAlphanumericInput(string prompt)
    {
        string value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return "";
            getline(cin, value);

            if (!value.empty() && isAlphanumeric(value))
            {
                return value;
            }
            cout << "Error: Input must contain only letters, numbers, spaces, commas, periods, and hyphens.\n";
        }
    }

    // Validate positive number
    bool isPositive(int value)
    {
        return value > 0;
    }

    // Get a positive integer input
    int getPositiveIntInput(string prompt)
    {
        int value;
        while (true)
        {
            cout << prompt;
            if (cin.eof())
                return 1;
            if (cin >> value && value > 0)
            {
                clearInputBuffer();
                return value;
            }
            else
            {
                cout << "Error: Please enter a positive number.\n";
                clearInputBuffer();
            }
        }
    }

    // Validate parcel status for operations
    bool canModifyParcel(const string &status)
    {
        return status != "Delivered" && status != "Returned to Sender" && status != "Missing";
    }

    // Display validation error with context
    void showValidationError(const string &field, const string &reason)
    {
        cout << "Validation Error [" << field << "]: " << reason << "\n";
    }
}

#endif // VALIDATION_UTILS_H

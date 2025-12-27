#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <iostream>
#include <string>
#include "Graph.h"

using namespace std;

namespace Utils {

    // Clears the input buffer
    void clearInputBuffer() {
        cin.clear();
        // Manual flush without numeric_limits
        while (cin.get() != '\n') {
            continue;
        }
    }

    // Get a validated integer within a range
    int getIntInput(string prompt, int min, int max) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                if (value >= min && value <= max) {
                    clearInputBuffer(); // Consume trailing newline
                    return value;
                } else {
                    cout << "Error: valid range is " << min << " to " << max << ".\n";
                }
            } else {
                cout << "Error: Invalid input. Please enter a number.\n";
                clearInputBuffer();
            }
        }
    }

    // Get a non-empty string
    string getStringInput(string prompt) {
        string value;
        while (true) {
            cout << prompt;
            getline(cin, value);
            if (!value.empty()) {
                return value;
            }
            cout << "Error: Input cannot be empty.\n";
        }
    }

    // Check if string is alphabetic
    bool isAlphabetic(const string& s) {
        for (char c : s) {
            if (!isalpha(c)) return false;
        }
        return true;
    }

    // Get a string strictly containing only alphabets
    string getAlphabeticStringInput(string prompt) {
        string value;
        while (true) {
            cout << prompt;
            cin >> value; // Read word
            clearInputBuffer(); // Consume rest of line if any
            
            if (!value.empty() && isAlphabetic(value)) {
                return value;
            }
            cout << "Error: Input must contain only alphabetic characters (A-Z, a-z).\n";
        }
    }

    // Get a valid city name (must exist in Graph)
    // We need to check existence. Since Graph uses private lookup, we might need a helper method in Graph
    // or just rely on try/catch (if we had exceptions) or bool check.
    // For now, let's just get a string. Validation happens at logic layer or we add 'isValidCity' to Graph.
    
    // Helper to get ANY integer
    int getIntInputSimple(string prompt) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value) {
                clearInputBuffer();
                return value;
            } else {
                cout << "Error: Please enter a valid number.\n";
                clearInputBuffer();
            }
        }
    }
}

#endif // VALIDATION_UTILS_H

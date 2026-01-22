# SwiftEx Courier System

SwiftEx is a comprehensive logistics and courier management system designed to handle parcel tracking, routing, and delivery operations. This project features a dual-interface approach, offering both a high-performance C++ Console Engine and a modern Web Application.

## 🚀 Features

### Core Capabilities
*   **Parcels Management:** Add, withdraw, track, and categorize parcels (by weight, destination, priority).
*   **Intelligent Routing:**
    *   Find the shortest path between cities (Dijkstra's Algorithm).
    *   Calculate alternative routes.
    *   Handle blocked or overloaded routes dynamically.
*   **Rider Management:** Manage rider fleet, assign parcels, and track capacity/load.
*   **Operations:** Process parcels through stages: Pickup -> Warehouse -> Transit -> Delivered.
*   **Reporting:** Track missing parcels, view statistics, and generate operation logs.

### Interfaces

#### 1. C++ Console Engine
A robust command-line interface for efficient backend management and simulation.
*   **Algorithms:** Implements custom Data Structures (Vector, LinkedList, Stack, Queue, MinHeap, Graph, HashTable) for optimized performance.
*   **File Persistence:** Saves and loads system state using CSV/Text files.
*   **Security:** Admin authentication system.

#### 2. Web Application
A user-friendly browser-based dashboard.
*   **Dashboard:** Real-time overview of system stats (Queue status, Active riders, Delivery counts).
*   **Interactive UI:** Visual management of parcels, riders, and routes.
*   **Technology:** Node.js (Express) backend serving a vanilla JS/HTML/CSS frontend.
*   **Data Persistence:** Uses JSON files for data storage.

## 🛠️ Technology Stack

*   **Logic Core:** C++ (Standard 11+)
*   **Web Backend:** Node.js, Express
*   **Frontend:** HTML5, CSS3, JavaScript (ES6+)
*   **Data Storage:** CSV (Console App) & JSON (Web App)

## 📂 Project Structure

```
├── CourierSystem.cpp/h   # C++ Core Logic (System Controller)
├── DataStructures.h      # Custom Template Classes (Graph, Heap, etc.)
├── Models.h              # Data Structures (Parcel, City, Rider)
├── Utils.h               # Utility functions (File I/O, UI Helpers)
├── main.cpp              # C++ Console Application Entry Point
├── server.js             # Node.js Web Server & API
├── data/                 # JSON Data Storage for Web App
├── web/                  # Frontend Assets (HTML/CSS/JS)
├── compile.bat           # Script to compile C++ application
├── start.bat             # Script to start Node.js server (Windows)
└── start.sh              # Script to start Node.js server (Linux/Mac)
```

## ⚡ Getting Started

### Prerequisites

*   **C++ Compiler:** G++ (MinGW), Clang, or MSVC.
*   **Node.js & npm:** For running the web dashboard.

### 🖥️ Running the C++ Console App

1.  **Compile the code**:
    Run the provided batch script:
    ```bash
    ./compile.bat
    ```
    Or manually compile:
    ```bash
    g++ -std=c++11 -O2 -o courier_app.exe main.cpp CourierSystem.cpp
    ```

2.  **Run the executable**:
    ```bash
    ./courier_app.exe
    ```
3.  **Login**:
    *   Default credentials may need to be initialized or checked in `admins.txt` (if generated) or the source code.

### 🌐 Running the Web Application

1.  **Install Dependencies**:
    ```bash
    npm install
    ```

2.  **Start the Server**:
    *   **Windows:** Run `start.bat`
    *   **Linux/Mac:** Run `./start.sh`
    *   **Manual:** `node server.js`

3.  **Access Dashboard**:
    Open your browser and navigate to:
    `http://localhost:3000`

    *   **Default Web Login**:
        *   Username: `admin`
        *   Password: `admin123`

## 📝 Notes

*   The C++ application and Web application currently maintain separate data stores (CSV vs JSON). Changes in one may not reflect in the other immediately unless synchronized manually or through future integration updates.
*   Ensure the `data/` directory exists for the Web App to function correctly.

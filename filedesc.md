# Intelligent Courier Logistics Engine v3.0
## DSA Lab Project - File Description

---

### 1. main.cpp
- **Entry point** of the application.
- Supports dual modes:
  - **CLI Mode**: Menu-driven console interface with 19+ options
  - **Web Mode**: REST API server (launch with `--web` flag)
- Coordinates interaction between all modules.
- Loads map data (`map_data.txt`) and parcel data (`parcels.txt`) at startup.
- Initializes default riders (Ali, Ahmed, Sara).
- Controls overall system flow including:
  - Parcel management (add, search, dispatch)
  - Delivery operations (attempts, mark delivered, return to sender)
  - Queue management (pickup, warehouse, transit)
w  - Rider assignment
  - Undo functionality

---

### 2. Parcel.h
- Defines the **Parcel class** with encapsulated attributes.
- Stores attributes:
  - Parcel ID
  - Priority (1=Overnight, 2=2-Day, 3=Normal)
  - Weight & Weight Category (Light/Medium/Heavy)
  - Destination & Zone
  - Current status (Created, Pending Pickup, In Transit, Delivered, etc.)
  - Delivery attempts tracking (max 3 attempts)
  - Sender address (for return-to-sender)
  - Assigned rider ID
  - History log (using CustomLinkedList)
- Provides getter/setter methods and status constants.
- Includes helper functions: `getWeightCategory()`, `getWeightCategoryName()`, `getCurrentTimestamp()`.

---

### 3. ParcelManager.h / ParcelManager.cpp
- Responsible for parcel storage, sorting, and tracking.
- Uses **MinHeap** for priority-based parcel queue.
- Uses **HashMap** for O(1) parcel lookup by ID.
- Supports:
  - Insertion and removal of parcels
  - Priority-based dispatching (`dispatchNext()`)
  - Duplicate ID checking (`parcelExists()`)
  - Status updates throughout parcel lifecycle
  - Re-insertion for undo operations
- File I/O: `saveParcels()` / `loadParcels()` for persistence.

---

### 4. Graph.h / Graph.cpp
- Implements a **weighted Graph** representing the delivery network.
- Uses **adjacency list** representation (CustomVector of CustomLinkedList).
- Uses **HashMap** for O(1) city name lookup.
- Key features:
  - Nodes represent cities/locations
  - Edges represent roads with weights (distance/cost)
  - Road blocking/unblocking for route simulation
- **Routing algorithms**:
  - Dijkstra's algorithm for shortest path
  - **Yen's K-Shortest Paths** algorithm for alternative routes
- **Zone Management**:
  - Add/manage zones (North, South, East, West, Central)
  - Assign cities to zones
  - Auto-assign zones based on city configuration
- File I/O: `loadGraph()` / `saveGraph()` for map persistence.

---

### 5. CourierOperations.h / CourierOperations.cpp
- Simulates complete courier workflow using custom data structures.
- **Queue Management** (using CustomQueue):
  - Pickup Queue: Parcels waiting to be collected
  - Warehouse Queue: Parcels staged for loading
  - Transit Queue: Parcels currently being delivered
- **Rider Class**: Manages courier capacity, load, zone assignment.
- **Rider Management** (using HashMap):
  - Add/find riders
  - Find best rider by capacity and zone
  - Assign/release parcels from riders
- **Missing Parcel Detection**: Track and resolve missing parcels.
- **Undo System** (using CustomStack):
  - Action class stores operation type and state
  - Supports undo for: add parcel, dispatch, block road, status changes, rider assignment, delivery attempts
  - Full audit log of all actions

---

### 6. structures.h
- Contains **custom data structure implementations** (no STL):
  - `Node<T>`: Generic node for linked structures
  - `CustomVector<T>`: Dynamic array with auto-resize
  - `CustomLinkedList<T>`: Doubly linked list
  - `CustomStack<T>`: LIFO stack
  - `CustomQueue<T>`: FIFO queue
  - `MinHeap<T>`: Priority queue (min-heap)
  - `HashMap<K, V>`: Hash table with chaining
- Used across all modules for consistency.
- Includes copy constructors and assignment operators.

---

### 7. ValidationUtils.h
- Provides **input validation utilities** in `Utils` namespace.
- Functions include:
  - `getIntInput()`: Validated integer input with range checking
  - `getStringInput()`: Non-empty single word input
  - `getLineInput()`: Full line input with spaces
  - `getValidCity()`: Validates city exists in graph
  - `getAlphanumericInput()`: Alphanumeric string validation
  - `clearInputBuffer()`: Safe input buffer clearing (handles EOF)
- Ensures safe and correct user input handling.
- Prevents invalid data processing in CLI mode.

---

### 8. WebServer.h / WebServer.cpp
- Implements **REST API server** using httplib library.
- Endpoints for:
  - Parcel management (CRUD operations)
  - Queue operations (pickup, warehouse, transit)
  - Rider management
  - Route finding and zone management
  - System status
- Serves static files from `public/` folder.
- Enables web-based UI interaction.

---

### 9. JsonUtils.h
- Provides JSON serialization utilities for web API.
- Converts Parcel, Rider, and Route objects to JSON format.
- Used by WebServer for API responses.

---

### 10. Input/Output Files
- **map_data.txt** → Stores delivery network:
  - `CITY <name>`: Define city nodes
  - `ROUTE <from> <to> <weight>`: Define weighted edges
  - `ZONE <name>`: Define delivery zones
  - `ZONECITY <zone> <city>`: Assign city to zone
- **parcels.txt** → Stores parcel data (pipe-delimited):
  - Format: `ID|Destination|Weight|Priority|Status`

---

### 11. public/ (Web UI)
- **index.html**: Main web interface page
- **styles.css**: Styling for web UI

---

## 3.3 Data Structures & Algorithm Justification

### Data Structures Used:

| Module | Data Structure | Purpose |
|--------|----------------|---------|
| Parcel Management | MinHeap | Priority-based parcel dispatching (O(log n) insert/extract) |
| Parcel Management | HashMap<int, Parcel> | O(1) parcel lookup by ID |
| Routing | Graph (Adjacency List) | Represents road network with weighted edges |
| Routing | HashMap<string, int> | O(1) city name to index lookup |
| Parcel Tracking | CustomLinkedList<string> | Maintains parcel status history log |
| Pickup Operations | CustomQueue<Parcel> | FIFO processing of pickup requests |
| Warehouse Operations | CustomQueue<Parcel> | FIFO staging of parcels for loading |
| Transit Operations | CustomQueue<Parcel> | FIFO tracking of in-transit parcels |
| Undo System | CustomStack<Action> | LIFO audit log for operation reversal |
| Rider Management | HashMap<int, Rider> | O(1) rider lookup and management |
| Missing Parcels | HashMap<int, Parcel> | Track potentially missing parcels |
| Zone Management | HashMap<string, Zone> | Zone storage and lookup |
| City-Zone Mapping | HashMap<string, string> | O(1) city to zone resolution |
| Dynamic Storage | CustomVector<T> | Auto-resizing arrays for flexible storage |

### Algorithms Used:

| Algorithm | Module | Purpose |
|-----------|--------|---------|
| Dijkstra's Algorithm | Graph.cpp | Find shortest path between cities |
| Yen's K-Shortest Paths | Graph.cpp | Find K alternative routes for delivery |
| Heap Operations (Heapify) | structures.h | Maintain priority queue property |
| Hash Function (Chaining) | structures.h | Collision resolution in HashMap |
| Linear Search | CustomVector | Find elements in dynamic arrays |
| Doubly Linked List Ops | CustomLinkedList | Efficient insert/delete at both ends |

### Complexity Analysis:

| Operation | Time Complexity | Data Structure |
|-----------|-----------------|----------------|
| Add Parcel | O(log n) | MinHeap |
| Dispatch Next (Priority) | O(log n) | MinHeap |
| Find Parcel by ID | O(1) average | HashMap |
| Shortest Path | O((V+E) log V) | Graph + MinHeap |
| K-Shortest Paths | O(K * V * (V+E) log V) | Yen's Algorithm |
| Enqueue/Dequeue | O(1) | CustomQueue |
| Push/Pop (Undo) | O(1) | CustomStack |
| Add to History | O(1) | CustomLinkedList |

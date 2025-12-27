# Project Analysis & Documentation
## Intelligent Courier Logistics Engine

### 1. Time Complexity Analysis

**a. Inserting a Parcel into the Heap (`MinHeap::insert`)**
- **Complexity:** $O(\log N)$
- **Explanation:** In a binary heap, insertion involves adding the element to the end of the array (leaf) and then "bubbling up" to restore the heap property. The height of the tree is $\log N$, so in the worst case, we perform swaps proportional to the height.

**b. Shortest Path Calculation (Dijkstra's Algorithm)**
- **Complexity:** $O(E \log V)$ (or more accurately $O(E \log E)$ with our implementation)
- **Explanation:** Our implementation uses a Binary Heap (MinHeap) as a priority queue.
    - Each node is extracted once: $V \times O(\log V)$ (or log E with lazy insertion).
    - Each edge is relaxed once. If a shorter path is found, we push to the heap. In the worst case (no `decreaseKey`), we might have multiple entries for a node in the heap, up to $E$ entries.
    - Thus, operations are roughly proportional to number of edges times the cost of heap insertion: $O(E \log E)$. Since $E \le V^2$, $\log E \approx 2 \log V$, so it simplifies to $O(E \log V)$ for sparse graphs.

**c. Searching for a Parcel in the Hash Map (`HashMap::get`)**
- **Complexity:** Average Case $O(1)$, Worst Case $O(N)$
- **Explanation:**
    - **Average:** The hash function distributes keys uniformly. Accessing the bucket and finding the element is fast.
    - **Worst:** If many keys collide (hash to the same bucket), the linked list at that bucket grows to length $N$, degrading search to linear time. Our implementation uses separate chaining for collisions.

### 2. Justification of Data Structures

**Why Min-Heap for Sorting Parcels?**
We need to efficiently retrieve the *highest priority* (minimum value) parcel repeatedly.
- **vs. Unsorted Array:** Finding the min would be $O(N)$, which is slow for repeated dispatching.
- **vs. Sorted Array:** Retrieving min is $O(1)$, but inserting a new parcel would be $O(N)$ (shifting elements).
- **Min-Heap:** Offers a balanced approach with $O(\log N)$ for both insertion and extraction, making it ideal for a dynamic priority queue where items are constantly added and removed.

**Why Adjacency List for Graph?**
The map of cities and roads resembles a **sparse graph** (each city connects to only a few others, not all).
- **vs. Adjacency Matrix:** A matrix requires $V^2$ space. If we have 100 cities but only 200 roads, a matrix stores 10,000 cells (mostly empty), whereas lists store only the 200 connections.
- **Performance:** Iterating over neighbors of a node (crucial for Dijkstra) takes time proportional to the degree of the node in an Adjacency List, whereas in a Matrix it takes $O(V)$ regardless of connections. Thus, Adjacency Lists are more space and time efficient for our use case.

### 3. Class Diagram Description

The system is composed of the following key relationships:

1.  **`ParcelManager`**
    - **Composition:** Contains one `MinHeap<Parcel>` (the `priorityQueue`).
    - **Composition:** Contains one `HashMap<int, Parcel>` (the `parcelMap`) for fast lookups.
    - **Usage:** Manages the lifecycle of `Parcel` objects (creation, storage, retrieval).

2.  **`Graph`**
    - **Composition:** Contains a `CustomVector<CustomLinkedList<Edge>>` (the `adjList`) to store the graph structure.
    - **Composition:** Contains `HashMap<string, int>` and `CustomVector<string>` to map City Names to ID indices.
    - **Dependency:** Uses `MinHeap` internally for Dijkstra's Algorithm logic.

3.  **`CourierOperations`**
    - **Composition:** Contains one `CustomQueue<Parcel>` (`warehouseQueue`) for FIFO loading.
    - **Composition:** Contains one `CustomStack<Action>` (`auditLog`) for tracking history for Undo.

4.  **`Parcel` (Struct)**
    - **Composition:** Contains a `CustomLinkedList<string>` for `history` logs.

5.  **`Main` (Client)**
    - Instantiates `Graph`, `ParcelManager`, and `CourierOperations`.
    - Coordinates interaction between them (e.g., getting a Parcel from Manager and passing it to Operations).

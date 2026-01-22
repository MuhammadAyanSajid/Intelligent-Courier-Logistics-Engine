#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;




template <typename T>
class Vector {
private:
    T* data;
    int capacity;
    int currentSize;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i < currentSize; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    Vector() {
        capacity = 10;
        currentSize = 0;
        data = new T[capacity];
    }

    
    Vector(const Vector& other) {
        capacity = other.capacity;
        currentSize = other.currentSize;
        data = new T[capacity];
        for (int i = 0; i < currentSize; i++) {
            data[i] = other.data[i];
        }
    }

    
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            currentSize = other.currentSize;
            data = new T[capacity];
            for (int i = 0; i < currentSize; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    ~Vector() {
        delete[] data;
    }

    void push_back(T value) {
        if (currentSize == capacity) {
            resize();
        }
        data[currentSize++] = value;
    }

    void pop_back() {
        if (currentSize > 0) {
            currentSize--;
        }
    }

    T& operator[](int index) {
        if (index < 0 || index >= currentSize) {
            throw out_of_range("Vector index out of bounds");
        }
        return data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= currentSize) {
            throw out_of_range("Vector index out of bounds");
        }
        return data[index];
    }

    int size() const {
        return currentSize;
    }

    bool empty() const {
        return currentSize == 0;
    }

    void clear() {
        currentSize = 0;
    }
    
    
    T* begin() { return data; }
    T* end() { return data + currentSize; }
};




template <typename T>
class LinkedList {
private:
    struct LLNode {
        T data;
        LLNode* next;
        LLNode(T val) : data(val), next(nullptr) {}
    };
    LLNode* head;
    LLNode* tail;
    int listSize;

public:
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}

    
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), listSize(0) {
        LLNode* current = other.head;
        while (current) {
            push_back(current->data);
            current = current->next;
        }
    }

    
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            
            while (head) {
                LLNode* temp = head;
                head = head->next;
                delete temp;
            }
            tail = nullptr;
            listSize = 0;

            
            LLNode* current = other.head;
            while (current) {
                push_back(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    ~LinkedList() {
        LLNode* current = head;
        while (current) {
            LLNode* next = current->next;
            delete current;
            current = next;
        }
    }

    void push_back(T value) {
        LLNode* newNode = new LLNode(value);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }

    void print() const {
        LLNode* current = head;
        while (current) {
cout << current->data << " -> ";
            current = current->next;
        }
cout << "NULL" << endl;
    }

    
    struct Iterator {
        LLNode* current;
        Iterator(LLNode* node) : current(node) {}
        T& operator*() { return current->data; }
        const T& operator*() const { return current->data; }
        Iterator& operator++() { current = current->next; return *this; }
        bool operator!=(const Iterator& other) { return current != other.current; }
    };
    
    
    struct ConstIterator {
        const LLNode* current;
        ConstIterator(const LLNode* node) : current(node) {}
        const T& operator*() const { return current->data; }
        ConstIterator& operator++() { current = current->next; return *this; }
        bool operator!=(const ConstIterator& other) const { return current != other.current; }
    };

    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head); }
    ConstIterator end() const { return ConstIterator(nullptr); }
    
    
    int size() const {
        return listSize;
    }
    
    
    bool empty() const {
        return listSize == 0;
    }
    
    
    void pop_back() {
        if (!head) return;
        if (head == tail) {
            delete head;
            head = tail = nullptr;
            listSize = 0;
            return;
        }
        LLNode* current = head;
        while (current->next != tail) {
            current = current->next;
        }
        delete tail;
        tail = current;
        tail->next = nullptr;
        listSize--;
    }
    
    
    T& back() {
        if (!tail) throw runtime_error("List is empty");
        return tail->data;
    }
    
    const T& back() const {
        if (!tail) throw runtime_error("List is empty");
        return tail->data;
    }
};




template <typename T>
class Stack {
private:
    struct StackNode {
        T data;
        StackNode* next;
        StackNode(T val) : data(val), next(nullptr) {}
    };
    StackNode* topNode;

public:
    Stack() : topNode(nullptr) {}
    
    
    Stack(const Stack& other) : topNode(nullptr) {
        if (other.topNode) {
            
            StackNode* current = other.topNode;
            Stack temp;
            while (current) {
                temp.push(current->data);
                current = current->next;
            }
            
            while (!temp.empty()) {
                push(temp.top());
                temp.pop();
            }
        }
    }
    
    
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            
            while (!empty()) pop();
            
            
            if (other.topNode) {
                StackNode* current = other.topNode;
                Stack temp;
                while (current) {
                    temp.push(current->data);
                    current = current->next;
                }
                while (!temp.empty()) {
                    push(temp.top());
                    temp.pop();
                }
            }
        }
        return *this;
    }

    ~Stack() {
        while (!empty()) pop();
    }

    void push(T value) {
        StackNode* newNode = new StackNode(value);
        newNode->next = topNode;
        topNode = newNode;
    }

    void pop() {
        if (empty()) return;
        StackNode* temp = topNode;
        topNode = topNode->next;
        delete temp;
    }

    T top() {
        if (empty()) throw runtime_error("Stack is empty");
        return topNode->data;
    }

    bool empty() {
        return topNode == nullptr;
    }
};




template <typename T>
class Queue {
private:
    struct QueueNode {
        T data;
        QueueNode* next;
        QueueNode(T val) : data(val), next(nullptr) {}
    };
    QueueNode* frontNode;
    QueueNode* rearNode;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr) {}

    ~Queue() {
        while (!empty()) pop();
    }

    void push(T value) {
        QueueNode* newNode = new QueueNode(value);
        if (rearNode) {
            rearNode->next = newNode;
            rearNode = newNode;
        } else {
            frontNode = rearNode = newNode;
        }
    }

    void pop() {
        if (empty()) return;
        QueueNode* temp = frontNode;
        frontNode = frontNode->next;
        if (!frontNode) rearNode = nullptr;
        delete temp;
    }

    T front() {
        if (empty()) throw runtime_error("Queue is empty");
        return frontNode->data;
    }

    bool empty() {
        return frontNode == nullptr;
    }
    
    
    
    bool remove(const T& item) {
        if (empty()) return false;
        
        
        if (frontNode->data == item) {
            pop();
            return true;
        }
        
        
        QueueNode* current = frontNode;
        QueueNode* prev = nullptr;
        
        while (current) {
            if (current->data == item) {
                
                if (prev) {
                    prev->next = current->next;
                    if (current == rearNode) {
                        rearNode = prev;
                    }
                    delete current;
                    return true;
                }
            }
            prev = current;
            current = current->next;
        }
        
        return false;
    }
    
    int size() {
        int count = 0;
        QueueNode* current = frontNode;
        while (current) {
            count++;
            current = current->next;
        }
        return count;
    }
};




template <typename T>
class MinHeap {
private:
    Vector<T> heap;

    void heapifyUp(int index) {
        if (index == 0) return;
        int parentIndex = (index - 1) / 2;
        if (heap[index] < heap[parentIndex]) {
            T temp = heap[index];
            heap[index] = heap[parentIndex];
            heap[parentIndex] = temp;
            heapifyUp(parentIndex);
        }
    }

    void heapifyDown(int index) {
        int leftChild = 2 * index + 1;
        int rightChild = 2 * index + 2;
        int smallest = index;

        if (leftChild < heap.size() && heap[leftChild] < heap[smallest]) {
            smallest = leftChild;
        }
        if (rightChild < heap.size() && heap[rightChild] < heap[smallest]) {
            smallest = rightChild;
        }

        if (smallest != index) {
            T temp = heap[index];
            heap[index] = heap[smallest];
            heap[smallest] = temp;
            heapifyDown(smallest);
        }
    }

public:
    void push(T value) {
        heap.push_back(value);
        heapifyUp(heap.size() - 1);
    }

    void pop() {
        if (heap.empty()) return;
        heap[0] = heap[heap.size() - 1];
        heap.pop_back();
        heapifyDown(0);
    }

    T top() {
        if (heap.empty()) throw runtime_error("Heap is empty");
        return heap[0];
    }

    bool empty() {
        return heap.empty();
    }
    
    int size() {
        return heap.size();
    }
    
    
    bool contains(const T& item) {
        for (int i = 0; i < heap.size(); i++) {
            if (heap[i] == item) {
                return true;
            }
        }
        return false;
    }
    
    
    
    bool remove(const T& item) {
        int index = -1;
        for (int i = 0; i < heap.size(); i++) {
            if (heap[i] == item) {
                index = i;
                break;
            }
        }
        
        if (index == -1) return false;
        
        
        heap[index] = heap[heap.size() - 1];
        heap.pop_back();
        
        
        if (index < heap.size()) {
            heapifyDown(index);
            heapifyUp(index);
        }
        
        return true;
    }
};




template <typename T>
class Graph {
public:
    struct Edge {
        int destID;
        int weight;
        Edge(int d, int w) : destID(d), weight(w) {}
    };

    struct GraphNode {
        int id;
        T data;
        LinkedList<Edge> adjacencyList;
        GraphNode() : id(-1) {}
        GraphNode(int i, T d) : id(i), data(d) {}
    };

private:
    Vector<GraphNode> nodes;

public:
    void addNode(int id, T data) {
        
        for(int i=0; i<nodes.size(); i++) {
            if(nodes[i].id == id) return; 
        }
        nodes.push_back(GraphNode(id, data));
    }

    void addEdge(int srcID, int destID, int weight) {
        
        for(int i=0; i<nodes.size(); i++) {
            if(nodes[i].id == srcID) {
                
                bool exists = false;
                for(auto& e : nodes[i].adjacencyList) {
                    if(e.destID == destID) {
                        exists = true;
                        e.weight = weight; 
                        break;
                    }
                }
                if(!exists) {
                    nodes[i].adjacencyList.push_back(Edge(destID, weight));
                }
                return;
            }
        }
    }

    
    LinkedList<Edge>* getNeighbors(int id) {
        for(int i=0; i<nodes.size(); i++) {
            if(nodes[i].id == id) {
                return &nodes[i].adjacencyList;
            }
        }
        return nullptr;
    }
    
    T* getNodeData(int id) {
        for(int i=0; i<nodes.size(); i++) {
            if(nodes[i].id == id) {
                return &nodes[i].data;
            }
        }
        return nullptr;
    }

    Vector<GraphNode>& getNodes() {
        return nodes;
    }
    
    const Vector<GraphNode>& getNodes() const {
        return nodes;
    }
};







template <typename K, typename V>
class HashTable {
private:
    struct HashNode {
        K key;
        V value;
        HashNode() : key(), value() {}
        HashNode(K k, V v) : key(k), value(v) {}
    };
    
    static const int DEFAULT_CAPACITY = 101; 
    int capacity;
    int currentSize;
    Vector<HashNode>* buckets;
    
    
    int hashFunction(int key) const {
        return (key % capacity + capacity) % capacity; 
    }
    
    
    int hashFunction(const string& key) const {
        int hash = 0;
        for (char c : key) {
            hash = (hash * 31 + c) % capacity;
        }
        return (hash + capacity) % capacity;
    }

public:
    HashTable() : capacity(DEFAULT_CAPACITY), currentSize(0) {
        buckets = new Vector<HashNode>[capacity];
    }
    
    ~HashTable() {
        delete[] buckets;
    }
    
    
    
    void insert(K key, V value) {
        int index = hashFunction(key);
        
        
        for (int i = 0; i < buckets[index].size(); i++) {
            if (buckets[index][i].key == key) {
                buckets[index][i].value = value; 
                return;
            }
        }
        
        
        buckets[index].push_back(HashNode(key, value));
        currentSize++;
    }
    
    
    
    V* get(K key) {
        int index = hashFunction(key);
        
        for (int i = 0; i < buckets[index].size(); i++) {
            if (buckets[index][i].key == key) {
                return &(buckets[index][i].value);
            }
        }
        return nullptr; 
    }
    
    
    
    bool contains(K key) {
        return get(key) != nullptr;
    }
    
    
    
    bool remove(K key) {
        int index = hashFunction(key);
        
        for (int i = 0; i < buckets[index].size(); i++) {
            if (buckets[index][i].key == key) {
                
                if (i < buckets[index].size() - 1) {
                    buckets[index][i] = buckets[index][buckets[index].size() - 1];
                }
                buckets[index].pop_back();
                currentSize--;
                return true;
            }
        }
        return false;
    }
    
    int size() const {
        return currentSize;
    }
    
    bool empty() const {
        return currentSize == 0;
    }
    
    
    Vector<K> getAllKeys() {
        Vector<K> keys;
        for (int i = 0; i < capacity; i++) {
            for (int j = 0; j < buckets[i].size(); j++) {
                keys.push_back(buckets[i][j].key);
            }
        }
        return keys;
    }
};

#endif

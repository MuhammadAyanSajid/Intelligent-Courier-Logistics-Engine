#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <iostream>
#include <string>

using namespace std;

// Node Structure for Linked Lists, Stacks, Queues
template <typename T>
struct Node
{
    T data;
    Node *next;
    Node *prev; // For Doubly Linked List

    Node(T val) : data(val), next(nullptr), prev(nullptr) {}
};

// Custom Vector (Dynamic Array)
template <typename T>
class CustomVector
{
private:
    T *data;
    int capacity;
    int size;

    void resize(int newCapacity)
    {
        T *newData = new T[newCapacity];
        for (int i = 0; i < size; i++)
        {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    CustomVector() : capacity(2), size(0)
    {
        data = new T[capacity];
    }

    ~CustomVector()
    {
        if (data)
            delete[] data;
    }

    // Copy Constructor
    CustomVector(const CustomVector &other) : capacity(other.capacity), size(other.size)
    {
        data = new T[capacity];
        for (int i = 0; i < size; i++)
        {
            data[i] = other.data[i];
        }
    }

    // Assignment Operator
    CustomVector &operator=(const CustomVector &other)
    {
        if (this != &other)
        {
            delete[] data;
            capacity = other.capacity;
            size = other.size;
            data = new T[capacity];
            for (int i = 0; i < size; i++)
            {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(T val)
    {
        if (size == capacity)
        {
            resize(capacity * 2);
        }
        data[size++] = val;
    }

    void pop_back()
    {
        if (size > 0)
            size--;
    }

    T &operator[](int index)
    {
        return data[index];
    }

    const T &operator[](int index) const
    {
        return data[index];
    }

    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }

    // Safe get with bounds checking
    T &get(int index)
    {
        if (index < 0 || index >= size)
        {
            static T defaultVal;
            return defaultVal;
        }
        return data[index];
    }

    const T &get(int index) const
    {
        if (index < 0 || index >= size)
        {
            static T defaultVal;
            return defaultVal;
        }
        return data[index];
    }

    // Check if vector contains a value (simple linear search for small vectors)
    bool contains(const T &val) const
    {
        for (int i = 0; i < size; i++)
        {
            if (data[i] == val)
                return true;
        }
        return false;
    }

    // Clear all elements
    void clear()
    {
        size = 0;
    }
};

// Custom Doubly Linked List
template <typename T>
class CustomLinkedList
{
private:
    Node<T> *head;
    Node<T> *tail;
    int size;

public:
    CustomLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    ~CustomLinkedList()
    {
        clear();
    }

    // Copy Constructor
    CustomLinkedList(const CustomLinkedList &other) : head(nullptr), tail(nullptr), size(0)
    {
        Node<T> *curr = other.head;
        while (curr)
        {
            append(curr->data);
            curr = curr->next;
        }
    }

    // Assignment Operator
    CustomLinkedList &operator=(const CustomLinkedList &other)
    {
        if (this != &other)
        {
            clear();
            Node<T> *curr = other.head;
            while (curr)
            {
                append(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }

    void clear()
    {
        Node<T> *current = head;
        while (current)
        {
            Node<T> *next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        size = 0;
    }

    void append(T val)
    {
        Node<T> *newNode = new Node<T>(val);
        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    // Needed for iterating
    Node<T> *getHead() const { return head; }
    Node<T> *getTail() const { return tail; }
    int getSize() const { return size; }
    bool isEmpty() const { return size == 0; }

    // Find a value and return pointer to node (nullptr if not found)
    Node<T> *find(const T &val) const
    {
        Node<T> *curr = head;
        while (curr)
        {
            if (curr->data == val)
                return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    // Remove first occurrence of a value
    bool remove(const T &val)
    {
        Node<T> *curr = head;
        while (curr)
        {
            if (curr->data == val)
            {
                if (curr->prev)
                    curr->prev->next = curr->next;
                else
                    head = curr->next;

                if (curr->next)
                    curr->next->prev = curr->prev;
                else
                    tail = curr->prev;

                delete curr;
                size--;
                return true;
            }
            curr = curr->next;
        }
        return false;
    }

    // Prepend to front of list
    void prepend(T val)
    {
        Node<T> *newNode = new Node<T>(val);
        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }
};

// Custom Stack (LIFO)
template <typename T>
class CustomStack
{
private:
    Node<T> *top;
    int size;

public:
    CustomStack() : top(nullptr), size(0) {}

    // Copy Constructor
    CustomStack(const CustomStack &other) : top(nullptr), size(0)
    {
        // Need to copy in reverse order to maintain stack order
        CustomStack<T> temp;
        Node<T> *curr = other.top;
        while (curr)
        {
            temp.push(curr->data);
            curr = curr->next;
        }
        // Now reverse it
        while (!temp.isEmpty())
        {
            push(temp.pop());
        }
    }

    // Assignment Operator
    CustomStack &operator=(const CustomStack &other)
    {
        if (this != &other)
        {
            while (!isEmpty())
                pop();
            CustomStack<T> temp;
            Node<T> *curr = other.top;
            while (curr)
            {
                temp.push(curr->data);
                curr = curr->next;
            }
            while (!temp.isEmpty())
            {
                push(temp.pop());
            }
        }
        return *this;
    }

    ~CustomStack()
    {
        while (!isEmpty())
        {
            pop();
        }
    }

    int getSize() const { return size; }

    void push(T val)
    {
        Node<T> *newNode = new Node<T>(val);
        newNode->next = top;
        top = newNode;
        size++;
    }

    T pop()
    {
        if (isEmpty())
            return T();
        Node<T> *temp = top;
        T val = temp->data;
        top = top->next;
        delete temp;
        size--;
        return val;
    }

    T peek()
    {
        if (isEmpty())
            return T();
        return top->data;
    }

    bool isEmpty() const { return top == nullptr; }
};

// Custom Queue (FIFO)
template <typename T>
class CustomQueue
{
private:
    Node<T> *front;
    Node<T> *rear;
    int size;

public:
    CustomQueue() : front(nullptr), rear(nullptr), size(0) {}

    // Copy Constructor
    CustomQueue(const CustomQueue &other) : front(nullptr), rear(nullptr), size(0)
    {
        Node<T> *curr = other.front;
        while (curr)
        {
            enqueue(curr->data);
            curr = curr->next;
        }
    }

    // Assignment Operator
    CustomQueue &operator=(const CustomQueue &other)
    {
        if (this != &other)
        {
            while (!isEmpty())
                dequeue();
            Node<T> *curr = other.front;
            while (curr)
            {
                enqueue(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }

    ~CustomQueue()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }

    int getSize() const { return size; }

    T peek() const
    {
        if (isEmpty())
            return T();
        return front->data;
    }

    void enqueue(T val)
    {
        Node<T> *newNode = new Node<T>(val);
        if (isEmpty())
        {
            front = rear = newNode;
        }
        else
        {
            rear->next = newNode;
            newNode->prev = rear;
            rear = newNode;
        }
        size++;
    }

    T dequeue()
    {
        if (isEmpty())
            return T();
        Node<T> *temp = front;
        T val = temp->data;
        front = front->next;
        if (front)
            front->prev = nullptr;
        else
            rear = nullptr;
        delete temp;
        size--;
        return val;
    }

    bool isEmpty() const { return front == nullptr; }
};

// Min-Heap (Priority Queue)
template <typename T>
class MinHeap
{
private:
    CustomVector<T> heap;

    void heapifyUp(int index)
    {
        if (index == 0)
            return;
        int parentIndex = (index - 1) / 2;
        if (heap[index] < heap[parentIndex])
        {
            T temp = heap[index];
            heap[index] = heap[parentIndex];
            heap[parentIndex] = temp;
            heapifyUp(parentIndex);
        }
    }

    void heapifyDown(int index)
    {
        int leftChild = 2 * index + 1;
        int rightChild = 2 * index + 2;
        int smallest = index;

        if (leftChild < heap.getSize() && heap[leftChild] < heap[smallest])
        {
            smallest = leftChild;
        }
        if (rightChild < heap.getSize() && heap[rightChild] < heap[smallest])
        {
            smallest = rightChild;
        }

        if (smallest != index)
        {
            T temp = heap[index];
            heap[index] = heap[smallest];
            heap[smallest] = temp;
            heapifyDown(smallest);
        }
    }

public:
    MinHeap() {}

    void insert(T val)
    {
        heap.push_back(val);
        heapifyUp(heap.getSize() - 1);
    }

    T peek() const
    {
        if (isEmpty())
            return T();
        return heap[0];
    }

    T extractMin()
    {
        if (isEmpty())
            return T();
        T minVal = heap[0];
        T lastVal = heap[heap.getSize() - 1];
        heap[0] = lastVal;
        heap.pop_back();
        heapifyDown(0);
        return minVal;
    }

    bool isEmpty() const
    {
        return heap.getSize() == 0;
    }
};

// Hash Map (Simple Hashing)
template <typename K, typename V>
class HashMap
{
private:
    struct HashNode
    {
        K key;
        V value;
        HashNode *next;
        HashNode(K k, V v) : key(k), value(v), next(nullptr) {}
    };

    HashNode **table;
    int capacity;
    int size;

    int hashFunction(int key)
    {
        return key % capacity;
    }

    int hashFunction(string key)
    {
        int hash = 0;
        for (char c : key)
        {
            hash = (hash * 31 + c) % capacity;
        }
        return hash < 0 ? -hash : hash; // Handle potential negative
    }

public:
    HashMap(int cap = 100) : capacity(cap), size(0)
    {
        table = new HashNode *[capacity];
        for (int i = 0; i < capacity; i++)
            table[i] = nullptr;
    }

    ~HashMap()
    {
        for (int i = 0; i < capacity; i++)
        {
            HashNode *entry = table[i];
            while (entry)
            {
                HashNode *prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
        delete[] table;
    }

    void insert(K key, V value)
    {
        int idx = hashFunction(key);
        // Check if key exists and update
        HashNode *entry = table[idx];
        while (entry)
        {
            if (entry->key == key)
            {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        // Insert new
        HashNode *newNode = new HashNode(key, value);
        newNode->next = table[idx];
        table[idx] = newNode;
        size++;
    }

    V get(K key)
    { // Returns default V() if not found
        int idx = hashFunction(key);
        HashNode *entry = table[idx];
        while (entry)
        {
            if (entry->key == key)
            {
                return entry->value;
            }
            entry = entry->next;
        }
        return V();
    }

    bool contains(K key) const
    {
        int idx = const_cast<HashMap *>(this)->hashFunction(key);
        HashNode *entry = table[idx];
        while (entry)
        {
            if (entry->key == key)
                return true;
            entry = entry->next;
        }
        return false;
    }

    CustomVector<V> getAllValues() const
    {
        CustomVector<V> values;
        for (int i = 0; i < capacity; i++)
        {
            HashNode *entry = table[i];
            while (entry)
            {
                values.push_back(entry->value);
                entry = entry->next;
            }
        }
        return values;
    }

    // Remove a key from the HashMap
    bool remove(K key)
    {
        int idx = hashFunction(key);
        HashNode *entry = table[idx];
        HashNode *prev = nullptr;

        while (entry)
        {
            if (entry->key == key)
            {
                if (prev)
                {
                    prev->next = entry->next;
                }
                else
                {
                    table[idx] = entry->next;
                }
                delete entry;
                size--;
                return true;
            }
            prev = entry;
            entry = entry->next;
        }
        return false;
    }

    // Get all keys
    CustomVector<K> getAllKeys()
    {
        CustomVector<K> keys;
        for (int i = 0; i < capacity; i++)
        {
            HashNode *entry = table[i];
            while (entry)
            {
                keys.push_back(entry->key);
                entry = entry->next;
            }
        }
        return keys;
    }

    int getSize() const { return size; }
};

#endif // STRUCTURES_H

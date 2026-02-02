#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>

using namespace std;

// ==================== CONSTANTS & ENUMS ====================
enum RequestState { REQUESTED, ALLOCATED, OCCUPIED, RELEASED, CANCELLED };

const int MAX_ZONES = 10;
const int MAX_VEHICLES = 100;
const int MAX_REQUESTS = 500;
const int MAX_ROLLBACK = 100;

// ==================== UTILITY FUNCTIONS ====================
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pause() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void printLine() {
    cout << "================================================================\n";
}

int getInt(string prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Enter a number between " << min << " and " << max << "\n";
    }
}

string getString(string prompt) {
    string value;
    cout << prompt;
    cin >> value;
    return value;
}
// ==================== TIME STAMP ====================
class TimeStamp {
private:
    time_t timestamp;
    
public:
    TimeStamp() {
        timestamp = time(0);
    }
    
    float getHoursDiff(TimeStamp& other) {
        return abs(difftime(timestamp, other.timestamp)) / 3600.0f;
    }
    
    void display() {
        tm* ltm = localtime(&timestamp);
        cout << setfill('0') << setw(2) << ltm->tm_hour << ":"
             << setw(2) << ltm->tm_min << ":" << setw(2) << ltm->tm_sec;
        cout << setfill(' ');
    }
    
    string toString() {
        tm* ltm = localtime(&timestamp);
        char buffer[9];
        sprintf(buffer, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        return string(buffer);
    }
};
// ==================== PARKING SLOT ====================
class ParkingSlot {
private:
    int slotID, zoneID, areaID;
    bool available;
    int vehicleID;
    
public:
    ParkingSlot() : slotID(-1), zoneID(-1), areaID(-1), 
                    available(true), vehicleID(-1) {}
    
    void init(int s, int z, int a) {
        slotID = s;
        zoneID = z;
        areaID = a;
        available = true;
        vehicleID = -1;
    }
    
    bool isAvailable() { return available; }
    int getSlotID() { return slotID; }
    int getAreaID() { return areaID; }
    int getVehicleID() { return vehicleID; }
    
    void occupy(int vID) {
        available = false;
        vehicleID = vID;
    }
    
    void free() {
        available = true;
        vehicleID = -1;
    }
};

// ==================== PARKING AREA ====================
class ParkingArea {
private:
    int areaID, zoneID;
    ParkingSlot slots[20];
    int totalSlots, availableSlots;
    
public:
    ParkingArea() : areaID(-1), zoneID(-1), totalSlots(0), availableSlots(0) {}
    
    void init(int a, int z, int numSlots) {
        areaID = a;
        zoneID = z;
        totalSlots = numSlots;
        availableSlots = numSlots;
        
        for (int i = 0; i < totalSlots; i++) {
            slots[i].init(i, zoneID, areaID);
        }
    }
    
    ParkingSlot* findSlot() {
        for (int i = 0; i < totalSlots; i++) {
            if (slots[i].isAvailable()) {
                return &slots[i];
            }
        }
        return nullptr;
    }
    
    bool releaseSlot(int slotID) {
        if (slotID >= 0 && slotID < totalSlots && !slots[slotID].isAvailable()) {
            slots[slotID].free();
            availableSlots++;
            return true;
        }
        return false;
    }
    
    void occupySlot(int slotID, int vID) {
        if (slotID >= 0 && slotID < totalSlots) {
            slots[slotID].occupy(vID);
            availableSlots--;
        }
    }
    
    int getAvailable() { return availableSlots; }
    int getTotal() { return totalSlots; }
    int getAreaID() { return areaID; }
    
    void display() {
        cout << "  Area " << areaID << ": " << availableSlots << "/" << totalSlots << " slots available\n";
    }
};
// ==================== ZONE ====================
class Zone {
private:
    int zoneID;
    string name;
    ParkingArea areas[5];
    int areaCount;
    int adjacentZones[MAX_ZONES];
    int adjacentCount;
    int totalSlots, availableSlots;
    
public:
    Zone() : zoneID(-1), name(""), areaCount(0), adjacentCount(0),
             totalSlots(0), availableSlots(0) {}
    
    void init(int id, string n, int numAreas, int* areaCapacities) {
        zoneID = id;
        name = n;
        areaCount = numAreas;
        adjacentCount = 0;
        totalSlots = 0;
        availableSlots = 0;
        
        for (int i = 0; i < areaCount; i++) {
            areas[i].init(i, zoneID, areaCapacities[i]);
            totalSlots += areaCapacities[i];
            availableSlots += areaCapacities[i];
        }
    }
    
    void addAdjacent(int zID) {
        if (adjacentCount < MAX_ZONES) {
            adjacentZones[adjacentCount++] = zID;
        }
    }
    
    bool hasSlots() { return availableSlots > 0; }
    
    ParkingSlot* findSlot() {
        for (int i = 0; i < areaCount; i++) {
            ParkingSlot* slot = areas[i].findSlot();
            if (slot) return slot;
        }
        return nullptr;
    }
    
    void occupySlot(ParkingSlot* slot, int vID) {
        areas[slot->getAreaID()].occupySlot(slot->getSlotID(), vID);
        availableSlots--;
    }
    
    bool releaseSlot(int areaID, int slotID) {
        if (areaID >= 0 && areaID < areaCount) {
            if (areas[areaID].releaseSlot(slotID)) {
                availableSlots++;
                return true;
            }
        }
        return false;
    }
    
    int getID() { return zoneID; }
    string getName() { return name; }
    int getAvailable() { return availableSlots; }
    int getTotal() { return totalSlots; }
    int getAdjacentCount() { return adjacentCount; }
    int getAdjacent(int i) { return adjacentZones[i]; }
    
    float getOccupancyRate() {
        if (totalSlots == 0) return 0;
        return ((float)(totalSlots - availableSlots) / totalSlots) * 100;
    }
    
    void display() {
        cout << "Zone " << zoneID << ": " << name << " - " 
             << availableSlots << "/" << totalSlots << " slots available ("
             << fixed << setprecision(1) << getOccupancyRate() << "% occupied)\n";
        
        if (adjacentCount > 0) {
            cout << "  Adjacent Zones: ";
            for (int i = 0; i < adjacentCount; i++) {
                cout << adjacentZones[i];
                if (i < adjacentCount - 1) cout << ", ";
            }
            cout << "\n";
        }
    }
    
    void displayDetailed() {
        cout << "\n=== Zone " << zoneID << ": " << name << " ===\n";
        cout << "Total Capacity: " << totalSlots << " slots\n";
        cout << "Available: " << availableSlots << " slots\n";
        cout << "Occupancy Rate: " << fixed << setprecision(1) << getOccupancyRate() << "%\n";
        cout << "\nAreas:\n";
        for (int i = 0; i < areaCount; i++) {
            areas[i].display();
        }
        
        if (adjacentCount > 0) {
            cout << "\nAdjacent Zones: ";
            for (int i = 0; i < adjacentCount; i++) {
                cout << adjacentZones[i];
                if (i < adjacentCount - 1) cout << ", ";
            }
            cout << "\n";
        }
    }
};
// ==================== VEHICLE ====================
class Vehicle {
private:
    int id;
    string plate;
    int preferredZone;
    bool active;
    
public:
    Vehicle() : id(-1), plate(""), preferredZone(-1), active(false) {}
    
    void init(int i, string p, int z) {
        id = i;
        plate = p;
        preferredZone = z;
        active = true;
    }
    
    int getID() { return id; }
    string getPlate() { return plate; }
    int getPreferredZone() { return preferredZone; }
    bool isActive() { return active; }
    
    void display() {
        cout << "Vehicle ID: " << id << " | Plate: " << plate 
             << " | Preferred Zone: " << preferredZone << "\n";
    }
};

// ==================== PARKING REQUEST ====================
class ParkingRequest {
private:
    int requestID, vehicleID, requestedZone;
    int allocatedZone, allocatedArea, allocatedSlot;
    RequestState state;
    float penalty;
    TimeStamp requestTime, allocationTime, releaseTime;
    
public:
    ParkingRequest() : requestID(-1), vehicleID(-1), requestedZone(-1),
                       allocatedZone(-1), allocatedArea(-1), allocatedSlot(-1),
                       state(REQUESTED), penalty(0) {}
    
    void init(int rID, int vID, int zone) {
        requestID = rID;
        vehicleID = vID;
        requestedZone = zone;
        allocatedZone = -1;
        allocatedArea = -1;
        allocatedSlot = -1;
        state = REQUESTED;
        penalty = 0;
        requestTime = TimeStamp();
    }
    
    bool canTransition(RequestState newState) {
        if (state == REQUESTED && (newState == ALLOCATED || newState == CANCELLED)) return true;
        if (state == ALLOCATED && (newState == OCCUPIED || newState == CANCELLED)) return true;
        if (state == OCCUPIED && newState == RELEASED) return true;
        return false;
    }
    
    bool changeState(RequestState newState) {
        if (!canTransition(newState)) return false;
        
        if (newState == ALLOCATED) allocationTime = TimeStamp();
        if (newState == RELEASED || newState == CANCELLED) releaseTime = TimeStamp();
        
        state = newState;
        return true;
    }
    
    void setAllocation(int z, int a, int s, float p) {
        allocatedZone = z;
        allocatedArea = a;
        allocatedSlot = s;
        penalty = p;
    }
    
    int getRequestID() { return requestID; }
    int getVehicleID() { return vehicleID; }
    int getRequestedZone() { return requestedZone; }
    int getAllocatedZone() { return allocatedZone; }
    int getAllocatedArea() { return allocatedArea; }
    int getAllocatedSlot() { return allocatedSlot; }
    RequestState getState() { return state; }
    float getPenalty() { return penalty; }
    
    void setState(RequestState s) { state = s; }
    
    bool isCrossZone() { 
        return allocatedZone != requestedZone && allocatedZone != -1; 
    }
    
    float getDuration() {
        if (state == RELEASED) {
            return requestTime.getHoursDiff(releaseTime);
        }
        return 0;
    }
    
    string getStateString() {
        switch(state) {
            case REQUESTED: return "REQUESTED";
            case ALLOCATED: return "ALLOCATED";
            case OCCUPIED: return "OCCUPIED";
            case RELEASED: return "RELEASED";
            case CANCELLED: return "CANCELLED";
        }
        return "UNKNOWN";
    }
    
    void display() {
        printLine();
        cout << "Request ID: " << requestID << "\n";
        cout << "Vehicle ID: " << vehicleID << "\n";
        cout << "State: " << getStateString() << "\n";
        cout << "Requested Zone: " << requestedZone << "\n";
        
        if (allocatedSlot != -1) {
            cout << "Allocated Location: Zone " << allocatedZone 
                 << ", Area " << allocatedArea << ", Slot " << allocatedSlot;
            if (isCrossZone()) {
                cout << " (Cross-Zone Allocation)";
            }
            cout << "\n";
            if (penalty > 0) {
                cout << "Penalty: $" << fixed << setprecision(2) << penalty << "\n";
            }
        }
        
        if (state == RELEASED) {
            cout << "Parking Duration: " << fixed << setprecision(2) 
                 << getDuration() << " hours\n";
        }
        printLine();
    }
};

// ==================== REQUEST HISTORY (LINKED LIST) ====================
class RequestHistory {
private:
    struct Node {
        ParkingRequest req;
        Node* next;
        
        Node(ParkingRequest& r) : req(r), next(nullptr) {}
    };
    
    Node* head;
    int count;
    
public:
    RequestHistory() : head(nullptr), count(0) {}
    
    ~RequestHistory() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
    
    void add(ParkingRequest& req) {
        Node* newNode = new Node(req);
        newNode->next = head;
        head = newNode;
        count++;
    }
    
    void getStats(int& completed, int& cancelled, int& crossZone, float& avgDuration) {
        completed = 0;
        cancelled = 0;
        crossZone = 0;
        float totalDuration = 0;
        
        Node* curr = head;
        while (curr) {
            if (curr->req.getState() == RELEASED) {
                completed++;
                totalDuration += curr->req.getDuration();
                if (curr->req.isCrossZone()) crossZone++;
            } else if (curr->req.getState() == CANCELLED) {
                cancelled++;
            }
            curr = curr->next;
        }
        
        avgDuration = (completed > 0) ? totalDuration / completed : 0;
    }
    
    int getCount() { return count; }
};
// ==================== WAITING QUEUE ====================
class WaitingQueue {
private:
    struct Node {
        int vehicleID, zone;
        TimeStamp addedTime;
        Node* next;
        Node(int v, int z) : vehicleID(v), zone(z), next(nullptr) {}
    };
    
    Node* front;
    Node* rear;
    int size;
    
public:
    WaitingQueue() : front(nullptr), rear(nullptr), size(0) {}
    
    ~WaitingQueue() {
        while (front) {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }
    
    void enqueue(int vID, int zone) {
        Node* node = new Node(vID, zone);
        if (!rear) {
            front = rear = node;
        } else {
            rear->next = node;
            rear = node;
        }
        size++;
    }
    
    bool dequeue(int& vID, int& zone) {
        if (!front) return false;
        
        vID = front->vehicleID;
        zone = front->zone;
        
        Node* temp = front;
        front = front->next;
        if (!front) rear = nullptr;
        
        delete temp;
        size--;
        return true;
    }
    
    int getSize() { return size; }
    bool isEmpty() { return size == 0; }
    
    void display() {
        if (size == 0) {
            cout << "Waiting Queue: Empty\n";
            return;
        }
        
        cout << "Waiting Queue (" << size << " vehicles):\n";
        printLine();
        Node* curr = front;
        int pos = 1;
        while (curr) {
            cout << pos++ << ". Vehicle ID: " << curr->vehicleID 
                 << " | Zone: " << curr->zone 
                 << " | Added: " << curr->addedTime.toString() << "\n";
            curr = curr->next;
        }
    }
};

// ==================== ROLLBACK MANAGER (STACK) ====================
struct RollbackEntry {
    int requestID, zone, area, slot;
    RequestState prevState;
    TimeStamp timestamp;
    
    RollbackEntry() : requestID(-1), zone(-1), area(-1), 
                      slot(-1), prevState(REQUESTED) {}
    
    RollbackEntry(int r, int z, int a, int s, RequestState st) :
        requestID(r), zone(z), area(a), slot(s), prevState(st) {}
};

class RollbackManager {
private:
    RollbackEntry stack[MAX_ROLLBACK];
    int top;
    
public:
    RollbackManager() : top(-1) {}
    
    bool push(RollbackEntry& entry) {
        if (top >= MAX_ROLLBACK - 1) {
            cout << "Error: Rollback stack is full!\n";
            return false;
        }
        stack[++top] = entry;
        return true;
    }
    
    bool pop(RollbackEntry& entry) {
        if (top < 0) return false;
        entry = stack[top--];
        return true;
    }
    
    int getSize() { return top + 1; }
    bool isEmpty() { return top < 0; }
    
    void display() {
        if (isEmpty()) {
            cout << "Rollback Stack: Empty\n";
            return;
        }
        
        cout << "Rollback Stack (" << getSize() << " operations):\n";
        printLine();
        for (int i = top; i >= 0 && i > top - 10; i--) {
            cout << (top - i + 1) << ". Request " << stack[i].requestID 
                 << " | Zone " << stack[i].zone << ", Area " << stack[i].area 
                 << ", Slot " << stack[i].slot 
                 << " | Time: " << stack[i].timestamp.toString() << "\n";
        }
        if (top >= 10) {
            cout << "... and " << (top - 9) << " more operations\n";
        }
    }
};

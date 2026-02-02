#include "ParkingSystem.h"
#include "Constants.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std;

// Utility functions
void printLine() {
    cout << "================================================================\n";
}

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

// RequestHistory class (Linked List)
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

// WaitingQueue class
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

// ParkingSystem implementation
ParkingSystem::ParkingSystem() : zoneCount(0), vehicleCount(0), requestCount(0),
                                 completed(0), cancelled(0) {
    for (int i = 0; i < MAX_ZONES; i++) zoneUsage[i] = 0;
    history = new RequestHistory();
    waitQueue = new WaitingQueue();
}

ParkingSystem::~ParkingSystem() {
    delete history;
    delete waitQueue;
}

void ParkingSystem::initCity() {
    clearScreen();
    cout << "CITY PARKING SYSTEM INITIALIZATION\n";
    printLine();
    
    cout << "\nInitializing city infrastructure...\n";
    
    zoneCount = 5;
    
    int downtown[] = {10, 8, 6};
    int commercial[] = {12, 10, 8, 6};
    int residential[] = {10, 8, 6, 4};
    int industrial[] = {8, 6};
    int suburban[] = {10, 8, 6};
    
    zones[0].init(0, "Downtown", 3, downtown);
    zones[1].init(1, "Commercial", 4, commercial);
    zones[2].init(2, "Residential", 4, residential);
    zones[3].init(3, "Industrial", 2, industrial);
    zones[4].init(4, "Suburban", 3, suburban);
    
    // Set adjacencies
    zones[0].addAdjacent(1); zones[0].addAdjacent(2);
    zones[1].addAdjacent(0); zones[1].addAdjacent(2);
    zones[2].addAdjacent(0); zones[2].addAdjacent(1); zones[2].addAdjacent(3);
    zones[3].addAdjacent(2); zones[3].addAdjacent(4);
    zones[4].addAdjacent(3);
    
    cout << "\nCity initialized successfully with " << zoneCount << " zones!\n\n";
    printLine();
    cout << "ZONE OVERVIEW:\n";
    printLine();
    for (int i = 0; i < zoneCount; i++) {
        zones[i].display();
    }
    
    pause();
}

void ParkingSystem::registerVehicle() {
    clearScreen();
    cout << "VEHICLE REGISTRATION\n";
    printLine();
    
    if (vehicleCount >= MAX_VEHICLES) {
        cout << "Error: Maximum vehicle limit reached!\n";
        pause();
        return;
    }
    
    string plate = getString("Enter License Plate: ");
    
    cout << "\nAvailable Zones:\n";
    for (int i = 0; i < zoneCount; i++) {
        cout << i << ". " << zones[i].getName() 
             << " (" << zones[i].getAvailable() << " slots available)\n";
    }
    
    int zone = getInt("\nSelect Preferred Zone (0-" + to_string(zoneCount - 1) + "): ", 
                    0, zoneCount - 1);
    
    vehicles[vehicleCount].init(vehicleCount, plate, zone);
    vehicleCount++;
    
    cout << "\nVehicle registered successfully!\n";
    cout << "Vehicle ID: " << (vehicleCount - 1) << "\n";
    cout << "License Plate: " << plate << "\n";
    cout << "Preferred Zone: " << zones[zone].getName() << "\n";
    
    pause();
}

void ParkingSystem::requestParking() {
    clearScreen();
    cout << "REQUEST PARKING SLOT\n";
    printLine();
    
    if (vehicleCount == 0) {
        cout << "Error: No vehicles registered in the system!\n";
        cout << "Please register a vehicle first.\n";
        pause();
        return;
    }
    
    cout << "\nRegistered Vehicles:\n";
    for (int i = 0; i < vehicleCount; i++) {
        vehicles[i].display();
    }
    
    int vID = getInt("\nEnter Vehicle ID (0-" + to_string(vehicleCount-1) + "): ", 
                    0, vehicleCount - 1);
    
    // Check for active parking
    for (int i = 0; i < requestCount; i++) {
        if (requests[i].getVehicleID() == vID &&
            (requests[i].getState() == ALLOCATED || 
             requests[i].getState() == OCCUPIED)) {
            cout << "\nError: This vehicle already has an active parking allocation!\n";
            requests[i].display();
            pause();
            return;
        }
    }
    
    cout << "\nAvailable Zones:\n";
    for (int i = 0; i < zoneCount; i++) {
        cout << i << ". " << zones[i].getName() 
             << " - " << zones[i].getAvailable() << "/" << zones[i].getTotal() 
             << " slots available\n";
    }
    
    int zone = getInt("\nSelect Zone (0-" + to_string(zoneCount - 1) + "): ", 
                    0, zoneCount - 1);
    
    cout << "\nProcessing parking request...\n";
    
    // Create request
    requests[requestCount].init(requestCount, vID, zone);
    
    // Try to allocate
    int allocZone, allocArea, allocSlot;
    float penalty = 0;
    
    if (allocate(zone, vID, allocZone, allocArea, allocSlot, penalty)) {
        requests[requestCount].setAllocation(allocZone, allocArea, allocSlot, penalty);
        requests[requestCount].changeState(ALLOCATED);
        
        // Save for rollback
        RollbackEntry entry(requestCount, allocZone, allocArea, allocSlot, REQUESTED);
        rollbackMgr.push(entry);
        
        history->add(requests[requestCount]);
        zoneUsage[allocZone]++;
        
        cout << "\nParking slot allocated successfully!\n";
        requests[requestCount].display();
        requestCount++;
    } else {
        waitQueue->enqueue(vID, zone);
        cout << "\nNo parking available in requested or nearby zones.\n";
        cout << "Vehicle added to waiting queue.\n\n";
        waitQueue->display();
    }
    
    pause();
}

bool ParkingSystem::allocate(int reqZone, int vID, int& allocZone, int& allocArea, 
                            int& allocSlot, float& penalty) {
    return allocEngine.allocate(reqZone, vID, allocZone, allocArea, allocSlot, 
                                penalty, zones, zoneCount);
}

void ParkingSystem::changeState() {
    clearScreen();
    cout << "MANAGE REQUEST STATE\n";
    printLine();
    
    if (requestCount == 0) {
        cout << "No parking requests in the system!\n";
        pause();
        return;
    }
    
    cout << "\nRecent Requests:\n";
    for (int i = max(0, requestCount - 10); i < requestCount; i++) {
        cout << "Request " << requests[i].getRequestID() 
             << " | Vehicle " << requests[i].getVehicleID()
             << " | State: " << requests[i].getStateString() << "\n";
    }
    
    int rID = getInt("\nEnter Request ID (0-" + to_string(requestCount-1) + "): ",
                    0, requestCount - 1);
    
    cout << "\nCurrent Request Status:\n";
    requests[rID].display();
    
    cout << "\nState Transition Options:\n";
    cout << "1. Mark as OCCUPIED (vehicle has parked)\n";
    cout << "2. RELEASE parking (vehicle has left)\n";
    cout << "3. CANCEL request\n";
    cout << "0. Go back\n";
    
    int choice = getInt("\nSelect action (0-3): ", 0, 3);
    
    if (choice == 0) return;
    
    RequestState newState;
    if (choice == 1) newState = OCCUPIED;
    else if (choice == 2) newState = RELEASED;
    else newState = CANCELLED;
    
    if (requests[rID].changeState(newState)) {
        if (newState == RELEASED || newState == CANCELLED) {
            zones[requests[rID].getAllocatedZone()].releaseSlot(
                requests[rID].getAllocatedArea(),
                requests[rID].getAllocatedSlot()
            );
            
            if (newState == RELEASED) {
                completed++;
                cout << "\nParking released successfully!\n";
                cout << "Duration: " << fixed << setprecision(2) 
                     << requests[rID].getDuration() << " hours\n";
            } else {
                cancelled++;
                cout << "\nRequest cancelled successfully!\n";
            }
        } else {
            cout << "\nState changed to " << requests[rID].getStateString() << " successfully!\n";
        }
    } else {
        cout << "\nError: Invalid state transition!\n";
        cout << "Please check the current state and try again.\n";
    }
    
    pause();
}

void ParkingSystem::rollback() {
    clearScreen();
    cout << "ROLLBACK OPERATIONS\n";
    printLine();
    
    if (rollbackMgr.isEmpty()) {
        cout << "No operations available to rollback!\n";
        pause();
        return;
    }
    
    rollbackMgr.display();
    
    int k = getInt("\nHow many operations to rollback? (1-" + 
                  to_string(rollbackMgr.getSize()) + "): ", 
                  1, rollbackMgr.getSize());
    
    cout << "\nRolling back " << k << " operation(s)...\n\n";
    
    for (int i = 0; i < k; i++) {
        RollbackEntry entry;
        if (rollbackMgr.pop(entry)) {
            zones[entry.zone].releaseSlot(entry.area, entry.slot);
            requests[entry.requestID].setState(entry.prevState);
            cout << "Rolled back Request #" << entry.requestID << "\n";
        }
    }
    
    cout << "\nRollback completed successfully!\n";
    
    pause();
}

void ParkingSystem::showAnalytics() {
    clearScreen();
    cout << "SYSTEM ANALYTICS & STATISTICS\n";
    printLine();
    
    // Request Statistics
    cout << "\nREQUEST STATISTICS:\n";
    printLine();
    int active = requestCount - completed - cancelled;
    cout << "Total Requests: " << requestCount << "\n";
    cout << "Completed: " << completed << "\n";
    cout << "Cancelled: " << cancelled << "\n";
    cout << "Currently Active: " << active << "\n";
    
    // Zone Usage
    cout << "\nZONE UTILIZATION:\n";
    printLine();
    int maxUse = 0, peak = -1;
    for (int i = 0; i < zoneCount; i++) {
        cout << zones[i].getName() << ": " << zoneUsage[i] << " allocations";
        if (zoneUsage[i] > maxUse) {
            maxUse = zoneUsage[i];
            peak = i;
        }
        if (i == peak && maxUse > 0) {
            cout << " [PEAK ZONE]";
        }
        cout << "\n";
    }
    
    // Parking Statistics
    int comp, canc, cross;
    float avg;
    history->getStats(comp, canc, cross, avg);
    
    cout << "\nPARKING INSIGHTS:\n";
    printLine();
    cout << "Average Parking Duration: " << fixed << setprecision(2) << avg << " hours\n";
    cout << "Cross-Zone Allocations: " << cross;
    if (comp > 0) {
        cout << " (" << fixed << setprecision(1) << ((float)cross/comp * 100) << "%)";
    }
    cout << "\n";
    
    // System Status
    cout << "\nCURRENT SYSTEM STATUS:\n";
    printLine();
    cout << "Registered Vehicles: " << vehicleCount << "\n";
    cout << "Vehicles in Queue: " << waitQueue->getSize() << "\n";
    cout << "Rollback Stack Size: " << rollbackMgr.getSize() << "\n";
    
    // Zone Details
    cout << "\nZONE STATUS:\n";
    printLine();
    for (int i = 0; i < zoneCount; i++) {
        zones[i].display();
    }
    
    pause();
}

void ParkingSystem::showVehicles() {
    clearScreen();
    cout << "REGISTERED VEHICLES\n";
    printLine();
    
    if (vehicleCount == 0) {
        cout << "No vehicles registered in the system.\n";
        cout << "Use 'Register Vehicle' option to add vehicles.\n";
    } else {
        cout << "\nTotal Vehicles: " << vehicleCount << "\n\n";
        for (int i = 0; i < vehicleCount; i++) {
            vehicles[i].display();
        }
    }
    
    pause();
}

void ParkingSystem::showRequests() {
    clearScreen();
    cout << "ALL PARKING REQUESTS\n";
    printLine();
    
    if (requestCount == 0) {
        cout << "No parking requests in the system.\n";
        cout << "Use 'Request Parking' option to create requests.\n";
    } else {
        cout << "\nTotal Requests: " << requestCount << "\n";
        
        // Count by state
        int countByState[5] = {0};
        for (int i = 0; i < requestCount; i++) {
            countByState[requests[i].getState()]++;
        }
        
        cout << "\nRequests by State:\n";
        cout << "REQUESTED: " << countByState[REQUESTED] << "\n";
        cout << "ALLOCATED: " << countByState[ALLOCATED] << "\n";
        cout << "OCCUPIED: " << countByState[OCCUPIED] << "\n";
        cout << "RELEASED: " << countByState[RELEASED] << "\n";
        cout << "CANCELLED: " << countByState[CANCELLED] << "\n";
        
        cout << "\nAll Requests:\n";
        printLine();
        for (int i = 0; i < requestCount; i++) {
            requests[i].display();
        }
    }
    
    pause();
}

void ParkingSystem::showZoneDetails() {
    clearScreen();
    cout << "DETAILED ZONE VIEW\n";
    printLine();
    
    cout << "\nSelect a zone to view details:\n";
    for (int i = 0; i < zoneCount; i++) {
        cout << i << ". " << zones[i].getName() 
             << " (" << zones[i].getAvailable() << "/" << zones[i].getTotal() 
             << " slots available)\n";
    }
    
    int choice = getInt("\nSelect zone (0-" + to_string(zoneCount - 1) + "): ", 
                      0, zoneCount - 1);
    
    zones[choice].displayDetailed();
    
    pause();
}

void ParkingSystem::runDemo() {
    clearScreen();
    cout << "DEMO MODE - AUTOMATED DEMONSTRATION\n";
    printLine();
    
    cout << "\nThis will demonstrate the parking system features automatically.\n\n";
    
    cout << "Step 1: Registering 5 vehicles...\n";
    vehicles[vehicleCount++].init(0, "ABC-123", 0);
    vehicles[vehicleCount++].init(1, "XYZ-789", 1);
    vehicles[vehicleCount++].init(2, "DEF-456", 0);
    vehicles[vehicleCount++].init(3, "GHI-012", 2);
    vehicles[vehicleCount++].init(4, "JKL-345", 3);
    cout << "Registered 5 vehicles successfully.\n\n";
    
    cout << "Step 2: Creating parking requests...\n";
    for (int i = 0; i < 4; i++) {
        requests[requestCount].init(requestCount, i, i % zoneCount);
        int z, a, s;
        float p;
        if (allocate(i % zoneCount, i, z, a, s, p)) {
            requests[requestCount].setAllocation(z, a, s, p);
            requests[requestCount].changeState(ALLOCATED);
            RollbackEntry entry(requestCount, z, a, s, REQUESTED);
            rollbackMgr.push(entry);
            history->add(requests[requestCount]);
            zoneUsage[z]++;
            cout << "  Allocated parking for Vehicle #" << i << "\n";
            requestCount++;
        }
    }
    
    cout << "\nStep 3: Updating request states...\n";
    if (requestCount > 0) {
        requests[0].changeState(OCCUPIED);
        cout << "  Request #0 marked as OCCUPIED\n";
    }
    if (requestCount > 1) {
        requests[1].changeState(OCCUPIED);
        cout << "  Request #1 marked as OCCUPIED\n";
    }
    
    cout << "\nStep 4: Cancelling a request...\n";
    if (requestCount > 2) {
        requests[2].changeState(CANCELLED);
        zones[requests[2].getAllocatedZone()].releaseSlot(
            requests[2].getAllocatedArea(),
            requests[2].getAllocatedSlot()
        );
        cancelled++;
        cout << "  Request #2 cancelled\n";
    }
    
    cout << "\nStep 5: Releasing parking...\n";
    if (requestCount > 0) {
        requests[0].changeState(RELEASED);
        zones[requests[0].getAllocatedZone()].releaseSlot(
            requests[0].getAllocatedArea(),
            requests[0].getAllocatedSlot()
        );
        completed++;
        cout << "  Request #0 released (Duration: " << fixed << setprecision(2) 
             << requests[0].getDuration() << " hours)\n";
    }
    
    cout << "\nDemo Summary:\n";
    printLine();
    cout << "Requests Created: " << requestCount << "\n";
    cout << "Completed: " << completed << "\n";
    cout << "Cancelled: " << cancelled << "\n";
    cout << "Active: " << (requestCount - completed - cancelled) << "\n";
    
    cout << "\nDemo completed successfully!\n";
    
    pause();
}

void ParkingSystem::mainMenu() {
    int choice;
    
    do {
        clearScreen();
        cout << "SMART PARKING MANAGEMENT SYSTEM\n";
        printLine();
        
        cout << "\nSystem Status:\n";
        cout << "Vehicles: " << vehicleCount 
             << " | Requests: " << requestCount 
             << " | Queue: " << waitQueue->getSize() 
             << " | Stack: " << rollbackMgr.getSize() << "\n";
        printLine();
        
        cout << "\nMAIN MENU:\n";
        cout << "1. System Overview & Analytics\n";
        cout << "2. Register New Vehicle\n";
        cout << "3. Request Parking Slot\n";
        cout << "4. Manage Request State\n";
        cout << "5. Rollback Operations\n";
        cout << "6. View Zone Details\n";
        cout << "7. View All Vehicles\n";
        cout << "8. View All Requests\n";
        cout << "9. Run System Demo\n";
        cout << "0. Exit System\n";
        
        choice = getInt("\nEnter your choice (0-9): ", 0, 9);
        
        switch(choice) {
            case 1: showAnalytics(); break;
            case 2: registerVehicle(); break;
            case 3: requestParking(); break;
            case 4: changeState(); break;
            case 5: rollback(); break;
            case 6: showZoneDetails(); break;
            case 7: showVehicles(); break;
            case 8: showRequests(); break;
            case 9: runDemo(); break;
            case 0: 
                clearScreen();
                cout << "\nThank you for using Smart Parking Management System!\n";
                cout << "Goodbye!\n\n";
                break;
        }
        
    } while (choice != 0);
}
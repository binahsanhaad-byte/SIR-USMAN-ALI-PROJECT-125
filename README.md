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

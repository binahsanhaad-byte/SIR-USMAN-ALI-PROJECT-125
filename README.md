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

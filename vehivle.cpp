#include "Vehicle.h"

Vehicle::Vehicle() : id(-1), plate(""), preferredZone(-1), active(false) {}

void Vehicle::init(int i, std::string p, int z) {
    id = i;
    plate = p;
    preferredZone = z;
    active = true;
}

int Vehicle::getID() { return id; }
std::string Vehicle::getPlate() { return plate; }
int Vehicle::getPreferredZone() { return preferredZone; }
bool Vehicle::isActive() { return active; }

void Vehicle::display() {
    std::cout << "Vehicle ID: " << id << " | Plate: " << plate 
              << " | Preferred Zone: " << preferredZone << "\n";
}
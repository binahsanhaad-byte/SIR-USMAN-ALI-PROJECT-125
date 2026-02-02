#include "Zone.h"

Zone::Zone() : zoneID(-1), name(""), areaCount(0), adjacentCount(0),
               totalSlots(0), availableSlots(0) {}

void Zone::init(int id, std::string n, int numAreas, int* areaCapacities) {
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

void Zone::addAdjacent(int zID) {
    if (adjacentCount < MAX_ZONES) {
        adjacentZones[adjacentCount++] = zID;
    }
}

bool Zone::hasSlots() { return availableSlots > 0; }

ParkingSlot* Zone::findSlot() {
    for (int i = 0; i < areaCount; i++) {
        ParkingSlot* slot = areas[i].findSlot();
        if (slot) return slot;
    }
    return nullptr;
}

void Zone::occupySlot(ParkingSlot* slot, int vID) {
    areas[slot->getAreaID()].occupySlot(slot->getSlotID(), vID);
    availableSlots--;
}

bool Zone::releaseSlot(int areaID, int slotID) {
    if (areaID >= 0 && areaID < areaCount) {
        if (areas[areaID].releaseSlot(slotID)) {
            availableSlots++;
            return true;
        }
    }
    return false;
}

int Zone::getID() { return zoneID; }
std::string Zone::getName() { return name; }
int Zone::getAvailable() { return availableSlots; }
int Zone::getTotal() { return totalSlots; }
int Zone::getAdjacentCount() { return adjacentCount; }
int Zone::getAdjacent(int i) { return adjacentZones[i]; }

float Zone::getOccupancyRate() {
    if (totalSlots == 0) return 0;
    return ((float)(totalSlots - availableSlots) / totalSlots) * 100;
}

void Zone::display() {
    std::cout << "Zone " << zoneID << ": " << name << " - " 
              << availableSlots << "/" << totalSlots << " slots available ("
              << std::fixed << std::setprecision(1) << getOccupancyRate() << "% occupied)\n";
    
    if (adjacentCount > 0) {
        std::cout << "  Adjacent Zones: ";
        for (int i = 0; i < adjacentCount; i++) {
            std::cout << adjacentZones[i];
            if (i < adjacentCount - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

void Zone::displayDetailed() {
    std::cout << "\n=== Zone " << zoneID << ": " << name << " ===\n";
    std::cout << "Total Capacity: " << totalSlots << " slots\n";
    std::cout << "Available: " << availableSlots << " slots\n";
    std::cout << "Occupancy Rate: " << std::fixed << std::setprecision(1) << getOccupancyRate() << "%\n";
    std::cout << "\nAreas:\n";
    for (int i = 0; i < areaCount; i++) {
        areas[i].display();
    }
    
    if (adjacentCount > 0) {
        std::cout << "\nAdjacent Zones: ";
        for (int i = 0; i < adjacentCount; i++) {
            std::cout << adjacentZones[i];
            if (i < adjacentCount - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
}

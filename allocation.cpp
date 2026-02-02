#include "AllocationEngine.h"

bool AllocationEngine::allocate(int reqZone, int vID, int& allocZone, int& allocArea, 
                                int& allocSlot, float& penalty, Zone* zones, int zoneCount) {
    // Try requested zone first
    if (reqZone >= 0 && reqZone < zoneCount && zones[reqZone].hasSlots()) {
        ParkingSlot* slot = zones[reqZone].findSlot();
        if (slot) {
            allocZone = reqZone;
            allocArea = slot->getAreaID();
            allocSlot = slot->getSlotID();
            penalty = 0;
            zones[reqZone].occupySlot(slot, vID);
            return true;
        }
    }
    
    // Try adjacent zones with moderate penalty
    if (reqZone >= 0 && reqZone < zoneCount) {
        for (int i = 0; i < zones[reqZone].getAdjacentCount(); i++) {
            int adj = zones[reqZone].getAdjacent(i);
            if (adj >= 0 && adj < zoneCount && zones[adj].hasSlots()) {
                ParkingSlot* slot = zones[adj].findSlot();
                if (slot) {
                    allocZone = adj;
                    allocArea = slot->getAreaID();
                    allocSlot = slot->getSlotID();
                    penalty = 15.0;
                    zones[adj].occupySlot(slot, vID);
                    return true;
                }
            }
        }
    }
    
    // Try any available zone with higher penalty
    for (int i = 0; i < zoneCount; i++) {
        if (i != reqZone && zones[i].hasSlots()) {
            ParkingSlot* slot = zones[i].findSlot();
            if (slot) {
                allocZone = i;
                allocArea = slot->getAreaID();
                allocSlot = slot->getSlotID();
                penalty = 25.0;
                zones[i].occupySlot(slot, vID);
                return true;
            }
        }
    }
    
    return false;
}
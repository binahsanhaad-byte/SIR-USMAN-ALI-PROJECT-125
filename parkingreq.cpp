#include "ParkingRequest.h"

ParkingRequest::ParkingRequest() : requestID(-1), vehicleID(-1), requestedZone(-1),
                                   allocatedZone(-1), allocatedArea(-1), allocatedSlot(-1),
                                   state(REQUESTED), penalty(0) {}

void ParkingRequest::init(int rID, int vID, int zone) {
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

bool ParkingRequest::canTransition(RequestState newState) {
    if (state == REQUESTED && (newState == ALLOCATED || newState == CANCELLED)) return true;
    if (state == ALLOCATED && (newState == OCCUPIED || newState == CANCELLED)) return true;
    if (state == OCCUPIED && newState == RELEASED) return true;
    return false;
}

bool ParkingRequest::changeState(RequestState newState) {
    if (!canTransition(newState)) return false;
    
    if (newState == ALLOCATED) allocationTime = TimeStamp();
    if (newState == RELEASED || newState == CANCELLED) releaseTime = TimeStamp();
    
    state = newState;
    return true;
}

void ParkingRequest::setAllocation(int z, int a, int s, float p) {
    allocatedZone = z;
    allocatedArea = a;
    allocatedSlot = s;
    penalty = p;
}

int ParkingRequest::getRequestID() { return requestID; }
int ParkingRequest::getVehicleID() { return vehicleID; }
int ParkingRequest::getRequestedZone() { return requestedZone; }
int ParkingRequest::getAllocatedZone() { return allocatedZone; }
int ParkingRequest::getAllocatedArea() { return allocatedArea; }
int ParkingRequest::getAllocatedSlot() { return allocatedSlot; }
RequestState ParkingRequest::getState() { return state; }
float ParkingRequest::getPenalty() { return penalty; }

void ParkingRequest::setState(RequestState s) { state = s; }

bool ParkingRequest::isCrossZone() { 
    return allocatedZone != requestedZone && allocatedZone != -1; 
}

float ParkingRequest::getDuration() {
    if (state == RELEASED) {
        return requestTime.getHoursDiff(releaseTime);
    }
    return 0;
}

std::string ParkingRequest::getStateString() {
    switch(state) {
        case REQUESTED: return "REQUESTED";
        case ALLOCATED: return "ALLOCATED";
        case OCCUPIED: return "OCCUPIED";
        case RELEASED: return "RELEASED";
        case CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}

void ParkingRequest::display() {
    std::cout << "================================================================\n";
    std::cout << "Request ID: " << requestID << "\n";
    std::cout << "Vehicle ID: " << vehicleID << "\n";
    std::cout << "State: " << getStateString() << "\n";
    std::cout << "Requested Zone: " << requestedZone << "\n";
    
    if (allocatedSlot != -1) {
        std::cout << "Allocated Location: Zone " << allocatedZone 
                  << ", Area " << allocatedArea << ", Slot " << allocatedSlot;
        if (isCrossZone()) {
            std::cout << " (Cross-Zone Allocation)";
        }
        std::cout << "\n";
        if (penalty > 0) {
            std::cout << "Penalty: $" << std::fixed << std::setprecision(2) << penalty << "\n";
        }
    }
    
    if (state == RELEASED) {
        std::cout << "Parking Duration: " << std::fixed << std::setprecision(2) 
                  << getDuration() << " hours\n";
    }
    std::cout << "================================================================\n";
}
#include "RollbackManager.h"
#include <iostream>

RollbackEntry::RollbackEntry() : requestID(-1), zone(-1), area(-1), 
                                 slot(-1), prevState(REQUESTED) {}

RollbackEntry::RollbackEntry(int r, int z, int a, int s, RequestState st) :
    requestID(r), zone(z), area(a), slot(s), prevState(st) {}

RollbackManager::RollbackManager() : top(-1) {}

bool RollbackManager::push(RollbackEntry& entry) {
    if (top >= 100 - 1) {
        std::cout << "Error: Rollback stack is full!\n";
        return false;
    }
    stack[++top] = entry;
    return true;
}

bool RollbackManager::pop(RollbackEntry& entry) {
    if (top < 0) return false;
    entry = stack[top--];
    return true;
}

int RollbackManager::getSize() { return top + 1; }
bool RollbackManager::isEmpty() { return top < 0; }

void RollbackManager::display() {
    if (isEmpty()) {
        std::cout << "Rollback Stack: Empty\n";
        return;
    }
    
    std::cout << "Rollback Stack (" << getSize() << " operations):\n";
    std::cout << "================================================================\n";
    for (int i = top; i >= 0 && i > top - 10; i--) {
        std::cout << (top - i + 1) << ". Request " << stack[i].requestID 
                  << " | Zone " << stack[i].zone << ", Area " << stack[i].area 
                  << ", Slot " << stack[i].slot 
                  << " | Time: " << stack[i].timestamp.toString() << "\n";
    }
    if (top >= 10) {
        std::cout << "... and " << (top - 9) << " more operations\n";
    }
}
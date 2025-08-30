#include "SemaphoreHandler.h"

SemaphoreHandler::SemaphoreHandler() {
    semaphoreList = nullptr; // Initialize the semaphore list to null
    numSemaphores = 0; // Initialize the semaphore count to zero
}

bool SemaphoreHandler::takeSemaphore(String semaphoreName, TickType_t timeout) {
    //Find the semaphore by name
    SemaphoreNode* current = findSemaphoreByName(semaphoreName);
    if(current != nullptr && current->semaphore != nullptr) {
        // Try to take the semaphore with the specified timeout
        if(xSemaphoreTake(current->semaphore, timeout) == pdTRUE) {
            return true; // Semaphore taken successfully
        }
    }
    return false; // Semaphore not found or could not be taken
}

SemaphoreNode* SemaphoreHandler::findSemaphoreByName(String semaphoreName) {
    SemaphoreNode* current = semaphoreList;
    while(current != nullptr) {
        if(current->name.equals(semaphoreName)) 
            return current; // Return the semaphore node if found

        current = current->next; // Move to the next node
    }
    return nullptr; // Semaphore not found
}

bool SemaphoreHandler::releaseSemaphore(String semaphoreName) {
    //Find the semaphore by name
    SemaphoreNode* current = findSemaphoreByName(semaphoreName);
    if(current != nullptr && current->semaphore != nullptr) {
        // Release the semaphore
        xSemaphoreGive(current->semaphore);
        return true; // Semaphore released successfully
    }
    return false; // Semaphore not found or could not be released
}

void SemaphoreHandler::addSemaphore(String semaphoreName) {
    //Create new semaphore node and initialize it
    SemaphoreNode* newSemaphore = new SemaphoreNode;
    newSemaphore->semaphore = xSemaphoreCreateMutex();
    newSemaphore->name = semaphoreName;

    if(numSemaphores == 0) semaphoreList = newSemaphore; // If this is the first semaphore, set it as

    else {
        SemaphoreNode* current = semaphoreList;
        while(current->next != nullptr) {
            current = current->next; // Traverse to the end of the list
        }
        newSemaphore->next = nullptr; // Initialize the next pointer of the new task
        current->next = newSemaphore; // Add the new task to the end of the list
    }
    numSemaphores++; // Increment the task count
}

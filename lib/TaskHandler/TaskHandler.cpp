#include "TaskHandler.h"

TaskHandler::TaskHandler() {
    numTasks = 0;
    numSemaphores = 0;
    semaphoreList = nullptr; // Initialize the semaphore list to null
}

void TaskHandler::addTask(TaskFunction_t function, const char* taskName, uint32_t stackDepth, UBaseType_t priority, BaseType_t core) {
    //Create new task node and initialize it
    TaskNode* newTask = new TaskNode;
    newTask->function = function;
    newTask->name = taskName;
    newTask->stackDepth = stackDepth;
    newTask->priority = priority;
    newTask->core = core;


    if(numTasks == 0) taskList = newTask; // If this is the first task, set it as the head of the list

    else {
        //Now traverse to the end of the list and add the new task
        TaskNode* current = taskList;
        while(current->next != nullptr) {
            current = current->next; // Traverse to the end of the list
        }
        newTask->next = nullptr; // Initialize the next pointer of the new task
        current->next = newTask; // Add the new task to the end of the list
    }
    numTasks++; // Increment the task count
}

bool TaskHandler::startTask(String taskName) {
    //Find the task by name
    TaskNode* current = findTaskByName(taskName);
    if(current != nullptr) {
        if(current->task != nullptr) {
            vTaskResume(current->task); // Resume the task if it was suspended
            return true;
        }
        else {
            xTaskCreatePinnedToCore(
                current->function, // Task function
                current->name, // Name of the task
                current->stackDepth, // Stack size in bytes
                this, // Parameter passed to the task (not used here)
                current->priority, // Task priority
                &current->task, // Task handle to be created
                current->core // Core to run the task
            );
        }
        return true; // Task started successfully
    }
    return false; // Task not found
}

TaskNode* TaskHandler::findTaskByName(String taskName) {
    TaskNode* current = taskList;
    while(current != nullptr) {
        if(String(current->name).equals(taskName)) {
            return current; // Return the task node if found
        }
        current = current->next; // Move to the next node
    }
    return nullptr; // Task not found
}

bool TaskHandler::pauseTask(String taskName) {
    //Find the task by name
    TaskNode* current = findTaskByName(taskName);
    if(current != nullptr && current->task != nullptr) {
        vTaskSuspend(current->task); // Suspend the task
        return true; // Task paused successfully
    }
    return false; // Task not found or not running
}

bool TaskHandler::endTask(String taskName) {
    //Find the task by name
    TaskNode* current = findTaskByName(taskName);
    if(current != nullptr && current->task != nullptr) {
        vTaskDelete(current->task); // Delete the task
        current->task = nullptr; // Set the task handle to null
        return true; // Task ended successfully
    }
    return false; // Task not found or not running
}

bool TaskHandler::takeSemaphore(String semaphoreName, TickType_t timeout) {
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

SemaphoreNode* TaskHandler::findSemaphoreByName(String semaphoreName) {
    SemaphoreNode* current = semaphoreList;
    while(current != nullptr) {
        if(current->name.equals(semaphoreName)) 
            return current; // Return the semaphore node if found

        current = current->next; // Move to the next node
    }
    return nullptr; // Semaphore not found
}

bool TaskHandler::releaseSemaphore(String semaphoreName) {
    //Find the semaphore by name
    SemaphoreNode* current = findSemaphoreByName(semaphoreName);
    if(current != nullptr && current->semaphore != nullptr) {
        // Release the semaphore
        xSemaphoreGive(current->semaphore);
        return true; // Semaphore released successfully
    }
    return false; // Semaphore not found or could not be released
}

void TaskHandler::addSemaphore(String semaphoreName) {
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

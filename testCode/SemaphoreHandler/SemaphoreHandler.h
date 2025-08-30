#pragma once
#ifndef SEMAPHOREHANDLER_H
#define SEMAPHOREHANDLER_H
#include <Arduino.h>

//using linked list to store semaphores
struct SemaphoreNode {
    //Semaphore handle object
    SemaphoreHandle_t semaphore;
    //Semaphore Name
    String name;

    //next node
    struct SemaphoreNode* next = nullptr;
};

class SemaphoreHandler {
    public:
    SemaphoreHandler();
    void addSemaphore(String semaphoreName);
    bool takeSemaphore(String semaphoreName, TickType_t timeout = portMAX_DELAY);
    bool releaseSemaphore(String semaphoreName);

    private:
    SemaphoreNode* semaphoreList;
    SemaphoreNode* findSemaphoreByName(String semaphoreName);
    int numSemaphores = 0;
};

#endif // SEMAPHOREHANDLER_H
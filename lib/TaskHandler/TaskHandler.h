#pragma once
#ifndef TASKHANDLER_H
#define TASKHANDLER_H
#include <Arduino.h>

/*
Task Handler class allows for tasks to dynamically be created, started, paused, and ended.
Tasks can manage each other and take semaphores
Tasks will be provided with the class object so they can access the task handler
*/

//using linked list to store tasks
struct TaskNode {
    //task handle object (reference to task)
     TaskFunction_t function;
     const char* name;
     uint32_t stackDepth;
     UBaseType_t priority;
     BaseType_t core;

     TaskHandle_t task = nullptr;

    //next node
    struct TaskNode* next = nullptr;
};

//using linked list to store semaphores
struct SemaphoreNode {
    //Semaphore handle object
    SemaphoreHandle_t semaphore;
    //Semaphore Name
    String name;

    //next node
    struct SemaphoreNode* next = nullptr;
};



class TaskHandler {
public:
TaskHandler();

void addTask(TaskFunction_t function, const char* taskName, uint32_t stackDepth = 4096, UBaseType_t priority = 1, BaseType_t core = 0);
bool startTask(String taskName);
bool pauseTask(String taskName);
bool endTask(String taskName);


void addSemaphore(String semaphoreName);
bool takeSemaphore(String semaphoreName, TickType_t timeout = portMAX_DELAY);
bool releaseSemaphore(String semaphoreName);



private:
uint16_t numTasks;
uint16_t numSemaphores;
TaskNode* taskList;

TaskNode* findTaskByName(String taskName);

SemaphoreNode* semaphoreList;
SemaphoreNode* findSemaphoreByName(String semaphoreName);
};

#endif
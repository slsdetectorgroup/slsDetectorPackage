#pragma once

#include <semaphore.h>

typedef struct Memory {
    int version;
    sem_t sem;
#ifdef VIRTUAL
    int status;
    int stop;
#endif
} sharedMem;

char *getSharedMemoryError();
void printSharedMemory(sharedMem *shm);
int createSharedMemory(sharedMem **shm, int port);
void initializeSharedMemory(sharedMem *shm);
int openSharedMemory(sharedMem **shm, int port);
int attachSharedMemory(sharedMem **shm);
int detachSharedMemory(sharedMem **shm);
int removeSharedMemory();
void lockSharedMemory();
void unlockSharedMemory();

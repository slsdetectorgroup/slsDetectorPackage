#pragma once

#include <semaphore.h>

void sharedMemory_print();
int sharedMemory_create(int port);
void sharedMemory_initialize();
int sharedMemory_open(int port);
int sharedMemory_attach();
int sharedMemory_detach();
int sharedMemory_remove();
void sharedMemory_lock();
void sharedMemory_unlock();
#ifdef VIRTUAL
void sharedMemory_setStatus(int s);
int sharedMemory_getStatus();
void sharedMemory_setStop(int s);
int sharedMemory_getStop();
#endif
void sharedMemory_setScanStatus(int s);
int sharedMemory_getScanStatus();
void sharedMemory_setScanStop(int s);
int sharedMemory_getScanStop();
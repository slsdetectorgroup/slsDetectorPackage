#pragma once
#include "sls/sls_detector_defs.h"

void sharedMemory_print();
int sharedMemory_create(int port);
int sharedMemory_initialize();
int sharedMemory_open(int port);
int sharedMemory_attach();
int sharedMemory_detach();
int sharedMemory_remove();
void sharedMemory_lockStatus();
void sharedMemory_unlockStatus();
#ifdef VIRTUAL
void sharedMemory_setStatus(enum runStatus s);
enum runStatus sharedMemory_getStatus();
void sharedMemory_setStop(int s);
int sharedMemory_getStop();
#endif
void sharedMemory_setScanStatus(enum runStatus s);
enum runStatus sharedMemory_getScanStatus();
void sharedMemory_setScanStop(int s);
int sharedMemory_getScanStop();
#ifdef EIGERD
void sharedMemory_lockLocalLink();
void sharedMemory_unlockLocalLink();
#endif
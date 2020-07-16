#include "sharedMemory.h"
#include "clogger.h"

#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h> 

#define SHM_NAME    "sls_server_shared_memory"
#define SHM_VERSION 0x200625
#define SHM_KEY     5678
#define MEM_SIZE    128

typedef struct Memory {
    int version;
    pthread_mutex_t lock;
    enum runStatus scanStatus; // idle, running or error
    int scanStop;
#ifdef VIRTUAL
    enum runStatus status;
    int stop;
#endif
} sharedMem;

sharedMem *shm = NULL;
int shmFd = -1;

extern int isControlServer;

void sharedMemory_print() {
    LOG(logINFO, ("%s Shared Memory:\n", isControlServer ? "c" : "s"));
    LOG(logINFO,
        ("%s version:0x%x\n", isControlServer ? "c" : "s", shm->version));
    LOG(logINFO, ("%s scan status: %d\n", isControlServer ? "c" : "s",
                  (int)shm->scanStatus));
    LOG(logINFO,
        ("%s scan stop: %d\n", isControlServer ? "c" : "s", shm->scanStop));
#ifdef VIRTUAL
    LOG(logINFO,
        ("%s status: %d\n", isControlServer ? "c" : "s", (int)shm->status));
    LOG(logINFO, ("%s stop: %d\n", isControlServer ? "c" : "s", shm->stop));
#endif
}

int sharedMemory_create(int port) {
    // if shm existed, delete old shm and create again
    shmFd =
        shmget(SHM_KEY + port, MEM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmFd == -1 && errno == EEXIST) {
        // open existing one
        shmFd = shmget(SHM_KEY + port, MEM_SIZE,
                       IPC_CREAT | 0666);
        if (shmFd == -1) {
            LOG(logERROR, ("c: open existing shared memory (to delete) failed: %s\n", strerror(errno)));
            return FAIL;
        }
        // delete existing one
        sharedMemory_remove();
        LOG(logWARNING,
            ("Removed old shared memory with id 0x%x (%d)\n", SHM_KEY + port, SHM_KEY + port));
        
        // create it again with current structure
        shmFd = shmget(SHM_KEY + port, MEM_SIZE,
                       IPC_CREAT | IPC_EXCL | 0666);
    }
    if (shmFd == -1) {
        LOG(logERROR, ("Create shared memory failed: %s\n", strerror(errno)));
        return FAIL;
    }
    LOG(logINFO, ("Shared memory created with key 0x%x\n", SHM_KEY + port));
    if (sharedMemory_attach() == FAIL) {
        return FAIL;
    }
    if (sharedMemory_initialize() == FAIL) {
        return FAIL;
    }
    return OK;
}

int sharedMemory_initialize() {
    shm->version = SHM_VERSION;
    if (pthread_mutex_init(&(shm->lock), NULL) != 0) { 
        LOG(logERROR, ("Failed to initialize pthread lock for shared memory\n"));
        return FAIL;
    } 
    shm->scanStatus = IDLE;
    shm->scanStop = 0;
#ifdef VIRTUAL
    shm->status = IDLE;
    shm->stop = 0;
#endif
    LOG(logINFO, ("Shared memory initialized\n"))
    return OK;
}

int sharedMemory_open(int port) {
    shmFd = shmget(SHM_KEY + port, MEM_SIZE, 0666);
    if (shmFd == -1) {
        LOG(logERROR, ("Open shared memory failed: %s\n", strerror(errno)));
        return FAIL;
    }
    if (sharedMemory_attach() == FAIL) {
        return FAIL;
    }
    if (shm->version != SHM_VERSION) {
        LOG(logERROR,
            ("Shared memory version 0x%x does not match! (expected: 0x%x)\n",
             shm->version, SHM_VERSION));
    }
    LOG(logINFO, ("Shared memory opened\n"));
    return OK;
}

int sharedMemory_attach() {
    shm = (sharedMem *)shmat(shmFd, NULL, 0);
    if (shm == (void *)-1) {
        LOG(logERROR, ("could not attach: %s\n", strerror(errno)));
        return FAIL;
    }
    LOG(logINFO, ("Shared memory attached\n"));
    return OK;
}

int sharedMemory_detach() {
    if (shmdt(shm) == -1) {
        LOG(logERROR, ("could not detach: %s\n", strerror(errno)));
        return FAIL;
    }
    LOG(logINFO, ("Shared memory detached\n"));
    return OK;
}

int sharedMemory_remove() {
    if (shmctl(shmFd, IPC_RMID, NULL) == -1) {
        LOG(logERROR, ("could not remove: %s\n", strerror(errno)));
        return FAIL;
    }
    LOG(logINFO, ("Shared memory removed\n"));
    return OK;
}

void sharedMemory_lock() { pthread_mutex_lock(&(shm->lock)); }

void sharedMemory_unlock() { pthread_mutex_unlock(&(shm->lock)); }

#ifdef VIRTUAL
void sharedMemory_setStatus(enum runStatus s) {
    sharedMemory_lock();
    shm->status = s;
    sharedMemory_unlock();
}

enum runStatus sharedMemory_getStatus() {
    enum runStatus s = 0;
    sharedMemory_lock();
    s = shm->status;
    sharedMemory_unlock();
    return s;
}

void sharedMemory_setStop(int s) {
    sharedMemory_lock();
    shm->stop = s;
    sharedMemory_unlock();
}

int sharedMemory_getStop() {
    int s = 0;
    sharedMemory_lock();
    s = shm->stop;
    sharedMemory_unlock();
    return s;
}
#endif

void sharedMemory_setScanStatus(enum runStatus s) {
    sharedMemory_lock();
    shm->scanStatus = s;
    sharedMemory_unlock();
}

enum runStatus sharedMemory_getScanStatus() {
    enum runStatus s = IDLE;
    sharedMemory_lock();
    s = shm->scanStatus;
    sharedMemory_unlock();
    return s;
}

void sharedMemory_setScanStop(int s) {
    sharedMemory_lock();
    shm->scanStop = s;
    sharedMemory_unlock();
}

int sharedMemory_getScanStop() {
    int s = 0;
    sharedMemory_lock();
    s = shm->scanStop;
    sharedMemory_unlock();
    return s;
}
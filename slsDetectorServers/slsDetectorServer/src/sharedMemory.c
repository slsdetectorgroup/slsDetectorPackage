#include "sharedMemory.h"
#include "clogger.h"
#include "sls_detector_defs.h"

#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_NAME    "sls_server_shared_memory"
#define SHM_VERSION 0x200625
#define SHM_KEY     5678

typedef struct Memory {
    int version;
    sem_t sem;
    int scanStatus;
    int scanStop;
#ifdef VIRTUAL
    int status;
    int stop;
#endif
} sharedMem;

sharedMem *shm = NULL;
char shmMess[MAX_STR_LENGTH];
int shmFd = -1;

extern int isControlServer;

char *sharedMemory_getError() { return shmMess; }

void sharedMemory_print() {
    LOG(logINFO, ("%s Shared Memory:\n", isControlServer ? "c" : "s"));
    LOG(logINFO,
        ("%s version:0x%x\n", isControlServer ? "c" : "s", shm->version));
#ifdef VIRTUAL
    LOG(logINFO, ("%s status: %d\n", isControlServer ? "c" : "s", shm->status));
    LOG(logINFO, ("%s stop: %d\n", isControlServer ? "c" : "s", shm->stop));
#endif
}

int sharedMemory_create(int port) {
    memset(shmMess, 0, MAX_STR_LENGTH);

    // if sham existed, delete old shm and create again
    shmFd =
        shmget(SHM_KEY + port, sizeof(sharedMem), IPC_CREAT | IPC_EXCL | 0666);
    if (shmFd == -1 && errno == EEXIST) {
        LOG(logWARNING, ("Removing old shared memory\n"));
        // cuz of unknown previous shm size, it has to  be deleted like this
        system(
            "for i in seq `ipcs -m | cut -d ' ' -f1`; do ipcrm -M $i; done;");
        shmFd = shmget(SHM_KEY + port, sizeof(sharedMem),
                       IPC_CREAT | IPC_EXCL | 0666);
    }
    if (shmFd == -1) {
        sprintf(shmMess, "Create shared memory failed: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory created\n"));
    if (!sharedMemory_attach()) {
        return 0;
    }
    sharedMemory_initialize();
    return 1;
}

void sharedMemory_initialize() {
    shm->version = SHM_VERSION;
    sem_init(&(shm->sem), 1, 1);
#ifdef VIRTUAL
    shm->status = 0;
    shm->stop = 0;
#endif
    LOG(logINFO, ("Shared memory initialized\n"))
}

int sharedMemory_open(int port) {
    memset(shmMess, 0, MAX_STR_LENGTH);
    shmFd = shmget(SHM_KEY + port, sizeof(sharedMem), 0666);
    if (shmFd == -1) {
        sprintf(shmMess, "Open shared memory failed: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    if (!sharedMemory_attach()) {
        return 0;
    }
    if (shm->version != SHM_VERSION) {
        sprintf(shmMess,
                "Shared memory version 0x%x does not match! (expected: 0x%x)\n",
                shm->version, SHM_VERSION);
        LOG(logERROR, (shmMess));
    }
    LOG(logINFO, ("Shared memory opened\n"));
    return 1;
}

int sharedMemory_attach() {
    shm = (sharedMem *)shmat(shmFd, NULL, 0);
    if (shm == (void *)-1) {
        sprintf(shmMess, "could not attach: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory attached\n"));
    return 1;
}

int sharedMemory_detach() {
    memset(shmMess, 0, MAX_STR_LENGTH);
    if (shmdt(shm) == -1) {
        sprintf(shmMess, "could not detach: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory detached\n"));
    return 1;
}

int sharedMemory_remove() {
    memset(shmMess, 0, MAX_STR_LENGTH);
    if (shmctl(shmFd, IPC_RMID, NULL) == -1) {
        sprintf(shmMess, "could not remove: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory removed\n"));
    return 1;
}

void sharedMemory_lock() { sem_wait(&(shm->sem)); }

void sharedMemory_unlock() { sem_post(&(shm->sem)); }

#ifdef VIRTUAL
void sharedMemory_setStatus(int s) {
    sharedMemory_lock();
    shm->status = s;
    sharedMemory_unlock();
}

int sharedMemory_getStatus() {
    int s = 0;
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

void sharedMemory_setScanStatus(int s) {
    sharedMemory_lock();
    shm->scanStatus = s;
    sharedMemory_unlock();
}

int sharedMemory_getScanStatus() {
    int s = 0;
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
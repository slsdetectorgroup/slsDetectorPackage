#include "sharedMemory.h"
#include "clogger.h"
#include "sls_detector_defs.h"

#include <errno.h> // errno
#include <fcntl.h> // O_CREAT, O_TRUNC..
#include <string.h>
#include <sys/mman.h> // shared memory
#include <sys/stat.h> // fstat
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_NAME    "sls_server_shared_memory"
#define SHM_VERSION 0x200625
#define SHM_KEY     5678
char shmMess[MAX_STR_LENGTH];
int shmFd = -1;

extern int isControlServer;

char *getSharedMemoryError() { return shmMess; }

void printSharedMemory(sharedMem *shm) {
    LOG(logINFO, ("%s Shared Memory:\n", isControlServer ? "c" : "s"));
    LOG(logINFO,
        ("%s version:0x%x\n", isControlServer ? "c" : "s", shm->version));
#ifdef VIRTUAL
    LOG(logINFO, ("%s status: %d\n", isControlServer ? "c" : "s", shm->status));
    LOG(logINFO, ("%s stop: %d\n", isControlServer ? "c" : "s", shm->stop));
#endif
}

int createSharedMemory(sharedMem **shm, int port) {
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
    if (!attachSharedMemory(shm)) {
        return 0;
    }
    initializeSharedMemory(*shm);
    return 1;
}

void initializeSharedMemory(sharedMem *shm) {
    shm->version = SHM_VERSION;
    sem_init(&(shm->sem), 1, 1);
#ifdef VIRTUAL
    shm->status = 0;
    shm->stop = 0;
#endif
    LOG(logINFO, ("Shared memory initialized\n"))
}

int openSharedMemory(sharedMem **shm, int port) {
    memset(shmMess, 0, MAX_STR_LENGTH);
    shmFd = shmget(SHM_KEY + port, sizeof(sharedMem), 0666);
    if (shmFd == -1) {
        sprintf(shmMess, "Open shared memory failed: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    if (!attachSharedMemory(shm)) {
        return 0;
    }
    if ((*shm)->version != SHM_VERSION) {
        sprintf(shmMess,
                "Shared memory version 0x%x does not match! (expected: 0x%x)\n",
                (*shm)->version, SHM_VERSION);
        LOG(logERROR, (shmMess));
    }
    LOG(logINFO, ("Shared memory opened\n"));
    return 1;
}

int attachSharedMemory(sharedMem **shm) {
    *shm = (sharedMem *)shmat(shmFd, NULL, 0);
    if (*shm == (void *)-1) {
        sprintf(shmMess, "could not attach: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory attached\n"));
    return 1;
}

int detachSharedMemory(sharedMem **shm) {
    memset(shmMess, 0, MAX_STR_LENGTH);
    if (shmdt(*shm) == -1) {
        sprintf(shmMess, "could not detach: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory detached\n"));
    return 1;
}

int removeSharedMemory() {
    memset(shmMess, 0, MAX_STR_LENGTH);
    if (shmctl(shmFd, IPC_RMID, NULL) == -1) {
        sprintf(shmMess, "could not remove: %s\n", strerror(errno));
        LOG(logERROR, (shmMess));
        return 0;
    }
    LOG(logINFO, ("Shared memory removed\n"));
    return 1;
}

void lockSharedMemory(sharedMem *shm) { sem_wait(&(shm->sem)); }

void unlockSharedMemory(sharedMem *shm) { sem_post(&(shm->sem)); }
#ifdef VIRTUAL
#include "communication_virtual.h"
#include "clogger.h"

#include <string.h>
#include <unistd.h> // usleep

#define FILE_STATUS      "/tmp/sls_virtual_server_status_"
#define FILE_STOP        "/tmp/sls_virtual_server_stop_"
#define FD_STATUS        0
#define FD_STOP          1
#define FILE_NAME_LENGTH 1000

FILE *fd[2] = {NULL, NULL};
char fnameStatus[FILE_NAME_LENGTH];
char fnameStop[FILE_NAME_LENGTH];
int portNumber = 0;

int ComVirtual_createFiles(const int port) {
    portNumber = port;
    // control server writign status file
    memset(fnameStatus, 0, FILE_NAME_LENGTH);
    sprintf(fnameStatus, "%s%d", FILE_STATUS, port);
    FILE *fd = NULL;
    if (NULL == (fd = fopen(fnameStatus, "w"))) {
        LOG(logERROR, ("Could not open the file %s for virtual communication\n",
                       fnameStatus));
        return 0;
    }
    fclose(fd);
    LOG(logINFOBLUE, ("Created status file %s\n", fnameStatus));

    // stop server writing stop file
    memset(fnameStop, 0, FILE_NAME_LENGTH);
    sprintf(fnameStop, "%s%d", FILE_STOP, port);
    if (NULL == (fd = fopen(fnameStop, "w"))) {
        LOG(logERROR, ("Could not open the file %s for virtual communication\n",
                       fnameStop));
        return 0;
    }
    fclose(fd);
    LOG(logINFOBLUE, ("Created stop file %s\n", fnameStop));

    return 1;
}

void ComVirtual_setFileNames(const int port) {
    portNumber = port;
    memset(fnameStatus, 0, FILE_NAME_LENGTH);
    memset(fnameStop, 0, FILE_NAME_LENGTH);
    sprintf(fnameStatus, "%s%d", FILE_STATUS, port);
    sprintf(fnameStop, "%s%d", FILE_STOP, port);
}

void ComVirtual_setStatus(int value) {
    while (!ComVirtual_writeToFile(value, fnameStatus, "Control")) {
        usleep(100);
    }
}

int ComVirtual_getStatus() {
    int retval = 0;
    while (!ComVirtual_readFromFile(&retval, fnameStatus, "Stop")) {
        usleep(100);
    }
    return retval;
}

void ComVirtual_setStop(int value) {
    while (!ComVirtual_writeToFile(value, fnameStop, "Stop")) {
        usleep(100);
    }
}

int ComVirtual_getStop() {
    int retval = 0;
    while (!ComVirtual_readFromFile(&retval, fnameStop, "Control")) {
        usleep(100);
    }
    return retval;
}

int ComVirtual_writeToFile(int value, const char *fname,
                           const char *serverName) {
    FILE *fd = NULL;
    if (NULL == (fd = fopen(fname, "w"))) {
        LOG(logERROR, ("Vritual %s Server [%d] could not open "
                       "the file %s for writing\n",
                       serverName, portNumber, fname));
        return 0;
    }
    while (fwrite(&value, sizeof(value), 1, fd) < 1) {
        LOG(logERROR, ("Vritual %s Server [%d] could not write "
                       "to file %s\n",
                       serverName, portNumber, fname));
        return 0;
    }
    fclose(fd);
    return 1;
}

int ComVirtual_readFromFile(int *value, const char *fname,
                            const char *serverName) {
    FILE *fd = NULL;
    if (NULL == (fd = fopen(fname, "r"))) {
        LOG(logERROR, ("Vritual %s Server [%d] could not open "
                       "the file %s for reading\n",
                       serverName, portNumber, fname));
        return 0;
    }
    while (fread(value, sizeof(int), 1, fd) < 1) {
        LOG(logERROR, ("Vritual %s Server [%d] could not read "
                       "from file %s\n",
                       serverName, portNumber, fname));
        return 0;
    }
    fclose(fd);
    return 1;
}

#endif
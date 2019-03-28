#include "ClientInterface.h"
#include "ClientSocket.h"

ClientInterface::ClientInterface(sls::ClientSocket *socket, int n) : socket_(socket){}

void ClientInterface::Client_Receive(int &ret, char *mess, void *retval, int sizeOfRetval) {
    // get result of operation
    socket_->receiveData(reinterpret_cast<char *>(&ret), sizeof(ret));

    bool unrecognizedFunction = false;
    if (ret == FAIL) {
        bool created = false;
        // allocate mess if null
        if (!mess) {
            created = true;
            mess = new char[MAX_STR_LENGTH];
            memset(mess, 0, MAX_STR_LENGTH);
        }
        // get error message
        socket_->receiveData(mess, MAX_STR_LENGTH);
        // cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);

        // unrecognized function, do not ask for retval
        if (strstr(mess, "Unrecognized Function") != nullptr)
            unrecognizedFunction = true;
        // delete allocated mess
        if (created)
            delete[] mess;
    }
    // get retval
    if (!unrecognizedFunction)
        socket_->receiveData(reinterpret_cast<char *>(retval), sizeOfRetval);
}

int ClientInterface::Client_Send(int fnum, void *args, int sizeOfArgs, void *retval,
                                 int sizeOfRetval, char *mess) {
    int ret = FAIL;
    socket_->sendData(reinterpret_cast<char *>(&fnum), sizeof(fnum));
    socket_->sendData(reinterpret_cast<char *>(args), sizeOfArgs);
    Client_Receive(ret, mess, retval, sizeOfRetval);
    return ret;
}
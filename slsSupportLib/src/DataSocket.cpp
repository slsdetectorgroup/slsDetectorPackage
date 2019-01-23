
#include "DataSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

namespace sls {

DataSocket::DataSocket(int socketId) : socketId_(socketId) {}

size_t DataSocket::receiveData(char *buffer, size_t size) {
    std::cout << "Sending\n";
    size_t dataRead = 0;
    while (dataRead < size) {
        dataRead += read(getSocketId(), buffer + dataRead, size - dataRead);
    }
    return dataRead;
}

size_t DataSocket::sendData(char *buffer, size_t size) {
    std::cout << "Receiving\n";
    size_t dataSent = 0;
    while (dataSent < size) {
        dataSent += write(getSocketId(), buffer + dataSent, size - dataSent);
    }
    return dataSent;
}

void DataSocket::close() {
    if (socketId_ > 0) {
        ::close(socketId_);
    } else {
        throw std::runtime_error("Socket ERROR: close called on bad socket\n");
    }
}

} // namespace sls

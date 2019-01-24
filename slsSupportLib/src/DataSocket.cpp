
#include "DataSocket.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <algorithm>
namespace sls {

DataSocket::DataSocket(int socketId) : socketId_(socketId) {}

void DataSocket::swap(DataSocket& other) noexcept{
    std::swap(socketId_, other.socketId_);
}

DataSocket::DataSocket(DataSocket&& move) noexcept{
    move.swap(*this);
}
DataSocket& DataSocket::operator=(DataSocket&& move)noexcept{
    move.swap(*this);
    return *this;
}

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

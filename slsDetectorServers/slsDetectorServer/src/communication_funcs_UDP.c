// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "communication_funcs_UDP.h"
#include "clogger.h"
#include "sls/sls_detector_defs.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int udpSockfd[MAX_UDP_DESTINATION][2] = {};
struct addrinfo *udpServerAddrInfo[MAX_UDP_DESTINATION][2] = {};
unsigned short int udpDestinationPort[MAX_UDP_DESTINATION][2] = {};
char udpDestinationIp[MAX_UDP_DESTINATION][2][INET_ADDRSTRLEN] = {};
extern int numUdpDestinations;

void setupUDPCommParameters() {
    for (int i = 0; i != MAX_UDP_DESTINATION; i++) {
        udpSockfd[i][0] = -1;
        udpSockfd[i][1] = -1;
    }
    memset(udpServerAddrInfo, 0, sizeof(udpServerAddrInfo));
    memset(udpDestinationIp, 0, sizeof(udpDestinationIp));
}

int getUdPSocketDescriptor(int iRxEntry, int index) {
    return udpSockfd[iRxEntry][index];
}

int setUDPDestinationDetails(int iRxEntry, int index, const char *ip,
                             unsigned short int port) {
    LOG(logDEBUG1,
        ("Setting udp destination details for socket %d [iRxEntry:%d]\n", index,
         iRxEntry));
    udpDestinationPort[iRxEntry][index] = port;
    size_t len = strlen(ip);
    memset(udpDestinationIp[iRxEntry][index], 0, INET_ADDRSTRLEN);
    strncpy(udpDestinationIp[iRxEntry][index], ip,
            len > INET_ADDRSTRLEN ? INET_ADDRSTRLEN : len);

    if (udpServerAddrInfo[iRxEntry][index]) {
        freeaddrinfo(udpServerAddrInfo[iRxEntry][index]);
        udpServerAddrInfo[iRxEntry][index] = 0;
    }

    // convert ip to internet address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    char sport[100];
    memset(sport, 0, 100);
    sprintf(sport, "%d", udpDestinationPort[iRxEntry][index]);
    int err = getaddrinfo(udpDestinationIp[iRxEntry][index], sport, &hints,
                          &udpServerAddrInfo[iRxEntry][index]);
    if (err != 0) {
        LOG(logERROR, ("Failed to resolve remote socket address %s at port %d "
                       "[entry:%d]. "
                       "(Error code:%d, %s)\n",
                       udpDestinationIp[iRxEntry][index],
                       udpDestinationPort[iRxEntry][index], iRxEntry, err,
                       gai_strerror(err)));
        return FAIL;
    }
    if (udpServerAddrInfo[iRxEntry][index] == NULL) {
        LOG(logERROR,
            ("Failed to resolve remote socket address %s at port %d [entry:%d]."
             "(getaddrinfo returned NULL)\n",
             udpDestinationIp[iRxEntry][index],
             udpDestinationPort[iRxEntry][index], iRxEntry));
        udpServerAddrInfo[iRxEntry][index] = 0;
        return FAIL;
    }

    return OK;
}

int createUDPSocket(int index) {

    for (int iRxEntry = 0; iRxEntry != numUdpDestinations; ++iRxEntry) {

        LOG(logDEBUG2,
            ("Creating UDP Socket %d [entry:%d]\n", index, iRxEntry));
        if (!strlen(udpDestinationIp[iRxEntry][index])) {
            LOG(logERROR,
                ("No destination UDP ip specified for socket %d  [entry:%d].\n",
                 index, iRxEntry));
            return FAIL;
        }

        if (udpSockfd[iRxEntry][index] != -1) {
            LOG(logERROR, ("Strange that Udp socket was still open [socket:%d, "
                           "entry:%d]. Closing it to "
                           "create a new one.\n",
                           index, iRxEntry));
            close(udpSockfd[iRxEntry][index]);
            udpSockfd[iRxEntry][index] = -1;
        }

        // Creating socket file descriptor
        udpSockfd[iRxEntry][index] =
            socket(udpServerAddrInfo[iRxEntry][index]->ai_family,
                   udpServerAddrInfo[iRxEntry][index]->ai_socktype,
                   udpServerAddrInfo[iRxEntry][index]->ai_protocol);
        if (udpSockfd[iRxEntry][index] == -1) {
            LOG(logERROR, ("UDP socket at port %d failed [entry:%d]. (Error "
                           "code:%d, %s)\n",
                           udpDestinationPort[iRxEntry][index], iRxEntry, errno,
                           gai_strerror(errno)));
            return FAIL;
        }
        LOG(logINFO, ("Udp client socket created for server (entry:%d, port "
                      "%d, ip:%s)\n",
                      iRxEntry, udpDestinationPort[iRxEntry][index],
                      udpDestinationIp[iRxEntry][index]));

        // Using connect expects that the receiver (udp server) exists to listen
        // to these packets connecting allows to use "send/write" instead of
        // "sendto", avoiding checking for server address for each packet using
        // write without a connect will end in segv
        LOG(logINFO, ("Udp client socket connected [%d, %d, %s]\n", iRxEntry,
                      udpDestinationPort[iRxEntry][index],
                      udpDestinationIp[iRxEntry][index]));
    }
    return OK;
}

int sendUDPPacket(int iRxEntry, int index, const char *buf, int length) {
    int n = sendto(udpSockfd[iRxEntry][index], buf, length, 0,
                   udpServerAddrInfo[iRxEntry][index]->ai_addr,
                   udpServerAddrInfo[iRxEntry][index]->ai_addrlen);
    // udp sends atomically, no need to handle partial data
    if (n == -1) {
        LOG(logERROR, ("Could not send udp packet for socket %d [entry:%d]. "
                       "(Error code:%d, %s)\n",
                       index, iRxEntry, errno, gai_strerror(errno)));
    } else {
        LOG(logDEBUG2, ("%d bytes sent\n", n));
    }
    return n;
}

void closeUDPSocket(int index) {
    for (int iRxEntry = 0; iRxEntry != numUdpDestinations; ++iRxEntry) {
        if (udpSockfd[iRxEntry][index] != -1) {
            LOG(logINFO, ("Udp client socket closed\n"));
            close(udpSockfd[iRxEntry][index]);
            udpSockfd[iRxEntry][index] = -1;
        }
    }
}

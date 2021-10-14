// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

void setupUDPCommParameters();

int getUdPSocketDescriptor(int iRxEntry, int index);

void setNumberOfUDPDestinations(int value);

int setUDPDestinationDetails(int iRxEntry, int index, const char *ip,
                             unsigned short int port);
int createUDPSocket(int index);

int sendUDPPacket(int iRxEntry, int index, const char *buf, int length);

void closeUDPSocket(int index);

#pragma once
#ifdef VIRTUAL
// communciate between control and  stop server

int ComVirtual_createFiles(const int port);
void ComVirtual_setFileNames(const int port);
int ComVirtual_writeStatus(int value);
int ComVirtual_readStatus(int* value);
int ComVirtual_writeStop(int value);
int ComVirtual_readStop(int* value);
int ComVirtual_writeToFile(int value, const char* fname, const char* serverName);
int ComVirtual_readFromFile(int* value, const char* fname, const char* serverName);

#endif

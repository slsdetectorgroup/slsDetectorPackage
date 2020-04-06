#pragma once
#ifdef VIRTUAL
// communciate between control and  stop server

int ComVirtual_createFiles(const int port);
void ComVirtual_setFileNames(const int port);
void ComVirtual_setStatus(int value);
int ComVirtual_getStatus();
void ComVirtual_setStop(int value);
int ComVirtual_getStop();
int ComVirtual_writeToFile(int value, const char* fname, const char* serverName);
int ComVirtual_readFromFile(int* value, const char* fname, const char* serverName);

#endif

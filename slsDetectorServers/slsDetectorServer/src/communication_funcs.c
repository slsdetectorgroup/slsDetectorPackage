#include "communication_funcs.h" 
#include "clogger.h"

#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/select.h>
#include <unistd.h>



#define SEND_REC_MAX_SIZE 4096
#define DEFAULT_PORTNO    1952
#define DEFAULT_BACKLOG 5

//  blackfin limits
#define CPU_DRVR_SND_LMT   (30000) // rough limit
#define CPU_RSND_PCKT_LOOP (10)
#define CPU_RSND_WAIT_US   (1)


// Global variables  from errno.h
extern int errno;

// Variables that will be exported
int lockStatus = 0;
char lastClientIP[INET_ADDRSTRLEN] = "";
char thisClientIP[INET_ADDRSTRLEN] = "";
int differentClients = 0;
int isControlServer = 1;
int ret = FAIL;
int fnum = 0;
char mess[MAX_STR_LENGTH];

// Local variables
char dummyClientIP[INET_ADDRSTRLEN] = "";
int myport = -1;
// socket descriptor set
fd_set readset, tempset;
// number of socket descrptor listening to
int isock = 0;
// value of socket descriptor,
//becomes max value of socket descriptor (listen) and file descriptor (accept)
int maxfd = 0;


int bindSocket(unsigned short int port_number) {
	ret = FAIL;
	int socketDescriptor = -1;
	int i = 0;
	struct sockaddr_in addressS;

	// same port
	if (myport == port_number) {
		sprintf(mess, "Cannot create %s socket with port %d. Already in use before.\n",
				(isControlServer ? "control":"stop"), port_number);
		FILE_LOG(logERROR, (mess));
	}
	// port ok
	else {

		// create socket
		socketDescriptor = socket(AF_INET, SOCK_STREAM,0);
			// socket error
		if (socketDescriptor < 0) {
			sprintf(mess, "Cannot create %s socket with port %d\n",
					(isControlServer ? "control":"stop"), port_number);
			FILE_LOG(logERROR, (mess));
		}
		// socket success
		else {
			i = 1;
			// set port reusable
			setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
			// Set some fields in the serverAddress structure
			addressS.sin_family = AF_INET;
			addressS.sin_addr.s_addr = htonl(INADDR_ANY);
			addressS.sin_port = htons(port_number);

			// bind socket error
			if(bind(socketDescriptor,(struct sockaddr *) &addressS,sizeof(addressS)) < 0){
				sprintf(mess, "Cannot bind %s socket to port %d.\n",
						(isControlServer ? "control":"stop"), port_number);
				FILE_LOG(logERROR, (mess));
			}
			// bind socket ok
			else {

				// listen to socket
				if (listen(socketDescriptor, DEFAULT_BACKLOG) == 0) {
					// clear set of descriptors. set of descriptors needed?
					if (isock == 0) {
						FD_ZERO(&readset);
					}
					// add a socket descriptor from listen
					FD_SET(socketDescriptor, &readset);
					isock++;
					maxfd = socketDescriptor;
					// success
					myport = port_number;
					ret = OK;
					FILE_LOG(logDEBUG1, ("%s socket bound: isock=%d, port=%d, fd=%d\n",
							(isControlServer ? "Control":"Stop"), isock, port_number, socketDescriptor));

				}
				// listen socket error
				else {
					sprintf(mess, "Cannot bind %s socket to port %d.\n",
							(isControlServer ? "control":"stop"), port_number);
					FILE_LOG(logERROR, (mess));
				}
			}
		}
	}

	return socketDescriptor;
}



int acceptConnection(int socketDescriptor) {
	int j;
	struct sockaddr_in addressC;
	int file_des = -1;
	struct timeval tv;
	socklen_t address_length = sizeof(struct sockaddr_in);

	if (socketDescriptor < 0)
		return -1;

	// copy file descriptor set temporarily
	memcpy(&tempset, &readset, sizeof(tempset));

	// set time out as 2777.77 hours?
	tv.tv_sec = 10000000;
	tv.tv_usec = 0;

	// monitor file descrptors
	int result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

	// timeout
	if (result == 0) {
		FILE_LOG(logDEBUG3, ("%s socket select() timed out!\n",
				(isControlServer ? "control":"stop"), myport));
	}

	// error (not signal caught)
	else if (result < 0 && errno != EINTR) {
		FILE_LOG(logERROR, ("%s socket select() error: %s\n",
				(isControlServer ? "control":"stop"), myport, strerror(errno)));
	}

	// activity in descriptor set
	else if (result > 0) {
		FILE_LOG(logDEBUG3, ("%s select returned!\n", (isControlServer ? "control":"stop")));

		// loop through the file descriptor set
		for (j = 0; j < maxfd + 1; ++j) {

			// checks if file descriptor part of set
			if (FD_ISSET(j, &tempset)) {
				FILE_LOG(logDEBUG3, ("fd %d is set\n",j));

				// clear the temporary set
				FD_CLR(j, &tempset);

				// accept connection (if error)
				if ((file_des = accept(j,(struct sockaddr *) &addressC, &address_length)) < 0) {
					FILE_LOG(logERROR, ("%s socket accept() error. Connection refused.\n",
							"Error Number: %d, Message: %s\n",
							(isControlServer ? "control":"stop"),
							myport, errno, strerror(errno)));
					switch(errno) {
					case EWOULDBLOCK:
						FILE_LOG(logERROR, ("ewouldblock eagain"));
						break;
					case EBADF:
						FILE_LOG(logERROR, ("ebadf\n"));
						break;
					case ECONNABORTED:
						FILE_LOG(logERROR, ("econnaborted\n"));
						break;
					case EFAULT:
						FILE_LOG(logERROR, ("efault\n"));
						break;
					case EINTR:
						FILE_LOG(logERROR, ("eintr\n"));
						break;
					case EINVAL:
						FILE_LOG(logERROR, ("einval\n"));
						break;
					case EMFILE:
						FILE_LOG(logERROR, ("emfile\n"));
						break;
					case ENFILE:
						FILE_LOG(logERROR, ("enfile\n"));
						break;
					case ENOTSOCK:
						FILE_LOG(logERROR, ("enotsock\n"));
						break;
					case EOPNOTSUPP:
						FILE_LOG(logERROR, ("eOPNOTSUPP\n"));
						break;
					case ENOBUFS:
						FILE_LOG(logERROR, ("ENOBUFS\n"));
						break;
					case ENOMEM:
						FILE_LOG(logERROR, ("ENOMEM\n"));
						break;
					case ENOSR:
						FILE_LOG(logERROR, ("ENOSR\n"));
						break;
					case EPROTO:
						FILE_LOG(logERROR, ("EPROTO\n"));
						break;
					default:
						FILE_LOG(logERROR, ("unknown error\n"));
					}
				}
				// accept success
				else {
					inet_ntop(AF_INET, &(addressC.sin_addr), dummyClientIP, INET_ADDRSTRLEN);
					FILE_LOG(logDEBUG3, ("%s socket accepted connection, fd= %d\n",
							(isControlServer ? "control":"stop"), file_des));
					// add the file descriptor from accept
					FD_SET(file_des, &readset);
					maxfd = (maxfd < file_des)?file_des:maxfd;
				}
			}
		}
	}
	return file_des;
}







void closeConnection(int file_des) {
	if(file_des >= 0)
		close(file_des);
	// remove file descriptor from set
	FD_CLR(file_des, &readset);
}

void exitServer(int socketDescriptor) {
	if (socketDescriptor >= 0)
		close(socketDescriptor);
	FILE_LOG(logINFO, ("Closing %s server\n", (isControlServer ? "control":"stop")));
	FD_CLR(socketDescriptor, &readset);
	isock--;
	fflush(stdout);
}




void swapData(void* val,int length,intType itype){
	int i;
	int16_t* c = (int16_t*)val;
	int32_t* a = (int32_t*)val;
	int64_t* b = (int64_t*)val;
	for(i = 0; length > 0; i++){
		switch(itype){
		case INT16:
			c[i] = ((c[i] & 0x00FF) << 8) | ((c[i] & 0xFF00) >> 8);
			length -= sizeof(int16_t);
			break;
		case INT32:
			a[i] = ((a[i] << 8) & 0xFF00FF00) | ((a[i] >> 8) & 0xFF00FF );
			a[i] = (a[i] << 16) 			  | ((a[i] >> 16) & 0xFFFF);
			length -= sizeof(int32_t);
			break;
		case INT64:
			b[i] = ((b[i] << 8) & 0xFF00FF00FF00FF00ULL ) | ((b[i] >> 8) & 0x00FF00FF00FF00FFULL );
			b[i] = ((b[i] << 16) & 0xFFFF0000FFFF0000ULL ) | ((b[i] >> 16) & 0x0000FFFF0000FFFFULL );
			b[i] =  (b[i] << 32) | ((b[i] >> 32) & 0xFFFFFFFFULL);
			length -= sizeof(int64_t);
			break;
		default:
			length = 0;
			break;
		}
	}
}

int sendData(int file_des, void* buf,int length, intType itype){
#ifndef PCCOMPILE
#ifdef EIGERD
	swapData(buf, length, itype);
#endif
#endif
	return sendDataOnly(file_des, buf, length);
}


int receiveData(int file_des, void* buf,int length, intType itype){
	int lret = receiveDataOnly(file_des, buf, length);
#ifndef PCCOMPILE
#ifdef EIGERD
	if (lret >= 0) swapData(buf, length, itype);
#endif
#endif
	return lret;
}


int sendDataOnly(int file_des, void* buf,int length) {
     if (!length)
         return 0;


      int bytesSent = 0;
      int retry = 0; // retry index when buffer is blocked (write returns 0)
      while (bytesSent < length) {

          // setting a max packet size for blackfin driver (and network driver does not do a check if packets sent)
          int bytesToSend = length - bytesSent;
          if (bytesToSend > CPU_DRVR_SND_LMT)
              bytesToSend = CPU_DRVR_SND_LMT;

          // send
          int rc = write(file_des, (char*)((char*)buf + bytesSent), bytesToSend);
          // error
          if (rc < 0) {
              FILE_LOG(logERROR, ("Could not write to %s socket. Possible socket crash\n",
                              (isControlServer ? "control":"stop")));
              return bytesSent;
          }
          // also error, wrote nothing, buffer blocked up, too fast sending for client
          if (rc == 0) {
              FILE_LOG(logERROR, ("Could not write to %s socket. Buffer full. Retry: %d\n",
                              (isControlServer ? "control":"stop"), retry));
              ++retry;
              // wrote nothing for many loops
              if (retry >= CPU_RSND_PCKT_LOOP) {
                  FILE_LOG(logERROR, ("Could not write to %s socket. Buffer full! Too fast! No more.\n",
                                  (isControlServer ? "control":"stop")));
                  return bytesSent;
              }
              usleep(CPU_RSND_WAIT_US);
          }
          // wrote something, reset retry
          else  {
              retry = 0;
              if (rc != bytesToSend) {
                  FILE_LOG(logWARNING, ("Only partial write to %s socket. Expected to write %d bytes, wrote %d\n",
                                                (isControlServer ? "control":"stop"), bytesToSend, rc));
              }
          }
          bytesSent += rc;
      }

      return bytesSent;
}


int receiveDataOnly(int file_des, void* buf,int length) {

	int total_received = 0;
	int nreceiving;
	int nreceived;
	if (file_des<0) return -1;
	FILE_LOG(logDEBUG3, ("want to receive %d Bytes to %s server\n",
			length, (isControlServer ? "control":"stop")));

	while(length > 0) {
		nreceiving = (length>SEND_REC_MAX_SIZE) ? SEND_REC_MAX_SIZE:length; // (condition) ? if_true : if_false
		nreceived = read(file_des,(char*)buf+total_received,nreceiving);
		if(!nreceived){
			if(!total_received) {
				return -1; //to handle it
			}
			break;
		}
		length -= nreceived;
		total_received += nreceived;
	}

	if (total_received>0)
		strcpy(thisClientIP,dummyClientIP);

	if (strcmp(lastClientIP,thisClientIP)) {
		differentClients = 1;
	}
	else
		differentClients = 0;

	return total_received;
}




int sendModule(int file_des, sls_detector_module *myMod) {
	int ts = 0, n = 0;
	n = sendData(file_des,&(myMod->serialnumber),sizeof(myMod->serialnumber),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->nchan), sizeof(myMod->nchan), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->nchip), sizeof(myMod->nchip), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->ndac), sizeof(myMod->ndac), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->reg), sizeof(myMod->reg), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->iodelay), sizeof(myMod->iodelay), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->tau), sizeof(myMod->tau), INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->eV), sizeof(myMod->eV), INT32);
	if (!n)	return -1; ts += n;
	// dacs
	n = sendData(file_des,myMod->dacs, sizeof(int)*(myMod->ndac), INT32);
	if (!n)	return -1; ts += n;
	// channels
#ifdef EIGERD
	n = sendData(file_des,myMod->chanregs, sizeof(int) * (myMod->nchan), INT32);
	if (!n)	return -1; ts += n;
#endif
	FILE_LOG(logDEBUG1, ("module of size %d sent register %x\n", ts, myMod->reg));
	return ts;
}



int  receiveModule(int file_des, sls_detector_module* myMod) {
    enum TLogLevel level = logDEBUG1;
    FILE_LOG(level, ("Receiving Module\n"));
	int ts = 0, n = 0;
	int nDacs = myMod->ndac;
#ifdef EIGERD
	int nChans = myMod->nchan; // can be zero for no trimbits
	FILE_LOG(level, ("nChans: %d\n",nChans));
#endif
	n = receiveData(file_des,&(myMod->serialnumber), sizeof(myMod->serialnumber), INT32);
	if (!n)	return -1; ts += n;
	FILE_LOG(level, ("serialno received. %d bytes. serialno: %d\n", n, myMod->serialnumber));
	n = receiveData(file_des,&(myMod->nchan), sizeof(myMod->nchan), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("nchan received. %d bytes. nchan: %d\n", n, myMod->nchan));
	n = receiveData(file_des,&(myMod->nchip), sizeof(myMod->nchip), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("nchip received. %d bytes. nchip: %d\n", n, myMod->nchip));
	n = receiveData(file_des,&(myMod->ndac), sizeof(myMod->ndac), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("ndac received. %d bytes. ndac: %d\n", n, myMod->ndac));
	n = receiveData(file_des,&(myMod->reg), sizeof(myMod->reg), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("reg received. %d bytes. reg: %d\n", n, myMod->reg));
	n = receiveData(file_des,&(myMod->iodelay), sizeof(myMod->iodelay), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("iodelay received. %d bytes. iodelay: %d\n", n, myMod->iodelay));
	n = receiveData(file_des,&(myMod->tau), sizeof(myMod->tau), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("tau received. %d bytes. tau: %d\n", n, myMod->tau));
	n = receiveData(file_des,&(myMod->eV), sizeof(myMod->eV), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("eV received. %d bytes. eV: %d\n", n, myMod->eV));
	// dacs
	if (nDacs != (myMod->ndac)) {
		FILE_LOG(logERROR, ("received wrong number of dacs. "
				"Expected %d, got %d\n", nDacs, myMod->ndac));
		return 0;
	}
	n = receiveData(file_des, myMod->dacs, sizeof(int) * (myMod->ndac), INT32);
	if (!n)	return -1; ts += n;
    FILE_LOG(level, ("dacs received. %d bytes.\n", n));
	// channels
#ifdef EIGERD
	if (((myMod->nchan) != 0 ) &&  // no trimbits
			(nChans != (myMod->nchan))) { // with trimbits
		FILE_LOG(logERROR, ("received wrong number of channels. "
				"Expected %d, got %d\n",	nChans, (myMod->nchan)));
		return 0;
	}
	n = receiveData(file_des, myMod->chanregs, sizeof(int) * (myMod->nchan), INT32);
    FILE_LOG(level, ("chanregs received. %d bytes.\n", n));
    if (!n) return -1; ts += n;
#endif
	FILE_LOG(level, ("received module of size %d register %x\n",ts,myMod->reg));
	return ts;
}


void Server_LockedError() {
	ret = FAIL;
	sprintf(mess,"Detector locked by %s\n", lastClientIP);
	FILE_LOG(logWARNING, (mess));
}


int Server_VerifyLock() {
	if (differentClients && lockStatus)
		Server_LockedError();
	return ret;
}


int Server_SendResult(int fileDes, intType itype, int update, void* retval, int retvalSize) {

	// update if different clients (ret can be ok or acquisition finished), not fail to not overwrite e message
	if (update && isControlServer && ret != FAIL && differentClients)
		ret = FORCE_UPDATE;

	// send success of operation
	int ret1 = ret;
	sendData(fileDes, &ret1,sizeof(ret1), INT32);
	if(ret == FAIL) {
		// send error message
		if (strlen(mess))
			sendData(fileDes, mess, MAX_STR_LENGTH, OTHER);
		// debugging feature. should not happen.
		else
			FILE_LOG(logERROR, ("No error message provided for this failure in %s "
					"server. Will mess up TCP.\n",
					(isControlServer ? "control":"stop")));
	}
	// send return value
	sendData(fileDes, retval, retvalSize, itype);

	return ret;
}


void getMacAddressinString(char* cmac, int size, uint64_t mac) {
	memset(cmac, 0, size);
	sprintf(cmac,"%02x:%02x:%02x:%02x:%02x:%02x",
		(unsigned int)((mac>>40)&0xFF),
		(unsigned int)((mac>>32)&0xFF),
		(unsigned int)((mac>>24)&0xFF),
		(unsigned int)((mac>>16)&0xFF),
		(unsigned int)((mac>>8)&0xFF),
		(unsigned int)((mac>>0)&0xFF));
}

void getIpAddressinString(char* cip, uint32_t ip) {
	memset(cip, 0, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &ip, cip, INET_ADDRSTRLEN);
}

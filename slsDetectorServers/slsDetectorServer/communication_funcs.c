

#include "communication_funcs.h" 
#include "logger.h"

//#include <netinet/tcp.h> /* for TCP_NODELAY */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

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
const int send_rec_max_size = SEND_REC_MAX_SIZE;
int myport = -1;
// socket descriptor set
fd_set readset, tempset;
// number of socket descrptor listening to
int isock = 0;
// value of socket descriptor,
//becomes max value of socket descriptor (listen) and file descriptor (accept)
int maxfd = 0;




#define DEFAULT_BACKLOG 5

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
					FILE_LOG(logDEBUG5, ("%s socket bound: isock=%d, port=%d, fd=%d\n",
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
		FILE_LOG(logDEBUG5, ("%s socket select() timed out!\n",
				(isControlServer ? "control":"stop"), myport));
	}

	// error (not signal caught)
	else if (result < 0 && errno != EINTR) {
		FILE_LOG(logERROR, ("%s socket select() error: %s\n",
				(isControlServer ? "control":"stop"), myport, strerror(errno)));
	}

	// activity in descriptor set
	else if (result > 0) {
		FILE_LOG(logDEBUG5, ("%s select returned!\n", (isControlServer ? "control":"stop")));

		// loop through the file descriptor set
		for (j = 0; j < maxfd + 1; ++j) {

			// checks if file descriptor part of set
			if (FD_ISSET(j, &tempset)) {
				FILE_LOG(logDEBUG5, ("fd %d is set\n",j));

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
					FILE_LOG(logDEBUG5, ("%s socket accepted connection, fd= %d\n",
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
	FILE_LOG(logDEBUG5, ("Closing %s server\n", (isControlServer ? "control":"stop")));
	FD_CLR(socketDescriptor, &readset);
	isock--;
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
	int lret =  write(file_des, buf, length); //value of -1 is other end socket crash as sigpipe is ignored
	if (lret < 0) {
		FILE_LOG(logERROR, ("Could not write to %s socket. Possible socket crash\n",
				(isControlServer ? "control":"stop")));
	}
	return lret;
}


int receiveDataOnly(int file_des, void* buf,int length) {

	int total_received = 0;
	int nreceiving;
	int nreceived;
	if (file_des<0) return -1;
	FILE_LOG(logDEBUG5, ("want to receive %d Bytes to %s server\n",
			length, (isControlServer ? "control":"stop")));

	while(length > 0) {
		nreceiving = (length>send_rec_max_size) ? send_rec_max_size:length; // (condition) ? if_true : if_false
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

	if (strcmp(lastClientIP,thisClientIP))
		differentClients = 1;
	else
		differentClients = 0;

	return total_received;
}




int sendModule(int file_des, sls_detector_module *myMod) {
	return sendModuleGeneral(file_des, myMod, 1);
}


int sendModuleGeneral(int file_des, sls_detector_module *myMod, int sendAll) {
	int ts = 0, n = 0;
	int nChips = myMod->nchip;
	int nChans = myMod->nchan;
	int nAdcs = myMod->nadc;
	int nDacs = myMod->ndac;

	// send module structure
	n = sendData(file_des,&(myMod->serialnumber),sizeof(myMod->serialnumber),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->nchan),sizeof(myMod->nchan),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->nchip),sizeof(myMod->nchip),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->ndac),sizeof(myMod->ndac),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->nadc),sizeof(myMod->nadc),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->reg),sizeof(myMod->reg),INT32);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->gain), sizeof(myMod->gain),OTHER);
	if (!n)	return -1; ts += n;
	n = sendData(file_des,&(myMod->offset), sizeof(myMod->offset),OTHER);
	if (!n)	return -1; ts += n;
	FILE_LOG(logDEBUG5, ("module of size %d sent\n",ts));

	// send dac
	n = sendData(file_des,myMod->dacs,sizeof(int)*nDacs,INT32);
	if (!n)	return -1; ts += n;
	FILE_LOG(logDEBUG5, ("dacs of size %d sent\n",ts));
	{
		int idac;
		for (idac = 0; idac < nDacs; idac++)
			FILE_LOG(logDEBUG5, ("dac %d is %d\n",idac,(int)myMod->dacs[idac]));
	}

	// send adc
	n = sendData(file_des,myMod->adcs,sizeof(int)*nAdcs,INT32);
	if (!n)	return -1; ts += n;
	FILE_LOG(logDEBUG5, ("adcs of size %d sent\n", ts));

	// some detectors dont require sending all trimbits etc.
	if(sendAll) {
		// chips
		n = sendData(file_des,myMod->chipregs,sizeof(int)*nChips,INT32);
		if (!n)	return -1; ts += n;
		FILE_LOG(logDEBUG5, ("chips of size %d sent\n", ts));

		// channels
		n = sendData(file_des,myMod->chanregs,sizeof(int)*nChans,INT32);
		FILE_LOG(logDEBUG5, ("chans of size %d sent - %d\n", ts, myMod->nchan));
		if (!n)	return -1; ts += n;
	}

	FILE_LOG(logDEBUG5, ("module of size %d sent register %x\n", ts, myMod->reg));
	return ts;
}



int  receiveModule(int file_des, sls_detector_module* myMod) {
	return receiveModuleGeneral(file_des,myMod,1);
}

int  receiveModuleGeneral(int file_des, sls_detector_module* myMod, int receiveAll) {
	int ts = 0, n = 0;
	int *dacptr = myMod->dacs;
	int *adcptr = myMod->adcs;
	int *chipptr = myMod->chipregs, *chanptr = myMod->chanregs;
	int nChips, nchipold = myMod->nchip, nchipdiff;
	int nChans, nchanold = myMod->nchan, nchandiff;
	int nDacs, ndold = myMod->ndac, ndacdiff;
	int nAdcs, naold = myMod->nadc, nadcdiff;
	n = receiveData(file_des,&(myMod->serialnumber),sizeof(myMod->serialnumber),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->nchan),sizeof(myMod->nchan),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->nchip),sizeof(myMod->nchip),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->ndac),sizeof(myMod->ndac),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->nadc),sizeof(myMod->nadc),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->reg),sizeof(myMod->reg),INT32);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->gain), sizeof(myMod->gain),OTHER);
	if (!n)	return -1; ts += n;
	n = receiveData(file_des,&(myMod->offset), sizeof(myMod->offset),OTHER);
	if (!n)	return -1; ts += n;

	myMod->dacs = dacptr;
	myMod->adcs = adcptr;
	myMod->chipregs = chipptr;
	myMod->chanregs = chanptr;

#ifdef EIGERD
//exclude sending of trimbtis, nchips = 0,nchans = 0 in that case
	if(myMod->nchip == 0 && myMod->nchan == 0) {
		receiveAll = 0;
		nchipold = 0;
		nchanold = 0;
	}
#endif


	nChips = myMod->nchip;
	nchipdiff = nChips-nchipold;
	if (nchipold != nChips) {
		FILE_LOG(logERROR, ("received wrong number of chips\n"));
	}
	else
		FILE_LOG(logDEBUG5, ("received %d chips\n",nChips));

	nChans = myMod->nchan;
	nchandiff = nChans-nchanold;
	if (nchanold != nChans) {
		FILE_LOG(logERROR, ("received wrong number of channels\n"));
	}
	else
		FILE_LOG(logDEBUG5, ("received %d chans\n",nChans));


	nDacs = myMod->ndac;
	ndacdiff = nDacs-ndold;
	if (ndold != nDacs) {
		FILE_LOG(logERROR, ("received wrong number of dacs\n"));
	}
	else
		FILE_LOG(logDEBUG5, ("received %d dacs\n",nDacs));

	nAdcs = myMod->nadc;
	nadcdiff = nAdcs-naold;
	if (naold != nAdcs) {
		FILE_LOG(logERROR, ("received wrong number of adcs\n"));
	}
	else
		FILE_LOG(logDEBUG5, ("received %d adcs\n",nAdcs));
	if (ndacdiff <= 0) {
		n = receiveData(file_des,myMod->dacs, sizeof(int)*nDacs,INT32);
		if (!n)	return -1; ts += n;
		FILE_LOG(logDEBUG5, ("dacs received\n"));
		int id;
		for (id = 0; id<nDacs; id++)
			FILE_LOG(logDEBUG5, ("dac %d val %d\n",id,  (int)myMod->dacs[id]));
	} else {
		dacptr = (int*)malloc(ndacdiff*sizeof(int));
		myMod->ndac = ndold;
		n = receiveData(file_des,myMod->dacs, sizeof(int)*ndold,INT32);
		if (!n)	return -1; ts += n;
		n = receiveData(file_des,dacptr, sizeof(int)*ndacdiff,INT32);
		if (!n)	return -1; ts += n;
		free(dacptr);
		return FAIL;
	}

	if (nadcdiff <= 0) {
		n = receiveData(file_des,myMod->adcs, sizeof(int)*nAdcs,INT32);
		if (!n)	return -1; ts += n;
		FILE_LOG(logDEBUG5, ("adcs received\n"));
	} else {
		adcptr = (int*)malloc(nadcdiff*sizeof(int));
		myMod->nadc = naold;
		n = receiveData(file_des,myMod->adcs, sizeof(int)*naold,INT32);
		if (!n)	return -1; ts += n;
		n = receiveData(file_des,adcptr, sizeof(int)*nadcdiff,INT32);
		if (!n)	return -1; ts += n;
		free(adcptr);
		return FAIL;
	}


	// some detectors dont require sending all trimbits etc.
	if(receiveAll){

		if (nchipdiff <= 0) {
			n = receiveData(file_des,myMod->chipregs, sizeof(int)*nChips,INT32);
			if (!n)	return -1; ts += n;
			FILE_LOG(logDEBUG5, ("chips received\n"));
		} else {
			chipptr = (int*)malloc(nchipdiff*sizeof(int));
			myMod->nchip = nchipold;
			n = receiveData(file_des,myMod->chipregs, sizeof(int)*nchipold,INT32);
			if (!n)	return -1; ts += n;
			n = receiveData(file_des,chipptr, sizeof(int)*nchipdiff,INT32);
			if (!n)	return -1; ts += n;
			free(chipptr);
			return FAIL;
		}

		if (nchandiff <= 0) {
			n = receiveData(file_des,myMod->chanregs, sizeof(int)*nChans,INT32);
			if (!n)	return -1; ts += n;
			FILE_LOG(logDEBUG5, ("chans received\n"));
		} else {
			chanptr = (int*)malloc(nchandiff*sizeof(int));
			myMod->nchan = nchanold;
			n = receiveData(file_des,myMod->chanregs, sizeof(int)*nchanold,INT32);
			if (!n)	return -1; ts += n;
			n = receiveData(file_des,chanptr, sizeof(int)*nchandiff,INT32);
			if (!n)	return -1; ts += n;
			free(chanptr);
			return FAIL;
		}
	}
	FILE_LOG(logDEBUG5, ("received module of size %d register %x\n",ts,myMod->reg));
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
	if (update && ret != FAIL && differentClients)
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

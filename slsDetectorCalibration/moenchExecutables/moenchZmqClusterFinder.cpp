#include "sls_receiver_defs.h"
#include "ZmqSocket.h"
#include "moench03T1ZmqDataNew.h"
#include <vector> 
#include <string> 
#include <sstream> 
#include <iomanip> 
#include <fstream> 
#include "tiffIO.h"

#include<iostream>

//#include "analogDetector.h"
#include "singlePhotonDetector.h"
#include "multiThreadedAnalogDetector.h"
#include "ansi.h"
#include <iostream>
using namespace std;


#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [send_socket ip] [send port number]
 *
 */
  FILE *of=NULL;
  int fifosize=1000;
  int nthreads=20;
	// help
	if (argc < 3 ) {
		cprintf(RED, "Help: ./trial [receive socket ip] [receive starting port number] [send_socket ip] [send starting port number]\n");
		return EXIT_FAILURE;
	}

	// receive parameters
	bool send = false;
	  char* socketip=argv[1];
	uint32_t portnum = atoi(argv[2]);
	int size = 32*2*5000;//atoi(argv[3]);

	// send parameters if any
	char* socketip2 = 0;
	uint32_t portnum2 = 0;
	if (argc > 3) {
		send = true;
		socketip2 = argv[3];
		portnum2 = atoi(argv[4]);
	}
	cout << "\nrx socket ip : " << socketip <<
			"\nrx port num  : " <<  portnum ;
	if (send) {
		cout << "\nsd socket ip : " << socketip2 <<
				"\nsd port num  : " <<  portnum2;
	}
	cout << endl;

	//slsDetectorData *det=new moench03T1ZmqDataNew(); 
	 moench03T1ZmqDataNew *det=new moench03T1ZmqDataNew(); 
	 //analogDetector<uint16_t> *filter=new analogDetector<uint16_t>(det,1,NULL,1000);
	 singlePhotonDetector *filter=new singlePhotonDetector(det,3, 5, 1, 0, 1000, 10);


	  char* buff;
	  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
	  mt->setFrameMode(eFrame);
	  mt->StartThreads();
	  mt->popFree(buff);


	  


	// receive socket
	  ZmqSocket* zmqsocket = new ZmqSocket(socketip,portnum); 


	  
	if (zmqsocket->IsError()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum, socketip);
		delete zmqsocket;
		return EXIT_FAILURE;
	}
	zmqsocket->Connect();
	printf("Zmq Client at %s\n", zmqsocket->GetZmqServerAddress());

	// send socket
	ZmqSocket* zmqsocket2 = 0;
	  cout << "zmq2 " << endl;
	if (send) {
		zmqsocket2 = new ZmqSocket(portnum2, socketip2);
		if (zmqsocket2->IsError()) {
			bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			delete zmqsocket2;
			delete zmqsocket;
			return EXIT_FAILURE;
		}
		zmqsocket2->Connect();
		printf("Zmq Server started at %s\n", zmqsocket2->GetZmqServerAddress());
	}


	// header variables
	uint64_t acqIndex = -1;
	uint64_t frameIndex = -1;
	uint32_t subframeIndex = -1;
	uint64_t fileindex = -1;
	string filename = "";
	char* image = new char[size];
	//int* image = new int[(size/sizeof(int))]();

	int *nph;
	int iframe=0;
	char ofname[10000];
		    
	char fname[10000];
	int length;
	int *detimage;
	int nnx, nny,nns;
	filter->getImageSize(nnx, nny,nns);
	int16_t *dout=new int16_t [nnx*nny];
	// infinite loop
	while(1) {


	  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
		// get header, (if dummy, fail is on parse error or end of acquisition)
	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)){
	    //	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)) {
	    cprintf(RED, "Got Dummy\n");
	    while (mt->isBusy()) {;}//wait until all data are processed from the queues
			
	    
	    detimage=mt->getImage(nnx,nny,nns);
	    for (int ix=0; ix<nnx; ix++) {
	      for (int iy=0; iy<nny; iy++) {
		dout[iy*nnx+ix]=detimage[iy*nnx+ix];
	      }
	    }

	    if (send) {
	      strcpy(fname,filename.c_str());
	      //  zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,16,fileindex,400,400,400*400, acqIndex,frameIndex,fname, acqIndex, 0,0,0,0,0,0,0,0,0,0,0,1);
	      zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
	     
	      zmqsocket2->SendData((char*)dout,length);
	      cprintf(GREEN, "Sent Data\n");
	    }
	    
	    
	    // stream dummy  to socket2 to signal end of acquisition
	    if (send) {
	      zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	      cprintf(RED, "Sent Dummy\n");
	    }
	    fclose(of);
	    of=NULL;
	    
	    continue; //continue to not get out
	  }


	  if (of==NULL) {
	    sprintf(ofname,"%s_%d.clust",filename.c_str(),fileindex);
	    of=fopen(ofname,"w");
	    if (of) {
	      mt->setFilePointer(of);
	    }else {
	      cout << "Could not open "<< ofname << " for writing " << endl;
	      mt->setFilePointer(NULL);
	    }
	  }
	  
	  // get data
	  length = zmqsocket->ReceiveData(0, buff, size);
	  mt->pushData(buff);
	  mt->nextThread();
	  mt->popFree(buff);
	  
	
	  
	  
	  iframe++;
	}	// exiting infinite loop



	delete zmqsocket;
	if (send)
	  delete zmqsocket2;

	
	cout<<"Goodbye"<<  endl;
	return 0;
}


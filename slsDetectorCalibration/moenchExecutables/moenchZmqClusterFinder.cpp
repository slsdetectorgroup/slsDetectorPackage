#include "sls_receiver_defs.h"
#include "ZmqSocket.h"
#include "moench03T1ZmqDataNew.h"
#include <vector> 
#include <string> 
#include <sstream> 
#include <iomanip> 
#include <fstream> 
#include "tiffIO.h"


//#define NEWZMQ
#ifdef NEWZMQ
#include <rapidjson/document.h> //json header in zmq stream
#endif

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
  int maxSize = 32*2*8192;//5000;//atoi(argv[3]);
  int size= 32*2*5000;
  int multisize=size;
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
  
  
  ZmqSocket* zmqsocket=NULL;

#ifdef NEWZMQ
  // receive socket
  try{
#endif
    
    zmqsocket = new ZmqSocket(socketip,portnum); 
    

#ifdef NEWZMQ
	  }  catch (...) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum, socketip);
		delete zmqsocket;
		return EXIT_FAILURE;
        }
#endif

#ifndef NEWZMQ	  
	if (zmqsocket->IsError()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum, socketip);
		delete zmqsocket;
		return EXIT_FAILURE;
	}
#endif
	if (zmqsocket->Connect()) {
	  cprintf(RED, "Error: Could not connect to socket  %s\n",
		  zmqsocket->GetZmqServerAddress());
	  delete zmqsocket;
	  return EXIT_FAILURE;
	} else 
	  printf("Zmq Client at %s\n", zmqsocket->GetZmqServerAddress());
	
	// send socket
	ZmqSocket* zmqsocket2 = 0;
	  cout << "zmq2 " << endl;
	if (send) {
#ifdef NEWZMQ
	// receive socket
	  try{
#endif
		zmqsocket2 = new ZmqSocket(portnum2, socketip2);



#ifdef NEWZMQ
	  }  catch (...) {
			cprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			delete zmqsocket2;
			delete zmqsocket;
			return EXIT_FAILURE;
	  }
#endif

#ifndef NEWZMQ	  
		if (zmqsocket2->IsError()) {
			cprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			delete zmqsocket2;
			delete zmqsocket;
			return EXIT_FAILURE;
		}
#endif	
		if (zmqsocket2->Connect()) {
		  cprintf(RED, "Error: Could not connect to socket  %s\n",
					zmqsocket2->GetZmqServerAddress());
		delete zmqsocket2;
		return EXIT_FAILURE;
	} else 
		  printf("Zmq Client at %s\n", zmqsocket2->GetZmqServerAddress());

	}


	// header variables
	uint64_t acqIndex = -1;
	uint64_t frameIndex = -1;
	uint32_t subFrameIndex = -1;
	uint64_t fileindex = -1;
	string filename = "";
	//	char* image = new char[size];
	//int* image = new int[(size/sizeof(int))]();
	uint32_t flippedDataX = -1;
	int *nph;
	int iframe=0;
	char ofname[10000];
		    
	char fname[10000];
	int length;
	int *detimage;
	int nnx, nny,nns;
	uint32_t imageSize = 0, nPixelsX = 0, nPixelsY = 0, dynamicRange = 0;
	filter->getImageSize(nnx, nny,nns);
	int16_t *dout=new int16_t [nnx*nny];
	// infinite loop
	uint32_t packetNumber = 0;
	uint64_t bunchId = 0;
	uint64_t timestamp = 0;
	int16_t modId = 0;
	uint16_t xCoord = 0;
	uint16_t yCoord = 0;
	uint16_t zCoord = 0;
	uint32_t debug = 0;
	uint32_t dr = 16;
	uint16_t roundRNumber = 0;
	uint8_t detType = 0;
	uint8_t version = 0;
	int* flippedData = 0;
	char* additionalJsonHeader = 0;

	uint32_t threshold=0;
	
	uint32_t xmin=0, xmax=400, ymin=0, ymax=400;
	
	string frameMode_s, detectorMode_s;

	int emin, emax;


	int newFrame=1;

	while(1) {


	  //  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
		// get header, (if dummy, fail is on parse error or end of acquisition)
#ifndef NEWZMQ	
	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)){
#endif

#ifdef NEWZMQ	
	    rapidjson::Document doc;
	    if (!zmqsocket->ReceiveHeader(0, doc, SLS_DETECTOR_JSON_HEADER_VERSION)) {
	      zmqsocket->CloseHeaderMessage();

#endif
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
#ifdef NEWZMQ	
	      //zmqsocket2->SendHeaderData (0, false, SLS_DETECTOR_JSON_HEADER_VERSION, dynamicRange, fileindex,
	      //			  nnx, nny, nns*dynamicRange/8,acqIndex, frameIndex, fname, acqIndex, subFrameIndex, packetNumber,bunchId, timestamp, modId, xCoord, yCoord, zCoord,debug, roundRNumber, detType, version, flippedData, additionalJsonHeader);
	
	      zmqsocket2->SendHeaderData (0, false, SLS_DETECTOR_JSON_HEADER_VERSION, dr, fileindex,
					  nnx, nny, nns*dr/8,acqIndex, frameIndex, fname, acqIndex, subFrameIndex, packetNumber,bunchId, timestamp, modId, xCoord, yCoord, zCoord,debug, roundRNumber, detType, version, flippedData, additionalJsonHeader);
					   				   
#endif

#ifndef NEWZMQ
	        zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
#endif
	     
	      zmqsocket2->SendData((char*)dout,length);
	      cprintf(GREEN, "Sent Data\n");
	    }
	    
	    
	    // stream dummy  to socket2 to signal end of acquisition
	    if (send) {
	      zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	      cprintf(RED, "Sent Dummy\n");
	    }
	    mt->clearImage();
	    if (of) {
	      fclose(of);
	      of=NULL;
	    }
	    
	    newFrame=1;
	    continue; //continue to not get out
	  }

#ifdef NEWZMQ
	    if (newFrame) {
	      //  acqIndex, frameIndex, subframeIndex, filename, fileindex
	      size       = doc["size"].GetUint();
	      multisize  = size;// * zmqsocket->size();
	      dynamicRange  = doc["bitmode"].GetUint();
	      nPixelsX = doc["shape"][0].GetUint();
	      nPixelsY = doc["shape"][1].GetUint();
	      filename        = doc["fname"].GetString();
	      acqIndex = doc["acqIndex"].GetUint64();
	      frameIndex       = doc["fIndex"].GetUint64();
	      fileindex        = doc["fileIndex"].GetUint64();
	      subFrameIndex    = doc["expLength"].GetUint();
	      xCoord                  = doc["xCoord"].GetUint();
	      yCoord                  = doc["yCoord"].GetUint();
	      zCoord                  = doc["zCoord"].GetUint();
	      flippedDataX=doc["flippedDataX"].GetUint();
	      packetNumber=doc["packetNumber"].GetUint();
	      bunchId=doc["bunchId"].GetUint();
	      timestamp=doc["timestamp"].GetUint();
	      modId=doc["modId"].GetUint();
	      debug=doc["debug"].GetUint();
	      roundRNumber=doc["roundRNumber"].GetUint();
	      detType=doc["detType"].GetUint();
	      version=doc["version"].GetUint();



	      cprintf(BLUE, "Header Info:\n"
		      "size: %u\n"
		      "multisize: %u\n"
		      "dynamicRange: %u\n"
		      "nPixelsX: %u\n"
		      "nPixelsY: %u\n"
		      "currentFileName: %s\n"
		      "currentAcquisitionIndex: %lu\n"
		      "currentFrameIndex: %lu\n"
		      "currentFileIndex: %lu\n"
		      "currentSubFrameIndex: %u\n"
		      "xCoordX: %u\n"
		      "yCoordY: %u\n"
		      "zCoordZ: %u\n"
		      "flippedDataX: %u\n"
		      "packetNumber: %u\n"
		      "bunchId: %u\n"
		      "timestamp: %u\n"
		      "modId: %u\n"
		      "debug: %u\n"
		      "roundRNumber: %u\n"
		      "detType: %u\n"
		      "version: %u\n",
		      size, multisize, dynamicRange, nPixelsX, nPixelsY,
		      filename.c_str(), acqIndex,
		      frameIndex, fileindex, subFrameIndex,
		      xCoord, yCoord,zCoord,
		      flippedDataX, packetNumber, bunchId, timestamp, modId, debug, roundRNumber, detType, version);


	      if (doc.HasMember("threshold")) {
		version=doc["threshold"].GetUint();
	
	      }

	      if (doc.HasMember("roi")) {
		xmin=doc["roi"][0].GetUint();
		xmax=doc["roi"][1].GetUint();
		ymin=doc["roi"][2].GetUint();
		ymax=doc["roi"][3].GetUint();	

	      }
	      
	      if (doc.HasMember("frameMode")) {
		frameMode_s=doc["frameMode"].GetString();

	      }
	      
	      if (doc.HasMember("detectorMode")) {
		detectorMode_s=doc["detectorMode"].GetString();

	      }
	      
	      if (doc.HasMember("energyRange")) {
		emin=doc["energyRange"][0].GetUint();
		emax=doc["energyRange"][0].GetUint();

	      }
	      
	      
	      if (doc.HasMember("dynamicRange")) {
		dr=doc["dynamicRange"].GetUint();
	      }
	      
	      if (doc.HasMember("nSubPixels")) {
		nsubPixels=doc["nSubPixels"].GetUint();
	      }
	      
	      

	      newFrame=0;
	      zmqsocket->CloseHeaderMessage();
	    }
#endif

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


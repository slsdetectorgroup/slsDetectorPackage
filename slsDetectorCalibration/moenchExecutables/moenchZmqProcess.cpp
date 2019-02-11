#define WRITE_QUAD

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
//#include "multiThreadedAnalogDetector.h"
//#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"
//#include "multiThreadedCountingDetector.h"
#include "multiThreadedInterpolatingDetector.h"
#include "etaInterpolationPosXY.h"
#include "ansi.h"
#include <iostream>

//#include <chrono>
#include <ctime> // time_t
#include <cstdio>

using namespace std;
//using namespace std::chrono;

//#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2

	//	myDet->setNetworkParameter(ADDITIONAL_JSON_HEADER, " \"what\":\"nothing\" ");

int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [send_socket ip] [send port number]
 *
 */
  FILE *of=NULL;
  int fifosize=5000;
  int etabins=1000;//nsubpix*2*100;
  double etamin=-1, etamax=2;
	// help
  if (argc < 3 ) {
    cprintf(RED, "Help: ./trial [receive socket ip] [receive starting port number] [send_socket ip] [send starting port number] [nthreads] [nsubpix] [etafile]\n");
    return EXIT_FAILURE;  
  }
  
  // receive parameters
  bool send = false;
  char* socketip=argv[1];
  uint32_t portnum = atoi(argv[2]);
  // send parameters if any
  char* socketip2 = 0;
  uint32_t portnum2 = 0;


  int ok;

  // high_resolution_clock::time_point t1;
  // high_resolution_clock::time_point t2 ;
  time_t begin,end,finished;


  if (argc > 4) {
	  socketip2 = argv[3];
	  portnum2 = atoi(argv[4]);
	  if (portnum2>0)
	    send = true;
  }
  cout << "\nrx socket ip : " << socketip <<
    "\nrx port num  : " <<  portnum ;
  if (send) {
    cout << "\ntx socket ip : " << socketip2 <<
      "\ntx port num  : " <<  portnum2;
  }
  int nthreads=5;
  if (argc>5)
    nthreads=atoi(argv[5]);

  cout << "Number of threads is: " << nthreads << endl;
  int nSubPixels=2;
  if (argc>6)
    nSubPixels=atoi(argv[6]);
  cout << "Number of subpixels is: " << nSubPixels << endl;
  
  char *etafname=NULL;
  if (argc>7) {
    etafname=argv[7];
    cout << "Eta file name is: " << etafname << endl;
  }


  //slsDetectorData *det=new moench03T1ZmqDataNew(); 
  moench03T1ZmqDataNew *det=new moench03T1ZmqDataNew(); 
  cout << endl << " det" <<endl;
  int npx, npy;
  det->getDetectorSize(npx, npy);




  int maxSize = npx*npy*2;//32*2*8192;//5000;//atoi(argv[3]);
  int size= maxSize;//32*2*5000;
  int multisize=size;
  int dataSize=size;

  char dummybuff[size];



  //analogDetector<uint16_t> *filter=new analogDetector<uint16_t>(det,1,NULL,1000);
#ifndef INTERP
  singlePhotonDetector *filter=new singlePhotonDetector(det,3, 5, 1, 0, 1000, 10);

    multiThreadedCountingDetector *mt=new multiThreadedCountingDetector(filter,nthreads,fifosize);

    // multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
#endif
#ifdef INTERP
  eta2InterpolationPosXY *interp=new eta2InterpolationPosXY(npx, npy, nSubPixels, etabins, etamin, etamax);

  if (etafname) interp->readFlatField(etafname);

  interpolatingDetector *filter=new interpolatingDetector(det,interp, 5, 1, 0, 1000, 10);
  multiThreadedInterpolatingDetector *mt=new multiThreadedInterpolatingDetector(filter,nthreads,fifosize);
#endif



  char* buff;
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
	// cout << "zmq2 " << endl;
	if (send) {
#ifdef NEWZMQ
	// receive socket
	  try{
#endif
		zmqsocket2 = new ZmqSocket(portnum2, socketip2);



#ifdef NEWZMQ
	  }  catch (...) {
			cprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			//	delete zmqsocket2;
			//	zmqsocket2=NULL;
			//	delete zmqsocket;
			//	return EXIT_FAILURE;
			send = false;
	  }
#endif

#ifndef NEWZMQ	  
		if (zmqsocket2->IsError()) {
			cprintf(RED, "AAA Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			//	delete zmqsocket2;
			//delete zmqsocket;
			//	return EXIT_FAILURE;
			send = false;
		}
#endif	
		if (zmqsocket2->Connect()) {
		  cprintf(RED, "BBB Error: Could not connect to socket  %s\n",
					zmqsocket2->GetZmqServerAddress());
		  //	delete zmqsocket2;
		send = false;
		//	return EXIT_FAILURE;
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
	// infinite loop
	uint32_t packetNumber = 0;
	uint64_t bunchId = 0;
	uint64_t timestamp = 0;
	int16_t modId = 0;
	uint16_t xCoord = 0;
	uint16_t yCoord = 0;
	uint16_t zCoord = 0;
	uint32_t debug = 0;
	//uint32_t dr = 16;
	//int16_t *dout;//=new int16_t [nnx*nny];
	uint32_t dr = 32;
	int32_t *dout=NULL;//=new int32_t [nnx*nny];
	uint32_t nSigma=5;
	uint16_t roundRNumber = 0;
	uint8_t detType = 0;
	uint8_t version = 0;
	int* flippedData = 0;
	char* additionalJsonHeader = 0;

	int32_t threshold=0;
	
	int32_t xmin=0, xmax=400, ymin=0, ymax=400;
	
	string frameMode_s, detectorMode_s, intMode_s;

	int emin, emax;
	int resetFlat=0;
	int resetPed=0;
	int nsubPixels=1;
	int isPedestal;
	int isFlat=0;
	int newFrame=1;
	detectorMode dMode;
	frameMode fMode;
	double *ped;

	filter->getImageSize(nnx, nny,nns);

	




	while(1) {


	  //  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
		// get header, (if dummy, fail is on parse error or end of acquisition)
#ifndef NEWZMQ	
	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)){
#endif

#ifdef NEWZMQ	
	    rapidjson::Document doc;
	    if (!zmqsocket->ReceiveHeader(0, doc, SLS_DETECTOR_JSON_HEADER_VERSION)) {
	      /* zmqsocket->CloseHeaderMessage();*/

#endif
	    //	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)) {
	      //  cprintf(RED, "Got Dummy\n");
	      //   t1=high_resolution_clock::now();
	      time(&end);


	    while (mt->isBusy()) {;}//wait until all data are processed from the queues
	    
	    if (of) {
	      fclose(of);
	      of=NULL;
	    }
	    if (newFrame>0) {
	      cprintf(RED,"DIDn't receive any data!\n");
	    if (send) { 
	      zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	      cprintf(RED, "Sent Dummy\n");
	    }
	    } else {
	    if (fMode==ePedestal) {
	      sprintf(ofname,"%s_%d_ped.tiff",fname,fileindex);
	      mt->writePedestal(ofname);
	       cout << "Writing pedestal to " << ofname << endl;
	    }
#ifdef INTERP
	    else if  (fMode==eFlat) {
	      mt->prepareInterpolation(ok);
	      sprintf(ofname,"%s_%d_eta.tiff",fname,fileindex);
	      mt->writeFlatField(ofname);
	       cout << "Writing eta to " << ofname << endl;
	    }
#endif
	    else {
	      sprintf(ofname,"%s_%d.tiff",fname,fileindex);
	      mt->writeImage(ofname);
	       cout << "Writing image to " << ofname << endl;
	    }
	    //  cout << nns*nnx*nny*nns*dr/8 << " " << length << endl;

	    if (send) {

	    if (fMode==ePedestal) {   
	      cprintf(MAGENTA,"Get pedestal!\n");
	      nns=1;
	      nnx=npx;
	      nny=npy;
	      //dout= new int16_t[nnx*nny*nns*nns];
	      dout= new int32_t[nnx*nny*nns*nns];
	      // cout << "get pedestal " << endl;
	      ped=mt->getPedestal();
	      // cout << "got pedestal " << endl;
	      for (int ix=0; ix<nnx*nny; ix++) {
	
		  dout[ix]=ped[ix];
		  // if (ix<100*400)
		  //   cout << ix << " " << ped[ix] << endl;
	      }
	      
	    }
#ifdef INTERP
	    else if  (fMode==eFlat) {
	       int nb;
	      double emi, ema;
	       int *ff=mt->getFlatField(nb, emi, ema);
	       nnx=nb;
	       nny=nb;
	       dout= new int32_t[nb*nb];
	       for (int ix=0; ix<nb*nb; ix++) {
	     	dout[ix]=ff[ix];
	       }
	     }
#endif
	    else {
	      detimage=mt->getImage(nnx,nny,nns);
	      cprintf(MAGENTA,"Get image!\n");
	      cout << nnx << " " << nny << " " << nns << endl;
	      // nns=1;
	      // nnx=npx;
	      // nny=npy;
	      // nnx=nnx*nns;
	      //nny=nny*nns;
	      dout= new int32_t[nnx*nny];
	      for (int ix=0; ix<nnx*nny; ix++) {
		// for (int iy=0; iy<nny*nns; iy++) {
		 //  for (int isx=0; isx<nns; isx++) {
		//   for (int isy=0; isy<nns; isy++) {
		//     if (isx==0 && isy==0)
		//       dout[iy*nnx+ix]=detimage[(iy+isy)*nnx*nns+ix+isx];
		//     else
		//       dout[iy*nnx+ix]+=detimage[(iy+isy)*nnx*nns+ix+isx];
		    
		//   }
		// }
		  dout[ix]=detimage[ix];
		  if (dout[ix]<0) dout[ix]=0;
		  //   cout << ix << " " << dout[ix] << endl;
		  // }
	      }
	    }



#ifdef NEWZMQ	
	    cout << "Sending image size " << nnx << " " << nny << endl;
	      zmqsocket2->SendHeaderData (0, false, SLS_DETECTOR_JSON_HEADER_VERSION, dr, fileindex, nnx, nny, nnx*nny*dr/8,acqIndex, frameIndex, fname, acqIndex, subFrameIndex, packetNumber,bunchId, timestamp, modId, xCoord, yCoord, zCoord,debug, roundRNumber, detType, version, flippedData, additionalJsonHeader);
					   				   
#endif

#ifndef NEWZMQ
	        zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
#endif
		
		zmqsocket2->SendData((char*)dout,nnx*nny*dr/8);
		cprintf(GREEN, "Sent Data\n");
		
		zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
		cprintf(RED, "Sent Dummy\n");
		if (dout)
		  delete [] dout;
		dout=NULL;

	    }
	    }

	    mt->clearImage();
	    
	    newFrame=1;
	    //t2 = high_resolution_clock::now();
	    
	      time(&finished);
	      //   auto meas_duration = duration_cast<microseconds>( t2 - t0 ).count();
	      //   auto real_duration = duration_cast<microseconds>( t2 - t1 ).count();
	    
	    cout << "Measurement lasted " << difftime(end,begin) << endl;
	    cout << "Processing lasted " << difftime(finished,begin) << endl;
	    continue; //continue to not get out


	    }

#ifdef NEWZMQ
	    if (newFrame) {
	      time(&begin);
	      // t0 = high_resolution_clock::now();
	      //cout <<"new frame" << endl;

	      //  acqIndex, frameIndex, subframeIndex, filename, fileindex
	      size       = doc["size"].GetUint();
	      // multisize  = size;// * zmqsocket->size();
	      dynamicRange  = doc["bitmode"].GetUint();
	      //  nPixelsX = doc["shape"][0].GetUint();
	      // nPixelsY = doc["shape"][1].GetUint();
	      filename        = doc["fname"].GetString();
	      //acqIndex = doc["acqIndex"].GetUint64();
	      //frameIndex       = doc["fIndex"].GetUint64();
	      fileindex        = doc["fileIndex"].GetUint64();
	      //subFrameIndex    = doc["expLength"].GetUint();
	      //packetNumber=doc["packetNumber"].GetUint();
	      //bunchId=doc["bunchId"].GetUint();
	      //timestamp=doc["timestamp"].GetUint();
	      //modId=doc["modId"].GetUint();
	      //debug=doc["debug"].GetUint();
	      //roundRNumber=doc["roundRNumber"].GetUint();
	      //detType=doc["detType"].GetUint();
	      //version=doc["version"].GetUint();

	      dataSize=size;

	      strcpy(fname,filename.c_str());

	      // cprintf(BLUE, "Header Info:\n"
	      // 	      "size: %u\n"
	      // 	      "multisize: %u\n"
	      // 	      "dynamicRange: %u\n"
	      // 	      "nPixelsX: %u\n"
	      // 	      "nPixelsY: %u\n"
	      // 	      "currentFileName: %s\n"
	      // 	      "currentAcquisitionIndex: %lu\n"
	      // 	      "currentFrameIndex: %lu\n"
	      // 	      "currentFileIndex: %lu\n"
	      // 	      "currentSubFrameIndex: %u\n"
	      // 	      "xCoordX: %u\n"
	      // 	      "yCoordY: %u\n"
	      // 	      "zCoordZ: %u\n"
	      // 	      "flippedDataX: %u\n"
	      // 	      "packetNumber: %u\n"
	      // 	      "bunchId: %u\n"
	      // 	      "timestamp: %u\n"
	      // 	      "modId: %u\n"
	      // 	      "debug: %u\n"
	      // 	      "roundRNumber: %u\n"
	      // 	      "detType: %u\n"
	      // 	      "version: %u\n",
	      // 	      size, multisize, dynamicRange, nPixelsX, nPixelsY,
	      // 	      filename.c_str(), acqIndex,
	      // 	      frameIndex, fileindex, subFrameIndex,
	      // 	      xCoord, yCoord,zCoord,
	      // 	      flippedDataX, packetNumber, bunchId, timestamp, modId, debug, roundRNumber, detType, version);
	      
	      /* Analog detector commands */
	      isPedestal=0;
	      isFlat=0;
	      fMode=eFrame;
	      frameMode_s="frame";
	      cprintf(MAGENTA, "Frame mode: ");
	      if (doc.HasMember("frameMode")) {
		if (doc["frameMode"].IsString()) {
		  frameMode_s=doc["frameMode"].GetString();
		  if (frameMode_s == "pedestal"){
		    fMode=ePedestal;
		    isPedestal=1;
		  } else if (frameMode_s == "newPedestal"){
		    mt->newDataSet(); //resets pedestal  
		    // cprintf(MAGENTA, "Resetting pedestal\n");
		    fMode=ePedestal;
		    isPedestal=1;
		  }
#ifdef INTERP 
		  else if (frameMode_s == "flatfield") {
		     fMode=eFlat;
		     isFlat=1;
		   } else if (frameMode_s == "newFlatfield") {
		     mt->resetFlatField();
		     isFlat=1;
		     cprintf(MAGENTA, "Resetting flatfield\n");
		     fMode=eFlat;
		   }
#endif
		  else {
		    fMode=eFrame;
		    isPedestal=0;
		    isFlat=0;
		    fMode=eFrame;
		    frameMode_s="frame";
		  }
		}
	      }
	      cprintf(MAGENTA, "%s\n" , frameMode_s.c_str());
	      mt->setFrameMode(fMode);

	      // threshold=0;
	      cprintf(MAGENTA, "Threshold: ");
	      if (doc.HasMember("threshold")) {	
		if (doc["threshold"].IsInt()) {
		  threshold=doc["threshold"].GetInt();
		  mt->setThreshold(threshold);
		}
	      }	
	      cprintf(MAGENTA, "%d\n", threshold);

	      xmin=0;
	      xmax=npx;
	      ymin=0;
	      ymax=npy;
	      cprintf(MAGENTA, "ROI: ");
	      if (doc.HasMember("roi")) {
		if (doc["roi"].IsArray()) {
		  if (doc["roi"].Size() > 0 )
		    if (doc["roi"][0].IsInt())
		      xmin=doc["roi"][0].GetInt();
		  
		  if (doc["roi"].Size() > 1 )
		    if (doc["roi"][1].IsInt())
		      xmax=doc["roi"][1].GetInt();

		  if (doc["roi"].Size() > 2 )
		    if (doc["roi"][2].IsInt())
		      ymin=doc["roi"][2].GetInt();
		  
		  if (doc["roi"].Size() > 3 )
		    if (doc["roi"][3].IsInt())
		      ymax=doc["roi"][3].GetInt();
		}
	      }

	      cprintf(MAGENTA, "%d %d %d %d\n", xmin, xmax, ymin, ymax);
	      mt->setROI(xmin, xmax, ymin, ymax);	
	
	      if (doc.HasMember("dynamicRange")) {
		dr=doc["dynamicRange"].GetUint();
		dr=32;
	      }

	      dMode=eAnalog;
	      detectorMode_s="analog";
	      cprintf(MAGENTA, "Detector mode: ");
	      if (doc.HasMember("detectorMode")) {
		if (doc["detectorMode"].IsString()) {
		    detectorMode_s=doc["detectorMode"].GetString();
#ifdef INTERP
		    if (detectorMode_s == "interpolating"){
		      dMode=eInterpolating;
		      mt->setInterpolation(interp);
		    } else 
#endif
		      if (detectorMode_s == "counting"){
		      dMode=ePhotonCounting;
#ifdef INTERP
		      mt->setInterpolation(NULL);
#endif
		    } else {
		      dMode=eAnalog;
#ifdef INTERP
		      mt->setInterpolation(NULL);
#endif
		    }
		  }
		  
	      }

	      mt->setDetectorMode(dMode);
	      cprintf(MAGENTA, "%s\n" , detectorMode_s.c_str());

	      // cout << "done " << endl;

	      // /* Single Photon Detector commands */
	      // nSigma=5;
	      // if (doc.HasMember("nSigma")) {
	      // 	if (doc["nSigma"].IsInt())
	      // 	  nSigma=doc["nSigma"].GetInt();
	      // 	mt->setNSigma(nSigma);
	      // }
	      
	      // emin=-1;
	      // emax=-1;
	      // if (doc.HasMember("energyRange")) {
	      // 	if (doc["energyRange"].IsArray()) {
	      // 	  if (doc["energyRange"].Size() > 0 )
	      // 	    if (doc["energyRange"][0].IsInt())
	      // 	      emin=doc["energyRange"][0].GetInt();
		  
	      // 	  if (doc["energyRange"].Size() > 1 )
	      // 	    if (doc["energyRange"][1].IsInt())
	      // 	      emax=doc["energyRange"][1].GetUint();
	      // 	}
	      // }
	      // if (doc.HasMember("eMin")) {
	      // 	if (doc["eMin"][1].IsInt())
	      // 	  emin=doc["eMin"].GetInt();
	      // }
	      // if (doc.HasMember("eMax")) {
	      // 	if (doc["eMax"][1].IsInt())
	      // 	  emin=doc["eMax"].GetInt();
	      // }
	      // mt->setEnergyRange(emin,emax);
	      
	      // /* interpolating detector commands */

	      // if (doc.HasMember("nSubPixels")) {
	      // 	if (doc["nSubPixels"].IsUint())
	      // 	nSubPixels=doc["nSubPixels"].GetUint();
	      // 	mt->setNSubPixels(nSubPixels);
	      // }
	      
	      
	      newFrame=0;
	      /* zmqsocket->CloseHeaderMessage();*/
	    }
#endif

	    // cout << "file" << endl;
	    //  cout << "data " << endl;
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
	  

	  // cout << "data" << endl;
	  // get data
	  // acqIndex = doc["acqIndex"].GetUint64();
	  frameIndex       = doc["fIndex"].GetUint64();
	  // subFrameIndex    = doc["expLength"].GetUint();

	  //  bunchId=doc["bunchId"].GetUint();
	  //  timestamp=doc["timestamp"].GetUint();
	      packetNumber=doc["packetNumber"].GetUint();
	      // cout << acqIndex << " " << frameIndex << " " << subFrameIndex << " "<< bunchId << " " << timestamp << " " << packetNumber << endl;
	      if (packetNumber>=40) {
		//*((int*)buff)=frameIndex;
		memcpy(buff,&frameIndex,sizeof(int));
		length = zmqsocket->ReceiveData(0, buff+sizeof(int), size);
		mt->pushData(buff);
		mt->nextThread();
		mt->popFree(buff);
	      } else {
		cprintf(RED, "Incomplete frame: received only %d packet\n", packetNumber);
		length = zmqsocket->ReceiveData(0, dummybuff, size);

	      }
	
	  
	  
	  iframe++;

	}	// exiting infinite loop



	delete zmqsocket;
	if (send)
	  delete zmqsocket2;

	
	cout<<"Goodbye"<<  endl;
	return 0;
}


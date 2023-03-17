// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#define WRITE_QUAD
//#define DEVELOPER
#undef CORR
//#undef MOENCH04

#define C_GHOST 0.0004

#define CM_ROWS 20

#define ZMQ
#include "sls/ZmqSocket.h"
#include "sls/sls_detector_defs.h"

//#include "moench03T1ZmqDataNew.h"

#include "jungfrauModuleData.h"


#include "moench03CommonMode.h"
#include "moench03GhostSummation.h"

#include "sls/tiffIO.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <rapidjson/document.h> //json header in zmq stream

#include <iostream>

//#include "analogDetector.h"
//#include "multiThreadedAnalogDetector.h"
//#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"
//#include "multiThreadedCountingDetector.h"
#include "etaInterpolationPosXY.h"
#include "multiThreadedInterpolatingDetector.h"
#include "sls/ansi.h"
#include <iostream>

#include <chrono>
#include <cstdio>
#include <ctime> // time_t

using namespace std;
using namespace std::chrono;

//#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2

//	myDet->setNetworkParameter(ADDITIONAL_JSON_HEADER, "
//\"what\":\"nothing\" ");

int main(int argc, char *argv[]) {
  /**
   * trial.o [socket ip] [starting port number] [send_socket ip] [send port
   * number]
   *
   */
  FILE *of = NULL;
  int fifosize = 500;
  int etabins = 1000, etabinsy = 1000; // nsubpix*2*100;
  double etamin = -1, etamax = 2;
  int nSubPixelsX = 2;
  int emin, emax;
  int nSubPixelsY = 2;

  // help
  if (argc < 3) {
    cprintf(RED, "Help: ./trial [receive socket ip] [receive starting port "
	    "number] [send_socket ip] [send starting port number] "
	    "[nthreads] [nsubpix] [gainmap]  [etafile]\n");
    return EXIT_FAILURE;
  }

  // receive parameters
  bool send = false;
  char *socketip = argv[1];
  uint32_t portnum = atoi(argv[2]);
  // send parameters if any
  char *socketip2 = 0;
  uint32_t portnum2 = 0;

  sls::zmqHeader zHeader, outHeader;
  zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;
  outHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;

  uint32_t nSigma = 5;

  int ok;

  high_resolution_clock::time_point t1;
  high_resolution_clock::time_point t2;
  std::chrono::steady_clock::time_point begin, end, finished;
  // time_t begin,end,finished;
  int rms = 0;

  if (argc > 4) {
    socketip2 = argv[3];
    portnum2 = atoi(argv[4]);
    if (portnum2 > 0)
      send = true;
  }
  cout << "\nrx socket ip : " << socketip << "\nrx port num  : " << portnum;
  if (send) {
    cout << "\ntx socket ip : " << socketip2
	 << "\ntx port num  : " << portnum2;
  }
  int nthreads = 4;
  if (argc > 5)
    nthreads = atoi(argv[5]);
  cout << "Number of threads is: " << nthreads << endl;
   
  if (argc > 6) {
    nSubPixelsX = atoi(argv[6]);
    nSubPixelsY = nSubPixelsX;
#ifdef RECT
    nSubPixelsX = 2;
#endif
  }
  cout << "Number of subpixels is: " << nSubPixelsX << " " << nSubPixelsY
       << endl;

  char *gainfname = NULL;
  if (argc > 7) {
    gainfname = argv[7];
    cout << "Gain map file name is: " << gainfname << endl;
  }

  char *etafname = NULL;
  if (argc > 8) {
    etafname = argv[8];
    cout << "Eta file name is: " << etafname << endl;
  }

  // slsDetectorData *det=new moench03T1ZmqDataNew();



  jungfrauModuleData *det = new jungfrauModuleData();
  cout << endl << " det" << endl;
  int npx, npy;
  det->getDetectorSize(npx, npy);

  int send_something = 0;

  int maxSize = npx * npy * 2; // 32*2*8192;//5000;//atoi(argv[3]);
  int size = maxSize+sizeof(int);          // 32*2*5000;
  // int multisize=size;
  // int dataSize=size;

  char *dummybuff = new char[size];

  moench03CommonMode *cm = NULL;
  moench03GhostSummation *gs = NULL;
#ifdef CORR

  // int ncol_cm=CM_ROWS;
  // double xt_ghost=C_GHOST;

  cm = new moench03CommonMode(CM_ROWS);
  gs = new moench03GhostSummation(det, C_GHOST);
#endif
  double *gainmap = NULL;
  float *gm;
  double *gmap = NULL;

  uint32_t nnnx, nnny;
  if (gainfname) {
    gm = ReadFromTiff(gainfname, nnny, nnnx);
    if (gm && nnnx == (uint)npx && nnny == (uint)npy) {
      gmap = new double[npx * npy];
      for (int i = 0; i < npx * npy; i++) {
	gmap[i] = gm[i];
      }
      delete[] gm;
    } else
      cout << "Could not open gain map " << gainfname << endl;
  }

  // analogDetector<uint16_t> *filter=new
  // analogDetector<uint16_t>(det,1,NULL,1000);
#ifndef INTERP
  singlePhotonDetector *filter = new singlePhotonDetector(
							  det, 3, nSigma, 1, cm, 1000, 0, -1, -1, gainmap, gs);

  multiThreadedCountingDetector *mt =
    new multiThreadedCountingDetector(filter, nthreads, fifosize);

  // multiThreadedAnalogDetector *mt=new
  // multiThreadedAnalogDetector(filter,nthreads,fifosize);
#endif
#ifdef INTERP
  eta2InterpolationPosXY *interp = new eta2InterpolationPosXY(
							      npx, npy, nSubPixelsX, nSubPixelsY, etabins, etabinsy, etamin, etamax);

  if (etafname)
    interp->readFlatField(etafname);

  interpolatingDetector *filter = new interpolatingDetector(
							    det, interp, nSigma, 1, cm, 1000, 10, -1, -1, gainmap, gs);
  multiThreadedInterpolatingDetector *mt =
    new multiThreadedInterpolatingDetector(filter, nthreads, fifosize);
#endif

  char *buff;
  mt->setFrameMode(eFrame);
  mt->StartThreads();
  mt->popFree(buff);

  sls::ZmqSocket *zmqsocket = NULL;

#ifdef NEWZMQ
  // receive socket
  try {
#endif

    zmqsocket = new sls::ZmqSocket(socketip, portnum);

#ifdef NEWZMQ
  } catch (...) {
    cprintf(RED,
	    "Error: Could not create Zmq socket on port %d with ip %s\n",
	    portnum, socketip);
    delete zmqsocket;
    return EXIT_FAILURE;
  }
#endif



  if (zmqsocket->Connect()) {
    cprintf(RED, "Error: Could not connect to socket  %s\n",
	    (zmqsocket->GetZmqServerAddress()).c_str());
    delete zmqsocket;
    return EXIT_FAILURE;
  } else
    printf("Zmq Client at %s\n", zmqsocket->GetZmqServerAddress().c_str());

  // send socket
  sls::ZmqSocket *zmqsocket2 = 0;
  // cout << "zmq2 " << endl;
  if (send) {
#ifdef NEWZMQ
    // receive socket
    try {
#endif
      zmqsocket2 = new sls::ZmqSocket(portnum2, socketip2);

#ifdef NEWZMQ
    } catch (...) {
      cprintf(RED,
	      "Error: Could not create Zmq socket server on port %d and "
	      "ip %s\n",
	      portnum2, socketip2);
    
      send = false;
    }
#endif

    if (zmqsocket2->Connect()) {
      cprintf(RED, "Error: Could not connect to socket  %s\n",
	      zmqsocket2->GetZmqServerAddress().c_str());
      //	delete zmqsocket2;
      send = false;
      //	return EXIT_FAILURE;
    } else
      printf("Zmq Client at %s\n",
	     zmqsocket2->GetZmqServerAddress().c_str());
  }

  // header variables
  uint64_t acqIndex = -1;
  uint64_t frameIndex = -1;
 
  uint64_t subframes = 0;
  uint64_t insubframe = 0;
 
  double subnorm = 1;
  uint64_t f0 = -1, nsubframes = 0, nnsubframe = 0;
  uint64_t fileindex = -1;
  string filename = "";

  int iframe = 0;
  char ofname[10000];

  string fname;
  //	int length;
  int *detimage = NULL;
  int nnx, nny, nnsx, nnsy;
  uint32_t packetNumber = 0;
  uint64_t detSpec1 = 0;
  uint64_t timestamp = 0;
  int16_t modId = 0;
  uint32_t expLength = 0;
  uint16_t xCoord = 0;
  uint16_t yCoord = 0;
  // uint16_t zCoord = 0;
  uint32_t detSpec3 = 0;
  // uint32_t dr = 16;
  // int16_t *dout;//=new int16_t [nnx*nny];
  uint32_t dr = 32;
  int32_t *dout = NULL; //=new int32_t [nnx*nny];
  float *doutf = NULL;  //=new int32_t [nnx*nny];
  uint16_t detSpec4 = 0;
  uint8_t detType = 0;
  uint8_t version = 0;
  string additionalJsonHeader = "";

  int32_t threshold = 0;

  int32_t xmin = 0, xmax = 400, ymin = 0, ymax = 400;

  string frameMode_s, detectorMode_s, intMode_s;

  //	int resetFlat=0;
  // int resetPed=0;
  // int nsubPixels=1;
  // int isPedestal=0;
  // int isFlat=0;
  int newFrame = 1;
  detectorMode dMode = eAnalog;
  frameMode fMode = eFrame;
  double *ped;

  filter->getImageSize(nnx, nny, nnsx, nnsy);

  std::map<std::string, std::string> addJsonHeader;

  while (1) {

    //  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
    // get header, (if dummy, fail is on parse error or end of acquisition)

    //  rapidjson::Document doc;
    if (!zmqsocket->ReceiveHeader(0, zHeader,
				  SLS_DETECTOR_JSON_HEADER_VERSION)) {
      /* zmqsocket->CloseHeaderMessage();*/

      //	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex,
      //subframeIndex, filename, fileindex)) {
      cprintf(RED, "Got Dummy\n");
      //   t1=high_resolution_clock::now();
      // time(&end);
      // cout << "Measurement lasted " << difftime(end,begin) << endl;

      end = std::chrono::steady_clock::now();
      cout << "Measurement lasted " << (end - begin).count() * 0.000001
	   << " ms" << endl;

      while (mt->isBusy()) {
	;
      } // wait until all data are processed from the queues
      usleep(100);
      if (of) {
	mt->setFilePointer(NULL);
	fclose(of);
	of = NULL;
      }
      if (newFrame > 0) {
	cprintf(RED, "DIDn't receive any data!\n");
	if (send) {

	  // zHeader.data = false;
	  outHeader.data = false;
	  //  zmqsocket2->SendHeaderData(0, true,
	  //  SLS_DETECTOR_JSON_HEADER_VERSION);
	  zmqsocket2->SendHeader(0, outHeader);
	  cprintf(RED, "Sent Dummy\n");
	}
      } else {
	send_something = 0;
	if (fMode == ePedestal) {
	  sprintf(ofname, "%s_%ld_ped.tiff", fname.c_str(),
		  fileindex);
	  mt->writePedestal(ofname);
	  cout << "Writing pedestal to " << ofname << endl;
	  if (rms) {
	    sprintf(ofname, "%s_%ld_var.tiff", fname.c_str(),
		    fileindex);
	    mt->writePedestalRMS(ofname);
	  }
	  send_something = 1;
	}
#ifdef INTERP
	else if (fMode == eFlat) {
	  mt->prepareInterpolation(ok);
	  sprintf(ofname, "%s_%ld_eta.tiff", fname.c_str(),
		  fileindex);
	  mt->writeFlatField(ofname);
	  cout << "Writing eta to " << ofname << endl;
	  send_something = 1;
	}
#endif
	else {
	  if (subframes > 0) {
	    if (insubframe > 0) {
	      sprintf(ofname, "%s_sf%ld_%ld.tiff", fname.c_str(),
		      nnsubframe, fileindex);
	      //		  mt->writeImage(ofname);
	      doutf = new float[nnx * nny];
	      if (subframes > 0 && insubframe != subframes &&
		  insubframe > 0)
		subnorm =
		  ((double)subframes) / ((double)insubframe);
	      else
		subnorm = 1.;
	      for (int ix = 0; ix < nnx * nny; ix++) {
		doutf[ix] = detimage[ix] * subnorm;
		if (doutf[ix] < 0)
		  doutf[ix] = 0;
	      }

	      cout << "Writing image to " << ofname << endl;

	      WriteToTiff(doutf, ofname, nnx, nny);

	      if (doutf)
		delete[] doutf;
	      doutf = NULL;

	      nsubframes++;
	      insubframe = 0;
	      send_something = 1;
	    }
	  } else {
	    sprintf(ofname, "%s_%ld.tiff", fname.c_str(),
		    fileindex);
	    mt->writeImage(ofname);
	    send_something = 1;
	  }

	  cout << "Writing image to " << ofname << endl;
	}
	//  cout << nns*nnx*nny*nns*dr/8 << " " << length << endl;

	if (send) {

	  if (fMode == ePedestal) {
	    cprintf(MAGENTA, "Get pedestal!\n");
	    nnsx = 1;
	    nnsy = 1;

	    nnx = npx;
	    nny = npy;
	    // dout= new int16_t[nnx*nny*nns*nns];
	    dout = new int32_t[nnx * nny * nnsx * nnsy];
	    // cout << "get pedestal " << endl;
	    ped = mt->getPedestal();
	    // cout << "got pedestal " << endl;
	    for (int ix = 0; ix < nnx * nny; ix++) {

	      dout[ix] = ped[ix];
	   
	    }

	  }
#ifdef INTERP
	  else if (fMode == eFlat) {
	    int nbx, nby;
	    double emi = 0, ema = 1;
	    int *ff = mt->getFlatField(nbx, nby, emi, ema);
	    nnx = nbx;
	    nny = nby;
	    dout = new int32_t[nbx * nby];
	    for (int ix = 0; ix < nbx * nby; ix++) {
	      dout[ix] = ff[ix];
	    }
	  }
#endif
	  else {
	    detimage = mt->getImage(nnx, nny, nnsx, nnsy);
	    cprintf(MAGENTA, "Get image!\n");
	    cout << nnx << " " << nny << " " << nnsx << " " << nnsy
		 << endl;
	    // nns=1;
	    // nnx=npx;
	    // nny=npy;
	    // nnx=nnx*nns;
	    // nny=nny*nns;
	    dout = new int32_t[nnx * nny];
	    if (subframes > 0 && insubframe != subframes &&
		insubframe > 0)
	      subnorm =
		((double)subframes) / ((double)insubframe);
	    else
	      subnorm = 1.;
	    for (int ix = 0; ix < nnx * nny; ix++) {
	      // for (int iy=0; iy<nny*nns; iy++) {
	      //  for (int isx=0; isx<nns; isx++) {
	      //   for (int isy=0; isy<nns; isy++) {
	      //     if (isx==0 && isy==0)
	      //       dout[iy*nnx+ix]=detimage[(iy+isy)*nnx*nns+ix+isx];
	      //     else
	      //       dout[iy*nnx+ix]+=detimage[(iy+isy)*nnx*nns+ix+isx];

	      //   }
	      // }
	      dout[ix] = detimage[ix] * subnorm;
	      if (dout[ix] < 0)
		dout[ix] = 0;
	      //   cout << ix << " " << dout[ix] << endl;
	      // }
	    }
	  }
	  // if ((insubframe>0 && subframes>0) || (subframes<=0) ){

	  if (send_something) {

	    outHeader.data = true;
	    outHeader.dynamicRange = dr;
	    outHeader.fileIndex = fileindex;
	    outHeader.ndetx = 1;
	    outHeader.ndety = 1;
	    outHeader.npixelsx = nnx;
	    outHeader.npixelsy = nny;
	    outHeader.imageSize = nnx * nny * dr / 8;
	    outHeader.acqIndex = acqIndex;
	    outHeader.frameIndex = frameIndex;
	    outHeader.fname = fname;
	    outHeader.frameNumber = acqIndex;
	    outHeader.expLength = expLength;
	    outHeader.packetNumber = packetNumber;
	    outHeader.detSpec1 = detSpec1;
	    outHeader.timestamp = timestamp;
	    outHeader.modId = modId;
	    outHeader.row = xCoord;
	    outHeader.column = yCoord;
	    outHeader.detSpec3 = detSpec3;
	    outHeader.detSpec4 = detSpec4;
	    outHeader.detType = detType;
	    outHeader.version = version;

	    zmqsocket2->SendHeader(0, outHeader);
	    zmqsocket2->SendData((char *)dout, nnx * nny * dr / 8);
	    cprintf(GREEN, "Sent Data\n");
	  }
	  outHeader.data = false;
	  zmqsocket2->SendHeader(0, outHeader);
	  // zmqsocket2->SendHeaderData(0, true,
	  // SLS_DETECTOR_JSON_HEADER_VERSION);
	  cprintf(RED, "Sent Dummy\n");
	  if (dout)
	    delete[] dout;
	  dout = NULL;
	}
      }

      mt->clearImage();

      newFrame = 1;

      // time(&finished);
      // cout << "Processing lasted " << difftime(finished,begin) << endl;

      finished = std::chrono::steady_clock::now();
      cout << "Processing lasted "
	   << (finished - begin).count() * 0.000001 << " ms" << endl;
#ifdef OPTIMIZE
      return 0;
#endif
      continue; // continue to not get out
    }

    //#ifdef NEWZMQ
    if (newFrame) {
      begin = std::chrono::steady_clock::now();

      size = zHeader.imageSize; // doc["size"].GetUint();

      // dynamicRange  = zheader.dynamicRange; //doc["bitmode"].GetUint();
      // nPixelsX = zHeader.npixelsx; //doc["shape"][0].GetUint();
      // nPixelsY = zHeader.npixelsy;// doc["shape"][1].GetUint();
      filename = zHeader.fname; // doc["fname"].GetString();
      acqIndex =
	zHeader
	.acqIndex; // doc["acqIndex"].GetUint64();
      //  frameIndex       =
      //  zHeader.frameIndex;//doc["fIndex"].GetUint64();
      fileindex = zHeader.fileIndex; // doc["fileIndex"].GetUint64();
      expLength = zHeader.expLength; // doc["expLength"].GetUint();
      packetNumber =
	zHeader.packetNumber;      // doc["packetNumber"].GetUint();
      detSpec1 = zHeader.detSpec1;     // doc["detSpec1"].GetUint();
      timestamp = zHeader.timestamp; // doc["timestamp"].GetUint();
      modId = zHeader.modId;         // doc["modId"].GetUint();
      detSpec3 = zHeader.detSpec3;         // doc["detSpec3"].GetUint();
      //  detSpec4=r.detSpec4;//doc["detSpec4"].GetUint();
      detType = zHeader.detType; // doc["detType"].GetUint();
      version = zHeader.version; // doc["version"].GetUint();
      /*document["bitmode"].GetUint(); zHeader.dynamicRange


       */

      // strcpy(fname,filename.c_str());
      fname = filename;
      addJsonHeader = zHeader.addJsonHeader;

      rms = 0;
      fMode = eFrame;
      frameMode_s = "frame";
      cprintf(MAGENTA, "Frame mode: ");
      //	      if (doc.HasMember("frameMode")) {
      if (addJsonHeader.find("frameMode") != addJsonHeader.end()) {
	//	if (doc["frameMode"].IsString()) {
	frameMode_s = addJsonHeader.at(
				       "frameMode"); // doc["frameMode"].GetString();
	if (frameMode_s == "pedestal") {
	  fMode = ePedestal;
	  // isPedestal=1;
	} else if (frameMode_s == "newPedestal") {
	  mt->newDataSet(); // resets pedestal
	  // cprintf(MAGENTA, "Resetting pedestal\n");
	  fMode = ePedestal;
	  // isPedestal=1;
	} else if (frameMode_s == "variance") {
	  mt->newDataSet(); // resets pedestal
	  // cprintf(MAGENTA, "Resetting pedestal\n");
	  fMode = ePedestal;
	  // isPedestal=1;
	  rms = 1;
	} else if (frameMode_s == "raw") {
	  // mt->newDataSet(); //resets pedestal
	  //  cprintf(MAGENTA, "Resetting pedestal\n");
	  fMode = eRaw;
	  // isPedestal=1;
	}
#ifdef INTERP
	else if (frameMode_s == "flatfield") {
	  fMode = eFlat;
	  // isFlat=1;
	} else if (frameMode_s == "newFlatfield") {
	  mt->resetFlatField();
	  // isFlat=1;
	  cprintf(MAGENTA, "Resetting flatfield\n");
	  fMode = eFlat;
	}
	//#endif
	else {
	  // isPedestal=0;
	  // isFlat=0;
	  fMode = eFrame;
	  frameMode_s = "frame";
	}
	//}
      }
      cprintf(MAGENTA, "%s\n", frameMode_s.c_str());
      mt->setFrameMode(fMode);

      // threshold=0;
      cprintf(MAGENTA, "Threshold: ");
      if (addJsonHeader.find("threshold") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("threshold")) >> threshold;
      }
      mt->setThreshold(threshold);
      cprintf(MAGENTA, "%d\n", threshold);
      
      cprintf(MAGENTA, "subframes: ");
      if (addJsonHeader.find("subframes") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("subframes")) >> subframes;
      }
     
      cprintf(MAGENTA, "%d\n", subframes);
      



      xmin = 0;
      xmax = npx;
      ymin = 0;
      ymax = npy;
      cprintf(MAGENTA, "ROI: ");

      if (addJsonHeader.find("roi") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("roi")) >> xmin >> xmax >>
	  ymin >> ymax;
      }

      cprintf(MAGENTA, "%d %d %d %d\n", xmin, xmax, ymin, ymax);
      mt->setROI(xmin, xmax, ymin, ymax);
      if (addJsonHeader.find("dynamicRange") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("dynamicRange")) >> dr;
	dr = 32;
      }
      dMode = eAnalog;
      detectorMode_s = "analog";
      cprintf(MAGENTA, "Detector mode: ");
      if (addJsonHeader.find("detectorMode") != addJsonHeader.end()) {
	;
	detectorMode_s = addJsonHeader.at(
					  "detectorMode"); //=doc["detectorMode"].GetString();
#ifdef INTERP
	if (detectorMode_s == "interpolating") {
	  dMode = eInterpolating;
	  mt->setInterpolation(interp);
	} else
#endif
	  if (detectorMode_s == "counting") {
	    dMode = ePhotonCounting;
#ifdef INTERP
	    mt->setInterpolation(NULL);
#endif
	  } else {
	    dMode = eAnalog;
#ifdef INTERP
	    mt->setInterpolation(NULL);
#endif
	  }
	// }
	if (fMode == eRaw) {
	  detectorMode_s = "analog";
	  dMode = eAnalog;
	}
      }

      mt->setDetectorMode(dMode);
      cprintf(MAGENTA, "%s\n", detectorMode_s.c_str());

      cout << "done " << endl;

      /* Single Photon Detector commands */
      nSigma = 5;

      if (addJsonHeader.find("nSigma") != addJsonHeader.end()) {
	;
	istringstream(addJsonHeader.at("nSigma")) >> nSigma;
	mt->setNSigma(nSigma);
      }

      emin = -1;
      emax = -1;
      if (addJsonHeader.find("energyRange") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("energyRange")) >> emin >> emax;
      }
      if (addJsonHeader.find("eMin") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("eMin")) >> emin;
      }

      if (addJsonHeader.find("eMax") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("eMax")) >> emax;
      }

      mt->setEnergyRange(emin, emax);

      /* interpolating detector commands */
      // must set subpixels X and Y separately
      //  if (addJsonHeader.find("nSubPixels")!= addJsonHeader.end()) {
      //  	istringstream(addJsonHeader.at("nSubPixels")) >>
      //  nSubPixels ; 	mt->setNSubPixels(nSubPixels);
      //  }

      threshold = 0;
      cprintf(MAGENTA, "Subframes: ");
      subframes = 0;
      // isubframe=0;
      insubframe = 0;
      subnorm = 1;
      f0 = 0;
      nnsubframe = 0;
      if (addJsonHeader.find("subframes") != addJsonHeader.end()) {
	istringstream(addJsonHeader.at("subframes")) >> subframes;
      }

      cprintf(MAGENTA, "%ld\n", subframes);

      newFrame = 0;
    }
#endif

    frameIndex = zHeader.frameIndex; ////doc["fIndex"].GetUint64();

    // subFrameIndex    = doc["expLength"].GetUint();

    //  detSpec1=doc["detSpec1"].GetUint();
    //  timestamp=doc["timestamp"].GetUint();
    packetNumber = zHeader.packetNumber; // doc["packetNumber"].GetUint();
    // cout << acqIndex << " " << frameIndex << " " << subFrameIndex << "
    // "<< detSpec1 << " " << timestamp << " " << packetNumber << endl;
    // cprintf(GREEN, "frame\n");
    if (packetNumber >= 40) {
      //*((int*)buff)=frameIndex;
      if (insubframe == 0)
	f0 = frameIndex;
      memcpy(buff, &frameIndex, sizeof(int));
      // length =
      zmqsocket->ReceiveData(0, buff + sizeof(int), size);

      mt->setFilePointer(NULL);

      mt->pushData(buff);
      mt->nextThread();
      mt->popFree(buff);
      insubframe++;
      nsubframes = frameIndex + 1 - f0;
      // cout << "insubframe " << insubframe << endl;
      // cout << "nsubframes " << nsubframes << endl;
      // cout << "f0 " << f0 << endl;
      // cout << "frameIndex " << frameIndex << endl;

    } else {
      cprintf(RED, "Incomplete frame: received only %d packet\n",
	      packetNumber);
      // length =
      zmqsocket->ReceiveData(0, dummybuff, size);
    }

    if (subframes > 0 && insubframe >= subframes &&
	(fMode == eFrame || fMode == eRaw)) {
      while (mt->isBusy()) {
	;
      } // wait until all data are processed from the queues
      usleep(100);

      detimage = mt->getImage(nnx, nny, nnsx, nnsy);

      cprintf(MAGENTA, "Get image!\n");
      dout = new int32_t[nnx * nny];
      doutf = new float[nnx * nny];
      if (subframes > 0 && insubframe != subframes && insubframe > 0)
	subnorm = ((double)subframes) / ((double)insubframe);
      else
	subnorm = 1.;
      for (int ix = 0; ix < nnx * nny; ix++) {
	dout[ix] = detimage[ix] * subnorm;
	if (dout[ix] < 0)
	  dout[ix] = 0;
	doutf[ix] = dout[ix];
      }
      sprintf(ofname, "%s_sf%ld_%ld.tiff", fname.c_str(), nnsubframe,
	      fileindex);

      cout << "Writing image to " << ofname << endl;

      WriteToTiff(doutf, ofname, nnx, nny);
      nsubframes++;
      insubframe = 0;
      nnsubframe++;

      //   zmqsocket2->SendHeaderData (0,
      //   false,SLS_DETECTOR_JSON_HEADER_VERSION , dr, fileindex,
      //   1,1,nnx,nny,nnx*nny*dr/8,acqIndex, frameIndex, fname,acqIndex,0
      //   , packetNumber,detSpec1, timestamp, modId,xCoord, yCoord,
      //   zCoord,detSpec3, detSpec4, detType, version, 0,0,
      //   0,&additionalJsonHeader);

      //  zmqsocket2->SendHeaderData (0,
      //  false,SLS_DETECTOR_JSON_HEADER_VERSION , dr, fileindex,
      //  1,1,nnx,nny,nnx*nny*dr/8,acqIndex, frameIndex, fname,acqIndex,0
      //  , packetNumber,detSpec1, timestamp, modId,xCoord, yCoord,
      //  zCoord,detSpec3, detSpec4, detType, version, 0,0,
      //  0,&additionalJsonHeader);

      outHeader.data = true;
      outHeader.dynamicRange = dr;
      outHeader.fileIndex = fileindex;
      outHeader.ndetx = 1;
      outHeader.ndety = 1;
      outHeader.npixelsx = nnx;
      outHeader.npixelsy = nny;
      outHeader.imageSize = nnx * nny * dr / 8;
      outHeader.acqIndex = acqIndex;
      outHeader.frameIndex = frameIndex;
      outHeader.fname = fname;
      outHeader.frameNumber = acqIndex;
      outHeader.expLength = expLength;
      outHeader.packetNumber = packetNumber;
      outHeader.detSpec1 = detSpec1;
      outHeader.timestamp = timestamp;
      outHeader.modId = modId;
      outHeader.row = xCoord;
      outHeader.column = yCoord;
      outHeader.detSpec3 = detSpec3;
      outHeader.detSpec4 = detSpec4;
      outHeader.detType = detType;
      outHeader.version = version;

      zmqsocket2->SendHeader(0, outHeader);
      zmqsocket2->SendData((char *)dout, nnx * nny * dr / 8);
      cprintf(GREEN, "Sent subdata\n");

      if (dout)
	delete[] dout;
      dout = NULL;

      if (doutf)
	delete[] doutf;
      doutf = NULL;

      mt->clearImage();
    }

    iframe++;

  } // exiting infinite loop

  delete zmqsocket;
  if (send)
    delete zmqsocket2;

  cout << "Goodbye" << endl;
  return 0;
}

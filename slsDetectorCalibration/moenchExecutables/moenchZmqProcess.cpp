// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#define WRITE_QUAD
//#define DEVELOPER
#undef CORR
//#undef MOENCH04

#define C_GHOST 0.0004

#define CM_ROWS 20

#include "sls/ZmqSocket.h"
#include "sls/sls_detector_defs.h"
#ifndef MOENCH04
//#ifndef RECT
#include "moench03v2Data.h"
//#include "moench03T1ZmqDataNew.h"
//#endif
//#ifdef RECT
//#include "moench03T1ZmqDataNewRect.h"
//#endif
#endif
#ifdef MOENCH04
#include "moench04CtbZmq10GbData.h"
#endif

#include "moench03CommonMode.h"
#include "moench03GhostSummation.h"
#include "sls/tiffIO.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>

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
  std::map<std::string, std::string> args = {
    {"numinterfaces","1"},	
    {"rx_zmqip","10.1.2.102"},	
    {"rx_zmqport","7770"},
    {"zmqip","129.129.202.153"},
    {"zmqport","7780"},
    {"nthreads","5"},
    {"fifosize","5000"},
    {"nsigma","5"},
    {"gainfile","none"},
    {"nbinsx","5"},
    {"nbinsy","5"},
    {"etafile","none"},
    {"etabinsx","1000"},
    {"etamin","-1"},
    {"etamax","2"}  };
  FILE *of = NULL;
    int etabins = 1000, etabinsy = 1000; // nsubpix*2*100;
    double etamin = -1, etamax = 2;
    int nSubPixelsX = 2;
    int emin, emax;
    int nSubPixelsY = 2;
    int nthreads = 5;
    int fifosize = 5000;
    uint32_t nSigma = 5;

    string etafname;// = NULL;
    string gainfname;// = NULL;

    // receive parameters
    bool send = false;
    // send parameters if any
    string socketip2;// = 0;
    uint32_t portnum2 = 0;  
    string socketip;// = 0;
    uint32_t portnum = 0;

    sls::zmqHeader zHeader, outHeader;
    zHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;
    outHeader.jsonversion = SLS_DETECTOR_JSON_HEADER_VERSION;


    int ok;

    high_resolution_clock::time_point t1;
    high_resolution_clock::time_point t2;
    std::chrono::steady_clock::time_point begin, end, finished;
    // time_t begin,end,finished;
    int rms = 0;

    send = true;
    // help
    if (argc < 5) {
      std::string name, value,sline;
      int ic=0;
      ifstream flist;
      flist.open (argv[1], std::ifstream::in);
      if (flist.is_open()) {
	cout << "Using config file " <<argv[1] << endl;
       while (std::getline(flist,sline)){
	 if (sline.at(0)!='#') {
	   ic=sline.find(' ');
	   name = sline.substr(0,ic); 
	   value = sline.substr(ic+1,sline.size()-ic); 
	   args[name]=value;
	 }

       } 
       flist.close();
      } else {
        cprintf(RED, "Arguments are either: \n [config file] \n or the following list (deprecated): [receive socket ip] [receive starting port "
		"number] [send_socket ip] [send starting port number] "
		"[nthreads] [nsubpix] [gainmap]  [etafile]\n");
        return EXIT_FAILURE;
      }
    } else {
       args["rx_zmqip"]=argv[1];	
       args["rx_zmqport"]=argv[2];

       args["zmqip"]=argv[3];
       args["zmqport"]=argv[4];
       if (argc > 5)
        args["nthreads"] = argv[5];
       if (argc > 6) {
	 args["nbinsx"]=argv[6];
	 args["nbinsy"]=argv[6];
       }
       
      if (argc > 7) {
	args["gainfile"]=argv[7];
      }
      if (argc > 8) {
	args["etafilefile"]=argv[8];
      }

    }

       for (auto const& x : args)
	 {
	   std::cout << x.first  // string (key)
		     << ':' 
		     << x.second // string's value 
		     << std::endl;
	 }

    socketip = args["rx_zmqip"];
    portnum = atoi(args["rx_zmqport"].c_str());
    
    socketip2 = args["zmqip"];
    portnum2 = atoi(args["zmqport"].c_str());

    nthreads = atoi(args["nthreads"].c_str());
    nSubPixelsX =atoi(args["nbinsx"].c_str());
    nSubPixelsY =atoi(args["nbinsy"].c_str());
    gainfname = args["gainfile"];
    etafname = args["etafilefile"];
    
    if (atoi(args["numinterfaces"].c_str())>1){
      cprintf(RED, "Sorry, at the  moment only a single interface is supported instead of %d\n",atoi(args["numinterfaces"].c_str()));
      return EXIT_FAILURE;
    }


    // slsDetectorData *det=new moench03T1ZmqDataNew();
#ifndef MOENCH04
    cout << "This is a Moench03 v2" << endl;
    //moench03T1ZmqDataNew *det = new moench03T1ZmqDataNew();
    moench03v2Data *det = new moench03v2Data();
    cout << "MOENCH03!" << endl;
#endif
#ifdef MOENCH04
    cout << "This is a Moench04" << endl;
    moench04CtbZmq10GbData *det = new moench04CtbZmq10GbData();
#endif
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
    //if (gainfname) {
        gm = ReadFromTiff(gainfname.c_str(), nnny, nnnx);
        if (gm && nnnx == (uint)npx && nnny == (uint)npy) {
            gmap = new double[npx * npy];
            for (int i = 0; i < npx * npy; i++) {
                gmap[i] = gm[i];
            }
            delete[] gm;
        } else
            cout << "Could not open gain map " << gainfname << endl;
	//}

    // analogDetector<uint16_t> *filter=new
    // analogDetector<uint16_t>(det,1,NULL,1000);
#ifndef INTERP
    singlePhotonDetector *filter = new singlePhotonDetector(
        det, 3, nSigma, 1, cm, 1000, 100, -1, -1, gainmap, gs);

    multiThreadedCountingDetector *mt =
        new multiThreadedCountingDetector(filter, nthreads, fifosize);

    // multiThreadedAnalogDetector *mt=new
    // multiThreadedAnalogDetector(filter,nthreads,fifosize);
#endif
#ifdef INTERP
    eta2InterpolationPosXY *interp = new eta2InterpolationPosXY(
        npx, npy, nSubPixelsX, nSubPixelsY, etabins, etabinsy, etamin, etamax);

    //if (etafname)
    interp->readFlatField(etafname.c_str());

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

    // receive socket
    try {
      zmqsocket = new sls::ZmqSocket(socketip.c_str(), portnum);
    } catch (...) {
        cprintf(RED,
                "Error: Could not create Zmq receiving socket on port %d with ip %s\n",
                portnum, socketip.c_str());
        delete zmqsocket;
        return EXIT_FAILURE;
    }
    if (zmqsocket->Connect()) {
        cprintf(RED, "Error: Could not connect to Zmq receiving socket  %s\n",
                (zmqsocket->GetZmqServerAddress()).c_str());
        delete zmqsocket;
        return EXIT_FAILURE;
    } else
        printf("Zmq receiving socket at %s\n", zmqsocket->GetZmqServerAddress().c_str());

    // send socket
    sls::ZmqSocket *zmqsocket2 = 0;
    // cout << "zmq2 " << endl;
    if (send) {
        // receive socket
        try {
	        zmqsocket2 = new sls::ZmqSocket(portnum2);
        } catch (...) {
            cprintf(RED,
                    "Error: Could not create Zmq sending socket on port %d\n",
                    portnum2);
            //	delete zmqsocket2;
            //	zmqsocket2=NULL;
            //	delete zmqsocket;
            //	return EXIT_FAILURE;
            send = false;
        }
        printf("Zmq sending socket at %s\n",
                   zmqsocket2->GetZmqServerAddress().c_str());
    }

    // header variables
    uint64_t acqIndex = -1;
    uint64_t frameIndex = -1;
// #ifdef MOENCH_BRANCH
//     uint32_t subFrameIndex = -1;
//     int *flippedData = 0;
// #endif

    uint64_t subframes = 0;
    // uint64_t  isubframe=0;
    uint64_t insubframe = 0;
    double subnorm = 1;
    uint64_t f0 = -1, nsubframes = 0, nnsubframe = 0;

    uint64_t fileindex = -1;
    string filename = "";
    //	char* image = new char[size];
    // int* image = new int[(size/sizeof(int))]();
    // uint32_t flippedDataX = -1;
    // int *nph;
    int iframe = 0;
    char ofname[10000];

    string fname;
    //	int length;
    int *detimage = NULL;
    int nnx, nny, nnsx, nnsy;
    // uint32_t imageSize = 0, nPixelsX = 0, nPixelsY = 0,
    // uint32_t  dynamicRange = 0;
    //  infinite loop
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
                            // if (ix<100*400)
                            //   cout << ix << " " << ped[ix] << endl;
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

                        //  zmqsocket2->SendHeaderData (0,
                        //  false,SLS_DETECTOR_JSON_HEADER_VERSION , dr,
                        //  fileindex, 1,1,nnx,nny,nnx*nny*dr/8,acqIndex,
                        //  frameIndex, fname,acqIndex,0 , packetNumber,detSpec1,
                        //  timestamp, modId,xCoord, yCoord, zCoord,detSpec3,
                        //  detSpec4, detType, version, 0,0,
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

             if (addJsonHeader.find("xMin") != addJsonHeader.end()) {
                 istringstream(addJsonHeader.at("xMin")) >> xmin;
             }
 
             if (addJsonHeader.find("yMin") != addJsonHeader.end()) {
                 istringstream(addJsonHeader.at("yMin")) >> ymin;
             }
 
             if (addJsonHeader.find("xMax") != addJsonHeader.end()) {
                 istringstream(addJsonHeader.at("xMax")) >> xmax;
             }
 
             if (addJsonHeader.find("yMax") != addJsonHeader.end()) {
                 istringstream(addJsonHeader.at("yMax")) >> ymax;
             }



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
        if (packetNumber <= 50) {
            //*((int*)buff)=frameIndex;
            if (insubframe == 0)
                f0 = frameIndex;
            memcpy(buff, &frameIndex, sizeof(int));
            // length =
            zmqsocket->ReceiveData(0, buff + sizeof(int), size);

            if (fMode != ePedestal || dMode != eAnalog) {
                if (of == NULL) {
#ifdef WRITE_QUAD
                    sprintf(ofname, "%s_%ld.clust2", filename.c_str(),
                            fileindex);
#endif
#ifndef WRITE_QUAD
                    sprintf(ofname, "%s_%ld.clust", filename.c_str(),
                            fileindex);
#endif
                    of = fopen(ofname, "w");
                    if (of) {
                        mt->setFilePointer(of);
                    } else {
                        cout << "Could not open " << ofname << " for writing "
                             << endl;
                        mt->setFilePointer(NULL);
                    }
                }
            }

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

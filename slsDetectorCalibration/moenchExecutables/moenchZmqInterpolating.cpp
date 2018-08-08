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
#include "interpolatingDetector.h"
#include "linearInterpolation.h"
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

  int nthreads=20;
  int nsigma=5;
  int xmin=0;
  int xmax=400;
  int ymin=0;
  int ymax=400;
  int nsubpixels=2;


  FILE *of=NULL;
  int fifosize=1000;
  int int_ready=0;
  int ok;
  // help
  if (argc < 3 ) {
    cprintf(RED, "Help: %s [receive socket ip] [receive starting port number] [send_socket ip] [send starting port number] [nsubpixels] [nthreads] [nsigma] [xmin xmax ymin ymax]\n", argv[0]);
    return EXIT_FAILURE;
  }

  char* socketip2 = 0;
  uint32_t portnum2 = 0;
	// receive parameters
  int size = 32*2*5000;//atoi(argv[3]);
  bool send = false;



  char* socketip=argv[1];
  uint32_t portnum = atoi(argv[2]);
  if (argc > 3) {
    send = true;
    socketip2 = argv[3];
    portnum2 = atoi(argv[4]);
  }
  if (argc > 5) {
    nsubpixels=atoi(argv[5]);
  }
  if (argc>6) {
    nthreads=atoi(argv[6]);
  }
  if (argc>7) {
    nsigma=atoi(argv[7]);
  }
  if (argc>11) {
    xmin=atoi(argv[8]);
    xmax=atoi(argv[8]);
    ymin=atoi(argv[10]);
    ymax=atoi(argv[11]);
  }

  cout << "\nrx socket ip : " << socketip <<
    "\nrx port num  : " <<  portnum ;
  if (send) {
    cout << "\nsd socket ip : " << socketip2 <<
      "\nsd port num  : " <<  portnum2;
	}
  cout << endl;

  //slsDetectorData *det=new moench03T1ZmqDataNew(); 
  int npx, npy;
  moench03T1ZmqDataNew *det=new moench03T1ZmqDataNew(); 
  det->getDetectorSize(npx, npy);
  linearInterpolation *interp=new linearInterpolation(npx,npy,nsubpixels);
  interpolatingDetector *filter=new interpolatingDetector(det,interp, nsigma, 1, 0, 1000, 100,npx,npy);
  cout << "Setting noise cut to " << nsigma << " sigma"<< endl;
  filter->setROI(xmin,xmax,ymin,ymax);
  cout << "Setting ROI to "<< xmin << " " << xmax << " " << ymin << " " << ymax << endl;
  
  char* buff;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
	  int frameMode=eFrame;
	  mt->setFrameMode(frameMode);
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
	char ffname[10000];
	int ffindex;

	char* image = new char[size];
	//int* image = new int[(size/sizeof(int))]();

	int *nph;
	int iframe=0;
	char ofname[10000];
		    
	char fname[10000];
	int length;
	int *detimage;
	int nnx, nny,nns;
	int nix, niy,nis;
	filter->getImageSize(nnx, nny,nns);
	int16_t *dout=new int16_t [nnx*nny];
	// infinite loop
	int ix, iy, isx, isy;
	while(1) {


	  //  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
		// get header, (if dummy, fail is on parse error or end of acquisition)
	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)){
	    //	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)) {
	    // cprintf(RED, "Got Dummy\n");
	    while (mt->isBusy()) {;}//wait until all data are processed from the queues
	    if (frameMode==ePedestal) {	
	      detped=mt->getPedestal();
 	      if (detped) {
		
 		for (ix=0; ix<400; ix++) {
 		  for (iy=0; iy<400; iy++) {
 		    dout[iy*400+ix]+=detped[iy*400+ix];
		  }
		}
	      }

	    } else {	      
 	      detimage=mt->getImage(nix,niy,nis);
 	      if (detimage) {
 		for (ix=0; ix<nix/nis; ix++) {
                   for (iy=0; iy<niy/nis; iy++) {
                     dout[iy*(nix/nis)+ix]=0;
 		  }
 		}
 		for (ix=0; ix<nix; ix++) {
 		  for (iy=0; iy<niy; iy++) {
 		    dout[(iy/nis)*(nix/nis)+(ix/nis)]+=detimage[iy*nix+ix];
		  }
		}
	      }
	    }
	  	    
	     
	   
	   
	    if (send) {
	      strcpy(fname,filename.c_str());
	      //  zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,16,fileindex,400,400,400*400, acqIndex,frameIndex,fname, acqIndex, 0,0,0,0,0,0,0,0,0,0,0,1);
	      zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
	      
	      zmqsocket2->SendData((char*)dout,length);
	      cprintf(GREEN, "Sent Data\n");
	    }
	    
	    sprintf(ofname,"%s_%d.tiff",ffname,ffindex);
	    if (frameMode==eFlat)
	      mt->writeFlatField(ofname); 
	    else if (frameMode==ePedestal)
	      mt->writePedestal(ofname);
	    else
	      mt->writeImage(ofname); 
	    
	  
	    // stream dummy  to socket2 to signal end of acquisition
	    if (send) {
	      zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	      // cprintf(RED, "Sent Dummy\n");
	      cprintf(RED, "Received %d frames\n", iframe);
	    }
	    mt->clearImage();
	    if (of) {
	      fclose(of);
	      of=NULL;
	    }
	    iframe=0;
	    continue; //continue to not get out
	  }


	  if (of==NULL) {  
	    while (mt->isBusy()) {;}
	    sprintf(ofname,"%s_%d.clust",filename.c_str(),fileindex);
	    of=fopen(ofname,"w");
	    if (of) {
	      mt->setFilePointer(of);
	    }else {
	      cout << "Could not open "<< ofname << " for writing " << endl;
	      mt->setFilePointer(NULL);
	    }
	    ffindex=fileindex;
	    strcpy(ffname,filename.c_str());
	    if (filename.find("flat")!=std::string::npos) {
	      cout << "add to ff" << endl;
	      frameMode=eFlat;//ePedestal;
	      int_ready=0;
	      
	    } else if  (filename.find("newped")!=std::string::npos) {
	      frameMode=ePedestal;
	      cout << "new pedestal" << endl;
	      
	      mt->newDataSet();
	    } else if  (filename.find("ped")!=std::string::npos){ 
	      frameMode=ePedestal;
	      cout << "pedestal" << endl;
	    } else { 
	      frameMode=eFrame;
	      cout << "data" << endl;
	      if (int_ready==0) {
		mt->prepareInterpolation(ok);
		cout << "prepare interpolation " << endl;
		int_ready=1;
	      }
	    }
	   
	    mt->setFrameMode(frameMode);
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

	
	//	cout<<"Goodbye"<<  endl;
	return 0;
}


//#define ROOTSPECTRUM

#include "multiThreadedAnalogDetector.h"

#include "sls_receiver_defs.h"
#include "ZmqSocket.h"
#include "moench03T1ZmqDataNew.h"

#ifdef ROOTSPECTRUM
#include <TPaveText.h> 
#include <TLegend.h> 
#include <TF1.h> 
#include <TGraphErrors.h> 
#include <TH2F.h> 
#include <TASImage.h> 
#include <TImage.h> 
#include <TFile.h> 
#endif

#include <vector> 
#include <string> 
#include <sstream> 
#include <iomanip> 
#include <fstream> 
#include "tiffIO.h"

#include<iostream>

//#include "analogDetector.h"
#include "singlePhotonDetector.h"
#include "ansi.h"
#include <iostream>
using namespace std;


#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [send_socket ip] [send port number]
 *
 */
  int fifosize=1000;
  int nthreads=20;
  char* buff;
  char tit[10000];
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
	 analogDetector<uint16_t> *filter=new analogDetector<uint16_t>(det,1,NULL,1000);
	 //singlePhotonDetector *filter=new singlePhotonDetector(det,3, 5, 1, 0, 1000, 10);
	 //filter->setROI(250, 400, 30, 150);
	 float threshold=1;
	if (argc > 5) {
	  threshold=atof(argv[5]);
	 filter->setThreshold(threshold);
	  cout << "Threshold set to " << threshold << endl;
	}

	int nnx, nny, nns;
	int imsize=filter->getImageSize(nnx,nny,nns);

  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);



  // mt->prepareInterpolation(ok);
  mt->StartThreads();
  mt->popFree(buff);



	  cout << "det " << endl;
	int16_t dout[400*400];
	double ddark[400*400];
	  cout << "dout " << endl;

	// receive socket
	  ZmqSocket* zmqsocket = new ZmqSocket(socketip,portnum); 
#ifdef ROOTSPECTRUM
	  TH2F *h2=NULL;
	  TH2F *hmap=NULL;
	  TFile *froot=NULL;

	  h2=new TH2F("hs","hs",500,-500,500,400*400,-0.5,400*400-0.5);
	  hmap=new TH2F("hmap","hmap",400,-0.5,400-0.5,400,-0.5,400-0.5);
#endif
	  cout << "zmq1 " << endl;
	  
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
		//	zmqsocket2->Disconnect();
	}


	// header variables
	uint64_t acqIndex = -1;
	uint64_t frameIndex = -1;
	uint32_t subframeIndex = -1;
	uint64_t fileindex = -1;
	string filename = "";
	//char* image = new char[size];
	//int* image = new int[(size/sizeof(int))]();

	int *nph;//[400*400];
	int iframe=0;
	char rootfname[10000];
		    
	char fname[10000];
	char ff[10000];
	int fi;
	int length;
	char newped=-1, ped=-1, dat=-1, isdark=-1;
	
	double *peds;
	int *im;
	int fnumber;
	float *gm=new float[400*400];
	// infinite loop
	while(1) {
	  

	  //	  cout << "+++++++++++++++++++++++++++++++LOOP" << endl;
		// get header, (if dummy, fail is on parse error or end of acquisition)
	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)){
	    //	  if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename, fileindex)) {
			cprintf(RED, "Got Dummy\n");
			while (mt->isBusy()) {;}//wait until all data are processed from the queues
			if (dat==1) {
			  
			  sprintf(tit,"%s_%lld.tiff",filename.c_str(),fi);
			  cout << tit << endl;
			  im=mt->getImage(nnx,nny,nns);
			  if (isdark) cout << "getting dark "<< endl;
			  else cout << "subtracting dark"<< endl;
			  if (gm) {
			    for (int ii=0; ii<400*400; ii++) {
			      //      if (image[ix]>0) cout << ix << " " << image[ix]<< endl;
			      if (isdark) {
				ddark[ii]=(double)im[ii]/((double)iframe);
				if (ddark[ii]>0) cout << "*" ;
				gm[ii]=im[ii];
				if (send) dout[ii]=im[ii];
			      } else {
				gm[ii]=im[ii];//-ddark[ii]*iframe;
				if (gm[ii]<0) gm[ii]=0;
				if (send) dout[ii]=gm[ii];
			      }
			      //cout << endl;
			      
			    }
			    cout << endl;
			    //cout << "image " << nnx << " " << nny << endl;
			    WriteToTiff(gm,tit ,nnx, nny); 
			    // delete [] gm;
			  } else cout << "Could not allocate float image " << endl;
     


			} else {

			  sprintf(tit,"%s_%lld.tiff",filename.c_str(),fi);
			  cout << tit << endl;
			  mt->writePedestal(tit);

			}

			// mt->writeImage(tit);
			//  cout << "wrote" << endl;
	
	 
	 




	 
			  if (send) {
			  if (dat==1) {
			    //im=mt->getImage(nnx,nny,nns);
			    
			    //if (im)
			    //  cout << "got image" << endl;
			    //else
			    //  cout << "could not get image" << endl;
			    

			    //for (int ii=0; ii<400*400; ii++) {
			      //if (im[ii]>0)
			      //cout << im[ii] << endl;
			      //  if (im[ii]>=0)
			      
			    // if (isdark) {
				//ddark[ii]=im[ii];
			    //	dout[ii]=im[ii];
			    // } else {
			    //	dout[ii]=im[ii]-ddark[ii];
			    //	if (dout[ii]<0) dout[ii]=0;
			    // }
			      // else
			      //	dout[ii]=0;
				//else
				//dout[ii]=0;
			      // cout << im[ii] << " " << dout[ii] << endl;
			    //  }
			    //for (int iiy=49; iiy<52; iiy++)
			    //  for (int iix=80; iix<83; iix++)
			    //	dout[iiy*400+iix]=0;
			    ;
			  } else {
			    peds=mt->getPedestal();
			    //  sprintf(tit,"%s_%lld.tiff",filename.c_str(),fi);
			    //cout << tit << endl;
			    //mt->writePedestal(tit);
			    if (peds)
			      cout << "got peds" << endl;
			    else
			      cout << "could not get peds" << endl;
			    for (int ii=0; ii<400*400; ii++) {
			      dout[ii]=peds[ii];
			      // if (ii%400==10 && ii/400==10)
			      // 	cout << ii/400 << " " << ii%400 << " " << peds[ii] << " " << dout[ii] << endl; 
			      // if (ii%400==100 && ii/400==100)
			      // 	cout << ii/400 << " " << ii%400 << " " << peds[ii] << " " << dout[ii] << endl; 
			    }

			  }

			    
		// zmqsocket2 = new ZmqSocket(portnum2, socketip2);
		// if (zmqsocket2->IsError()) {
		// 	bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
		// 	delete zmqsocket2;
		// 	delete zmqsocket;
		// 	return EXIT_FAILURE;
		// }
		// zmqsocket2->Connect();
		// printf("Zmq Server started at %s\n", zmqsocket2->GetZmqServerAddress());
			  //zmqsocket2->Connect();

	
			  zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,ff, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
			  
			  
			  zmqsocket2->SendData((char*)dout,length);
			  cprintf(GREEN, "Sent Data %d \n",length);
			  
			  zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
			  cprintf(RED, "Sent Dummy\n");
			  //	zmqsocket2->Disconnect();

			  // delete zmqsocket2;
			  }
			  cout << "Acquired "<< iframe << " frames " << endl;
			  iframe=0;
			  newped=-1;
			  ped=-1;
			  dat=-1;
			  mt->clearImage();
			  isdark=0;
			  continue; //continue to not get out
	  }
	  //	cprintf(GREEN, "Got Header \n");

	  strcpy(ff,filename.c_str());
	  fi=fileindex;
	  //isdark=0;
	  if (newped<0) {
	    if (filename.find("newped")!=std::string::npos) {
	      cout << "NEWPED" << endl;
	      if (newped<=0) {
		newped=1;
		ped=1;
		while (mt->isBusy()) {;}
		mt->newDataSet(); //resets pedestal
		mt->setFrameMode(ePedestal);
		cout << "New data set"<< endl;
	      }
	    } else {
	      newped=0;
	    }
	  }
	  if (ped<0) { 
	    if (filename.find("ped")!=std::string::npos) {
	      ped=1;
	      dat=0;
	      while (mt->isBusy()) {;}
	      mt->setFrameMode(ePedestal);
	      cout << "pedestal!"<< endl;
	    } else {
	      ped=0;
	      dat=1;	
	      while (mt->isBusy()) {;}
	      mt->setFrameMode(eFrame);
	      cout << "data!"<< endl;
	      if (filename.find("dark")!=std::string::npos) {
		isdark=1;
		cout << "this is a dark image" << endl;
	      }
	      
	    }
	  }
	  
		// get data
	  length = zmqsocket->ReceiveData(0, buff, size);
		//	cprintf(GREEN, "Got Data\n");

		//processing with image
		//...
		//	if (iframe<10) {
		// filter->addToPedestal(image);
		//} else {
		//SLOW!!!
		//***
		//filter->getNPhotons(image);
		//nph=filter->getImage();
		//filter->addToPedestal(image);
		//*****
	 
		//	cprintf(BLUE, "Data processed\n");
	  

		mt->pushData(buff);
		mt->nextThread();
		//	cout << " " << (void*)buff;
		mt->popFree(buff);
		
	  
	  
		//stream data from socket 2
		  
	  iframe++;
	}
	
	//	}// exiting infinite loop
	

	
	delete zmqsocket;
	if (send)
	  delete zmqsocket2;
	

	cout<<"Goodbye"<<  endl;
	return 0;
}


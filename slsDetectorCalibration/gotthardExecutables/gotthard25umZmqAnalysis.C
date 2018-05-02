#include "sls_receiver_defs.h"
//#ifdef ZMQ
#include "ZmqSocket.h"
//#endif
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
//#include <deque>
//#include <list>
//#include <queue>
#include <fstream>
#include <cstring>

//#define BCHIP074_BCHIP075

#include "gotthardModuleDataNew.h"
#include "gotthardDoubleModuleDataNew.h"
#include "gotthardDoubleModuleCommonModeSubtractionNew.h"

#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"
//#include "linearInterpolation.h"
#include "multiThreadedAnalogDetector.h"

#include <ctime>

#define NC 1280
#define NR 1

//#include "tiffIO.h"





#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2
int main(int argc, char *argv[]){
  //void *gotthardProcessFrame() {



  int fifosize=1000;
  int nthreads=1;
  int nph, nph1;
  int etabins=550;
  double etamin=-1, etamax=2;
  int nsubpix=1;
  float *etah=new float[etabins*etabins];
  int *heta, *himage;
  int offset=48;
  #ifdef ZMQ
  offset=0;
  #endif
  #ifndef ZMQ
  offset=48;
  #endif
 
  //commonModeSubtraction *cm=NULL;

  gotthardDoubleModuleCommonModeSubtraction *cm=new gotthardDoubleModuleCommonModeSubtraction();
  gotthardModuleDataNew *decoder=new   gotthardModuleDataNew();
  gotthardDoubleModuleDataNew *det=new   gotthardDoubleModuleDataNew(offset);
  singlePhotonDetector *filter=new singlePhotonDetector(det,3, 5, 1, cm, 1000, 100);
  // analogDetector<uint16_t> *filter=new analogDetector<uint16_t>(det, 1, cm, 1000);
  // analogDetector<uint16_t> *filter_nocm=new analogDetector<uint16_t>(det, 1, NULL, 1000);
  filter->setROI(0,2560,0,1);
  char *buff;//[2*(48+1280*2)];
  char *buff0;
  char *buff1;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
  mt->setFrameMode(eFrame);
  // mt->setFrameMode(eFrame);
  // mt->setFrameMode(ePedestal);
  mt->StartThreads();
  mt->popFree(buff);
  buff0=buff;
  buff1=buff+offset*2+1280*2;
  int photons[1280*2];
  int nf=0;
  int ok=0;
  std::time_t end_time;
  //int16_t dout[1280*2];
  int iFrame=-1;
  int np=-1;
  nph=0;
  nph1=0;
  //int np;
  int iph;
  int data_ready=1;
  int *image;



	int length;
	int nnx, nny,nns;
	int nix, niy,nis;
	// infinite loop
	int ix, iy, isx, isy;

  filter->getImageSize(nnx, nny,nns);
  int16_t *dout=new int16_t [nnx*nny];



#ifdef ZMQ
	if (argc < 3 ) {
	  cprintf(RED, "Help: %s [receive socket ip] [receive starting port number] [send_socket ip] [send starting port number]\n",argv[0]);
		return EXIT_FAILURE;
	}

	// receive parameters
	bool send = false;
	  char* socketip=argv[1];
	uint32_t portnum = atoi(argv[2]);
	int size = nnx*nny*2;

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



	// receive socket
	  ZmqSocket* zmqsocket0 = new ZmqSocket(socketip,portnum); 
	if (zmqsocket0->IsError()) {
	  cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum, socketip);
	  delete zmqsocket0;
	  return EXIT_FAILURE;
	}

	  ZmqSocket* zmqsocket1 = new ZmqSocket(socketip,portnum+1); 
	if (zmqsocket1->IsError()) {
	  cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum+1, socketip);
	  delete zmqsocket1;
	  delete zmqsocket0;
	  return EXIT_FAILURE;
	}


	zmqsocket0->Connect();
	printf("Zmq Client 0 at %s\n", zmqsocket0->GetZmqServerAddress());
	zmqsocket1->Connect();
	printf("Zmq Client 1 at %s\n", zmqsocket1->GetZmqServerAddress());

	// send socket
	ZmqSocket* zmqsocket2 = 0;
	ZmqSocket* zmqsocket3 = 0;
	if (send) {
	  zmqsocket2 = new ZmqSocket(portnum2, socketip2);
	  if (zmqsocket2->IsError()) {
	    bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
	    delete zmqsocket2;
	    delete zmqsocket1;
	    delete zmqsocket0;
	    return EXIT_FAILURE;
	  }
	  
	  zmqsocket3 = new ZmqSocket(portnum2+1, socketip2);
	  if (zmqsocket3->IsError()) {
	    bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2+1, socketip2);
	    delete zmqsocket3;
	    delete zmqsocket2;
	    delete zmqsocket1;
	    delete zmqsocket0;
	    return EXIT_FAILURE;
	  }
	  
	  zmqsocket2->Connect();
	  printf("Zmq Server 0 started at %s\n", zmqsocket2->GetZmqServerAddress());
	  zmqsocket3->Connect();
	  printf("Zmq Server 1 started at %s\n", zmqsocket3->GetZmqServerAddress());
	}







	uint64_t acqIndex1 = -1;
	uint64_t frameIndex1 = -1;
	uint32_t subframeIndex1 = -1;
	uint64_t fileindex1 = -1;
	string filename1 = "";

	uint64_t acqIndex0 = -1;
	uint64_t frameIndex0 = -1;
	uint32_t subframeIndex0 = -1;
	uint64_t fileindex0 = -1;
	string filename0 = "";


	int eoa0=0;
	int eoa1=0;





  #endif












	char ofname[10000];
	char fn0[10000], fn1[10000];
	FILE *fout=NULL;
	FILE *fclust=NULL;
	for (int i=0; i<nnx; i++)
	  dout[i]=0;
	char fname0[10000], fname1[10000];
	int irun;

#ifndef ZMQ

	char ff[10000];
  if (argc < 6 ) {
    cprintf(RED, "Help: %s [indir] [fformat] [runmin] [runmax] [out file format]\n",argv[0]);
    return EXIT_FAILURE;
  }
  char *indir=argv[1];
  char *fformat=argv[2];
  int runmin=atoi(argv[3]);
  int runmax=atoi(argv[4]);
  char *outdir=argv[5];
  sprintf(ff,"%s/%s",indir,fformat);
  // strcpy(fformat,"/external_pool/gotthard_data/datadir_gotthardI/bchip074075/20170731/Xray/xray_15kV_200uA_5us_d%d_f000000000000_0.raw");
  // sprintf(fname0,fformat,0,0);
  // sprintf(fname1,fformat,1,1);
  
  ifstream filebin0,filebin1;
  for (irun=runmin; irun<runmax; irun++) {

    sprintf(fname0,ff,0,irun);
    sprintf(fname1,ff,1,irun);
    sprintf(ofname,outdir,irun);

    filebin0.open((const char *)(fname0), ios::in | ios::binary);
    filebin1.open((const char *)(fname1), ios::in | ios::binary);
    if (filebin0.is_open() && filebin1.is_open()) {
      cout << "Opened file " << fname0<< endl;
      cout << "Opened file " << fname1<< endl;
      nf=0;
      iFrame=-1;
      while ((decoder->readNextFrame(filebin0, iFrame, np, buff0)) && (decoder->readNextFrame(filebin1, iFrame, np, buff1))) {
#endif





#ifdef ZMQ

	int end_of_acquisition;
	while(1) {
	  end_of_acquisition=0;
	  eoa0=0;
	  eoa1=0;
	  
	  //  cout << "Receive header " << nf << endl;
	  if (!zmqsocket0->ReceiveHeader(0, acqIndex0, frameIndex0, subframeIndex0, filename0, fileindex0)) {
	    
	    //   cout << "************************************************************************** packet0!*****************************"<< endl;
	    eoa0=1;
	    end_of_acquisition++;
	  } 
	  if (!zmqsocket1->ReceiveHeader(0, acqIndex1, frameIndex1, subframeIndex1, filename1, fileindex1)) {
	    //cout << "************************************************************************** packet1!*****************************"<< endl;
	    eoa1=1;
	    end_of_acquisition++;
	  }





	  //	  if ((!zmqsocket0->ReceiveHeader(0, acqIndex0, frameIndex0, subframeIndex0, filename0, fileindex0)) && (!zmqsocket1->ReceiveHeader(0, acqIndex1, frameIndex1, subframeIndex1, filename1, fileindex1))){
	  if (end_of_acquisition==0) {
	    
	    if (acqIndex0!=acqIndex1)
	      cout << "different acquisition indexes " << acqIndex0 << " and " << acqIndex1 << endl;
	    if (frameIndex0!=frameIndex1)
	      cout << "different frame indexes " << frameIndex0 << " and " << frameIndex1 << endl;
	    
	    
	    while (frameIndex0<frameIndex1) {
	      cout << "aligning det 0 " << endl; 
	      length = zmqsocket0->ReceiveData(0, buff0, size/2);
	      if (!zmqsocket0->ReceiveHeader(0, acqIndex0, frameIndex0, subframeIndex0, filename0, fileindex0)) {
		end_of_acquisition++;
		eoa0=1;
		break;
	      } 
	    }
	    
	    while (frameIndex1<frameIndex0) {
	      cout << "aligning det 1 " << endl; 
	      length = zmqsocket1->ReceiveData(0, buff1, size/2);
	      if (!zmqsocket1->ReceiveHeader(0, acqIndex1, frameIndex1, subframeIndex1, filename1, fileindex1)) {
		end_of_acquisition++;
		eoa1=1;
		break;
	      } 
	    }
	  }



	  if (eoa0!=eoa1) {

	    while (eoa0<1) {
	      length = zmqsocket0->ReceiveData(0, buff0, size/2);
	      if (!zmqsocket0->ReceiveHeader(0, acqIndex0, frameIndex0, subframeIndex0, filename0, fileindex0)) {
		end_of_acquisition++;
		eoa0=1;
	      }
	    }


	    while (eoa1<1) {
	      length = zmqsocket1->ReceiveData(0, buff1, size/2);
	      if (!zmqsocket1->ReceiveHeader(0, acqIndex1, frameIndex1, subframeIndex1, filename1, fileindex1)) {
		end_of_acquisition++;
		eoa1=1;
	      }
	    }
	  }

	  



	  
	  if (end_of_acquisition) {
	    // cout << "************************************************************************** END OF FRAME" << end_of_acquisition << " !*****************************"<< endl;
	    //  return 0;
	    
	    sprintf(ofname,"%s_%d.ph",fn0,irun);
	    while (mt->isBusy()) {;}
	    image=filter->getImage();
	    if (image) {
	      fout=fopen(ofname,"w");
	      cout << nf << "*****************" << endl;
	      for (int i=0; i<2560; i++) {
		fprintf(fout,"%d %d\n",i,image[i]);
		dout[i]=image[i];
		if (dout[i]<0)
		   dout[i]=0;
	      }
	      fclose(fout);
	    }
	    

	    if (send) {
	      
	      zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fn0, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
	      zmqsocket3->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fn1, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
	      
	      zmqsocket2->SendData((char*)dout,size/2);
	      zmqsocket3->SendData(((char*)dout)+size/2,size/2);
	      //    	//	cprintf(GREEN, "Sent Data\n");

	      
	      zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	      zmqsocket3->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	    }
	    
	    
	    //mt->setFrameMode(eFrame);
	    filter->clearImage();
	    // std::time(&end_time);
	    // cout << std::ctime(&end_time) << " " << nf <<   endl;
	    fclose(fclust);
	    fclust=NULL;
	    
	    
	    
	    continue; 
	  }
	  
	  if (fclust==NULL) {

	    strcpy(fn0,filename0.c_str());
	    strcpy(fn1,filename1.c_str());
	    sprintf(ofname,"%s_%d.clust",fn0,irun);
	    fclust=fopen(ofname,"w");
	    while (mt->isBusy()) {;}
	    mt->setFilePointer(fclust);
	  }

	  // strcpy(fn0,filename0.c_str());
	  // strcpy(fn1,filename1.c_str());
	  
	  //	  cout << "Receive data " << nf << endl;
	  length = zmqsocket0->ReceiveData(0, buff0, size/2);
	  length += zmqsocket1->ReceiveData(0, buff1, size/2);  
	  
	  irun=fileindex0;
	  
	  
	    
	    // //  if (nf>100)
	    // //  mt->setFrameMode(eFrame);
	    // //filter->clearImage();


#endif


	mt->pushData(buff);
	mt->nextThread();
	// cout << "==" << nf << endl;

	    // while (mt->isBusy()) {;}
	    // image=filter->getImage();
	    // if (image) {
	    //   for (int i=0; i<2560; i++) {
	    // 	//	if (i<512)
	
	    // 	//  	fprintf(fout,"%d %d\n",i,image[i]);
	    //   	dout[i]=filter->subtractPedestal(buff,i,0,1);//image[i];//filter->getPedestal(i,0);//
	    // 	if (dout[i]<0)
	    // 	   dout[i]=0;
	    // 	//	cout << i << " " << image[i] << " " << dout[i] << endl;
	    //   }
	    // }


	    // if (send) {
	    // 	strcpy(fname0,filename0.c_str());
	    // 	strcpy(fname1,filename1.c_str());
	    // 	//  zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,16,fileindex,400,400,400*400, acqIndex,frameIndex,fname, acqIndex, 0,0,0,0,0,0,0,0,0,0,0,1);
	    // 	zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname0, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
	    // 	zmqsocket3->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,0,0,0,0,0, 0,0,fname1, 0, 0,0,0,0,0,0,0,0,0,0,0,1);
		
	    // 	zmqsocket2->SendData((char*)dout,size/2);
	    // 	zmqsocket3->SendData(((char*)dout)+size/2,size/2);
	    // 	//	cprintf(GREEN, "Sent Data\n");


	    // 	// zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
	    // 	// zmqsocket3->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);

	    // 	//	cprintf(RED, "Received %d frames\n", nf);

	    // }
	    

	mt->popFree(buff);
	buff0=buff;
	buff1=buff+offset*2+1280*2;
	  


	nf++;
	



#ifndef ZMQ


	 

	  while (mt->isBusy()) {;}
	  image=filter->getImage();
	  if (image) {
	      fout=fopen(ofname,"w");
	      //cout << nf << "*****************" << endl;
	      for (int i=0; i<512; i++) {
		fprintf(fout,"%d %d\n",i,image[i]);
	      }
	      fclose(fout);
	  }
	  filter->clearImage();
      


	  iFrame=-1;
      }
      
      filebin0.close();
      filebin1.close();
    }
    else
      cout << "Could not open file " << fname0<< " or " << fname1 <<  endl;
#endif

    }
  return NULL;



}

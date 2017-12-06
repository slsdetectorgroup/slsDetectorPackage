#include "ZmqSocket.h"

#include "ansi.h"
#include <iostream>

#include "moench03T1ZmqData.h"

// #include "interpolatingDetector.h"
//#include "etaInterpolationPosXY.h"
// #include "linearInterpolation.h"
// #include "noInterpolation.h"
#include "multiThreadedAnalogDetector.h"
#include "singlePhotonDetector.h"
#include "interpolatingDetector.h"


using namespace std;
#define NC 400
#define NR 400


#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */

  int p=10000;
  int fifosize=1000;
  int nthreads=20;
  int nsubpix=25;
  int etabins=nsubpix*10;
  double etamin=-1, etamax=2;
  int csize=3;
  int nx=400, ny=400;
  int save=1;
  int nsigma=5;
  int nped=1000;
  int ndark=100;
  int ok;
  int iprog=0;
  char outfname[10000];


  moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  // cout << "decoder "<< endl;
  //etaInterpolationPosXY *interp=new etaInterpolationPosXY(nx, ny, nsubpix, etabins, etamin, etamax);
  // linearInterpolation *interp=new linearInterpolation(NC, NR, nsubpix);
  //noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);
  //interpolatingDetector *filter=new interpolatingDetector(decoder,interp, nsigma, 1, 0, nped, 100);
     singlePhotonDetector *filter=new singlePhotonDetector(decoder,csize, nsigma, 1, 0, nped, 100);
  char tit[10000];




  // filter->readPedestals("/scratch/ped_100.tiff");
  // interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  // cout << "filter "<< endl;
  

	int size = 327680;////atoi(argv[3]);

	char* buff;
	int* image;
	//int* image =new int[327680/sizeof(int)];
  filter->newDataSet();



  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);


  //mt->setFrameMode(eFrame);
  mt->setFrameMode(eFlat);
  
  // mt->setFrameMode(eFrame); //need to find a way to switch between flat and frames!
  // mt->prepareInterpolation(ok);
  mt->StartThreads();
  mt->popFree(buff);


	// help
	if (argc < 2 || (argc > 2)) {
	  cprintf(RED, "Help: %s [receive socket ip]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// receive parameters
	bool send = false;
	char* socketip = argv[1];
	uint32_t portnum = 30001;//atoi(argv[2]);
	//	char fn[10000];
	//	strcpy(fn, argv[2]);
	// send parameters if any
	// char* socketip2 = 0;
	// uint32_t portnum2 = 0;
	// if (argc > 4) {
	// 	send = true;
	// 	socketip2 = argv[4];
	// 	portnum2 = atoi(argv[5]);
	// }
	cout << "\nrx socket ip : " << socketip ; // <<
	  // "\nrx port num  : " <<  portnum <<
	//	"\nsize         : " <<  size;
	// "\nfname  : " <<  fn ;
	// if (send) {
	// 	cout << "\nsd socket ip : " << socketip2 <<
	// 			"\nsd port num  : " <<  portnum2;
	// }
	cout << endl;






	// receive socket
	ZmqSocket* zmqsocket = new ZmqSocket(socketip,portnum);
	if (zmqsocket->IsError()) {
		cprintf(RED, "Error: Could not create Zmq socket on port %d with ip %s\n", portnum, socketip);
		delete zmqsocket;
		return EXIT_FAILURE;
	}
	printf("Zmq Client at %s\n", zmqsocket->GetZmqServerAddress());

	// send socket
	// ZmqSocket* zmqsocket2 = 0;
	// if (send) {
	// 	zmqsocket2 = new ZmqSocket(portnum2, socketip2);
	// 	if (zmqsocket2->IsError()) {
	// 		bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
	// 		delete zmqsocket2;
	// 		delete zmqsocket;
	// 		return EXIT_FAILURE;
	// 	}
	// 	printf("Zmq Server started at %s\n", zmqsocket2->GetZmqServerAddress());
	// }


	//	cout << "size " <<  327680/sizeof(int) << endl;

	// header variables
	uint64_t acqIndex = -1;
	uint64_t frameIndex = -1;
	uint32_t subframeIndex = -1;
	string filename = "";
	int nnx, nny, nns;
	int imsize=filter->getImageSize(nnx,nny,nns);
	//int imsize=nx*ny;
	int i_image=0;
	cout << "Image size is "<< nnx << " " << nny << " " << nns << " " << imsize << endl;
	int iframe=0;
	int ff=0;
	
	// infinite loop
	while(1) {


	  //	cprintf(GREEN, "Got ?\n");
		// get header, (if dummy, fail is on parse error or end of acquisition)
		if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename)) {
		  cprintf(RED, "Acquisition finished\n");

			

		  cout << "Received " << ff <<  " frames for a total of "<< iframe << endl;
			while (mt->isBusy()) {;}//wait until all data are processed from the queues
			
			// stream dummy  to socket2 to signal end of acquisition
			// if (send) {	
			//   zmqsocket2->SendData((char*)(mt->getImage()),imsize*sizeof(int));
			//   cprintf(BLUE, "Sent Interpolated image\n");
			// 	// zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
			// 	// cprintf(RED, "Sent Dummy\n");
			// }
			//	if (save) {
			sprintf(tit,"%s_%05d.tiff",filename.c_str(), ff);
			//	cout << tit << endl;
			 mt->writeImage(tit);
				 mt->clearImage();
			   //	}
			// sprintf(tit,"%s_%05d_eta_i.tiff",filename.c_str(), ff);
			// interp->writeFlatField(tit);
			// sprintf(tit,"%s_%05d_eta.tiff",filename.c_str(), ff);
			// cout << tit << endl;
			// mt->writeFlatField(tit);

			i_image++;
			ff=0;
			// dont get data
			continue; //continue to not get out
		}

		if (ff==0)
		  cprintf(GREEN, "Start acquisition \n");
		  //	  cout << filename << endl;
		//	cprintf(GREEN, "Got Header \n");
		// get data
		//	cprintf(GREEN, "Got Header\n");
		//image=(int*)buff;
		//cout << buff << endl;
		int length = zmqsocket->ReceiveData(0, (int*)buff, size);
		//	int length = zmqsocket->ReceiveData(0, (int*)image, size);
		//	cprintf(GREEN, "Got Data\n");

		//processing with image
		//...




		mt->pushData(buff);
		mt->nextThread();
		//	cout << " " << (void*)buff;
		mt->popFree(buff);
		
		
		iframe++;
		ff++;
		//  if (iframe%p==0) {
		  
		//   while (mt->isBusy()) {;}//wait until all data are processed from the queues 
		//   sprintf(tit,"tmp.tiff",filename.c_str(), i_image);
		//   mt->writeImage(tit);
		//   // mt->clearImage();
		//   //	}
		//   //interp->writeFlatField(tit);
		//   //  i_image++;
		//   //   sprintf(tit,"%s_tmp.tiff",fn);
		// //   mt->writeImage(tit);
		// //   //mt->clearImage();
		  
		// //   cout <<"*"<< iprog++ << endl;
		  
		//  }









		// //stream data from socket 2
		// if (send) {
		// 	zmqsocket2->SendHeaderData(0, false, SLS_DETECTOR_JSON_HEADER_VERSION,
		// 			0,0,0,acqIndex,frameIndex,(char*)"run", acqIndex, 0,0,0,0,0,0,0,0,0,0,0,1);
		// 	cprintf(GREEN, "Sent Header\n");

		// 	zmqsocket2->SendData((char*)image,length);
		// 	cprintf(GREEN, "Sent Data\n");
		// }


	}// exiting infinite loop



	delete zmqsocket;
	// if (send)
	// 	delete zmqsocket2;


	//	cout<<"Goodbye"<<  endl;
	return 0;
}


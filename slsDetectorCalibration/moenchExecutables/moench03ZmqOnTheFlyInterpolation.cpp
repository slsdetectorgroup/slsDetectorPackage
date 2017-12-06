#include "ZmqSocket.h"

#include "ansi.h"
#include <iostream>

#include "moench03T1ZmqData.h"

#include "interpolatingDetector.h"
#include "etaInterpolationPosXY.h"
#include "linearInterpolation.h"
#include "noInterpolation.h"
#include "multiThreadedDetector.h"

using namespace std;


#define SLS_DETECTOR_JSON_HEADER_VERSION 0x2


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [image size] [send_socket ip] [send port number]
 *
 */



  int nthreads=3;
  int nsubpix=5;
  int etabins=550;
  double etamin=-1, etamax=2;
  int nx=400, ny=400;
  int save=1;
  int nsigma=5;
  int nped=1000;
  int ok;
  //int* image = new int[(size/sizeof(int))]();
  char* buff;

  moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  // cout << "decoder "<< endl;
  etaInterpolationPosXY *interp=new etaInterpolationPosXY(nx, ny, nsubpix, etabins, etamin, etamax);
  //linearInterpolation *interp=new linearInterpolation(NC, NR, nsubpix);
  //noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);
  interpolatingDetector *filter=new interpolatingDetector(decoder,interp, nsigma, 1, 0, nped, 10);




  filter->readPedestals("/scratch/ped_100.tiff");
  interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  cout << "filter "<< endl;
  


  filter->newDataSet();



  multiThreadedDetector *mt=new multiThreadedDetector(filter,nthreads,100);



  mt->setFrameMode(eFrame); //need to find a way to switch between flat and frames!
  mt->prepareInterpolation(ok);
  mt->StartThreads();
  mt->popFree(buff);


	// help
	if (argc < 4 || (argc > 4 && argc != 6)) {
		cprintf(RED, "Help: ./trial [receive socket ip] [receive starting port number] [image size]  [send_socket ip] [send starting port number]\n");
		return EXIT_FAILURE;
	}

	// receive parameters
	bool send = false;
	char* socketip = argv[1];
	uint32_t portnum = atoi(argv[2]);
	int size = atoi(argv[3]);

	// send parameters if any
	char* socketip2 = 0;
	uint32_t portnum2 = 0;
	if (argc > 4) {
		send = true;
		socketip2 = argv[4];
		portnum2 = atoi(argv[5]);
	}
	cout << "\nrx socket ip : " << socketip <<
			"\nrx port num  : " <<  portnum <<
			"\nsize         : " <<  size;
	if (send) {
		cout << "\nsd socket ip : " << socketip2 <<
				"\nsd port num  : " <<  portnum2;
	}
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
	ZmqSocket* zmqsocket2 = 0;
	if (send) {
		zmqsocket2 = new ZmqSocket(portnum2, socketip2);
		if (zmqsocket2->IsError()) {
			bprintf(RED, "Error: Could not create Zmq socket server on port %d and ip %s\n",  portnum2, socketip2);
			delete zmqsocket2;
			delete zmqsocket;
			return EXIT_FAILURE;
		}
		printf("Zmq Server started at %s\n", zmqsocket2->GetZmqServerAddress());
	}




	// header variables
	uint64_t acqIndex = -1;
	uint64_t frameIndex = -1;
	uint32_t subframeIndex = -1;
	string filename = "";
	int imsize=nx*ny*nsubpix*nsubpix;
	int i_image=0;

	// infinite loop
	while(1) {


		// get header, (if dummy, fail is on parse error or end of acquisition)
		if (!zmqsocket->ReceiveHeader(0, acqIndex, frameIndex, subframeIndex, filename)) {
			cprintf(RED, "Acquisition finished\n");



			while (mt->isBusy()) {;}//wait until all data are processed from the queues
			
			
			// stream dummy  to socket2 to signal end of acquisition
			if (send) {	
			  zmqsocket2->SendData((char*)(mt->getInterpolatedImage()),imsize*sizeof(int));
			  cprintf(BLUE, "Sent Interpolated image\n");
				// zmqsocket2->SendHeaderData(0, true, SLS_DETECTOR_JSON_HEADER_VERSION);
				// cprintf(RED, "Sent Dummy\n");
			}
			if (save) {
			   char tit[10000];
			   sprintf(tit,"/scratch/int_image_%d.tiff",i_image);
			   mt->writeInterpolatedImage(tit);
			   mt->clearInterpolatedImage(tit);
			}
			i_image++;
			// dont get data
			break; //continue to not get out
		}
		cprintf(GREEN, "Got Header \n");
		// get data
		int length = zmqsocket->ReceiveData(0, (int*)buff, size);
		cprintf(GREEN, "Got Data\n");

		//processing with image
		//...





	mt->pushData(buff);
	mt->nextThread();
	//	cout << " " << (void*)buff;
	mt->popFree(buff);













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
	if (send)
		delete zmqsocket2;


	cout<<"Goodbye"<<  endl;
	return 0;
}


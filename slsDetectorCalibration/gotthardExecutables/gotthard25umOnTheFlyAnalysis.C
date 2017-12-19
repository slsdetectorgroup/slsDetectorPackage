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

#include "gotthardModuleDataNew.h"
#include "gotthardDoubleModuleDataNew.h"

#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"
//#include "linearInterpolation.h"
#include "multiThreadedAnalogDetector.h"

#include <ctime>

#define NC 1280
#define NR 1

#include "tiffIO.h"


void *gotthardProcessFrame() {





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
  


	















  char fname0[10000], fname1[10000];
  char fformat[10000];
  int fifosize=1000;
  strcpy(fformat,"/external_pool/gotthard_data/datadir_gotthardI/bchip074075/20170731/Xray/xray_15kV_200uA_5us_d%d_f000000000000_0.raw");
  sprintf(fname0,fformat,0,0);
  sprintf(fname1,fformat,1,1);
  
  
  int nthreads=3;
  
  int nph, nph1;
  //  single_photon_hit clusters[NR*NC];
  // cout << "hits "<< endl;
  int etabins=550;
  double etamin=-1, etamax=2;
  int nsubpix=1;
  float *etah=new float[etabins*etabins];
  // cout << "etah "<< endl;
  cout << "image size "<< nsubpix*nsubpix*NC*NR << endl;
 
  int *heta, *himage;

    gotthardModuleDataNew *decoder=new   gotthardModuleDataNew();
    gotthardDoubleModuleDataNew *det=new   gotthardDoubleModuleDataNew();
  // cout << "decoder "<< endl;
  // etaInterpolationPosXY *interp=new etaInterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
  // cout << "interp "<< endl;
    //   linearInterpolation *interp=new linearInterpolation(NC, NR, nsubpix);
  //noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);
    // interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
    // interpolatingDetector *filter0=new interpolatingDetector(decoder,interp, 5, 1, 0, 1000, 10);
    // interpolatingDetector *filter1=new interpolatingDetector(decoder,interp, 5, 1, 0, 1000, 10);
  //filter->readPedestals("/scratch/ped_100.tiff");
  //cout << "filter "<< endl;
  
  singlePhotonDetector *filter=new singlePhotonDetector(det,3, 5, 1, 0, 1000, 200);

  filter->setFrameMode(eFrame);
  
  char *buff;//[2*(48+1280*2)];


  char *buff0;
  char *buff1;


  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
  mt->setFrameMode(eFrame);
  mt->StartThreads();
  mt->popFree(buff);
  buff0=buff;
  buff1=buff+48+1280*2;


  int photons[1280*2];

  int nf=0;
  int ok=0;
  ifstream filebin0,filebin1;
  std::time_t end_time;
  int16_t dout[1280*2];
  int iFrame=-1;
  int np=-1;

  nph=0;
  nph1=0;
  //int np;
  int iph;
  int data_ready=1;
  int *image;

  // filter->setROI(0,512,0,1);

    filebin0.open((const char *)(fname0), ios::in | ios::binary);
    filebin1.open((const char *)(fname1), ios::in | ios::binary);
    if (filebin0.is_open() && filebin1.is_open()) {
      cout << "Opened file " << fname0<< endl;
      cout << "Opened file " << fname1<< endl;
	//	mt->setFrameMode(eFrame);
  // 	mt->prepareInterpolation(ok);
  // mt->StartThreads();
  // mt->popFree(buff);
      nf=0;
      iFrame=-1;
      while ((decoder->readNextFrame(filebin0, iFrame, np, buff0)) && (decoder->readNextFrame(filebin1, iFrame, np, buff1))) {
	//filter->processData(buff, photons);
	//	cout << nf << " " << decoder->getFrameNumber(buff0) << " " << decoder->getFrameNumber(buff1) << " " << filter->getPhFrame() << " " << filter->getPhTot() << endl; 
	// for (int i=0; i<1280*2; i++) {
	//   filter->addToPedestal(buff,i,0);
	//   dout[i]=filter->subtractPedestal(buff,i,0);
	//   if (nf>10 && i<512)
	//     if (i%2)
	//       cout << nf << " " << i << " " << filter->getPedestal(i,0) << " " << det->getValue(buff,i,0) << " " << decoder->getValue(buff1,1280-1-i/2,0)<< " " << dout[i] << endl;
	//     else
	//       cout << nf << " " << i << " " << filter->getPedestal(i,0) << " " << det->getValue(buff,i,0) << " " << decoder->getValue(buff0,i/2,0)<< " " << dout[i] << endl;
	
	// }
	  mt->pushData(buff);
	  mt->nextThread();
	  mt->popFree(buff);
	  buff0=buff;
	  buff1=buff+48+1280*2;
	  
	nf++;
	
	//	cout << id << " " << nf << endl;
	if (nf%10000==0) {
	  while (mt->isBusy()) {;}
	  image=filter->getImage();
	  if (image) {
	    cout << nf << "*****************" << endl;
	    for (int i=0; i<512; i++) {
	      cout << image[i] << "\t";
	    }
	    cout << endl;
	  }
	  filter->clearImage();
	  std::time(&end_time);
	  cout << std::ctime(&end_time) << " " << nf <<   endl;
	}
	iFrame=-1;
      }
      
      filebin0.close();
      filebin1.close();
    }
    else
      cout << "Could not open file " << fname0<< " or " << fname1 <<  endl;
  return NULL;
}
  
int main(int argc, char *argv[]){

   gotthardProcessFrame();

}

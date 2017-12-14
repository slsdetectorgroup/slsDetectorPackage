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
//#include "multiThreadedAnalogDetector.h"

#include <ctime>

#define NC 1280
#define NR 1

#include "tiffIO.h"


void *gotthardProcessFrame() {
  char fname0[10000], fname1[10000];
  char fformat[10000];
  strcpy(fformat,"/external_pool/gotthard_data/datadir_gotthardI/bchip074075/20170731/Xray/xray_15kV_200uA_5us_d%d_f000000040000_0.raw");
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
  
  char buff[2*(48+1280*2)];
  char *buff0=buff;
  char *buff1=buff+48+1280*2;

  int photons[1280*2];

  int nf=0;
  int ok=0;
  ifstream filebin0,filebin1;
  std::time_t end_time;
 
  int iFrame=-1;
  int np=-1;
	// typedef struct {
	// 	uint64_t frameNumber;	/**< is the frame number */
	// 	uint32_t expLength;		/**< is the subframe number (32 bit eiger) or real time exposure time in 100ns (others) */
	// 	uint32_t packetNumber;	/**< is the packet number */
	// 	uint64_t bunchId;		/**< is the bunch id from beamline */
	// 	uint64_t timestamp;		/**< is the time stamp with 10 MHz clock */
	// 	uint16_t modId;			/**< is the unique module id (unique even for left, right, top, bottom) */
	// 	uint16_t xCoord;		/**< is the x coordinate in the complete detector system */
	// 	uint16_t yCoord;		/**< is the y coordinate in the complete detector system */
	// 	uint16_t zCoord;		/**< is the z coordinate in the complete detector system */
	// 	uint32_t debug;			/**< is for debugging purposes */
	// 	uint16_t roundRNumber;	/**< is the round robin set number */
	// 	uint8_t detType;		/**< is the detector type see :: detectorType */
	// 	uint8_t version;		/**< is the version number of this structure format */
	// } sls_detector_header;


  //  multiThreadedDetector *mt=new multiThreadedDetector(filter,nthreads,100);
  nph=0;
  nph1=0;
  //int np;
  int iph;
  int data_ready=1;


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
	filter->processData(buff, photons);
	cout << nf << " " << decoder->getFrameNumber(buff0) << " " << decoder->getFrameNumber(buff1) << " " << filter->getPhFrame() << " " << filter->getPhTot() << endl; 
	nf++;
	}
	
	//	cout << id << " " << nf << endl;
      if (nf%1000==0) {
	std::time(&end_time);
	cout << std::ctime(&end_time) << " " << nf <<   endl;
      }
      
      //delete [] buff;
      iFrame=-1;
      
      
      filebin0.close();
      filebin1.close();
    }
    else
      cout << "Could not open file " << fname0<< " or " << fname1 <<  endl;


  //mt->StopThreads();
 
  // char tit[10000];
  //sprintf(tit,"/scratch/int_image_mt%d.tiff",nthreads);
 
  //mt->writeInterpolatedImage(tit);
  // delete [] etah;
 
  // delete interp;
  //delete decoder;
  //cout << "Read " << nf << " frames" << endl;
  return NULL;
}
  
int main(int argc, char *argv[]){

   gotthardProcessFrame();

}

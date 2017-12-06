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

#include "moench03Ctb10GbT1Data.h"

#include "interpolatingDetector.h"
#include "etaInterpolationPosXY.h"
#include "linearInterpolation.h"
#include "noInterpolation.h"
#include "multiThreadedDetector.h"

#include <ctime>

#define NC 400
#define NR 400

#include "tiffIO.h"


void *moenchProcessFrame() {
  char fname[10000];
  strcpy(fname,"/mnt/moench_data/m03-15_mufocustube/plant_40kV_10uA/m03-15_100V_g4hg_300us_dtf_0.raw");
  
  int nthreads=3;
  
  int nph, nph1;
  single_photon_hit clusters[NR*NC];
  // cout << "hits "<< endl;
  int etabins=550;
  double etamin=-1, etamax=2;
  int nsubpix=4;
  float *etah=new float[etabins*etabins];
  // cout << "etah "<< endl;
  cout << "image size "<< nsubpix*nsubpix*NC*NR << endl;
  float *image=new float[nsubpix*nsubpix*NC*NR];
  int *heta, *himage;

  moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data();
  // cout << "decoder "<< endl;
  etaInterpolationPosXY *interp=new etaInterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
  // cout << "interp "<< endl;
  //linearInterpolation *interp=new linearInterpolation(NC, NR, nsubpix);
  //noInterpolation *interp=new noInterpolation(NC, NR, nsubpix);
  interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  interpolatingDetector *filter=new interpolatingDetector(decoder,interp, 5, 1, 0, 1000, 10);
  filter->readPedestals("/scratch/ped_100.tiff");
  cout << "filter "<< endl;
  

  
  char *buff;
  int nf=0;
  int ok=0;
  ifstream filebin;
  std::time_t end_time;
 
  int iFrame=-1;
  int np=-1;

  
  filter->newDataSet();


  multiThreadedDetector *mt=new multiThreadedDetector(filter,nthreads,100);
  nph=0;
  nph1=0;
  //int np;
  int iph;

  cout << "file name " << fname << endl;
  filebin.open((const char *)(fname), ios::in | ios::binary);
  if (filebin.is_open())
     cout << "Opened file " << fname<< endl;
  else
    cout << "Could not open file " << fname<< endl;
  mt->setFrameMode(eFrame);
  mt->prepareInterpolation(ok);
  mt->StartThreads();
  mt->popFree(buff);
  
  while ((decoder->readNextFrame(filebin, iFrame, np, buff)) && nf<1.5E4) {
      if (nf<9E3) 
	;
      else { 
	
	// if (nf>1.1E4 && ok==0) {
	//   mt->prepareInterpolation(ok);
	//   mt->setFrameMode(eFrame);
	//   //ok=1;
	// }
	  
	mt->pushData(buff);
	mt->nextThread();
	//	cout << " " << (void*)buff;
	mt->popFree(buff);

	// if (ok==0) {
	//   cout << "**************************************************************************"<< endl;
	//   heta=interp->getFlatField();
	//   // for (int ii=0; ii<etabins*etabins; ii++) {
	//   //   etah[ii]=(float)heta[ii];
	//   // }


	//   std::time(&end_time);
	//   cout << std::ctime(&end_time) << " " << nf <<  endl;
	//   // WriteToTiff(etah, "/scratch/eta.tiff", etabins, etabins);
	  
	//   interp->prepareInterpolation(ok);
	//   cout << "**************************************************************************"<< endl;
	//   std::time(&end_time);
	//   cout << std::ctime(&end_time) << " " << nf <<  endl;
	// }
	//	filter->processData(buff,eFrame);
	//  }
      
	// nph+=nph1;
      }
      if (nf%1000==0) {
	std::time(&end_time);
	cout << std::ctime(&end_time) << " " << nf <<   endl;
      }
      
      nf++;
      //delete [] buff;
      iFrame=-1;
  } 
 
  if (filebin.is_open())
    filebin.close();	  
  else
    cout << "could not open file " << fname << endl;
    
  mt->StopThreads();
 
  char tit[10000];
  sprintf(tit,"/scratch/int_image_mt%d.tiff",nthreads);
 
  mt->writeInterpolatedImage(tit);
  // delete [] etah;
 
  // delete interp;
  //delete decoder;
  //cout << "Read " << nf << " frames" << endl;
  return NULL;
}
  
int main(int argc, char *argv[]){

   moenchProcessFrame();

}

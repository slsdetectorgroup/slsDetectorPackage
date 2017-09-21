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


#include <ctime>

#define NC 400
#define NR 400

#include "tiffIO.h"


void *moenchProcessFrame() {
  char fname[10000];
  strcpy(fname,"/mnt/moench_data/m03-15_mufocustube/plant_40kV_10uA/m03-15_100V_g4hg_300us_dtf_0.raw");
  

  int nph, nph1;
  single_photon_hit clusters[NR*NC];
  // cout << "hits "<< endl;
  int etabins=250;
  double etamin=-0.1, etamax=1.1;
  int nsubpix=5;
  float *etah=new float[etabins*etabins];
  // cout << "etah "<< endl;
  cout << "image size "<< nsubpix*nsubpix*NC*NR << endl;
  float *image=new float[nsubpix*nsubpix*NC*NR];
  int *heta, *himage;

  moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data();
  // cout << "decoder "<< endl;
  etaInterpolationPosXY *interp=new etaInterpolationPosXY(NC, NR, nsubpix, etabins, etamin, etamax);
  // cout << "interp "<< endl;

  interpolatingDetector *filter=new interpolatingDetector(decoder,interp, 5, 1, 0, 10000, 10000);
  cout << "filter "<< endl;
  

  char *buff;
  int nf=0;
  int ok=0;
  ifstream filebin;
  std::time_t end_time;
 
  int iFrame=-1;
  int np=-1;

  
  filter->newDataSet();


  nph=0;
  nph1=0;
  int iph;

  cout << "file name " << fname << endl;
  filebin.open((const char *)(fname), ios::in | ios::binary);
  if (filebin.is_open())
     cout << "Opened file " << fname<< endl;
  else
    cout << "Could not open file " << fname<< endl;
    while ((buff=decoder->readNextFrame(filebin, iFrame)) && nf<5E4) {
      if (nf<1.1E4)
	filter->addToFlatField(buff,clusters,nph1);
      else {
	if (ok==0) {
	  cout << "**************************************************************************"<< endl;
	  heta=interp->getFlatField();
	  for (int ii=0; ii<etabins*etabins; ii++) {
	    etah[ii]=(float)heta[ii];
	  }


	  std::time(&end_time);
	  cout << std::ctime(&end_time) << " " << nf << " " << nph1 << " " << nph << endl;
	  WriteToTiff(etah, "/scratch/eta.tiff", etabins, etabins);
	  
	  interp->prepareInterpolation(ok);
	  cout << "**************************************************************************"<< endl;
	  std::time(&end_time);
	  cout << std::ctime(&end_time) << " " << nf << " " << nph1 << " " << nph << endl;
	}
	filter->addToInterpolatedImage(buff,clusters,nph1);
      }
      
      nph+=nph1;
      if (nf%1000==0) {
	std::time(&end_time);
	cout << std::ctime(&end_time) << " " << nf << " " << nph1 << " " << nph << endl;
      }
      nf++;
      delete [] buff;
      iFrame=-1;
    } 
 
    if (filebin.is_open())
      filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
    
    
    himage=interp->getInterpolatedImage();
    for (int ii=0; ii<nsubpix*nsubpix*NC*NR; ii++) {
      image[ii]=(float)himage[ii];
    }
    WriteToTiff(image, "/scratch/int_image.tiff", nsubpix*NR, nsubpix*NC);
      delete interp;
    delete decoder;
    cout << "Read " << nf << " frames" << endl;
    return NULL;
}
  
int main(int argc, char *argv[]){

   moenchProcessFrame();

}

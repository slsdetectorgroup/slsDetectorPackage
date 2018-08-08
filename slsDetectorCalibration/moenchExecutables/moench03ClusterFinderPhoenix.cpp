//#include "ansi.h"
#include <iostream>

//#include "moench03T1ZmqData.h"
//#include "moench03T1ReceiverData.h"

#include "moench03Ctb10GbT1Data.h"

// #include "interpolatingDetector.h"
//#include "etaInterpolationPosXY.h"
// #include "linearInterpolation.h"
// #include "noInterpolation.h"
#include "multiThreadedAnalogDetector.h"
#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"

#include <stdio.h>
#include <map>
#include <fstream>
#include <sys/stat.h>

#include <ctime>
using namespace std;


int main(int argc, char *argv[]) {


  if (argc<6) {
    cout << "Usage is " << argv[0] << "indir outdir fname runmin runmax " << endl;
    return 0;
  }
  int ii=0;
  int p=10000;
  int fifosize=1000;
  int nthreads=5;
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
  moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data();
  cout << "decoder" << endl;
  //moench03T1ReceiverData *decoder=new  moench03T1ReceiverData();
  //moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  singlePhotonDetector *filter=new singlePhotonDetector(decoder,csize, nsigma, 1, 0, nped, 100);
  //  char tit[10000];
  cout << "filter" << endl;




  // filter->readPedestals("/scratch/ped_100.tiff");
  // interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  // cout << "filter "<< endl;
  

  int size = 327680;////atoi(argv[3]);
  
  int* image;
	//int* image =new int[327680/sizeof(int)];
  filter->newDataSet();

  cout << "dataset" << endl;

  int ff, np;
  int dsize=decoder->getDataSize();
  cout << " data size is " << dsize << endl;
  

  char data[dsize];

  ifstream filebin;
  char *indir=argv[1];
  cout << "input directory is " << indir << endl;
  char *outdir=argv[2];
  cout << "output directory is " << outdir << endl;
  char *fformat=argv[3];
  cout << "fileformat is " << fformat << endl;
  int runmin=atoi(argv[4]);
  cout << "runmin : " << runmin << endl;
  int runmax=atoi(argv[5]);
  cout << "runmax : " << runmax << endl;
  
  char fname[10000];
  char outfname[10000];
  char imgfname[10000];
  char pedfname[10000];
  char fn[10000];
  
  std::time_t end_time;

  FILE *of=NULL;
  
  
  cout << "time " << endl;
  std::time(&end_time);
  cout << std::ctime(&end_time) <<   endl;
  filter->setFrameMode(eFrame);
	//  mt->setFrameMode(ePedestal);
 

  // for (int ix=0; ix<400; ix++)
  //   for (int iy=0; iy<400; iy++)
  //     cout << ix << " " << iy << " " << filter->getPedestal(ix,iy) << " " << filter->getPedestalRMS(ix,iy) << endl;


  char* buff;
  cout << "aa " << endl;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);
  cout << "mt " << endl;
  
  // mt->setFrameMode(eFrame); //need to find a way to switch between flat and frames!
  // mt->prepareInterpolation(ok);
  mt->setFrameMode(eFrame);
  mt->StartThreads();
  mt->popFree(buff);
  

  
  int ifr=0;
  // //loop on files
  // mt->setFrameMode(eFrame);
  //mt->setFrameMode(eFlat);
  
  
  
  
  
  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fn,fformat,irun);
    sprintf(fname,"%s/%s.raw",indir,fn);
    sprintf(outfname,"%s/%s.clust",outdir,fn);
    sprintf(imgfname,"%s/%s.tiff",outdir,fn);
    std::time(&end_time);
    cout << std::ctime(&end_time) <<    endl;
    cout <<  fname << " " << outfname << " " << imgfname <<  endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    //      //open file
    if (filebin.is_open()){
      of=fopen(outfname,"w");
      if (of) {
  	mt->setFilePointer(of);
	//	cout << "file pointer set " << endl;
      } else {
  	cout << "Could not open "<< outfname << " for writing " << endl;
  	mt->setFilePointer(NULL);
  	return 1;
      }
      //     //while read frame 
      ff=-1;
      while (decoder->readNextFrame(filebin, ff, np,buff)) {
	//cout << "*"<<ifr++<<"*"<<ff<< endl;
	//cout << ff << " " << np << endl;
  	//         //push
  		mt->pushData(buff);
  // 	//         //pop
  		mt->nextThread();
  // // 		//	cout << " " << (void*)buff;
   		mt->popFree(buff);
	
		ff=-1;
		ii++;
		if (ii%10000==0) {

		  cout << ii << endl;

		  while (mt->isBusy()) {;}//wait until all data are processed from the queues
		  mt->writeImage("/scratch/tmp.tiff");
      		}
      }
      //  cout << "--" << endl;
      filebin.close();	 
      //      //close file 
      //     //join threads
      while (mt->isBusy()) {;}//wait until all data are processed from the queues
      if (of)
  	fclose(of);
      
      mt->writeImage(imgfname);
      mt->clearImage();
   
      std::time(&end_time);
      cout << std::ctime(&end_time) <<   endl;

    } else 
     cout << "Could not open "<< fname << " for reading " << endl;
      
    
  }
    

  return 0;
}


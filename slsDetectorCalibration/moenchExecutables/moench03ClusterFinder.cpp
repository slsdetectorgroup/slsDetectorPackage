//#include "sls/ansi.h"
#include <iostream>


//#include "moench03T1ZmqData.h"
#ifdef NEWRECEIVER
#ifndef RECT
#include "moench03T1ReceiverDataNew.h"
#endif

#ifdef RECT
#include "moench03T1ReceiverDataNewRect.h"
#endif

#endif


#ifdef CSAXS_FP
#include "moench03T1ReceiverData.h"
#endif 
#ifdef OLDDATA
#include "moench03Ctb10GbT1Data.h"
#endif 

#ifdef REORDERED
#include "moench03T1ReorderedData.h"
#endif

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
    return 1;
  }
  int p=10000;
  int fifosize=1000;
  int nthreads=1;
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





#ifdef NEWRECEIVER
#ifdef RECT
  cout << "Should be rectangular!" <<endl;
#endif
  moench03T1ReceiverDataNew *decoder=new  moench03T1ReceiverDataNew();
  cout << "RECEIVER DATA WITH ONE HEADER!"<<endl;
#endif

#ifdef CSAXS_FP
  moench03T1ReceiverData *decoder=new  moench03T1ReceiverData();
  cout << "RECEIVER DATA WITH ALL HEADERS!"<<endl;
#endif

#ifdef OLDDATA
  moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data();
  cout << "OLD RECEIVER DATA!"<<endl;
#endif

#ifdef REORDERED
  moench03T1ReorderedData *decoder=new  moench03T1ReorderedData();
  cout << "REORDERED DATA!"<<endl;
#endif


  decoder->getDetectorSize(nx,ny);
  cout << "nx " << nx << " ny " << ny << endl;

  //moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  singlePhotonDetector *filter=new singlePhotonDetector(decoder,csize, nsigma, 1, 0, nped, 200);
  //  char tit[10000];
  cout << "filter " << endl;



  // filter->readPedestals("/scratch/ped_100.tiff");
  // interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  // cout << "filter "<< endl;
  

  int size = 327680;////atoi(argv[3]);
  
  int* image;
	//int* image =new int[327680/sizeof(int)];
  filter->newDataSet();


  int ff, np;
  int dsize=decoder->getDataSize();
  cout << " data size is " << dsize;
  

  char data[dsize];

  ifstream filebin;
  char *indir=argv[1];
  char *outdir=argv[2];
  char *fformat=argv[3];
  int runmin=atoi(argv[4]);
  int runmax=atoi(argv[5]);
  
  char fname[10000];
  char outfname[10000];
  char imgfname[10000];
  char pedfname[10000];
  //  strcpy(pedfname,argv[6]);
  char fn[10000];
  
  std::time_t end_time;

  FILE *of=NULL;
  cout << "input directory is " << indir << endl;
  cout << "output directory is " << outdir << endl;
  cout << "fileformat is " << fformat << endl;
  

  std::time(&end_time);
  cout << std::ctime(&end_time) <<   endl;
 







  char* buff;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);

 
  mt->setDetectorMode(ePhotonCounting);
  mt->setFrameMode(eFrame);
  mt->StartThreads();
  mt->popFree(buff);


  cout << "mt " << endl;

  int ifr=0;
 

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
	//	cout << "*"<<ifr++<<"*"<<ff<< endl;
	//	cout << ff << " " << np << endl;
  	//         //push
	// for (int ix=0; ix<400; ix++)
	//   for (int iy=0; iy<400; iy++) {
	//     if (decoder->getChannel(buff, ix, iy)<3000 || decoder->getChannel(buff, ix, iy)>8000) {
	//       cout <<  ifr << " " << ff << " " << ix << " " << iy << " " << decoder->getChannel(buff, ix, iy) << endl ;
	//     }
	//   }

  	mt->pushData(buff);
  // 	//         //pop
  	mt->nextThread();
  // // 		//	cout << " " << (void*)buff;
  	mt->popFree(buff);
	ifr++;
	if (ifr%10000==0) cout << ifr << " " << ff << endl;
	ff=-1;
      }
        cout << "--" << endl;
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


//#include "ansi.h"
#include <iostream>

//#include "moench03T1ZmqData.h"
#include "moench03T1ReceiverData.h"

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


  if (argc<7) {
    cout << "Usage is " << argv[0] << "indir outdir fname runmin runmax pedfname" << endl;
  }
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

  moench03T1ReceiverDataNew *decoder=new  moench03T1ReceiverDataNew();
  //moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  singlePhotonDetector *filter=new singlePhotonDetector(decoder,csize, nsigma, 1, 0, nped, 200);
  //  char tit[10000];




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
  strcpy(pedfname,argv[6]);
  char fn[10000];
  
  std::time_t end_time;

  FILE *of=NULL;
  cout << "input directory is " << indir << endl;
  cout << "output directory is " << outdir << endl;
  cout << "fileformat is " << fformat << endl;
  cout << "pedestal file is " << fformat << endl;
  

  std::time(&end_time);
  cout << std::ctime(&end_time) <<   endl;
filter->setFrameMode(eFrame);
	//  mt->setFrameMode(ePedestal);
  cout <<   pedfname<< endl;

    filebin.open((const char *)(pedfname), ios::in | ios::binary);
    //filebin.open((const char *)(fname), ios::in | ios::binary);

  //      //open file
  if (filebin.is_open()){
      //     //while read frame 
    cout << "pedestal file " << endl;
    while (decoder->readNextFrame(filebin, ff, np,data)) {
      //   cout << ff << " " << np << endl;
      //         //push
      // mt->pushData(buff);
      // //         //pop
      //mt->nextThread();
      // // 		//	cout << " " << (void*)buff;
      //mt->popFree(buff);
      filter->processData(data);
      }
    filebin.close();	 
    //      //close file 
    //     //join threads
    //   while (mt->isBusy()) {;}//wait until all data are processed from the queues
    // cout << outfname << endl; 
    // filter->writePedestals(outfname); 
    // sprintf(outfname,"%s/%s_pedimg.tiff",outdir,fn);

    // cout << outfname << endl; 
    
    // filter->writeImage(outfname);
    // //mt->clearImage();
  } else 
     cout << "Could not open "<< pedfname << " for reading " << endl;
  


  // for (int ix=0; ix<400; ix++)
  //   for (int iy=0; iy<400; iy++)
  //     cout << ix << " " << iy << " " << filter->getPedestal(ix,iy) << " " << filter->getPedestalRMS(ix,iy) << endl;








  char* buff;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);

  
  // mt->setFrameMode(eFrame); //need to find a way to switch between flat and frames!
  // mt->prepareInterpolation(ok);
 mt->setFrameMode(eFrame);
  mt->StartThreads();
  mt->popFree(buff);



  int ifr=0;
  // //loop on files
  // mt->setFrameMode(eFrame);
 //mt->setFrameMode(eFlat);






  for (int irun=runmin; irun<runmin+5; irun++) {
    sprintf(fn,fformat,irun);
    sprintf(fname,"%s/%s.raw",indir,fn);



    filebin.open((const char *)(fname), ios::in | ios::binary);
    //      //open file
    if (filebin.is_open()){
      //     //while read frame 
      while (decoder->readNextFrame(filebin, ff, np,buff)) {
	//	cout << "*"<<ifr++<<"*"<<ff<< endl;
	//	cout << ff << " " << np << endl;
  	//         //push
  		mt->pushData(buff);
  // 	//         //pop
  		mt->nextThread();
  // // 		//	cout << " " << (void*)buff;
   		mt->popFree(buff);
	
      }
      //  cout << "--" << endl;
      filebin.close();	 
    

    }

  }

  while (mt->isBusy()) {;}//wait until all data are processed from the queues
  mt->clearImage();

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
      while (decoder->readNextFrame(filebin, ff, np,buff)) {
	//	cout << "*"<<ifr++<<"*"<<ff<< endl;
	//	cout << ff << " " << np << endl;
  	//         //push
  		mt->pushData(buff);
  // 	//         //pop
  		mt->nextThread();
  // // 		//	cout << " " << (void*)buff;
   		mt->popFree(buff);
	
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


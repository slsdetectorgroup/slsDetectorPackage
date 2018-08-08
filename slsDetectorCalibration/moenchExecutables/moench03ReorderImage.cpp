//#include "ansi.h"
#include <iostream>



//#include "moench03T1ZmqData.h"

#include "moench03T1ReceiverDataNew.h"

// #include "interpolatingDetector.h"
//#include "etaInterpolationPosXY.h"
// #include "linearInterpolation.h"
// #include "noInterpolation.h"
//#include "interpolatingDetector.h"

#include <stdio.h>
#include <map>
#include <fstream>
#include <sys/stat.h>

#include <cstdlib>

#include <ctime>
using namespace std;

#define NX 400 //number of x pixels
#define NY 400 //number of y pixels

int main(int argc, char *argv[]) {


  if (argc<6) {
    cout << "Usage is " << argv[0] << "indir outdir fname runmin runmax " << endl;
    return 1;
  }
  int p=10000;
  int fifosize=1000;
  int nthreads=8;
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


  uint16_t data[NY*NX];


  int size = 327680;////atoi(argv[3]);
  
  int* image;

  int ff, np;
  int dsize=decoder->getDataSize();
  

  //char data[dsize];

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

 


  int ifr=0;
 

  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fn,fformat,irun);
    sprintf(fname,"%s/%s.raw",indir,fn);
    sprintf(outfname,"%s/%s_image.raw",outdir,fn);
    std::time(&end_time);
    cout << std::ctime(&end_time) <<    endl;
    cout <<  fname << " " << outfname << " " << imgfname <<  endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    //      //open file
    if (filebin.is_open()){
      of=fopen(outfname,"w");
      if (of) {
	;
      } else {
  	cout << "Could not open "<< outfname << " for writing " << endl;
  	return 1;
      }
      //     //while read frame 
      ff=-1;
      while (decoder->readNextFrame(filebin, ff, np,buff)) {
	for (int ix=0; ix<400; ix++)
	  for (int iy=0; iy<400; iy++)
	    data[iy*NX+ix]=decoder->getChannel(buff,ix,iy);
	
	ifr++;

	fwrite(&ff, 8, 1,of);//write detector frame number
	fwrite(&ifr, 8, 1,of);//write datset frame number
	fwrite(data,2,NX*NY,of);//write reordered data

	if (ifr%10000==0) cout << ifr << " " << ff << endl;
	ff=-1;

      }
        cout << "--" << endl;
      filebin.close();	 
      //      //close file 
      //     //join threads
  
      if (of)
  	fclose(of);
      
      std::time(&end_time);
      cout << std::ctime(&end_time) <<   endl;

    } else 
     cout << "Could not open "<< fname << " for reading " << endl;
      
    
  }
    

  return 0;
}


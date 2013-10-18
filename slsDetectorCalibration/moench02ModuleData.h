#ifndef MOENCH02MODULEDATA_H
#define  MOENCH02MODULEDATA_H
#include "slsDetectorData.h"

#include <iostream>
#include <fstream>

using namespace std;

class moench02ModuleData : public slsDetectorData {
 public:
  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  

  moench02ModuleData(): slsDetectorData(160, 160, 1286*40) {
    int headerOff=0;
    int footerOff=1284;
    int dataOff=4;

    int ix, iy;

    int **dMask;
    int **dMap;

    dMask=new int*[160];
    for(int i = 0; i < 160; i++) {
      dMask[i] = new int[160];
    }
    dMap=new int*[160];
    for(int i = 0; i < 160; i++) {
      dMap[i] = new int[160];
    }

    for (int isc=0; isc<4; isc++) {
      for (int ip=0; ip<10; ip++) {

	for (int ir=0; ir<16; ir++) {
	  for (int ic=0; ic<40; ic++) {

	    ix=isc*40+ic;
	    iy=ip*16+ir;

	    dMap[iy][ix]=1280*(isc*10+ip)+2*ir*40+2*ic;
	    // cout << ix << " " << iy << " " << dMap[ix][iy] << endl;
	  }
	}
      }
    }

    for (int ix=0; ix<120; ix++) {
      for (int iy=0; iy<160; iy++)
	dMask[iy][ix]=0x7fff;
    }
    for (int ix=120; ix<160; ix++) {
      for (int iy=0; iy<160; iy++)
	dMask[iy][ix]=0x0;
    }


    setDataMap(dMap);
    setDataMask(dMask);
  };

  
  int getFrameNumber(char *buff){
    return ((*(int*)buff)&(0xffffff00))>>8;;
  };

  int getPacketNumber(char *buff){
    return (*(int*)buff)&0xff;
  };


  char *readNextFrame(ifstream &filebin) {
    char *buff=new char[1280*40];
    char p[1286];
    
    int fn, fnum=-1,  np=0, pnum=-1;

    if (filebin.is_open()) {
      while (filebin.read(p,4)) { //read header
	pnum=getPacketNumber(p);
	fn=getFrameNumber(p);

	if (pnum<0 || pnum>39) {
	  cout << "Bad packet number " << pnum << " frame "<< fn << endl;
	  continue;
	}

	if (pnum==0)
	  pnum=40;
	
	if (pnum==1) {
	  fnum=fn;
	  if (np>0)
	    cout << "*Incomplete frame number " << fnum << endl;
	  np=0;
	} else if (fn!=fnum) {
	  if (fnum!=-1)
	    cout << " **Incomplete frame number " << fnum << " pnum " << pnum << " " << getFrameNumber(p) << endl;
	  np=0;
	}
	filebin.read(buff+1280*(pnum-1),1280); //readdata
	np++;
	filebin.read(p,2); //readfooter

	if (np==40)
	  if (pnum==40)
	    break;
	  else
	    cout << "Too many packets for this frame! "<< fnum << " " << pnum << endl;

      }
    }
    if (np<40) {
      if (np>0)
	cout << "Too few packets for this frame! "<< fnum << " " << pnum << endl;
      delete [] buff;
      buff=NULL;
    }
    return buff;
  };


};



#endif

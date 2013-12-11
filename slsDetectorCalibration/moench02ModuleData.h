#ifndef MOENCH02MODULEDATA_H
#define  MOENCH02MODULEDATA_H
//#include "slsDetectorData.h"
#include "singlePhotonDetector.h"

#include "RunningStat.h"
#include "MovingStat.h"

#include <iostream>
#include <fstream>

#include <TTree.h>
using namespace std;



class moench02ModuleData : public slsDetectorData<uint16_t> {
 public:




  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  

  moench02ModuleData(double c=0): slsDetectorData<uint16_t>(160, 160, 40, 1286), xtalk(c) {




    uint16_t **dMask;
    int **dMap;
    int ix, iy;



    dMask=new uint16_t*[160];
    dMap=new int*[160];
    for (int i = 0; i < 160; i++) {
      dMap[i] = new int[160];
      dMask[i] = new uint16_t[160];
    }

    for (int isc=0; isc<4; isc++) {
      for (int ip=0; ip<10; ip++) {

	for (int ir=0; ir<16; ir++) {
	  for (int ic=0; ic<40; ic++) {

	    ix=isc*40+ic;
	    iy=ip*16+ir;

	    dMap[iy][ix]=1286*(isc*10+ip)+2*ir*40+2*ic;
	    // cout << ix << " " << iy << " " << dMap[ix][iy] << endl;
	  }
	}
      }
    }

    for (ix=0; ix<120; ix++) {
      for (iy=0; iy<160; iy++)
	dMask[iy][ix]=0x3fff;
    }
    for (ix=120; ix<160; ix++) {
      for (iy=0; iy<160; iy++)
	dMask[iy][ix]=0x0;
    }


    setDataMap(dMap);
    setDataMask(dMask);


    

  };
    
    //   ~moench02ModuleData() {if (buff) delete [] buff; if (oldbuff) delete [] oldbuff; };
  

  int getPacketNumber(char *buff){
    int np=(*(int*)buff)&0xff;
    if (np==0)
      np=40;
    return np;
  };

  double getValue(char *data, int ix, int iy=0) {
    if (xtalk==0 || ix%40==0)
      return (double)getValue(data, ix, iy);
    else
      return slsDetectorData<uint16_t>::getValue(data, ix, iy)-xtalk*slsDetectorData<uint16_t>::getValue(data, ix-1, iy);
  };
  
  double setXTalk(double c) {xtalk=c; return xtalk;}
  double getXTalk() {return xtalk;}
  




  

 private:
  
  double xtalk;


};



#endif

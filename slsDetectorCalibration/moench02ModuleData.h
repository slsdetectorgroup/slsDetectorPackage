#ifndef MOENCH02MODULEDATA_H
#define  MOENCH02MODULEDATA_H
#include "slsReceiverData.h"



class moench02ModuleData : public slsReceiverData<uint16_t> {
 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
  

  moench02ModuleData(double c=0): slsReceiverData<uint16_t>(160, 160, 40, 1286), 
    xtalk(c) {




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

	    dMap[iy][ix]=1286*(isc*10+ip)+2*ir*40+2*ic+4; 
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
    
  
  
  /**
     gets the packets number (last packet is labelled with 0 and is replaced with 40)
     \param buff pointer to the memory
     \returns packet number

  */
  
  int getPacketNumber(char *buff){
    int np=(*(int*)buff)&0xff;
    if (np==0)
      np=40;
    return np;
  };


  /**
    returns the pixel value as double correcting for the output buffer crosstalk
     \param data pointer to the memory
     \param ix coordinate in the x direction
     \param iy coordinate in the y direction
     \returns channel value as double

  */
  double getValue(char *data, int ix, int iy=0) {
    //  cout << "##" << (void*)data << " " << ix << " " <<iy << endl;
    if (xtalk==0 || ix%40==0)
      return  slsDetectorData<uint16_t>::getValue(data, ix, iy);
    else
      return slsDetectorData<uint16_t>::getValue(data, ix, iy)-xtalk*slsDetectorData<uint16_t>::getValue(data, ix-1, iy);
  };
  
  

  /** sets the output buffer crosstalk correction parameter
      \param c output buffer crosstalk correction parameter to be set
      \returns current value for the output buffer crosstalk correction parameter

  */
  double setXTalk(double c) {xtalk=c; return xtalk;}


  /** gets the output buffer crosstalk parameter
      \returns current value for the output buffer crosstalk correction parameter
  */
  double getXTalk() {return xtalk;}
  




  

 private:
  
  double xtalk; /**<output buffer crosstalk correction parameter */


};



#endif

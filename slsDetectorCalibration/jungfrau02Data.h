#ifndef JUNGFRAU02DATA_H
#define JUNGFRAU02DATA_H

#include "chiptestBoardData.h"



class jungfrau02Data : public chiptestBoardData {
 public:

/* test :) */


  /**
     Implements the chiptestBoardData structure for the jungfrau02 prototype 
     (48x48 pixels, ADC2 for analog output, 2 more ADCs used for readout of digital bits, pixel offset configurable.)
     \param c crosstalk parameter for the output buffer

  */
  

  jungfrau02Data(int nadc, int offset, double c=0): chiptestBoardData(48, 48, nadc, offset), 
    xtalk(c) {




    uint16_t **dMask;
    int **dMap;



    dMask=new uint16_t*[48];
    dMap=new int*[48];
    for (int i = 0; i < 48; i++) {
      dMap[i] = new int[48];
      dMask[i] = new uint16_t[48];
    }


    for (int iy=0; iy<48; iy++) {
      for (int ix=0; ix<48; ix++) {
	
	dMap[ix][iy]=offset+(iy*48+ix)*nAdc;//dMap[iy][ix]=offset+(iy*48+ix)*nAdc;
	// cout << ix << " " << iy << " " << dMap[ix][iy] << endl;
      }
    }

    cout << (0,0) << " " << dMap[0][0] << endl;
    cout << (47,47) << " " << dMap[47][47] << endl;
	

    for (int ix=0; ix<48; ix++) {
      for (int iy=0; iy<48; iy++)
	dMask[iy][ix]=0x0; 

    setDataMap(dMap);
    setDataMask(dMask);
    }



  };
    
   /**

     Returns the value of the selected channel for the given dataset. Since the ADC is only 14bit wide, only bit 0-13 are occupied. If the gain bits are read out, they are returned in bit 14-15.
     \param data pointer to the dataset (including headers etc)
     \param ix pixel number in the x direction
     \param iy pixel number in the y direction
     \returns data for the selected channel, with inversion if required

  */
 
 virtual uint16_t getChannel(char *data, int ix, int iy=0) { 
    uint16_t m=0, d=0;
    if (ix>=0 && ix<nx && iy>=0 && iy<ny && dataMap[iy][ix]>=0 && dataMap[iy][ix]<dataSize) { 
          m=dataMask[iy][ix]; 
          d=*(((uint16_t*)(data))+dataMap[iy][ix]); 
      // cout << ix << " " << iy << " " << (uint16_t*)data << " " << ((uint16_t*)(data))+dataMap[iy][ix] << " " << d << endl;
	if (nAdc==3) {
	  //cout << "Gain bit!" << endl;
	  if (*((uint16_t*)(data)+dataMap[iy][ix]+2)>8000) //exchange if gainBits==2 is returned!
		d|=(1<<14); // gain bit 1
	  if (*((uint16_t*)(data)+dataMap[iy][ix]+1)>8000) //exchange if gainBits==2 is returned!
		d|=(1<<15); // gain bit 0
	  
        }

    }  
    return d^m;
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
    uint16_t d=getChannel(data, ix, iy);
    // cout << d << " " << (d&0x3fff) << endl;
    if (xtalk==0 || ix==0)
      return  (double)(getChannel(data, ix, iy)&0x3fff);
    else
      return  (double)(getChannel(data, ix, iy)&0x3fff)-xtalk* (double)(getChannel(data, ix, iy)&0x3fff);
  };
  
  /**
    returns the gain bit value, i.e. returns 0 if(GB0==0 && GB1==0), returns 1 if(GB0==1 && GB1==0), returns 3 if(GB0==1 && GB1==1), if it returns 2 -> the gain bits are read out the wrong way around (i.e. GB0 and GB1 have to be reversed!)
     \param data pointer to the memory
     \param ix coordinate in the x direction
     \param iy coordinate in the y direction
     \returns gain bits as int
  */
  int getGainBits(char *data, int ix, int iy=0) { 
  	uint16_t d=getChannel(data, ix, iy);
  	return ((d&0xc000)>>14); 
  };
  //*/
 

  

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

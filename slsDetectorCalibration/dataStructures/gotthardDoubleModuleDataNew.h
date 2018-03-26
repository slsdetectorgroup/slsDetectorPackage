#ifndef GOTTHARD2MODULEDATANEW_H
#define  GOTTHARD2MODULEDATANEW_H
#include "gotthardModuleDataNew.h"








class gotthardDoubleModuleDataNew : public slsDetectorData<uint16_t> {

 private:
  const int nModules;
  const int offset;
  int iframe;
    

public:



	/**
     Implements the slsReceiverData structure for the gotthard read out by a module i.e. using the slsReceiver
     (1x1280 pixels, 2 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

	 */


 gotthardDoubleModuleDataNew(int off=24*2, int nmod=2): slsDetectorData<uint16_t>(1280*nmod, 1, nmod*(1280*2+off)), nModules(nmod), offset(off),iframe(0) {
    
    
    
    
#ifdef BCHIP074_BCHIP075
		  cout << "This is a bchip074-bchip075 system " << endl;
#endif


		uint16_t **dMask;
		int **dMap;
		int ix, iy;
		int ypixels=1;
		int xpixels=1280*nmod;
		int imod, ipix;
		dMask=new uint16_t*[1];
		dMap=new int*[1];
		dMap[0] = new int[1280*nmod];
		dMask[0] = new uint16_t[1280*nmod];

		for(int ix=0; ix<xpixels; ix++) {
		  imod=ix%2;
		  if (imod==0)
		    ipix=ix/2;
		  else
		    ipix=1280-1-ix/2;
		  if (imod==0)
		    dMap[0][ix] =ipix*2+offset;
		  else
		    dMap[0][ix] = 1280*2+2*offset+ipix*2;//dataSize-2-ix;//+2*offset;
		  // dMap[0][ix] = 2*ipix+offset*(imod+1)+1280*2*imod; 
		  dMask[0][ix] = 0x0;
#ifdef BCHIP074_BCHIP075
		  int ibad=ix/2+1280*imod;
		  if ((ibad>=128*4 && ibad<128*5) || (ibad>=9*128 && ibad<10*128) || (ibad>=(1280+128*4) && ibad<ibad>=(1280+128*6))) 
		    	dataROIMask[0][ix]=0; 
#endif
		}

		setDataMap(dMap);
		setDataMask(dMask);

	};


	  /**

	     Returns the frame number for the given dataset.
	     \param buff pointer to the dataset
	     \returns frame number

	   */


  int getFrameNumber(char *buff){if (offset>=sizeof(sls_detector_header)) return ((sls_detector_header*)buff)->frameNumber; return iframe;};//*((int*)(buff+5))&0xffffff;};   

 

	/**
     gets the packets number (last packet is labelled with 0 and is replaced with 40)
     \param buff pointer to the memory
     \returns packet number

	 */

  int getPacketNumber(char *buff){if (offset>=sizeof(sls_detector_header))return ((sls_detector_header*)buff)->packetNumber;};




  /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 
     
  */
  virtual  char *findNextFrame(char *data, int &ndata, int dsize){
    if (dsize<dataSize) ndata=dsize;
    else ndata=dataSize;
    return data;

  }

  virtual char *readNextFrame(ifstream &filebin) {
    int ff=-1, np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int &ff) {
    int np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) {
	  char *data=new char[dataSize];
	  char *d=readNextFrame(filebin, ff, np, data);
	  if (d==NULL) {delete [] data; data=NULL;}
	  return data;
  }




  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np, char *data) {
	  char *retval=0;
	  int  nd;
	  int fnum = -1;
	  np=0;
	  int  pn;
	  
	  //  cout << dataSize << endl;
	  if (ff>=0)
	    fnum=ff;

	  if (filebin.is_open()) {
	    if (filebin.read(data, dataSize) ){
	      ff=getFrameNumber(data);
	      np=getPacketNumber(data);
	      return data;
	    }
	  }
	    return NULL;
	    
	    
	  
  };

  


};





#endif

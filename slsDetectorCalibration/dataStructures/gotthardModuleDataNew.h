#ifndef GOTTHARDMODULEDATANEW_H
#define  GOTTHARDMODULEDATANEW_H
#include "slsDetectorData.h"





    typedef struct {
        uint64_t frameNumber;    /**< is the frame number */
        uint32_t expLength;        /**< is the subframe number (32 bit eiger) or real time exposure time in 100ns (others) */
        uint32_t packetNumber;    /**< is the packet number */
        uint64_t bunchId;        /**< is the bunch id from beamline */
        uint64_t timestamp;        /**< is the time stamp with 10 MHz clock */
        uint16_t modId;            /**< is the unique module id (unique even for left, right, top, bottom) */
        uint16_t xCoord;        /**< is the x coordinate in the complete detector system */
        uint16_t yCoord;        /**< is the y coordinate in the complete detector system */
        uint16_t zCoord;        /**< is the z coordinate in the complete detector system */
        uint32_t debug;            /**< is for debugging purposes */
        uint16_t roundRNumber;    /**< is the round robin set number */
        uint8_t detType;        /**< is the detector type see :: detectorType */
        uint8_t version;        /**< is the version number of this structure format */
    } sls_detector_header;








class gotthardModuleDataNew : public slsDetectorData<uint16_t> {

 private:
  
  int iframe;
  const int offset;


public:



	/**
     Implements the slsReceiverData structure for the gotthard read out by a module i.e. using the slsReceiver
     (1x1280 pixels, 2 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

	 */


    gotthardModuleDataNew(int off=24*2, int nch=1280): slsDetectorData<uint16_t>(nch, 1, nch*2+off), offset(off) {

		uint16_t **dMask;
		int **dMap;
		int ix, iy;
		int ypixels=1;
		int xpixels=nch;

		dMask=new uint16_t*[1];
		dMap=new int*[1];
		dMap[0] = new int[nch];
		dMask[0] = new uint16_t[nch];

		for(int ix=0; ix<xpixels; ix++) {
		  dMap[0][ix] = 2*ix+offset; 
		  dMask[0][ix] = 0x0;
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

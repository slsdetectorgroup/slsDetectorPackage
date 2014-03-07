#ifndef GOTTHARDMODULEDATA_H
#define  GOTTHARDMODULEDATA_H
#include "slsReceiverData.h"





#define FRAMEMASK 0xFFFFFFFE
#define PACKETMASK 1
#define FRAMEOFFSET 0x1


class gotthardModuleData : public slsReceiverData<uint16_t> {
public:




	/**
     Implements the slsReceiverData structure for the gotthard read out by a module i.e. using the slsReceiver
     (1x1280 pixels, 2 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

	 */


	gotthardModuleData(double c=0): slsReceiverData<uint16_t>(xpixels, ypixels, npackets, buffersize), xtalk(c) {

		uint16_t **dMask;
		int **dMap;
		int ix, iy;
		int initial_offset = 2;
		int offset = initial_offset;

		dMask=new uint16_t*[ypixels];
		dMap=new int*[ypixels];
		for (int i = 0; i < ypixels; i++) {
			dMap[i] = new int[xpixels];
			dMask[i] = new uint16_t[xpixels];
		}

		for(ix=0; ix<ypixels; ++ix)
			for(iy=0; iy<xpixels; ++iy)
				dMask[ix][iy] = 0x0;

		for(ix=0; ix<ypixels; ++ix){
			if(ix == (ypixels/2)){
				offset += initial_offset;//2
				offset++;
			}
			for(iy=0; iy<xpixels; ++iy){
				dMap[ix][iy] = offset;
				offset++;
			}
		}

		setDataMap(dMap);
		setDataMask(dMask);

	};


	  /**

	     Returns the frame number for the given dataset.
	     \param buff pointer to the dataset
	     \returns frame number

	   */

	int getFrameNumber(char *buff){
		int np=(*(int*)buff);
		//gotthards frame header must be incremented
		++np;
		//packet index should be 1 or 2
		return ((np&FRAMEMASK)>>FRAMEOFFSET);
	};



	/**
     gets the packets number (last packet is labelled with 0 and is replaced with 40)
     \param buff pointer to the memory
     \returns packet number

	 */

	int getPacketNumber(char *buff){
		int np=(*(int*)buff);
		//gotthards frame header must be incremented
		++np;
		//packet index should be 1 or 2
		return ((np&PACKETMASK)+1);
	};


	/**
    returns the pixel value as double correcting for the output buffer crosstalk
     \param data pointer to the memory
     \param ix coordinate in the x direction
     \param iy coordinate in the y direction
     \returns channel value as double

	 */
	double getValue(char *data, int ix, int iy=0) {
		//check how it is for gotthard
	    if (xtalk==0)
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
	const static int xpixels = 1280;
	const static int ypixels = 1;
	const static int npackets = 2;
	const static int buffersize = 1286;
};





#endif

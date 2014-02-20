#ifndef GOTTHARDSHORTMODULEDATA_H
#define  GOTTHARDSHORTMODULEDATA_H
#include "slsReceiverData.h"



#define X_PIXELS 256
#define Y_PIXELS 1
#define NPACKETS 1
#define BUFFERSIZE 518



class gotthardShortModuleData : public slsReceiverData<uint16_t> {
public:




	/**
     Implements the slsReceiverData structure for the gotthard short read out by a module i.e. using the slsReceiver
     (1x256 pixels, 1 packet 256 large etc.)
     \param c crosstalk parameter for the output buffer

	 */


	gotthardShortModuleData(double c=0): slsReceiverData<uint16_t>(X_PIXELS, Y_PIXELS, NPACKETS, BUFFERSIZE), xtalk(c){

		uint16_t **dMask;
		int **dMap;
		int ix, iy;
		int offset = 2;

		dMask=new uint16_t*[Y_PIXELS];
		dMap=new int*[Y_PIXELS];
		for (int i = 0; i < Y_PIXELS; i++) {
			dMap[i] = new int[X_PIXELS];
			dMask[i] = new uint16_t[X_PIXELS];
		}

		for(ix=0; ix<Y_PIXELS; ++ix)
			for(iy=0; iy<X_PIXELS; ++iy)
				dMask[ix][iy] = 0x0;

		for(ix=0; ix<Y_PIXELS; ++ix)
			for(iy=0; iy<X_PIXELS; ++iy){
				dMap[ix][iy] = offset;
				offset++;
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
		return (*(int*)buff);
	};



	/**
     gets the packets number (last packet is labelled with 0 and is replaced with 40)
     \param buff pointer to the memory
     \returns packet number

	 */

	int getPacketNumber(char *buff){
			return 1;
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

};



#endif

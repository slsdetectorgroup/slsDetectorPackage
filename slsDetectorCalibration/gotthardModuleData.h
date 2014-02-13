#ifndef GOTTHARDMODULEDATA_H
#define  GOTTHARDMODULEDATA_H
#include "slsReceiverData.h"



#define X_PIXELS 1280
#define Y_PIXELS 1
#define NPACKETS 2
#define BUFFERSIZE 1286
#define NUMPACKETS 2
#define NUMSHORTPACKETS 1
#define FRAMEMASK 0xFFFFFFFE
#define PACKETMASK 1
#define FRAMEOFFSET 0x1
#define SHORT_FRAMEMASK 0xFFFFFFFF
#define SHORT_PACKETMASK 0
#define SHORT_FRAMEOFFSET 0


class gotthardModuleData : public slsReceiverData<uint16_t> {
public:




	/**
     Implements the slsReceiverData structure for the gotthard read out by a module i.e. using the slsReceiver
     (1x1280 pixels, 2 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

	 */


	gotthardModuleData(double c=0, bool s=-1): slsReceiverData<uint16_t>(X_PIXELS, Y_PIXELS, NPACKETS, BUFFERSIZE), xtalk(c),shortFrame(s) {


		if(shortFrame == -1)
			shortFrame = 0;
		else
			shortFrame = 1;

		uint16_t **dMask;
		int **dMap;
		int ix, iy;
		int initial_offset = 2;
		int offset = initial_offset;


		dMask=new uint16_t*[Y_PIXELS];
		dMap=new int*[Y_PIXELS];
		for (int i = 0; i < Y_PIXELS; i++) {
			dMap[i] = new int[X_PIXELS];
			dMask[i] = new uint16_t[X_PIXELS];
		}


		for(ix=0; ix<Y_PIXELS; ++ix)
			for(iy=0; iy<X_PIXELS; ++iy)
				dMask[ix][iy] = 0x0;


		if(!shortFrame){
			for(ix=0; ix<Y_PIXELS; ++ix){
				if(ix == (Y_PIXELS/2)){
					offset += initial_offset;//2
					offset++;
				}
				for(iy=0; iy<X_PIXELS; ++iy){
					dMap[ix][iy] = offset;
					offset++;
				}
			}
		}else{
			for(ix=0; ix<Y_PIXELS; ++ix)
				for(iy=0; iy<X_PIXELS; ++iy){
					dMap[ix][iy] = offset;
					offset++;
				}

		}



		setDataMap(dMap);
		setDataMask(dMask);
		if(shortFrame){
			setFrameIndexMask(FRAMEMASK);
			setPacketIndexMask(PACKETMASK);
			setFrameIndexOffset(FRAMEOFFSET);
		}else{
			setFrameIndexMask(SHORT_FRAMEMASK);
			setPacketIndexMask(SHORT_PACKETMASK);
			setFrameIndexOffset(SHORT_FRAMEOFFSET);
		}

	};



	/**
     gets the packets number (last packet is labelled with 0 and is replaced with 40)
     \param buff pointer to the memory
     \returns packet number

	 */

	int getPacketNumber(char *buff){
		if(shortFrame)
			return 1;

		int np=(*(int*)buff)&0x1;
		++np;
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
	int shortFrame; /**<short frame */


};



#endif

#ifndef EIGERMODULEDATA_H
#define  EIGERMODULEDATA_H
#include "slsReceiverData.h"



class eigerHalfModuleData : public slsReceiverData<uint32_t> {
public:




	/**
     Implements the slsReceiverData structure for the eiger prototype read out by a half module i.e. using the slsReceiver
     (256*256 pixels, 512 packets for 16 bit mode, 256 for 8, 128 for 4, 1024 for 32, 1040  etc.)
     \param d dynamic range
     \param c crosstalk parameter for the output buffer

	 */


	eigerHalfModuleData(int dr, int np, int bsize, int dsize, double c=0): slsReceiverData<uint32_t>(xpixels, ypixels, np, bsize),
	xtalk(c), dynamicRange(dr), bufferSize(bsize), dataSize(dsize){


		int **dMap;
		uint32_t **dMask;
		int ix, iy;

		dMap=new int*[ypixels];
		dMask=new uint32_t*[ypixels];


		for (int i = 0; i < ypixels; i++) {
			dMap[i] = new int[xpixels];
			dMask[i] = new uint32_t[xpixels];
		}

		//Map
		int totalNumberOfBytes = 1040 * dynamicRange * 16 *2; //for both 1g and 10g
		int iPacket1 = 8;
		int iPacket2 = (totalNumberOfBytes/2) + 8;
		int iData1 = 0, iData2 = 0;
		int iPort;

		for (int ir=0; ir<ypixels; ir++) {
			for (int ic=0; ic<xpixels; ic++) {
				iPort = ic / (xpixels/2);
				if(!iPort){
					dMap[ir][ic]  = iPacket1;
					iPacket1 += (dynamicRange / 8);
					iData1 +=(dynamicRange / 8);
					if(iData1 >= dataSize){
						iPacket1 += 16;
						iData1 = 0;
					}
				}else{
					dMap[ir][ic]  = iPacket2;
					iPacket2 += (dynamicRange / 8);
					iData2 +=(dynamicRange / 8);
					if(iData2 >= dataSize){
						iPacket2 += 16;
						iData2 = 0;
					}
				}
			}
		}




		//Mask
		for(ix=0; ix<ypixels; ++ix)
			for(iy=0; iy<xpixels; ++iy)
				dMask[ix][iy] = 0x0;

		setDataMap(dMap);
		setDataMask(dMask);




	};


	/** Returns the frame number for the given dataset.
	     \param buff pointer to the dataset
	     \returns frame number
	 */
	int getFrameNumber(char *buff){
		return htonl(*(unsigned int*)((eiger_image_header *)((char*)(buff)))->fnum);
	};


	/** gets the packets number (last packet is labelled with 0 and is replaced with 40)
     	 \param buff pointer to the memory
     	 \returns packet number
	 */
	int getPacketNumber(char *buff){
		return htonl(*(unsigned int*)((eiger_packet_header *)((char*)(buff)))->num2);
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
		if (xtalk==0)
			  return slsDetectorData<uint32_t>::getValue(data, ix, iy);
		else
			return slsDetectorData<uint32_t>::getValue(data, ix, iy)-xtalk*slsDetectorData<uint32_t>::getValue(data, ix-1, iy);
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

	const static int xpixels = 1024;
	const static int ypixels = 256;
	const int bufferSize;
	const int dataSize;
	const int dynamicRange;


	/** structure of an eiger image header*/
	typedef struct
	{
		unsigned char header_before[20];
		unsigned char  fnum[4];
		unsigned char  header_after[24];
	} eiger_image_header;

	/** structure of an eiger image header*/
	typedef struct
	{
		unsigned char num1[4];
		unsigned char num2[4];
	} eiger_packet_header;

};



#endif

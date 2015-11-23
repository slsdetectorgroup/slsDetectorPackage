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


	eigerHalfModuleData(int dr, int np, int bsize, int dsize, bool top, double c=0): slsReceiverData<uint32_t>(xpixels, ypixels, np, bsize),
	xtalk(c), bufferSize(bsize), actualDataSize(dsize), dynamicRange(dr), numberOfPackets(np), top(top),header_t(0), footer_t(0){


		int **dMap;
		uint32_t **dMask;

		dMap=new int*[ypixels];
		dMask=new uint32_t*[ypixels];


		for (int i = 0; i < ypixels; i++) {
			dMap[i] = new int[xpixels];
			dMask[i] = new uint32_t[xpixels];
		}

		//Map
		int totalNumberOfBytes = numberOfPackets * bufferSize;
		int iPacket1 = 8;
		int iPacket2 = (totalNumberOfBytes/2) + 8;
		int iData1 = 0, iData2 = 0;
		int increment = (dynamicRange/8);
		int ic_increment = 1;
		if (dynamicRange == 4) {
			increment = 1;
			ic_increment = 2;
		}
		int iPort;

		if(top){
			for (int ir=0; ir<ypixels; ir++) {
				for (int ic=0; ic<xpixels; ic = ic + ic_increment) {
					iPort = ic / (xpixels/2);
					if(!iPort){
						dMap[ir][ic]  = iPacket1;
						iPacket1 += increment;
						iData1 += increment;
						//increment header
						if(iData1 >= actualDataSize){
							iPacket1 += 16;
							iData1 = 0;
						}
					}else{
						dMap[ir][ic]  = iPacket2;
						iPacket2 += increment;
						iData2 += increment;
						//increment header
						if(iData2 >= actualDataSize){
							iPacket2 += 16;
							iData2 = 0;
						}
					}
				}
			}
		}


		//bottom
		else{
			iData1 = 0; iData2 = 0;
			int numbytesperlineperport;

			switch(dynamicRange){
				case 4: numbytesperlineperport = 256; break;
				case 8:	numbytesperlineperport = 512; break;
				case 16:numbytesperlineperport = 1024; break;
				case 32:numbytesperlineperport = 2048; break;
			}



			iPacket1 = (totalNumberOfBytes/2) - numbytesperlineperport - 8;
			iPacket2 = totalNumberOfBytes - numbytesperlineperport - 8;
			if (dynamicRange == 32){
				iPacket1 -= 16;
				iPacket2 -= 16;
			}

			for (int ir=0; ir<ypixels; ir++) {
				for (int ic=0; ic<xpixels; ic = ic + ic_increment) {
					iPort = ic / (xpixels/2);
					if(!iPort){
						dMap[ir][ic]  = iPacket1;
						iPacket1 += increment;
						iData1 += increment;
						if(dynamicRange == 32){
							if(iData1 == numbytesperlineperport){
								iPacket1 -= (numbytesperlineperport*2 + 16*3);
								iData1 = 0;
							}
							if(iData1 == actualDataSize){
								iPacket1 += 16;
							}
						}else if((iData1 % numbytesperlineperport) == 0){
							iPacket1 -= (numbytesperlineperport*2);
							if(iData1 == actualDataSize){
								iPacket1 -= 16;
								iData1 = 0;
							}

						}
					}
					//other port
					else{
						dMap[ir][ic]  = iPacket2;
						iPacket2 += increment;
						iData2 += increment;
						if(dynamicRange == 32){
							if(iData2 == numbytesperlineperport){
								iPacket2 -= (numbytesperlineperport*2 + 16*3);
								iData2 = 0;
							}
							if(iData2 == actualDataSize){
								iPacket2 += 16;
							}
						}else if((iData2 % numbytesperlineperport) == 0){
							iPacket2 -= (numbytesperlineperport*2);
							if(iData2 == actualDataSize){
								iPacket2 -= 16;
								iData2 = 0;
							}
						}
					}
				}
			}
		}




		//Mask
		for(int ir=0; ir<ypixels; ++ir)
			for(int ic=0; ic<xpixels; ++ic)
				dMask[ir][ic] = 0x0;

		setDataMap(dMap);
		setDataMask(dMask);




	};






	/** Returns the frame number for the given dataset.
	     \param buff pointer to the dataset
	     \returns frame number
	 */
	int getFrameNumber(char *buff){
		footer_t = (eiger_packet_footer_t*)(buff + actualDataSize + sizeof(eiger_packet_header_t));
		return ((uint32_t)(*( (uint64_t*) footer_t)));
	};







	/** gets the packets number
     	 \param buff pointer to the memory
     	 \returns packet number
	 */
	int getPacketNumber(char *buff){
		footer_t = (eiger_packet_footer_t*)(buff + actualDataSize + sizeof(eiger_packet_header_t));
		return(*( (uint16_t*) footer_t->packetnum));
	};



	/** gets the dynamic range for offline processing
     	 \param buff pointer to the memory
     	 \returns dynamic range
	 */
	static int getDynamicRange(char *buff){
		eiger_packet_header_t* header_t;
		header_t = (eiger_packet_header_t*)buff;
		return(*( (uint8_t*) header_t->dynamicrange));
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
			return getChannel(data, ix, iy, dynamicRange);
		else
			return getChannel(data, ix, iy,dynamicRange)-xtalk * getChannel(data, ix-1, iy,dynamicRange);
	};


	/**

	     Returns the value of the selected channel for the given dataset. Virtual function, can be overloaded.
	     \param data pointer to the dataset (including headers etc)
	     \param ix pixel number in the x direction
	     \param iy pixel number in the y direction
	     \param dr dynamic range
	     \returns data for the selected channel, with inversion if required

	 */

	virtual int getChannel(char *data, int ix, int iy, int dr) {
		uint32_t m=0, n = 0;
		int linesperpacket,newix, newiy,origX;


		//cout <<"ix:"<<ix<<" nx:"<<nx<<" iy:"<<iy<<" ny:"<<ny<<" datamap[iy][ix]:"<< dataMap[iy][ix] <<" datasize:"<< dataSize <<endl;
		if (ix>=0 && ix<nx && iy>=0 && iy<ny && dataMap[iy][ix]>=0 && dataMap[iy][ix]<dataSize) {
			m=dataMask[iy][ix];


			//pixelpos1d = (nx * iy + ix);

			switch(dr){
			case 4: linesperpacket=4;break;
			case 8: linesperpacket=2;break;
			case 16: linesperpacket=1;break;
			case 32: linesperpacket=2;break;
			}

			//each byte is shared by 2 pixels for 4 bit mode
			origX = ix;
			if((dr == 4) && (ix%2))
				ix--;


			//check if missing packet, get to pixel at start of packet

			//to get the starting of a packet, ix is divided by 512pixels because of 2 ports
			newix = ix - (ix%512);
			//iy divided by linesperpacket depending on bitmode
			if(!(iy%linesperpacket))
				newiy = iy;
			else
				newiy = (iy - (iy%linesperpacket));

			header_t = (eiger_packet_header_t*)((char*)(data +(dataMap[newiy][newix]-8)));
			if(*( (uint16_t*) header_t->missingpacket)==0xFFFF){
			  //	cprintf(RED,"missing packet\n");
			  return -1;
			}



		}else{
			cprintf(RED,"outside limits\n");
			return -99;
		}

		//get proper data
		n = ((uint32_t)(*((uint32_t*)(((char*)data)+(dataMap[iy][ix])))));

		//each byte is shared by 2 pixels for 4 bit mode
		if(dr == 4){
			if(ix != origX)
				return ((n & 0xf0)>>4)^m;
			return (n & 0xf)^m;
		}
		else if(dr == 8)	return (n & 0xff)^m;
		else if(dr == 16)	return (n & 0xffff)^m;
		else				return (n & 0xffffffff)^m;


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
	const int actualDataSize;
	const int dynamicRange;
	const int numberOfPackets;
	bool top;


	/** structure of an eiger packet*/
	typedef struct{
		unsigned char subframenum[4];
		unsigned char missingpacket[2];
		unsigned char portnum[1];
		unsigned char dynamicrange[1];
	} eiger_packet_header_t;

	typedef struct{
		unsigned char framenum[6];
		unsigned char packetnum[2];
	} eiger_packet_footer_t;

	eiger_packet_header_t* header_t;
	eiger_packet_footer_t* footer_t;
};




#endif

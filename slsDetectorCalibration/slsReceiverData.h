#ifndef SLSRECEIVERDATA_H
#define  SLSRECEIVERDATA_H

#include "slsDetectorData.h"
#include <cstring>
#include <stdlib.h>			// exit()
template <class dataType>
class slsReceiverData : public slsDetectorData<dataType> {


public:

	/**
  slsReceiver data structure. Works for data acquired using the slsDetectorReceiver subdivided in different packets with headers and footers.
  Inherits and implements slsDetectorData.

  Constructor (no error checking if datasize and offsets are compatible!)
  \param npx number of pixels in the x direction
  \param npy number of pixels in the y direction (1 for strips)
  \param np number of packets
  \param psize packets size
  \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
  \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)
  \param dROI Array of size nx*ny. The elements are 1s if the channel is good or in the ROI, 0 is bad or out of the ROI. NULL (default) means all 1s.

	 */
	slsReceiverData(int npx, int npy, int np, int psize, int **dMap=NULL, dataType **dMask=NULL, int **dROI=NULL): slsDetectorData<dataType>(npx, npy, np*psize, dMap, dMask, dROI), nPackets(np), packetSize(psize) {};


	/**

     Returns the frame number for the given dataset. Virtual func: works for slsDetectorReceiver data (also for each packet), but can be overloaded.
     \param buff pointer to the dataset
     \returns frame number

	 */

	virtual  int getFrameNumber(char *buff){return ((*(int*)buff)&(0xffffff00))>>8;};

	/**

     Returns the packet number for the given dataset. Virtual func: works for slsDetectorReceiver packets, but can be overloaded.
     \param buff pointer to the dataset
     \returns packet number number

	 */

	virtual int getPacketNumber(char *buff){return (*(int*)buff)&0xff;};



	/**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!
     \param data pointer to the memory to be analyzed
     \param ndata size of frame returned
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the first packet of the last good frame (might be incomplete if npackets lower than the number of packets), or NULL if no frame is found

	 */

	virtual  char *findNextFrame(char *data, int &ndata, int dsize) {
		char *retval=NULL, *p=data;
		int dd=0;
		int fn, fnum=-1,  np=0, pnum=-1;
		while (dd<=(dsize-packetSize)) {
			pnum=getPacketNumber(p);
			fn=getFrameNumber(p);
			//cout <<"fnum:"<<fn<<" pnum:"<<pnum<<" np:"<< np << "\t";

			if (pnum<1 || pnum>nPackets) {
				//cout << "Bad packet number " << pnum << " frame "<< fn  << endl;
				retval=NULL;
				np=0;
			}     else if  (pnum==1) {
				retval=p;
				if (np>0)
					/*cout << "*Incomplete frame number " << fnum << endl;*/
					np=0;
				fnum=fn;
			} else if (fn!=fnum) {
				if (fnum!=-1) {
					/* cout << " **Incomplete frame number " << fnum << " pnum " << pnum << " " << getFrameNumber(p) << endl;*/
					retval=NULL;
				}
				np=0;
			}
			p+=packetSize;
			dd+=packetSize;
			np++;
			//cout <<"fnum:"<<fn<<" pnum:"<<pnum<<" np:"<< np << "\t";
			// cout << pnum << " " << fn << " " << np << " " << dd << " " << dsize << endl;
			if (np==nPackets){
				if (pnum==nPackets) {
					//cprintf(BG_GREEN, "Frame Found\n");
				  // cout << "Frame found!" << endl;
					break;
				}     else {
					//cprintf(BG_RED, "Too many packets for this frame! fnum:%d, pnum:%d np:%d\n",fnum,pnum,np);
					cout << "Too many packets for this frame! "<< fnum << " " << pnum << endl;cprintf(BG_RED,"Exiting\n");exit(-1);
					retval=NULL;
				}
			}
		}
		if (np<nPackets) {
			if (np>0){
				//cprintf(BG_RED, "Too few packets for this frame! fnum:%d, pnum:%d np:%d\n",fnum,pnum,np);
				cout << "Too few packets for this frame! "<< fnum << " " << pnum << " " << np <<endl;cprintf(BG_RED,"Exiting\n");exit(-1);

			}
		}

		ndata=np*packetSize;
		//  cout << "return " << ndata << endl;
		return retval;
	};

	/**

     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!
     \param filebin input file stream (binary)
     \returns pointer to the first packet of the last good frame, NULL if no frame is found or last frame is incomplete

	 */

	virtual char *readNextFrame(ifstream &filebin) {
		char *data=new char[packetSize*nPackets];
		char *retval=0;
		int np=0, nd;

		if (filebin.is_open()) {
			while (filebin.read(data+np*packetSize,packetSize)) {

				if (np==(nPackets-1)) {

					retval=findNextFrame(data,nd,packetSize*nPackets);
					np=nd/packetSize;
					//      cout << np << endl;


					if (retval==data && np==nPackets) {
						//      cout << "-" << endl;
						return data;

					}       else if (np>nPackets) {
						cout << "too many packets!!!!!!!!!!" << endl;
						delete [] data;
						return NULL;
					} else if (retval!=NULL) {
						//  cout << "+" << endl;;
						for (int ip=0; ip<np; ip++)
							memcpy(data+ip*packetSize,retval+ip*packetSize,packetSize);
					}

				} else if (np>nPackets) {
					cout << "*******too many packets!!!!!!!!!!" << endl;
					delete [] data;
					return NULL;
				} else {
					//  cout << "." << endl;;
					np++;
				}
			}
		}
		delete [] data;
		return NULL;
	};


	/**

	     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!
	     \param filebin input file stream (binary)
	     \param fnum frame number of frame returned
	     \returns pointer to the first packet of the last good frame, NULL if no frame is found or last frame is incomplete

		 */

		virtual char *readNextFrame(ifstream &filebin, int& fnum) {
			char *data=new char[packetSize*nPackets];
			char *retval=0;
			int np=0, nd;
			fnum = -1;

			if (filebin.is_open()) {
				while (filebin.read(data+np*packetSize,packetSize)) {

					if (np==(nPackets-1)) {

						fnum=getFrameNumber(data); //cout << "fnum:"<<fnum<<endl;
						retval=findNextFrame(data,nd,packetSize*nPackets);
						np=nd/packetSize;
						//      cout << np << endl;


						if (retval==data && np==nPackets) {
							//      cout << "-" << endl;
							return data;

						}       else if (np>nPackets) {
							cout << "too many packets!!!!!!!!!!" << endl;
							delete [] data;
							return NULL;
						} else if (retval!=NULL) {
							//  cout << "+" << endl;;
							for (int ip=0; ip<np; ip++)
								memcpy(data+ip*packetSize,retval+ip*packetSize,packetSize);
						}

					} else if (np>nPackets) {
						cout << "*******too many packets!!!!!!!!!!" << endl;
						delete [] data;
						return NULL;
					} else {
						//  cout << "." << endl;;
						np++;
						//cout<<"np:"<<np<<endl;
					}
				}
			}
			delete [] data;
			return NULL;
		};

		virtual int* readNextFramewithMissingPackets(ifstream &filebin, int& fnum) {return NULL;}
		virtual void getChannelArray(double* data, char* buffer){};
		virtual int* readNextFrameOnlyData(ifstream &filebin, int& fnum) {return NULL;};
		virtual int* decodeData(int* datain) {return NULL;};

private:
	const int nPackets; /**<number of UDP packets constituting one frame */
	const int packetSize; /**< size of a udp packet */
};



#endif

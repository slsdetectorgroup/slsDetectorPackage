#ifndef SLSDETECTORDATA_H
#define  SLSDETECTORDATA_H

#include <iostream>
#include <fstream>

using namespace std;


template <class dataType>
class slsDetectorData {


 public:

  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  slsDetectorData(int npx, int npy, int np, int psize, int **dMap=NULL, dataType **dMask=NULL, int **dROI=NULL): nx(npx), ny(npy), nPackets(np), packetSize(psize) {

   

    dataMask=new dataType*[ny];
    for(int i = 0; i < ny; i++) {
      dataMask[i] = new dataType[nx];
    }
    dataMap=new int*[ny];
    for(int i = 0; i < ny; i++) {
      dataMap[i] = new int[nx];
    }
    
    dataROIMask=new int*[ny];
    for(int i = 0; i < ny; i++) {
      dataROIMask[i] = new int[nx];
    }
    
    setDataMap(dMap);
    setDataMask(dMask);
    setDataROIMask(dROI);

  };

  virtual ~slsDetectorData() {
    for(int i = 0; i < ny; i++) {
    delete [] dataMap[i];
    delete [] dataMask[i];
    delete [] dataROIMask[i];
    }
    delete [] dataMap;
    delete [] dataMask;
    delete [] dataROIMask;
  }

  void setDataMap(int **dMap=NULL) {


    if (dMap==NULL) {
      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMap[iy][ix]=ix*ny+ix;
    } else {
      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMap[iy][ix]=dMap[iy][ix];
    }
    
  };

  void setDataMask(dataType **dMask=NULL){

    if (dMask!=NULL) {

      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMask[iy][ix]=dMask[iy][ix];
    }
    
  };
  void setDataROIMask(int **dROI=NULL){

    if (dROI!=NULL) {

      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataROIMask[iy][ix]=dROI[iy][ix];
    } else {

      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataROIMask[iy][ix]=1;

    }
      
    
  };

  int setGood(int ix, int iy, int i=1) {  if (ix>=0 && ix<nx && iy>=0 && iy<ny) dataROIMask[iy][ix]=i; return isGood(ix,iy);};

  int isGood(int ix, int iy) {  if (ix>=0 && ix<nx && iy>=0 && iy<ny) return dataROIMask[iy][ix]; else return -1;};


  void getDetectorSize(int &npx, int &npy){npx=nx; npy=ny;};


  /**

     Returns the value of the selected channel for the given dataset (no error checking if number of pixels are compatible!)
     \param data pointer to the dataset (including headers etc)
     \param ix pixel numb er in the x direction
     \param iy pixel number in the y direction

     \returns data for the selected channel, with inversion if required

  */


  virtual dataType getChannel(char *data, int ix, int iy=0) {
    dataType m=0, d=0;
    if (ix>=0 && ix<nx && iy>=0 && iy<ny && dataMap[iy][ix]>=0 && dataMap[iy][ix]<nPackets*packetSize) {
      m=dataMask[iy][ix];
      d=*((dataType*)(data+dataMap[iy][ix]));
    }  
    return d^m;
  };

  virtual double getValue(char *data, int ix, int iy=0) {return (double)getChannel(data, ix, iy);};


  virtual  int getFrameNumber(char *buff){return ((*(int*)buff)&(0xffffff00))>>8;};
  virtual int getPacketNumber(char *buff) {return (*(int*)buff)&0xff;};


  virtual  char *findNextFrame(char *data, int &npackets, int dsize) {
    char *retval=NULL, *p=data;
    int dd=0;
    int fn, fnum=-1,  np=0, pnum=-1;
    while (dd<=(dsize-packetSize)) {
      pnum=getPacketNumber(p);
      fn=getFrameNumber(p);
      if (pnum<0 || pnum>=nPackets) {
	cout << "Bad packet number " << pnum << " frame "<< fn << endl;
	retval=NULL;
	continue;
      }	
      if (pnum==1) {
	fnum=fn;
	retval=p;
	if (np>0)
	  cout << "*Incomplete frame number " << fnum << endl;
	np=0;
      } else if (fn!=fnum) {
	if (fnum!=-1) {
	  cout << " **Incomplete frame number " << fnum << " pnum " << pnum << " " << getFrameNumber(p) << endl;
	  retval=NULL;
	}
	np=0;
      }
      p+=packetSize;
      dd+=packetSize;
      np++;
      if (np==nPackets)
	if (pnum==nPackets)
	  break;
	else {
	  cout << "Too many packets for this frame! "<< fnum << " " << pnum << endl;
	  retval=NULL;
	}
    }
    if (np<40) {
      if (np>0) 
	cout << "Too few packets for this frame! "<< fnum << " " << pnum << endl;
    }
    
    npackets=np;
    
    return retval;
  };


  virtual char *readNextFrame(ifstream &filebin) {
    

/*     if (oldbuff)  */
/*       delete [] oldbuff; */

/*     oldbuff=buff; */
    
    // char *p=new char[1286];
    char *data=new char[packetSize*nPackets];
    char *retval=0;
    int np=40;

    if (filebin.is_open()) {
      while (filebin.read(data+np*packetSize,packetSize)) {

	if (np==nPackets) {

	  retval=findNextFrame(data,np,packetSize*nPackets);
	  if (retval==data && np==nPackets)
	    return data;
	  else if (np>nPackets) {
	    cout << "too many packets!!!!!!!!!!" << endl;
	    delete [] data;
	    return NULL;
	  } else {
	    for (int ip=0; ip<np; ip++)
	      memcpy(data+ip*packetSize,retval+ip*packetSize,packetSize);
	  } 

	} else if (np>nPackets) {
	  cout << "*******too many packets!!!!!!!!!!" << endl;
	  delete [] data;
	  return NULL;
	} else {
	  // memcpy(data+np*1286,p,1286);
	  np++;
	}
	    
      }
    }
    delete [] data;
    return NULL;
  };



 private:
  const int nx; /**< Number of pixels in the x direction */
  const int ny; /**< Number of pixels in the y direction */
  const int nPackets; /**<number of UDP packets constituting one frame */
  const int packetSize; /**< size of a udp packet */
  int **dataMap; /**< Array of size nx*ny storing the pointers to the data in the dataset (as offset)*/
  dataType **dataMask; /**< Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required) */
  int **dataROIMask; /**< Array of size nx*ny 1 if channel is good (or in the ROI), 0 if bad channel (or out of ROI)   */

};



#endif

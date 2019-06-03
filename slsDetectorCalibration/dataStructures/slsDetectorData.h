#ifndef SLSDETECTORDATA_H
#define SLSDETECTORDATA_H

#include <inttypes.h>
#include <iostream>
#include <fstream>

using namespace std;


template <class dataType>
class slsDetectorData {

 protected:
  const int nx; /**< Number of pixels in the x direction */
  const int ny; /**< Number of pixels in the y direction */
  int dataSize; /**<size of the data constituting one frame */
  int **dataMap; /**< Array of size nx*ny storing the pointers to the data in the dataset (as offset)*/
  dataType **dataMask; /**< Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required) */
  int **dataROIMask; /**< Array of size nx*ny 1 if channel is good (or in the ROI), 0 if bad channel (or out of ROI)   */
  int *xmap;
  int *ymap;
  
 public:
  
  /**
     
     
     General slsDetectors data structure. Works for data acquired using the slsDetectorReceiver. Can be generalized to other detectors (many virtual funcs).

     Constructor (no error checking if datasize and offsets are compatible!)
  \param npx number of pixels in the x direction
  \param npy number of pixels in the y direction (1 for strips)
  \param dsize size of the data
  \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
  \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)
  \param dROI Array of size nx*ny. The elements are 1s if the channel is good or in the ROI, 0 is bad or out of the ROI. NULL (default) means all 1s. 
  
  */
 slsDetectorData(int npx, int npy, int dsize, int **dMap=NULL, dataType **dMask=NULL, int **dROI=NULL): nx(npx), ny(npy), dataSize(dsize) {
    
    
    xmap=new int[dsize/sizeof(dataType)];
    ymap=new int[dsize/sizeof(dataType)];
    

    //  if (dataMask==NULL) {
    dataMask=new dataType*[ny];
    for(int i = 0; i < ny; i++) {
      dataMask[i] = new dataType[nx];
    }
    // }
    
    // if (dataMap==NULL) {
    dataMap=new int*[ny];
    for(int i = 0; i < ny; i++) {
      dataMap[i] = new int[nx];
      }
    // }
    // if (dataROIMask==NULL) {
    dataROIMask=new int*[ny];
    for(int i = 0; i < ny; i++) {
      dataROIMask[i] = new int[nx];
      for (int j=0; j<nx; j++)
	dataROIMask[i][j]=1;
    }
    // }
    for (int ip=0; ip<dsize/sizeof(dataType); ip++){
      xmap[ip]=-1;
      ymap[ip]=-1;
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
    delete [] xmap;
    delete [] ymap;
  };
  
  
  /**
     defines the data map (as offset) - no error checking if datasize and offsets are compatible!
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset). If NULL (default),the data are arranged as if read out row by row (dataMap[iy][ix]=(iy*nx+ix)*sizeof(dataType);)
  */
  void setDataMap(int **dMap=NULL) {
    
    int ip=0;
    int ix, iy;
    if (dMap==NULL) {
      for (iy=0; iy<ny; iy++) {
	for (ix=0; ix<nx; ix++) {
		dataMap[iy][ix]=(iy*nx+ix)*sizeof(dataType);
	}
      }
    } else {
      //cout << "set dmap "<< dataMap << " " << dMap << endl;
      for (iy=0; iy<ny; iy++){
			 // cout << iy << endl;
	for (ix=0; ix<nx; ix++) {
	  dataMap[iy][ix]=dMap[iy][ix];
	  // cout << ix << " " << iy << endl;
	  /*ip=dataMap[ix][iy]/sizeof(dataType);
	    xmap[ip]=ix;
	    ymap[ip]=iy;Annaa*/
	}
      }
    }
    for (iy=0; iy<ny; iy++){
      for (ix=0; ix<nx; ix++) {
	ip=dataMap[iy][ix]/sizeof(dataType);
	xmap[ip]=ix;
	ymap[ip]=iy;
      }
    }
    
	  // cout << "nx:" <<nx << " ny:" << ny << endl;
    
  };
  

  /**
     defines the data mask i.e. the polarity of the data
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)

  */
  void setDataMask(dataType **dMask=NULL){
    
    if (dMask!=NULL) {
      
      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMask[iy][ix]=dMask[iy][ix];
    } else {
      for (int iy=0; iy<ny; iy++)
	for (int ix=0; ix<nx; ix++)
	  dataMask[iy][ix]=0;
    }
  };
  /**
     defines the region of interest and/or the bad channels mask
     \param dROI Array of size nx*ny. The lements are 1s if the channel is good or in the ROI, 0 is bad or out of the ROI. NULL (default) means all 1s. 
     
  */
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
  
  /**
     Define bad channel or roi mask for a single channel
     \param ix channel x coordinate
     \param iy channel y coordinate (1 for strips)
     \param i 1 if pixel is good (or in the roi), 0 if bad
     \returns 1 if pixel is good, 0 if it's bad, -1 if pixel is out of range
  */
  int setGood(int ix, int iy, int i=1) {  if (ix>=0 && ix<nx && iy>=0 && iy<ny) dataROIMask[iy][ix]=i; return isGood(ix,iy);};
  /**
     Define bad channel or roi mask for a single channel
     \param ix channel x coordinate
     \param iy channel y coordinate (1 for strips)
     \returns 1 if pixel is good, 0 if it's bad, -1 if pixel is out of range
  */
  int isGood(int ix, int iy) {  if (ix>=0 && ix<nx && iy>=0 && iy<ny) return dataROIMask[iy][ix]; else return -1;};
  
  /**
     Returns detector size in x,y
     \param npx reference to number of channels in x
     \param npy reference to number of channels in y (will be 1 for strips)
     \returns total number of channels
  */
  int getDetectorSize(int &npx, int &npy){npx=nx; npy=ny; return nx*ny;};
  
  /** Returns the size of the data frame */
  int getDataSize() {return dataSize;};
  
  /** changes the size of the data frame */
  int setDataSize(int d) {dataSize=d; return dataSize;};

  
  virtual void getPixel(int ip, int &x, int &y) {x=xmap[ip]; y=ymap[ip];};
  
  virtual dataType **getData(char *ptr, int dsize=-1) {
    
    dataType **data;
    int ix,iy;
    data=new dataType*[ny];
    for(int i = 0; i < ny; i++) {
      data[i]=new dataType[nx];
    }
    if (dsize<=0 || dsize>dataSize) dsize=dataSize;
    for (int ip=0; ip<(dsize/sizeof(dataType)); ip++) {
      getPixel(ip,ix,iy);
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	data[iy][ix]=getChannel(ptr,ix,iy);
      }
    }
    return data;
    
  };
  
  virtual double **getImage(char *ptr, int dsize=-1) {
    
    double **data;
    int ix,iy;
    data=new double*[ny];
    for(int i = 0; i < ny; i++) {
      data[i]=new double[nx];
    }
    if (dsize<=0 || dsize>dataSize) dsize=dataSize;
    for (int ip=0; ip<(dsize/sizeof(dataType)); ip++) {
      getPixel(ip,ix,iy);
      if (ix>=0 && ix<nx && iy>=0 && iy<ny) {
	data[iy][ix]=getValue(ptr,ix,iy);
      }
    }
    return data;
    
  };
  
  /** 
     Returns the value of the selected channel for the given dataset. Virtual function, can be overloaded.
     \param data pointer to the dataset (including headers etc)
     \param ix pixel number in the x direction
     \param iy pixel number in the y direction
     \returns data for the selected channel, with inversion if required

  */
  virtual dataType getChannel(char *data, int ix, int iy=0) {
    dataType m=0, d=0;
    if (ix>=0 && ix<nx && iy>=0 && iy<ny && dataMap[iy][ix]>=0 && dataMap[iy][ix]<dataSize) {
      // cout << ix << " " << iy << " " ;
      //cout << dataMap[ix][iy] << " " << (void*)data << " " << dataSize<< endl;
      m=dataMask[iy][ix];
      d=*((dataType*)(data+dataMap[iy][ix]));
    }  
    return d^m;
  };

  /**

     Returns the value of the selected channel for the given dataset. Virtual function, can be overloaded.
     \param data pointer to the dataset (including headers etc)
     \param ix pixel number in the x direction
     \param iy pixel number in the y direction
     \returns data for the selected channel, with inversion if required or -1 if its a missing packet

  */
  
  virtual int getChannelwithMissingPackets(char *data, int ix, int iy) {
    return 0;
  };
  


  /**
     Returns the value of the selected channel for the given dataset as double.
     \param data pointer to the dataset (including headers etc)
     \param ix pixel number in the x direction
     \param iy pixel number in the y direction
     \returns data for the selected channel, with inversion if required as double

  */
  virtual double getValue(char *data, int ix, int iy=0) {
    /* cout << " x "<< ix << " y"<< iy << " val " << getChannel(data, ix, iy)<< endl;*/
    return (double)getChannel(data, ix, iy);
  };

  
  /**
     Returns the frame number for the given dataset. Purely virtual func.
     \param buff pointer to the dataset
     \returns frame number
     
  */
  virtual  int getFrameNumber(char *buff)=0;   
   
  /**
     
     Returns the packet number for the given dataset. purely virtual func
     \param buff pointer to the dataset
     \returns packet number number

     
     virtual int getPacketNumber(char *buff)=0;
     
  */
  
  /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 
     
  */
  virtual  char *findNextFrame(char *data, int &ndata, int dsize)=0;
  
  
   /**
      
     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors! 
     \param filebin input file stream (binary)
     \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete

   */
  virtual char *readNextFrame(ifstream &filebin)=0;
  
};



#endif

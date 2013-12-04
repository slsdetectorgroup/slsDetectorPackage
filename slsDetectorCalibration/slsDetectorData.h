#ifndef SLSDETECTORDATA_H
#define  SLSDETECTORDATA_H

#include <iostream>
#include <fstream>

using namespace std;

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
  slsDetectorData(int npx, int npy=1, int ds=-1, int **dMap=NULL, int **dMask=NULL, int **dROI=NULL): nx(npx), ny(npy) {

    if (ds<=0)
      dataSize=nx*ny;
    else
      dataSize=ds;

    dataMask=new int*[ny];
    for(int i = 0; i < ny; i++) {
      dataMask[i] = new int[nx];
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

  ~slsDetectorData() {
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

  void setDataMask(int **dMask=NULL){

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

  int setBad(int ix, int iy, int i=1) {dataROIMask[iy][ix]=i; return isGood(ix,iy);};

  int isGood(int ix, int iy) {return dataROIMask[iy][ix];};


  /**

     Returns the value of the selected channel for the given dataset (no error checking if number of pixels are compatible!)
     \param data pointer to the dataset (including headers etc)
     \param ix pixel numb er in the x direction
     \param iy pixel number in the y direction

     \returns data for the selected channel, with inversion if required

  */


  inline int getChannel(char *data, int ix, int iy=1) {
    return (*(data+dataMap[iy][ix]))^dataMask[iy][ix];
  };
  
  /**

     Returns the value of the selected channel for the given dataset (no error checking if number of pixels are compatible!)
     \param data pointer to the dataset (including headers etc)
     \param ix pixel numb er in the x direction
     \param iy pixel number in the y direction

     \returns data for the selected channel, with inversion if required

  */

  inline u_int16_t getChannelShort(char  *data, int ix, int iy=1) {
    u_int16_t m=dataMask[iy][ix], d=*(u_int16_t*)(data+dataMap[iy][ix]);
    // cout << ix << " " << iy << " " << dataMap[ix][iy] << endl;
    // return (*(dd+dataMap[ix][iy]))^((u_int16_t)dataMask[ix][iy]);
    return d^m;
  };
  


 private:
  const int nx; /**< Number of pixels in the x direction */
  const int ny; /**< Number of pixels in the y direction */
  int dataSize; /**< Size of a dataset */
  int **dataMap; /**< Array of size nx*ny storing the pointers to the data in the dataset (as offset)*/
  int **dataMask; /**< Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required) */
  int **dataROIMask; /**< Array of size nx*ny 1 if channel is good (or in the ROI), 0 if bad channel (or out of ROI)   */

};



#endif

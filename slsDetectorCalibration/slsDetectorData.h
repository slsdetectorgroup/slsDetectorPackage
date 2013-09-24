#ifndef SLSDETECTORDATA_H
#define  SLSDETECTORDATA_H

class slsDetectorData {

  /**

     Constructor (no error checking if datasize and offsets are compatible!)
     \param npx number of pixels in the x direction
     \param npy number of pixels in the y direction
     \param ds size of the dataset
     \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
     \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)


  */
  slsDetectorData(int npx, int npy=1, int ds=-1, int **dMap=NULL, int **dMask=NULL) {
    nx=npx;
    ny=npy;

    if (ds<=0)
      dataSize=nx*ny;
    else
      dataSize=ds;

    dataMap=new int[nx][ny];
    dataMask=new int[nx][ny];
    
    if (dMap==NULL) {
      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMap[ix][iy]=iy*nx+ix;
    } else {
      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMap[ix][iy]=dMap[ix][iy];
    }
    if (dMap!=NULL) {

      for (int iy=0; iy<ny; iy++)
	 for (int ix=0; ix<nx; ix++)
	   dataMask[ix][iy]=dMask[ix][iy];
    }
    
  };
  /**

     Returns the value of the selected channel for the given dataset (no error checking if number of pixels are compatible!)
     \param data pointer to the dataset (including headers etc)
     \param ix pixel numb er in the x direction
     \param iy pixel number in the y direction

     \returns data for the selected channel, with inversion if required

  */


  int getChannel(int *data, int ix, int iy=1) {
    return *(data+dataMap[ix][iy])^dataMask[ix][iy];
  };
  
  /**

     Returns the value of the selected channel for the given dataset (no error checking if number of pixels are compatible!)
     \param data pointer to the dataset (including headers etc)
     \param ix pixel numb er in the x direction
     \param iy pixel number in the y direction

     \returns data for the selected channel, with inversion if required

  */


  int getChannel(u_int16_t  *data, int ix, int iy=1) {
    return *(data+dataMap[ix][iy])^((u_int16_t)dataMask[ix][iy]);
  };
  


 private:
  int nx; /**< Number of pixels in the x direction */
  int ny; /**< Number of pixels in the y direction */
  int dataSize; /**< Size of a dataset */
  int **dataMap; /**< Array of size nx*ny storing the pointers to the data in the dataset (as offset)*/
  int **dataMask; /**< Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required */

};



#endif

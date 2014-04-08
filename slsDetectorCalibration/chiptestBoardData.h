#ifndef CHIPTESTDATA_H
#define CHIPTESTDATA_H

#include "slsDetectorData.h"

class chiptestBoardData : public slsDetectorData<uint16_t> {


 public:

  /**
  chiptestBoard data structure. Works for data acquired using the chiptestBoard. 
  Inherits and implements slsDetectorData.

  Constructor (no error checking if datasize and offsets are compatible!)
  \param npx number of pixels in the x direction
  \param npy number of pixels in the y direction (1 for strips)
  \param nadc number of adcs
  \param offset offset at the beginning of the pattern
  \param dMap array of size nx*ny storing the pointers to the data in the dataset (as offset)
  \param dMask Array of size nx*ny storing the polarity of the data in the dataset (should be 0 if no inversion is required, 0xffffffff is inversion is required)
  \param dROI Array of size nx*ny. The elements are 1s if the channel is good or in the ROI, 0 is bad or out of the ROI. NULL (default) means all 1s. 

  */
 chiptestBoardData(int npx, int npy, int nadc, int offset, int **dMap=NULL, uint16_t **dMask=NULL, int **dROI=NULL): slsDetectorData<uint16_t>(npx, npy, nadc*(npx*npy)+offset, dMap, dMask, dROI), nAdc(nadc), offSize(offset), iframe(0) {}; // should be? nadc*(npx*npy+offset)

   
 
   /**

     Returns the frame number for the given dataset. Virtual func: works for slsDetectorReceiver data (also for each packet), but can be overloaded. 
     \param buff pointer to the dataset
     \returns frame number

  */

  virtual  int getFrameNumber(char *buff){(void)buff; return iframe;};

  
   /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!
     \param data pointer to the memory to be analyzed
     \param ndata size of frame returned
     \param dsize size of the memory slot to be analyzed
     \returns always return the pointer to data (no frame loss!) 
  */

  virtual  char *findNextFrame(char *data, int &ndata, int dsize) {ndata=dsize;setDataSize(dsize); return data;};

   /**
     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors! 
     \param filebin input file stream (binary)
     \returns pointer to the first packet of the last good frame, NULL if no frame is found or last frame is incomplete
  */

  virtual char *readNextFrame(ifstream &filebin) {
	
 	int afifo_length=0;  
 	uint16_t *afifo_cont; 

    if (filebin.is_open()) {
     if (filebin.read((char*)&afifo_length,sizeof(uint32_t))) {
	setDataSize(afifo_length*nAdc*sizeof(uint16_t));
	afifo_cont=new uint16_t[afifo_length*nAdc];
 	if (filebin.read((char*)afifo_cont,afifo_length*sizeof(uint16_t)*nAdc)) {
		iframe++;
		return (char*)afifo_cont;
	} else {
	delete [] afifo_cont;
	return NULL;
		}
     } else {
	return NULL;
     }  
    }     
    return NULL;
  };

 private:
  const int nAdc; /**<number of ADC read out */
  const int offSize; /**< offset at the beginning of the frame (depends on the pattern) */
  int iframe; /**< frame number (calculated in software! not in the data)*/

};



#endif

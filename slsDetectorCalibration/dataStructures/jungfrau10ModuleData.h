#ifndef JUNGFRAU10MODULEDATA_H
#define  JUNGFRAU10MODULEBDATA_H
#include "slsDetectorData.h"



class jungfrau10ModuleData : public slsDetectorData<uint16_t> {

 private:

     typedef struct {
        uint64_t frameNumber;    /**< is the frame number */
        uint32_t expLength;        /**< is the subframe number (32 bit eiger) or real time exposure time in 100ns (others) */
        uint32_t packetNumber;    /**< is the packet number */
        uint64_t bunchId;        /**< is the bunch id from beamline */
        uint64_t timestamp;        /**< is the time stamp with 10 MHz clock */
        uint16_t modId;            /**< is the unique module id (unique even for left, right, top, bottom) */
        uint16_t xCoord;        /**< is the x coordinate in the complete detector system */
        uint16_t yCoord;        /**< is the y coordinate in the complete detector system */
        uint16_t zCoord;        /**< is the z coordinate in the complete detector system */
        uint32_t debug;            /**< is for debugging purposes */
        uint16_t roundRNumber;    /**< is the round robin set number */
        uint8_t detType;        /**< is the detector type see :: detectorType */
        uint8_t version;        /**< is the version number of this structure format */
    } sls_detector_header;


  int iframe;
  int nadc;
  int sc_width;
  int sc_height;
 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
  

 jungfrau10ModuleData(int ns=16384): slsDetectorData<uint16_t>(256*4, 256*2, 256*256*8*2+48, NULL, NULL, NULL) , iframe(0) {

    
   
    int row, col;

    int isample;
    int iadc;
    int ix, iy;

    int ichip;

    // cout << sizeof(uint16_t) << endl;
    int ip=0;
    for (int iy=0; iy<256*2; iy++) {
      for (int ix=0; ix<256*4; ix++){
	dataMap[iy][ix]=ip*2+48;
	ip++;
      }
    }
      

    /* for (iadc=0; iadc<nadc; iadc++) { */
    /*   ichip=iadc/4; */

    /*   for (int i=0; i<sc_width*sc_height; i++) { */

/* 	if (ichip%2==0) { */
/* 	  row=sc_height+i/sc_width; */
/* 	  col=(ichip/2)*256+iadc%4*sc_width+(i%sc_width); */
/* 	} else {  */
/* 	  row=sc_height-1-i/sc_width;  */
/* 	  col=((ichip/2)*256+iadc%4*sc_width)+sc_width-(i%sc_width)-1; */
/* 	} */
	  

	
/* /\* 	if (iadc<nadc/2) { *\/ */
/* /\* 	  row=sc_height+i/sc_width; *\/ */
/* /\* 	  col=iadc*sc_width+(i%sc_width); *\/ */
/* /\* 	} else { *\/ */
/* /\* 	  row=sc_height-1-i/sc_width; *\/ */
/* /\* 	  col=(nx-1)-((iadc-16)*sc_width)-(i%sc_width); *\/ */
/* /\* 	} *\/ */
/* 	if (row<0 || row>=ny ||  col<0 || col>=nx) { */
/* 	    cout << "Wrong row, column " << row << " " << col << " " << iadc << " " << i << endl; */
/* 	} else */
/* 	  dataMap[row][col]=(nadc*i+iadc)*2; */
      /* 	if (dataMap[row][col]<0 || dataMap[row][col]>=dataSize) */
      /* 	   cout << "Error: pointer " << dataMap[row][col] << " out of range " << row << " " << col  <<" " << iadc << " " << i << endl; */
      /* 	else { */
      /* 	  xmap[nadc*i+iadc]=col; */
      /* 	  ymap[nadc*i+iadc]=row; */

      /* 	} */
      /* } */

    //  }

    
    
  };
  
  

     /**

     Returns the frame number for the given dataset. Purely virtual func.
     \param buff pointer to the dataset
     \returns frame number

  */


  int getFrameNumber(return (sls_detector_header*)buff)->frameNumber;};   

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
   char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;};


   /**

     Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors! 
     \param filebin input file stream (binary)
     \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete

  */
  virtual char *readNextFrame(ifstream &filebin) {
    int ff=-1, np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int &ff) {
    int np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) {
	  char *data=new char[dataSize];
	  char *d=readNextFrame(filebin, ff, np, data);
	  if (d==NULL) {delete [] data; data=NULL;}
	  return data;
  }


  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np, char *data) {
    // char *readNextFrame(ifstream &filebin){
      //	int afifo_length=0;  
    /*   uint16_t *afifo_cont;  */
    /*   int ib=0; */
    /*   if (filebin.is_open()) { */
    /* 	afifo_cont=new uint16_t[dataSize/2]; */
    /* 	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
    /* 	  ib+=2; */
    /* 	  if (ib==dataSize) break; */
    /* 	} */
    /* 	if (ib>0) { */
    /* 	  iframe++; */
    /* 	  // cout << ib << "-" << endl; */
    /* 	  return (char*)afifo_cont; */
    /* 	} else { */
    /* 	  delete [] afifo_cont; */
    /* 	  return NULL; */
    /* 	} */
    /*   }      */
    /*   return NULL; */
    /* }; */

 char *retval=0;
	  int  nd;
	  int fnum = -1;
	  np=0;
	  int  pn;
	  
	  //  cout << dataSize << endl;
	  if (ff>=0)
	    fnum=ff;

	  if (filebin.is_open()) {
	    if (filebin.read(data, dataSize) ){
	      ff=getFrameNumber(data);
	      np=getPacketNumber(data);
	      return data;
	    }
	  }
	    return NULL;
	    
  

};



#endif

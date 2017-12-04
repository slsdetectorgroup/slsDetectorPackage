#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H


#include "detectorData.h"
#include "fileIO.h"
#include <string>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>


class angularConversion;

using namespace std;

#define MAX_BADCHANS 2000


#define defaultTDead {170,90,750} /**< should be changed in order to have it separate for the different detector types */


/**
   @short methods for data postprocessing 

   (including thread for writing data files and plotting in parallel with the acquisition) 
*/

class postProcessing : public virtual fileIO  {


//: public angularConversion, public fileIO

 public:
  postProcessing();
  virtual ~postProcessing(){};




  /**
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  virtual int getBadChannelCorrection(int *bad=NULL)=0;


  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int getFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL)=0;
  
  /** 
      set flat field corrections
      \param corr if !=NULL the flat field corrections will be filled with corr (NULL usets ff corrections)
      \param ecorr if !=NULL the flat field correction errors will be filled with ecorr (1 otherwise)
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int  setFlatFieldCorrection(float *corr, float *ecorr=NULL)=0;
  
  /**
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(string fname="")=0;
  
  static int setBadChannelCorrection(ifstream &infile, int &nbad, int *badlist, int moff=0);
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)

ff
      \param nbad reference to number of bad channels
      \param badlist array of badchannels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(string fname, int &nbad, int *badlist, int off=0)=0;

  
  /** 
      set bad channels correction
      \param nch number of bad channels
      \param chs array of channels
      \param ff 0 if normal bad channels, 1 if ff bad channels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(int nch, int *chs, int ff=0)=0; 
  
  /** 
     flat field correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param ffcoefficient flat field correction coefficient
     \param fferr erro on ffcoefficient
     \returns 0
  */
   static int flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr);

  /** 
     rate correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param tau dead time 9in ns)
     \param t acquisition time (in ns)
     \returns 0
  */
   static int rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t);
 

  int enableWriteToFile(int i=-1) {if (i>0) ((*correctionMask)|=(1<<WRITE_FILE)); if(i==0)  ((*correctionMask)&=~(1<< WRITE_FILE)); return ((*correctionMask)&(1<< WRITE_FILE));};


  int setAngularCorrectionMask(int i=-1){if (i==0) (*correctionMask)&=~(1<< ANGULAR_CONVERSION); if (i>0) (*correctionMask)|=(1<< ANGULAR_CONVERSION); return ((*correctionMask)&(1<< ANGULAR_CONVERSION));};



  int enableAngularConversion(int i=-1) {if (i>0) return setAngularConversionFile("default"); if (i==0) return setAngularConversionFile(""); return setAngularCorrectionMask();};


  int enableBadChannelCorrection(int i=-1) {if (i>0) return setBadChannelCorrection("default"); if (i==0) return setBadChannelCorrection(""); return ((*correctionMask)&(1<< DISCARD_BAD_CHANNELS));};


  
  
  /** returns the bad channel list file */
  string getBadChannelCorrectionFile() {if ((*correctionMask)&(1<< DISCARD_BAD_CHANNELS)) return string(badChanFile); else return string("none");};


  /** 
      get flat field corrections file directory
      \returns flat field correction file directory
  */
  string getFlatFieldCorrectionDir(){return string(flatFieldDir);};
 /** 
      set flat field corrections file directory
      \param flat field correction file directory
      \returns flat field correction file directory
  */
  string setFlatFieldCorrectionDir(string dir){strcpy(flatFieldDir,dir.c_str()); return string(flatFieldDir);};
  
 /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  string getFlatFieldCorrectionFile(){  if ((*correctionMask)&(1<<FLAT_FIELD_CORRECTION)) return string(flatFieldFile); else return string("none");};



  /** 
      set/get if the data processing and file writing should be done by a separate thread
s
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */

  int setThreadedProcessing(int b=-1) {if (b>=0) *threadedProcessing=b; return  *threadedProcessing;};




  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void *processData(int delflag);

  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void processFrame(int* myData, int delflag);

  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void doProcessing(float* myData, int delflag, string fname);


  /**
   pops the data from the data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  dataQueue
  */ 
  int* popDataQueue();

  /**
   pops the data from thepostprocessed data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  finalDataQueue
  */ 
  detectorData* popFinalDataQueue();


  /**
  resets the raw data queue
    \sa  dataQueue
  */ 
  void resetDataQueue();

  /**
  resets the postprocessed  data queue
    \sa  finalDataQueue
  */ 
  void resetFinalDataQueue();





   int fillBadChannelMask();
    



  virtual int rateCorrect(float*, float*, float*, float*)=0;
  virtual int flatFieldCorrect(float*, float*, float*, float*)=0;







  void registerDataCallback(int( *userCallback)(detectorData*, void*),  void *pArg) {dataReady = userCallback; pCallbackArg = pArg;};
  //void registerCallBackGetChansPerMod(int (*func)(int, void *),void *arg){ getChansPerMod=func;pChpermod=arg;}



  /**
     sets  the angular conversion file
     \param fname file to read
     \returns angular conversion flag
  */

  int setAngularConversionFile(string fname);

  
  /**
      returns the angular conversion file
     */
  string getAngularConversionFile(){if (setAngularCorrectionMask()) return string(angConvFile); else return string("none");};



 protected:
     
  int *threadedProcessing;

  int *correctionMask;

  char *flatFieldDir;
  char *flatFieldFile;

  char *badChanFile;
  int *nBadChans;
  int *badChansList;
  int *nBadFF;
  int *badFFList;

  /** pointer to angular conversion file name*/
   char *angConvFile;
 

  /** mutex to synchronize main and data processing threads */
  pthread_mutex_t mp;


 /** mutex to synchronizedata processing and plotting threads */
   pthread_mutex_t mg;

 /** sets when the acquisition is finished */
  int jointhread;

 /** sets when the position is finished */
  int posfinished;  

 /**
     data queue
  */
  queue<int*> dataQueue;
  /**
     queue containing the postprocessed data
  */
  queue<detectorData*> finalDataQueue;
  

 /** data queue size */
  int queuesize;


  

  /**
    start data processing thread
  */
  void startThread(int delflag=1); //
  /** the data processing thread */

  pthread_t dataProcessingThread;



  /** pointer to bad channel mask  0 is channel is good 1 if it is bad \sa fillBadChannelMask() */ 
  int *badChannelMask;




  /**
     I0 measured
  */
  float currentI0;
  
  float *fdata;


  //int (*getChansPerMod)(int, void*);
	
  
  int (*dataReady)(detectorData*,void*); 
  void *pCallbackArg, *pChpermod; 



 private:
  angularConversion *angConv;
  

};


static void* startProcessData(void *n){\
   postProcessing *myDet=(postProcessing*)n;\
   myDet->processData(1);\
   pthread_exit(NULL);\
   
};

static void* startProcessDataNoDelete(void *n){\
  postProcessing *myDet=(postProcessing*)n;\
  myDet->processData(0);\
  pthread_exit(NULL);\

};



#endif

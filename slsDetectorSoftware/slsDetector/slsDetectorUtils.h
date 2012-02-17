

#ifndef SLS_DETECTOR_UTILS_H
#define SLS_DETECTOR_UTILS_H

//#include "MySocketTCP.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
 #include <queue>
extern "C" {
 #include <pthread.h>
}
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <sys/uio.h>
#include <math.h>
using namespace std;


#include "sls_detector_defs.h"
#include "slsDetectorCommand.h"

#define MAX_TIMERS 10
#define MAX_ROIS 100
#define MAX_BADCHANS 2000
#define MAXPOS 50
#define MAX_SCAN_LEVELS 2

#define NMODMAXX 24
#define NMODMAXY 24
#define MAXMODS 36
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16

#define DEFAULT_HOSTNAME  "localhost"
#define DEFAULT_SHM_KEY  5678

#define defaultTDead {170,90,750} /**< should be changed in order to have it separate for the different detector types */



/**
   data structure to hold the detector data after postprocessing (e.g. to plot, store in a root tree etc.)
 */
class detectorData {
 public:
  /** The constructor
      \param val pointer to the data
      \param err pointer to errors
      \param ang pointer to the angles
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points defaults to the number of detector channels
  */
  detectorData(float *val=NULL, float *err=NULL, float *ang=NULL,  float p_ind=-1, const char *fname="", int np=-1) : values(val), errors(err), angles(ang),  progressIndex(p_ind), npoints(np){strcpy(fileName,fname);};
    /** 
	the destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
    */
    ~detectorData() {if (values) delete [] values; if (errors) delete [] errors; if (angles) delete [] angles;};
    //private:
    float *values; /**< pointer to the data */
    float *errors; /**< pointer to the errors */
    float *angles;/**< pointer to the angles */
    float progressIndex;/**< file index */
    char fileName[1000];/**< file name */
    int npoints;/**< number of points */
};





class slsDetectorUtils : public slsDetectorCommand {



 public:
  
  slsDetectorUtils();
    
    virtual ~slsDetectorUtils(){};
















  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index
      \param filepath outdir
      \param filename file root name
      \param aMask action mask (scans, positions)
      \param sv0 scan variable 0
      \param prec0 scan precision 0
      \param sv1 scan variable 1
      \param prec1 scan precision 1
      \param pindex position index
      \param number of positions
      \param findex file index
      \returns file name without extension
  */
  static string  createFileName(char *filepath, char *filename, int aMask, float sv0, int prec0, float sv1, int prec1, int pindex, int npos, int findex);

  virtual string createFileName();


    /** static function that returns the file index from the file name 
      \param fname file name
      \returns file index*/
   static int getFileIndexFromFileName(string fname);

  /** static function that returns the variables from the file name 
      \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
  */
   static int getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1);
  




    /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
 
  */
  int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
   
  int writeDataFile(ofstream &outfile, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
  

  /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  int writeDataFile(string fname, int *data);
  
  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
       \sa mythenDetector::readDataFile
  */
  int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f');  

  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
       \sa mythenDetector::readDataFile
  */
  int readDataFile(string fname, int *data);

    /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
 
  */

   static  int  writeDataFile(string fname, int nch, float *data, float *err=NULL, float *ang=NULL, char dataformat='f'); 
  
  /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  static int writeDataFile(string fname,int nch,  int *data);
   
   /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns number of channels read or -1 if it could not read the file or data=NULL
       
       \sa mythenDetector::readDataFile
  */
   static int readDataFile(int nch, string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f');  

  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
       \sa mythenDetector::readDataFile
  */
   static int readDataFile(string fname, int *data, int nch);
  /**
   
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
   static int readAngularConversion(string fname, int nmod, angleConversionConstant *angOff);  

   /**
   
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
   static int readAngularConversion(ifstream& ifs, int nmod, angleConversionConstant *angOff);
  /**
     Pure virtual function
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
   static int writeAngularConversion(string fname, int nmod, angleConversionConstant *angOff);
 /**
     Pure virtual function
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
   static int writeAngularConversion(ofstream& ofs, int nmod, angleConversionConstant *angOff);
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \param nbad reference to number of bad channels
      \param badlist array of badchannels
      \returns 0 if bad channel disabled, >0 otherwise
  */
   static int setBadChannelCorrection(string fname, int &nbad, int *badlist);
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
    /** 
      pure virtual function
  sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize(); 
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns OK or FAIL
      \sa mythenDetector::resetMerging
  */
   static int resetMerging(float *mp, float *mv,float *me, int *mm, float binsize);
   int resetMerging(float *mp, float *mv,float *me, int *mm);

  /** 
      pure virtual function
  merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \sa mythenDetector::addToMerging
  */

   static int  addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm, int nchans, float binsize,int angDirection, int correctionMask, int *badChanMask );
   
   int  addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm);

  /** pure virtual function
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern)
      \sa mythenDetector::finalizeMerging
  */

   static int finalizeMerging(float *mp, float *mv,float *me, int *mm, float binsize);

   int finalizeMerging(float *mp, float *mv,float *me, int *mm);





  /**
   
      reads a calibration file
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  \sa  sharedSlsDetector mythenDetector::readCalibrationFile
  */
  static int readCalibrationFile(string fname, float &gain, float &offset);
  //virtual int readCalibrationFile(string fname, float &gain, float &offset);
  
  /**
   
      writes a calibration file
      \param fname file to be written
      \param gain 
      \param offset
  \sa  sharedSlsDetector mythenDetector::writeCalibrationFile
  */
  static int writeCalibrationFile(string fname, float gain, float offset);
  //virtual int writeCalibrationFile(string fname, float gain, float offset);




  /**
     sets the default output files path
  \sa  sharedSlsDetector
  */
  char* setFilePath(string s) {sprintf(filePath, s.c_str()); return filePath;};

  /**
     sets the default output files root name
  \sa  sharedSlsDetector
  */
  char* setFileName(string s) {sprintf(fileName, s.c_str()); return fileName;}; 

  /**
     sets the default output file index
  \sa  sharedSlsDetector
  */
  int setFileIndex(int i) {*fileIndex=i; return *fileIndex;}; 
  
  /**
     returns the default output files path
  \sa  sharedSlsDetector
  */
  char* getFilePath() {return filePath;};
  
  /**
     returns the default output files root name
  \sa  sharedSlsDetector
  */
  char* getFileName() {return fileName;};

  /**
     returns the default output file index
  \sa  sharedSlsDetector
  */
  int getFileIndex() {return *fileIndex;};
  



  /** 
      pure virtual function
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
      \sa mythenDetector::setPositions
  */
  int setPositions(int nPos, float *pos);
   /** 
      pure virtual function
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
      \sa mythenDetector::getPositions
  */
  int getPositions(float *pos=NULL);
  
 



  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \param par for script 
      \returns 0 if action disabled, >0 otherwise
  */
  int setAction(int iaction, string fname="", string par="");

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionScript(int iaction, string fname="");
  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param par for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionParameter(int iaction, string par="");

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
  string getActionScript(int iaction);

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
  string getActionParameter(int iaction);

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
  int getActionMode(int iaction);


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disables, "none" disables and overwrites current)
      \param nvalues number of steps (0 disables, -1 leaves current value)
      \param values pointer to steps (if NULL leaves current values)
      \param par parameter for the scan script ("" leaves unchanged)
      \returns 0 is scan disabled, >0 otherwise
  */
  int setScan(int index, string script="", int nvalues=-1, float *values=NULL, string par="", int precision=-1);
  
  int setScanScript(int index, string script="");
  int setScanParameter(int index, string par="");
  int setScanPrecision(int index, int precision=-1);
  int setScanSteps(int index, int nvalues=-1, float *values=NULL);
  float getScanStep(int index, int istep){if (index<MAX_SCAN_LEVELS && istep<MAX_SCAN_STEPS) return scanSteps[index][istep]; else return -1;};
  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
  string getScanScript(int iscan);

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
  string getScanParameter(int iscan);

   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
  int getScanMode(int iscan);

   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
  int getScanSteps(int iscan, float *v=NULL);


   /** 
	returns scan precision
	\param iscan can be (0,1)
	\returns scan precision
    */
  int getScanPrecision(int iscan);





  /** 
      set/get if the data processing and file writing should be done by a separate thread
s
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */

  int setThreadedProcessing(int b=-1) {if (b>=0) *threadedProcessing=b; return  *threadedProcessing;}



  /** 
      pure virtual function
      set detector global offset
      \sa mythenDetector::setGlobalOffset
  */
  float setGlobalOffset(float f){*globalOffset=f; return *globalOffset;}; 

  /** 
      pure virtual function
      set detector fine offset
      \sa mythenDetector::setFineOffset
  */
  float setFineOffset(float f){*fineOffset=f; return *fineOffset;};
  /** 
      pure virtual function
      get detector fine offset
      \sa mythenDetector::getFineOffset
  */
  float getFineOffset(){return *fineOffset;};
  
  /** 
      pure virtual function
      get detector global offset
      \sa mythenDetector::getGlobalOffset
  */
  float getGlobalOffset(){return *globalOffset;};


 
  /** pure virtual function
      set detector bin size used for merging (approx angular resolution)
      \param bs bin size in degrees
      \returns current bin size
      \sa mythenDetector::setBinSize
*/
  float setBinSize(float bs){*binSize=bs; return *binSize;};

  /** pure virtual function
      return detector bin size used for merging (approx angular resolution)
      \sa mythenDetector::getBinSize
  */
  float getBinSize() {return *binSize;};




  
  /**
      pure virtual function
      returns the angular conversion file
      \sa mythenDetector::getAngularConversion */
  string getAngularConversion(){if ((*correctionMask)&(1<< ANGULAR_CONVERSION)) return string(angConvFile); else return string("none");};
  

  /** returns the bad channel list file */
  string getBadChannelCorrectionFile() {if ((*correctionMask)&(1<< DISCARD_BAD_CHANNELS)) return string(badChanFile); else return string("none");};




 /** 
      get flat field corrections file directory
      \returns flat field correction file directory
  */
  char *getFlatFieldCorrectionDir(){return flatFieldDir;};
 /** 
      set flat field corrections file directory
      \param flat field correction file directory
  */
  void setFlatFieldCorrectionDir(string dir){strcpy(flatFieldDir,dir.c_str());};
  
 /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  char *getFlatFieldCorrectionFile(){  if ((*correctionMask)&(1<<FLAT_FIELD_CORRECTION)) return flatFieldFile; else return "none";};












  void acquire(int delflag);

  // must change to total number of channels!
  void *processData(int delflag);

  virtual float* convertAngles(float pos)=0;
  virtual int setThresholdEnergy(int, int im=-1, detectorSettings isettings=GET_SETTINGS)=0;
  virtual float setDAC(float, dacIndex, int im=-1)=0;
  virtual int setChannel(long long, int ich=-1, int ichip=-1, int imod=-1)=0;

  virtual float getRateCorrectionTau()=0;
  virtual int* startAndReadAll()=0;
  virtual float* decodeData(int *datain)=0;
  virtual int rateCorrect(float*, float*, float*, float*)=0;
  virtual int flatFieldCorrect(float*, float*, float*, float*)=0;

  virtual int getTotalNumberOfChannels()=0;
  


  //  virtual int getParameters();
  
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

/*   virtual string getScanScript(int iscan)=0; */
/*   virtual string getScanParameter(int iscan)=0; */
/*   virtual string getActionScript(int iscan)=0; */
/*   virtual string getActionParameter(int iscan)=0; */
/*   virtual float getScanStep(int iscan, int istep)=0; */


  int setTotalProgress();
  float getCurrentProgress();

 protected:
   static const int64_t thisSoftwareVersion=0x20120124;


   int fillBadChannelMask();
    
   int getPointers(int * const l_stoppedFlag,				\
		   int * const l_threadedProcessing,			\
		   int * const l_actionMask,				\
		   mystring * const l_actionScript,			\
		   mystring * const l_actionParameter,			\
		   int * const l_nScanSteps,				\
		   int * const l_scanMode,				\
		   mystring * const l_scanScript,			\
		   mystring * const l_scanParameter,			\
		   mysteps * const l_scanSteps,				\
		   int * const l_scanPrecision,				\
		   int * const l_numberOfPositions,			\
		   float * const l_detPositions,			\
		   char * const l_angConvFile,				\
		   int * const l_correctionMask,			\
		   float * const l_binSize,				\
		   float * const l_fineOffset,				\
		   float * const l_globalOffset,			\
		   int * const l_angDirection,				\
		   char * const l_flatFieldDir,				\
		   char * const l_flatFieldFile,			\
		   char * const l_badChanFile,				\
		   int64_t * const l_timerValue,			\
		   detectorSettings * const l_currentSettings,		\
		   int * const l_currentThresholdEV,			\
		   char * const l_filePath,				\
		   char * const l_fileName,				\
		   int * const l_fileIndex);





 /**
     data queue
  */
  queue<int*> dataQueue;
  /**
     queue containing the postprocessed data
  */
  queue<detectorData*> finalDataQueue;
  
  

 private:



  int totalProgress;
	      		  
  int progressIndex;	  
  int *stoppedFlag;	    
  int *threadedProcessing;


  int *actionMask;  	   
  mystring *actionScript;	      
  mystring *actionParameter; 	      

  int *nScanSteps;		      
  mysteps *scanSteps;	 	    
  int *scanMode;		    
  int *scanPrecision;
  mystring *scanScript; 
  mystring *scanParameter; 

  int *numberOfPositions;
  float *detPositions;


  char *angConvFile;
  int *correctionMask;
  float *binSize;
  float *fineOffset;
  float *globalOffset;
  int *angDirection;

  char *flatFieldDir;
  char *flatFieldFile;

  char *badChanFile;
  int *nBadChans;
  int *badChansList;
  int *nBadFF;
  int *badFFList;

  int64_t *timerValue;
  detectorSettings *currentSettings;
  int *currentThresholdEV;

  char *filePath;
  char *fileName;
  int *fileIndex;


 /** mutex to synchronize threads */
   pthread_mutex_t mp;


 /** sets when the acquisition is finished */
  int jointhread;

 /** data queue size */
  int queuesize;




  /**
     current position of the detector
  */
  float currentPosition;
  
  /**
     current position index of the detector
  */
  int currentPositionIndex;
  
  /**
     I0 measured
  */
  float currentI0;
  
  


  /**
     current scan variable of the detector
  */
  float currentScanVariable[MAX_SCAN_LEVELS];
  
  /**
     current scan variable index of the detector
  */
  int currentScanIndex[MAX_SCAN_LEVELS];
  
  

  /** merging bins */
  float *mergingBins;

  /** merging counts */
  float *mergingCounts;

  /** merging errors */
  float *mergingErrors;

  /** merging multiplicity */
  int *mergingMultiplicity;
  



  /** pointer to bad channel mask  0 is channel is good 1 if it is bad \sa fillBadChannelMask() */ 
  int *badChannelMask;


  /**
    start data processing thread
  */
  void startThread(int delflag=1); //
  /** the data processing thread */

  pthread_t dataProcessingThread;

  /** 
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  virtual int getBadChannelCorrection(int *bad=NULL)=0;


  
};


static void* startProcessData(void *n){\
   slsDetectorUtils *myDet=(slsDetectorUtils*)n;\
   myDet->processData(1);\
   pthread_exit(NULL);\
   
};

static void* startProcessDataNoDelete(void *n){\
  slsDetectorUtils *myDet=(slsDetectorUtils*)n;\
  myDet->processData(0);\
  pthread_exit(NULL);\

};



#endif

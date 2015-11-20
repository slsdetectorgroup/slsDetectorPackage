

#ifndef SLS_DETECTOR_UTILS_H
#define SLS_DETECTOR_UTILS_H


#ifdef __CINT__
class pthread_mutex_t;
class pthread_t;
#endif


extern "C" {
#include <pthread.h>
}

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

using namespace std;


//#include "slsDetectorActions_Standalone.h"
#include "slsDetectorActions.h"
#include "postProcessing.h"

//#define MAX_TIMERS 11
#define MAXPOS 50

#define DEFAULT_HOSTNAME  "localhost"
#define DEFAULT_SHM_KEY  5678
#define THIS_REVISION "$Rev: 822 $"

//test

/**
   @short class containing all the possible detector functionalities 

   (used in the PSi command line interface)
*/


class slsDetectorUtils :  public slsDetectorActions, public postProcessing {


 public:
  
  slsDetectorUtils();
    
  virtual ~slsDetectorUtils(){};


  virtual int getNumberOfDetectors(){return 1; };
  

  virtual int getMaxNumberOfChannelsPerDetector(dimension d){return -1;};

  virtual int setMaxNumberOfChannelsPerDetector(dimension d,int i){return -1;};

  //int setPositions(int nPos, double *pos){return angularConversion::setPositions(nPos, pos);};

  // int getPositions(double *pos=NULL){return angularConversion::getPositions(pos);};
  
  using slsDetectorBase::setFlatFieldCorrection;
  using slsDetectorBase::getDetectorsType;
  using postProcessing::setBadChannelCorrection;

  int enableFlatFieldCorrection(int i=-1) {if (i>0) setFlatFieldCorrectionFile("default"); else if (i==0) setFlatFieldCorrectionFile(""); return getFlatFieldCorrection();};
  int enablePixelMaskCorrection(int i=-1) {if (i>0) setBadChannelCorrection("default"); else if (i==0) setBadChannelCorrection(""); return getBadChannelCorrection();};
  int enableCountRateCorrection(int i=-1){if (i>0) setRateCorrection(-1); else if (i==0) setRateCorrection(0); return getRateCorrection();};
  // string getFilePath(){return fileIO::getFilePath();};;
  // string setFilePath(string s){return fileIO::setFilePath(s);};

  // string getFileName(){return fileIO::getFileName();};
  // string setFileName(string s){return fileIO::setFileName(s);};

  // int getFileIndex(){return fileIO::getFileIndex();};
  // int setFileIndex(int s){return fileIO::setFileIndex(s);};

  /*
    int getScanPrecision(int i){return slsDetectorActions::getScanPrecision(i);};

    int getActionMask() {return slsDetectorActions::getActionMask();};
    float getCurrentScanVariable(int i) {return slsDetectorActions::getCurrentScanVariable(i);};
    int getCurrentPositionIndex(){return angularConversion::getCurrentPositionIndex();}; 
    int getNumberOfPositions(){return angularConversion::getNumberOfPositions();};
  */

  // int getActionMask() {return slsDetectorActions::getActionMask();};
  // double getCurrentScanVariable(int i) {return slsDetectorActions::getCurrentScanVariable(i);};
  // int getCurrentPositionIndex(){return angularConversion::getCurrentPositionIndex();}; 
  // int getNumberOfPositions(){return angularConversion::getNumberOfPositions();};


  // string getFlatFieldCorrectionDir(){return postProcessing::getFlatFieldCorrectionDir();};
  // string setFlatFieldCorrectionDir(string s){return postProcessing::setFlatFieldCorrectionDir(s);};
  // string getFlatFieldCorrectionFile(){return postProcessing::getFlatFieldCorrectionFile();};
  // int enableBadChannelCorrection(int i){return postProcessing::enableBadChannelCorrection(i);};
  // int enableAngularConversion(int i){return postProcessing::enableAngularConversion(i);};
  

  /** returns the detector hostname 
      \param pos position in the multi detector structure (is -1 returns concatenated hostnames divided by a +)
      \returns hostname
  */
  virtual string getHostname(int pos=-1)=0;

  
  /** sets the detector hostname   
      \param name hostname
      \param pos position in the multi detector structure (is -1 expects concatenated hostnames divided by a +)
      \returns  hostname  
  */
  virtual string setHostname(const char* name, int pos=-1)=0;


  /** returns the detector type
      \param pos position in the multi detector structure (is -1 returns type of detector with id -1)
      \returns type
  */
  virtual string sgetDetectorsType(int pos=-1)=0;

  /** returns the detector type
      \param pos position in the multi detector structure (is -1 returns type of detector with id -1)
      \returns type
  */
  virtual detectorType setDetectorsType(detectorType t=GET_DETECTOR_TYPE, int pos=-1)=0;
  virtual string ssetDetectorsType(detectorType t=GET_DETECTOR_TYPE, int pos=-1)=0;
  virtual string ssetDetectorsType(string s, int pos=-1)=0;


  
  /** Gets the detector id (shared memory id) of an slsDetector
      \param i position in the multiSlsDetector structure
      \return id or -1 if FAIL
  */
  virtual int getDetectorId(int i=-1)=0;

  /** Sets the detector id (shared memory id) of an slsDetector in a multiSlsDetector structure
      \param ival id to be set
      \param i position in the multiSlsDetector structure
      \return id or -1 if FAIL  (e.g. in case of an slsDetector)
  */
  virtual int setDetectorId(int ival, int i=-1){return -1;};



  /**
     gets the network parameters (implemented for gotthard)
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \returns parameter

  */
  virtual char *getNetworkParameter(networkParameter i)=0;

  /**
     sets the network parameters  (implemented for gotthard)
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \param s value to be set
     \returns parameter

  */
  virtual char *setNetworkParameter(networkParameter i, string s)=0; 

  /**
     changes/gets the port number
     \param t type port type can be CONTROL_PORT, DATA_PORT, STOP_PORT
     \param i new port number (<1024 gets)
     \returns actual port number
  */
  virtual int setPort(portType t, int i=-1)=0; 

  /**
     checks if the detector(s) are online/offline
     \returns hostname if offline
  */
  virtual string checkOnline()=0;

  /**
     Digital test of the modules
     \param mode test mode
     \param imod module number for chip test or module firmware test
     \returns OK or error mask
  */
  virtual int digitalTest(digitalTestMode mode, int imod=0)=0;

  /**
     execute trimming
     \param mode trim mode
     \param par1 if noise, beam or fixed setting trimming it is count limit, if improve maximum number of iterations
     \param par2 if noise or beam nsigma, if improve par2!=means vthreshold will be optimized, if fixed settings par2<0 trimwith median, par2>=0 trim with level
     \param imod module number (-1 all)
     \returns OK or FAIl (FAIL also if some channel are 0 or 63
  */
  virtual int executeTrimming(trimMode mode, int par1, int par2, int imod=-1)=0;


  /**
     returns currently the loaded trimfile/settingsfile name
  */
  virtual const char *getSettingsFile()=0;

  
  /** 
      get current timer value
      \param index timer index
      \returns elapsed time value in ns or number of...(e.g. frames, gates, probes)
  */
  virtual int64_t getTimeLeft(timerIndex index)=0;



  /** sets the number of trim energies and their value  \sa sharedSlsDetector 
      \param nen number of energies
      \param en array of energies
      \returns number of trim energies

      unused!

  */
  virtual int setTrimEn(int nen, int *en=NULL)=0;

  /** returns the number of trim energies and their value  \sa sharedSlsDetector 
      \param en pointer to the array that will contain the trim energies (in ev)
      \returns number of trim energies

      unused!
  */
  virtual int getTrimEn(int *en=NULL)=0;



  /** 
      set/get the use of an external signal 
      \param pol meaning of the signal \sa externalSignalFlag
      \param signalindex index of the signal
      \returns current meaning of signal signalIndex
  */
  virtual externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0)=0;




  /** sets/gets the value of important readout speed parameters
      \param sp is the parameter to be set/get
      \param value is the value to be set, if -1 get value
      \returns current value for the specified parameter
      \sa speedVariable
  */
  virtual int setSpeed(speedVariable sp, int value=-1)=0;






  /**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
  */
  virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;





  int setBadChannelCorrection(string fname, int &nbadtot, int *badchanlist, int off=0);





  /** sets/gets position of the master in a multi detector structure
      \param i position of the detector in the multidetector structure
      \returns position of the master in a multi detector structure (-1 no master or always in slsDetector)
  */
  virtual int setMaster(int i=-1){return -1;};

  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode
      \returns current syncronization mode   
  */   
  virtual synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE)=0;


  /** 
      returns the detector trimbit/settings directory   
  */
  virtual char* getSettingsDir()=0;

  /** sets the detector trimbit/settings directory  */
  virtual char* setSettingsDir(string s)=0;

  /**
     returns the location of the calibration files
  */
  virtual char* getCalDir()=0; 

  /**
     sets the location of the calibration files
  */
  virtual char* setCalDir(string s)=0;

  /** Frees the shared memory  -  should not be used except for debugging*/
  virtual int freeSharedMemory()=0;


  /** adds the detector with ID id in postion pos
      \param id of the detector to be added (should already exist!)
      \param pos position where it should be added (normally at the end of the list (default to -1)
      \returns the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int addSlsDetector(int id, int pos=-1){return -1;};


  /** adds the detector name in position pos
      \param name of the detector to be added (should already exist in shared memory or at least be online) 
      \param pos position where it should be added (normally at the end of the list (default to -1)
      \return the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int addSlsDetector(char* name, int pos=-1){return -1;};


  /**
     removes the detector in position pos from the multidetector
     \param pos position of the detector to be removed from the multidetector system (defaults to -1 i.e. last detector)
     \returns the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int removeSlsDetector(int pos=-1){return -1;};

  /**removes the detector in position pos from the multidetector
     \param name is the name of the detector
     \returns the actual number of detectors or -1 if it failed  (always for slsDetector)
  */
  virtual int removeSlsDetector(char* name){return -1;};

  /** 
      Turns off the server -  do not use except for debugging!
  */   
  virtual int exitServer()=0;




  /**
     Loads dark image or gain image to the detector
     \param index can be DARK_IMAGE or GAIN_IMAGE
     \fname file name to load data from
     \returns OK or FAIL
  */
  virtual int loadImageToDetector(imageType index,string const fname)=0;
  

  /**
     writes the counter memory block from the detector
     \param startACQ is 1 to start acquisition after reading counter
     \fname file fname to load data from
     \returns OK or FAIL
  */
  virtual int writeCounterBlockFile(string const fname,int startACQ=0)=0;


  /**
     Resets counter memory block in detector
     \param startACQ is 1 to start acquisition after resetting counter
     \returns OK or FAIL
  */
  virtual int resetCounterBlock(int startACQ=0)=0;

  /** set/get counter bit in detector
   * @param i is -1 to get, 0 to reset and any other value to set the counter bit
     /returns the counter bit in detector
   */
  virtual int setCounterBit(int i = -1)=0;


  /**
     asks and  receives all data  from the detector  and puts them in a data queue
     \returns pointer to the front of the queue  or NULL.
  */
  virtual int* readAll()=0;




  /** performs a complete acquisition including scansand data processing 
      moves the detector to next position <br>
      starts and reads the detector <br>
      reads the IC (if required) <br>
      reads the encoder (iof required for angualr conversion) <br>
      processes the data (flat field, rate, angular conversion and merging ::processData())
      \param delflag 0 leaves the data in the final data queue
      \returns OK or FAIL depending on if it already started
  */

  int acquire(int delflag=1);


  //  double* convertAngles(){return convertAngles(currentPosition);};
  // virtual double* convertAngles(double pos)=0;

  virtual int setThresholdEnergy(int, int im=-1, detectorSettings isettings=GET_SETTINGS)=0;
  virtual int setChannel(int64_t, int ich=-1, int ichip=-1, int imod=-1)=0;

  virtual double getRateCorrectionTau()=0;
  virtual int* startAndReadAll()=0;

  virtual int getTotalNumberOfChannels()=0;
  virtual int getMaxNumberOfChannels()=0;
  virtual int getMaxNumberOfChannels(dimension d)=0;

  //  virtual int getParameters();
  



  int setTotalProgress();
  
  double getCurrentProgress();


  void incrementProgress();
  void setCurrentProgress(int i=0);


  /** 
      write  register 
      \param addr address
      \param val value
      \returns current register value
      
      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual int writeRegister(int addr, int val)=0; 


  /** 
      write  ADC register 
      \param addr address
      \param val value
      \returns current register value
      
      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual int writeAdcRegister(int addr, int val)=0; 

 
  /** 
      read  register 
      \param addr address
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual int readRegister(int addr)=0;
  /**
     Returns the IP of the last client connecting to the detector
  */
  virtual string getLastClientIP()=0;



  /**
     configures mac for gotthard readout
     \returns OK or FAIL
  */

  virtual int configureMAC()=0;


  /** loads the modules settings/trimbits reading from a file
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  virtual int loadSettingsFile(string fname, int imod=-1)=0;



  /** saves the modules settings/trimbits writing to  a file
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  virtual int saveSettingsFile(string fname, int imod=-1)=0;

  /** sets all the trimbits to a particular value
      \param val trimbit value
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  virtual int setAllTrimbits(int val, int imod=-1)=0;




  /**
     set dacs value
     \param val value (in V)
     \param index DAC index
     \param mV 0 in dac units or 1 in mV
     \param imod module number (if -1 alla modules)
     \returns current DAC value
  */
  virtual dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1)=0;


  /**
     gets ADC value
     \param index ADC index
     \param imod module number
     \returns current ADC value
  */
  virtual dacs_t getADC(dacIndex index, int imod=0)=0;

  /**
     get the maximum size of the detector
     \param d dimension
     \returns maximum number of modules that can be installed in direction d
  */
  virtual int getMaxNumberOfModules(dimension d=X)=0;
 
  double moveDetector(double pos){if (go_to_position) go_to_position (pos,GTarg); else cout << "no move detector callback registered" << endl; return getDetectorPosition();};
  double getDetectorPosition(){double pos=-1; if (get_position) pos=get_position(POarg); else cout << "no get position callback registered" << endl; return pos;};

  /**
     Writes the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels etc.)
     \param fname file name
     \returns OK or FAIL
  */
  virtual int writeConfigurationFile(string const fname)=0;


  void registerGetPositionCallback( double (*func)(void*),void *arg){get_position=func; POarg=arg;};
  void registerConnectChannelsCallback( int (*func)(void*),void *arg){connect_channels=func; CCarg=arg;};
  void registerDisconnectChannelsCallback(int (*func)(void*),void*arg){disconnect_channels=func;DCarg=arg;};
  
  void registerGoToPositionCallback( int (*func)(double, void*),void *arg){go_to_position=func;GTarg=arg;};
  void registerGoToPositionNoWaitCallback(int (*func)(double, void*),void*arg){go_to_position_no_wait=func;GTNarg=arg;};
  void registerGetI0Callback( double (*func)(int, void*),void *arg){get_i0=func;IOarg=arg;};
  
  void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg){acquisition_finished=func; acqFinished_p=pArg;};
  void registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg){measurement_finished=func; measFinished_p=pArg;};

  void registerProgressCallback(int( *func)(double,void*), void *pArg){progress_call=func; pProgressCallArg=pArg;};


  /** 
      Saves the detector setup to file
      \param fname file to write to
      \param level if 2 reads also trimbits, flat field, angular correction etc. and writes them to files with automatically added extension
      \returns OK or FAIL
  
  */
  int dumpDetectorSetup(string const fname, int level=0);  


  /** 
      Loads the detector setup from file
      \param fname file to read from
      \param level if 2 reads also reads trimbits, angular conversion coefficients etc. from files with default extensions as generated by dumpDetectorSetup
      \returns OK or FAIL
  
  */
  int retrieveDetectorSetup(string const fname, int level=0);

  static int dummyAcquisitionFinished(double prog,int status,void* p){cout <<"Acquisition finished callback! " << prog << " " << status << endl; return 0;}
  static int dummyMeasurementFinished(int im,int findex,void* p){cout <<"Measurement finished callback! " << im << " " << findex << endl; return 0;}




  //receiver


  /**
     Checks if the receiver is really online
  */
  virtual string checkReceiverOnline()=0;

  /**
     Returns the IP of the last client connecting to the receiver
  */
  virtual string getReceiverLastClientIP()=0;


  /**
     Sets up the file directory
     @param fileName fileDir file directory
     \returns file dir
  */
  virtual string setFilePath(string s="")=0;

  /**
     Sets up the file name
     @param fileName file name
     \returns file name
  */
  virtual string setFileName(string s="")=0;

  /**
     \returns file dir
  */
  virtual string getFilePath()=0;

  /**
     \returns file name
  */
  virtual string getFileName()=0;

  /**
     \returns frames caught by receiver
  */
 virtual int getFramesCaughtByReceiver()=0;

 /**
    \returns current frame index of receiver
 */
virtual int getReceiverCurrentFrameIndex()=0;

/**
 * resets framescaught
 * @param index frames caught by receiver
*/
virtual int resetFramesCaught()=0;

/**
 * Reads a frame from receiver
 * @param fName file name of current frame()
 * @param acquisitionIndex current acquisition index
 * @param frameIndex current frame index (for each scan)
 * @param subFrameIndex current sub frame index (for 32 bit mode for eiger)
 /returns a frame read from recever
*/
virtual int* readFrameFromReceiver(char* fName, int &acquisitionIndex, int &frameIndex, int &subFrameIndex)=0;


/**
    Turns off the receiver server!
*/
virtual int exitReceiver()=0;

/**
   Sets/Gets file overwrite enable
   @param enable 1 or 0 to set/reset file overwrite enable
   /returns file overwrite enable
*/
virtual int overwriteFile(int enable=-1)=0;


/**
   Sets/Gets receiver file write enable
   @param enable 1 or 0 to set/reset file write enable
   /returns file write enable
*/
virtual int enableWriteToFile(int enable=-1)=0;

/** Starts acquisition, calibrates pedestal and writes to fpga
   /returns number of frames
*/
virtual int calibratePedestal(int frames = 0)=0;


/**
    set roi
     \param n number of rois
     \param roiLimits array of roi
     \returns success or failure
*/
virtual int setROI(int n=-1,ROI roiLimits[]=NULL)=0;

/**
 	get roi from each detector and convert it to the multi detector scale
 	\param n number of rois
 	\returns an array of multidetector's rois
*/
virtual ROI* getROI(int &n)=0;

/** Sets the read receiver frequency
 	  if Receiver read upon gui request, readRxrFrequency=0,
 	   else every nth frame to be sent to gui
 	   @param getFromReceiver is 1 if it should ask the receiver,
 	   0 if it can get it from multislsdetecter
 	   @param i is the receiver read frequency
 	   /returns read receiver frequency
 */
virtual int setReadReceiverFrequency(int getFromReceiver, int i=-1)=0;


/** enable/disable or get data compression in receiver
 * @param i is -1 to get, 0 to disable and 1 to enable
   /returns data compression in receiver
 */
virtual int enableReceiverCompression(int i = -1)=0;

/** enable/disable or 10Gbe
 * @param i is -1 to get, 0 to disable and 1 to enable
   /returns if 10Gbe is enabled
 */
virtual int enableTenGigabitEthernet(int i = -1)=0;

/** set/get receiver fifo depth
 * @param i is -1 to get, any other value to set the fifo deph
   /returns the receiver fifo depth
 */
virtual int setReceiverFifoDepth(int i = -1)=0;

  /******** CTB funcs */

  /** opens pattern file and sends pattern to CTB 
      @param fname pattern file to open
      @returns OK/FAIL
  */
  virtual int setCTBPattern(string fname)=0;

  
  /** Writes a pattern word to the CTB
      @param addr address of the word, -1 is I/O control register,  -2 is clk control register
      @param word 64bit word to be written, -1 gets
      @returns actual value
  */
  virtual uint64_t setCTBWord(int addr,uint64_t word=-1)=0;  
  
  /** Sets the pattern or loop limits in the CTB
      @param level -1 complete pattern, 0,1,2, loop level
      @param start start address if >=0
      @param stop stop address if >=0
      @param n number of loops (if level >=0)
      @returns OK/FAIL
  */
  virtual int setCTBPatLoops(int level,int &start, int &stop, int &n)=0;  


  /** Sets the wait address in the CTB
      @param level  0,1,2, wait level
      @param addr wait address, -1 gets
      @returns actual value
  */
  virtual int setCTBPatWaitAddr(int level, int addr=-1)=0;  

   /** Sets the wait time in the CTB
      @param level  0,1,2, wait level
      @param t wait time, -1 gets
      @returns actual value
  */
  virtual int setCTBPatWaitTime(int level, uint64_t t=-1)=0;  

  /**
     Pulse Pixel
     \param n is number of times to pulse
     \param x is x coordinate
     \param y is y coordinate
     \returns OK or FAIL
  */
  virtual int pulsePixel(int n=0,int x=0,int y=0)=0;

  /**
     Pulse Pixel and move by a relative value
     \param n is number of times to pulse
     \param x is relative x value
     \param y is relative y value
     \returns OK or FAIL
  */
  virtual int pulsePixelNMove(int n=0,int x=0,int y=0)=0;

  /**
     Pulse Chip
     \param n is number of times to pulse
     \returns OK or FAIL
  */
  virtual int pulseChip(int n=0)=0;

  /**
     Set acquiring flag in shared memory
     \param b acquiring flag
   */
  virtual void setAcquiringFlag(bool b=false)=0;

  /**
     Get acquiring flag from shared memory
     \returns acquiring flag
   */
  virtual bool getAcquiringFlag() = 0;


















  protected:


  static const int64_t thisSoftwareVersion=0x20141013;

 
 
  //protected:
  int *stoppedFlag;	 

  int64_t *timerValue;
  detectorSettings *currentSettings;
  int *currentThresholdEV;


  int totalProgress;
	      		  
  int progressIndex;
	  
  double (*get_position)(void*);
  int (*go_to_position)(double, void*);
  int (*go_to_position_no_wait)(double, void*);
  int (*connect_channels)(void*);
  int (*disconnect_channels)(void*);
  double (*get_i0)(int, void*);
  void *POarg,*CCarg,*DCarg,*GTarg,*GTNarg,*IOarg;
  int (*acquisition_finished)(double,int,void*);
  int (*measurement_finished)(int,int,void*);
  void *acqFinished_p, *measFinished_p;



  int (*progress_call)(double,void*);
  void *pProgressCallArg;
  
};



#endif

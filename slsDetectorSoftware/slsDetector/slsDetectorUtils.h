

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
#include <semaphore.h>
#include <cstdlib>



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

  /**
   * Used when reference is slsDetectorUtils and to determine if command can be implemented as slsDetector/multiSlsDetector object/
   */
  virtual bool isMultiSlsDetectorClass()=0;

  virtual int getNumberOfDetectors(){return 1; };
  

  virtual int getMaxNumberOfChannelsPerDetector(dimension d){return -1;};

  virtual int setMaxNumberOfChannelsPerDetector(dimension d,int i){return -1;};

  /** sets the enable which determines if data will be flipped across x or y axis
   *  \param d axis across which data is flipped
   *  \param value 0 or 1 to reset/set or -1 to get value
   *  \return enable flipped data across x or y axis
   */
  virtual int setFlippedData(dimension d=X, int value=-1)=0;

  /**
   * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. 4 bit mode gap pixels only in gui call back
   * @param val 1 sets, 0 unsets, -1 gets
   * @return gap pixel enable or -1 for error
   */
  virtual int enableGapPixels(int val=-1) = 0;

  //int setPositions(int nPos, double *pos){return angularConversion::setPositions(nPos, pos);};

  // int getPositions(double *pos=NULL){return angularConversion::getPositions(pos);};
  
  using slsDetectorBase::setFlatFieldCorrection;
  using slsDetectorBase::getDetectorsType;
  using postProcessing::setBadChannelCorrection;

  int enableFlatFieldCorrection(int i=-1) {if (i>0) setFlatFieldCorrectionFile("default"); else if (i==0) setFlatFieldCorrectionFile(""); return getFlatFieldCorrection();};
  int enablePixelMaskCorrection(int i=-1) {if (i>0) setBadChannelCorrection("default"); else if (i==0) setBadChannelCorrection(""); return getBadChannelCorrection();};
  int enableCountRateCorrection(int i=-1) {if (i>0) setRateCorrection(-1); else if (i==0) setRateCorrection(0); return getRateCorrection();};


  /**
   * Set/Get receiver streaming out ZMQ port
   * For multi modules, it calculates (increments) and sets the ports
   * @param i sets, -1 gets
   * @returns receiver streaming out ZMQ port
   */
  int setReceiverDataStreamingOutPort(int i) {								\
	  if (i >= 0) {															\
		  std::ostringstream ss; ss << i; std::string s = ss.str();					\
		  int prev_streaming = enableDataStreamingFromReceiver();			\
		  setNetworkParameter(RECEIVER_STREAMING_PORT, s);					\
		  if (prev_streaming) {												\
			  enableDataStreamingFromReceiver(0);							\
			  enableDataStreamingFromReceiver(1);}}							\
	  return atoi(getNetworkParameter(RECEIVER_STREAMING_PORT).c_str());};	\

  /**
   * Set/Get client streaming in ZMQ port
   * For multi modules, it calculates (increments) and sets the ports
   * @param i sets, -1 gets
   * @returns client streaming in ZMQ port
   */
  int setClientDataStreamingInPort(int i){										\
		  if (i >= 0) {															\
			  std::ostringstream ss; ss << i; std::string s = ss.str();					\
			  int prev_streaming = enableDataStreamingToClient();				\
			  setNetworkParameter(CLIENT_STREAMING_PORT, s);					\
			  if (prev_streaming) {												\
				  enableDataStreamingToClient(0);								\
				  enableDataStreamingToClient(1);}}								\
		  return atoi(getNetworkParameter(CLIENT_STREAMING_PORT).c_str());};	\

  /**
   * Set/Get receiver streaming out ZMQ port
   * For multi modules, it calculates (increments) and sets the ports
   * @param i sets, -1 gets
   * @returns receiver streaming out ZMQ port
   */
   std::string setReceiverDataStreamingOutIP(std::string ip) {							\
		if (ip.length()) {														\
			int prev_streaming = enableDataStreamingFromReceiver();				\
			setNetworkParameter(RECEIVER_STREAMING_SRC_IP, ip);					\
			if (prev_streaming) {												\
				enableDataStreamingFromReceiver(0);								\
				enableDataStreamingFromReceiver(1);}}							\
		return getNetworkParameter(RECEIVER_STREAMING_SRC_IP);};				\

  /**
   * Set/Get client streaming in ZMQ port
   * For multi modules, it calculates (increments) and sets the ports
   * @param i sets, -1 gets
   * @returns client streaming in ZMQ port
   */
   std::string setClientDataStreamingInIP(std::string ip){								\
		if (ip.length()) {														\
			int prev_streaming = enableDataStreamingToClient();					\
			setNetworkParameter(CLIENT_STREAMING_SRC_IP, ip);					\
			if (prev_streaming) {												\
				enableDataStreamingToClient(0);									\
				enableDataStreamingToClient(1);}}								\
		return getNetworkParameter(CLIENT_STREAMING_SRC_IP);};					\

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
  virtual std::string getHostname(int pos=-1)=0;

  
  /** sets the detector hostname   
      \param name hostname
  */
  virtual void setHostname(const char* name)=0;

  /** adds the detector hostnames to the end of the list
      \param name hostname
  */
  virtual void addMultipleDetectors(const char* name)=0;

  /** returns the detector type
      \param pos position in the multi detector structure (is -1 returns type of detector with id -1)
      \returns type
  */
  virtual std::string sgetDetectorsType(int pos=-1)=0;

  /**
     gets the network parameters (implemented for gotthard)
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \returns parameter

  */
  virtual std::string getNetworkParameter(networkParameter i)=0;

  /**
     sets the network parameters
     must restart streaming in client/receiver if to do with zmq after calling this function
     \param i network parameter type
     \param s value to be set
     \returns parameter

  */
  virtual std::string setNetworkParameter(networkParameter i, std::string s)=0;

  int setFlowControl10G(int i = -1) {
      std::string sret="";
      if (i != -1) {
          std::ostringstream o;
          o << ((i >= 1) ? 1 : 0);
          std::string sval = o.str();
          sret = setNetworkParameter(FLOW_CONTROL_10G, sval);
      } else
          sret = getNetworkParameter(FLOW_CONTROL_10G);

      return atoi(sret.c_str());
  }

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
  virtual std::string checkOnline()=0;

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
  virtual std::string getSettingsFile()=0;

  
  /** 
      get current timer value
      \param index timer index
      \param imod module number
      \returns elapsed time value in ns or number of...(e.g. frames, gates, probes)
  */
  virtual int64_t getTimeLeft(timerIndex index, int imod = -1)=0;

  /**
   * set storage cell that stores first acquisition of the series (Jungfrau only)
   * \param value storage cell index. Value can be 0 to 15. (-1 gets)
   * \returns the storage cell that stores the first acquisition of the series
   */
  virtual int setStoragecellStart(int pos=-1)=0;



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





  int setBadChannelCorrection(std::string fname, int &nbadtot, int *badchanlist, int off=0);





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
  virtual std::string getSettingsDir()=0;

  /** sets the detector trimbit/settings directory  */
  virtual std::string setSettingsDir(std::string s)=0;

  /**
     returns the location of the calibration files
  */
  virtual std::string getCalDir()=0; 

  /**
     sets the location of the calibration files
  */
  virtual std::string setCalDir(std::string s)=0;

  /** Frees the shared memory  -  should not be used except for debugging*/
  virtual void freeSharedMemory()=0;

	/**
	 * Get user details of shared memory
	 * @returns string with user details
	 */
  virtual std::string getUserDetails() = 0;

  /** adds the detector name in position pos
      \param name of the detector to be added (should already exist in shared memory or at least be online) 
      \param pos position where it should be added (normally at the end of the list (default to -1)
      \return the actual number of detectors or -1 if it failed (always for slsDetector)
  */
  virtual int addSlsDetector(char* name, int pos=-1){return -1;};


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
  virtual int loadImageToDetector(imageType index,std::string const fname)=0;
  

  /**
     writes the counter memory block from the detector
     \param startACQ is 1 to start acquisition after reading counter
     \fname file fname to load data from
     \returns OK or FAIL
  */
  virtual int writeCounterBlockFile(std::string const fname,int startACQ=0)=0;


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

  /**
   * Give an internal software trigger to the detector (Eiger only)
   * @return OK or FAIL
   */
  virtual int sendSoftwareTrigger()=0;



  //  double* convertAngles(){return convertAngles(currentPosition);};
  // virtual double* convertAngles(double pos)=0;

  virtual int setThresholdEnergy(int, int im=-1, detectorSettings isettings=GET_SETTINGS, int tb=1)=0;
  virtual int setChannel(int64_t, int ich=-1, int ichip=-1, int imod=-1)=0;

  virtual double getRateCorrectionTau()=0;
  virtual int* startAndReadAll()=0;

  virtual int getTotalNumberOfChannels()=0;
  virtual int getTotalNumberOfChannels(dimension d)=0;
  virtual int getMaxNumberOfChannels()=0;
  virtual int getMaxNumberOfChannels(dimension d)=0;

  /** returns the enable if data will be flipped across x or y axis
   *  \param d axis across which data is flipped
   *  returns 1 or 0
   */
  virtual int getFlippedData(dimension d=X)=0;

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
  virtual uint32_t writeRegister(uint32_t addr, uint32_t val)=0;


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
  virtual uint32_t readRegister(uint32_t addr)=0;


  /**
      sets a bit in a register
      \param addr address
      \param n nth bit ranging from 0 to 31
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual uint32_t setBit(uint32_t addr, int n)=0;


  /**
      clear a bit in a register
      \param addr address
      \param n nth bit ranging from 0 to 31
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
  */
  virtual uint32_t clearBit(uint32_t addr, int n)=0;


  /**
     Returns the IP of the last client connecting to the detector
  */
  virtual std::string getLastClientIP()=0;



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
  virtual int loadSettingsFile(std::string fname, int imod=-1)=0;

  /** programs FPGA with pof file
      \param fname file name
      \returns OK or FAIL
  */
  virtual int programFPGA(std::string fname)=0;

  /** resets FPGA
      \returns OK or FAIL
  */
  virtual int resetFPGA()=0;

  /** power on/off the chip
     \param ival on is 1, off is 0, -1 to get
      \returns OK or FAIL
  */
  virtual int powerChip(int ival= -1)=0;

  /** automatic comparator disable for Jungfrau only
     \param ival on is 1, off is 0, -1 to get
      \returns OK or FAIL
  */
  virtual int setAutoComparatorDisableMode(int ival= -1)=0;

  /** saves the modules settings/trimbits writing to  a file
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  virtual int saveSettingsFile(std::string fname, int imod=-1)=0;

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
  virtual dacs_t getADC(dacIndex index, int imod=-1)=0;

  /**
     set/gets threshold temperature (Jungfrau only)
     \param val value in millidegrees, -1 gets
     \param imod module number, -1 is all
     \returns threshold temperature in millidegrees
  */
  virtual int setThresholdTemperature(int val=-1, int imod=-1)=0;

  /**
     enables/disables temperature control (Jungfrau only)
     \param val value, -1 gets
     \param imod module number, -1 is all
     \returns temperature control enable
  */
  virtual int setTemperatureControl(int val=-1, int imod=-1)=0;

  /**
     Resets/ gets over-temperature event (Jungfrau only)
     \param val value, -1 gets
     \param imod module number, -1 is all
     \returns over-temperature event
  */
  virtual int setTemperatureEvent(int val=-1, int imod=-1)=0;

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
  virtual int writeConfigurationFile(std::string const fname)=0;


  void registerGetPositionCallback( double (*func)(void*),void *arg){get_position=func; POarg=arg;};
  void registerConnectChannelsCallback( int (*func)(void*),void *arg){connect_channels=func; CCarg=arg;};
  void registerDisconnectChannelsCallback(int (*func)(void*),void*arg){disconnect_channels=func;DCarg=arg;};
  
  void registerGoToPositionCallback( int (*func)(double, void*),void *arg){go_to_position=func;GTarg=arg;};
  void registerGoToPositionNoWaitCallback(int (*func)(double, void*),void*arg){go_to_position_no_wait=func;GTNarg=arg;};
  void registerGetI0Callback( double (*func)(int, void*),void *arg){get_i0=func;IOarg=arg;};
  
  void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg){acquisition_finished=func; acqFinished_p=pArg;};
  void registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg){measurement_finished=func; measFinished_p=pArg;};

  void registerProgressCallback(int( *func)(double,void*), void *pArg){progress_call=func; pProgressCallArg=pArg;};


  static int dummyAcquisitionFinished(double prog,int status,void* p){cout <<"Acquisition finished callback! " << prog << " " << status << endl; return 0;}
  static int dummyMeasurementFinished(int im,int findex,void* p){cout <<"Measurement finished callback! " << im << " " << findex << endl; return 0;}




  //receiver


  /**
     Checks if the receiver is really online
  */
  virtual std::string checkReceiverOnline()=0;

  /**
     Returns the IP of the last client connecting to the receiver
  */
  virtual std::string getReceiverLastClientIP()=0;


  /**
     Sets up the file directory
     @param fileName fileDir file directory
     \returns file dir
  */
  virtual std::string setFilePath(std::string s="")=0;

  /**
     Sets up the file name
     @param fileName file name
     \returns file name
  */
  virtual std::string setFileName(std::string s="")=0;

  /**
     Sets the max frames per file in receiver
     @param f max frames per file
     \returns max frames per file in receiver
  */
  virtual int setReceiverFramesPerFile(int f = -1) = 0;

  /**
     Sets the frames discard policy in receiver
     @param f frames discard policy
     \returns frames discard policy set in receiver
  */
  virtual frameDiscardPolicy setReceiverFramesDiscardPolicy(frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY) = 0;

  /**
     Sets the partial frames padding enable in receiver
     @param f partial frames padding enable
     \returns partial frames padding enable in receiver
  */
  virtual int setReceiverPartialFramesPadding(int f = -1) = 0;

  /**
     Sets up the file format
     @param f file format
     \returns file format
  */
  virtual fileFormat setFileFormat(fileFormat f=GET_FILE_FORMAT)=0;

  /**
     \returns file dir
  */
  virtual std::string getFilePath()=0;

  /**
     \returns file name
  */
  virtual std::string getFileName()=0;

  /**
     \returns file name
  */
  virtual fileFormat getFileFormat()=0;

  /**
     \returns frames caught by receiver
  */
 virtual int getFramesCaughtByReceiver()=0;

 /**   gets the number of frames caught by any one receiver (to avoid using threadpool)
	\returns number of frames caught by any one receiver (master receiver if exists)
 */
 virtual  int getFramesCaughtByAnyReceiver()=0;

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
 * Create Receiving Data Sockets
 * @param destroy is true to destroy all the sockets
 * @return OK or FAIL
 */
virtual int createReceivingDataSockets(const bool destroy = false){return -1;};


/** Reads frames from receiver through a constant socket
*/

virtual void readFrameFromReceiver(){};

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
     \param imod module number (-1 for all)
     \returns success or failure
*/
virtual int setROI(int n=-1,ROI roiLimits[]=NULL, int imod = -1)=0;

/**
 	get roi from each detector and convert it to the multi detector scale
 	\param n number of rois
 	\param imod module number (-1 for all)
 	\returns pointer to array of ROI structure
*/
virtual ROI* getROI(int &n, int imod = -1)=0;

/** Sets the read receiver frequency
 	  if data required from receiver randomly readRxrFrequency=0,
 	   else every nth frame to be sent to gui
 	   @param freq is the receiver read frequency
 	   /returns read receiver frequency
 */
virtual int setReadReceiverFrequency(int freq=-1)=0;

/**
 * Enable data streaming to client
 * @param enable 0 to disable, 1 to enable, -1 to get the value
 * @returns data streaming to client enable
 */
virtual int enableDataStreamingToClient(int enable=-1)=0;

/** Enable or disable streaming data from receiver to client
 * @param enable 0 to disable 1 to enable -1 to only get the value
 * @returns data streaming from receiver enable
*/
virtual int enableDataStreamingFromReceiver(int enable=-1)=0;


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

/** set/get receiver silent mode
 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
   /returns the receiver silent mode enable
 */
virtual int setReceiverSilentMode(int i = -1)=0;

  /******** CTB funcs */

  /** opens pattern file and sends pattern to CTB 
      @param fname pattern file to open
      @returns OK/FAIL
  */
  virtual int setCTBPattern(std::string fname)=0;

  
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


  /**
   * Check if acquiring flag is set, set error if set
   * \returns FAIL if not ready, OK if ready
   */
  virtual bool isAcquireReady() = 0;


  /**
   * Check version compatibility with detector/receiver software
   * (if hostname/rx_hostname has been set/ sockets created)
   * \param p port type control port or data (receiver) port
   * \returns FAIL for incompatibility, OK for compatibility
   */
  virtual int checkVersionCompatibility(portType t) = 0;

	/**
	 * Set or Get Quad Type (Only for Eiger Quad detector hardware)
	 * @param val 1 if quad type set, else 0, -1 gets
	 * @returns  1 if quad type set, else 0
	 */
	virtual int setQuad(int val = -1) = 0;

   /**
	 * Set or Get Interrupt last sub frame(Only for Eiger)
	 * @param val 1 if interrupt last subframe set, else 0, -1 gets
	 * @returns  1 if interrupt last subframe set, else 0, -1 different values
	 */
	virtual int setInterruptSubframe(int val = -1) = 0;

 protected:

 
 
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
  
  /** semaphore to let postprocessing thread continue for next scan/measurement */
  sem_t sem_newRTAcquisition;
  /** semaphore to let main thread know it got all the dummy packets (also from ext. process) */
  sem_t sem_endRTAcquisition;


};



#endif

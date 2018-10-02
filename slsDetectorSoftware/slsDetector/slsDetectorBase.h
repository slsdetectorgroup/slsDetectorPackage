
#ifndef SLS_DETECTOR_BASE_H
#define SLS_DETECTOR_BASE_H
/**
   \mainpage Common C++ library for SLS detectors data acquisition
   *
   * \section intro_sec Introduction

   * \subsection mot_sec Motivation
   Although the SLS detectors group delvelops several types of detectors (1/2D, counting/integrating etc.) it is common interest of the group to use a common platfor for data acquisition
   \subsection arch_sec System Architecture
   The architecture of the acquisitions system is intended as follows:
   \li A socket server running on the detector (or more than one in some special cases)
   \li C++ classes common to all detectors for client-server communication. These can be supplied to users as libraries and embedded also in acquisition systems which are not developed by the SLS
   \li the possibility of using a Qt-based graphical user interface (with eventually root analisys capabilities)
   \li the possibility of running all commands from command line. In order to ensure a fast operation of this so called "text client" the detector parameters should not be re-initialized everytime. For this reason a shared memory block is allocated where the main detector flags and parameters are stored 
   \li a Root library for data postprocessing and detector calibration (energy, angle).

   \section howto_sec How to use it

   The detectors can be simply operated by using the provided GUi or command line executable. <br>
   In case you need to embed the detector control e.g in the beamline control software, compile these classes using 
   <BR>
   make package
   <br>
   and link the shared library created to your software slsDetectorSoftware/bin/libSlsDetector.so
   <br>
   The software can also be installed (with super-user rights)<br>
   make install
   <br>
   <br>
   Most methods of interest for the user are implemented in the ::slsDetectorBase interface class, but the classes to be implemented in the main program are either ::slsDetector (for single controller detectors) or ::multiSlsDetector (for multiple controllers, but can work also for single controllers).

   @author Anna Bergamaschi
   @version 0.1alpha

*/



/**
 * 
 *
 *
 * @author Anna Bergamaschi
 * @version 0.1alpha
 */


#include "sls_detector_defs.h"
#include "sls_receiver_defs.h"
#include "slsDetectorUsers.h"
#include "error_defs.h"

#include <string>

/** 

@libdoc The slsDetectorBase contains also a set of purely virtual functions useful for the implementation of the derived classes


* @short This is the base class for all detector functionalities

*/

//public virtual slsDetectorUsers,
class slsDetectorBase :  public virtual slsDetectorDefs, public virtual errorDefs {

 public:

  /** default constructor */
  slsDetectorBase(){};


  /** virtual destructor */
  virtual ~slsDetectorBase(){};

  /** returns the detector type
      \param pos position in the multi detector structure (is -1 returns type of detector with id -1)
      \returns type
  */
  virtual detectorType getDetectorsType(int pos=-1)=0;

  std::string getDetectorDeveloper(){return std::string("PSI");};
  // protected:

  /**
     Writes the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels etc.)
     \param fname file name
     \returns OK or FAIL
  */
  virtual int writeConfigurationFile(std::string const fname)=0;


  /**
     Loads dark image or gain image to the detector
     \param index can be DARK_IMAGE or GAIN_IMAGE
     \param fname file name to load data from
     \returns OK or FAIL
  */
  virtual int loadImageToDetector(imageType index,std::string const fname)=0;

  /**
     \returns total number of channels
  */
  virtual int getTotalNumberOfChannels()=0;

  /**
     \returns total number of channels for each dimension
  */
  virtual int getTotalNumberOfChannels(dimension d)=0;



  virtual void incrementProgress()=0;
  virtual void setCurrentProgress(int i=0)=0;
  virtual double getCurrentProgress()=0;
  virtual int setTotalProgress()=0;



  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int setRateCorrection(double t=0)=0;

  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection(double &t)=0;
  
  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection()=0;
    
  /** 
      set/get dynamic range
      \param i dynamic range (-1 get)
      \returns current dynamic range
  */
  virtual int setDynamicRange(int i=-1)=0;


  /** Locks/Unlocks the connection to the server
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the server
  */
  virtual int lockServer(int i=-1)=0;  

  

  /** performs a complete acquisition including scansand data processing 
      moves the detector to next position <br>
      starts and reads the detector <br>
      reads the IC (if required) <br>
      reads the encoder (iof required for angualr conversion) <br>
      processes the data (flat field, rate, angular conversion and merging ::processData())
      \param delflag 0 leaves the data in the final data queue (default is 1)
      \returns OK or FAIL depending on if it already started
  */
  virtual int acquire(int delflag=1)=0;

  int startMeasurement(){acquire(0); return OK;};


  /**
     get detector ids/versions for module=0
     \param mode which id/version has to be read
     \param imod module number for module serial number
     \returns id
  */
  virtual int64_t getId(idMode mode, int imod=0)=0;
  int64_t getDetectorFirmwareVersion(){return getId(DETECTOR_FIRMWARE_VERSION,-1);};
  int64_t getDetectorSerialNumber(){return getId(DETECTOR_SERIAL_NUMBER,-1);};
  int64_t getDetectorSoftwareVersion(){return getId(DETECTOR_SOFTWARE_VERSION,-1);};
  int64_t getThisSoftwareVersion(){return getId(THIS_SOFTWARE_VERSION,-1);};

  /**
     start detector acquisition
     \returns OK/FAIL
  */
  virtual int startAcquisition()=0;
  /**
     stop detector acquisition
     \returns OK/FAIL
  */
  virtual int stopAcquisition()=0;
  int stopMeasurement(){return stopAcquisition();};

  /**
     set/get timer value
     \param index timer index
     \param t time in ns or number of...(e.g. frames, gates)
     \param imod module number
     \returns timer set value in ns or number of...(e.g. frames, gates)
  */
  virtual int64_t setTimer(timerIndex index, int64_t t=-1, int imod = -1)=0;
  int64_t setExposureTime(int64_t t=-1, int imod = -1){return setTimer(ACQUISITION_TIME,t,imod);};
  int64_t setSubFrameExposureTime(int64_t t=-1, int imod = -1){return setTimer(SUBFRAME_ACQUISITION_TIME,t,imod);};
  int64_t setSubFrameDeadTime(int64_t t=-1, int imod = -1){return setTimer(SUBFRAME_DEADTIME,t,imod);};
  int64_t setExposurePeriod(int64_t t=-1, int imod = -1){return setTimer(FRAME_PERIOD,t,imod);};
  int64_t setDelayAfterTrigger(int64_t t=-1, int imod = -1){return setTimer(DELAY_AFTER_TRIGGER,t,imod);};
  int64_t setNumberOfGates(int64_t t=-1, int imod = -1){return setTimer(GATES_NUMBER,t,imod);};
  int64_t setNumberOfFrames(int64_t t=-1, int imod = -1){return setTimer(FRAME_NUMBER,t,imod);};
  int64_t setNumberOfCycles(int64_t t=-1, int imod = -1){return setTimer(CYCLES_NUMBER,t,imod);};


  /** sets/gets the value of important readout speed parameters
      \param sp is the parameter to be set/get
      \param value is the value to be set, if -1 get value
      \returns current value for the specified parameter
      \sa speedVariable
  */
  virtual int setSpeed(speedVariable sp, int value=-1)=0;
  int setClockDivider(int s=-1){return setSpeed(CLOCK_DIVIDER,s);};

  /**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
  */
  virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;
  void setParallelMode(int value){						\
	  if(value>=0){										\
		  switch(value){								\
		  case 0: setReadOutFlags(NONPARALLEL);break;	\
		  case 1: setReadOutFlags(PARALLEL);break;		\
		  default: setReadOutFlags(SAFE);break;			\
		  }												\
	  }													\
  };

  void setOverflowMode(int value){						\
	  if(value>=0){										\
		  switch(value){								\
		  case 1: setReadOutFlags(SHOW_OVERFLOW);break;	\
		  case 0: setReadOutFlags(NOOVERFLOW);break;	\
		  }												\
	  }													\
  };

  /**
   	 get readout mode of detector (eiger specific)
   	 \returns 0 for nonparallel, 1 for parallel, 2 for safe
   */
  int getParallelMode(){								\
	  int ret = setReadOutFlags();						\
	  if (ret&NONPARALLEL) return 0;					\
	  if (ret&PARALLEL) return 1;						\
	  if (ret&SAFE) return 2; 							\
	  return -1;										\
  }


  /**
   	 get readout overflow mode of detector (eiger specific)
   	 \returns 1 for show overflow, 0 for do not show overflow
   */
  int getOverflowMode(){								\
	  int ret = setReadOutFlags();						\
	  if (ret&SHOW_OVERFLOW) return 1;					\
	  if (ret&NOOVERFLOW) return 0;						\
	  return -1;										\
  }														\

  /**
     set/ get high voltage
     \param val high voltage (>0 sets, 0 unsets, -1 gets)
     \returns high voltage
   */
  int setHighVoltage(int val){return setDAC(val, HV_NEW, 0, -1);}														\

  /**
     set dacs value
     \param val value
     \param index DAC index
     \param mV 0 in dac units or 1 in mV
     \param imod module number (if -1 alla modules)
     \returns current DAC value
  */
  virtual dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1)=0;
  int setDACValue(int val, int index , int imod=-1) { return (int)setDAC((dacs_t)val,(dacIndex)index,0,imod);};

  /**
     gets ADC value
     \param index ADC index
     \param imod module number
     \returns current ADC value
  */
  virtual dacs_t getADC(dacIndex index, int imod=-1)=0;
  int getADCValue(int index, int imod=-1){return (int)getADC((dacIndex)index, imod);};

  /**
     @short get run status
     \returns status mask
  */
  virtual runStatus getRunStatus()=0;
  int getDetectorStatus() {return (int)getRunStatus();};


  /**  @short sets the onlineFlag
       \param online can be: -1 returns wether the detector is in online (1) or offline (0) state; 0 detector in offline state; 1  detector in online state
       \returns 0 (offline) or 1 (online)
  */
  virtual int setOnline(int const online=-1)=0;

	/**
	 * Activates/Deactivates the detector (Eiger only)
	 * @param enable active (1) or inactive (0), -1 gets
	 * @returns 0 (inactive) or 1 (active)for activate mode
	 */
  virtual int activate(int const enable=-1)=0;

	/**
	 * Set deactivated Receiver padding mode (Eiger only)
	 * @param padding padding option for deactivated receiver.  Can be 1 (padding), 0 (no padding), -1 (gets)
	 * @returns 1 (padding), 0 (no padding), -1 (inconsistent values) for padding option
	 */
  virtual int setDeactivatedRxrPaddingMode(int padding=-1)=0;

  /**
     @short set detector settings
     \param isettings  settings index (-1 gets)
     \returns current settings
  */
  virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1)=0;
  int setSettings(int isettings){return (int)setSettings((detectorSettings)isettings,-1);};

  virtual detectorSettings getSettings(int imod=-1)=0;  
  /**
     get threshold energy
     \param imod module number (-1 all)
     \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int getThresholdEnergy(int imod)=0;  
  int getThresholdEnergy(){return getThresholdEnergy(-1);};

  /** 
      set/get the external communication mode
     
      obsolete \sa setExternalSignalFlags
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
  */
  virtual externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE)=0;
  int setTimingMode(int i=-1){return slsDetectorUsers::getTimingMode( externalCommunicationType( setExternalCommunicationMode(externalCommunicationType( slsDetectorUsers::getTimingMode(i)  ) ) ) );};

  virtual int setThresholdEnergy(int e_eV,  int imod, detectorSettings isettings=GET_SETTINGS, int tb=1)=0;
  int setThresholdEnergy(int e_eV){return setThresholdEnergy(e_eV,-1);};
  int setThresholdEnergy(int e_ev, int tb, int isettings, int id){return setThresholdEnergy(e_ev, id, (detectorSettings)isettings, tb);}


  /**
     Prints receiver configuration
     \returns OK or FAIL
  */
  virtual int printReceiverConfiguration()=0;

  /**
     Reads the configuration file fname
     \param fname file name
     \returns OK or FAIL
  */
  virtual int readConfigurationFile(std::string const fname)=0; 

  virtual int dumpDetectorSetup(std::string const fname, int level){return 0;};
  int dumpDetectorSetup(std::string const fname){return dumpDetectorSetup(fname,0);};
  virtual int retrieveDetectorSetup(std::string const fname, int level){return 0;};
  int retrieveDetectorSetup(std::string const fname){return retrieveDetectorSetup(fname,0);};
  /** 
      @short 
      \returns the default output file index
  */
  virtual int getFileIndex()=0;
  
  /**
     @short sets the default output file index
     \param i file index
     \returns the default output file index
  */
  virtual int setFileIndex(int i)=0;

  /**
     @short increments file index
     \returns the file index
  */
  virtual int incrementFileIndex()=0;


 //receiver
  /**
     calls setReceiverTCPSocket if online and sets the flag
  */
  virtual int setReceiverOnline(int const online=GET_ONLINE_FLAG)=0;

  /**   Starts the listening mode of receiver
        \returns OK or FAIL
  */
  virtual int startReceiver()=0;

  /**   Stops the listening mode of receiver
        \returns OK or FAIL
  */
  virtual int stopReceiver()=0;

  /**   gets the status of the listening mode of receiver
        \returns status
  */
  virtual runStatus getReceiverStatus()=0;

  /**   gets the number of frames caught by receiver
        \returns number of frames caught by receiver
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

  /** Locks/Unlocks the connection to the receiver
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the receiver
  */
  virtual int lockReceiver(int lock=-1)=0;


  /** Reads frames from receiver through a constant socket
  */
virtual void readFrameFromReceiver()=0;


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

  /** Sets the read receiver frequency
   	  if data required from receiver randomly readRxrFrequency=0,
   	   else every nth frame to be sent to gui
   	   @param freq is the receiver read frequency
   	   /returns read receiver frequency
   */
  virtual int setReadReceiverFrequency(int freq=-1)=0;


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
				 * Used when reference is slsDetectorUtils and to determine
				 * if command can be implemented as slsDetector/multiSlsDetector object/
				 */
				virtual bool isMultiSlsDetectorClass() = 0;

				/**
				 * Set acquiring flag in shared memory
				 * @param b acquiring flag
				 */
				virtual void setAcquiringFlag(bool b=false) = 0;

				/**
				 * Get acquiring flag from shared memory
				 * @returns acquiring flag
				 */
				virtual bool getAcquiringFlag() = 0;

				/**
				 * Check if acquiring flag is set, set error if set
				 * @returns FAIL if not ready, OK if ready
				 */
				virtual bool isAcquireReady() = 0;

				/**
				 * Check version compatibility with detector/receiver software
				 * (if hostname/rx_hostname has been set/ sockets created)
				 * @param p port type control port or receiver port
				 * @returns FAIL for incompatibility, OK for compatibility
				 */
				virtual int checkVersionCompatibility(portType t) = 0;

				/**
				 * Free shared memory and delete shared memory structure
				 */
				virtual void freeSharedMemory() = 0;

				/**
				 * Get user details of shared memory
				 * Should only be called from multi detector level
				 * @returns string with user details
				 */
				virtual std::string getUserDetails() = 0;

				/**
				 * Sets the hostname of all sls detectors in shared memory
				 * Connects to them to set up online flag
				 * @param name concatenated hostname of all the sls detectors
				 */
				virtual void setHostname(const char* name)=0;

				/**
				 * Gets the hostname of detector at particular position
				 * or concatenated hostnames of all the sls detectors
				 * @param pos position of detector in array, -1 for all detectors
				 * @returns concatenated hostnames of all detectors or hostname of specific one
				 */
				virtual std::string getHostname(int pos=-1)=0;

				/**
				 * Appends detectors to the end of the list in shared memory
				 * Connects to them to set up online flag
				 * @param name concatenated hostname of the sls detectors to be appended to the list
				 */
				virtual void addMultipleDetectors(const char* name)=0;


				using slsDetectorBase::getDetectorsType;
				/**
				 * Concatenates string types of all sls detectors or
				 * returns the detector type of the first sls detector
				 * @param pos position of sls detector in array, if -1, returns first detector type
				 * @returns detector type of sls detector in position pos, if -1, concatenates
				 */
				virtual std::string sgetDetectorsType(int pos=-1)=0;

				/**
				 * Returns the number of detectors in the multidetector structure
				 * @returns number of detectors
				 */
				virtual int getNumberOfDetectors(){return 1;};

				/**
				 * Returns the total number of channels of all sls detectors from shared memory
				 * @returns the total number of channels of all sls detectors
				 */
				virtual int getTotalNumberOfChannels()=0;

				/**
				 * Returns the total number of channels of all sls detectors in dimension d
				 * from shared memory
				 * @param d dimension d
				 * @returns the total number of channels of all sls detectors in dimension d
				 */
				virtual int getTotalNumberOfChannels(dimension d)=0;

				/**
				 * Returns the maximum number of channels of all sls detectors in each dimension d
				 * from shared memory. multi detector shared memory variable to calculate
				 * offsets for each sls detector
				 * @param d dimension d
				 * @returns the maximum number of channels of all sls detectors in dimension d
				 */
				virtual int getMaxNumberOfChannelsPerDetector(dimension d){return -1;};

				/**
				 * Sets the maximum number of channels of all sls detectors in each dimension d
				 * from shared memory, multi detector shared memory variable to calculate
				 * offsets for each sls detector
				 * @param d dimension d
				 * @param i maximum number of channels for multi structure in dimension d
				 * @returns the maximum number of channels of all sls detectors in dimension d
				 */
				virtual int setMaxNumberOfChannelsPerDetector(dimension d,int i){return -1;};

				/**
				 * Checks if each of the detectors are online/offline
				 * @returns empty string if they are all online,
				 * else returns concatenation of strings of all detectors that are offline
				 */
				virtual std::string checkOnline()=0;

				/**
				 * Set/Gets TCP Port of detector or receiver
				 * @param t port type
				 * @param num port number (-1 gets)
				 * @returns port number
				 */
				virtual int setPort(portType t, int num=-1)=0;

				/**
				 * Get last client IP saved on detector server
				 * @returns last client IP saved on detector server
				 */
				virtual std::string getLastClientIP()=0;

				/**
				 * Exit detector server
				 * @returns OK or FAIL
				 */
				virtual int exitServer()=0;

				/**
				 * Write current configuration to a file
				 * @param fname configuration file name
				 * @returns OK or FAIL
				 */
				virtual int writeConfigurationFile(std::string const fname)=0;

				/**
				 * Returns the trimfile or settings file name (Useless??)
				 * @returns the trimfile or settings file name
				 */
				virtual std::string getSettingsFile()=0;

				/**
				 * Set threshold energy (Mythen and Eiger)
				 * @param e_eV threshold in eV
				 * @param imod module number (-1 all)
				 * @param isettings ev. change settings
				 * @param tb 1 to include trimbits, 0 to exclude
				 * @returns current threshold value for imod in ev (-1 failed)
				 */
				virtual int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS,int tb=1)=0;

				/**
				 * Returns the detector trimbit/settings directory  \sa sharedSlsDetector
				 * @returns the trimbit/settings directory
				 */
				virtual std::string getSettingsDir()=0;

				/**
				 * Sets the detector trimbit/settings directory  \sa sharedSlsDetector
				 * @param s trimbits/settings directory
				 * @returns the trimbit/settings directory
				 */
				virtual std::string setSettingsDir(std::string s)=0;

				/**
				 * Returns the calibration files directory   \sa  sharedSlsDetector (Mythen)
				 * @returns the calibration files directory
				 */
				virtual std::string getCalDir()=0;

				/**
				 * Sets the calibration files directory   \sa  sharedSlsDetector (Mythen)
				 * @param s the calibration files directory
				 * @returns the calibration files directory
				 */
				virtual std::string setCalDir(std::string s)=0;

				/**
				 * Loads the modules settings/trimbits reading from a specific file
				 * file name extension is automatically generated.
				 * @param fname specific settings/trimbits file
				 * @param imod module number (-1 for all)
				 * returns OK or FAIL
				 */
				virtual int loadSettingsFile(std::string fname, int imod=-1)=0;

				/**
				 * Saves the modules settings/trimbits to a specific file
				 * file name extension is automatically generated.
				 * @param fname specific settings/trimbits file
				 * @param imod module number (-1 for all)
				 * returns OK or FAIL
				 */
				virtual int saveSettingsFile(std::string fname, int imod=-1)=0;

				/**
				 * Give an internal software trigger to the detector (Eiger only)
				 * @return OK or FAIL
				 */
				virtual int sendSoftwareTrigger()=0;

				/**
				 * Start detector acquisition and read all data (Blocking until end of acquisition)
				 * @returns OK or FAIL
				 */
				virtual int startAndReadAll()=0;

				/**
				 * Requests and  receives all data from the detector (Eiger store in ram)
				 * @returns OK or FAIL
				 */
				virtual int readAll()=0;

				/**
				 * Configures in detector the destination for UDP packets (Not Mythen)
				 * @returns OK or FAIL
				 */
				virtual int configureMAC()=0;

				/**
				 * Set/get timer value left in acquisition (not all implemented for all detectors)
				 * @param index timer index
				 * @param t time in ns or number of...(e.g. frames, gates, probes)
				 * @param imod module number
				 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
				 */
				virtual int64_t getTimeLeft(timerIndex index, int imod = -1)=0;

				/**
				 * Set speed
				 * @param sp speed type  (clkdivider option for Jungfrau and Eiger, others for Mythen/Gotthard)
				 * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other values check manual
				 * @returns value of speed set
				 */
				virtual int setSpeed(speedVariable sp, int value=-1)=0;

				/**
				 * Set/get dacs value
				 * @param val value (in V)
				 * @param index DAC index
				 * @param mV 0 in dac units or 1 in mV
				 * @param imod module number (if -1 all modules)
				 * @returns current DAC value
				 */
				virtual dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1)=0;

				/**
				 * Get adc value
				 * @param index adc(DAC) index
				 * @param imod module number (if -1 all modules)
				 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
				 */
				virtual dacs_t getADC(dacIndex index, int imod=-1)=0;

				/**
				 * Set/get external signal flags (to specify triggerinrising edge etc) (Gotthard, Mythen)
				 * @param pol external signal flag (-1 gets)
				 * @param signalindex singal index (0 - 3)
				 * @returns current timing mode
				 */
				virtual externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0)=0;

				/**
				 * Set/get readout flags (Eiger, Mythen)
				 * @param flag readout flag (Eiger options: parallel, nonparallel, safe etc.) (-1 gets)
				 * @returns readout flag
				 */
				virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;

				/**
				 * Write in a register. For Advanced users
				 * @param addr address of register
				 * @param val value to write into register
				 * @returns value read after writing
				 */
				virtual uint32_t writeRegister(uint32_t addr, uint32_t val)=0;

				/**
				 * Read from a register. For Advanced users
				 * @param addr address of register
				 * @returns value read from register
				 */
				virtual uint32_t readRegister(uint32_t addr)=0;

				/**
				 * Set bit in a register. For Advanced users
				 * @param addr address of register
				 * @param n nth bit
				 * @returns value read from register
				 */
				virtual uint32_t setBit(uint32_t addr, int n)=0;

				/**
				 * Clear bit in a register. For Advanced users
				 * @param addr address of register
				 * @param n nth bit
				 * @returns value read from register
				 */
				virtual uint32_t clearBit(uint32_t addr, int n)=0;

				/**
				 * Set network parameter
				 * @param p network parameter type
				 * @param s network parameter value
				 * @returns network parameter value set (from getNetworkParameter)
				 */
				virtual std::string setNetworkParameter(networkParameter p, std::string s)=0;

				/**
				 * Get network parameter
				 * @param p network parameter type
				 * @returns network parameter value set (from getNetworkParameter)
				 */
				virtual std::string getNetworkParameter(networkParameter)=0;

				/**
				 * Execute a digital test (Gotthard, Mythen)
				 * @param mode testmode type
				 * @param imod module index (-1 for all)
				 * @returns result of test
				 */
				virtual int digitalTest(digitalTestMode mode, int imod=0)=0;

				/**
				 * Load dark or gain image to detector (Gotthard)
				 * @param index image type
				 * @param fname file name from which to load image
				 * @returns OK or FAIL
				 */
				virtual int loadImageToDetector(imageType index,std::string const fname)=0;

				/**
				 * Writes the counter memory block from the detector (Gotthard)
				 * @param fname file name to load data from
				 * @param startACQ is 1 to start acquisition after reading counter
				 * @returns OK or FAIL
				 */
				virtual int writeCounterBlockFile(std::string const fname,int startACQ=0)=0;

				/**
				 * Resets counter in detector (Gotthard)
				 * @param startACQ is 1 to start acquisition after resetting counter
				 * @returns OK or FAIL
				 */
				virtual int resetCounterBlock(int startACQ=0)=0;

				/**
				 * Set/get counter bit in detector (Gotthard)
				 * @param i is -1 to get, 0 to reset and any other value to set the counter bit
				 * @returns the counter bit in detector
				 */
				virtual int setCounterBit(int i = -1)=0;

				/**
				 * Set ROI (Gotthard)
				 * At the moment only one set allowed
				 * @param n number of rois
				 * @param roiLimits array of roi
				 * @returns OK or FAIL
				 */
				virtual int setROI(int n=-1,ROI roiLimits[]=NULL)=0;

				/**
				 * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
				 * @param n number of rois
				 * @returns OK or FAIL
				 */
				virtual ROI* getROI(int &n)=0;

				/**
				 * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert users
				 * @param addr address of adc register
				 * @param val value
				 * @returns return value  (mostly -1 as it can't read adc register)
				 */
				virtual int writeAdcRegister(int addr, int val)=0;

				/**
				 * Returns the enable if data will be flipped across x or y axis (Eiger)
				 * @param d axis across which data is flipped
				 * @returns 1 for flipped, else 0
				 */
				virtual int getFlippedData(dimension d=X)=0;

				/**
				 * Sets the enable which determines if
				 * data will be flipped across x or y axis (Eiger)
				 * @param d axis across which data is flipped
				 * @param value 0 or 1 to reset/set or -1 to get value
				 * @returns enable flipped data across x or y axis
				 */
				virtual int setFlippedData(dimension d=X, int value=-1)=0;

				/**
				 * Sets all the trimbits to a particular value (Eiger)
				 * @param val trimbit value
				 * @param imod module number, -1 means all modules
				 * @returns OK or FAIL
				 */
				virtual int setAllTrimbits(int val, int imod=-1)=0;

				/**
				 * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
				 * 4 bit mode gap pixels only in gui call back
				 * @param val 1 sets, 0 unsets, -1 gets
				 * @returns gap pixel enable or -1 for error
				 */
				virtual int enableGapPixels(int val=-1)=0;

				/**
				 * Sets the number of trim energies and their value  (Eiger)
				 * \sa sharedSlsDetector
				 * @param nen number of energies
				 * @param en array of energies
				 * @returns number of trim energies
				 */
				virtual int setTrimEn(int nen, int *en=NULL)=0;

				/**
				 * Returns the number of trim energies and their value  (Eiger)
				 * \sa sharedSlsDetector
				 * @param en array of energies
				 * @returns number of trim energies
				 */
				virtual int getTrimEn(int *en=NULL)=0;

				/**
				 * Pulse Pixel (Eiger)
				 * @param n is number of times to pulse
				 * @param x is x coordinate
				 * @param y is y coordinate
				 * @returns OK or FAIL
				 */
				virtual int pulsePixel(int n=0,int x=0,int y=0)=0;

				/**
				 * Pulse Pixel and move by a relative value (Eiger)
				 * @param n is number of times to pulse
				 * @param x is relative x value
				 * @param y is relative y value
				 * @returns OK or FAIL
				 */
				virtual int pulsePixelNMove(int n=0,int x=0,int y=0)=0;

				/**
				 * Pulse Chip (Eiger)
				 * @param n is number of times to pulse
				 * @returns OK or FAIL
				 */
				virtual int pulseChip(int n=0)=0;

				/**
				 * Set/gets threshold temperature (Jungfrau)
				 * @param val value in millidegrees, -1 gets
				 * @param imod module number, -1 is all
				 * @returns threshold temperature in millidegrees
				 */
				virtual int setThresholdTemperature(int val=-1, int imod=-1)=0;

				/**
				 * Enables/disables temperature control (Jungfrau)
				 * @param val value, -1 gets
				 * @param imod module number, -1 is all
				 * @returns temperature control enable
				 */
				virtual int setTemperatureControl(int val=-1, int imod=-1)=0;

				/**
				 * Resets/ gets over-temperature event (Jungfrau)
				 * @param val value, -1 gets
				 * @param imod module number, -1 is all
				 * @returns over-temperature event
				 */
				virtual int setTemperatureEvent(int val=-1, int imod=-1)=0;

				/**
				 * Set storage cell that stores first acquisition of the series (Jungfrau)
				 * @param value storage cell index. Value can be 0 to 15. (-1 gets)
				 * @returns the storage cell that stores the first acquisition of the series
				 */
				virtual int setStoragecellStart(int pos=-1)=0;

				/**
				 * Programs FPGA with pof file (Jungfrau)
				 * @param fname file name
				 * @returns OK or FAIL
				 */
				virtual int programFPGA(std::string fname)=0;

				/**
				 * Resets FPGA (Jungfrau)
				 * @returns OK or FAIL
				 */
				virtual int resetFPGA()=0;

				/**
				 * Power on/off Chip (Jungfrau)
				 * @param ival on is 1, off is 0, -1 to get
				 * @returns OK or FAIL
				 */
				virtual int powerChip(int ival= -1)=0;

				/**
				 * Automatic comparator disable (Jungfrau)
				 * @param ival on is 1, off is 0, -1 to get
				 * @returns OK or FAIL
				 */
				virtual int setAutoComparatorDisableMode(int ival= -1)=0;

				/**
				 * Calibrate Pedestal (ChipTestBoard)
				 * Starts acquisition, calibrates pedestal and writes to fpga
				 * @param frames number of frames
				 * @returns number of frames
				 */
				virtual int calibratePedestal(int frames = 0)=0;

				/**
				 * Get rate correction tau (Mythen, Eiger)
				 * @returns 0 if rate correction disabled, otherwise the tau used for the correction
				 */
				virtual double getRateCorrectionTau()=0;

				/**
				 * Checks if the receiver is really online
				 * @returns empty string if all online, else concatenates hostnames of all
				 * detectors that are offline
				 */
				virtual std::string checkReceiverOnline()=0;

				/**
				 * Locks/Unlocks the connection to the receiver
				 * @param lock sets (1), usets (0), gets (-1) the lock
				 * @returns lock status of the receiver
				 */
				virtual int lockReceiver(int lock=-1)=0;

				/**
				 * Returns the IP of the last client connecting to the receiver
				 * @returns IP of last client connecting to receiver
				 */
				virtual std::string getReceiverLastClientIP()=0;

				/**
				 * Turns off the receiver server!
				 * @returns OK or FAIL
				 */
				virtual int exitReceiver()=0;

				/**
				 * Returns output file directory
				 * @returns output file directory
				 */
				virtual std::string getFilePath()=0;

				/**
				 * Sets up the file directory
				 * @param s file directory
				 * @returns file dir
				 */
				virtual std::string setFilePath(std::string s)=0;

				/**
				 * Returns file name prefix
				 * @returns file name prefix
				 */
				virtual std::string getFileName()=0;

				/**
				 * Sets up the file name prefix
				 * @param s file name prefix
				 * @returns file name prefix
				 */
				virtual std::string setFileName(std::string s)=0;

				/**
				 * Sets the max frames per file in receiver
				 * @param f max frames per file
				 * @returns max frames per file in receiver
				 */
				virtual int setReceiverFramesPerFile(int f=-1)=0;

				/**
				 * Sets the frames discard policy in receiver
				 * @param f frames discard policy
				 * @returns frames discard policy set in receiver
				 */
				virtual frameDiscardPolicy setReceiverFramesDiscardPolicy(frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY)=0;

				/**
				 * Sets the partial frames padding enable in receiver
				 * @param f partial frames padding enable
				 * @returns partial frames padding enable in receiver
				 */
				virtual int setReceiverPartialFramesPadding(int f = -1)=0;

				/**
				 * Returns file format
				 * @returns file name
				 */
				virtual fileFormat getFileFormat()=0;

				/**
				 * Sets up the file format
				 * @param f file format
				 * @returns file format
				 */
				virtual fileFormat setFileFormat(fileFormat f)=0;

				/**
				 * Gets the number of frames caught by receiver
				 * @returns number of frames caught by receiver
				 */
				virtual int getFramesCaughtByReceiver()=0;

				/**
				 * Gets the number of frames caught by any one receiver (to avoid using threadpool)
				 * @returns number of frames caught by any one receiver (master receiver if exists)
				 */
				virtual int getFramesCaughtByAnyReceiver()=0;

				/**
				 * Gets the current frame index of receiver
				 * @returns current frame index of receiver
				 */
				virtual int getReceiverCurrentFrameIndex()=0;

				/**
				 * Resets framescaught in receiver
				 * Use this when using startAcquisition instead of acquire
				 * @returns OK or FAIL
				 */
				virtual int resetFramesCaught()=0;

				/**
				 * Create Receiving Data Sockets
				 * @param destroy is true to destroy all the sockets
				 * @returns OK or FAIL
				 */
				virtual int createReceivingDataSockets(const bool destroy = false){return -1;};

				/**
				 * Reads frames from receiver through a constant socket
				 * Called during acquire() when call back registered or when using gui
				 */
				virtual void readFrameFromReceiver(){};

				/**
				 * Sets/Gets receiver file write enable
				 * @param enable 1 or 0 to set/reset file write enable
				 * @returns file write enable
				 */
				virtual int enableWriteToFile(int enable=-1)=0;

				/**
				 * Sets/Gets file overwrite enable
				 * @param enable 1 or 0 to set/reset file overwrite enable
				 * @returns file overwrite enable
				 */
				virtual int overwriteFile(int enable=-1)=0;

				/**
				 * Sets the read receiver frequency
				 * if data required from receiver randomly readRxrFrequency=0,
				 * else every nth frame to be sent to gui/callback
				 * @param freq is the receiver read frequency. Value 0 is 200 ms timer (other
				 * frames not sent), 1 is every frame, 2 is every second frame etc.
				 * @returns read receiver frequency
				 */
				virtual int setReadReceiverFrequency(int freq=-1)=0;

				/**
				 * Enable data streaming to client
				 * @param enable 0 to disable, 1 to enable, -1 to get the value
				 * @returns data streaming to client enable
				 */
				virtual int enableDataStreamingToClient(int enable=-1)=0;

				/**
				 * Enable or disable streaming data from receiver to client
				 * @param enable 0 to disable 1 to enable -1 to only get the value
				 * @returns data streaming from receiver enable
				 */
				virtual int enableDataStreamingFromReceiver(int enable=-1)=0;

				/**
				 * Enable/disable or 10Gbe
				 * @param i is -1 to get, 0 to disable and 1 to enable
				 * @returns if 10Gbe is enabled
				 */
				virtual int enableTenGigabitEthernet(int i = -1)=0;

				/**
				 * Set/get receiver fifo depth
				 * @param i is -1 to get, any other value to set the fifo deph
				 * @returns the receiver fifo depth
				 */
				virtual int setReceiverFifoDepth(int i = -1)=0;

				/**
				 * Set/get receiver silent mode
				 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
				 * @returns the receiver silent mode enable
				 */
				virtual int setReceiverSilentMode(int i = -1)=0;

				/**
				 * Opens pattern file and sends pattern to CTB
				 * @param fname pattern file to open
				 * @returns OK/FAIL
				 */
				virtual int setCTBPattern(std::string fname)=0;

				/**
				 * Writes a pattern word to the CTB
				 * @param addr address of the word, -1 is I/O control register,
				 * -2 is clk control register
				 * @param word 64bit word to be written, -1 gets
				 * @returns actual value
				 */
				virtual uint64_t setCTBWord(int addr,uint64_t word=-1)=0;

				/**
				 * Sets the pattern or loop limits in the CTB
				 * @param level -1 complete pattern, 0,1,2, loop level
				 * @param start start address if >=0
				 * @param stop stop address if >=0
				 * @param n number of loops (if level >=0)
				 * @returns OK/FAIL
				 */
				virtual int setCTBPatLoops(int level,int &start, int &stop, int &n)=0;

				/**
				 * Sets the wait address in the CTB
				 * @param level  0,1,2, wait level
				 * @param addr wait address, -1 gets
				 * @returns actual value
				 */
				virtual int setCTBPatWaitAddr(int level, int addr=-1)=0;

				/**
				 * Sets the wait time in the CTB
				 * @param level  0,1,2, wait level
				 * @param t wait time, -1 gets
				 * @returns actual value
				 */
				virtual int setCTBPatWaitTime(int level, uint64_t t=-1)=0;


  /** returns detector type std::string from detector type index
      \param t std::string can be  Eiger, Gotthard, Jungfrau, Unknown
      \returns  EIGER, GOTTHARD, JUNGFRAU, GENERIC
  */
  static std::string getDetectorType(detectorType t){\
    switch (t) {\
    case EIGER:    return std::string("Eiger");	\
    case GOTTHARD:    return std::string("Gotthard");	\
    case JUNGFRAU:    return std::string("Jungfrau");		\
    case JUNGFRAUCTB:    return std::string("JungfrauCTB");		\
    default:    return std::string("Unknown");		\
    }};

  /** returns detector type index from detector type std::string
      \param type can be EIGER, GOTTHARD, JUNGFRAU, GENERIC
      \returns Eiger, Gotthard, Jungfrau, Unknown
  */
  static detectorType getDetectorType(std::string const type){\
    if  (type=="Eiger")    return EIGER;		\
    if  (type=="Gotthard")    return GOTTHARD;	\
    if  (type=="Jungfrau")    return JUNGFRAU;		\
    if  (type=="JungfrauCTB")    return JUNGFRAUCTB;		\
    return GENERIC;};



  /** returns std::string from external signal type index
      \param f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG
      \returns std::string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
  */
  static std::string externalSignalType(externalSignalFlag f){\
    switch(f) {						 \
    case SIGNAL_OFF:      return std::string( "off");			\
    case GATE_IN_ACTIVE_HIGH:    return std::string( "gate_in_active_high");	\
    case GATE_IN_ACTIVE_LOW:    return std::string( "gate_in_active_low");	\
    case TRIGGER_IN_RISING_EDGE:    return std::string( "trigger_in_rising_edge"); \
    case TRIGGER_IN_FALLING_EDGE:    return std::string( "trigger_in_falling_edge");	\
    case RO_TRIGGER_IN_RISING_EDGE:    return std::string( "ro_trigger_in_rising_edge"); \
    case RO_TRIGGER_IN_FALLING_EDGE:    return std::string( "ro_trigger_in_falling_edge"); \
    case GATE_OUT_ACTIVE_HIGH:    return std::string( "gate_out_active_high"); \
    case GATE_OUT_ACTIVE_LOW:    return std::string( "gate_out_active_low");	\
    case TRIGGER_OUT_RISING_EDGE:    return std::string( "trigger_out_rising_edge");	\
    case TRIGGER_OUT_FALLING_EDGE:    return std::string( "trigger_out_falling_edge"); \
    case RO_TRIGGER_OUT_RISING_EDGE:      return std::string( "ro_trigger_out_rising_edge");	\
    case RO_TRIGGER_OUT_FALLING_EDGE:    return std::string( "ro_trigger_out_falling_edge");	\
    case MASTER_SLAVE_SYNCHRONIZATION: return std::string("sync");		\
    case OUTPUT_LOW: return std::string("gnd");		\
    case OUTPUT_HIGH: return std::string("vcc");		\
    default:    return std::string( "unknown");				\
    }    };
  



  /** returns external signal type index from std::string
      \param sval  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
      \returns can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG (if unknown)
  */

  static externalSignalFlag externalSignalType(std::string sval){\
    if (sval=="off")      return SIGNAL_OFF;\
    if (sval=="gate_in_active_high")      return GATE_IN_ACTIVE_HIGH;	\
    if  (sval=="gate_in_active_low") return GATE_IN_ACTIVE_LOW;\
    if  (sval=="trigger_in_rising_edge")  return TRIGGER_IN_RISING_EDGE;\
    if  (sval=="trigger_in_falling_edge") return TRIGGER_IN_FALLING_EDGE;\
    if  (sval=="ro_trigger_in_rising_edge") return RO_TRIGGER_IN_RISING_EDGE;\
    if  (sval=="ro_trigger_in_falling_edge") return RO_TRIGGER_IN_FALLING_EDGE;\
    if (sval=="gate_out_active_high")      return GATE_OUT_ACTIVE_HIGH;\
    if  (sval=="gate_out_active_low") return GATE_OUT_ACTIVE_LOW;\
    if  (sval=="trigger_out_rising_edge") return TRIGGER_OUT_RISING_EDGE;\
    if  (sval=="trigger_out_falling_edge") return TRIGGER_OUT_FALLING_EDGE;\
    if  (sval=="ro_trigger_out_rising_edge") return RO_TRIGGER_OUT_RISING_EDGE;\
    if  (sval=="ro_trigger_out_falling_edge") return RO_TRIGGER_OUT_FALLING_EDGE;\
    if  (sval=="sync") return MASTER_SLAVE_SYNCHRONIZATION;\
    if  (sval=="gnd") return OUTPUT_LOW;\
    if  (sval=="vcc") return OUTPUT_HIGH;\
    return GET_EXTERNAL_SIGNAL_FLAG ;};

  /** returns detector settings std::string from index
      \param s can be STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, LOWNOISE,
       DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2, GET_SETTINGS
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, lownoise,
      dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2, verylowgain, undefined
  */
  static std::string getDetectorSettings(detectorSettings s){\
    switch(s) {											\
    case STANDARD:      return std::string("standard");		\
    case FAST:      	return std::string("fast");			\
    case HIGHGAIN:      return std::string("highgain");		\
    case DYNAMICGAIN:   return std::string("dynamicgain");	\
    case LOWGAIN:    	return std::string("lowgain");		\
    case MEDIUMGAIN:    return std::string("mediumgain");	\
    case VERYHIGHGAIN:  return std::string("veryhighgain");	\
    case LOWNOISE:      return  std::string("lownoise");		\
    case DYNAMICHG0:    return  std::string("dynamichg0");	\
    case FIXGAIN1:      return  std::string("fixgain1");		\
    case FIXGAIN2:      return  std::string("fixgain2");		\
    case FORCESWITCHG1: return  std::string("forceswitchg1");\
    case FORCESWITCHG2: return  std::string("forceswitchg2");\
    case VERYLOWGAIN: return  std::string("verylowgain");\
    default:    		return std::string("undefined");		\
    }};

  /** returns detector settings std::string from index
      \param s can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, lownoise,
      dynamichg0, fixgain1, fixgain2, forceswitchg1, forceswitchg2, undefined
      \returns   setting index STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN,LOWNOISE,
      DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2, VERYLOWGAIN, GET_SETTINGS
  */

  static detectorSettings getDetectorSettings(std::string s){	\
    if (s=="standard") 		return STANDARD;				\
    if (s=="fast") 			return FAST;					\
    if (s=="highgain") 		return HIGHGAIN;				\
    if (s=="dynamicgain") 	return DYNAMICGAIN;				\
    if (s=="lowgain") 		return LOWGAIN;					\
    if (s=="mediumgain") 	return MEDIUMGAIN;				\
    if (s=="veryhighgain") 	return VERYHIGHGAIN;			\
    if (s=="lownoise") 		return LOWNOISE;				\
    if (s=="dynamichg0") 	return DYNAMICHG0;				\
    if (s=="fixgain1") 		return FIXGAIN1;				\
    if (s=="fixgain2") 		return FIXGAIN2;				\
    if (s=="forceswitchg1") return FORCESWITCHG1;			\
    if (s=="forceswitchg2")	return FORCESWITCHG2;			\
    if (s=="verylowgain")	return VERYLOWGAIN;				\
    return GET_SETTINGS;									\
  };


  /**
     returns external communication mode std::string from index
     \param f can be AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, BURST_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown
  */

  static std::string externalCommunicationType(externalCommunicationMode f){	\
    switch(f) {						 \
    case AUTO_TIMING:      return std::string( "auto");			\
    case TRIGGER_EXPOSURE: return std::string("trigger");			\
    case TRIGGER_READOUT: return std::string("ro_trigger");			\
    case GATE_FIX_NUMBER: return std::string("gating");			\
    case GATE_WITH_START_TRIGGER: return std::string("triggered_gating");	\
    case BURST_TRIGGER: return std::string("burst_trigger");	\
    default:    return std::string( "unknown");				\
    }    };
  


  /**
     returns external communication mode index from std::string
     \param sval can be auto, trigger, ro_trigger, gating, triggered_gating
     \returns AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, BURST_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
  */

  static externalCommunicationMode externalCommunicationType(std::string sval){\
    if (sval=="auto")      return AUTO_TIMING;\
    if (sval=="trigger")     return TRIGGER_EXPOSURE;	\
    if  (sval=="ro_trigger") return TRIGGER_READOUT;\
    if  (sval=="gating") return GATE_FIX_NUMBER;\
    if  (sval=="triggered_gating") return GATE_WITH_START_TRIGGER;\
    if  (sval=="burst_trigger") return BURST_TRIGGER;\
    return GET_EXTERNAL_COMMUNICATION_MODE;			\
  };

  /** returns std::string from run status index
      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
      \returns std::string error, waiting, running, data, finished
  */
  static std::string runStatusType(runStatus s){\
    switch (s) {				\
    case ERROR:       return std::string("error");		\
    case  WAITING:      return  std::string("waiting");	\
    case RUNNING:      return std::string("running");\
    case TRANSMITTING:      return std::string("data");	\
    case  RUN_FINISHED:      return std::string("finished");	\
    default:       return std::string("idle");		\
    }};

  /** returns std::string from file format index
      \param s can be RAW, HDF5
      \returns std::string raw, hdf5
  */
  static std::string fileFormats(fileFormat f){\
    switch (f) {				\
    case BINARY:       return std::string("binary");		\
    case ASCII:       return std::string("ascii");		\
    case HDF5:      return  std::string("hdf5");	\
    default:       return std::string("unknown");		\
    }};

  /** returns std::string from timer index
      \param s can be FRAME_NUMBER,ACQUISITION_TIME,FRAME_PERIOD, DELAY_AFTER_TRIGGER,GATES_NUMBER, CYCLES_NUMBER, ACTUAL_TIME,MEASUREMENT_TIME, PROGRESS,MEASUREMENTS_NUMBER,FRAMES_FROM_START,FRAMES_FROM_START_PG,SAMPLES_JCTB,SUBFRAME_ACQUISITION_TIME,STORAGE_CELL_NUMBER, SUBFRAME_DEADTIME
      \returns std::string frame_number,acquisition_time,frame_period, delay_after_trigger,gates_number, cycles_number, actual_time,measurement_time, progress,measurements_number,frames_from_start,frames_from_start_pg,samples_jctb,subframe_acquisition_time,storage_cell_number, SUBFRAME_DEADTIME
  */
  static std::string getTimerType(timerIndex t){										\
    switch (t) {																\
    case FRAME_NUMBER: 				return std::string("frame_number"); 				\
    case ACQUISITION_TIME: 			return std::string("acquisition_time"); 			\
    case FRAME_PERIOD: 				return std::string("frame_period"); 				\
    case DELAY_AFTER_TRIGGER: 		return std::string("delay_after_trigger"); 		\
    case GATES_NUMBER: 				return std::string("gates_number"); 				\
    case CYCLES_NUMBER: 			return std::string("cycles_number"); 			\
    case ACTUAL_TIME: 				return std::string("actual_time"); 				\
    case MEASUREMENT_TIME: 			return std::string("measurement_time"); 			\
    case PROGRESS: 					return std::string("progress"); 					\
    case MEASUREMENTS_NUMBER: 		return std::string("measurements_number"); 		\
    case FRAMES_FROM_START: 		return std::string("frames_from_start"); 		\
    case FRAMES_FROM_START_PG: 		return std::string("frames_from_start_pg"); 		\
    case SAMPLES_JCTB: 				return std::string("samples_jctb"); 				\
    case SUBFRAME_ACQUISITION_TIME:	return std::string("subframe_acquisition_time");	\
    case SUBFRAME_DEADTIME:			return std::string("subframe_deadtime");			\
    case STORAGE_CELL_NUMBER:       return std::string("storage_cell_number");       \
    default:       					return std::string("unknown");					\
    }};


  /**
     @short returns adc index from std::string
     \param s can be temp_fpga, temp_fpgaext, temp_10ge, temp_dcdc, temp_sodl, temp_sodr, temp_fpgafl, temp_fpgafr
     \returns  TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE, TEMPERATURE_DCDC, TEMPERATURE_SODL,
     TEMPERATURE_SODR, TEMPERATURE_FPGA2, TEMPERATURE_FPGA3, -1 when unknown mode
  */
  static int getADCIndex(std::string s){					\
	  if (s=="temp_fpga")	  	return TEMPERATURE_FPGA;	\
	  if (s=="temp_fpgaext")	return TEMPERATURE_FPGAEXT;	\
	  if (s=="temp_10ge")	  	return TEMPERATURE_10GE;	\
	  if (s=="temp_dcdc")	  	return TEMPERATURE_DCDC;	\
	  if (s=="temp_sodl")	  	return TEMPERATURE_SODL;	\
	  if (s=="temp_sodr")	  	return TEMPERATURE_SODR;	\
	  if (s=="temp_fpgafl")		return TEMPERATURE_FPGA2;	\
	  if (s=="temp_fpgafr")		return TEMPERATURE_FPGA3;	\
	  return -1;											\
  };														\


  /**
     @short returns dac index from std::string
     \param s can be vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr, vthreshold, vrf, vrs, vtr, vcall, vcp
     \returns E_Vcmp_ll, E_Vcmp_lr, E_Vcmp_rl, E_Vcmp_rr, THRESHOLD, E_Vrf, E_Vrs, E_Vtr, E_cal, E_Vcp , -1 when unknown mode
  */
  static int getDACIndex(std::string s){		\
	  if (s=="vcmp_ll")	  	return E_Vcmp_ll;	\
	  if (s=="vcmp_lr")	  	return E_Vcmp_lr;	\
	  if (s=="vcmp_rl")		return E_Vcmp_rl;	\
	  if (s=="vcmp_rr")	  	return E_Vcmp_rr;	\
	  if (s=="vthreshold")	return THRESHOLD;	\
	  if (s=="vrf")	  		return E_Vrf;		\
	  if (s=="vrs")	  		return E_Vrs;		\
	  if (s=="vtr")			return E_Vtr;		\
	  if (s=="vcall")		return E_cal;		\
	  if (s=="vcp")			return E_Vcp;		\
	  return -1;								\
  };											\

  /**
     @short returns receiver frame discard policy from std::string
     \param s can be nodiscard, discardempty, discardpartial
     \returns NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES, GET_FRAME_DISCARD_POLICY when unknown mode
  */
  static frameDiscardPolicy getReceiverFrameDiscardPolicy(std::string s){		\
	  if (s=="nodiscard")	  	return NO_DISCARD;				\
	  if (s=="discardempty")	return DISCARD_EMPTY_FRAMES;	\
	  if (s=="discardpartial")	return DISCARD_PARTIAL_FRAMES;	\
	  return GET_FRAME_DISCARD_POLICY;							\
  };															\

  /** returns std::string from frame discard policy
      \param f can be NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
      \returns std::string nodiscard, discardempty, discardpartial, unknown
  */
  static std::string getReceiverFrameDiscardPolicy(frameDiscardPolicy f){	\
    switch (f) {															\
    case NO_DISCARD: 				return std::string("nodiscard"); 		\
    case DISCARD_EMPTY_FRAMES: 		return std::string("discardempty"); 	\
    case DISCARD_PARTIAL_FRAMES: 	return std::string("discardpartial"); 	\
    default:       					return std::string("unknown");			\
    }};																		\


};


#endif

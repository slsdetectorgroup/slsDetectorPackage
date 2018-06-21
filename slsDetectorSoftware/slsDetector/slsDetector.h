#ifndef SLS_DETECTOR_H
#define SLS_DETECTOR_H

/**
 *
 * @short complete detector functionalities for a single module detector.
 * The slsDetector class takes care of the communication with the
 * detector and all kind actions related with a single detector controller
 * @author Anna Bergamaschi
 */

#include "slsDetectorUtils.h"
#include "energyConversion.h"
#include "angleConversionConstant.h"
#include "MySocketTCP.h"
#include "angleConversionConstant.h"

class multiSlsDetector;
class SharedMemory;
class receiverInterface;


#define SLS_SHMVERSION	0x180620
#define NMODMAXX 24
#define NMODMAXY 24
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16

class slsDetector : public slsDetectorUtils, public energyConversion {

private:
	/**
	 * @short structure allocated in shared memory to store detector settings for IPC and cache
	 */
	typedef  struct sharedSlsDetector {

		/* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

		/** shared memory version */
		int shmversion;

		/** END OF FIXED PATTERN -----------------------------------------------*/




		/** online flag - is set if the detector is connected, unset if socket
		 * connection is not possible  */
		int onlineFlag;

		/** stopped flag - is set if an acquisition error occurs or the detector
		 * is stopped manually. Is reset to 0 at the start of the acquisition */
		int stoppedFlag;

		/** is the hostname (or IP address) of the detector. needs to be set
		 * before starting the communication */
		char hostname[MAX_STR_LENGTH];

		/** is the port used for control functions */
		int controlPort;

		/** is the port used to stop the acquisition */
		int stopPort;

		/** detector type  \ see :: detectorType*/
		detectorType myDetectorType;

		/** path of the trimbits/settings files */
		char settingsDir[MAX_STR_LENGTH];

		/** path of the calibration files */
		char calDir[MAX_STR_LENGTH];

		/** number of energies at which the detector has been trimmed */
		int nTrimEn;

		/** list of the energies at which the detector has been trimmed  */
		int trimEnergies[100];

		/** indicator for the acquisition progress - set to 0 at the beginning
		 * of the acquisition and incremented when each frame is processed */
		int progressIndex;

		/** total number of frames to be acquired */
		int totalProgress;

		/** path of the output files */
		char filePath[MAX_STR_LENGTH];

		/** number of installed modules of the detector (x and y directions) */
		int nMod[2];

		/**  number of modules ( nMod[X]*nMod[Y]) \see nMod */
		int nMods;

		/** maximum number of modules of the detector (x and y directions) */
		int nModMax[2];

		/**  maximum number of modules (nModMax[X]*nModMax[Y]) \see nModMax */
		int nModsMax;

		/**  number of channels per chip */
		int nChans;

		/**  number of channels per chip in one direction */
		int nChan[2];

		/**  number of chips per module*/
		int nChips;

		/**  number of chips per module in one direction */
		int nChip[2];

		/**  number of dacs per module*/
		int nDacs;

		/** number of adcs per module */
		int nAdcs;

		/**  number of extra gain values*/
		int nGain;

		/** number of extra offset values */
		int nOffset;

		/** dynamic range of the detector data */
		int dynamicRange;

		/**  size of the data that are transfered from the detector */
		int dataBytes;

		/** corrections  to be applied to the data \see ::correctionFlags */
		int correctionMask;

		/** threaded processing flag
		 * (i.e. if data are processed in a separate thread)  */
		int threadedProcessing;

		/** dead time (in ns) for rate corrections */
		double tDead;

		/** directory where the flat field files are stored */
		char flatFieldDir[MAX_STR_LENGTH];

		/** file used for flat field corrections */
		char flatFieldFile[MAX_STR_LENGTH];

		/** number of bad channels from bad channel list */
		int nBadChans;

		/** file with the bad channels */
		char badChanFile[MAX_STR_LENGTH];

		/** list of bad channels */
		int badChansList[MAX_BADCHANS];

		/** number of bad channels from flat field
		 * i.e. channels which read 0 in the flat field file */
		int nBadFF;

		/** list of bad channels from flat field
		 * i.e. channels which read 0 in the flat field file */
		int badFFList[MAX_BADCHANS];

		/** file with the angular conversion factors */
		char angConvFile[MAX_STR_LENGTH];

		/** array of angular conversion constants for each module
		 * \see ::angleConversionConstant */
		angleConversionConstant angOff[MAXMODS];

		/** angular direction (1 if it corresponds to the encoder direction
		 * i.e. channel 0 is 0, maxchan is positive high angle, 0 otherwise  */
		int angDirection;

		/** beamline fine offset (of the order of mdeg,
		 * might be adjusted for each measurements)  */
		double fineOffset;

		/** beamline offset (might be a few degrees beacuse of encoder offset -
		 * normally it is kept fixed for a long period of time)  */
		double globalOffset;

		/** number of positions at which the detector should acquire  */
		int numberOfPositions;

		/** list of encoder positions at which the detector should acquire */
		double detPositions[MAXPOS];

		/** bin size for data merging */
		double binSize;

		/** add encoder value flag (i.e. wether the detector is
		 * moving - 1 - or stationary - 0) */
		int moveFlag;

		/** number of rois defined */
		int nROI;

		/** list of rois */
		ROI roiLimits[MAX_ROIS];

		/** readout flags */
		readOutFlags roFlags;

		/** name root of the output files */
		char settingsFile[MAX_STR_LENGTH];

		/** detector settings (standard, fast, etc.) */
		detectorSettings currentSettings;

		/** detector threshold (eV) */
		int currentThresholdEV;

		/** timer values */
		int64_t timerValue[MAX_TIMERS];

		/** action mask */
		int actionMask;

		/** action script */
		mystring actionScript[MAX_ACTIONS];

		/** action parameter */
		mystring actionParameter[MAX_ACTIONS];

		/** scan mode */
		int scanMode[MAX_SCAN_LEVELS];

		/** scan script */
		mystring scanScript[MAX_SCAN_LEVELS];

		/** scan parameter */
		mystring scanParameter[MAX_SCAN_LEVELS];

		/** n scan steps */
		int nScanSteps[MAX_SCAN_LEVELS];

		/** scan steps */
		mysteps scanSteps[MAX_SCAN_LEVELS];

		/** scan precision */
		int scanPrecision[MAX_SCAN_LEVELS];

		/** memory offsets for the flat field coefficients */
		int ffoff;

		/** memory offsets for the flat filed coefficient errors */
		int fferroff;

		/** memory offsets for the module structures  */
		int modoff;

		/** memory offsets for the dac arrays */
		int dacoff;

		/** memory offsets for the adc arrays */
		int adcoff;

		/** memory offsets for the chip register arrays */
		int chipoff;

		/** memory offsets for the channel register arrays  -trimbits*/
		int chanoff;

		/** memory offsets for the gain register arrays */
		int gainoff;

		/** memory offsets for the offset register arrays  -trimbits*/
		int offsetoff;

		/** ip address/hostname of the receiver for client control via TCP */
		char receiver_hostname[MAX_STR_LENGTH];

		/** is the TCP port used to communicate between client and the receiver */
		int receiverTCPPort;

		/** is the UDP port used to send data from detector to receiver */
		int receiverUDPPort;

		/** is the port used to communicate between second half module of
		 * Eiger detector and the receiver*/
		int receiverUDPPort2;

		/** ip address of the receiver for the detector to send packets to**/
		char receiverUDPIP[MAX_STR_LENGTH];

		/** mac address of receiver for the detector to send packets to **/
		char receiverUDPMAC[MAX_STR_LENGTH];

		/**  mac address of the detector **/
		char detectorMAC[MAX_STR_LENGTH];

		/**  ip address of the detector **/
		char detectorIP[MAX_STR_LENGTH];

		/** online flag - is set if the receiver is connected,
		 * unset if socket connection is not possible  */
		int receiverOnlineFlag;

		/** 10 Gbe enable*/
		int tenGigaEnable;

		/** flipped data across x or y axis */
		int flippedData[2];

		/** tcp port from gui/different process to receiver (only data) */
		int zmqport;

		/** tcp port from receiver to gui/different process (only data) */
		int receiver_zmqport;

		/** data streaming (up stream) enable in receiver */
		bool receiver_upstream;

		/* Receiver read frequency */
		int receiver_read_freq;

		/**  zmq tcp src ip address in client (only data) **/
		char zmqip[MAX_STR_LENGTH];

		/**  zmq tcp src ip address in receiver (only data) **/
		char receiver_zmqip[MAX_STR_LENGTH];

		/** gap pixels enable */
		int gappixels;

		/** gap pixels in each direction */
		int nGappixels[2];

		/** data bytes including gap pixels */
		int dataBytesInclGapPixels;

		/** additional json header */
		char receiver_additionalJsonHeader[MAX_STR_LENGTH];

		/** frames per file in receiver */
		int receiver_framesPerFile;

		/** detector control server software API version */
		int64_t detectorControlAPIVersion;

		/** detector stop server software API version */
		int64_t detectorStopAPIVersion;

		/** receiver server software API version */
		int64_t receiverAPIVersion;

	} sharedSlsDetector;





public:

	using slsDetectorUtils::getDetectorType;
	using postProcessing::flatFieldCorrect;
	using postProcessing::rateCorrect;
	using postProcessing::setBadChannelCorrection;
	using angularConversion::readAngularConversion;
	using angularConversion::writeAngularConversion;
	using slsDetectorUtils::getAngularConversion;


	/**
	 * Constructor called when creating new shared memory
	 * @param type detector type
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 * @param m multiSlsDetector reference
	 */
	slsDetector(detectorType type, int multiId = 0, int id = 0, bool verify = true, MultiDet* m = NULL);

	/**
	 * Constructor called when opening existing shared memory
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 * @param m multiSlsDetector reference
	 */
	slsDetector(int multiId = 0, int id = 0, bool verify = true, MultiDet* m = NULL);

	/**
	 * Destructor
	 */
	virtual ~slsDetector();

	/**
	 * returns false. Used when reference is slsDetectorUtils and to determine
	 * if command can be implemented as slsDetector/multiSlsDetector object/
	 */
	bool isMultiSlsDetectorClass();

	/**
	 * Decode data from the detector converting them to an array of doubles,
	 * one for each channel (Mythen only)
	 * @param datain data from the detector
	 * @param nn size of datain array
	 * @param fdata double array of decoded data
	 * @returns pointer to a double array with a data per channel
	 */
	double* decodeData(int *datain, int &nn, double *fdata=NULL);

	/**
	 * Clears error mask and also the bit in parent det multi error mask
	 * @returns error mask
	 */
	int64_t clearAllErrorMask();

	/**
	 * Set acquiring flag in shared memory
	 * @param b acquiring flag
	 */
	void setAcquiringFlag(bool b=false);

	/**
	 * Get acquiring flag from shared memory
	 * @returns acquiring flag
	 */
	bool getAcquiringFlag();

	/**
	 * Check if acquiring flag is set, set error if set
	 * @returns FAIL if not ready, OK if ready
	 */
	bool isAcquireReady();

	/**
	 * Check version compatibility with detector/receiver software
	 * (if hostname/rx_hostname has been set/ sockets created)
	 * @param p port type control port or receiver port
	 * @returns FAIL for incompatibility, OK for compatibility
	 */
	int checkVersionCompatibility(portType t);

	/**
	 * Get ID or version numbers
	 * @param mode version type
	 * @param imod module number in entire module list (gets decoded) (-1 for all)
	 * @returns Id or version number of that type
	 */
	int64_t getId(idMode mode, int imod=0);

	/**
	 * Free shared memory without creating objects
	 * If this is called, must take care to update
	 * multiSlsDetectors thisMultiDetector->numberofDetectors
	 * avoiding creating the constructor classes and mapping
	 * @param multiId multi detector Id
	 * @param slsId slsDetectorId or position of slsDetector in detectors list
	 */
	static void freeSharedMemory(int multiId, int slsId);

	/**
	 * Free shared memory and delete shared memory structure
	 * occupied by the sharedSlsDetector structure
	 * If this is called, must take care to update
	 * multiSlsDetectors thisMultiDetector->numberofDetectors
	 * and not use this object again (delete and start anew)
	 */
	void freeSharedMemory();



	/** returns the detector hostname \sa sharedSlsDetector  */
	string setHostname(const char *name);


	/** returns the detector hostname \sa sharedSlsDetector  */
	string getHostname();


	/**
     returns the detector type from hostname and controlport
     \param
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)
	 */
	static detectorType getDetectorType(const char *name, int cport=DEFAULT_PORTNO);




	/**
     sets/gets detector type
     normally  the detector knows what type of detector it is
     \param type is the detector type (defaults to GET_DETECTOR_TYPE)
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH, -1 command failed)
	 */
	int setDetectorType(detectorType type=GET_DETECTOR_TYPE);

	/**
      sets/gets detector type
      normally  the detector knows what type of detector it is
      \param type is the detector type ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd", "Mönch")
      \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH, -1 command failed)
	 */
	int setDetectorType(string type);

	/**
      gets detector type
      normally  the detector knows what type of detector it is
      \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH,-1 command failed)
	 */
	detectorType getDetectorsType(int pos=-1);

	detectorType setDetectorsType(detectorType type=GET_DETECTOR_TYPE, int pos=-1){return getDetectorsType(pos);};

	string sgetDetectorsType(int pos=-1){return getDetectorType(getDetectorsType(pos));};

	string ssetDetectorsType(detectorType type=GET_DETECTOR_TYPE, int pos=-1){return getDetectorType(getDetectorsType(pos));};
	string ssetDetectorsType(string t, int pos=-1){return getDetectorType(getDetectorsType(pos));}

	string getDetectorType(){return sgetDetectorsType();};




	/**
     returns the detector type from hostname and controlport
     \param
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)
	 */
	static detectorType getDetectorType(int id);




	/** Returns the number of  modules (without connecting to the detector) */
	int getNMods(){return thisDetector->nMods;}; //

	/** Returns the number of  modules in direction d (without connecting to the detector) */
	int getNMod(dimension d){return thisDetector->nMod[d];}; //


	/** Returns the number of  modules (without connecting to the detector) */
	int getMaxMods(){return thisDetector->nModsMax;}; //

	/** Returns the max number of  modules in direction d (without connecting to the detector) */
	int getNMaxMod(dimension d){return thisDetector->nModMax[d];}; //

	/**
      get the maximum size of the detector
      \param d dimension
      \returns maximum number of modules that can be installed in direction d
	 */
	int getMaxNumberOfModules(dimension d=X); //


	/**
      set/get the size of the detector
      \param n number of modules
      \param d dimension
      \returns current number of modules in direction d
	 */
	int setNumberOfModules(int n=GET_FLAG, dimension d=X); // if n=GET_FLAG returns the number of installed modules


	int getChansPerMod(int imod=0){return thisDetector->nChans*thisDetector->nChips;};

	int getChansPerMod( dimension d,int imod=0){return thisDetector->nChan[d]*thisDetector->nChip[d];};


	int getTotalNumberOfChannels();
	//{return thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;};

	int getTotalNumberOfChannels(dimension d);
	//{return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nMod[d];};

	int getTotalNumberOfChannelsInclGapPixels(dimension d);


	int getMaxNumberOfChannels();//{return thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;};

	int getMaxNumberOfChannels(dimension d);//{return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nModMax[d];};

	int getMaxNumberOfChannelsInclGapPixels(dimension d);



	/** Returns the number of channels per chip (without connecting to the detector) */
	int getNChans(){return thisDetector->nChans;}; //

	/** Returns the number of channels per chip (without connecting to the detector) in one direction */
	int getNChans(dimension d){return thisDetector->nChan[d];}; //

	/** Returns the number of chips per module (without connecting to the detector) */
	int getNChips(){return thisDetector->nChips;}; //

	/** Returns the number of chips per module (without connecting to the detector) */
	int getNChips(dimension d){return thisDetector->nChip[d];}; //

	int setOnline(int const online=GET_ONLINE_FLAG);

	string checkOnline();

	/**
      configure the socket communication and initializes the socket instances

      \param name hostname - if "" the current hostname is used
      \param control_port port for control commands - if -1 the current is used
      \param stop_port port for stop command - if -1 the current is used

      \returns OK is connection succeded, FAIL otherwise
      \sa sharedSlsDetector
	 */
	int setTCPSocket(string const name="", int const control_port=-1, int const stop_port=-1);


	/**
     changes/gets the port number
     \param type port type
     \param num new port number (-1 gets)
     \returns actual port number
	 */
	int setPort(portType type, int num=-1);

	/** returns the detector control port  \sa sharedSlsDetector */
	int getControlPort() {return  thisDetector->controlPort;};
	/** returns the detector stop  port  \sa sharedSlsDetector */
	int getStopPort() {return thisDetector->stopPort;};
	/** returns the receiver port  \sa sharedSlsDetector */
	int getReceiverPort() {return thisDetector->receiverTCPPort;};



	/** Locks/Unlocks the connection to the server
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the server
	 */
	int lockServer(int lock=-1);

	/**
      Returns the IP of the last client connecting to the detector
	 */
	string getLastClientIP();
	/**
      turns off server
	 */
	int exitServer();

	/**
     executes a system command on the server
     e.g. mount an nfs disk, reboot and returns answer etc.
     \param cmd is the command to be executed
     \param answer is the answer from the detector
     \returns OK or FAIL depending on the command outcome
	 */
	int execCommand(string cmd, string answer);

	/**
  updates the shared memory receiving the data from the detector (without asking and closing the connection
  /returns OK
	 */
	int updateDetectorNoWait();

	/**
  updates the shared memory receiving the data from the detector
  /returns OK

	 */
	int updateDetector();


	/** connect to the control port */
	int connectControl();
	/** disconnect from the control port */
	int disconnectControl();

	/** connect to the receiver port */
	int connectData();
	/** disconnect from the receiver port */
	int disconnectData();

	/** connect to the stop port */
	int connectStop();
	/** disconnect from the stop port */
	int disconnectStop();

	/** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

	 */
	sls_detector_module*  createModule(){return createModule(thisDetector->myDetectorType);};


	/** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

	 */
	sls_detector_module*  createModule(detectorType myDetectorType);

	/** frees the memory for a sls_detector_module structure
      \param myMod the pointer to the memory to be freed

	 */

	void deleteModule(sls_detector_module *myMod);

	/**
     Reads the configuration file fname
     \param fname file name
     \returns OK or FAIL
	 */
	int readConfigurationFile(string const fname);


	int readConfigurationFile(ifstream &infile);


	/**

  Writes the configuration file fname
  \param fname file name
  \returns OK or FAIL

	 */
	int writeConfigurationFile(string const fname);
	int writeConfigurationFile(ofstream &outfile, int id=-1);


	/**
     returns currently the loaded trimfile/settingsfile name
	 */
	string getSettingsFile(){\
		string s(thisDetector->settingsFile); \
		if (s.length()>6) {\
			if (s.substr(s.length()-6,3)==string(".sn") && s.substr(s.length()-3)!=string("xxx") ) \
			return s.substr(0,s.length()-6);			\
		}									\
		return string(thisDetector->settingsFile);\
	};


	/**
     writes a trim/settings file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \param iodelay io delay (detector specific)
     \param tau tau (detector specific)
     \returns OK or FAIL if the file could not be written
     \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeSettingsFile(string, int)
	 */
	using energyConversion::writeSettingsFile;
	int writeSettingsFile(string fname, int imod, int iodelay, int tau);



	/**
     get detector settings
     \param imod module number (-1 all)
     \returns current settings  detectorSettings sendSettingsOnly(detectorSettings isettings);
	 */
	detectorSettings getSettings(int imod=-1);

	/**
     set detector settings
     \param isettings  settings
     \param imod module number (-1 all)
     \returns current settings

     in this function trimbits/settings and calibration files are searched in the settingsDir and calDir directories and the detector is initialized
	 */
	detectorSettings setSettings(detectorSettings isettings, int imod=-1);



	/** returns the detector trimbit/settings directory  \sa sharedSlsDetector */
	std::string getSettingsDir() {return std::string(thisDetector->settingsDir);};
	/** sets the detector trimbit/settings directory  \sa sharedSlsDetector */
	std::string setSettingsDir(string s) {sprintf(thisDetector->settingsDir, s.c_str()); return thisDetector->settingsDir;};



	/**
     returns the location of the calibration files
     \sa  sharedSlsDetector
	 */
	std::string getCalDir() {return thisDetector->calDir;};
	/**
     sets the location of the calibration files
     \sa  sharedSlsDetector
	 */
	std::string setCalDir(string s) {sprintf(thisDetector->calDir, s.c_str()); return thisDetector->calDir;};

	/** loads the modules settings/trimbits reading from a file
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
	 */
	int loadSettingsFile(string fname, int imod=-1);


	/** saves the modules settings/trimbits writing to  a file
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
	 */
	int saveSettingsFile(string fname, int imod=-1);


	/** loads the modules calibration data  reading from a file
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
	 */
	int loadCalibrationFile(string fname, int imod=-1);


	/** saves the modules calibration data  writing to  a file
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
	 */
	int saveCalibrationFile(string fname, int imod=-1);


	/** returns if the detector is Master, slave or nothing
      \param flag can be GET_MASTER, NO_MASTER, IS_MASTER, IS_SLAVE
      \returns master flag of the detector
	 */
	masterFlags  setMaster(masterFlags flag);


	/**
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode can be GET_SYNCHRONIZATION_MODE, NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns current syncronization mode
	 */
	synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE);

	/** calcualtes the total number of steps of the acquisition.
      called when number of frames, number of cycles, number of positions and scan steps change
	 */
	int setTotalProgress();

	/** returns the current progress in % */
	double getCurrentProgress();

	runStatus  getRunStatus();

	/**
     prepares detector for acquisition
     \returns OK/FAIL
	 */
	int prepareAcquisition();

	/**
     prepares detector for acquisition
     \returns OK/FAIL
	 */
	int cleanupAcquisition();

	/**
     start detector acquisition
     \returns OK/FAIL
	 */
	int startAcquisition();

	/**
     stop detector acquisition
     \returns OK/FAIL
	 */
	int stopAcquisition();

	/**
     start readout (without exposure or interrupting exposure)
     \returns OK/FAIL
	 */
	int startReadOut();

	/**
     start detector acquisition and read all data putting them a data queue
     \returns pointer to the front of the data queue
     \sa startAndReadAllNoWait getDataFromDetector dataQueue
	 */
	int* startAndReadAll();


	/**
     start detector acquisition and read out, but does not read data from socket  leaving socket opened
     \returns OK or FAIL

	 */
	int startAndReadAllNoWait();

	/**
      Receives a data frame from the detector socket
      \returns pointer to the data (or NULL if failed)

	 */
	int* getDataFromDetector(int *retval=NULL);

	/**
     asks and  receives a data frame from the detector and puts it in the data queue
     \returns pointer to the data or NULL.
     \sa getDataFromDetector
	 */
	int* readFrame();

	/**
     asks and  receives all data  from the detector  and puts them in a data queue
     \returns pointer to the front of the queue  or NULL.
     \sa getDataFromDetector  dataQueue
	 */
	int* readAll();

	/**
     asks and  receives all data  from the detector  and leaves the socket opened
     \returns OK or FAIL
	 */
	int readAllNoWait();

	/**
       configures mac for gotthard, moench readout
     \returns OK or FAIL
	 */
	int configureMAC();

	/**
     get threshold energy
     \param imod module number (-1 all)
     \returns current threshold value for imod in ev (-1 failed)
	 */
	int getThresholdEnergy(int imod=-1);


	/**
     set threshold energy
     \param e_eV threshold in eV
     \param imod module number (-1 all)
     \param isettings ev. change settings
     \param tb 1 to include trimbits, 0 to exclude
     \returns current threshold value for imod in ev (-1 failed)
	 */
	int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS, int tb=1);

	/**
     set threshold energy
     \param e_eV threshold in eV
     \param isettings ev. change settings
     \param tb 1 to include trimbits, 0 to exclude
     \returns OK if successful, else FAIL
	 */
	int setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings, int tb=1);


	/**
     send detector settings only (set only for Jungfrau, Gotthard, Moench, get for all)
     \param isettings  settings
     \param imod module number (-1 all)
     \returns current settings

     in this function only the settings is sent to the detector, where it will initialize all the dacs already stored in the detector server.
	 */
	detectorSettings sendSettingsOnly(detectorSettings isettings, int imod=-1);



	/**
      set/get timer value
      \param index timer index
      \param t time in ns or number of...(e.g. frames, gates, probes)
      \returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t setTimer(timerIndex index, int64_t t=-1);

	/**
      get current timer value
      \param index timer index
      \returns elapsed time value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t getTimeLeft(timerIndex index);

	/** sets/gets the value of important readout speed parameters
      \param sp is the parameter to be set/get
      \param value is the value to be set, if -1 get value
      \returns current value for the specified parameter
      \sa speedVariable
	 */
	int setSpeed(speedVariable sp, int value=-1);

	// Flags
	/**
      set/get dynamic range
      \param n dynamic range (-1 get)
      \returns current dynamic range
      updates the size of the data expected from the detector
      \sa sharedSlsDetector
	 */
	int setDynamicRange(int n=-1);

	/**
      set/get dynamic range
      \returns number of bytes sent by the detector
      \sa sharedSlsDetector
	 */
	int getDataBytes(){return thisDetector->dataBytes;};

	/**
	 * returns number of bytes sent by detector including gap pixels
	 * \sa sharedSlsDetector
	 */
	int getDataBytesInclGapPixels(){return thisDetector->dataBytesInclGapPixels;};

	/**
     set dacs value
     \param val value (in V)
     \param index DAC index
     \param mV 0 in dac units or 1 in mV
     \param imod module number (if -1 alla modules)
     \returns current DAC value
	 */
	dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1);

	/**
     set dacs value
     \param index ADC index
     \param imod module number
     \returns current ADC value  (temperature for eiger and jungfrau in millidegrees)
	 */
	dacs_t getADC(dacIndex index, int imod=0);
	/**
      set/get the external communication mode

      obsolete \sa setExternalSignalFlags
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
	 */
	externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE);

	/**
      set/get the use of an external signal 
      \param pol meaning of the signal \sa externalSignalFlag
      \param signalIndex index of the signal
      \returns current meaning of signal signalIndex
	 */
	externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0);

	/**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
	 */
	int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS);

	/**
      write  register
      \param addr address
      \val value
      \returns current register value

	 */
	uint32_t writeRegister(uint32_t addr, uint32_t val);

	/**
      read  register
      \param addr address
      \returns current register value

	 */
	uint32_t readRegister(uint32_t addr);

	/**
      sets a bit in a register
      \param addr address
      \param n nth bit ranging from 0 to 31
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
	 */
	uint32_t setBit(uint32_t addr, int n);


	/**
      clear a bit in a register
      \param addr address
      \param n nth bit ranging from 0 to 31
      \returns current register value

      DO NOT USE!!! ONLY EXPERT USER!!!
	 */
	uint32_t clearBit(uint32_t addr, int n);

	/**
     sets the network parameters
     must restart streaming in client/receiver if to do with zmq after calling this function
     \param i network parameter type
     \param s value to be set
     \returns parameter

	 */
	string setNetworkParameter(networkParameter index, string value);

	/**
     gets the network parameters
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \returns parameter

	 */
	string getNetworkParameter(networkParameter index);

	/**
     Digital test of the modules
     \param mode test mode
     \param imod module number for chip test or module firmware test
     \returns OK or error mask
	 */
	int digitalTest(digitalTestMode mode, int imod=0);


	/**
     execute trimming
     \param mode trim mode
     \param par1 if noise, beam or fixed setting trimming it is count limit, if improve maximum number of iterations
     \param par2 if noise or beam nsigma, if improve par2!=means vthreshold will be optimized, if fixed settings par2<0 trimwith median, par2>=0 trim with level
     \param imod module number (-1 all)
     \returns OK or FAIl (FAIL also if some channel are 0 or 63
	 */
	int executeTrimming(trimMode mode, int par1, int par2, int imod=-1);
	/**
     Loads dark image or gain image from a file and sends it to the detector
     \param index is 0 for dark image and 1 for gain image
     \param fname file name to load data from

	 */
	int loadImageToDetector(imageType index,string const fname);

	/**
     Called from loadImageToDetector to send the image to detector
     \param index is 0 for dark image and 1 for gain image
     \param arg image

	 */
	int sendImageToDetector(imageType index,short int imageVals[]);

	/**
     writes the counter memory block from the detector
     \param startACQ is 1 to start acquisition after reading counter
     \param fname file name to load data from
     \returns OK or FAIL
	 */
	int writeCounterBlockFile(string const fname,int startACQ=0);


	/**
     gets counter memory block in detector
     \param startACQ is 1 to start acquisition after reading counter
     \param arg counter memory block from detector
     \returns OK or FAIL
	 */
	int getCounterBlock(short int arg[],int startACQ=0);


	/**
     Resets counter in detector
     \param startACQ is 1 to start acquisition after resetting counter
     \returns OK or FAIL
	 */
	int resetCounterBlock(int startACQ=0);

	/** set/get counter bit in detector
	 * @param i is -1 to get, 0 to reset and any other value to set the counter bit
     /returns the counter bit in detector
	 */
	int setCounterBit(int i = -1);


	/**
      set roi
       \param n number of rois
       \param roiLimits array of roi
       \returns success or failure
	 */
	int setROI(int n=-1,ROI roiLimits[]=NULL);

	/**
    	  get roi from each detector and convert it to the multi detector scale
    	  \param n number of roi
    	  \returns an array of multidetector's rois
	 */
	slsDetectorDefs::ROI* getROI(int &n);


	/** Returns number of rois */
	int getNRoi(){return thisDetector->nROI;};


	int sendROI(int n=-1,ROI roiLimits[]=NULL);


	/**
      write  register 
      \param addr address
      \val value
      \returns current register value

	 */
	int writeAdcRegister(int addr, int val);


	/**  @short activates the detector (detector specific)
       \param enable can be: -1 returns wether the detector is in active (1) or inactive (0) state
       \returns 0 (inactive) or 1 (active)
	 */
	int activate(int const enable=GET_ONLINE_FLAG);


	/** returns the enable if data will be flipped across x or y axis
	 *  \param d axis across which data is flipped
	 *  returns 1 or 0
	 */
	int getFlippedData(dimension d=X){return thisDetector->flippedData[d];};

	/** sets the enable which determines if data will be flipped across x or y axis
	 *  \param d axis across which data is flipped
	 *  \param value 0 or 1 to reset/set or -1 to get value
	 *  \return enable flipped data across x or y axis
	 */
	int setFlippedData(dimension d=X, int value=-1);
	/** sets all the trimbits to a particular value
      \param val trimbit value
      \param imod module number, -1 means all modules
      \returns OK or FAIL
	 */
	int setAllTrimbits(int val, int imod=-1);

	/**
	 * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. 4 bit mode gap pixels only in gui call back
	 * @param val 1 sets, 0 unsets, -1 gets
	 * @return gap pixel enable or -1 for error
	 */
	int enableGapPixels(int val=-1);
	/** sets the number of trim energies and their value  \sa sharedSlsDetector
      \param nen number of energies
      \param en array of energies
      \returns number of trim energies

      unused!

      \sa  sharedSlsDetector
	 */
	int setTrimEn(int nen, int *en=NULL) {if (en) {for (int ien=0; ien<nen; ien++) thisDetector->trimEnergies[ien]=en[ien]; thisDetector->nTrimEn=nen;} return (thisDetector->nTrimEn);};


	/** returns the number of trim energies and their value  \sa sharedSlsDetector
      \param point to the array that will contain the trim energies (in ev)
      \returns number of trim energies


      unused!

      \sa  sharedSlsDetector
	 */
	int getTrimEn(int *en=NULL) {if (en) {for (int ien=0; ien<thisDetector->nTrimEn; ien++) en[ien]=thisDetector->trimEnergies[ien];} return (thisDetector->nTrimEn);};

	/**
     Pulse Pixel
     \param n is number of times to pulse
     \param x is x coordinate
     \param y is y coordinate
     \returns OK or FAIL
	 */
	int pulsePixel(int n=0,int x=0,int y=0);

	/**
     Pulse Pixel and move by a relative value
     \param n is number of times to pulse
     \param x is relative x value
     \param y is relative y value
     \returns OK or FAIL
	 */
	int pulsePixelNMove(int n=0,int x=0,int y=0);

	/**
     Pulse Chip
     \param n is number of times to pulse
     \returns OK or FAIL
	 */
	int pulseChip(int n=0);

	/**
     set/gets threshold temperature (Jungfrau only)
     \param val value in millidegrees, -1 gets
     \param imod module number, -1 is all
     \returns threshold temperature in millidegrees
	 */
	int setThresholdTemperature(int val=-1, int imod=-1);

	/**
     enables/disables temperature control (Jungfrau only)
     \param val value, -1 gets
     \param imod module number, -1 is all
     \returns temperature control enable
	 */
	int setTemperatureControl(int val=-1, int imod=-1);

	/**
     Resets/ gets over-temperature event (Jungfrau only)
     \param val value, -1 gets
     \param imod module number, -1 is all
     \returns over-temperature event
	 */
	int setTemperatureEvent(int val=-1, int imod=-1);

	/**
	 * set storage cell that stores first acquisition of the series (Jungfrau only)
	 * \param value storage cell index. Value can be 0 to 15. (-1 gets)
	 * \returns the storage cell that stores the first acquisition of the series
	 */
	int setStoragecellStart(int pos=-1);


	/** programs FPGA with pof file
      \param fname file name
      \returns OK or FAIL
	 */
	int programFPGA(string fname);

	/** resets FPGA
      \returns OK or FAIL
	 */
	int resetFPGA();

	/** power on/off the chip
     \param ival on is 1, off is 0, -1 to get
      \returns OK or FAIL
	 */
	int powerChip(int ival= -1);

	/** automatic comparator disable for Jungfrau only
     \param ival on is 1, off is 0, -1 to get
      \returns OK or FAIL
	 */
	int setAutoComparatorDisableMode(int ival= -1);


	/**
     gets the trimbits from shared memory *chanRegs
     \param retval is the array with the trimbits
     \param fromDetector is true if the trimbits shared memory have to be uploaded from detector
     \returns the total number of channels for the detector
     \sa ::sls_detector_module
	 */
	int getChanRegs(double* retval,bool fromDetector);

	/**
      configure module
      \param imod module number (-1 all)
      \returns current register value
      \sa ::sls_detector_module
	 */
	int setModule(int reg, int imod=-1);
	//virtual int setModule(int reg, int imod=-1);

	/**
      configure chip
      \param module module to be set - must contain correct module number and also channel and chip registers
      \param iodelay iodelay (detector specific)
      \param tau tau (detector specific)
      \param e_eV threashold in eV (detector specific)
      \param gainval pointer to extra gain values
      \param offsetval pointer to extra offset values
      \param tb 1 to include trimbits, 0 to exclude (used for eiger)
      \returns current register value
      \sa ::sls_detector_module
	 */
	int setModule(sls_detector_module module, int iodelay, int tau, int e_eV, int* gainval=0, int* offsetval=0, int tb=1);
	//virtual int setModule(sls_detector_module module);

	/**
     get module
     \param imod module number
     \returns pointer to module structure (which has bee created and must then be deleted)
	 */
	sls_detector_module *getModule(int imod);

	/**
     configure channel
     \param reg channel register
     \param ichan channel number (-1 all)
     \param ichip chip number (-1 all)
     \param imod module number (-1 all)
     \returns current register value
     \sa ::sls_detector_channel
	 */
	int setChannel(int64_t reg, int ichan=-1, int ichip=-1, int imod=-1);

	/**
     configure channel
     \param chan channel to be set - must contain correct channel, module and chip number
     \returns current register value
	 */
	int setChannel(sls_detector_channel chan);

	/**
     get channel
     \param ichan channel number 
     \param ichip chip number
     \param imod module number
     \returns current channel structure for channel
	 */
	sls_detector_channel getChannel(int ichan, int ichip, int imod);

	/**
      configure chip
      \param reg chip register
      \param ichip chip number (-1 all)
      \param imod module number (-1 all)
      \returns current register value
      \sa ::sls_detector_chip
	 */
	int setChip(int reg, int ichip=-1, int imod=-1);

	/**
      configure chip
      \param chip chip to be set - must contain correct module and chip number and also channel registers
      \returns current register value
      \sa ::sls_detector_chip
	 */
	int setChip(sls_detector_chip chip);

	/**
     get chip
     \param ichip chip number
     \param imod module number
     \returns current chip structure for channel

     \bug probably does not return corretly!
	 */
	sls_detector_chip getChip(int ichip, int imod);

	int getMoveFlag(int imod){if (moveFlag) return *moveFlag; else return 1;};

	int fillModuleMask(int *mM);

	/** Starts acquisition, calibrates pedestal and writes to fpga
     /returns number of frames
	 */
	int calibratePedestal(int frames = 0);

	/**
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
	 */
	int setRateCorrection(double t=0);


	/**
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
	 */
	int getRateCorrection(double &t);


	/**
      get rate correction tau
      \returns 0 if rate correction disabled, otherwise the tau used for the correction
	 */
	double getRateCorrectionTau();
	/**
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
	 */
	int getRateCorrection();


	/**
      rate correct data
      \param datain data array
      \param errin error array on data (if NULL will default to sqrt(datain)
      \param dataout array of corrected data
      \param errout error on corrected data (if not NULL)
      \returns 0
	 */
	int rateCorrect(double* datain, double *errin, double* dataout, double *errout);


	/**
      set flat field corrections
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
	 */
	int setFlatFieldCorrection(string fname="");

	/**
      set flat field corrections
      \param corr if !=NULL the flat field corrections will be filled with corr (NULL usets ff corrections)
      \param ecorr if !=NULL the flat field correction errors will be filled with ecorr (1 otherwise)
      \returns 0 if ff correction disabled, >0 otherwise
	 */
	int setFlatFieldCorrection(double *corr, double *ecorr=NULL);


	/**
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
	 */
	int getFlatFieldCorrection(double *corr=NULL, double *ecorr=NULL);

	/**
      flat field correct data
      \param datain data array
      \param errin error array on data (if NULL will default to sqrt(datain)
      \param dataout array of corrected data
      \param errout error on corrected data (if not NULL)
      \returns 0
	 */
	int flatFieldCorrect(double* datain, double *errin, double* dataout, double *errout);



	/**
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
	 */
	int setBadChannelCorrection(string fname="");

	/**
      set bad channels correction
      \param nch number of bad channels
      \param chs array of channels
      \param ff 0 if normal bad channels, 1 if ff bad channels
      \returns 0 if bad channel disabled, >0 otherwise
	 */
	int setBadChannelCorrection(int nch, int *chs, int ff=0);

	/**
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
	 */
	int getBadChannelCorrection(int *bad=NULL);

	/**

  reads an angular conversion file
  \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
	 */
	int readAngularConversionFile(string fname="");


	/**

  reads an angular conversion file
  \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
	 */
	int readAngularConversion(ifstream& ifs);



	/**
     Pure virtual function
     writes an angular conversion file
     \param fname file to be written
     \sa  angleConversionConstant mythenDetector::writeAngularConversion
	 */
	int writeAngularConversion(string fname="");



	/**
     Pure virtual function
     writes an angular conversion file
     \param fname file to be written
     \sa  angleConversionConstant mythenDetector::writeAngularConversion
	 */
	int writeAngularConversion(ofstream &ofs);


	/**
      pure virtual function
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::getAngularConversion
	 */
	int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL) ;

	angleConversionConstant *getAngularConversionPointer(int imod=0) {return &thisDetector->angOff[imod];};


	/**
     Prints receiver configuration
     \returns OK or FAIL
	 */
	int printReceiverConfiguration();


	/**
     calls setReceiverTCPSocket if online and sets the flag
	 */
	int setReceiverOnline(int const online=GET_ONLINE_FLAG);

	/**
     Checks if the receiver is really online
	 */
	string checkReceiverOnline();

	/**
     configure the socket communication and initializes the socket instances

     \param name receiver ip - if "" the current receiver hostname is used
     \param receiver_port port for receiving data - if -1 the current is used

     \returns OK is connection succeded, FAIL otherwise
     \sa sharedSlsDetector
	 */
	int setReceiverTCPSocket(string const name="", int const receiver_port=-1);



	/** Locks/Unlocks the connection to the receiver
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the receiver
	 */
	int lockReceiver(int lock=-1);

	/**
     Returns the IP of the last client connecting to the receiver
	 */
	string getReceiverLastClientIP();
	/**
      Turns off the receiver server!
	 */
	int exitReceiver();
	/**
     updates the shared memory receiving the data from the detector (without asking and closing the connection
     /returns OK
	 */
	int updateReceiverNoWait();

	/**
     updates the shared memory receiving the data from the detector
     /returns OK
	 */
	int updateReceiver();

	/**
	 * Send the multi detector size to the detector
	 */
	void sendMultiDetectorSize();

	/** send the detector pos id to the receiver
	 * for various file naming conventions for multi detectors in receiver
	 */
	void setDetectorId();

	/** send the detector host name to the  receiver
	 * for various handshaking required with the detector
	 */
	void setDetectorHostname();


	/**
     \returns file dir
	 */
	string getFilePath(){return setFilePath();};

	/**
     Sets up the file directory
     @param s fileDir file directory
     \returns file dir
	 */
	string setFilePath(string s="");


	/**
     \returns file name
	 */
	string getFileName(){return setFileName();};
	/**
     Sets up the file name
     @param s file name
     \returns file name
	 */
	string setFileName(string s="");


	/**
     Sets the max frames per file in receiver
     @param f max frames per file
     \returns max frames per file in receiver
	 */
	int setReceiverFramesPerFile(int f = -1);

	/**
     \returns file name
	 */
	fileFormat getFileFormat(){return setFileFormat();};

	/**
     Sets up the file format
     @param f file format
     \returns file format
	 */
	fileFormat setFileFormat(fileFormat f=GET_FILE_FORMAT);

	/**
     \returns file index
	 */
	int getFileIndex(){return setFileIndex();};
	/**
     Sets up the file index
     @param i file index
     \returns file index
	 */
	int setFileIndex(int i=-1);




	/**   Starts the listening mode of receiver
        \returns OK or FAIL
	 */
	int startReceiver();

	/**   Stops the listening mode of receiver
        \returns OK or FAIL
	 */
	int stopReceiver();

	/** Sets the receiver to start any readout remaining in the fifo and
	 * change status to transmitting.
	 * The status changes to run_finished when fifo is empty
	 */
	runStatus startReceiverReadout();


	/**   gets the status of the listening mode of receiver
        \returns status
	 */
	runStatus getReceiverStatus();

	/**   gets the number of frames caught by receiver
        \returns number of frames caught by receiver
	 */
	int getFramesCaughtByReceiver();

	/**   gets the number of frames caught by any one receiver (to avoid using threadpool)
 	\returns number of frames caught by any one receiver (master receiver if exists)
	 */
	int getFramesCaughtByAnyReceiver() {return getFramesCaughtByReceiver();};

	/**  gets the current frame index of receiver
     \returns current frame index of receiver
	 */
	int getReceiverCurrentFrameIndex();

	/**
	 * resets framescaught
	 * @param index frames caught by receiver
	 */
	int resetFramesCaught();



	/**
     Sets/Gets receiver file write enable
     @param enable 1 or 0 to set/reset file write enable
     /returns file write enable
	 */
	int enableWriteToFile(int enable=-1);

	/**
     Sets/Gets file overwrite enable
     @param enable 1 or 0 to set/reset file overwrite enable
     /returns file overwrite enable
	 */
	int overwriteFile(int enable=-1);


	/** Sets the read receiver frequency
   	  if data required from receiver randomly readRxrFrequency=0,
   	   else every nth frame to be sent to gui
   	   @param freq is the receiver read frequency
   	   /returns read receiver frequency
	 */
	int setReadReceiverFrequency(int freq=-1);

	/** Sets the read receiver timer
   	  if data required from receiver randomly readRxrFrequency=0,
   	   then the timer between each data stream is set with time_in_ms
   	   @param time_in_ms timer between frames
   	   /returns read receiver timer
	 */
	int setReceiverReadTimer(int time_in_ms=500);

	/**
	 * Enable data streaming to client
	 * @param enable 0 to disable, 1 to enable, -1 to get the value
	 * @returns data streaming to client enable
	 */
	int enableDataStreamingToClient(int enable=-1) {
		cprintf(RED,"ERROR: Must be called from the multi Detector level\n");
		return 0;
	}

	/** Enable or disable streaming data from receiver to client
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @returns data streaming from receiver enable
	 */
	int enableDataStreamingFromReceiver(int enable=-1);

	/** enable/disable or get data compression in receiver
	 * @param i is -1 to get, 0 to disable and 1 to enable
     /returns data compression in receiver
	 */
	int enableReceiverCompression(int i = -1);

	/** enable/disable or 10Gbe
	 * @param i is -1 to get, 0 to disable and 1 to enable
     /returns if 10Gbe is enabled
	 */
	int enableTenGigabitEthernet(int i = -1);

	/** set/get receiver fifo depth
	 * @param i is -1 to get, any other value to set the fifo deph
     /returns the receiver fifo depth
	 */
	int setReceiverFifoDepth(int i = -1);

	/** set/get receiver silent mode
	 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
     /returns the receiver silent mode enable
	 */
	int setReceiverSilentMode(int i = -1);

	/**
     If data streaming in receiver is enabled,
     restream the stop dummy packet from receiver
     Used usually for Moench,
     in case it is lost in network due to high data rate
     \returns OK if success else FAIL
	 */
	int restreamStopFromReceiver();

	/** opens pattern file and sends pattern to CTB
      @param fname pattern file to open
      @returns OK/FAIL
	 */
	int setCTBPattern(string fname);


	/** Writes a pattern word to the CTB
      @param addr address of the word, -1 is I/O control register,  -2 is clk control register
      @param word 64bit word to be written, -1 gets
      @returns actual value
	 */
	uint64_t setCTBWord(int addr,uint64_t word=-1);

	/** Sets the pattern or loop limits in the CTB
      @param level -1 complete pattern, 0,1,2, loop level
      @param start start address if >=0
      @param stop stop address if >=0
      @param n number of loops (if level >=0)
      @returns OK/FAIL
	 */
	int setCTBPatLoops(int level,int &start, int &stop, int &n);


	/** Sets the wait address in the CTB
      @param level  0,1,2, wait level
      @param addr wait address, -1 gets
      @returns actual value
	 */
	int setCTBPatWaitAddr(int level, int addr=-1);

	/** Sets the wait time in the CTB
      @param level  0,1,2, wait level
      @param t wait time, -1 gets
      @returns actual value
	 */
	int setCTBPatWaitTime(int level, uint64_t t=-1);

	/** returns the  detector MAC address\sa sharedSlsDetector  */
	string getDetectorMAC() {return string(thisDetector->detectorMAC);};
	/** returns the  detector IP address\sa sharedSlsDetector  */
	string getDetectorIP() {return string(thisDetector->detectorIP);};
	/** returns the receiver IP address \sa sharedSlsDetector  */
	string getReceiver() {return string(thisDetector->receiver_hostname);};
	/** returns the receiver UDP IP address \sa sharedSlsDetector  */
	string getReceiverUDPIP() {return string(thisDetector->receiverUDPIP);};
	/** returns the receiver UDP MAC address \sa sharedSlsDetector  */
	string getReceiverUDPMAC() {return string(thisDetector->receiverUDPMAC);};
	/** returns the receiver UDP IP address \sa sharedSlsDetector  */
	string getReceiverUDPPort() {ostringstream ss; ss << thisDetector->receiverUDPPort; string s = ss.str(); return s;};
	/** returns the receiver UDP2 for Eiger IP address \sa sharedSlsDetector  */
	string getReceiverUDPPort2() {ostringstream ss; ss << thisDetector->receiverUDPPort2; string s = ss.str(); return s;};
	/** returns the client zmq port \sa sharedSlsDetector  */
	string getClientStreamingPort() {ostringstream ss; ss << thisDetector->zmqport; string s = ss.str(); return s;};
	/** returns the receiver zmq port \sa sharedSlsDetector  */
	string getReceiverStreamingPort() {ostringstream ss; ss << thisDetector->receiver_zmqport; string s = ss.str(); return s;};
	/** gets the zmq source ip in client, returns "none" if default setting and no custom ip set*/
	string getClientStreamingIP(){return string(thisDetector->zmqip);};
	/** gets the zmq source ip in receiver, returns "none" if default setting and no custom ip set*/
	string getReceiverStreamingIP(){return string(thisDetector->receiver_zmqip);};
	/** gets the additional json header, returns "none" if default setting and no custom set*/
	string getAdditionalJsonHeader(){return string(thisDetector->receiver_additionalJsonHeader);};
	/** returns the receiver UDP socket buffer size  */
	string getReceiverUDPSocketBufferSize() {return setReceiverUDPSocketBufferSize();};
	/** returns the real receiver UDP socket buffer size  */
	string getReceiverRealUDPSocketBufferSize();


	/** validates the format of detector MAC address and sets it \sa sharedSlsDetector  */
	string setDetectorMAC(string detectorMAC);
	/** validates the format of detector IP address and sets it \sa sharedSlsDetector  */
	string setDetectorIP(string detectorIP);
	/** validates and sets the receiver IP address/hostname \sa sharedSlsDetector  */
	string setReceiver(string receiver);
	/** validates the format of receiver udp ip and sets it \sa sharedSlsDetector  */
	string setReceiverUDPIP(string udpip);
	/** validates the format of receiver udp mac and sets it \sa sharedSlsDetector  */
	string setReceiverUDPMAC(string udpmac);
	/** sets the receiver udp port \sa sharedSlsDetector  */
	int setReceiverUDPPort(int udpport);
	/** sets the receiver udp port2 for Eiger \sa sharedSlsDetector  */
	int setReceiverUDPPort2(int udpport);
	/** sets the zmq port in client (includes "multi" at the end if it should calculate individual ports \sa sharedSlsDetector  */
	string setClientStreamingPort(string port);
	/** sets the zmq port in receiver (includes "multi" at the end if it should calculate individual ports \sa sharedSlsDetector  */
	string setReceiverStreamingPort(string port);
	/** sets the zmq source ip in client */
	string setClientStreamingIP(string sourceIP);
	/** sets the zmq source ip in receiver. if empty, uses rx_hostname*/
	string setReceiverStreamingIP(string sourceIP);
	/** additional json header, returns "none" if default setting and no custom set */
	string setAdditionalJsonHeader(string jsonheader);
	/** sets the receiver UDP socket buffer size  */
	string setReceiverUDPSocketBufferSize(int udpsockbufsize=-1);

	/** sets the transmission delay for left or right port or for an entire frame*/
	string setDetectorNetworkParameter(networkParameter index, int delay);



	/**
     analog test
     \param modte test mode
     \return pointer to acquired data

     not yet implemented
	 */

	int* analogTest(analogTestMode mode);

	/**
      enable analog output of channel ichan

      not yet implemented
	 */
	int enableAnalogOutput(int ichan);

	/**
      enable analog output of channel ichan, chip ichip, module imod

      not yet implemented
	 */
	int enableAnalogOutput(int imod, int ichip, int ichan);

	/**
     give a train of calibration pulses
     \param vcal pulse amplitude
     \param npulses number of pulses

     not yet implemented

	 */
	int giveCalibrationPulse(double vcal, int npulses);


private:
	/**
	 * Get Detector Type from Shared Memory (opening shm without verifying size)
	 * @param multiId multi detector Id
	 * @param verify true to verify if shm size matches existing one
	 * @returns detector type
	 */
	detectorType getDetectorTypeFromShm(int multiId, bool verify = true);
	/**
	 * Initialize shared memory
	 * @param created true if shared memory must be created, else false to open
	 * @param type type of detector
	 * @param multiId multi detector Id
	 * @param verify true to verify if shm size matches existing one
	 * @returns true if the shared memory was created now
	 */
	void initSharedMemory(bool created, detectorType type, int multiId, bool verify = true);

	/**
	 * Calculate shared memory size based on detector type
	 * @param type type of detector
	 */
	int calculateSharedMemorySize(detectorType type);

	/**
	 * Initialize detector structure
	 * @param created true if shared memory was just created now
	 * @param type type of detector
	 * @param verify true to verify if shm size matches existing one
	 */
	void initializeDetectorStructure(bool created, detectorType type, bool verify = true);

	/**
	 * Initialize class members (and from parent classes)
	 */
	void initializeMembers();

	/** Gets MAC from receiver and sets up UDP Connection */
	int setUDPConnection();


	/**
     send a sls_detector_channel structure over socket
	 */
	int sendChannel(sls_detector_channel*);
	/**
     send a sls_detector_chip structure over socket
	 */
	int sendChip(sls_detector_chip*);
	/**
     send a sls_detector_module structure over socket
	 */
	int sendModule(sls_detector_module*);
	/**
     receive a sls_detector_channel structure over socket
	 */
	int receiveChannel(sls_detector_channel*);
	/**
     receive a sls_detector_chip structure over socket
	 */
	int receiveChip(sls_detector_chip*);
	/**
     receive a sls_detector_module structure over socket
	 */
	int receiveModule(sls_detector_module*);



	/** slsDetector Id or position in the detectors list */
	int detId;

	/** Shared Memory object */
	SharedMemory* sharedMemory;

	/** Shared memory structure */
	sharedSlsDetector *thisDetector;

	/** multiSlsDetector referece */
	multiSlsDetector *multiDet;

	receiverInterface *thisReceiver;

	/**    socket for control commands	 */
	MySocketTCP *controlSocket;

	/**     socket for emergency stop	 */
	MySocketTCP *stopSocket;

	/**     socket for data acquisition	 */
	MySocketTCP *dataSocket;

	/** pointer to detector module structures in shared memory */
	sls_detector_module *detectorModules;
	/** pointer to dac valuse in shared memory  */
	dacs_t *dacs;
	/** pointer to adc valuse in shared memory  */
	dacs_t *adcs;
	/** pointer to chip registers in shared memory */
	int *chipregs;
	/** pointer to channal registers  in shared memory */
	int *chanregs;
	/** pointer to gain values in shared memory  */
	int *gain;
	/** pointer to offset values in shared memory  */
	int *offset;

	/** pointer to flat field coefficients in shared memory  */
	double *ffcoefficients;
	/** pointer to flat field coefficient errors in shared memory  */
	double *fferrors;

};

#endif

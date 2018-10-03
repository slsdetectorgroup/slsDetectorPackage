#ifndef MULTI_SLS_DETECTOR_H
#define MULTI_SLS_DETECTOR_H

/**
 @libdoc The multiSlsDetector class is used to operate several slsDetectors in parallel.
 * @short This is the base class for multi detector system functionalities
 * @author Anna Bergamaschi
 */

#include "slsDetectorBase.h"

class slsDetector;
class SharedMemory;
class ZmqSocket;
class detectorData;

#include <vector>
#include <string>


#define MULTI_SHMVERSION	0x181002
#define SHORT_STRING_LENGTH	50
#define DATE_LENGTH			30

class multiSlsDetector  : public slsDetectorBase {

private:

	/**
	 * @short structure allocated in shared memory to store detector settings for IPC and cache
	 */
	typedef  struct sharedMultiSlsDetector {


		/* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

		/** shared memory version */
		int shmversion;

		/** last process id accessing the shared memory */
		pid_t lastPID;

		/** last user name accessing the shared memory */
		char lastUser[SHORT_STRING_LENGTH];

		/** last time stamp when accessing the shared memory */
		char lastDate[SHORT_STRING_LENGTH];

		/** number of sls detectors in shared memory */
		int numberOfDetectors;

		/** END OF FIXED PATTERN -----------------------------------------------*/




		/** Number of detectors operated at once */
		int numberOfDetector[2];

		/** online flag - is set if the detector is connected, unset if socket
		 * connection is not possible  */
		int onlineFlag;

		/** stopped flag - is set if an acquisition error occurs or the detector
		 * is stopped manually. Is reset to 0 at the start of the acquisition */
		int stoppedFlag;

		/**  size of the data that are transfered from all detectors */
		int dataBytes;

		/** data bytes including gap pixels transferred from all detectors */
		int dataBytesInclGapPixels;

		/**  total number of channels for all detectors */
		int numberOfChannels;

		/**  total number of channels for all detectors  in one dimension*/
		int numberOfChannel[2];

		/** total number of channels including gap pixels in one dimension */
		int numberOfChannelInclGapPixels[2];

		/**  total number of channels for all detectors */
		int maxNumberOfChannels;

		/**  max number of channels for all detectors  in one dimension*/
		int maxNumberOfChannel[2];

		/**  max number of channels including gap pixels for all detectors  in
		 * one dimension*/
		int maxNumberOfChannelInclGapPixels[2];

		/** max number of channels allowed for the complete set of detectors in
		 * one dimension */
		int maxNumberOfChannelsPerDetector[2];

		/** timer values */
		int64_t timerValue[MAX_TIMERS];

		/** detector settings (standard, fast, etc.) */
		detectorSettings currentSettings;

		/** detector threshold (eV) */
		int currentThresholdEV;

		/** threaded processing flag (i.e. if data are processed and written to
		 * file in a separate thread)  */
		int threadedProcessing;

		/** dead time (in ns) for rate corrections */
		double tDead;

		/** flag for acquiring */
		bool acquiringFlag;

		/** receiver online flag - is set if the receiver is connected,
		 * unset if socket connection is not possible  */
		int receiverOnlineFlag;

		/** data streaming (up stream) enable in receiver */
		bool receiver_upstream;

	} sharedMultiSlsDetector;




public:


	/**
	 * Constructor
	 * @param id multi detector id
	 * @param verify true to verify if shared memory version matches existing one
	 * @param update true to update last user pid, date etc
	 */
	multiSlsDetector(int id = 0, bool verify = true, bool update = true);

	/**
	 * Destructor
	 */
	virtual ~multiSlsDetector();

	/**
	 * Creates/open shared memory, initializes detector structure and members
	 * Called by constructor/ set hostname / read config file
	 * @param verify true to verify if shared memory version matches existing one
	 * @param update true to update last user pid, date etc
	 */
	void setupMultiDetector(bool verify = true, bool update = true);

	/**
	 * If specific position, then provide result with that detector at position pos
	 * else concatenate the result of all detectors
	 * @param somefunc function pointer
	 * @param pos positin of detector in array (-1 is for all)
	 * @returns result for detector at that position or concatenated string of all detectors
	 */
	std::string concatResultOrPos(std::string (slsDetector::*somefunc)(int), int pos);

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * @param somefunc function pointer
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T>
	T callDetectorMember(T (slsDetector::*somefunc)());

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * with string as return
	 * @param somefunc function pointer
	 * @returns concatenated string of results ifdifferent, otherwise result in
	 * calling method
	 */
	std::string callDetectorMember(std::string(slsDetector::*somefunc)());

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * with string argument and string return
	 * @param somefunc function pointer
	 * @param s0 argument for calling method
	 * @returns concatenated result if values are different, otherwise result in calling method
	 */
	std::string callDetectorMember(std::string (slsDetector::*somefunc)(std::string),
			std::string s0);

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * with an extra argument
	 * @param somefunc function pointer
	 * @param value argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename V>
	T callDetectorMember(T (slsDetector::*somefunc)(V), V value);

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * with two extra arguments
	 * @param somefunc function pointer
	 * @param par1 argument for calling method
	 * @param par2 second argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename P1, typename P2>
	T callDetectorMember(T (slsDetector::*somefunc)(P1, P2), P1 par1, P2 par2);


	/**
	 * Parallel calls to all the detectors in calling a particular method
	 * @param somefunc function pointer
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T>
	T parallelCallDetectorMember(T (slsDetector::*somefunc)());

	/**
	 * Parallel calls to all the detectors in calling a particular method
	 * with an extra argument
	 * @param somefunc function pointer
	 * @param value argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename P1>
	T parallelCallDetectorMember(T (slsDetector::*somefunc)(P1), P1 value);

	/**
	 * Parallel calls to all the detectors in calling a particular method
	 * with two extra arguments
	 * @param somefunc function pointer
	 * @param par1 argument for calling method
	 * @param par2 second argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename P1, typename P2>
	T parallelCallDetectorMember(T (slsDetector::*somefunc)(P1, P2), P1 par1, P2 par2);

	/**
	 * Parallel calls to all the detectors in calling a particular method
	 * with three int arguments
	 * @param somefunc function pointer
	 * @param v0 argument for calling method
	 * @param v1 second argument for calling method
	 * @param v2 third argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	int parallelCallDetectorMember(int (slsDetector::*somefunc)(int, int, int),
			int v0, int v1, int v2);

	/**
	 * Parallel calls to all the detectors in calling a particular method
	 * with string argument and string return
	 * @param somefunc function pointer
	 * @param s0 argument for calling method
	 * @returns concatenated result if values are different, otherwise result in calling method
	 */
	std::string parallelCallDetectorMember(std::string (slsDetector::*somefunc)(std::string),
			std::string s0);
	/**
	 * Loop serially through all results and
	 * return a value if they are all same, else return -1
	 * @param return_values vector of results
	 * @returns -1 if values are different, otherwise result
	 */
	template<typename T>
	T minusOneIfDifferent(const std::vector<T>&);

	/**
	 * Loop serially through all results and
	 * return a value if they are all same, else concatenate them
	 * @param return_values vector of results
	 * @returns concatenated result if values are different, otherwise result
	 */
	std::string concatenateIfDifferent(const std::vector<std::string>& return_values);

	/**
	 * Decodes which detector and the corresponding channel numbers for it
	 * Mainly useful in a multi detector setROI (Gotthard, Mythen?)
	 * @param offsetX channel number or total channel offset in x direction
	 * @param offsetY channel number or total channel offset in y direction
	 * @param channelX channel number from detector offset in x direction
	 * @param channelY channel number from detector offset in x direction
	 * @returns detector id or -1 if channel number out of range
	 */
	int decodeNChannel(int offsetX, int offsetY, int &channelX, int &channelY);

	/**
	 * Checks error mask and returns error message and its severity if it exists
	 * @param critical is 1 if any of the messages is critical
	 * @returns error message else an empty std::string
	 */
	std::string getErrorMessage(int &critical, int detPos = -1);

	/**
	 * Clears error mask of both multi and sls
	 * @returns error mask
	 */
	int64_t clearAllErrorMask(int detPos = -1);

	/**
	 * Set Error Mask from all detectors
	 * if each had errors in the mask already
	 */
	void setErrorMaskFromAllDetectors();

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
	int checkVersionCompatibility(portType t, int detPos = -1);

	/**
	 * Get ID or version numbers
	 * @param mode version type
	 * @param imod module number in entire module list (gets decoded) (-1 for all)
	 * @returns Id or version number of that type
	 */
	int64_t getId(idMode mode, int detPos = -1);

	/**
	 * Get sls detector object from position in detectors array
	 * @param pos position in detectors array
	 * @returns pointer to sls detector object
	 */
	slsDetector* getSlsDetector(int detPos = -1);

	/**
	 * Accessing the sls detector from the multi list using position
	 * @param pos position in the multi list
	 * @returns slsDetector object
	 */
	slsDetector *operator()(int detPos = -1) const;

	/**
	 * Free shared memory from the command line
	 * avoiding creating the constructor classes and mapping
	 * @param multiId multi detector Id
	 */
	static void freeSharedMemory(int multiId, int detPos = -1);

	/**
	 * Free shared memory and delete shared memory structure
	 * occupied by the sharedMultiSlsDetector structure
	 * Clears all the vectors and destroys threadpool to bring
	 * object back to state before object creation amap
	 */
	void freeSharedMemory(int detPos = -1);

	/**
	 * Get user details of shared memory
	 * @returns string with user details
	 */
	std::string getUserDetails();

	/**
	 * Sets the hostname of all sls detectors in shared memory
	 * Connects to them to set up online flag
	 * @param name concatenated hostname of all the sls detectors
	 */
	void setHostname(const char* name, int detPos = -1);

	/**
	 * Gets the hostname of detector at particular position
	 * or concatenated hostnames of all the sls detectors
	 * @param pos position of detector in array, -1 for all detectors
	 * @returns concatenated hostnames of all detectors or hostname of specific one
	 */
	std::string getHostname(int detPos = -1);

	/**
	 * Appends detectors to the end of the list in shared memory
	 * Connects to them to set up online flag
	 * @param name concatenated hostname of the sls detectors to be appended to the list
	 */
	 void addMultipleDetectors(const char* name);


	using slsDetectorBase::getDetectorType;
	/**
	 * Get Detector type for a particular sls detector or get the first one
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, returns the first det type
	 */
	detectorType getDetectorsType(int detPos = -1);

	/**
	 * Concatenates string types of all sls detectors or
	 * returns the detector type of the first sls detector
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, concatenates
	 */
	std::string sgetDetectorsType(int detPos = -1);

	/**
	 * Just to overload getDetectorType from users
	 * Concatenates string types of all sls detectors or
	 * returns the detector type of the first sls detector
	 * @returns detector type of sls detector in position pos, if -1, concatenates
	 */
	std::string getDetectorType(int detPos = -1);

	/**
	 * Returns the number of detectors in the multidetector structure
	 * @returns number of detectors
	 */
	int getNumberOfDetectors();

	/**
	 * Returns number of detectors in dimension d
	 * @param d dimension d
	 * @returns number of detectors in dimension d
	 */
	int getNumberOfDetectors(dimension d);

	/**
	 * Returns the number of detectors in each direction
   	   @param nx number of detectors in x direction
   	   @param ny number of detectors in y direction
	 */
	void getNumberOfDetectors(int& nx, int& ny);


	/**
	 * Returns the total number of channels of all sls detectors from shared memory
	 * @returns the total number of channels of all sls detectors
	 */
	int getTotalNumberOfChannels(int detPos = -1);

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * from shared memory
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 */
	int getTotalNumberOfChannels(dimension d, int detPos = -1);

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels from shared memory
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels
	 */
	int getTotalNumberOfChannelsInclGapPixels(dimension d, int detPos = -1);

	/**
	 * Returns the maximum number of channels of all sls detectors in each dimension d
	 * from shared memory. multi detector shared memory variable to calculate
	 * offsets for each sls detector
	 * @param d dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	int getMaxNumberOfChannelsPerDetector(dimension d);

	/**
	 * Sets the maximum number of channels of all sls detectors in each dimension d
	 * from shared memory, multi detector shared memory variable to calculate
	 * offsets for each sls detector
	 * @param d dimension d
	 * @param i maximum number of channels for multi structure in dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	int setMaxNumberOfChannelsPerDetector(dimension d,int i);

	/**
	 * Get Detector offset from shared memory in dimension d
	 * @param d dimension d
	 * @param pos detector position in multi detector list
	 * @returns offset in dimension d, -1 if pos is not an actual position in list
	 */
	int getDetectorOffset(dimension d, int detPos = -1);

	/**
	 * Set Detector offset in shared memory in dimension d
	 * @param d dimension d
	 * @param off offset for detector
	 * @param pos detector position in multi detector list
	 */
	void setDetectorOffset(dimension d, int off, int detPos = -1);

	/**
	 * Updates the channel offsets in X and Y dimension for all the sls detectors
	 * It is required for decodeNMod and setting ROI
	 */
	void updateOffsets();

	/**
	 * Checks if the multi detectors are online and sets the online flag
	 * @param online if GET_ONLINE_FLAG, only returns shared memory online flag,
	 * else sets the detector in online/offline state
	 * if OFFLINE_FLAG, (i.e. no communication to the detector - using only local structure - no data acquisition possible!);
	 * if ONLINE_FLAG, detector in online state (i.e. communication to the detector updating the local structure)
	 * @returns online/offline status
	 */
	int setOnline(int const online=GET_ONLINE_FLAG, int detPos = -1);

	/**
	 * Checks if each of the detectors are online/offline
	 * @returns empty string if they are all online,
	 * else returns concatenation of strings of all detectors that are offline
	 */
	std::string checkOnline(int detPos = -1);

	/**
	 * Set/Gets TCP Port of detector or receiver
	 * @param t port type
	 * @param num port number (-1 gets)
	 * @returns port number
	 */
	int setPort(portType t, int num=-1, int detPos = -1);

	/**
	 * Lock server for this client IP
	 * @param p 0 to unlock, 1 to lock
	 * @returns 1 for locked or 0 for unlocked
	 */
	int lockServer(int p, int detPos = -1);

	/**
	 * Get last client IP saved on detector server
	 * @returns last client IP saved on detector server
	 */
	std::string getLastClientIP(int detPos = -1);

	/**
	 * Exit detector server
	 * @returns OK or FAIL
	 */
	int exitServer(int detPos = -1);

	/**
	 * Load configuration from a configuration File
	 * @param fname configuration file name
	 * @return OK or FAIL
	 */
	int readConfigurationFile(std::string const fname);

	/**
	 * Write current configuration to a file
	 * @param fname configuration file name
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(std::string const fname);

	/**
	 * Returns the trimfile or settings file name (Useless??)
	 * @returns the trimfile or settings file name
	 */
	std::string getSettingsFile(int detPos = -1);

	/**
	 * Get detector settings
	 * @param ipos position in multi list (-1 all)
	 * @returns current settings
	 */
	detectorSettings getSettings(int detPos = -1);

	/**
	 * Load detector settings from the settings file picked from the trimdir/settingsdir
	 * Eiger only stores in shared memory ( a get will overwrite this)
	 * For Eiger, one must use threshold
	 * @param isettings settings
	 * @param ipos position in multi list (-1 all)
	 * @returns current settings
	 */
	detectorSettings setSettings(detectorSettings isettings, int detPos = -1);

	/**
	 * Get threshold energy (Eiger)
	 * @param imod module number (-1 all)
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int getThresholdEnergy(, int detPos = -1);

	/**
	 * Set threshold energy (Eiger)
	 * @param e_eV threshold in eV
	 * @param imod module number (-1 all)
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int setThresholdEnergy(int e_eV, detectorSettings isettings=GET_SETTINGS,int tb=1, int detPos = -1);

	/**
	 * Returns the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @returns the trimbit/settings directory
	 */
	std::string getSettingsDir(int detPos = -1);

	/**
	 * Sets the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @param s trimbits/settings directory
	 * @returns the trimbit/settings directory
	 */
	std::string setSettingsDir(std::string s, int detPos = -1);

	/**
	 * Returns the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @returns the calibration files directory
	 */
	std::string getCalDir(int detPos = -1);

	/**
	 * Sets the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @param s the calibration files directory
	 * @returns the calibration files directory
	 */
	std::string setCalDir(std::string s, int detPos = -1);

	/**
	 * Loads the modules settings/trimbits reading from a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int loadSettingsFile(std::string fname, int detPos = -1);

	/**
	 * Saves the modules settings/trimbits to a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int saveSettingsFile(std::string fname, int detPos = -1);

	/**
	 * Loads the modules calibration data reading from a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int loadCalibrationFile(std::string fname, int detPos = -1);

	/**
	 * Saves the modules calibration data to a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int saveCalibrationFile(std::string fname, int detPos = -1);

	/**
	 * Get Detector run status
	 * @returns status
	 */
	runStatus  getRunStatus(int detPos = -1);

	/**
	 * Prepares detector for acquisition (Eiger)
	 * @returns OK if all detectors are ready for acquisition, FAIL otherwise
	 */
	int prepareAcquisition(int detPos = -1);

	/**
	 * Start detector acquisition (Non blocking)
	 * @returns OK or FAIL if even one does not start properly
	 */
	int startAcquisition(int detPos = -1);

	/**
	 * Stop detector acquisition
	 * @returns OK or FAIL
	 */
	int stopAcquisition(int detPos = -1);

	/**
	 * Give an internal software trigger to the detector (Eiger only)
	 * @return OK or FAIL
	 */
	int sendSoftwareTrigger(int detPos = -1);

	/**
	 * Start detector acquisition and read all data (Blocking until end of acquisition)
	 * @returns OK or FAIL
	 */
	int startAndReadAll(int detPos = -1);

	/**
	 * Start readout (without exposure or interrupting exposure) (Eiger store in ram)
	 * @returns OK or FAIL
	 */
	int startReadOut(int detPos = -1);

	/**
	 * Requests and  receives all data from the detector (Eiger store in ram)
	 * @returns OK or FAIL
	 */
	int readAll(int detPos = -1);

	/**
	 * Configures in detector the destination for UDP packets (Not Mythen)
	 * @returns OK or FAIL
	 */
	int configureMAC(int detPos = -1);

	/**
	 * Set/get timer value (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @param imod module number (gotthard delay can have different values)
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t setTimer(timerIndex index, int64_t t=-1, int detPos = -1);

	/**
	 * Set/get timer value left in acquisition (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @param imod module number
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t getTimeLeft(timerIndex index, int detPos = -1);

	/**
	 * Set speed
	 * @param sp speed type  (clkdivider option for Jungfrau and Eiger, others for Mythen/Gotthard)
	 * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other values check manual
	 * @returns value of speed set
	 */
	int setSpeed(speedVariable sp, int value=-1, int detPos = -1);

	/**
	 * Set/get dynamic range and updates the number of dataBytes
	 * (Eiger: If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1)
	 * @param i dynamic range (-1 get)
	 * @returns current dynamic range
	 * \sa sharedSlsDetector
	 */
	int setDynamicRange(int i=-1, int detPos = -1);

	/**
	 * Recalculated number of data bytes for multi detector
	 * @returns tota number of data bytes for multi detector
	 */
	int getDataBytes(int detPos = -1);

	/**
	 * Set/get dacs value
	 * @param val value (in V)
	 * @param index DAC index
	 * @param mV 0 in dac units or 1 in mV
	 * @param imod module number (if -1 all modules)
	 * @returns current DAC value
	 */
	dacs_t setDAC(dacs_t val, dacIndex index , int mV, int detPos = -1);

	/**
	 * Get adc value
	 * @param index adc(DAC) index
	 * @param imod module number (if -1 all modules)
	 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
	 */
	dacs_t getADC(dacIndex index, int detPos = -1);

	/**
	 * Set/get timing mode
	 * @param pol timing mode (-1 gets)
	 * @returns current timing mode
	 */
	externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE, int detPos = -1);

	/**
	 * Set/get external signal flags (to specify triggerinrising edge etc) (Gotthard, Mythen)
	 * @param pol external signal flag (-1 gets)
	 * @param signalindex singal index (0 - 3)
	 * @returns current timing mode
	 */
	externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0, int detPos = -1);

	/**
	 * Set/get readout flags (Eiger, Mythen)
	 * @param flag readout flag (Eiger options: parallel, nonparallel, safe etc.) (-1 gets)
	 * @returns readout flag
	 */
	int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS, int detPos = -1);

	/**
	 * Write in a register. For Advanced users
	 * @param addr address of register
	 * @param val value to write into register
	 * @returns value read after writing
	 */
	uint32_t writeRegister(uint32_t addr, uint32_t val, int detPos = -1);

	/**
	 * Read from a register. For Advanced users
	 * @param addr address of register
	 * @returns value read from register
	 */
	uint32_t readRegister(uint32_t addr, int detPos = -1);

	/**
	 * Set bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	uint32_t setBit(uint32_t addr, int n, int detPos = -1);

	/**
	 * Clear bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	uint32_t clearBit(uint32_t addr, int n, int detPos = -1);

	/**
	 * Set network parameter
	 * @param p network parameter type
	 * @param s network parameter value
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	std::string setNetworkParameter(networkParameter p, std::string s, int detPos = -1);

	/**
	 * Get network parameter
	 * @param p network parameter type
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	std::string getNetworkParameter(networkParameter, int detPos = -1);

	/**
	 * Execute a digital test (Gotthard, Mythen)
	 * @param mode testmode type
	 * @param imod module index (-1 for all)
	 * @returns result of test
	 */
	int digitalTest(digitalTestMode mode, int detPos = -1);

	/**
	 * Load dark or gain image to detector (Gotthard)
	 * @param index image type
	 * @param fname file name from which to load image
	 * @returns OK or FAIL
	 */
	int loadImageToDetector(imageType index,std::string const fname, int detPos = -1);

	/**
	 * Writes the counter memory block from the detector (Gotthard)
	 * @param fname file name to load data from
	 * @param startACQ is 1 to start acquisition after reading counter
	 * @returns OK or FAIL
	 */
	int writeCounterBlockFile(std::string const fname,int startACQ=0, int detPos = -1);

	/**
	 * Resets counter in detector (Gotthard)
	 * @param startACQ is 1 to start acquisition after resetting counter
	 * @returns OK or FAIL
	 */
	int resetCounterBlock(int startACQ=0, int detPos = -1);

	/**
	 * Set/get counter bit in detector (Gotthard)
	 * @param i is -1 to get, 0 to reset and any other value to set the counter bit
	 * @returns the counter bit in detector
	 */
	int setCounterBit(int i = -1, int detPos = -1);

	/**
	 * Ensures that min is less than max in both dimensions (Gotthard)
	 * @param n number of rois
	 * @param r array of rois
	 */
	void verifyMinMaxROI(int n, ROI r[], int detPos = -1);

	/**
	 * Set ROI (Gotthard)
	 * At the moment only one set allowed
	 * @param n number of rois
	 * @param roiLimits array of roi
	 * @returns OK or FAIL
	 */
	int setROI(int n=-1,ROI roiLimits[]=NULL, int detPos = -1);

	/**
	 * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
	 * @param n number of rois
	 * @returns OK or FAIL
	 */
	ROI* getROI(int &n, int detPos = -1);

	/**
	 * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert users
	 * @param addr address of adc register
	 * @param val value
	 * @returns return value  (mostly -1 as it can't read adc register)
	 */
	int writeAdcRegister(int addr, int val, int detPos = -1);

	/**
	 * Activates/Deactivates the detector (Eiger only)
	 * @param enable active (1) or inactive (0), -1 gets
	 * @returns 0 (inactive) or 1 (active)for activate mode
	 */
	int activate(int const enable=-1, int detPos = -1);

	/**
	 * Set deactivated Receiver padding mode (Eiger only)
	 * @param padding padding option for deactivated receiver. Can be 1 (padding), 0 (no padding), -1 (gets)
	 * @returns 1 (padding), 0 (no padding), -1 (inconsistent values) for padding option
	 */
	int setDeactivatedRxrPaddingMode(int padding=-1, int detPos = -1);

	/**
	 * Returns the enable if data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @returns 1 for flipped, else 0
	 */
	int getFlippedData(dimension d=X, int detPos = -1);

	/**
	 * Sets the enable which determines if
	 * data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @param value 0 or 1 to reset/set or -1 to get value
	 * @returns enable flipped data across x or y axis
	 */
	int setFlippedData(dimension d=X, int value=-1, int detPos = -1);

	/**
	 * Sets all the trimbits to a particular value (Eiger)
	 * @param val trimbit value
	 * @param imod module number, -1 means all modules
	 * @returns OK or FAIL
	 */
	int setAllTrimbits(int val, int detPos = -1);

	/**
	 * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
	 * 4 bit mode gap pixels only in gui call back
	 * @param val 1 sets, 0 unsets, -1 gets
	 * @returns gap pixel enable or -1 for error
	 */
	int enableGapPixels(int val=-1, int detPos = -1);

	/**
	 * Sets the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param nen number of energies
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	int setTrimEn(int nen, int *en=NULL, int detPos = -1);

	/**
	 * Returns the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	int getTrimEn(int *en=NULL, int detPos = -1);

	/**
	 * Pulse Pixel (Eiger)
	 * @param n is number of times to pulse
	 * @param x is x coordinate
	 * @param y is y coordinate
	 * @returns OK or FAIL
	 */
	int pulsePixel(int n=0,int x=0,int y=0, int detPos = -1);

	/**
	 * Pulse Pixel and move by a relative value (Eiger)
	 * @param n is number of times to pulse
	 * @param x is relative x value
	 * @param y is relative y value
	 * @returns OK or FAIL
	 */
	int pulsePixelNMove(int n=0,int x=0,int y=0, int detPos = -1);

	/**
	 * Pulse Chip (Eiger)
	 * @param n is number of times to pulse
	 * @returns OK or FAIL
	 */
	int pulseChip(int n=0, int detPos = -1);

	/**
	 * Set/gets threshold temperature (Jungfrau)
	 * @param val value in millidegrees, -1 gets
	 * @param imod module number, -1 is all
	 * @returns threshold temperature in millidegrees
	 */
	int setThresholdTemperature(int val=-1, int detPos = -1);

	/**
	 * Enables/disables temperature control (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns temperature control enable
	 */
	int setTemperatureControl(int val=-1, int detPos = -1);

	/**
	 * Resets/ gets over-temperature event (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns over-temperature event
	 */
	int setTemperatureEvent(int val=-1, int detPos = -1);

	/**
	 * Set storage cell that stores first acquisition of the series (Jungfrau)
	 * @param value storage cell index. Value can be 0 to 15. (-1 gets)
	 * @returns the storage cell that stores the first acquisition of the series
	 */
	int setStoragecellStart(int pos=-1, int detPos = -1);

	/**
	 * Programs FPGA with pof file (Jungfrau)
	 * @param fname file name
	 * @returns OK or FAIL
	 */
	int programFPGA(std::string fname, int detPos = -1);

	/**
	 * Resets FPGA (Jungfrau)
	 * @returns OK or FAIL
	 */
	int resetFPGA(int detPos = -1);

	/**
	 * Power on/off Chip (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	int powerChip(int ival= -1, int detPos = -1);

	/**
	 * Automatic comparator disable (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	int setAutoComparatorDisableMode(int ival= -1, int detPos = -1);

	/**
	 * Returns the trimbits from the detector's shared memmory (Mythen, Eiger)
	 * @param retval is the array with the trimbits
	 * @param fromDetector is true if the trimbits shared memory have to be
	 * uploaded from detector
	 * @returns total number of channels for the detector
	 */
	int getChanRegs(double* retval,bool fromDetector, int detPos = -1);

	/**
	 * Calibrate Pedestal (ChipTestBoard)
	 * Starts acquisition, calibrates pedestal and writes to fpga
	 * @param frames number of frames
	 * @returns number of frames
	 */
	int calibratePedestal(int frames = 0, int detPos = -1);

	/**
	 * Set Rate correction (Mythen, Eiger)
	 * @param t dead time in ns - if 0 disable correction,
	 * if >0 set dead time to t, if < 0 set deadtime to default dead time
	 * for current settings
	 * @returns 0 if rate correction disabled, >0 otherwise
	 */
	int setRateCorrection(double t=0, int detPos = -1);

	/**
	 * Get rate correction (Mythen, Eiger)
	 * @param t reference for dead time
	 * @returns 0 if rate correction disabled, > 0 otherwise
	 */
	int getRateCorrection(double &t, int detPos = -1);

	/**
	 * Get rate correction tau (Mythen, Eiger)
	 * @returns 0 if rate correction disabled, otherwise the tau used for the correction
	 */
	double getRateCorrectionTau(int detPos = -1);

	/**
	 * Get rate correction (Mythen, Eiger)
	 * @returns 0 if rate correction disabled,  > 0 otherwise
	 */
	int getRateCorrection(int detPos = -1);

	/**
	 * Prints receiver configuration
	 * @returns  OK or FAIL
	 */
	int printReceiverConfiguration(int detPos = -1);

	/**
	 * Sets up receiver socket if online and sets the flag
	 * @param online online/offline flag (-1 gets)
	 * @returns online/offline flag
	 */
	int setReceiverOnline(int const online=GET_ONLINE_FLAG, int detPos = -1);

	/**
	 * Checks if the receiver is really online
	 * @returns empty string if all online, else concatenates hostnames of all
	 * detectors that are offline
	 */
	std::string checkReceiverOnline(int detPos = -1);

	/**
	 * Locks/Unlocks the connection to the receiver
	 * @param lock sets (1), usets (0), gets (-1) the lock
	 * @returns lock status of the receiver
	 */
	int lockReceiver(int lock=-1, int detPos = -1);

	/**
	 * Returns the IP of the last client connecting to the receiver
	 * @returns IP of last client connecting to receiver
	 */
	std::string getReceiverLastClientIP(int detPos = -1);

	/**
	 * Turns off the receiver server!
	 * @returns OK or FAIL
	 */
	int exitReceiver(int detPos = -1);

	/**
	 * Returns output file directory
	 * @returns output file directory
	 */
	std::string getFilePath(int detPos = -1);

	/**
	 * Sets up the file directory
	 * @param s file directory
	 * @returns file dir
	 */
	std::string setFilePath(std::string s, int detPos = -1);

	/**
	 * Returns file name prefix
	 * @returns file name prefix
	 */
	std::string getFileName(int detPos = -1);

	/**
	 * Sets up the file name prefix
	 * @param s file name prefix
	 * @returns file name prefix
	 */
	std::string setFileName(std::string s, int detPos = -1);

	/**
	 * Sets the max frames per file in receiver
	 * @param f max frames per file
	 * @returns max frames per file in receiver
	 */
	int setReceiverFramesPerFile(int f = -1, int detPos = -1);

	/**
	 * Sets the frames discard policy in receiver
	 * @param f frames discard policy
	 * @returns frames discard policy set in receiver
	 */
	frameDiscardPolicy setReceiverFramesDiscardPolicy(frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY, int detPos = -1);

	/**
	 * Sets the partial frames padding enable in receiver
	 * @param f partial frames padding enable
	 * @returns partial frames padding enable in receiver
	 */
	int setReceiverPartialFramesPadding(int f = -1, int detPos = -1);

	/**
	 * Returns file format
	 * @returns file name
	 */
	fileFormat getFileFormat(int detPos = -1);

	/**
	 * Sets up the file format
	 * @param f file format
	 * @returns file format
	 */
	fileFormat setFileFormat(fileFormat f, int detPos = -1);

	/**
	 * Returns file index
	 * @returns file index
	 */
	int getFileIndex(int detPos = -1);

	/**
	 * Sets up the file index
	 * @param i file index
	 * @returns file index
	 */
	int setFileIndex(int i, int detPos = -1);

	/**
	 * increments file index
	 * @returns the file index
	 */
	int incrementFileIndex(int detPos = -1);

	/**
	 * Receiver starts listening to packets
	 * @returns OK or FAIL
	 */
	int startReceiver(int detPos = -1);

	/**
	 * Stops the listening mode of receiver
	 * @returns OK or FAIL
	 */
	int stopReceiver(int detPos = -1);

	/**
	 * Gets the status of the listening mode of receiver
	 * @returns status
	 */
	runStatus getReceiverStatus();

	/**
	 * Gets the number of frames caught by receiver
	 * @returns number of frames caught by receiver
	 */
	int getFramesCaughtByReceiver(int detPos = -1);

	/**
	 * Gets the number of frames caught by any one receiver (to avoid using threadpool)
	 * @returns number of frames caught by any one receiver (master receiver if exists)
	 */
	int getFramesCaughtByAnyReceiver(int detPos = -1);

	/**
	 * Gets the current frame index of receiver
	 * @returns current frame index of receiver
	 */
	int getReceiverCurrentFrameIndex(int detPos = -1);

	/**
	 * Resets framescaught in receiver
	 * Use this when using startAcquisition instead of acquire
	 * @returns OK or FAIL
	 */
	int resetFramesCaught(int detPos = -1);

	/**
	 * Create Receiving Data Sockets
	 * @param destroy is true to destroy all the sockets
	 * @returns OK or FAIL
	 */
	int createReceivingDataSockets(const bool destroy = false), int detPos = -1;

	/**
	 * Reads frames from receiver through a constant socket
	 * Called during acquire() when call back registered or when using gui
	 */
	void readFrameFromReceiver(int detPos = -1);

	/**
	 * Sets/Gets receiver file write enable
	 * @param enable 1 or 0 to set/reset file write enable
	 * @returns file write enable
	 */
	int enableWriteToFile(int enable=-1, int detPos = -1);

	/**
	 * Sets/Gets file overwrite enable
	 * @param enable 1 or 0 to set/reset file overwrite enable
	 * @returns file overwrite enable
	 */
	int overwriteFile(int enable=-1, int detPos = -1);

	/**
	 * Sets the read receiver frequency
	 * if data required from receiver randomly readRxrFrequency=0,
	 * else every nth frame to be sent to gui/callback
	 * @param freq is the receiver read frequency. Value 0 is 200 ms timer (other
	 * frames not sent), 1 is every frame, 2 is every second frame etc.
	 * @returns read receiver frequency
	 */
	int setReadReceiverFrequency(int freq=-1, int detPos = -1);

	/**
	 * Sets the read receiver timer
	 * if data required from receiver randomly readRxrFrequency=0,
	 * then the timer between each data stream is set with time_in_ms
	 * @param time_in_ms timer between frames
	 * @returns read receiver timer
	 */
	int setReceiverReadTimer(int time_in_ms=500, int detPos = -1);

	/**
	 * Enable data streaming to client
	 * @param enable 0 to disable, 1 to enable, -1 to get the value
	 * @returns data streaming to client enable
	 */
	int enableDataStreamingToClient(int enable=-1, int detPos = -1);

	/**
	 * Enable or disable streaming data from receiver to client
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @returns data streaming from receiver enable
	 */
	int enableDataStreamingFromReceiver(int enable=-1, int detPos = -1);

	/**
	 * Enable/disable or 10Gbe
	 * @param i is -1 to get, 0 to disable and 1 to enable
	 * @returns if 10Gbe is enabled
	 */
	int enableTenGigabitEthernet(int i = -1, int detPos = -1);

	/**
	 * Set/get receiver fifo depth
	 * @param i is -1 to get, any other value to set the fifo deph
	 * @returns the receiver fifo depth
	 */
	int setReceiverFifoDepth(int i = -1, int detPos = -1);

	/**
	 * Set/get receiver silent mode
	 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
	 * @returns the receiver silent mode enable
	 */
	int setReceiverSilentMode(int i = -1, int detPos = -1);

	/**
	 * Opens pattern file and sends pattern to CTB
	 * @param fname pattern file to open
	 * @returns OK/FAIL
	 */
	int setCTBPattern(std::string fname, int detPos = -1);

	/**
	 * Writes a pattern word to the CTB
	 * @param addr address of the word, -1 is I/O control register,
	 * -2 is clk control register
	 * @param word 64bit word to be written, -1 gets
	 * @returns actual value
	 */
	uint64_t setCTBWord(int addr,uint64_t word=-1, int detPos = -1);

	/**
	 * Sets the pattern or loop limits in the CTB
	 * @param level -1 complete pattern, 0,1,2, loop level
	 * @param start start address if >=0
	 * @param stop stop address if >=0
	 * @param n number of loops (if level >=0)
	 * @returns OK/FAIL
	 */
	int setCTBPatLoops(int level,int &start, int &stop, int &n, int detPos = -1);

	/**
	 * Sets the wait address in the CTB
	 * @param level  0,1,2, wait level
	 * @param addr wait address, -1 gets
	 * @returns actual value
	 */
	int setCTBPatWaitAddr(int level, int addr=-1, int detPos = -1);

	/**
	 * Sets the wait time in the CTB
	 * @param level  0,1,2, wait level
	 * @param t wait time, -1 gets
	 * @returns actual value
	 */
	int setCTBPatWaitTime(int level, uint64_t t=-1, int detPos = -1);

	/**
	 * Loads the detector setup from file
	 * @param fname file to read from
	 * @param level if 2 reads also reads trimbits, angular conversion coefficients etc.
	 * from files with default extensions as generated by dumpDetectorSetup
	 * @returns OK or FAIL
	 */
	int retrieveDetectorSetup(std::string const fname, int level=0, int detPos = -1);

	/**
	 * Saves the detector setup to file
	 * @param fname file to write to
	 * @param level if 2 reads also trimbits, flat field, angular correction etc.
	 * and writes them to files with automatically added extension
	 * @returns OK or FAIL
	 */
	int dumpDetectorSetup(std::string const fname, int level=0, int detPos = -1);

	/**
	 * register callback for accessing acquisition final data
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg);

	/**
	 * register callback for accessing measurement final data
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg);

	/**
	 * register callback for accessing detector progress
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerProgressCallback(int( *func)(double,void*), void *pArg);

	  /**
	   * register calbback for accessing detector final data,
	   * also enables data streaming in client and receiver
	   * @param userCallback function for plotting/analyzing the data.
	   * Its arguments are
	   * the data structure d and the frame number f,
	   * s is for subframe number for eiger for 32 bit mode
	   * @param pArg argument
	  */
	void registerDataCallback(int( *userCallback)(detectorData*, int, int, void*),  void *pArg);

	/**
	 * Performs a complete acquisition
	 * resets frames caught in receiver, starts receiver, starts detector,
	 * blocks till detector finished acquisition, stop receiver, increments file index,
	 * loops for measurements, calls required call backs.
	 * @returns OK or FAIL depending on if it already started
	 */
	int acquire();

	/**
	 * Set/get if the data processing thread si enabled
	 * @param enable 0 no data processing thread, 1 separate thread, -1 get
	 */
	int setThreadedProcessing(int enable=-1);

private:
	/**
	 * Initialize (open/create) shared memory for the sharedMultiDetector structure
	 * @param verify true to verify if shm size matches existing one
	 * @param update true to update last user pid, date etc
	 * @returns true if shared memory was created in this call, else false
	 */
	bool initSharedMemory(bool verify = true);

	/**
	 * Initialize detector structure for the shared memory just created
	 */
	void initializeDetectorStructure();

	/**
	 * Initialize class members (and from parent classes)
	 * @param verify true to verify if shm size matches existing one
	 */
	void initializeMembers(bool verify = true);

	/**
	 * Update user details in detector structure
	 */
	void updateUserdetails();

	/**
	 * Execute in command line and return result
	 * @param cmd command
	 * @returns result
	 */
	std::string exec(const char* cmd);

	/**
	 * Add sls detector
	 * @param s hostname of the single detector
	 */
	void addSlsDetector (std::string s);

	/**
	 * add gap pixels to the image (only for Eiger in 4 bit mode)
	 * @param image pointer to image without gap pixels
	 * @param gpImage poiner to image with gap pixels, if NULL, allocated inside function
	 * @returns number of data bytes of image with gap pixels
	 */
	int processImageWithGapPixels(char* image, char*& gpImage);

	/**
	 * Set total progress (total number of frames/images in an acquisition)
	 * @returns total progress
	 */
	int setTotalProgress();

	/**
	 * Get progress in current acquisition
	 * @returns current progress
	 */
	double getCurrentProgress();

	/**
	 * Increment progress by one
	 */
	void incrementProgress();

	/**
	 * Set current progress to argument
	 * @param i current progress
	 */
	void setCurrentProgress(int i=0);

	/**
	 * Start data processing thread
	 */
	void startProcessingThread();

	/**
	 * Static function to call processing thread
	 */
	static void* startProcessData(void *n);

	/**
	 * Combines data from all readouts and gives it to the gui
	 * or just gives progress of acquisition by polling receivers
	 */
	void* processData();

	/**
	 * Check if processing thread is ready to join main thread
	 * @returns true if ready, else false
	 */
	int checkJoinThread();

	/**
	 * Main thread sets if the processing thread should join it
	 * @param v true if it should join, else false
	 */
	void setJoinThread(int v);

	/**
	 * Listen to key event to stop acquiring
	 * when using acquire command
	 */
	int kbhit(void);


	/** Multi detector Id */
	int detId;

	/** Shared Memory object */
	SharedMemory* sharedMemory;

	/** Shared memory structure */
	sharedMultiSlsDetector *thisMultiDetector;

	/** pointers to the slsDetector structures */
	std::vector <slsDetector*> detectors;

	/** data streaming (down stream) enabled in client (zmq sckets created) */
	bool client_downstream;

	/** ZMQ Socket - Receiver to Client */
	std::vector <ZmqSocket*> zmqSocket;

	/** Threadpool */
	ThreadPool* threadpool;


	/** semaphore to let postprocessing thread continue for next scan/measurement */
	sem_t sem_newRTAcquisition;

	/** semaphore to let main thread know it got all the dummy packets (also from ext. process) */
	sem_t sem_endRTAcquisition;

	int totalProgress;
	int progressIndex;
	/** mutex to synchronize main and data processing threads */
	pthread_mutex_t mp;

	/** mutex to synchronizedata processing and plotting threads */
	pthread_mutex_t mg;

	/** mutex to synchronize slsdetector threads */
	pthread_mutex_t ms;

	int threadedProcessing;

	/** sets when the acquisition is finished */
	int jointhread;

	/** set when detector finishes acquiring */
	int acquiringDone;

	/** the data processing thread */
	pthread_t dataProcessingThread;

	double *fdata;
	detectorData *thisData;

	int (*acquisition_finished)(double,int,void*);
	void *acqFinished_p;

	int (*measurement_finished)(int,int,void*);
	void *measFinished_p;

	int (*progress_call)(double,void*);
	void *pProgressCallArg;

	int (*dataReady)(detectorData*,int, int, void*);
	void *pCallbackArg;






};



#endif

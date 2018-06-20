/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

 ********************************************************************/



#ifndef MULTI_SLS_DETECTOR_H
#define MULTI_SLS_DETECTOR_H

#include "slsDetectorUtils.h"
class slsDetector;
class SharedMemory;
class ThreadPool;
class ZmqSocket;


#include <vector>
#include <string>


#define MULTI_SHMVERSION	0x180618
#define SHORT_STRING_LENGTH	50
#define DATE_LENGTH			29

/**
 @libdoc The multiSlsDetector class is used to operate several slsDetectors in parallel.
 * @short This is the base class for multi detector system functionalities
 * @author Anna Bergamaschi
 */

class multiSlsDetector  : public slsDetectorUtils {


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

		/** position of the master detector */
		int masterPosition;

		/** type of synchronization between detectors */
		synchronizationMode syncMode;

		/** Detectors offset in the X direction (in number of channels)*/
		int offsetX[MAXDET];

		/** Detectors offsets  in the Y direction (in number of channels) */
		int offsetY[MAXDET];

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

		/** indicator for the acquisition progress - set to 0 at the beginning
		 * of the acquisition and incremented every time that the data are written
		 * to file */
		int progressIndex;

		/** total number of frames to be acquired */
		int totalProgress;

		/** current index of the output file */
		int fileIndex;

		/** name root of the output files */
		char fileName[MAX_STR_LENGTH];

		/** path of the output files */
		char filePath[MAX_STR_LENGTH];

		/** max frames per file */
		int framesPerFile;

		/** file format*/
		fileFormat fileFormatType;

		/** corrections  to be applied to the data \see ::correctionFlags */
		int correctionMask;

		/** threaded processing flag (i.e. if data are processed and written to
		 * file in a separate thread)  */
		int threadedProcessing;

		/** dead time (in ns) for rate corrections */
		double tDead;

		/** directory where the flat field files are stored */
		char flatFieldDir[MAX_STR_LENGTH];

		/** file used for flat field corrections */
		char flatFieldFile[MAX_STR_LENGTH];

		/** file with the bad channels */
		char badChanFile[MAX_STR_LENGTH];

		/** angular conversion file */
		char angConvFile[MAX_STR_LENGTH];

		/** angular direction (1 if it corresponds to the encoder direction
		 * i.e. channel 0 is 0, maxchan is positive high angle, 0 otherwise  */
		int angDirection;

		/** beamline fine offset (of the order of mdeg, might be adjusted for
		 * each measurements)  */
		double fineOffset;

		/** beamline offset (might be a few degrees beacuse of encoder offset -
		 * normally it is kept fixed for a long period of time)  */
		double globalOffset;

		/** bin size for data merging */
		double binSize;

		//X and Y displacement
		double sampleDisplacement[2];

		/** number of positions at which the detector should acquire  */
		int numberOfPositions;

		/** list of encoder positions at which the detector should acquire */
		double detPositions[MAXPOS];

		/** Scans and scripts */
		int actionMask;

		mystring actionScript[MAX_ACTIONS];
		mystring actionParameter[MAX_ACTIONS];
		int scanMode[MAX_SCAN_LEVELS];
		mystring scanScript[MAX_SCAN_LEVELS];
		mystring scanParameter[MAX_SCAN_LEVELS];
		int nScanSteps[MAX_SCAN_LEVELS];
		mysteps scanSteps[MAX_SCAN_LEVELS];
		int scanPrecision[MAX_SCAN_LEVELS];

		/** flag for acquiring */
		bool acquiringFlag;

		/** external gui */
		bool externalgui;

		/** receiver online flag - is set if the receiver is connected,
		 * unset if socket connection is not possible  */
		int receiverOnlineFlag;

		/** data streaming (up stream) enable in receiver */
		bool receiver_upstream;

	} sharedMultiSlsDetector;




public:



	using slsDetectorUtils::flatFieldCorrect;
	using slsDetectorUtils::rateCorrect;
	using slsDetectorUtils::setBadChannelCorrection;
	using slsDetectorUtils::readAngularConversion;
	using slsDetectorUtils::writeAngularConversion;

	/*
     @short Structure allocated in shared memory to store detector settings and
     be accessed in parallel by several applications

	 */


	/**
	 * Constructor
	 * @param id multi detector id
	 * @param verify true to verify if shared memory size matches existing one
	 * @param update true to update last user pid, date etc
	 */
	multiSlsDetector(int id = 0, bool verify = true, bool update = true);

	/**
	 * Destructor
	 */
	virtual ~multiSlsDetector();

	/**
	 * returns true. Used when reference is slsDetectorUtils and to determine
	 * if command can be implemented as slsDetector/multiSlsDetector object/
	 */
	bool isMultiSlsDetectorClass();

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
	 * Loop serially through all the detectors in calling a particular method
	 * with an extra argument
	 * @param somefunc function pointer
	 * @param value argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename P1>
	T parallelCallDetectorMember(T (slsDetector::*somefunc)(P1), P1 value);

	/**
	 * Loop serially through all the detectors in calling a particular method
	 * with two extra arguments
	 * @param somefunc function pointer
	 * @param par1 argument for calling method
	 * @param par2 second argument for calling method
	 * @returns -1 if values are different, otherwise result in calling method
	 */
	template<typename T, typename P1, typename P2>
	T parallelCallDetectorMember(T (slsDetector::*somefunc)(P1, P2), P1 par1, P2 par2);

	/**
	 * Loop serially through all the detectors in calling a particular method
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
	 * Loop serially through all results and
	 * return a value if they are all same, else return -1
	 * @param return_values vector of results
	 * @returns -1 if values are different, otherwise result
	 */
	template<typename T>
	T minusOneIfDifferent(const std::vector<T>&);

	/**
	 * Calculate the detector position index in multi vector and the module index
	 * using an index for all entire modules in list (Mythen)
	 * @param i position index of all modules in list
	 * @param idet position index in multi vector list
	 * @param imod module index in the sls detector
	 */
	int decodeNMod(int i, int &idet, int &imod);

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
	 * Decode data from the detector converting them to an array of doubles,
	 * one for each channel (Mythen only)
	 * @param datain data from the detector
	 * @param nn size of datain array
	 * @param fdata double array of decoded data
	 * @returns pointer to a double array with a data per channel
	 */
	double* decodeData(int *datain, int &nn, double *fdata=NULL);

	/**
	 * Writes a data file
	 * @param name of the file to be written
	 * @param data array of data values
	 * @param err array of errors on the data. If NULL no errors will be written
	 * @param ang array of angular values. If NULL data will be in the form
	 * chan-val(-err) otherwise ang-val(-err)
	 * @param dataformat format of the data: can be 'i' integer or 'f' double (default)
	 * @param nch number of channels to be written to file. if -1 defaults to
	 * the number of installed channels of the detector
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 * \sa mythenDetector::writeDataFile
	 */
	int writeDataFile(std::string fname, double *data, double *err=NULL,
			double *ang=NULL, char dataformat='f', int nch=-1);

	/**
	 * Writes a data file with an integer pointer to an array
	 * @param name of the file to be written
	 * @param data array of data values
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 * \sa mythenDetector::writeDataFile
	 */
	int writeDataFile(std::string fname, int *data);

	/**
	 * Reads a data file
	 * @param name of the file to be read
	 * @param data array of data values to be filled
	 * @param err array of arrors on the data. If NULL no errors are expected
	 * on the file
	 * @param ang array of angular values. If NULL data are expected in the
	 * form chan-val(-err) otherwise ang-val(-err)
	 * @param dataformat format of the data: can be 'i' integer or 'f' double (default)
	 * @param nch number of channels to be written to file. if <=0 defaults
	 * to the number of installed channels of the detector
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 *\sa mythenDetector::readDataFile
	 */
	int readDataFile(std::string fname, double *data, double *err=NULL,
			double *ang=NULL, char dataformat='f');


	/**
	 * Reads a data file
	 * @param name of the file to be read
	 * @param data array of data values
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 * \sa mythenDetector::readDataFile
	 */
	int readDataFile(std::string fname, int *data);

	/**
	 * Checks error mask and returns error message and its severity if it exists
	 * @param critical is 1 if any of the messages is critical
	 * @returns error message else an empty std::string
	 */
	std::string getErrorMessage(int &critical);

	/**
	 * Clears error mask of both multi and sls
	 * @returns error mask
	 */
	int64_t clearAllErrorMask();

	/**
	 * Set Error Mask from all detectors
	 * if each had errors in the mask already
	 */
	void setErrorMaskFromAllDetectors();

	/**
     Set acquiring flag in shared memory
     \param b acquiring flag
	 */
	void setAcquiringFlag(bool b=false);

	/**
     Get acquiring flag from shared memory
     \returns acquiring flag
	 */
	bool getAcquiringFlag();

	/**
	 * Check if acquiring flag is set, set error if set
	 * \returns FAIL if not ready, OK if ready
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
	 * Get sls detector object from position in detectors array
	 * @param pos position in detectors array
	 * @returns pointer to sls detector object
	 */
	slsDetector* getSlsDetector(unsigned int pos);

	/**
	 * Accessing the sls detector from the multi list using position
	 * @param pos position in the multi list
	 * @returns slsDetector object
	 */
	slsDetector *operator();

	/**
	 * Free shared memory from the command line
	 * avoiding creating the constructor classes and mapping
	 * @param multiId multi detector Id
	 */
	static void freeSharedMemory(int multiId);

	/**
	 * Free shared memory and delete shared memory structure
	 * occupied by the sharedMultiSlsDetector structure
	 */
	void freeSharedMemory();

	/**
	 * Get user details of shared memory
	 * @returns string with user details
	 */
	std::string getUserDetails();

	/**
	 * Sets the hostname of all sls detectors in shared memory
	 * @param s concatenated hostname of all the sls detectors
	 */
	void setHostname(std::string s);

	/**
	 * Gets the hostname of detector at particular position
	 * or concatenated hostnames of all the sls detectors
	 * @param pos position of detector in array, -1 for all detectors
	 * @returns concatenated hostnames of all detectors or hostname of specific one
	 */
	std::string getHostname(int pos = -1);


	using slsDetectorBase::getDetectorType;
	/**
	 * Get Detector type for a particular sls detector or get the first one
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, returns the first det type
	 */
	detectorType getDetectorsType(int pos = -1);

	/**
	 * Concatenates string types of all sls detectors or
	 * returns the detector type of the first sls detector
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, concatenates
	 */
	std::string sgetDetectorsType(int pos=-1);

	/**
	 * Just to overload getDetectorType
	 * Concatenates string types of all sls detectors or
	 * returns the detector type of the first sls detector
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, concatenates
	 */
	std::string getDetectorType();

	/**
	 * Creates all the threads in the threadpool
	 * throws an exception if it cannot create threads
	 */
	int createThreadPool();

	/**
	 * Destroys all the threads in the threadpool
	 */
	void destroyThreadPool();

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
	 * Returns sum of all modules per sls detector (Mythen)
	 * Other detectors, it is 1
	 * @returns sum of all modules per sls detector
	 */
	int getNMods();

	/**
	 * Returns sum of all modules per sls detector in dimension d (Mythen)
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns sum of all modules per sls detector in dimension d
	 */
	int getNMod(dimension d);

	/**
	 * Returns sum of all maximum modules per sls detector (Mythen).
	 * Other detectors, it is 1
	 * @returns sum of all maximum modules per sls detector
	 */
	int getMaxMods();


	/**
	 * Returns sum of all maximum modules per sls detector in dimension d (Mythen)
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns sum of all maximum modules per sls detector in dimension d
	 */
	int getMaxMod(dimension d);

	/**
	 * Returns the sum of all maximum modules per sls detector in dimension d (Mythen).
	 * from the detector directly.
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns sum of all maximum modules per sls detector in dimension d
	 */
	int getMaxNumberOfModules(dimension d=X);

	/**
	 * Sets the sum of all modules per sls detector in dimension d (Mythen).
	 * from the detector directly.
	 * Other detectors, it is 1
	 * @param i the number of modules to set to
	 * @param d dimension d
	 * @returns sum of all modules per sls detector in dimension d
	 */
	int setNumberOfModules(int i=-1, dimension d=X);

	/**
	 * Calculates the position of detector based on module number of entire multi
	 * detector list and returns the number of channels per that module (Mythen)
	 * @param imod module number of entire multi detector list
	 * @returns number of channels per module imod
	 */
	int getChansPerMod(int imod=0);

	/**
	 * Returns the total number of channels of all sls detectors
	 * @returns the total number of channels of all sls detectors
	 */
	int getTotalNumberOfChannels();

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 */
	int getTotalNumberOfChannels(dimension d);

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels
	 */
	int getTotalNumberOfChannelsInclGapPixels(dimension d);

	/**
	 * Returns the maximum number of channels of all sls detectors (Mythen)
	 * @returns the maximum number of channels of all sls detectors
	 */
	int getMaxNumberOfChannels();

	/**
	 * Returns the maximum number of channels of all sls detectors in dimension d (Mythen)
	 * @param d dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	int getMaxNumberOfChannels(dimension d);

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels (Mythen)
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 * including gap pixels
	 */
	int getMaxNumberOfChannelsInclGapPixels(dimension d);

	/**
	 * Returns the maximum number of channels of all sls detectors in each dimension d,
	 * multi detector shared memory variable to calculate offsets for each sls detector
	 * @param d dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	int getMaxNumberOfChannelsPerDetector(dimension d);

	/**
	 * Sets the maximum number of channels of all sls detectors in each dimension d,
	 * multi detector shared memory variable to calculate offsets for each sls detector
	 * @param d dimension d
	 * @param i maximum number of channels for multi structure in dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	int setMaxNumberOfChannelsPerDetector(dimension d,int i);

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
	int setOnline(int const online=GET_ONLINE_FLAG);

	/**
	 * Checks if each of the detectors are online/offline
	 * @returns empty string if they are all online,
	 * else returns concatenation of strings of all detectors that are offline
     */
	std::string checkOnline();

	/**
	 * Set TCP Port of detector or receiver
	 * @param t port type
	 * @param p port number
	 * @returns port number
	 */
	int setPort(portType t, int p);

	/**
	 * Lock server for this client IP
	 * @param p 0 to unlock, 1 to lock
	 * @returns 1 for locked or 0 for unlocked
	 */
	int lockServer(int p);

	/**
	 * Get last client IP saved on detector server
	 * @returns last client IP saved on detector server
	 */
	std::string getLastClientIP();

	/**
	 * Exit detector server
	 * @returns OK or FAIL
	 */
	int exitServer();

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
	std::string getSettingsFile();

	/**
	 * Get detector settings
	 * @param ipos position in multi list (-1 all)
	 * @returns current settings
	 */
	detectorSettings getSettings(int pos=-1);

	/**
	 * Load detector settings from the settings file picked from the trimdir/settingsdir
	 * @param isettings settings
	 * @param ipos position in multi list (-1 all)
	 * @returns current settings
	 */
	detectorSettings setSettings(detectorSettings isettings, int pos=-1);

	/**
	 * Returns the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @returns the trimbit/settings directory
	 */
	std::string getSettingsDir();

	/**
	 * Sets the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @param s trimbits/settings directory
	 * @returns the trimbit/settings directory
	 */
	std::string setSettingsDir(std::string s);

	/**
	 * Returns the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @returns the calibration files directory
	 */
	std::string getCalDir();

	/**
	 * Sets the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @param s the calibration files directory
	 * @returns the calibration files directory
	 */
	std::string setCalDir(std::string s);

	/**
	 * Loads the modules settings/trimbits reading from a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module index of the entire list,
	 * from which will be calculated the detector index and the module index (-1 for all)
	 * returns OK or FAIL
	 */
	int loadSettingsFile(std::string fname, int imod=-1);

	/**
	 * Saves the modules settings/trimbits to a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module index of the entire list,
	 * from which will be calculated the detector index and the module index (-1 for all)
	 * returns OK or FAIL
	 */
	int saveSettingsFile(std::string fname, int imod=-1);

	/**
	 * Loads the modules calibration data reading from a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module index of the entire list,
	 * from which will be calculated the detector index and the module index (-1 for all)
	 * returns OK or FAIL
	 */
	int loadCalibrationFile(std::string fname, int imod=-1);

	/**
	 * Saves the modules calibration data to a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module index of the entire list,
	 * from which will be calculated the detector index and the module index (-1 for all)
	 * returns OK or FAIL
	 */
	int saveCalibrationFile(std::string fname, int imod=-1);

	/**
	 * (Not implemented in any detector from the client)
	 * Sets/gets the detector in position i as master of the structure
	 * (e.g. it gates the other detectors and therefore must be started as last.
	 * Assumes that signal 0 is gate in, signal 1 is trigger in, signal 2 is gate out
	 * @param i position of master (-1 gets, -2 unset)
	 * @return master's position (-1 none)
	 */
	int setMaster(int i=-1);

	/** (Not implemented in any detector from the client)
	 * Sets/gets the synchronization mode of the various detector
	 * @param sync syncronization mode
	 * @returns current syncronization mode
	 */
	synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE);

	/**
	 * Get Detector run status
	 * @returns status
	 */
	runStatus  getRunStatus();

	/**
	 * Prepares detector for acquisition (Eiger and Gotthard)
	 * For Gotthard, it sets the detector data transmission mode (CPU or receiver)
	 * @returns OK if all detectors are ready for acquisition, FAIL otherwise
	 */
	int prepareAcquisition();

	/**
	 * Cleans up after acquisition (Gotthard only)
	 * For Gotthard, it sets the detector data transmission to default (via CPU)
	 * @returns OK or FAIL
	 */
	int cleanupAcquisition();

	/**
	 * Start detector acquisition (Non blocking)
	 * @returns OK or FAIL if even one does not start properly
	 */
	int startAcquisition();

	/**
	 * Stop detector acquisition
	 * @returns OK or FAIL
	 */
	int stopAcquisition();

	/**
	 * Start readout (without exposure or interrupting exposure) (Mythen)
	 * @returns OK or FAIL
	 */
	int startReadOut();

	/**
	 * Start detector acquisition and read all data (Blocking until end of acquisition)
	 * (Mythen, puts all data into a data queue. Others, data at receiver via udp packets)
	 * @returns pointer to the front of the data queue (return significant only for Mythen)
	 * \sa startAndReadAllNoWait getDataFromDetector dataQueue
	 */
	int* startAndReadAll();

	/**
	 * Start detector acquisition and call read out, but not reading (data for Mythen,
	 * and status for other detectors) from the socket.
	 * (startAndReadAll calls this and getDataFromDetector. Client is not blocking,
	 * but server is blocked until getDataFromDetector is called. so not recommended
	 * for users)
	 * @returns OK or FAIL
	 */
	int startAndReadAllNoWait();

	/**
	 * Reads from the detector socket (data frame for Mythen and status for other
	 * detectors)
	 * @returns pointer to the data or NULL. If NULL disconnects the socket
	 * (return significant only for Mythen)
	 * Other detectors return NULL
	 * \sa getDataFromDetector
	 */
	int* getDataFromDetector();

	/**
	 * Requests and receives a single data frame from the detector
	 * (Mythen: and puts it in the data queue)
	 * @returns pointer to the data or NULL. (return Mythen significant)
	 *  Other detectors return NULL
	 * \sa getDataFromDetector
	 */
	int* readFrame();

	/**
	 * Requests and  receives all data from the detector
	 * (Mythen: and puts them in a data queue)
	 * @returns pointer to the front of the queue or NULL (return Mythen significant)
	 * Other detectors return NULL
	 * \sa getDataFromDetector  dataQueue
	 */
	int* readAll();

	/**
	 * Pops the data from the data queue (Mythen)
	 * @returns pointer to the popped data  or NULL if the queue is empty.
	 * \sa  dataQueue
	 */
	int* popDataQueue();

	/**
	 * Pops the data from the postprocessed data queue (Mythen)
	 * @returns pointer to the popped data  or NULL if the queue is empty.
	 * \sa  finalDataQueue
	 */
	detectorData* popFinalDataQueue();

	/**
	 * Resets the raw data queue (Mythen)
	 * \sa  dataQueue
	 */
	void resetDataQueue();

	/**
	 * Resets the post processed  data queue (Mythen)
	 * \sa  finalDataQueue
	 */
	void resetFinalDataQueue();

	/**
	 * Configures in detector the destination for UDP packets (Not Mythen)
	 * @returns OK or FAIL
	 */
	int configureMAC();

	/**
	 * Get threshold energy (Mythen and Eiger)
	 * @param imod module number (-1 all)
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int getThresholdEnergy(int imod=-1);

	/**
	 * Set threshold energy (Mythen and Eiger)
	 * @param e_eV threshold in eV
	 * @param imod module number (-1 all)
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS,int tb=1);

	/**
	 * Set/get timer value (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t setTimer(timerIndex index, int64_t t=-1);

	/**
	 * Set/get timer value left in acquisition (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t getTimeLeft(timerIndex index);

	/**
	 * Set speed
	 * @param sp speed type  (clkdivider option for Jungfrau and Eiger, others for Mythen/Gotthard)
	 * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other values check manual
	 * @returns value of speed set
	 */
	int setSpeed(speedVariable sp, int value=-1);






	/**
      set/get dynamic range and updates the number of dataBytes
      \param n dynamic range (-1 get)
      \param pos detector position (-1 all detectors)
      \returns current dynamic range
      updates the size of the data expected from the detector
      \sa sharedSlsDetector
	 */
	int setDynamicRange(int n, int pos);

	int setDynamicRange(int i=-1);

	int getDataBytes();


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
     \param val value (in V)
     \param index DAC index
     \param imod module number (if -1 alla modules)
     \returns current DAC value (temperature for eiger and jungfrau in millidegrees)
	 */
	dacs_t getADC(dacIndex index, int imod=-1);

	externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE);

	externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0);

	int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS);



	uint32_t writeRegister(uint32_t addr, uint32_t val);

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
	std::string setNetworkParameter(networkParameter, std::string);

	std::string getNetworkParameter(networkParameter);





	int digitalTest(digitalTestMode mode, int imod=0);
	int executeTrimming(trimMode mode, int par1, int par2, int imod=-1);

	/**
     Loads dark image or gain image to the detector
     \param index can be DARK_IMAGE or GAIN_IMAGE
     \fname file name to load data from
     \returns OK or FAIL
	 */
	int loadImageToDetector(imageType index,std::string const fname);



	/**
     writes the counter memory block from the detector
     \param startACQ is 1 to start acquisition after reading counter
     \param fname file name to load data from
     \returns OK or FAIL
	 */
	int writeCounterBlockFile(std::string const fname,int startACQ=0);


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
 	 verifies that min is less than max
 	 \param n number of rois
 	 \param r array of rois
	 */
	void verifyMinMaxROI(int n, ROI r[]);

	/**
      set roi
       \param n number of rois
       \param roiLimits array of roi
       \returns success or failure
	 */
	int setROI(int n=-1,ROI roiLimits[]=NULL);

	/**
   	get roi from each detector and convert it to the multi detector scale
   	\param n number of rois
   	\returns an array of multidetector's rois
	 */
	ROI* getROI(int &n);


	int writeAdcRegister(int addr, int val);

	/**
	 * Activates the detector (Eiger only)
	 * @param enable active (1) or inactive (0), -1 gets
	 * @returns 0 (inactive) or 1 (active)
	 */
	int activate(int const enable=GET_ONLINE_FLAG);

	/** returns the enable if data will be flipped across x or y axis
	 *  \param d axis across which data is flipped
	 *  returns 1 or 0
	 */
	int getFlippedData(dimension d=X);

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

	int setTrimEn(int nen, int *en=NULL);
	int getTrimEn(int *en=NULL);

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
	int programFPGA(std::string fname);

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




	double getScanStep(int index, int istep);


	/**
     Returns the trimbits from the detector's shared memmory
     \param retval is the array with the trimbits
     \param fromDetector is true if the trimbits shared memory have to be uploaded from detector
     \returns the total number of channels for the detector
	 */
	int getChanRegs(double* retval,bool fromDetector);



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




	int getMoveFlag(int imod);

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
		int setFlatFieldCorrection(std::string fname="");

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
	      set bad channels correction
	      \param fname file with bad channel list ("" disable)
	      \returns 0 if bad channel disabled, >0 otherwise
		 */
		int setBadChannelCorrection(std::string fname="");

		int setBadChannelCorrection(int nch, int *chs, int ff);


		/**
	      get bad channels correction
	      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
	      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
		 */
		int getBadChannelCorrection(int *bad=NULL);


		int readAngularConversionFile(std::string fname);

		int writeAngularConversion(std::string fname);

		/**
	      pure virtual function
	      get angular conversion
	      \param reference to diffractometer direction
	      \param angconv array that will be filled with the angular conversion constants
	      \returns 0 if angular conversion disabled, >0 otherwise
	      \sa mythenDetector::getAngularConversion
		 */
		int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL) ;


		/**
	     sets the value of s angular conversion parameter
	     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
	     \param v the value to be set
	     \returns the actual value
		 */

		double setAngularConversionParameter(angleConversionParameter c, double v);

		angleConversionConstant *getAngularConversionPointer(int imod=0);

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
	std::string checkReceiverOnline();

	/** Locks/Unlocks the connection to the receiver
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the receiver
	 */
	int lockReceiver(int lock=-1);

	/**
     Returns the IP of the last client connecting to the receiver
	 */
	std::string getReceiverLastClientIP();

	/**
      Turns off the receiver server!
	 */
	int exitReceiver();

	/**
     \returns file dir
	 */
	std::string getFilePath();

	/**
     Sets up the file directory
     @param s file directory
     \returns file dir
	 */
	std::string setFilePath(std::string s="");

	/**
     \returns file name
	 */
	std::string getFileName();

	/**
     Sets up the file name
     @param s file name
     \returns file name
	 */
	std::string setFileName(std::string s="");

	/**
     Sets the max frames per file in receiver
     @param f max frames per file
     \returns max frames per file in receiver
	 */
	int setReceiverFramesPerFile(int f = -1);


	/**
     \returns file name
	 */
	fileFormat getFileFormat();

	/**
     Sets up the file format
     @param f file format
     \returns file format
	 */
	fileFormat setFileFormat(fileFormat f=GET_FILE_FORMAT);

	/**
     \returns file index
	 */
	int getFileIndex();

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
	int getFramesCaughtByAnyReceiver();

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
	 * Create Receiving Data Sockets
	 * @param destroy is true to destroy all the sockets
	 * @return OK or FAIL
	 */
	int createReceivingDataSockets(const bool destroy = false);



	/** Reads frames from receiver through a constant socket
	 */
	void readFrameFromReceiver();



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
	int enableDataStreamingToClient(int enable=-1);

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









	/******** CTB funcs */

	/** opens pattern file and sends pattern to CTB
      @param fname pattern file to open
      @returns OK/FAIL
	 */
	int setCTBPattern(std::string fname);


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






private:
	/**
	 * Initialize (open/create) shared memory for the sharedMultiDetector structure
	 * @param verify true to verify if shm size matches existing one
	 * @param update true to update last user pid, date etc
	 */
	bool initSharedMemory(bool verify = true);

	/**
	 * Initialize detector structure
	 * @param created true if shared memory was just created now
	 * @param verify true to verify if shm size matches existing one
	 */
	void initializeDetectorStructure(bool created, bool verify = true);

	/**
	 * Initialize class members (and from slsDetectorUtils)
	 */
	void initializeMembers();

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
};



#endif

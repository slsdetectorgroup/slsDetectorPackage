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

#define SLS_SHMVERSION	0x180629
#define NMODMAXX 24
#define NMODMAXY 24
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16
/**
 * parameter list that has to be initialized depending on the detector type
 */
typedef  struct detParameterList {
	int nModMaxX;
	int nModMaxY;
	int nChanX;
	int nChanY;
	int nChipX;
	int nChipY;
	int nDacs;
	int nAdcs;
	int nGain;
	int nOffset;
	int dynamicRange;
	int moveFlag;
	int nGappixelsX;
	int nGappixelsY;
} detParameterList;


class slsDetector : public slsDetectorUtils, public energyConversion {

private:
	/**
	 * @short structure allocated in shared memory to store detector settings for IPC and cache
	 */
	typedef  struct sharedSlsDetector {

		/* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

		/** shared memory version */
		int shmversion;

		/** online flag - is set if the detector is connected, unset if socket
		 * connection is not possible  */
		int onlineFlag;

		/** stopped flag - is set if an acquisition error occurs or the detector
		 * is stopped manually. Is reset to 0 at the start of the acquisition */
		int stoppedFlag;

		/** is the hostname (or IP address) of the detector. needs to be set
		 * before starting the communication */
		char hostname[MAX_STR_LENGTH];

		/** END OF FIXED PATTERN -----------------------------------------------*/




		/** Detector offset in the X & Y direction in the multi detector structure */
		int offset[2];

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
		int trimEnergies[MAX_TRIMEN];

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

		/** receiver frames discard policy */
		frameDiscardPolicy receiver_frameDiscardMode;

		/** receiver partial frames padding enable */
		bool receiver_framePadding;

		/** activated receiver */
		bool activated;

		/** padding enable in deactivated receiver */
		bool receiver_deactivatedPaddingEnable;

		/** silent receiver */
		bool receiver_silentMode;

	} sharedSlsDetector;





public:

	using slsDetectorUtils::getDetectorType;
	using postProcessing::flatFieldCorrect;
	using postProcessing::rateCorrect;
	using postProcessing::setBadChannelCorrection;
	using angularConversion::readAngularConversion;
	using angularConversion::writeAngularConversion;
	using slsDetectorUtils::getAngularConversion;

	//FIXME: all pos or id arguments needed only for same multi signature

	/**
	 * Constructor called when creating new shared memory
	 * @param type detector type
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 * @param m multiSlsDetector reference
	 */
	slsDetector(detectorType type, int multiId = 0, int id = 0, bool verify = true, multiSlsDetector* m = NULL);

	/**
	 * Constructor called when opening existing shared memory
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 * @param m multiSlsDetector reference
	 */
	slsDetector(int multiId = 0, int id = 0, bool verify = true, multiSlsDetector* m = NULL);

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
	 * Is only safe to call if one deletes the slsDetector object afterward
	 * and frees multi shared memory/updates thisMultiDetector->numberOfDetectors
	 */
	void freeSharedMemory();

	/**
	 * Get user details of shared memory
	 * Should only be called from multi detector level
	 * @returns string with user details
	 */
	std::string getUserDetails();

	/**
	 * Sets the hostname of all sls detectors in shared memory
	 * Connects to them to set up online flag
	 * @param name hostname
	 */
	void setHostname(const char *name);

	/**
	 * Gets the hostname of detector
	 * @param pos insignificant
	 * @returns hostname
	 */
	std::string getHostname(int pos = -1);

	/**
	 * Appends detectors to the end of the list in shared memory
	 * Connects to them to set up online flag
	 * Should only be called from multi detector level
	 * @param name concatenated hostname of the sls detectors to be appended to the list
	 */
	 void addMultipleDetectors(const char* name);

	/**
	 * Connect to the control port
	 * @returns OK, FAIL or undefined
	 */
	int connectControl();

	/**
	 * Disconnect the control port
	 */
	void disconnectControl();

	/**
	 * Connect to the data port
	 * @returns OK, FAIL or undefined
	 */
	int connectData();

	/**
	 * Disconnect the data port
	 */
	void disconnectData();

	/**
	 * Connect to the stop port
	 * @returns OK, FAIL or undefined
	 */
	int connectStop();

	/**
	 * Disconnect the stop port
	 */
	void disconnectStop();

	/**
	 * Get detector type by connecting to the detector without creating an object
	 * @param name hostname of detector
	 * @param cport TCP control port
	 * @returns detector tpe or GENERIC if failed
	 */
	static detectorType getDetectorType(const char *name, int cport=DEFAULT_PORTNO);

	/**
	 * Gets detector type from detector and set it in receiver
	 * @param type the detector type
	 * @returns detector type in receiver
	 */
	int setDetectorType(detectorType type=GET_DETECTOR_TYPE);

	/**
	 * Gets detector type (string) from detector and set it in receiver
	 * @param type string of detector type
	 * @returns detector type in receiver
 	 */
	int setDetectorType(std::string stype);

	/**
	 * Get Detector type from shared memory variable
	 * @param pos insignificant
	 * @returns detector type from shared memory variable
	 */
	detectorType getDetectorsType(int pos = -1);

	/**
	 * Gets string version of detector type from shared memory variable
	 * @param pos insignificant
	 * @returns string version of detector type from shared memory variable
	 */
	std::string sgetDetectorsType(int pos=-1);

	/**
	 * Just to overload getDetectorType from users
	 * Gets string version of detector type from shared memory variable
	 * @returns gets string version of detector type from shared memory variable
	 */
	std::string getDetectorType();

	/**
	 * Returns number of modules from shared memory (Mythen)
	 * Other detectors, it is 1
	 * @returns number of modules
	 */
	int getNMods();

	/**
	 * Returns number of modules in dimension d from shared memory (Mythen)
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns number of modules in dimension d
	 */
	int getNMod(dimension d);

	/**
	 * Returns maximum number of modules from shared memory (Mythen)
	 * Other detectors, it is 1
	 * @returns maximum number of modules
	 */
	int getMaxMods();

	/**
	 * Returns maximum number of modules  in dimension d  from shared memory (Mythen)
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns maximum number of modules  in dimension d
	 */
	int getNMaxMod(dimension d);

	/**
	 * Returns maximum number of modules in dimension d (Mythen)
	 * from the detector directly.
	 * Other detectors, it is 1
	 * @param d dimension d
	 * @returns maximum number of modules in dimension d
	 */
	int getMaxNumberOfModules(dimension d=X); //

	/**
	 * Sets/Gets the number of modules in dimension d (Mythen)
	 * from the detector directly.
	 * Other detectors, it is 1
	 * @param i the number of modules to set to (-1 gets)
	 * @param d dimension d
	 * @returns the number of modules in dimension d
	 */
	int setNumberOfModules(int n=GET_FLAG, dimension d=X);

	/**
	 * returns the number of channels per that module
	 * from shared memory (Mythen)
	 * @param imod insignificant
	 * @returns number of channels per module
	 */
	int getChansPerMod(int imod=0);

	/**
	 * returns the number of channels per that module in dimension d
	 * from shared memory (Mythen)
	 * @param d dimension d
	 * @param imod insignificant
	 * @returns number of channels per module in dimension d
	 */
	int getChansPerMod( dimension d,int imod=0);

	/**
	 * Returns the total number of channels from shared memory
	 * @returns the total number of channels
	 */
	int getTotalNumberOfChannels();

	/**
	 * Returns the total number of channels in dimension d from shared memory
	 * @param d dimension d
	 * @returns the total number of channels  in dimension d
	 */
	int getTotalNumberOfChannels(dimension d);

	/**
	 * Returns the total number of channels of in dimension d including gap pixels
	 * from shared memory
	 * @param d dimension d
	 * @returns the total number of channels including gap pixels in dimension d
	 * including gap pixels
	 */
	int getTotalNumberOfChannelsInclGapPixels(dimension d);

	/**
	 * Returns the maximum number of channels from shared memory (Mythen)
	 * @returns the maximum number of channels
	 */
	int getMaxNumberOfChannels();

	/**
	 * Returns the maximum number of channels in dimension d from shared memory (Mythen)
	 * @param d dimension d
	 * @returns the maximum number of channels in dimension d
	 */
	int getMaxNumberOfChannels(dimension d);

	/**
	 * Returns the maximum number of channels in dimension d from shared memory (Mythen)
	 * @param d dimension d
	 * @returns the maximum number of channels in dimension d
	 */
	int getMaxNumberOfChannelsInclGapPixels(dimension d);

	/**
	 * returns the number of channels per chip from shared memory (Mythen)
	 * @returns number of channels per chip
	 */
	int getNChans();

	/**
	 * returns the number of channels per chip in dimension d from shared memory (Mythen)
	 * @param d dimension d
	 * @returns number of channels per chip in dimension d
	 */
	int getNChans(dimension d);

	/**
	 * returns the number of chips per module from shared memory (Mythen)
	 * @returns number of chips per module
	 */
	int getNChips();

	/**
	 * returns the number of chips per module in dimension d from shared memory (Mythen)
	 * @param d dimension d
	 * @returns number of chips per module in dimension d
	 */
	int getNChips(dimension d);

	/**
	 * Get Detector offset from shared memory in dimension d
	 * @param d dimension d
	 * @returns offset in dimension d
	 */
	int getDetectorOffset(dimension d);

	/**
	 * Set Detector offset in shared memory in dimension d
	 * @param d dimension d
	 * @param off offset for detector
	 */
	void setDetectorOffset(dimension d, int off);

	/**
	 * Checks if the detector is online and sets the online flag
	 * @param online if GET_ONLINE_FLAG, only returns shared memory online flag,
	 * else sets the detector in online/offline state
	 * if OFFLINE_FLAG, (i.e. no communication to the detector - using only local structure - no data acquisition possible!);
	 * if ONLINE_FLAG, detector in online state (i.e. communication to the detector updating the local structure)
	 * @returns online/offline status
	 */
	int setOnline(int const online=GET_ONLINE_FLAG);

	/**
	 * Checks if each of the detector is online/offline
	 * @returns empty string if it is online
	 * else returns hostnameif it is offline
	 */
	std::string checkOnline();

	/**
	 * Configure the TCP socket communciation and initializes the socket instances
	 * @param name hostname, empty if current hostname
	 * @param control_port TCP port for control commands, -1 if current is used
	 * @param stop_port TCP port for data commands, -1 if current is used
	 * @returns OK or FAIL
	 * \sa sharedSlsDetector
	 */
	int setTCPSocket(std::string const name="", int const control_port=-1, int const stop_port=-1);


	/**
	 * Set/Gets TCP Port of detector or receiver
	 * @param t port type
	 * @param p port number (-1 gets)
	 * @returns port number
	 */
	int setPort(portType type, int num=-1);

	/**
	 * Returns the detector TCP control port  \sa sharedSlsDetector
	 * @returns the detector TCP control port
	 */
	int getControlPort();

	/**
	 * Returns the detector TCP stop port  \sa sharedSlsDetector
	 * @returns the detector TCP stop port
	 */
	int getStopPort();

	/**
	 * Returns the receiver TCP 	port  \sa sharedSlsDetector
	 * @returns the receiver TCP port
	 */
	int getReceiverPort();

	/**
	 * Lock server for this client IP
	 * @param p 0 to unlock, 1 to lock (-1 gets)
	 * @returns 1 for locked or 0 for unlocked
	 */
	int lockServer(int lock=-1);

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
	 * Executes a system command on the detector server
	 * e.g. mount an nfs disk, reboot and returns answer etc.
	 * @param cmd command to be executed
	 * @param answer is the answer from the detector
	 * @returns OK or FAIL
	 */
	int execCommand(std::string cmd, std::string answer);

	/**
	 * Updates some of the shared memory receiving the data from the detector
	 * @returns OK
	 */
	int updateDetectorNoWait();

	/**
	 * Updates soem of the shared memory receiving the data from the detector
	 * calls updateDetectorNoWait
	 * @returns OK or FAIL or FORCE_RET
	 */
	int updateDetector();

	/**
	 * Load configuration from a configuration File
	 * calls readConfigurationFile and gives it the stream
	 * @param fname configuration file name
	 * @return OK or FAIL
	 */
	int readConfigurationFile(std::string const fname);

	/**
	 * Load configuration from a stream
	 * @param infile stream
	 * @return OK or FAIL
	 */
	int readConfigurationFile(std::ifstream &infile);

	/**
	 * Write current configuration to a file
	 * calls writeConfigurationFile giving it a stream to write to
	 * @param fname configuration file name
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(std::string const fname);

	/**
	 * Write current configuration to a stream
	 * @param outfile outstream
	 * @param id detector id
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(std::ofstream &outfile, int id=-1);

	/**
	 * Returns the trimfile or settings file name (Useless??)
	 * @returns the trimfile or settings file name
	 */
	std::string getSettingsFile();

	/**
	 * Writes a trim/settings file for module number imod,
	 * the values will be read from the current detector structure
	 * @param fname name of the file to be written
	 * @param imod module number
	 * @param iodelay io delay (detector specific)
	 * @param tau tau (detector specific)
	 * @returns OK or FAIL if the file could not be written
	 * \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeSettingsFile(string, int)
	 */
	using energyConversion::writeSettingsFile;
	int writeSettingsFile(std::string fname, int imod, int iodelay, int tau);

	/**
	 * Get detector settings
	 * @param imod module number (-1 all)
	 * @returns current settings
	 */
	detectorSettings getSettings(int imod=-1);

	/**
	 * Load detector settings from the settings file picked from the trimdir/settingsdir
	 * Eiger only stores in shared memory ( a get will overwrite this)
	 * For Eiger, one must use threshold
	 * Gotthard, Propix, Jungfrau and Moench only sends the settings enum to the detector
	 * @param isettings settings
	 * @param imod module number (-1 all)
	 * @returns current settings
	 */
	detectorSettings setSettings(detectorSettings isettings, int imod=-1);

	/**
	 * Send detector settings only (set only for Jungfrau, Gotthard, Moench, get for all)
	 * Only the settings enum is sent to the detector, where it will
	 * initialize al the dacs already hard coded in the detector server
	 * @param isettings  settings
	 * @param imod module number (-1 all)
	 * @returns current settings
	 */
	detectorSettings sendSettingsOnly(detectorSettings isettings, int imod=-1);

	/**
	 * Get threshold energy (Mythen and Eiger)
	 * @param imod module number (-1 all)
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int getThresholdEnergy(int imod=-1);


	/**
	 * Set threshold energy (Mythen and Eiger)
	 * For Eiger, calls setThresholdEneryAndSettings
	 * @param e_eV threshold in eV
	 * @param imod module number (-1 all)
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS, int tb=1);

	/**
	 * Set threshold energy and settings (Eiger only)
	 * @param e_eV threshold in eV
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns OK if successful, else FAIL
	 */
	int setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings, int tb=1);

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
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int saveSettingsFile(std::string fname, int imod=-1);

	/**
	 * Loads the modules calibration data reading from a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int loadCalibrationFile(std::string fname, int imod=-1);

	/**
	 * Saves the modules calibration data to a specific file (Mythen)
	 * file name extension is automatically generated.
	 * @param fname specific calibration file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	int saveCalibrationFile(std::string fname, int imod=-1);

	/**
	 * Sets/gets the detector in position i as master of the structure (Mythen)
	 * (e.g. it gates the other detectors and therefore must be started as last.
	 * Assumes that signal 0 is gate in, signal 1 is trigger in, signal 2 is gate out
	 * @param i position of master (-1 gets, -2 unset)
	 * @return master's position (-1 none)
	 */
	masterFlags  setMaster(masterFlags flag);

	/**
	 * Sets/gets the synchronization mode of the various detector (Mythen)
	 * @param sync syncronization mode
	 * @returns current syncronization mode
	 */
	synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE);

	/**
	 * Calcualtes the total number of steps of the acquisition
	 * Called when number of frames, number of cycles, number of positions and scan steps change
	 * @returns the total number of steps of the acquisition
	 */
	int setTotalProgress();

	/**
	 * Returns the current progress in %
	 * @returns the current progress in %
	 */
	double getCurrentProgress();

	/**
	 * Get run status of the detector
	 * @returns the status of the detector
	 */
	runStatus  getRunStatus();

	/**
	 * Prepares detector for acquisition (Eiger and Gotthard)
	 * For Gotthard, it sets the detector data transmission mode (CPU or receiver)
	 * @returns OK or FAIL
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
	 * Give an internal software trigger to the detector (Eiger only)
	 * @return OK or FAIL
	 */
	int sendSoftwareTrigger();

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
	int* getDataFromDetector(int *retval=NULL);

	/**
	 * Requests and receives a single data frame from the detector
	 * (Mythen: and puts it in the data queue)
	 * @returns pointer to the data or NULL. (return Mythen significant)
	 *  Other detectors return NULL
	 * \sa getDataFromDetector
	 */
	int* readFrame();

	/**
	 * Receives all data from the detector
	 * (Mythen: and puts them in a data queue)
	 * @returns pointer to the front of the queue or NULL (return Mythen significant)
	 * Other detectors return NULL
	 * \sa getDataFromDetector  dataQueue
	 */
	int* readAll();

	/**
	 * Requests detector for all data, calls readAll afterwards
	 * (Mythen: and puts them in a data queue)
	 * @returns pointer to the front of the queue or NULL (return Mythen significant)
	 * Other detectors return NULL
	 * \sa getDataFromDetector  dataQueue
	 */
	int readAllNoWait();

	/**
	 * Configures in detector the destination for UDP packets (Not Mythen)
	 * @returns OK or FAIL
	 */
	int configureMAC();

	/**
	 * Set/get timer value (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * \param imod module number (pointless in slsDetector)
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t setTimer(timerIndex index, int64_t t=-1, int imod = -1);

	/**
	 * Set/get timer value left in acquisition (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @param imod module number
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	int64_t getTimeLeft(timerIndex index, int imod = -1);

	/**
	 * Set speed
	 * @param sp speed type  (clkdivider option for Jungfrau and Eiger, others for Mythen/Gotthard)
	 * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other values check manual
	 * @returns value of speed set
	 */
	int setSpeed(speedVariable sp, int value=-1);

	/**
	 * Set/get dynamic range and updates the number of dataBytes
	 * (Eiger: If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1)
	 * @param i dynamic range (-1 get)
	 * @returns current dynamic range
	 * \sa sharedSlsDetector
	 */
	int setDynamicRange(int n=-1);

	/**
	 * Recalculated number of data bytes
	 * @returns tota number of data bytes
	 */
	int getDataBytes();

	/**
	 * Recalculated number of data bytes including gap pixels
	 * @returns tota number of data bytes including gap pixels
	 */
	int getDataBytesInclGapPixels();

	/**
	 * Set/get dacs value
	 * @param val value (in V)
	 * @param index DAC index
	 * @param mV 0 in dac units or 1 in mV
	 * @param imod module number (if -1 all modules)
	 * @returns current DAC value
	 */
	dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1);

	/**
	 * Get adc value
	 * @param index adc(DAC) index
	 * @param imod module number (if -1 all modules)
	 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
	 */
	dacs_t getADC(dacIndex index, int imod=0);

	/**
	 * Set/get timing mode
	 * @param pol timing mode (-1 gets)
	 * @returns current timing mode
	 */
	externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE);

	/**
	 * Set/get external signal flags (to specify triggerinrising edge etc) (Gotthard, Mythen)
	 * @param pol external signal flag (-1 gets)
	 * @param signalindex singal index (0 - 3)
	 * @returns current timing mode
	 */
	externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0);

	/**
	 * Set/get readout flags (Eiger, Mythen)
	 * @param flag readout flag (Eiger options: parallel, nonparallel, safe etc.) (-1 gets)
	 * @returns readout flag
	 */
	int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS);

	/**
	 * Write in a register. For Advanced users
	 * @param addr address of register
	 * @param val value to write into register
	 * @returns value read after writing
	 */
	uint32_t writeRegister(uint32_t addr, uint32_t val);

	/**
	 * Read from a register. For Advanced users
	 * @param addr address of register
	 * @returns value read from register
	 */
	uint32_t readRegister(uint32_t addr);

	/**
	 * Set bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	uint32_t setBit(uint32_t addr, int n);

	/**
	 * Clear bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	uint32_t clearBit(uint32_t addr, int n);

	/**
	 * Set network parameter
	 * @param p network parameter type
	 * @param s network parameter value
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	std::string setNetworkParameter(networkParameter index, std::string value);

	/**
	 * Get network parameter
	 * @param p network parameter type
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	std::string getNetworkParameter(networkParameter index);

	/**
	 * Returns the detector MAC address\sa sharedSlsDetector
	 * @returns the detector MAC address
	 */
	std::string getDetectorMAC();

	/**
	 * Returns the detector IP address\sa sharedSlsDetector
	 * @returns the detector IP address
	 */
	std::string getDetectorIP();

	/**
	 * Returns the receiver IP address\sa sharedSlsDetector
	 * @returns the receiver IP address
	 */
	std::string getReceiver();

	/**
	 * Returns the receiver UDP IP address\sa sharedSlsDetector
	 * @returns the receiver UDP IP address
	 */
	std::string getReceiverUDPIP();

	/**
	 * Returns the receiver UDP MAC address\sa sharedSlsDetector
	 * @returns the receiver UDP MAC address
	 */
	std::string getReceiverUDPMAC();

	/**
	 * Returns the receiver UDP port\sa sharedSlsDetector
	 * @returns the receiver UDP port
	 */
	std::string getReceiverUDPPort();

	/**
	 * Returns the receiver UDP port 2 of same interface\sa sharedSlsDetector
	 * @returns the receiver UDP port 2 of same interface
	 */
	std::string getReceiverUDPPort2();

	/**
	 * Returns the client zmq port \sa sharedSlsDetector
	 * @returns the client zmq port
	 */
	std::string getClientStreamingPort();

	/**
	 * Returns the receiver zmq port \sa sharedSlsDetector
	 * @returns the receiver zmq port
	 */
	std::string getReceiverStreamingPort();

	/**
	 * Returns the client zmq ip \sa sharedSlsDetector
	 * @returns the client zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string getClientStreamingIP();

	/**
	 * Returns the receiver zmq ip \sa sharedSlsDetector
	 * @returns the receiver zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string getReceiverStreamingIP();

	/**
	 * Validates the format of the detector MAC address and sets it \sa sharedSlsDetector
	 * @param detectorMAC detector MAC address
	 * @returns the detector MAC address
	 */
	std::string setDetectorMAC(std::string detectorMAC);

	/**
	 * Validates the format of the detector IP address and sets it \sa sharedSlsDetector
	 * @param detectorIP detector IP address
	 * @returns the detector IP address
	 */
	std::string setDetectorIP(std::string detectorIP);

	/**
	 * Validates and sets the receiver.
	 * Also updates the receiver with all the shared memory parameters significant for the receiver
	 * Also configures the detector to the receiver as UDP destination
	 * @param receiver receiver hostname or IP address
	 * @returns the receiver IP address from shared memory
	 */
	std::string setReceiver(std::string receiver);

	/**
	 * Validates the format of the receiver UDP IP address and sets it \sa sharedSlsDetector
	 * @param udpip receiver UDP IP address
	 * @returns the receiver UDP IP address
	 */
	std::string setReceiverUDPIP(std::string udpip);

	/**
	 * Validates the format of the receiver UDP MAC address and sets it \sa sharedSlsDetector
	 * @param udpmac receiver UDP MAC address
	 * @returns the receiver UDP MAC address
	 */
	std::string setReceiverUDPMAC(std::string udpmac);

	/**
	 * Sets the receiver UDP port\sa sharedSlsDetector
	 * @param udpport receiver UDP port
	 * @returns the receiver UDP port
	 */
	int setReceiverUDPPort(int udpport);

	/**
	 * Sets the receiver UDP port 2\sa sharedSlsDetector
	 * @param udpport receiver UDP port 2
	 * @returns the receiver UDP port 2
	 */
	int setReceiverUDPPort2(int udpport);

	/**
	 * Sets the client zmq port\sa sharedSlsDetector
	 * @param port client zmq port (includes "multi" at the end if it should
	 * calculate individual ports)
	 * @returns the client zmq port
	 */
	std::string setClientStreamingPort(std::string port);

	/**
	 * Sets the receiver zmq port\sa sharedSlsDetector
	 * @param port receiver zmq port (includes "multi" at the end if it should
	 * calculate individual ports)
	 * @returns the receiver zmq port
	 */
	std::string setReceiverStreamingPort(std::string port);

	/**
	 * Sets the client zmq ip\sa sharedSlsDetector
	 * @param sourceIP client zmq ip
	 * @returns the client zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string setClientStreamingIP(std::string sourceIP);

	/**
	 * Sets the receiver zmq ip\sa sharedSlsDetector
	 * @param sourceIP receiver zmq ip. If empty, uses rx_hostname
	 * @returns the receiver zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string setReceiverStreamingIP(std::string sourceIP);

	/**
	 * Execute a digital test (Gotthard, Mythen)
	 * @param mode testmode type
	 * @param imod module index (-1 for all)
	 * @returns result of test
	 */
	int digitalTest(digitalTestMode mode, int imod=0);

	/**
	 * Execute trimming (Mythen)
	 * @param mode trimming mode type
	 * @param par1 parameter 1
	 * @param par2 parameter 2
	 * @param imod module index (-1 for all)
	 * @returns result of trimming
	 */
	int executeTrimming(trimMode mode, int par1, int par2, int imod=-1);

	/**
	 * Load dark or gain image to detector (Gotthard)
	 * @param index image type, 0 for dark image and 1 for gain image
	 * @param fname file name from which to load image
	 * @returns OK or FAIL
	 */
	int loadImageToDetector(imageType index,std::string const fname);

	/**
	 * Called from loadImageToDetector to send the image to detector
	 * @param index image type, 0 for dark image and 1 for gain image
	 * @param imageVals image
	 * @returns OK or FAIL
	 */
	int sendImageToDetector(imageType index,short int imageVals[]);

	/**
	 * Writes the counter memory block from the detector (Gotthard)
	 * @param fname file name to load data from
	 * @param startACQ is 1 to start acquisition after reading counter
	 * @returns OK or FAIL
	 */
	int writeCounterBlockFile(std::string const fname,int startACQ=0);

	/**
	 * Gets counter memory block in detector (Gotthard)
	 * @param arg counter memory block from detector
	 * @param startACQ 1 to start acquisition afterwards, else 0
	 * @returns OK or FAIL
	 */
	int getCounterBlock(short int arg[],int startACQ=0);

	/**
	 * Resets counter in detector
	 * @param startACQ is 1 to start acquisition after resetting counter
	 * @returns OK or FAIL
	 */
	int resetCounterBlock(int startACQ=0);

	/**
	 * Set/get counter bit in detector (Gotthard)
	 * @param i is -1 to get, 0 to reset and any other value to set the counter bit
	 * @returns the counter bit in detector
	 */
	int setCounterBit(int i = -1);

	/**
	 * Set ROI (Gotthard)
	 * At the moment only one set allowed
	 * @param n number of rois
	 * @param roiLimits array of roi
	 * @param imod module number (ignored)
	 * @returns OK or FAIL
	 */
	int setROI(int n=-1,ROI roiLimits[]=NULL, int imod = -1);

    /**
     * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
     * @param n number of rois
     * @param imod module number (ignored)
     * @returns pointer to array of ROI structure
     */
    ROI* getROI(int &n, int imod = -1);

	/**
	 * Returns number of rois
	 * @returns number of ROIs
	 */
	int getNRoi();

	/**
	 * Send ROI to the detector after calculating
	 * from setROI
	 * @param n number of ROIs (-1 to get)
	 * @param roiLimits ROI
	 * @returns OK or FAIL
	 */
	int sendROI(int n=-1,ROI roiLimits[]=NULL);

	/**
	 * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert users
	 * @param addr address of adc register
	 * @param val value
	 * @returns return value  (mostly -1 as it can't read adc register)
	 */
	int writeAdcRegister(int addr, int val);

	/**
	 * Activates/Deactivates the detector (Eiger only)
	 * @param enable active (1) or inactive (0), -1 gets
	 * @returns 0 (inactive) or 1 (active)for activate mode
	 */
	int activate(int const enable=-1);

	/**
	 * Set deactivated Receiver padding mode (Eiger only)
	 * @param padding padding option for deactivated receiver. Can be 1 (padding), 0 (no padding), -1 (gets)
	 * @returns 1 (padding), 0 (no padding), -1 (inconsistent values) for padding option
	 */
	int setDeactivatedRxrPaddingMode(int padding=-1);

	/**
	 * Returns the enable if data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @returns 1 for flipped, else 0
	 */
	int getFlippedData(dimension d=X);

	/**
	 * Sets the enable which determines if
	 * data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @param value 0 or 1 to reset/set or -1 to get value
	 * @returns enable flipped data across x or y axis
	 */
	int setFlippedData(dimension d=X, int value=-1);

	/**
	 * Sets all the trimbits to a particular value (Eiger)
	 * @param val trimbit value
	 * @param imod module number, -1 means all modules
	 * @returns OK or FAIL
	 */
	int setAllTrimbits(int val, int imod=-1);

	/**
	 * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
	 * 4 bit mode gap pixels only in gui call back
	 * @param val 1 sets, 0 unsets, -1 gets
	 * @returns gap pixel enable or -1 for error
	 */
	int enableGapPixels(int val=-1);

	/**
	 * Sets the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param nen number of energies
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	int setTrimEn(int nen, int *en=NULL);

	/**
	 * Returns the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	int getTrimEn(int *en=NULL);

	/**
	 * Pulse Pixel (Eiger)
	 * @param n is number of times to pulse
	 * @param x is x coordinate
	 * @param y is y coordinate
	 * @returns OK or FAIL
	 */
	int pulsePixel(int n=0,int x=0,int y=0);

	/**
	 * Pulse Pixel and move by a relative value (Eiger)
	 * @param n is number of times to pulse
	 * @param x is relative x value
	 * @param y is relative y value
	 * @returns OK or FAIL
	 */
	int pulsePixelNMove(int n=0,int x=0,int y=0);

	/**
	 * Pulse Chip (Eiger)
	 * @param n is number of times to pulse
	 * @returns OK or FAIL
	 */
	int pulseChip(int n=0);

	/**
	 * Set/gets threshold temperature (Jungfrau)
	 * @param val value in millidegrees, -1 gets
	 * @param imod module number, -1 is all
	 * @returns threshold temperature in millidegrees
	 */
	int setThresholdTemperature(int val=-1, int imod=-1);

	/**
	 * Enables/disables temperature control (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns temperature control enable
	 */
	int setTemperatureControl(int val=-1, int imod=-1);

	/**
	 * Resets/ gets over-temperature event (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns over-temperature event
	 */
	int setTemperatureEvent(int val=-1, int imod=-1);

	/**
	 * Set storage cell that stores first acquisition of the series (Jungfrau)
	 * @param value storage cell index. Value can be 0 to 15. (-1 gets)
	 * @returns the storage cell that stores the first acquisition of the series
	 */
	int setStoragecellStart(int pos=-1);

	/**
	 * Programs FPGA with pof file (Jungfrau)
	 * @param fname file name
	 * @returns OK or FAIL
	 */
	int programFPGA(std::string fname);

	/**
	 * Resets FPGA (Jungfrau)
	 * @returns OK or FAIL
	 */
	int resetFPGA();

	/**
	 * Power on/off Chip (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	int powerChip(int ival= -1);

	/**
	 * Automatic comparator disable (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	int setAutoComparatorDisableMode(int ival= -1);


	/**
	 * Returns the trimbits from the detector's shared memmory (Mythen, Eiger)
	 * @param retval is the array with the trimbits
	 * @param fromDetector is true if the trimbits shared memory have to be
	 * uploaded from detector
	 * @returns total number of channels for the detector
	 */
	int getChanRegs(double* retval,bool fromDetector);

	/**
	 * Configure module (who calls this?)
	 * @param imod module number (-1 all)
	 * @returns current register value
	 * \sa ::sls_detector_module
	 */
	int setModule(int reg, int imod=-1);

	/**
	 * Configure Module (Mythen, Eiger)
	 * Called for loading trimbits and settings settings to the detector
	 * @param module module to be set - must contain correct module number and
	 * also channel and chip registers
	 * @param iodelay iodelay (detector specific)
	 * @param tau tau (detector specific)
	 * @param e_eV threashold in eV (detector specific)
	 * @param gainval pointer to extra gain values
	 * @param offsetval pointer to extra offset values
	 * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
	 * @returns current register value
	 * \sa ::sls_detector_module
	 */
	int setModule(sls_detector_module module, int iodelay, int tau, int e_eV,
			int* gainval=0, int* offsetval=0, int tb=1);


	/**
	 * Get module structure from detector (all detectors)
	 * @param imod module number
	 * @returns pointer to module structure (which has been created and must then be deleted)
	 */
	sls_detector_module *getModule(int imod);

	/**
	 * Configure channel (Mythen)
	 * @param reg channel register
	 * @param ichan channel number (-1 all)
	 * @param ichip chip number (-1 all)
	 * @param imod module number (-1 all)
	 * @returns current register value
	 * \sa ::sls_detector_channel
	 */
	int setChannel(int64_t reg, int ichan=-1, int ichip=-1, int imod=-1);

	/**
	 * Configure channel (Mythen)
	 * @param chan channel to be set -
	 * must contain correct channel, module and chip number
	 * @returns current register value
	 */
	int setChannel(sls_detector_channel chan);

	/**
	 * Get channel (Mythen)
	 * @param ichan channel number
	 * @param ichip chip number
	 * @param imod module number
	 * @returns current channel structure for channel
	 */
	sls_detector_channel getChannel(int ichan, int ichip, int imod);

	/**
	 * Configure chip (Mythen)
	 * @param reg chip register
	 * @param ichip chip number (-1 all)
	 * @param imod module number (-1 all)
	 * @returns current register value
	 * \sa ::sls_detector_chip
	 */
	int setChip(int reg, int ichip=-1, int imod=-1);

	/**
	 * Configure chip (Mythen)
	 * @param chip chip to be set
	 * must contain correct module and chip number and also channel registers
	 * @returns current register value
	 * \sa ::sls_detector_chip
	 */
	int setChip(sls_detector_chip chip);

	/**
	 * Get chip (Mythen)
	 * @param ichip chip number
	 * @param imod module number
	 * @returns current chip structure for channel
	 * \bug probably does not return corretly!
	 */
	sls_detector_chip getChip(int ichip, int imod);

	/**
	 * Get Move Flag (Mythen)
	 * @param imod module number (-1 all)
	 * @param istep step index
	 * @returns move flag
	 */
	int getMoveFlag(int imod);

	/**
	 * Fill Module mask for flat field corrections (Mythen)
	 * @param mM array
	 * @returns number of modules
	 */
	int fillModuleMask(int *mM);

	/**
	 * Calibrate Pedestal (ChipTestBoard)
	 * Starts acquisition, calibrates pedestal and writes to fpga
	 * @param frames number of frames
	 * @returns number of frames
	 */
	int calibratePedestal(int frames = 0);

	/**
	 * Set Rate correction (Mythen, Eiger)
	 * @param t dead time in ns - if 0 disable correction,
	 * if >0 set dead time to t, if < 0 set deadtime to default dead time
	 * for current settings
	 * @returns 0 if rate correction disabled, >0 otherwise
	 */
	int setRateCorrection(double t=0);

	/**
	 * Get rate correction (Mythen, Eiger)
	 * @param t reference for dead time
	 * @returns 0 if rate correction disabled, > 0 otherwise
	 */
	int getRateCorrection(double &t);

	/**
	 * Get rate correction tau (Mythen, Eiger)
	 * @returns 0 if rate correction disabled, otherwise the tau used for the correction
	 */
	double getRateCorrectionTau();

	/**
	 * Get rate correction (Mythen, Eiger)
	 * @returns 0 if rate correction disabled,  > 0 otherwise
	 */
	int getRateCorrection();

	/**
	 * Rate correct data (Mythen)
	 * @param datain data array
	 * @param errin error array on data (if NULL will default to sqrt(datain)
	 * @param dataout array of corrected data
	 * @param errout error on corrected data (if not NULL)
	 * @returns 0
	 */
	int rateCorrect(double* datain, double *errin, double* dataout, double *errout);

	/**
	 * Set flat field corrections (Mythen)
	 * @param fname name of the flat field file (or "" if disable)
	 * @returns 0 if disable (or file could not be read), >0 otherwise
	 */
	int setFlatFieldCorrection(std::string fname="");

	/**
	 * Set flat field corrections (Mythen)
	 * @param corr if !=NULL the flat field corrections will be filled with
	 * corr (NULL usets ff corrections)
	 * @param  ecorr if !=NULL the flat field correction errors will be filled
	 * with ecorr (1 otherwise)
	 * @returns 0 if ff correction disabled, >0 otherwise
	 */
	int setFlatFieldCorrection(double *corr, double *ecorr=NULL);

	/**
	 * Get flat field corrections (Mythen)
	 * @param corr if !=NULL will be filled with the correction coefficients
	 * @param ecorr if !=NULL will be filled with the correction coefficients errors
	 * @returns 0 if ff correction disabled, >0 otherwise
	 */
	int getFlatFieldCorrection(double *corr=NULL, double *ecorr=NULL);

	/**
	 * Flat field correct data (Mythen)
	 * @param datain data array
	 * @param errin error array on data (if NULL will default to sqrt(datain)
	 * @param dataout array of corrected data
	 * @param errout error on corrected data (if not NULL)
	 * @returns 0
	 */
	int flatFieldCorrect(double* datain, double *errin, double* dataout, double *errout);

	/**
	 * Set bad channels correction (Mythen)
	 * @param fname file with bad channel list ("" disable)
	 * @returns 0 if bad channel disabled, >0 otherwise
	 */
	int setBadChannelCorrection(std::string fname="");

	/**
	 * Set bad channels correction (Mythen)
	 * @param nch number of bad channels
	 * @param chs array of channels
	 * @param ff 0 if normal bad channels, 1 if ff bad channels
	 * @returns 0 if bad channel disabled, >0 otherwise
	 */
	int setBadChannelCorrection(int nch, int *chs, int ff=0);

	/**
	 * Get bad channels correction (Mythen)
	 * @param bad pointer to array that if bad!=NULL will be filled with the
	 * bad channel list
	 * @returns 0 if bad channel disabled or no bad channels, >0 otherwise
	 */
	int getBadChannelCorrection(int *bad=NULL);

	/**
	 * Reads an angular conversion file (Mythen, Gotthard)
	 * \sa angleConversionConstant mythenDetector::readAngularConversion
	 * @param fname file to be read
	 * @returns 0 if angular conversion disabled, >0 otherwise
	 */
	int readAngularConversionFile(std::string fname="");

	/**
	 * Reads an angular conversion file (Mythen, Gotthard)
	 * \sa angleConversionConstant mythenDetector::readAngularConversion
	 * @param ifs input stream
	 * @returns 0 if angular conversion disabled, >0 otherwise
	 */
	int readAngularConversion(std::ifstream& ifs);

	/**
	 * Writes an angular conversion file (Mythen, Gotthard)
	 * \sa angleConversionConstant mythenDetector::writeAngularConversion
	 * @param fname file to be written
	 * @returns 0 if angular conversion disabled, >0 otherwise
	 */
	int writeAngularConversion(std::string fname="");

	/**
	 * Writes an angular conversion file (Mythen, Gotthard)
	 * \sa angleConversionConstant mythenDetector::writeAngularConversion
	 * @param ofs output stream
	 * @returns 0 if angular conversion disabled, >0 otherwise
	 */
	int writeAngularConversion(std::ofstream &ofs);

	/**
	 * Get angular conversion (Mythen, Gotthard)
	 * \sa angleConversionConstant mythenDetector::getAngularConversion
	 * @param direction reference to diffractometer
	 * @param angconv array that will be filled with the angular conversion constants
	 * @returns 0 if angular conversion disabled, >0 otherwise
	 */
	int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL) ;

	/**
	 * Return angular conversion pointer (Mythen, Gotthard)
	 * @param imod module number
	 * @returns angular conversion pointer
	 */
	angleConversionConstant *getAngularConversionPointer(int imod=0);

	/**
	 * Prints receiver configuration
	 * @returns OK or FAIL
	 */
	int printReceiverConfiguration();

	/**
	 * Checks if receiver is online and set flag
	 * Also initializes the data socekt
	 * @param online 1 to set online, 0 to set offline, -1 gets
	 * @returns online, offline (from shared memory)
	 */
	int setReceiverOnline(int const online=GET_ONLINE_FLAG);

	/**
	 * Checks if the receiver is really online
	 * @returns empty string if online, else returns receiver hostname
	 */
	std::string checkReceiverOnline();

	/**
	 * Configure the socket communication and initializes the socket instances
	 * @param name receiver ip - if "" the current receiver hostname is used
	 * @param receiver_port port for receiving data - if -1 the current is used
	 * @returns OK is connection succeded, FAIL otherwise
	 * \sa sharedSlsDetector
	 */
	int setReceiverTCPSocket(std::string const name="", int const receiver_port=-1);

	/**
	 * Locks/Unlocks the connection to the receiver
	 * @param lock sets (1), usets (0), gets (-1) the lock
	 * @returns lock status of the receiver
	 */
	int lockReceiver(int lock=-1);

	/**
	 * Returns the IP of the last client connecting to the receiver
	 * @returns the IP of the last client connecting to the receiver
	 */
	std::string getReceiverLastClientIP();

	/**
	 * Exits the receiver TCP server
	 * @retutns OK or FAIL
	 */
	int exitReceiver();

	/**
     updates the shared memory receiving the data from the detector (without asking and closing the connection
     /returns OK
	 */
	int updateReceiverNoWait();

	/**
	 * Updates the shared memory receiving the data from the detector
	 * @returns OK or FAIL
	 */
	int updateReceiver();

	/**
	 * Send the multi detector size to the detector
	 */
	void sendMultiDetectorSize();

	/**
	 * Send the detector pos id to the receiver
	 * for various file naming conventions for multi detectors in receiver
	 */
	void setDetectorId();

	/**
	 * Send the detector host name to the  receiver
	 * for various handshaking required with the detector
	 */
	void setDetectorHostname();

	/**
	 * Returns output file directory
	 * @returns output file directory
	 */
	std::string getFilePath();

	/**
	 * Sets up the file directory
	 * @param s file directory
	 * @returns file dir
	 */
	std::string setFilePath(std::string s="");

	/**
	 * Returns file name prefix
	 * @returns file name prefix
	 */
	std::string getFileName();

	/**
	 * Sets up the file name prefix
	 * @param s file name prefix
	 * @returns file name prefix
	 */
	std::string setFileName(std::string s="");

	/**
	 * Sets the max frames per file in receiver
	 * @param f max frames per file
	 * @returns max frames per file in receiver
	 */
	int setReceiverFramesPerFile(int f = -1);

	/**
	 * Sets the frames discard policy in receiver
	 * @param f frames discard policy
	 * @returns frames discard policy set in receiver
	 */
	frameDiscardPolicy setReceiverFramesDiscardPolicy(frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY);

	/**
	 * Sets the partial frames padding enable in receiver
	 * @param f partial frames padding enable
	 * @returns partial frames padding enable in receiver
	 */
	int setReceiverPartialFramesPadding(int f = -1);

	/**
	 * Returns file format
	 * @returns file name
	 */
	fileFormat getFileFormat();

	/**
	 * Sets up the file format
	 * @param f file format
	 * @returns file format
	 */
	fileFormat setFileFormat(fileFormat f=GET_FILE_FORMAT);

	/**
	 * Returns file index
	 * @returns file index
	 */
	int getFileIndex();

	/**
	 * Sets up the file index
	 * @param i file index
	 * @returns file index
	 */
	int setFileIndex(int i=-1);

	/**
	 * Receiver starts listening to packets
	 * @returns OK or FAIL
	 */
	int startReceiver();

	/**
	 * Stops the listening mode of receiver
	 * @returns OK or FAIL
	 */
	int stopReceiver();

	/**
	 * Sets the receiver to start any readout remaining in the fifo and
	 * change status to transmitting (Mythen)
	 * The status changes to run_finished when fifo is empty
	 */
	runStatus startReceiverReadout();

	/**
	 * Gets the status of the listening mode of receiver
	 * @returns status
	 */
	runStatus getReceiverStatus();

	/**
	 * Gets the number of frames caught by receiver
	 * @returns number of frames caught by receiver
	 */
	int getFramesCaughtByReceiver();

	/**
	 * Gets the number of frames caught by any one receiver (to avoid using threadpool)
	 * @returns number of frames caught by any one receiver (master receiver if exists)
	 */
	int getFramesCaughtByAnyReceiver();

	/**
	 * Gets the current frame index of receiver
	 * @returns current frame index of receiver
	 */
	int getReceiverCurrentFrameIndex();

	/**
	 * Resets framescaught in receiver
	 * Use this when using startAcquisition instead of acquire
	 * @returns OK or FAIL
	 */
	int resetFramesCaught();

	/**
	 * Sets/Gets receiver file write enable
	 * @param enable 1 or 0 to set/reset file write enable
	 * @returns file write enable
	 */
	int enableWriteToFile(int enable=-1);

	/**
	 * Sets/Gets file overwrite enable
	 * @param enable 1 or 0 to set/reset file overwrite enable
	 * @returns file overwrite enable
	 */
	int overwriteFile(int enable=-1);

	/**
	 * Sets the read receiver frequency
	 * if data required from receiver randomly readRxrFrequency=0,
	 * else every nth frame to be sent to gui/callback
	 * @param freq is the receiver read frequency. Value 0 is 200 ms timer (other
	 * frames not sent), 1 is every frame, 2 is every second frame etc.
	 * @returns read receiver frequency
	 */
	int setReadReceiverFrequency(int freq=-1);

	/**
	 * Sets the read receiver timer
	 * if data required from receiver randomly readRxrFrequency=0,
	 * then the timer between each data stream is set with time_in_ms
	 * @param time_in_ms timer between frames
	 * @returns read receiver timer
	 */
	int setReceiverReadTimer(int time_in_ms=500);

	/**
	 * Enable data streaming to client
	 * @param enable 0 to disable, 1 to enable, -1 to get the value
	 * @returns data streaming to client enable
	 */
	int enableDataStreamingToClient(int enable=-1);

	/**
	 * Enable or disable streaming data from receiver to client
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @returns data streaming from receiver enable
	 */
	int enableDataStreamingFromReceiver(int enable=-1);

	/**
	 * Enable/disable or get data compression in receiver
	 * @param i is -1 to get, 0 to disable and 1 to enable
	 * @returns data compression in receiver
	 */
	int enableReceiverCompression(int i = -1);

	/**
	 * Enable/disable or 10Gbe
	 * @param i is -1 to get, 0 to disable and 1 to enable
	 * @returns if 10Gbe is enabled
	 */
	int enableTenGigabitEthernet(int i = -1);

	/**
	 * Set/get receiver fifo depth
	 * @param i is -1 to get, any other value to set the fifo deph
	 * @returns the receiver fifo depth
	 */
	int setReceiverFifoDepth(int i = -1);

	/**
	 * Set/get receiver silent mode
	 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
	 * @returns the receiver silent mode enable
	 */
	int setReceiverSilentMode(int i = -1);

	/**
	 * If data streaming in receiver is enabled,
	 * restream the stop dummy packet from receiver
	 * Used usually for Moench,
	 * in case it is lost in network due to high data rate
	 * @returns OK if success else FAIL
	 */
	int restreamStopFromReceiver();

	/**
	 * Opens pattern file and sends pattern to CTB
	 * @param fname pattern file to open
	 * @returns OK/FAIL
	 */
	int setCTBPattern(std::string fname);

	/**
	 * Writes a pattern word to the CTB
	 * @param addr address of the word, -1 is I/O control register,  -2 is clk control register
	 * @param word 64bit word to be written, -1 gets
	 * @returns actual value
	 */
	uint64_t setCTBWord(int addr,uint64_t word=-1);

	/**
	 * Sets the pattern or loop limits in the CTB
	 * @param level -1 complete pattern, 0,1,2, loop level
	 * @param start start address if >=0
	 * @param stop stop address if >=0
	 * @param n number of loops (if level >=0)
	 * @returns OK/FAIL
	 */
	int setCTBPatLoops(int level,int &start, int &stop, int &n);

	/**
	 * Sets the wait address in the CTB
	 * @param level  0,1,2, wait level
	 * @param addr wait address, -1 gets
	 * @returns actual value
	 */
	int setCTBPatWaitAddr(int level, int addr=-1);

	/**
	 * Sets the wait time in the CTB
	 * @param level  0,1,2, wait level
	 * @param t wait time, -1 gets
	 * @returns actual value
	 */
	int setCTBPatWaitTime(int level, uint64_t t=-1);

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
	 * Sets detector parameters depending detector type
	 * @param type detector type
	 * @param list structure of parameters to initialize depending on detector type
	 */
	void setDetectorSpecificParameters(detectorType type, detParameterList& list);

	/**
	 * Calculate shared memory size based on detector type
	 * @param type type of detector
	 * @returns size of shared memory of sharedSlsDetector structure
	 */
	int calculateSharedMemorySize(detectorType type);

	/**
	 * Initialize detector structure to defaults
	 * Called when new shared memory is created
	 * @param type type of detector
	 */
	void initializeDetectorStructure(detectorType type);

	/**
	 * Initialize class members (and from parent classes)
	 * Also connect member pointers to detector structure pointers
	 * Called when shared memory created/existed
	 */
	void initializeMembers();

	/**
	 * Initialize detector structure
	 * Called when new shared memory created
	 * Initializes the member pointers to defaults as well
	 */
	void initializeDetectorStructurePointers();

	/**
	 * Allocates the memory for a sls_detector_module structure and initializes it
	 * Uses current detector type
	 * @returns myMod the pointer to the allocate dmemory location
	 */
	sls_detector_module*  createModule();

	/**
	 * Allocates the memory for a sls_detector_module structure and initializes it
	 * Has detector type
	 * @param type detector type
	 * @returns myMod the pointer to the allocate dmemory location
	 */
	sls_detector_module*  createModule(detectorType type);

	/**
	 * Frees the memory for a sls_detector_module structure
	 * @param myMod the pointer to the memory to be freed
	 */
	void deleteModule(sls_detector_module *myMod);

	/**
	 * Send a sls_detector_channel structure over socket
	 * @param myChan channel structure to send
	 * @returns number of bytes sent to the detector
	 */
	int sendChannel(sls_detector_channel* myChan);

	/**
	 * Send a sls_detector_chip structure over socket
	 * @param myChip chip structure to send
	 * @returns number of bytes sent to the detector
	 */
	int sendChip(sls_detector_chip* myChip);

	/**
	 * Send a sls_detector_module structure over socket
	 * @param myMod module structure to send
	 * @returns number of bytes sent to the detector
	 */
	int sendModule(sls_detector_module* myMod);

	/**
	 * Receive a sls_detector_channel structure over socket
	 * @param myChan channel structure to receive
	 * @returns number of bytes received from the detector
	 */
	int receiveChannel(sls_detector_channel* myChan);

	/**
	 * Receive a sls_detector_chip structure over socket
	 * @param myChip chip structure to receive
	 * @returns number of bytes received from the detector
	 */
	int receiveChip(sls_detector_chip* myChip);

	/**
	 * Receive a sls_detector_module structure over socket
	 * @param myMod module structure to receive
	 * @returns number of bytes received from the detector
	 */
	int receiveModule(sls_detector_module* myMod);

	/**
	 * Returns the additional json header \sa sharedSlsDetector
	 * @returns the additional json header, returns "none" if default setting and no custom ip set
	 */
	std::string getAdditionalJsonHeader();

	/**
	 * Returns the receiver UDP socket buffer size\sa sharedSlsDetector
	 * @returns the receiver UDP socket buffer size
	 */
	std::string getReceiverUDPSocketBufferSize() ;

	/**
	 * Returns the receiver real UDP socket buffer size\sa sharedSlsDetector
	 * @returns the receiver real UDP socket buffer size
	 */
	std::string getReceiverRealUDPSocketBufferSize();

	/**
	 * Sets the additional json header\sa sharedSlsDetector
	 * @param jsonheader additional json header
	 * @returns additional json header, returns "none" if default setting and no custom ip set
	 */
	std::string setAdditionalJsonHeader(std::string jsonheader);

	/**
	 * Sets the receiver UDP socket buffer size
	 * @param udpsockbufsize additional json header
	 * @returns receiver udp socket buffer size
	 */
	std::string setReceiverUDPSocketBufferSize(int udpsockbufsize=-1);

	/**
	 * Sets the transmission delay for left, right or entire frame
	 * (Eiger, Jungfrau(only entire frame))
	 * @param index type of delay
	 * @param delay delay
	 * @returns transmission delay
	 */
	std::string setDetectorNetworkParameter(networkParameter index, int delay);


	/**
	 * Get MAC from the receiver using udpip and
	 * set up UDP connection in detector
	 * @returns Ok or FAIL
	 */
	int setUDPConnection();

	/** slsDetector Id or position in the detectors list */
	int detId;

	/** Shared Memory object */
	SharedMemory* sharedMemory;

	/** Shared memory structure */
	sharedSlsDetector *thisDetector;

	/** multiSlsDetector referece */
	multiSlsDetector *multiDet;

	receiverInterface *thisReceiver;

	/** socket for control commands	 */
	MySocketTCP *controlSocket;

	/** socket for emergency stop	 */
	MySocketTCP *stopSocket;

	/** socket for data acquisition	 */
	MySocketTCP *dataSocket;

	/** pointer to flat field coefficients in shared memory  */
	double *ffcoefficients;

	/** pointer to flat field coefficient errors in shared memory  */
	double *fferrors;

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
};

#endif

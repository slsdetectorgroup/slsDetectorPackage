#ifndef SLS_DETECTOR_H
#define SLS_DETECTOR_H

/**
 *
 * @short complete detector functionalities for a single module detector.
 * The slsDetector class takes care of the communication with the
 * detector and all kind actions related with a single detector controller
 * @author Anna Bergamaschi
 */

#include "sls_detector_defs.h"
#include "error_defs.h"
#include "logger.h"
#include "ClientSocket.h"

class ClientInterface;

#include <cmath>

class multiSlsDetector;
class SharedMemory;
class ServerInterface;
class MySocketTCP;

#define SLS_SHMVERSION	0x181005
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16
/**
 * parameter list that has to be initialized depending on the detector type
 */
typedef  struct detParameterList {
	int nChanX;
	int nChanY;
	int nChipX;
	int nChipY;
	int nDacs;
	int dynamicRange;
	int nGappixelsX;
	int nGappixelsY;
} detParameterList;


class slsDetector : public virtual slsDetectorDefs, public virtual errorDefs {

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

		/** detector type  \ see :: detectorType*/
		detectorType myDetectorType;

		/** END OF FIXED PATTERN -----------------------------------------------*/




		/** Detector offset in the X & Y direction in the multi detector structure */
		int offset[2];

		/** Number of detectors in multi list in x dir and y dir */
		int multiSize[2];

		/** is the port used for control functions */
		int controlPort;

		/** is the port used to stop the acquisition */
		int stopPort;

		/** path of the trimbits/settings files */
		char settingsDir[MAX_STR_LENGTH];

		/** number of energies at which the detector has been trimmed */
		int nTrimEn;

		/** list of the energies at which the detector has been trimmed  */
		int trimEnergies[MAX_TRIMEN];

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

		/** dynamic range of the detector data */
		int dynamicRange;

		/**  size of the data that are transfered from the detector */
		int dataBytes;

		/** threaded processing flag
		 * (i.e. if data are processed in a separate thread)  */
		// int threadedProcessing;

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

		/** memory offsets for the module structures  */
		int modoff;

		/** memory offsets for the dac arrays */
		int dacoff;

		/** memory offsets for the channel register arrays  -trimbits*/
		int chanoff;

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

		/** path of the output files */
		char receiver_filePath[MAX_STR_LENGTH];

		/** file name prefix */
		char receiver_fileName[MAX_STR_LENGTH];

		/** file index */
		int receiver_fileIndex;

		/** file format */
		fileFormat receiver_fileFormatType;

		/** frames per file */
		int receiver_framesPerFile;

		/** filewriteenable */
		bool receiver_fileWriteEnable;

		/** overwriteenable */
		bool receiver_overWriteEnable;

	} sharedSlsDetector;





public:

	/**
	 * Constructor called when creating new shared memory
	 * @param type detector type
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 */
	explicit slsDetector(detectorType type, 
						 int multiId = 0, 
						 int id = 0, 
						 bool verify = true);

	/**
	 * Constructor called when opening existing shared memory
	 * @param multiId multi detector shared memory id
	 * @param id sls detector id (position in detectors list)
	 * @param verify true to verify if shared memory version matches existing one
	 */
	explicit slsDetector(int multiId = 0, 
						 int id = 0, 
						 bool verify = true);

	/**
	 * Destructor
	 */
	virtual ~slsDetector();

	/**
	 * Check version compatibility with receiver software
	 * (if hostname/rx_hostname has been set/ sockets created)
	 * @param p port type control port or receiver port
	 * @returns FAIL for incompatibility, OK for compatibility
	 */
	int checkReceiverVersionCompatibility();

	/**
	 * Check version compatibility with detector software
	 * @returns FAIL for incompatibility, OK for compatibility
	 */
	int checkDetectorVersionCompatibility();

	/**
	 * Get ID or version numbers
	 * @param mode version type
	 * @returns Id or version number of that type
	 */
	int64_t getId(idMode mode);

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
	 * Sets the hostname, if online flag is set connects to update the detector
	 * @param name hostname
	 */
	void setHostname(const std::string& hostname);

	/**
	 * Gets the hostname of detector
	 * @returns hostname
	 */
	std::string getHostname();

	/**
	 * Could not connect to receiver, log error
	 */
    void connectDataError();

	/**
	 * Get detector type by connecting to the detector 
	 * @returns detector tpe or GENERIC if failed
	 */
	static detectorType getTypeFromDetector(const std::string& hostname, int cport=DEFAULT_PORTNO);

	/**
	 * Get Detector type from shared memory variable
	 * @returns detector type from shared memory variable
	 */
	detectorType getDetectorTypeAsEnum();

	/**
	 * Gets string version of detector type from shared memory variable
	 * @returns string version of detector type from shared memory variable
	 */
	std::string getDetectorTypeAsString();

	/**
	 * Gets detector type from detector and set it in receiver
	 * @param type the detector type
	 * @returns detector type in receiver
	 */
	int setDetectorType(detectorType type=GET_DETECTOR_TYPE);


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
	 * Set Detector offset in shared memory in dimension d
	 * @param detx number of detectors in X dir in multi list
	 * @param dety number of detectors in Y dir in multi list
	 */
	void updateMultiSize(int detx, int dety);

	/**
	 * Checks if the detector is online and sets the online flag
	 * @param online if GET_ONLINE_FLAG, only returns shared memory online flag,
	 * else sets the detector in online/offline state
	 * if OFFLINE_FLAG, (i.e. no communication to the detector - using only local structure - no data acquisition possible!);
	 * if ONLINE_FLAG, detector in online state (i.e. communication to the detector updating the local structure)
	 * @returns online/offline status
	 */
	int setOnline(int value=GET_ONLINE_FLAG);


	/**
	 * Returns the online flag
	 */
	int getOnlineFlag() const;

	/**
	 * Checks if each of the detector is online/offline
	 * @returns empty string if it is online
	 * else returns hostname if it is offline
	 */
	std::string checkOnline();

	int setControlPort(int port_number);

	/**
	 * Returns the detector TCP control port  \sa sharedSlsDetector
	 * @returns the detector TCP control port
	 */
	int getControlPort() const;

	int setStopPort(int port_number);

	/**
	 * Returns the detector TCP stop port  \sa sharedSlsDetector
	 * @returns the detector TCP stop port
	 */
	int getStopPort() const;

	int setReceiverPort(int port_number);

	/**
	 * Returns the receiver TCP 	port  \sa sharedSlsDetector
	 * @returns the receiver TCP port
	 */
	int getReceiverPort() const ;

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
	 * @returns OK or FAIL
	 */
	int execCommand(const std::string& cmd);

	/**
	 * Updates some of the shared memory receiving the data from the detector
	 * @returns OK
	 */
	int updateDetectorNoWait( sls::ClientSocket &client);

	/**
	 * Updates some of the shared memory receiving the data from the detector
	 * calls updateDetectorNoWait
	 * @returns OK or FAIL or FORCE_RET
	 */
	int updateDetector();

	/**
	 * Write current configuration to a file
	 * calls writeConfigurationFile giving it a stream to write to
	 * @param fname configuration file name
	 * @param m multiSlsDetector reference to parse commands
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(const std::string& fname, multiSlsDetector* m);

	/**
	 * Write current configuration to a stream
	 * @param outfile outstream
	 * @param m multiSlsDetector reference to parse commands
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(std::ofstream &outfile, multiSlsDetector* m);

	/**
	 * Returns the trimfile or settings file name (Useless??)
	 * @returns the trimfile or settings file name
	 */
	std::string getSettingsFile();

	/**
	 * Writes a trim/settings file for module number
	 * the values will be read from the current detector structure
	 * @param fname name of the file to be written
	 * @returns OK or FAIL if the file could not be written
	 * \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeSettingsFile(string, int)
	 */
	int writeSettingsFile(const std::string& fname);

	/**
	 * Get detector settings
	 * @returns current settings
	 */
	detectorSettings getSettings();

	/**
	 * Load detector settings from the settings file picked from the trimdir/settingsdir
	 * Eiger only stores in shared memory ( a get will overwrite this)
	 * For Eiger, one must use threshold
	 * Gotthard, Propix, Jungfrau and Moench only sends the settings enum to the detector
	 * @param isettings settings
	 * @returns current settings
	 */
	detectorSettings setSettings(detectorSettings isettings);

	/**
	 * Send detector settings only (set only for Jungfrau, Gotthard, Moench, get for all)
	 * Only the settings enum is sent to the detector, where it will
	 * initialize al the dacs already hard coded in the detector server
	 * @param isettings  settings
	 * @returns current settings
	 */
	detectorSettings sendSettingsOnly(detectorSettings isettings);

	/**
	 * Get threshold energy (Mythen and Eiger)
	 * @returns current threshold value in ev (-1 failed)
	 */
	int getThresholdEnergy();


	/**
	 * Set threshold energy (Mythen and Eiger)
	 * For Eiger, calls setThresholdEneryAndSettings
	 * @param e_eV threshold in eV
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns current threshold value in ev (-1 failed)
	 */
	int setThresholdEnergy(int e_eV, detectorSettings isettings=GET_SETTINGS, int tb=1);

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
	std::string setSettingsDir(const std::string& dir);

	/**
	 * Loads the modules settings/trimbits reading from a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * returns OK or FAIL
	 */
	int loadSettingsFile(const std::string& fname);

	/**
	 * Saves the modules settings/trimbits to a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * returns OK or FAIL
	 */
	int saveSettingsFile(const std::string& fname);

	/**
	 * Get run status of the detector
	 * @returns the status of the detector
	 */
	runStatus  getRunStatus();

	/**
	 * Prepares detector for acquisition (Eiger)
	 * @returns OK or FAIL
	 */
	int prepareAcquisition();

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
	 * Start detector acquisition and read all data (Blocking until end of acquisition)
	 * @returns OK or FAIL
	 */
	int startAndReadAll();

	/**
	 * Start readout (without exposure or interrupting exposure) (Eiger store in ram)
	 * @returns OK or FAIL
	 */
	int startReadOut();

	/**
	 * Requests and  receives all data from the detector (Eiger store in ram)
	 * @returns OK or FAIL
	 */
	int readAll();

	/**
	 * Configures in detector the destination for UDP packets
	 * @returns OK or FAIL
	 */
	int configureMAC();

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
	 * @returns current DAC value
	 */
	int setDAC(int val, dacIndex index, int mV);

	/**
	 * Get adc value
	 * @param index adc(DAC) index
	 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
	 */
	int getADC(dacIndex index);

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
	 * Validates the format of the detector MAC address and sets it \sa sharedSlsDetector
	 * @param detectorMAC detector MAC address
	 * @returns the detector MAC address
	 */
	std::string setDetectorMAC(const std::string& detectorMAC);

	/**
	 * Returns the detector MAC address\sa sharedSlsDetector
	 * @returns the detector MAC address
	 */
	std::string getDetectorMAC();

	/**
	 * Validates the format of the detector IP address and sets it \sa sharedSlsDetector
	 * @param detectorIP detector IP address
	 * @returns the detector IP address
	 */
	std::string setDetectorIP(const std::string& detectorIP);

	/**
	 * Returns the detector IP address\sa sharedSlsDetector
	 * @returns the detector IP address
	 */
	std::string getDetectorIP();

	/**
	 * Validates and sets the receiver.
	 * Also updates the receiver with all the shared memory parameters significant for the receiver
	 * Also configures the detector to the receiver as UDP destination
	 * @param receiver receiver hostname or IP address
	 * @returns the receiver IP address from shared memory
	 */
	std::string setReceiver(const std::string& receiver);

	/**
	 * Returns the receiver IP address\sa sharedSlsDetector
	 * @returns the receiver IP address
	 */
	std::string getReceiver();

	/**
	 * Validates the format of the receiver UDP IP address and sets it \sa sharedSlsDetector
	 * @param udpip receiver UDP IP address
	 * @returns the receiver UDP IP address
	 */
	std::string setReceiverUDPIP(const std::string& udpip);

	/**
	 * Returns the receiver UDP IP address\sa sharedSlsDetector
	 * @returns the receiver UDP IP address
	 */
	std::string getReceiverUDPIP();

	/**
	 * Validates the format of the receiver UDP MAC address and sets it \sa sharedSlsDetector
	 * @param udpmac receiver UDP MAC address
	 * @returns the receiver UDP MAC address
	 */
	std::string setReceiverUDPMAC(const std::string& udpmac);

	/**
	 * Returns the receiver UDP MAC address\sa sharedSlsDetector
	 * @returns the receiver UDP MAC address
	 */
	std::string getReceiverUDPMAC();

	/**
	 * Sets the receiver UDP port\sa sharedSlsDetector
	 * @param udpport receiver UDP port
	 * @returns the receiver UDP port
	 */
	int setReceiverUDPPort(int udpport);

	/**
	 * Returns the receiver UDP port\sa sharedSlsDetector
	 * @returns the receiver UDP port
	 */
	int getReceiverUDPPort();

	/**
	 * Sets the receiver UDP port 2\sa sharedSlsDetector
	 * @param udpport receiver UDP port 2
	 * @returns the receiver UDP port 2
	 */
	int setReceiverUDPPort2(int udpport);

	/**
	 * Returns the receiver UDP port 2 of same interface\sa sharedSlsDetector
	 * @returns the receiver UDP port 2 of same interface
	 */
	int getReceiverUDPPort2();

	/**
	 * Sets the client zmq port\sa sharedSlsDetector
	 * @param port client zmq port
	 */
	void setClientStreamingPort(int port);

	/**
	 * Returns the client zmq port \sa sharedSlsDetector
	 * @returns the client zmq port
	 */
	int getClientStreamingPort();

	/**
	 * Sets the receiver zmq port\sa sharedSlsDetector
	 * @param port receiver zmq port
	 */
	void setReceiverStreamingPort(int port);

	/**
	 * Returns the receiver zmq port \sa sharedSlsDetector
	 * @returns the receiver zmq port
	 */
	int getReceiverStreamingPort();

	/**
	 * Sets the client zmq ip\sa sharedSlsDetector
	 * @param sourceIP client zmq ip
	 */
	void setClientStreamingIP(const std::string& sourceIP);

	/**
	 * Returns the client zmq ip \sa sharedSlsDetector
	 * @returns the client zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string getClientStreamingIP();

	/**
	 * Sets the receiver zmq ip\sa sharedSlsDetector
	 * @param sourceIP receiver zmq ip. If empty, uses rx_hostname
	 */
	void setReceiverStreamingIP(std::string sourceIP);

	/**
	 * Returns the receiver zmq ip \sa sharedSlsDetector
	 * @returns the receiver zmq ip, returns "none" if default setting and no custom ip set
	 */
	std::string getReceiverStreamingIP();

	/**
	 * Sets the transmission delay for left, right or entire frame
	 * (Eiger, Jungfrau(only entire frame))
	 * @param index type of delay
	 * @param delay delay
	 * @returns transmission delay
	 */
	int setDetectorNetworkParameter(networkParameter index, int delay);

	/**
	 * Sets the additional json header\sa sharedSlsDetector
	 * @param jsonheader additional json header
	 * @returns additional json header, returns "none" if default setting and no custom ip set
	 */
	std::string setAdditionalJsonHeader(const std::string& jsonheader);

	/**
	 * Returns the additional json header \sa sharedSlsDetector
	 * @returns the additional json header, returns "none" if default setting and no custom ip set
	 */
	std::string getAdditionalJsonHeader();

   /**
    * Sets the value for the additional json header parameter if found, else append it
    * @param key additional json header parameter
    * @param value additional json header parameter value (cannot be empty)
    * @returns the additional json header parameter value,
    * empty if no parameter found in additional json header
    */
	std::string setAdditionalJsonParameter(const std::string& key, const std::string& value);

   /**
    * Returns the additional json header parameter value
    * @param key additional json header parameter
    * @returns the additional json header parameter value,
    * empty if no parameter found in additional json header
    */
   std::string getAdditionalJsonParameter(const std::string& key);

	/**
	 * Sets the receiver UDP socket buffer size
	 * @param udpsockbufsize additional json header
	 * @returns receiver udp socket buffer size
	 */
	int setReceiverUDPSocketBufferSize(int udpsockbufsize=-1);

	/**
	 * Returns the receiver UDP socket buffer size\sa sharedSlsDetector
	 * @returns the receiver UDP socket buffer size
	 */
	int getReceiverUDPSocketBufferSize() ;

	/**
	 * Returns the receiver real UDP socket buffer size\sa sharedSlsDetector
	 * @returns the receiver real UDP socket buffer size
	 */
	int getReceiverRealUDPSocketBufferSize();

	/**
	 * Execute a digital test (Gotthard, Mythen)
	 * @param mode testmode type
	 * @param value 1 to set or 0 to clear the digital test bit
	 * @returns result of test
	 */
	int digitalTest(digitalTestMode mode, int ival=-1);

	/**
	 * Load dark or gain image to detector (Gotthard)
	 * @param index image type, 0 for dark image and 1 for gain image
	 * @param fname file name from which to load image
	 * @returns OK or FAIL
	 */
	int loadImageToDetector(imageType index, const std::string& fname);

	/**
	 * Called from loadImageToDetector to send the image to detector
	 * @param index image type, 0 for dark image and 1 for gain image
	 * @param imageVals image
	 * @returns OK or FAIL
	 */
	int sendImageToDetector(imageType index, int16_t imageVals[]);

	/**
	 * Writes the counter memory block from the detector (Gotthard)
	 * @param fname file name to load data from
	 * @param startACQ is 1 to start acquisition after reading counter
	 * @returns OK or FAIL
	 */
	int writeCounterBlockFile(const std::string&  fname,int startACQ=0);

	/**
	 * Gets counter memory block in detector (Gotthard)
	 * @param image counter memory block from detector
	 * @param startACQ 1 to start acquisition afterwards, else 0
	 * @returns OK or FAIL
	 */
	int getCounterBlock(int16_t image[],int startACQ=0);

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
	 * @returns OK or FAIL
	 */
	int setROI(int n=-1,ROI roiLimits[]=nullptr);

	/**
	 * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
	 * @param n number of rois
	 * @returns OK or FAIL
	 */
	slsDetectorDefs::ROI* getROI(int &n);

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
	int sendROI(int n=-1,ROI roiLimits[]=nullptr);

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
	 * @returns OK or FAIL
	 */
	int setAllTrimbits(int val);

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
	int setTrimEn(int nen, int *en=nullptr);

	/**
	 * Returns the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	int getTrimEn(int *en=nullptr);

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
	 * @returns threshold temperature in millidegrees
	 */
	int setThresholdTemperature(int val=-1);

	/**
	 * Enables/disables temperature control (Jungfrau)
	 * @param val value, -1 gets
	 * @returns temperature control enable
	 */
	int setTemperatureControl(int val=-1);

	/**
	 * Resets/ gets over-temperature event (Jungfrau)
	 * @param val value, -1 gets
	 * @returns over-temperature event
	 */
	int setTemperatureEvent(int val=-1);

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
	int programFPGA(const std::string& fname);

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
	 * Returns the trimbits from the detector's shared memmory (Eiger)
	 * @param retval is the array with the trimbits
	 * @returns total number of channels for the detector
	 */
	int getChanRegs(double* retval);

	/**
	 * Configure Module (Eiger)
	 * Called for loading trimbits and settings settings to the detector
	 * @param module module to be set - must contain correct module number and
	 * also channel and chip registers
	 * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
	 * @returns ok or fail
	 * \sa ::sls_detector_module
	 */
	int setModule(sls_detector_module module, int tb = 1);


	/**
	 * Get module structure from detector (all detectors)
	 * @returns pointer to module structure (which has been created and must then be deleted)
	 */
	sls_detector_module *getModule();

	/**
	 * Set Rate correction (Mythen, Eiger)
	 * @param t dead time in ns - if 0 disable correction,
	 * if >0 set dead time to t, if < 0 set deadtime to default dead time
	 * for current settings
	 * @returns 0 if rate correction disabled, >0 otherwise
	 */
	int setRateCorrection(int64_t t = 0);

	/**
	 * Get rate correction Eiger)
	 * @returns 0 if rate correction disabled,  > 0 otherwise
	 */
	int64_t getRateCorrection();

	/**
	 * Prints receiver configuration
	 * #param level print level
	 */
	void printReceiverConfiguration(TLogLevel level = logINFO);

	/**
	 * Checks if receiver is online and set flag
	 * Also initializes the data socekt
	 * @param online 1 to set online, 0 to set offline, -1 gets
	 * @returns online, offline (from shared memory)
	 */
	int setReceiverOnline(int value=GET_ONLINE_FLAG);

	int getReceiverOnline() const;

	/**
	 * Checks if the receiver is really online
	 * @returns empty string if online, else returns receiver hostname
	 */
	std::string checkReceiverOnline();

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
	 * Executes a system command on the receiver server
	 * e.g. mount an nfs disk, reboot and returns answer etc.
	 * @param cmd command to be executed
	 * @returns OK or FAIL
	 */
	int execReceiverCommand(const std::string& cmd);

	/**
     updates the shared memory receiving the data from the detector (without asking and closing the connection
     /returns OK
	 */
	int updateReceiverNoWait(sls::ClientSocket& receiver);

	/**
	 * Updates the shared memory receiving the data from the detector
	 * @returns OK or FAIL
	 */
	int updateReceiver();

	/**
	 * Send the multi detector size to the detector
	 * @param detx number of detectors in x dir
	 * @param dety number of detectors in y dir
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
	std::string setFilePath(const std::string& path);

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
	std::string setFileName(const std::string& fname);

	/**
	 * Sets the max frames per file in receiver
	 * @param f max frames per file
	 * @returns max frames per file in receiver
	 */
	int setReceiverFramesPerFile(int f=-1);

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
	fileFormat setFileFormat(fileFormat f);

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
	int setFileIndex(int i);

	/**
	 * increments file index
	 * @returns the file index
	 */
	int incrementFileIndex();

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
	 * (previously setReadReceiverFrequency)
	 * Sets the receiver streaming frequency
	 * @param freq nth frame streamed out, if 0, streamed out at a timer of 200 ms
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming frequency
	 */
	int setReceiverStreamingFrequency(int freq=-1);

	/**
	 * (previously setReceiverReadTimer)
	 * Sets the receiver streaming timer
	 * If receiver streaming frequency is 0, then this timer between each
	 * data stream is set. Default is 200 ms.
	 * @param time_in_ms timer between frames
	 * @returns receiver streaming timer in ms
	 */
	int setReceiverStreamingTimer(int time_in_ms=500);

	/**
	 * Enable or disable streaming data from receiver to client
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @returns data streaming from receiver enable
	 */
	int enableDataStreamingFromReceiver(int enable=-1);

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
	int setPattern(const std::string& fname);

	/**
	 * Writes a pattern word to the CTB
	 * @param addr address of the word, -1 is I/O control register,  -2 is clk control register
	 * @param word 64bit word to be written, -1 gets
	 * @returns actual value
	 */
	uint64_t setPatternWord(int addr,uint64_t word=-1);

	/**
	 * Sets the pattern or loop limits in the CTB
	 * @param level -1 complete pattern, 0,1,2, loop level
	 * @param start start address if >=0
	 * @param stop stop address if >=0
	 * @param n number of loops (if level >=0)
	 * @returns OK/FAIL
	 */
	int setPatternLoops(int level,int &start, int &stop, int &n);

	/**
	 * Sets the wait address in the CTB
	 * @param level  0,1,2, wait level
	 * @param addr wait address, -1 gets
	 * @returns actual value
	 */
	int setPatternWaitAddr(uint64_t level, uint64_t addr=-1);

	/**
	 * Sets the wait time in the CTB
	 * @param level  0,1,2, wait level
	 * @param t wait time, -1 gets
	 * @returns actual value
	 */
	uint64_t setPatternWaitTime(uint64_t level, uint64_t t=-1);

    /**
     * Sets the mask applied to every pattern
     * @param mask mask to be applied
     * @returns OK or FAIL
     */
    int setPatternMask(uint64_t mask);

    /**
     * Gets the mask applied to every pattern
     * @returns mask set
     */
    uint64_t getPatternMask();

    /**
     * Selects the bits that the mask will be applied to for every pattern
     * @param mask mask to select bits
     * @returns OK or FAIL
     */
    int setPatternBitMask(uint64_t mask);

    /**
     * Gets the bits that the mask will be applied to for every pattern
     * @returns mask  of bits selected
     */
    uint64_t getPatternBitMask();

    /**
     * Set LED Enable (Moench, CTB only)
     * @param enable 1 to switch on, 0 to switch off, -1 gets
     * @returns LED enable
     */
    int setLEDEnable(int enable = -1);

    /**
     * Set Digital IO Delay (Moench, CTB only)
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     * @returns OK or FAIL
     */
    int setDigitalIODelay(uint64_t pinMask, int delay);

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
	 * Send a sls_detector_module structure over socket
	 * @param myMod module structure to send
	 * @returns number of bytes sent to the detector
	 */
	int sendModule(sls_detector_module* myMod);

	/**
	 * Receive a sls_detector_module structure over socket
	 * @param myMod module structure to receive
	 * @returns number of bytes received from the detector
	 */
	int receiveModule(sls_detector_module* myMod);

	/**
	 * Get MAC from the receiver using udpip and
	 * set up UDP connection in detector
	 * @returns Ok or FAIL
	 */
	int setUDPConnection();

	/*
	 * Template function to do linear interpolation between two points (Eiger only)
	 */
	template <typename E, typename V>
	V linearInterpolation(const E x, const E x1, const E x2, const V y1, const V y2){
		double k = static_cast<double>(y2-y1)/(x2-x1);
		double m = y1-k*x1;
		int y = round( k*x+m );
		return static_cast<V>(y);
	}

	/**
	 * interpolates dacs and trimbits between 2 trim files
	 * @param a first module structure
	 * @param b second module structure
	 * @param energy energy to trim at
	 * @param e1 reference trim value
	 * @param e2 reference trim value
	 * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
	 * @returns  the pointer to the module structure with interpolated values or NULL if error
	 */
	sls_detector_module* interpolateTrim(
			sls_detector_module* a, sls_detector_module* b, const int energy,
			const int e1, const int e2, int tb=1);

	/**
	 * reads a trim/settings file
	 * @param fname name of the file to be read
	 * @param myMod pointer to the module structure which has to be set. <BR>
	 * If it is NULL a new module structure will be created
	 * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
	 * @returns the pointer to myMod or NULL if reading the file failed
	 */

	sls_detector_module* readSettingsFile(const std::string& fname, sls_detector_module* myMod=nullptr, int tb=1);

	/**
	 * writes a trim/settings file
	 * @param fname name of the file to be written
	 * @param mod module structure which has to be written to file
	 * @returns OK or FAIL if the file could not be written
	 */
	int writeSettingsFile(const std::string& fname, sls_detector_module mod);


	/** slsDetector Id or position in the detectors list */
	int detId;

	/** Shared Memory object */
	SharedMemory* sharedMemory {nullptr};

	/** Shared memory structure */
	sharedSlsDetector *thisDetector {nullptr};

	/** pointer to detector module structures in shared memory */
	sls_detector_module *detectorModules {nullptr};

	/** pointer to dac valuse in shared memory  */
	int *dacs {nullptr};

	/** pointer to channel registers  in shared memory */
	int *chanregs {nullptr};
};

#endif

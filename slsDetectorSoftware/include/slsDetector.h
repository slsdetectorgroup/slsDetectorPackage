#pragma once
#include "ClientSocket.h"
#include "FixedCapacityContainer.h"
#include "SharedMemory.h"
#include "logger.h"
#include "network_utils.h"
#include "sls_detector_defs.h"

#include <array>
#include <cmath>
#include <vector>

class ServerInterface;

#define SLS_SHMAPIVERSION 0x190726
#define SLS_SHMVERSION 0x190830

/**
 * @short structure allocated in shared memory to store detector settings for
 * IPC and cache
 */
struct sharedSlsDetector {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

    /** shared memory version */
    int shmversion;

    /** is the hostname (or IP address) of the detector. needs to be set
     * before starting the communication */
    char hostname[MAX_STR_LENGTH];

    /** detector type  \ see :: detectorType*/
    slsDetectorDefs::detectorType myDetectorType;

    /** END OF FIXED PATTERN -----------------------------------------------*/

    /** Number of detectors in multi list in x dir and y dir */
    slsDetectorDefs::xy multiSize;

    /** is the port used for control functions */
    int controlPort;

    /** is the port used to stop the acquisition */
    int stopPort;

    /** path of the trimbits/settings files */
    char settingsDir[MAX_STR_LENGTH];

    /** list of the energies at which the detector has been trimmed  */
    sls::FixedCapacityContainer<int, MAX_TRIMEN> trimEnergies;

    /**  number of channels per chip in one direction */
    slsDetectorDefs::xy nChan;

    /**  number of chips per module in one direction */
    slsDetectorDefs::xy nChip;

    /**  number of dacs per module*/
    int nDacs;

    /** dynamic range of the detector data */
    int dynamicRange;

    /** roi */
    slsDetectorDefs::ROI roi;

    /** adc enable mask */
    uint32_t adcEnableMask;

    /** readout mode */
    slsDetectorDefs::readoutMode roMode;

    /** detector settings (standard, fast, etc.) */
    slsDetectorDefs::detectorSettings currentSettings;

    /** detector threshold (eV) */
    int currentThresholdEV;

    /** timer values */
    int64_t timerValue[slsDetectorDefs::timerIndex::MAX_TIMERS];

    /** rate correction in ns */
    int64_t deadTime;

    /** ip address/hostname of the receiver for client control via TCP */
    char rxHostname[MAX_STR_LENGTH];

    /** is the TCP port used to communicate between client and the receiver */
    int rxTCPPort;

    /** is set if the receiver hostname given and is connected,
     * unset if socket connection is not possible  */
    bool useReceiverFlag;

    /** 10 Gbe enable*/
    int tenGigaEnable;

    /** flipped data across x or y axis */
    int flippedDataX;

    /** tcp port from gui/different process to receiver (only data) */
    int zmqport;

    /** tcp port from receiver to gui/different process (only data) */
    int rxZmqport;

    /** data streaming (up stream) enable in receiver */
    bool rxUpstream;

    /* Receiver read frequency */
    int rxReadFreq;

    /**  zmq tcp src ip address in client (only data) **/
    sls::IpAddr zmqip;

    /**  zmq tcp src ip address in receiver (only data) **/
    sls::IpAddr rxZmqip;

    /** gap pixels enable */
    int gappixels;

    /** gap pixels in each direction */
    slsDetectorDefs::xy nGappixels;

    /** additional json header */
    char rxAdditionalJsonHeader[MAX_STR_LENGTH];

    /** receiver frames discard policy */
    slsDetectorDefs::frameDiscardPolicy rxFrameDiscardMode;

    /** receiver partial frames padding enable */
    bool rxFramePadding;

    /** activated receiver */
    bool activated;

    /** padding enable in deactivated receiver */
    bool rxPadDeactivatedModules;

    /** silent receiver */
    bool rxSilentMode;

    /** path of the output files */
    char rxFilePath[MAX_STR_LENGTH];

    /** file name prefix */
    char rxFileName[MAX_STR_LENGTH];

    /** file index */
    int rxFileIndex;

    /** file format */
    slsDetectorDefs::fileFormat rxFileFormat;

    /** frames per file */
    int rxFramesPerFile;

    /** file write enable */
    bool rxFileWrite;

    /** master file write enable */
    bool rxMasterFileWrite;

    /** overwrite enable */
    bool rxFileOverWrite;

    sls::FixedCapacityContainer<int, MAX_RX_DBIT> rxDbitList;

    /** reciever dbit offset */
    int rxDbitOffset;
};

class slsDetector : public virtual slsDetectorDefs {
  public:
    /**
     * Constructor called when creating new shared memory
     * @param type detector type
     * @param multi_id multi detector shared memory id
     * @param id sls detector id (position in detectors list)
     * @param verify true to verify if shared memory version matches existing
     * one
     */
    explicit slsDetector(detectorType type, int multi_id = 0, int det_id = 0,
                         bool verify = true);

    /**
     * Constructor called when opening existing shared memory
     * @param multi_id multi detector shared memory id
     * @param id sls detector id (position in detectors list)
     * @param verify true to verify if shared memory version matches existing
     * one
     */
    explicit slsDetector(int multi_id = 0, int det_id = 0, bool verify = true);

    /**
     * Destructor
     */
    virtual ~slsDetector();

    /**
     * Returns false if it cannot get fixed pattern from an old version of shm
     * (hostname, type), else true
     */
    bool isFixedPatternSharedMemoryCompatible();

    /**
     * Check version compatibility with receiver software
     */
    void checkReceiverVersionCompatibility();

    /**
     * Check version compatibility with detector software
     */
    void checkDetectorVersionCompatibility();

    /**
     * Get ID or version numbers
     * @param mode version type
     * @returns Id or version number of that type
     */
    int64_t getId(idMode mode);

    /**
     * Get Receiver Software version
     */
    int64_t getReceiverSoftwareVersion() const;

    /**
     * Free shared memory and delete shared memory structure
     * occupied by the sharedSlsDetector structure
     * Is only safe to call if one deletes the slsDetector object afterward
     * and frees multi shared memory/updates
     * thisMultiDetector->numberOfDetectors
     */
    void freeSharedMemory();

    /**
     * Sets the hostname, if online flag is set connects to update the detector
     * @param name hostname
     */
    void setHostname(const std::string &hostname);

    /**
     * Gets the hostname of detector
     * @returns hostname
     */
    std::string getHostname() const;

    /**
     * Get detector type by connecting to the detector
     * @returns detector tpe or GENERIC if failed
     */
    static detectorType getTypeFromDetector(const std::string &hostname,
                                            int cport = DEFAULT_PORTNO);

    /**
     * Get Detector type from shared memory variable
     * @returns detector type from shared memory variable
     */
    detectorType getDetectorTypeAsEnum() const;

    /**
     * Gets string version of detector type from shared memory variable
     * @returns string version of detector type from shared memory variable
     */
    std::string getDetectorTypeAsString() const;

    /**
     * Gets detector type from detector and set it in receiver
     * @param type the detector type
     * @returns detector type in receiver
     */
    int setDetectorType(detectorType type = GET_DETECTOR_TYPE);

    /**
     * Update total number of channels (chiptestboard or moench)
     * depending on the number of samples, adenablemask, readout flags(ctb)
     */
    void updateNumberOfChannels();

    /**
     * Returns the total number of channels including gap pixels
     * @returns the total number of channels including gap pixels
     */
    slsDetectorDefs::xy getNumberOfChannels() const;

    /**
     * Get Quad Type (Only for Eiger Quad detector hardware)
     * @returns quad type
     */
    bool getQuad();

    /**
     * Set Quad Type (Only for Eiger Quad detector hardware)
     * @param enable true if quad type set, else false
     */
    void setQuad(const bool enable);

    /**
     * Set number of rows to read out (Only for Eiger)
     * @param value number of lines
     */
    void setReadNLines(const int value);

    /**
     * Get number of rows to read out (Only for Eiger)
     * @returns  number of lines
     */
    int getReadNLines();

    /**
     * Set Detector offset in shared memory in dimension d
     * @param det detector size
     */
    void updateMultiSize(slsDetectorDefs::xy det);

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
    int getReceiverPort() const;

    /**
     * Lock server for this client IP
     * @param p 0 to unlock, 1 to lock (-1 gets)
     * @returns true for locked or false for unlocked
     */
    bool lockServer(int lock = -1);

    /**
     * Get last client IP saved on detector server
     * @returns last client IP saved on detector server
     */
    std::string getLastClientIP();

    /**
     * Exit detector server
     */
    void exitServer();

    /**
     * Executes a system command on the detector server
     * e.g. mount an nfs disk, reboot and returns answer etc.
     * @param cmd command to be executed
     */
    void execCommand(const std::string &cmd);

    /**
     * Updates some of the shared memory receiving the data from the detector
     */
    void updateCachedDetectorVariables();

    /**
     * Get detector specific commands to write into config file
     * @returns vector of strings with commands
     */
    std::vector<std::string> getConfigFileCommands();

    /**
     * Get detector settings
     * @returns current settings
     */
    detectorSettings getSettings();

    /**
     * Load detector settings from the settings file picked from the
     * trimdir/settingsdir Eiger only stores in shared memory ( a get will
     * overwrite this) For Eiger, one must use threshold Gotthard, Propix,
     * Jungfrau and Moench only sends the settings enum to the detector
     * @param isettings settings
     * @returns current settings
     */
    detectorSettings setSettings(detectorSettings isettings);

    /**
     * Send detector settings only (set only for Jungfrau, Gotthard, Moench, get
     * for all) Only the settings enum is sent to the detector, where it will
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
    int setThresholdEnergy(int e_eV, detectorSettings isettings = GET_SETTINGS,
                           int tb = 1);

    /**
     * Set threshold energy and settings (Eiger only)
     * @param e_eV threshold in eV
     * @param isettings ev. change settings
     * @param tb 1 to include trimbits, 0 to exclude
     */
    void setThresholdEnergyAndSettings(int e_eV, detectorSettings isettings,
                                       int tb = 1);

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
    std::string setSettingsDir(const std::string &dir);

    /**
     * Loads the modules settings/trimbits reading from a specific file
     * file name extension is automatically generated.
     * @param fname specific settings/trimbits file
     */
    void loadSettingsFile(const std::string &fname);

    /**
     * Saves the modules settings/trimbits to a specific file
     * file name extension is automatically generated.
     * @param fname specific settings/trimbits file
     */
    void saveSettingsFile(const std::string &fname);

    /**
     * Get run status of the detector
     * @returns the status of the detector
     */
    runStatus getRunStatus() const;

    /**
     * Prepares detector for acquisition (Eiger)
     */
    void prepareAcquisition();

    /**
     * Start detector acquisition (Non blocking)
     */
    void startAcquisition();

    /**
     * Stop detector acquisition
     */
    void stopAcquisition();

    /**
     * Give an internal software trigger to the detector (Eiger only)
     */
    void sendSoftwareTrigger();

    /**
     * Start detector acquisition and read all data (Blocking until end of
     * acquisition)
     */
    void startAndReadAll();

    /**
     * Start readout (without exposure or interrupting exposure) (Eiger store in
     * ram)
     */
    void startReadOut();

    /**
     * Requests and  receives all data from the detector (Eiger store in ram)
     */
    void readAll();

    /**
     * Configures in detector the destination for UDP packets
     */
    void configureMAC();

    /**
     * Set starting frame number for the next acquisition
     * @param val starting frame number
     */
    void setStartingFrameNumber(uint64_t value);

    /**
     * Get starting frame number for the next acquisition
     * @returns starting frame number
     */
    uint64_t getStartingFrameNumber();

    /**
     * Set/get timer value (not all implemented for all detectors)
     * @param index timer index
     * @param t time in ns or number of...(e.g. frames, probes)
     * @returns timer set value in ns or number of...(e.g. frames,
     * probes)
     */
    int64_t setTimer(timerIndex index, int64_t t = -1);

    /**
     * Set/get timer value left in acquisition (not all implemented for all
     * detectors)
     * @param index timer index
     * @param t time in ns or number of...(e.g. frames, probes)
     * @returns timer set value in ns or number of...(e.g. frames,
     * probes)
     */
    int64_t getTimeLeft(timerIndex index) const;

    /**
     * Set speed
     * @param sp speed type  (clkdivider option for Jungfrau and Eiger,
     * adcphase for Gotthard, others for CTB & Moench)
     * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other
     * values check manual
     * @param mode 0 for shift, 1 for degrees. relevant only for speed type
     * adcphase and dbit phase
     * @returns value of speed set
     */
    int setSpeed(speedVariable sp, int value = -1, int mode = 0);

    /**
     * Set/get dynamic range
     * (Eiger: If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to
     * 1)
     * @param i dynamic range (-1 get)
     * @returns current dynamic range
     * \sa sharedSlsDetector
     */
    int setDynamicRange(int n = -1);

    int getDynamicRangeFromShm();

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
     * @returns current adc value (temperature for eiger and jungfrau in
     * millidegrees)
     */
    int getADC(dacIndex index);

    /**
     * Set/get timing mode
     * @param pol timing mode (-1 gets)
     * @returns current timing mode
     */
    timingMode setTimingMode(timingMode pol = GET_TIMING_MODE);

    /**
     * Set/get external signal flags (to specify triggerinrising edge etc)
     * (Gotthard, Mythen)
     * @param pol external signal flag (-1 gets)
     * @returns current timing mode
     */
    externalSignalFlag
    setExternalSignalFlags(externalSignalFlag pol = GET_EXTERNAL_SIGNAL_FLAG);

    /**
     * Set Parallel readout mode (Only for Eiger)
     * @param enable true if parallel, else false for non parallel
     */
    void setParallelMode(const bool enable);

    /**
     * Get parallel mode (Only for Eiger)
     * @returns parallel mode
     */
    bool getParallelMode();

    /**
     * Set overflow readout mode in 32 bit mode (Only for Eiger)
     * @param enable true if overflow, else false
     */
    void setOverFlowMode(const bool enable);
    
    /**
     * Get overflow mode in 32 bit mode (Only for Eiger)
     * @returns overflow mode
     */
    bool getOverFlowMode();

    /**
     * Set store in ram readout mode (Only for Eiger)
     * @param enable true if store in ram, else false
     */
    void setStoreInRamMode(const bool enable);

    /**
     * Get store in ram mode (Only for Eiger)
     * @returns store in ram mode
     */
    bool getStoreInRamMode();

    /**
     * Set readout mode (Only for CTB and Moench)
     * @param mode readout mode Options: ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL
     */
    void setReadoutMode(const readoutMode mode);

    /**
     * Get readout mode(Only for CTB and Moench)
     * @returns readout mode
     */
    readoutMode getReadoutMode();

    /**
     * Set Interrupt last sub frame (Only for Eiger)
     * @param enable true if interrupt last subframe set, else false
     */
    void setInterruptSubframe(const bool enable);

    /**
     * Get Interrupt last sub frame (Only for Eiger)
     * @returns true if interrupt last subframe set, else false
     */
    bool getInterruptSubframe();

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
     * Validates and sets the receiver.
     * Also updates the receiver with all the shared memory parameters
     * significant for the receiver Also configures the detector to the receiver
     * as UDP destination
     * @param receiver receiver hostname or IP address
     * @returns the receiver IP address from shared memory
     */
    std::string setReceiverHostname(const std::string &receiver);

    /**
     * Returns the receiver IP address\sa sharedSlsDetector
     * @returns the receiver IP address
     */
    std::string getReceiverHostname() const;

   /**
     * Validates the format of the detector MAC address and sets it 
     * @param mac detector MAC address
     */
    void setSourceUDPMAC(const sls::MacAddr mac);

    /**
     * Returns the detector MAC address
     * @returns the detector MAC address
     */
    sls::MacAddr getSourceUDPMAC();

    /**
     * Validates the format of the detector MAC address (bottom half) and sets
     * it (Jungfrau only)
     * @param mac detector MAC address (bottom half)
     */
    void setSourceUDPMAC2(const sls::MacAddr mac);

    /**
     * Returns the detector MAC address (bottom half) Jungfrau only
     * @returns the detector MAC address (bottom half)
     */
    sls::MacAddr getSourceUDPMAC2();

    /**
     * Validates the format of the detector IP address and sets it
     * @param ip detector IP address
     */
    void setSourceUDPIP(const sls::IpAddr ip);

    /**
     * Returns the detector IP address
     * @returns the detector IP address
     */
    sls::IpAddr getSourceUDPIP();

    /**
     * Validates the format of the detector IP address (bottom half) and sets it
     * (Jungfrau only)
     * @param ip detector IP address (bottom half)
     */
    void setSourceUDPIP2(const sls::IpAddr ip);

    /**
     * Returns the detector IP address (bottom half) Jungfrau only
     * @returns the detector IP address (bottom half)
     */
    sls::IpAddr getSourceUDPIP2();

    /**
     * Validates the format of the receiver UDP IP address and sets it
     * If slsReceiver used, Gets receiver udp mac address and sends it to the detector
     * @param ip receiver UDP IP address
     */
    void setDestinationUDPIP(const sls::IpAddr ip);

    /**
     * Returns the receiver UDP IP address
     * If slsReceiver used, Gets receiver udp mac address and sends it to the detector
     * @returns the receiver UDP IP address
     */
    sls::IpAddr getDestinationUDPIP();

    /**
     * Gets destination udp ip from detector,
     * if 0, it converts rx_hostname to ip and 
     * updates both detector and receiver
     */
    void updateRxDestinationUDPIP();

    /**
     * Validates the format of the receiver UDP IP address (bottom half) and
     * sets it(Jungfrau only)
     * If slsReceiver used, Gets receiver udp mac address2 and sends it to the detector
     * @param ip receiver UDP IP address (bottom half)
     */
    void setDestinationUDPIP2(const sls::IpAddr ip);

    /**
     * Returns the receiver UDP IP address (bottom half) Jungfrau only
     * If slsReceiver used, Gets receiver udp mac address2 and sends it to the detector
     * @returns the receiver UDP IP address (bottom half)
     */
    sls::IpAddr getDestinationUDPIP2();
  
    /**
     * Gets destination udp ip2 from detector,
     * if 0, it converts rx_hostname to ip and 
     * updates both detector and receiver
     */
    void updateRxDestinationUDPIP2();
    
    /**
     * Validates the format of the receiver UDP MAC address and sets it 
     * @param mac receiver UDP MAC address
     */
    void setDestinationUDPMAC(const sls::MacAddr mac);

    /**
     * Returns the receiver UDP MAC address
     * @returns the receiver UDP MAC address
     */
    sls::MacAddr getDestinationUDPMAC();

    /**
     * Validates the format of the receiver UDP MAC address  (bottom half) and
     * sets it (Jungfrau only)
     * @param mac receiver UDP MAC address (bottom half)
     */
    void setDestinationUDPMAC2(const sls::MacAddr mac);

    /**
     * Returns the receiver UDP MAC address (bottom half) Jungfrau only
     * @returns the receiver UDP MAC address (bottom half)
     */
    sls::MacAddr getDestinationUDPMAC2();

    /**
     * Sets the receiver UDP port\sa sharedSlsDetector
     * @param udpport receiver UDP port
     */
    void setDestinationUDPPort(int udpport);

    /**
     * Returns the receiver UDP port\sa sharedSlsDetector
     * @returns the receiver UDP port
     */
    int getDestinationUDPPort();

    /**
     * Sets the receiver UDP port 2\sa sharedSlsDetector (Eiger and Jungfrau
     * only)
     * @param udpport receiver UDP port 2
     */
    void setDestinationUDPPort2(int udpport);

    /**
     * Returns the receiver UDP port 2 of same interface\sa sharedSlsDetector
     * (Eiger and Jungfrau only)
     * @returns the receiver UDP port 2 of same interface
     */
    int getDestinationUDPPort2();

    /**
     * Sets the number of UDP interfaces to stream data from detector (Jungfrau
     * only)
     * @param n number of interfaces. Options 1 or 2.
     * @returns the number of interface
     */
    void setNumberofUDPInterfaces(int n);

    /**
     * Returns the number of UDP interfaces to stream data from detector
     * (Jungfrau only)
     * @returns the number of interfaces
     */
    int getNumberofUDPInterfaces();

    /**
     * Selects the UDP interfaces to stream data from detector. Effective only
     * when number of interfaces is 1. (Jungfrau only)
     * @param n selected interface. Options 1 or 2.
     * @returns the interface selected
     */
    void selectUDPInterface(int n);

    /**
     * Returns the UDP interfaces to stream data from detector. Effective only
     * when number of interfaces is 1. (Jungfrau only)
     * @returns the interface selected
     */
    int getSelectedUDPInterface();

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
    void setClientStreamingIP(const std::string &sourceIP);

    /**
     * Returns the client zmq ip \sa sharedSlsDetector
     * @returns the client zmq ip, returns "none" if default setting and no
     * custom ip set
     */
    std::string getClientStreamingIP();

    /**
     * Sets the receiver zmq ip\sa sharedSlsDetector
     * @param sourceIP receiver zmq ip. If empty, uses rx_hostname
     */
    void setReceiverStreamingIP(std::string sourceIP);

    /**
     * Returns the receiver zmq ip \sa sharedSlsDetector
     * @returns the receiver zmq ip, returns "none" if default setting and no
     * custom ip set
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
     * @returns additional json header, returns "none" if default setting and no
     * custom ip set
     */
    std::string setAdditionalJsonHeader(const std::string &jsonheader);

    /**
     * Returns the additional json header \sa sharedSlsDetector
     * @returns the additional json header, returns "none" if default setting
     * and no custom ip set
     */
    std::string getAdditionalJsonHeader();

    /**
     * Sets the value for the additional json header parameter if found, else
     * append it
     * @param key additional json header parameter
     * @param value additional json header parameter value (cannot be empty)
     * @returns the additional json header parameter value,
     * empty if no parameter found in additional json header
     */
    std::string setAdditionalJsonParameter(const std::string &key,
                                           const std::string &value);

    /**
     * Returns the additional json header parameter value
     * @param key additional json header parameter
     * @returns the additional json header parameter value,
     * empty if no parameter found in additional json header
     */
    std::string getAdditionalJsonParameter(const std::string &key);

    /**
     * Sets the receiver UDP socket buffer size
     * @param udpsockbufsize additional json header
     * @returns receiver udp socket buffer size
     */
    int64_t setReceiverUDPSocketBufferSize(int64_t udpsockbufsize = -1);

    /**
     * Returns the receiver UDP socket buffer size\sa sharedSlsDetector
     * @returns the receiver UDP socket buffer size
     */
    int64_t getReceiverUDPSocketBufferSize();

    /**
     * Returns the receiver real UDP socket buffer size\sa sharedSlsDetector
     * @returns the receiver real UDP socket buffer size
     */
    int64_t getReceiverRealUDPSocketBufferSize() const;

    /**
     * Execute a digital test (Gotthard, Jungfrau, CTB)
     * @param mode testmode type
     * @param value 1 to set or 0 to clear the digital test bit
     * @returns result of test
     */
    int digitalTest(digitalTestMode mode, int ival = -1);

    /**
     * Set/get counter bit in detector (Gotthard)
     * @param i is -1 to get, 0 to reset and any other value to set the counter
     * bit
     * @returns the counter bit in detector
     */
    int setCounterBit(int cb = -1);

    /**
     * Clear ROI (Gotthard)
     */
    void clearROI();

    /**
     * Set ROI (Gotthard)
     * Also calls configuremac
     * @param arg roi
     */
    void setROI(slsDetectorDefs::ROI arg);

    /**
     *  Send ROI from shared memory to Receiver (Gotthard)
     */
    void sendROItoReceiver();

    /**
     * Get ROI (Gotthard)
     * Update receiver if different from shm
     * @returns roi
     */
    slsDetectorDefs::ROI getROI();

    /**
     * Set ADC Enable Mask (CTB, Moench)
     * @param mask ADC Enable mask
     */
    void setADCEnableMask(uint32_t mask);

    /**
     * Get ADC Enable Mask (CTB, Moench)
     * @returns ADC Enable mask
     */
    uint32_t getADCEnableMask();

    /**
     * Set ADC invert register (CTB, Moench)
     * @param value ADC invert value
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setADCInvert(uint32_t value);

    /**
     * Get ADC invert register (CTB, Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns ADC invert value
     */
    uint32_t getADCInvert();

    /**
     * Set external sampling source (CTB only)
     * @param value external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling source
     */
    int setExternalSamplingSource(int value);

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling source
     */
    int getExternalSamplingSource();

    /**
     * Set external sampling enable (CTB only)
     * @param value external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns  external sampling enable
     */
    int setExternalSampling(int value);

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling enable
     */
    int getExternalSampling();

    /**
     * Set external sampling enable (CTB only)
     * @param list external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReceiverDbitList(std::vector<int> list);

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling enable
     */
    std::vector<int> getReceiverDbitList() const;

    /**
     * Set digital data offset in bytes (CTB only)
     * @param value digital data offset in bytes
     * @returns digital data offset in bytes
     */
    int setReceiverDbitOffset(int value);

    /**
     * Get digital data offset in bytes (CTB only)
     * @returns digital data offset in bytes
     */
    int getReceiverDbitOffset();

    /**
     * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert
     * users
     * @param addr address of adc register
     * @param val value
     */
    void writeAdcRegister(uint32_t addr, uint32_t val);

    /**
     * Activates/Deactivates the detector (Eiger only)
     * @param enable active (1) or inactive (0), -1 gets
     * @returns 0 (inactive) or 1 (active)for activate mode
     */
    int activate(int const enable = -1);

    /**
     * Set deactivated Receiver padding mode (Eiger only)
     * @param padding padding option for deactivated receiver. Can be 1
     * (padding), 0 (no padding), -1 (gets)
     * @returns 1 (padding), 0 (no padding), -1 (inconsistent values) for
     * padding option
     */
    bool setDeactivatedRxrPaddingMode(int padding = -1);

    /**
     * Returns the enable if data will be flipped across x axis (Eiger)
     * @returns 1 for flipped, else 0
     */
    int getFlippedDataX() const;

    /**
     * Sets the enable which determines if
     * data will be flipped across x axis (Eiger)
     * @param value 0 or 1 to reset/set or -1 to get value
     * @returns enable flipped data across x axis
     */
    int setFlippedDataX(int value = -1);

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
    int enableGapPixels(int val = -1);

    /**
     * Sets the number of trim energies and their value  (Eiger)
     * \sa sharedSlsDetector
     * @param nen number of energies
     * @param vector os trimmed energies
     * @returns number of trim energies
     */
    int setTrimEn(std::vector<int> energies = {});

    /**
     * Returns a vector with the trimmed energies  (Eiger)
     * \sa sharedSlsDetector
     * @returns vector with the trimmed energies
     */
    std::vector<int> getTrimEn();

    /**
     * Pulse Pixel (Eiger)
     * @param n is number of times to pulse
     * @param x is x coordinate
     * @param y is y coordinate
     */
    void pulsePixel(int n = 0, int x = 0, int y = 0);

    /**
     * Pulse Pixel and move by a relative value (Eiger)
     * @param n is number of times to pulse
     * @param x is relative x value
     * @param y is relative y value
     */
    void pulsePixelNMove(int n = 0, int x = 0, int y = 0);

    /**
     * Pulse Chip (Eiger)
     * @param n is number of times to pulse
     */
    void pulseChip(int n_pulses = 0);

    /**
     * Set/gets threshold temperature (Jungfrau)
     * @param val value in millidegrees, -1 gets
     * @returns threshold temperature in millidegrees
     */
    int setThresholdTemperature(int val = -1);

    /**
     * Enables/disables temperature control (Jungfrau)
     * @param val value, -1 gets
     * @returns temperature control enable
     */
    int setTemperatureControl(int val = -1);

    /**
     * Resets/ gets over-temperature event (Jungfrau)
     * @param val value, -1 gets
     * @returns over-temperature event
     */
    int setTemperatureEvent(int val = -1);

    /**
     * Set storage cell that stores first acquisition of the series (Jungfrau)
     * @param value storage cell index. Value can be 0 to 15. (-1 gets)
     * @returns the storage cell that stores the first acquisition of the series
     */
    int setStoragecellStart(int pos = -1);

    /**
     * Programs FPGA with pof file (Jungfrau, CTB, Moench)
     * @param buffer programming file in memory
     */
    void programFPGA(std::vector<char> buffer);

    /**
     * Resets FPGA (Jungfrau)
     */
    void resetFPGA();

    /**
     * Copies detector server from tftp and changes respawn server (Not Eiger)
     * @param fname name of detector server binary
     * @param hostname name of pc to tftp from
     */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname);

    /**
     * Reboot detector controller (blackfin/ powerpc)
     */
    void rebootController();

    /**
     * Power on/off Chip (Jungfrau)
     * @param ival on is 1, off is 0, -1 to get
     * @returns OK or FAIL
     */
    int powerChip(int ival = -1);

    /**
     * Automatic comparator disable (Jungfrau)
     * @param ival on is 1, off is 0, -1 to get
     * @returns OK or FAIL
     */
    int setAutoComparatorDisableMode(int ival = -1);

    /**
     * Get trimbit filename with path for settings and energy
     *
     */
    std::string getTrimbitFilename(detectorSettings settings, int e_eV);

    /**
     * Configure Module (Eiger)
     * Called for loading trimbits and settings settings to the detector
     * @param module module to be set - must contain correct module number and
     * also channel and chip registers
     * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
     * \sa ::sls_detector_module
     */
    void setModule(sls_detector_module &module, int tb = 1);

    /**
     * Get module structure from detector (all detectors)
     * @returns pointer to module structure (which has been created and must
     * then be deleted)
     */
    sls_detector_module getModule();

    /**
     * Set Default Rate correction from trimbit file(Eiger)
     */
    void setDefaultRateCorrection();

    /**
     * Set Rate correction (Eiger)
     * @param t dead time in ns - if 0 disable correction,
     * if >0 set dead time to t, cannot be < 0
     * for current settings
     */
    void setRateCorrection(int64_t t = 0);

    /**
     * Get rate correction (Eiger)
     * @returns 0 if rate correction disabled,  > 0 otherwise
     */
    int64_t getRateCorrection();

    /**
     * Update rate correction according to dynamic range (Eiger)
     * If rate correction enabled and dr is 8 or 16, it will throw
     * Otherwise update ratecorrection if enabled
     */
    void updateRateCorrection();

    /**
     * Prints receiver configuration
     * @returns receiver configuration
     */
    std::string printReceiverConfiguration();

    /**
     * Gets the use receiver flag from shared memory
     */
    bool getUseReceiverFlag() const;

    /**
     * Locks/Unlocks the connection to the receiver
     * @param lock sets (1), usets (0), gets (-1) the lock
     * @returns lock status of the receiver
     */
    int lockReceiver(int lock = -1);

    /**
     * Returns the IP of the last client connecting to the receiver
     * @returns the IP of the last client connecting to the receiver
     */
    std::string getReceiverLastClientIP() const;

    /**
     * Exits the receiver TCP server
     */
    void exitReceiver();

    /**
     * Executes a system command on the receiver server
     * e.g. mount an nfs disk, reboot and returns answer etc.
     * @param cmd command to be executed
     */
    void execReceiverCommand(const std::string &cmd);

    /**
     * Updates the shared memory receiving the data from the detector
     */
    void updateCachedReceiverVariables() const;

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
    std::string setFilePath(const std::string &path);

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
    std::string setFileName(const std::string &fname);

    /**
     * Sets the max frames per file in receiver
     * @param f max frames per file
     * @returns max frames per file in receiver
     */
    int setFramesPerFile(int n_frames);

    int getFramesPerFile() const;

    /**
     * Sets the frames discard policy in receiver
     * @param f frames discard policy
     * @returns frames discard policy set in receiver
     */
    frameDiscardPolicy setReceiverFramesDiscardPolicy(
        frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY);

    /**
     * Sets the partial frames padding enable in receiver
     * @param f partial frames padding enable
     * @returns partial frames padding enable in receiver
     */
    bool setPartialFramesPadding(bool padding);
    bool getPartialFramesPadding() const;

    /**
     * Returns file format
     * @returns file format
     */
    fileFormat getFileFormat() const;

    /**
     * Sets up the file format
     * @param f file format
     * @returns file format
     */
    fileFormat setFileFormat(fileFormat f);

    /**
     * Sets up the file index
     * @param i file index
     * @returns file index
     */
    int setFileIndex(int file_index);

    /**
     * Gets the file index
     * @returns file index
     */

    int getFileIndex() const;
    /**
     * increments file index
     * @returns the file index
     */
    int incrementFileIndex();

    /**
     * Receiver starts listening to packets
     */
    void startReceiver();

    /**
     * Stops the listening mode of receiver
     */
    void stopReceiver();

    /**
     * Gets the status of the listening mode of receiver
     * @returns status
     */
    runStatus getReceiverStatus() const;

    /**
     * Gets the number of frames caught by receiver
     * @returns number of frames caught by receiver
     */
    int getFramesCaughtByReceiver() const;

    /**
     * Gets the current frame index of receiver
     * @returns current frame index of receiver
     */
    uint64_t getReceiverCurrentFrameIndex() const;

    /**
     * Resets framescaught in receiver
     * Use this when using startAcquisition instead of acquire
     */
    void resetFramesCaught();

    /**
     * Sets/Gets receiver file write enable
     * @param enable 1 or 0 to set/reset file write enable
     * @returns file write enable
     */
    bool setFileWrite(bool value);

    /**
     * Gets file write enable
     * @returns file write enable
     */
    bool getFileWrite() const;

    /**
     * Sets/Gets receiver master file write enable
     * @param value 1 or 0 to set/reset master file write enable
     * @returns master file write enable
     */
    bool setMasterFileWrite(bool value);

    /**
     * Gets master file write enable
     * @returns master file write enable
     */
    bool getMasterFileWrite() const;

    /**
     * Sets file overwrite in the receiver
     * @param enable true or false to set/reset file overwrite enable
     * @returns file overwrite enable
     */
    bool setFileOverWrite(bool value);

    /**
     * Gets file overwrite in the receiver
     * @returns file overwrite enable
     */
    bool getFileOverWrite() const;

    /**
     * (previously setReadReceiverFrequency)
     * Sets the receiver streaming frequency
     * @param freq nth frame streamed out, if 0, streamed out at a timer of 200
     * ms
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns receiver streaming frequency
     */
    int setReceiverStreamingFrequency(int freq = -1);

    /**
     * (previously setReceiverReadTimer)
     * Sets the receiver streaming timer
     * If receiver streaming frequency is 0, then this timer between each
     * data stream is set. Default is 200 ms.
     * @param time_in_ms timer between frames
     * @returns receiver streaming timer in ms
     */
    int setReceiverStreamingTimer(int time_in_ms = 200);

    /**
     * Enable or disable streaming data from receiver to client
     * @param enable 0 to disable 1 to enable -1 to only get the value
     * @returns data streaming from receiver enable
     */
    bool enableDataStreamingFromReceiver(int enable = -1);

    /**
     * Enable/disable or 10Gbe
     * @param i is -1 to get, 0 to disable and 1 to enable
     * @returns if 10Gbe is enabled
     */
    int enableTenGigabitEthernet(int value = -1);

    /**
     * Set/get receiver fifo depth
     * @param i is -1 to get, any other value to set the fifo deph
     * @returns the receiver fifo depth
     */
    int setReceiverFifoDepth(int n_frames = -1);

    /**
     * Set/get receiver silent mode
     * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
     * @returns the receiver silent mode enable
     */
    bool setReceiverSilentMode(int value = -1);

    /**
     * If data streaming in receiver is enabled,
     * restream the stop dummy packet from receiver
     * Used usually for Moench,
     * in case it is lost in network due to high data rate
     */
    void restreamStopFromReceiver();

    /**
     * Opens pattern file and sends pattern to CTB
     * @param fname pattern file to open
     */
    void setPattern(const std::string &fname);

    /**
     * Sets pattern IO control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @returns actual value
     */
    uint64_t setPatternIOControl(uint64_t word = -1);

    /**
     * Sets pattern clock control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @returns actual value
     */
    uint64_t setPatternClockControl(uint64_t word = -1);

    /**
     * Writes a pattern word (CTB/ Moench)
     * @param addr address of the word
     * @param word 64bit word to be written, -1 reads the addr (same as
     * executing the pattern)
     * @returns actual value
     */
    uint64_t setPatternWord(int addr, uint64_t word);

    /**
     * Sets the pattern or loop limits (CTB/ Moench)
     * @param level -1 complete pattern, 0,1,2, loop level
     * @param start start address for level 0-2, -1 gets
     * @param stop stop address for level 0-2, -1 gets
     * @param n number of loops for level 0-2, -1 gets
     * @returns array of start addr, stop addr and number of loops
     */
    std::array<int, 3> setPatternLoops(int level = -1, int start = -1,
                                       int stop = -1, int n = -1);

    /**
     * Sets the wait address (CTB/ Moench)
     * @param level  0,1,2, wait level
     * @param addr wait address, -1 gets
     * @returns actual value
     */
    int setPatternWaitAddr(int level, int addr = -1);

    /**
     * Sets the wait time (CTB/ Moench)
     * @param level  0,1,2, wait level
     * @param t wait time, -1 gets
     * @returns actual value
     */
    uint64_t setPatternWaitTime(int level, uint64_t t = -1);

    /**
     * Sets the mask applied to every pattern (CTB/ Moench)
     * @param mask mask to be applied
     */
    void setPatternMask(uint64_t mask);

    /**
     * Gets the mask applied to every pattern (CTB/ Moench)
     * @returns mask set
     */
    uint64_t getPatternMask();

    /**
     * Selects the bits that the mask will be applied to for every pattern (CTB/
     * Moench)
     * @param mask mask to select bits
     */
    void setPatternBitMask(uint64_t mask);

    /**
     * Gets the bits that the mask will be applied to for every pattern (CTB/
     * Moench)
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
     */
    void setDigitalIODelay(uint64_t pinMask, int delay);

  private:
    /**
     * Send function parameters to detector (control server)
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
    void sendToDetector(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size);

    template <typename Arg, typename Ret>
    void sendToDetector(int fnum, const Arg &args, Ret &retval);  
    template <typename Arg>
    void sendToDetector(int fnum, const Arg &args, std::nullptr_t);
    template <typename Ret>
    void sendToDetector(int fnum, std::nullptr_t, Ret &retval);
    void sendToDetector(int fnum);

    /**
     * Send function parameters to detector (stop server)
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
    void sendToDetectorStop(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size);

    void sendToDetectorStop(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const;

    template <typename Arg, typename Ret>
    void sendToDetectorStop(int fnum, const Arg &args, Ret &retval);

    template <typename Arg, typename Ret>
    void sendToDetectorStop(int fnum, const Arg &args, Ret &retval) const;

    template <typename Arg>
    void sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t);

    template <typename Arg>
    void sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t) const;

    template <typename Ret>
    void sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval);

    template <typename Ret>
    void sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) const;

    void sendToDetectorStop(int fnum);

    void sendToDetectorStop(int fnum) const;

    /**
     * Send function parameters to receiver
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
    void sendToReceiver(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size);

    void sendToReceiver(int fnum, const void *args, size_t args_size,
                        void *retval, size_t retval_size) const;

    template <typename Arg, typename Ret>
    void sendToReceiver(int fnum, const Arg &args, Ret &retval);

    template <typename Arg, typename Ret>
    void sendToReceiver(int fnum, const Arg &args, Ret &retval) const;

    template <typename Arg>
    void sendToReceiver(int fnum, const Arg &args, std::nullptr_t);

    template <typename Arg>
    void sendToReceiver(int fnum, const Arg &args, std::nullptr_t) const;

    template <typename Ret>
    void sendToReceiver(int fnum, std::nullptr_t, Ret &retval);

    template <typename Ret>
    void sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const;

    void sendToReceiver(int fnum);

    void sendToReceiver(int fnum) const;

    /**
     * Get Detector Type from Shared Memory (opening shm without verifying size)
     * @param multi_id multi detector Id
     * @param verify true to verify if shm size matches existing one
     * @returns detector type
     */
    detectorType getDetectorTypeFromShm(int multi_id, bool verify = true);

    /**
     * Initialize shared memory
     * @param created true if shared memory must be created, else false to open
     * @param type type of detector
     * @param multi_id multi detector Id
     * @param verify true to verify if shm size matches existing one
     * @returns true if the shared memory was created now
     */
    void initSharedMemory(detectorType type, int multi_id, bool verify = true);

    /**
     * Initialize detector structure to defaults
     * Called when new shared memory is created
     * @param type type of detector
     */
    void initializeDetectorStructure(detectorType type);

    /**
     * Send a sls_detector_module structure over socket
     * @param myMod module structure to send
     * @returns number of bytes sent to the detector
     */
    int sendModule(sls_detector_module *myMod, sls::ClientSocket &client);

    /**
     * Receive a sls_detector_module structure over socket
     * @param myMod module structure to receive
     * @returns number of bytes received from the detector
     */
    int receiveModule(sls_detector_module *myMod, sls::ClientSocket &client);

    /**
     * Get MAC from the receiver using udpip and
     * set up UDP connection in detector
     */
    void setUDPConnection();

    /*
     * Template function to do linear interpolation between two points (Eiger
     * only)
     */
    template <typename E, typename V>
    V linearInterpolation(const E x, const E x1, const E x2, const V y1,
                          const V y2) {
        double k = static_cast<double>(y2 - y1) / (x2 - x1);
        double m = y1 - k * x1;
        int y = round(k * x + m);
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
     * @returns  the pointer to the module structure with interpolated values or
     * NULL if error
     */
    sls_detector_module interpolateTrim(sls_detector_module *a,
                                        sls_detector_module *b,
                                        const int energy, const int e1,
                                        const int e2, int tb = 1);

    /**
     * reads a trim/settings file
     * @param fname name of the file to be read
     * @param myMod pointer to the module structure which has to be set. <BR>
     * If it is NULL a new module structure will be created
     * @param tb 1 to include trimbits, 0 to exclude (used for eiger)
     * @returns the pointer to myMod or NULL if reading the file failed
     */

    sls_detector_module readSettingsFile(const std::string &fname, int tb = 1);

    /**
     * writes a trim/settings file
     * @param fname name of the file to be written
     * @param mod module structure which has to be written to file
     */
    void writeSettingsFile(const std::string &fname, sls_detector_module &mod);

    /**
     * Get Names of dacs in settings file
     * @returns vector dac names expected in settings file
     */
    std::vector<std::string> getSettingsFileDacNames();

    /** slsDetector Id or position in the detectors list */
    const int detId;

    /** Shared Memory object */
    mutable sls::SharedMemory<sharedSlsDetector> shm{0, 0};
};

#pragma once

#include "Result.h"
#include "SharedMemory.h"
#include "logger.h"
#include "sls_detector_defs.h"

class slsDetector;
class ZmqSocket;
class detectorData;

#include <memory>
#include <mutex>
#include <semaphore.h>
#include <string>
#include <thread>
#include <vector>

#define MULTI_SHMAPIVERSION 0x190809
#define MULTI_SHMVERSION 0x190814
#define SHORT_STRING_LENGTH 50
#define DATE_LENGTH 30

#include <future>
#include <numeric>
/**
 * @short structure allocated in shared memory to store detector settings
 * for IPC and cache
 */
struct sharedMultiSlsDetector {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND
     * ------*/

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

    /** multi detector type */
    slsDetectorDefs::detectorType multiDetectorType;

    /** END OF FIXED PATTERN
     * -----------------------------------------------*/

    /** Number of detectors operated at once */
    slsDetectorDefs::xy numberOfDetector;

    /**  max number of channels for complete detector*/
    slsDetectorDefs::xy numberOfChannels;

    /** flag for acquiring */
    bool acquiringFlag;

    /** data streaming (up stream) enable in receiver */
    bool receiver_upstream;
};

class multiSlsDetector : public virtual slsDetectorDefs {
  public:
    /**
     * Constructor
     * @param id multi detector id
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    explicit multiSlsDetector(int multi_id = 0, bool verify = true,
                              bool update = true);

    /**
     * Destructor
     */
    virtual ~multiSlsDetector();

    template <class CT> struct NonDeduced { using type = CT; };
    template <typename RT, typename... CT>
    sls::Result<RT> Parallel(RT (slsDetector::*somefunc)(CT...),
                             std::vector<int> positions,
                             typename NonDeduced<CT>::type... Args) {

        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        sls::Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename RT, typename... CT>
    sls::Result<RT> Parallel(RT (slsDetector::*somefunc)(CT...) const,
                             std::vector<int> positions,
                             typename NonDeduced<CT>::type... Args) const {

        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<RT>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        sls::Result<RT> result;
        result.reserve(positions.size());
        for (auto &i : futures) {
            result.push_back(i.get());
        }
        return result;
    }

    template <typename... CT>
    void Parallel(void (slsDetector::*somefunc)(CT...),
                  std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) {

        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    template <typename... CT>
    void Parallel(void (slsDetector::*somefunc)(CT...) const,
                  std::vector<int> positions,
                  typename NonDeduced<CT>::type... Args) const {

        if (positions.empty() ||
            (positions.size() == 1 && positions[0] == -1)) {
            positions.resize(detectors.size());
            std::iota(begin(positions), end(positions), 0);
        }
        std::vector<std::future<void>> futures;
        futures.reserve(positions.size());
        for (size_t i : positions) {
            if (i >= detectors.size())
                throw sls::RuntimeError("Detector out of range");
            futures.push_back(std::async(std::launch::async, somefunc,
                                         detectors[i].get(), Args...));
        }
        for (auto &i : futures) {
            i.get();
        }
    }

    /**
     * Loop through the detectors serially and return the result as a vector
     */

    template <typename RT, typename... CT>
    std::vector<RT> serialCall(RT (slsDetector::*somefunc)(CT...),
                               typename NonDeduced<CT>::type... Args);

    /**
     * Loop through the detectors serially and return the result as a vector
     * Const qualified version
     */
    template <typename RT, typename... CT>
    std::vector<RT> serialCall(RT (slsDetector::*somefunc)(CT...) const,
                               typename NonDeduced<CT>::type... Args) const;

    /**
     * Loop through the detectors in parallel and return the result as a vector
     */
    template <typename RT, typename... CT>
    std::vector<RT> parallelCall(RT (slsDetector::*somefunc)(CT...),
                                 typename NonDeduced<CT>::type... Args);

    /**
     * Loop through the detectors in parallel and return the result as a vector
     * Const qualified version
     */
    template <typename RT, typename... CT>
    std::vector<RT> parallelCall(RT (slsDetector::*somefunc)(CT...) const,
                                 typename NonDeduced<CT>::type... Args) const;

    template <typename... CT>
    void parallelCall(void (slsDetector::*somefunc)(CT...),
                      typename NonDeduced<CT>::type... Args);

    template <typename... CT>
    void parallelCall(void (slsDetector::*somefunc)(CT...) const,
                      typename NonDeduced<CT>::type... Args) const;

    /**
     * Set acquiring flag in shared memory
     * @param b acquiring flag
     */
    void setAcquiringFlag(bool flag); //

    /**
     * Get acquiring flag from shared memory
     * @returns acquiring flag
     */
    bool getAcquiringFlag() const; //

    /**
     * Check version compatibility with detector software
     * (if hostname/rx_hostname has been set/ sockets created)
     * @param p port type control port or receiver port
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void checkDetectorVersionCompatibility(int detPos = -1); //

    /**
     * Check version compatibility with receiver software
     * (if hostname/rx_hostname has been set/ sockets created)
     * @param p port type control port or receiver port
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void checkReceiverVersionCompatibility(int detPos = -1); //

    /**
     * Get ID or version numbers
     * @param mode version type
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns Id or version number of that type
     */
    int64_t getId(idMode mode,
                  int detPos = -1); // not needed anymore (later remove
                                    // this_software_version from enum)

    int getMultiId() const { return multiId; } // part of multi also

    /**
     * Get package version (git branch)
     * @returns package version
     */
    std::string getPackageVersion() const;
    
    /**
     * Get Client Software version
     * @returns client software version
     */
    int64_t getClientSoftwareVersion() const; //

    /**
     * Get Receiver software version
     * @return receiver software version
     */
    int64_t getReceiverSoftwareVersion(int detPos = -1); //

    /**
     * Get Detector Number
     * @returns vector of detector number
     */
    std::vector<int64_t>
    getDetectorNumber(); // renamed to getDetectorSerialNumber
    /**
     * Free shared memory from the command line
     * avoiding creating the constructor classes and mapping
     * @param multiId multi detector Id
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    static void freeSharedMemory(int multiId,
                                 int detPos = -1); // private or not needed

    /**
     * Free shared memory and delete shared memory structure
     * occupied by the sharedMultiSlsDetector structure
     * Clears all the vectors and  bring
     * object back to state before object creation amap
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void freeSharedMemory(int detPos = -1); //

    /**
     * Get user details of shared memory
     * @returns string with user details
     */
    std::string getUserDetails(); // part of multi

    /**
     * Connect to Virtual Detector Servers at local host
     * @param ndet number of detectors
     * @param port starting port number
     */
    void setVirtualDetectorServers(const int numdet, const int port);

    /**
     * Sets the hostname of all sls detectors in shared memory and updates local
     * cache
     * @param name hostname of all the sls detectors
     */
    void setHostname(
        const std::vector<std::string> &name); // cannot set individually

    /**
     * Sets the hostname of all sls detectors in shared memory
     * Connects to them
     * @param name concatenated hostname of all the sls detectors
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setHostname(const char *name, int detPos = -1); // not needed

    /**
     * Gets the hostname of detector at particular position
     * or concatenated hostnames of all the sls detectors
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns concatenated hostnames of all detectors or hostname of specific
     * one
     */
    std::string getHostname(int detPos = -1) const; //

    /**
     * Get Detector type as an enum
     * @returns detector type
     */
    detectorType getDetectorTypeAsEnum() const; //

    /**
     * Get Detector type for a particular sls detector or get the first one
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns detector type of sls detector in position pos, if -1, returns
     * the first det type
     */
    detectorType getDetectorTypeAsEnum(int detPos); //

    /**
     * Concatenates string types of all sls detectors or
     * returns the detector type of the first sls detector
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns detector type of sls detector in position pos, if -1,
     * concatenates
     */
    std::string getDetectorTypeAsString(int detPos = -1); //

    /**
     * Returns the number of detectors in the multidetector structure
     * @returns number of detectors
     */
    size_t size() const; //

    /**
     * Returns the number of detectors in each direction
     */
    slsDetectorDefs::xy getNumberOfDetectors() const; //

    /**
     * Returns the total number of channels of all sls detectors  including gap
     * pixels
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the total number of channels of all sls detectors including gap
     * pixels
     */
    slsDetectorDefs::xy getNumberOfChannels(int detPos = -1) const; //

    /**
     * Must be set before setting hostname
     * Sets maximum number of channels of all sls detectors in each
     * dimension d from shared memory
     * @param c maximum number of channels of all sls detectors
     */
    void setNumberOfChannels(const slsDetectorDefs::xy c); //

    /**
     * Get Quad Type (Only for Eiger Quad detector hardware)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns quad type
     */
    int getQuad(int detPos = -1); //

    /**
     * Set Quad Type (Only for Eiger Quad detector hardware)
     * @param enable true if quad type set, else false
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setQuad(const bool enable, int detPos = -1); //

    /**
     * Set number of rows to read out (Only for Eiger)
     * @param value number of lines
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReadNLines(const int value, int detPos = -1); //

    /**
     * Get number of rows to read out (Only for Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns  number of lines
     */
    int getReadNLines(int detPos = -1); //

    /**
     * Set/Gets TCP Port of the detector
     * @param port_number (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns port number
     */
    int setControlPort(int port_number = -1, int detPos = -1); //

    /**
     * Set/Gets TCP STOP Port of the detector
     * @param port_number (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns port number
     */
    int setStopPort(int port_number = -1, int detPos = -1); //

    /**
     * Set/Gets TCP Port of the receiver
     * @param port_number (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns port number
     */
    int setReceiverPort(int port_number = -1, int detPos = -1); //

    /**
     * Get Receiver port
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns vector of receiver port
     */
    int getReceiverPort(int detPos = -1) const; //

    /**
     * Lock server for this client IP
     * @param p 0 to unlock, 1 to lock
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 1 for locked or 0 for unlocked
     */
    int lockServer(int p = -1, int detPos = -1); //

    /**
     * Exit detector server
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void exitServer(int detPos = -1); //

    /**
     * Execute a command on the detector server
     * @param cmd command
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void execCommand(const std::string &cmd, int detPos); //

    /**
     * Load configuration from a configuration File
     * @param fname configuration file name
     */
    void readConfigurationFile(const std::string &fname); //

    /**
     * Write current configuration to a file
     * @param fname configuration file name
     */
    void writeConfigurationFile(const std::string &fname); //

    /**
     * Get detector settings
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current settings
     */
    detectorSettings getSettings(int detPos = -1); //

    /**
     * Load detector settings from the settings file picked from the
     * trimdir/settingsdir Eiger only stores in shared memory ( a get will
     * overwrite this) For Eiger, one must use threshold
     * @param isettings settings
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current settings
     */
    detectorSettings setSettings(detectorSettings isettings,
                                 int detPos = -1); //

    /**
     * Get threshold energy (Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current threshold value for imod in ev (-1 failed)
     */
    int getThresholdEnergy(int detPos = -1); //

    /**
     * Set threshold energy (Eiger)
     * @param e_eV threshold in eV
     * @param isettings ev. change settings
     * @param tb 1 to include trimbits, 0 to exclude
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current threshold value for imod in ev (-1 failed)
     */
    int setThresholdEnergy(int e_eV, detectorSettings isettings = GET_SETTINGS,
                           int tb = 1, int detPos = -1); //

    /**
     * Returns the detector trimbit/settings directory
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the trimbit/settings directory
     */
    std::string getSettingsDir(int detPos = -1); //

    /**
     * Sets the detector trimbit/settings directory
     * @param s trimbits/settings directory
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the trimbit/settings directory
     */
    std::string setSettingsDir(const std::string &directory,
                               int detPos = -1); //

    /**
     * Loads the modules settings/trimbits reading from a specific file
     * file name extension is automatically generated.
     * @param fname specific settings/trimbits file
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void loadSettingsFile(const std::string &fname, int detPos = -1); //

    /**
     * Saves the modules settings/trimbits to a specific file
     * file name extension is automatically generated.
     * @param fname specific settings/trimbits file
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void saveSettingsFile(const std::string &fname, int detPos = -1); //

    /**
     * Configures in detector the destination for UDP packets
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    //void configureMAC(int detPos = -1); //TODO

    /**
     * Set starting frame number for the next acquisition
     * @param val starting frame number
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setStartingFrameNumber(const uint64_t value, int detPos = -1); //

    /**
     * Get starting frame number for the next acquisition
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns starting frame number
     */
    uint64_t getStartingFrameNumber(int detPos = -1); //

    /**
     * Set/get timer value (not all implemented for all detectors)
     * @param index timer index
     * @param t time in ns or number of...(e.g. frames, probes)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns timer set value in ns or number of...(e.g. frames,
     * probes)
     */
    int64_t setTimer(timerIndex index, int64_t t = -1, int detPos = -1); //

    /**
     * Set/get exposure time
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns exposure time in ns, or s if specified
     */
    double setExposureTime(double t = -1, bool inseconds = false,
                           int detPos = -1); //

    /**
     * Set/get exposure period
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns exposure period in ns, or s if specified
     */
    double setExposurePeriod(double t = -1, bool inseconds = false,
                             int detPos = -1); //

    /**
     * Set/get delay after trigger (Gotthard, Jungfrau(not for this release))
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns delay after trigger in ns, or s if specified
     */
    double setDelayAfterTrigger(double t = -1, bool inseconds = false,
                                int detPos = -1); //

    /**
     * (Advanced users)
     * Set/get sub frame exposure time (Eiger in 32 bit mode)
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns sub frame exposure time in ns, or s if specified
     */
    double setSubFrameExposureTime(double t = -1, bool inseconds = false,
                                   int detPos = -1); //

    /**
     *  (Advanced users)
     * Set/get sub frame dead time (Eiger in 32 bit mode)
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns sub frame dead time in ns, or s if specified
     */
    double setSubFrameExposureDeadTime(double t = -1, bool inseconds = false,
                                       int detPos = -1); //

    /**
     * Set/get number of frames
     * @param t number of frames (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns number of frames
     */
    int64_t setNumberOfFrames(int64_t t = -1, int detPos = -1); //

    /**
     * Set/get number of triggers
     * @param t number of triggers (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns number of triggers
     */
    int64_t setNumberOfTriggers(int64_t t = -1, int detPos = -1); //

    /**
     * Set/get number of additional storage cells  (Jungfrau)
     * @param t number of additional storage cells. Default is 0.  (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns number of additional storage cells
     */
    int64_t setNumberOfStorageCells(int64_t t = -1, int detPos = -1); //

    /**
     * Get measured period between previous two frames (EIGER)
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns sub frame dead time in ns, or s if specified
     */
    double getMeasuredPeriod(bool inseconds = false, int detPos = -1); //

    /**
     * Get sub period between previous two sub frames in 32 bit mode (EIGER)
     * @param t time (-1 gets)
     * @param inseconds true if the value is in s, else ns
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns sub frame dead time in ns, or s if specified
     */
    double getMeasuredSubFramePeriod(bool inseconds = false,
                                     int detPos = -1); //

    /**
     * Set/get timer value left in acquisition (not all implemented for all
     * detectors)
     * @param index timer index
     * @param t time in ns or number of...(e.g. frames, probes)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns timer set value in ns or number of...(e.g. frames,
     * probes)
     */
    int64_t getTimeLeft(timerIndex index, int detPos = -1); //

    /**
     * Set speed
     * @param sp speed type  (clkdivider option for Jungfrau and Eiger,
     * adcphase for Gotthard, others for CTB & Moench)
     * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other
     * values check manual
     * @param mode 0 for shift, 1 for degrees. relevant only for speed type
     * adcphase and dbit phase
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns value of speed set
     */
    int setSpeed(speedVariable index, int value = -1, int mode = 0,
                 int detPos = -1); //

    /**
     * Set/get dynamic range and updates the number of dataBytes
     * (Eiger: If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to
     * 1)
     * @param i dynamic range (-1 get)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current dynamic range
     */
    int setDynamicRange(int dr = -1, int detPos = -1); //

    /**
     * Set/get dacs value
     * @param val value (in V)
     * @param index DAC index
     * @param mV 0 in dac units or 1 in mV
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current DAC value
     */
    int setDAC(int val, dacIndex index, int mV, int detPos = -1); //

    /**
     * Get adc value
     * @param index adc(DAC) index
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current adc value (temperature for eiger and jungfrau in
     * millidegrees)
     */
    int getADC(dacIndex index, int detPos = -1); //

    /**
     * Set/get timing mode
     * @param pol timing mode (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current timing mode
     */
    timingMode setTimingMode(timingMode pol = GET_TIMING_MODE,
                             int detPos = -1); //

    /**
     * Set/get external signal flags (to specify triggerinrising edge etc)
     * (Gotthard, Mythen)
     * @param pol external signal flag (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns current timing mode
     */
    externalSignalFlag
    setExternalSignalFlags(externalSignalFlag pol = GET_EXTERNAL_SIGNAL_FLAG,
                           int detPos = -1); //

    /**
     * Set readout mode (Only for CTB and Moench)
     * @param mode readout mode Options: ANALOG_ONLY, DIGITAL_ONLY, ANALOG_AND_DIGITAL
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReadoutMode(const readoutMode mode, int detPos = -1);

    /**
     * Get readout mode(Only for CTB and Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns readout mode
     */
    readoutMode getReadoutMode(int detPos = -1);

    /**
     * Set Interrupt last sub frame (Only for Eiger)
     * @param enable true if interrupt last subframe set, else false
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setInterruptSubframe(const bool enable, int detPos = -1);

    /**
     * Get Interrupt last sub frame (Only for Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 1 if interrupt last subframe set, else 0, -1 different values
     */
    int getInterruptSubframe(int detPos = -1);

    /**
     * Write in a register. For Advanced users
     * @param addr address of register
     * @param val value to write into register
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns value read after writing
     */
    uint32_t writeRegister(uint32_t addr, uint32_t val, int detPos = -1); //

    /**
     * Read from a register. For Advanced users
     * @param addr address of register
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns value read from register
     */
    uint32_t readRegister(uint32_t addr, int detPos = -1); //

    /**
     * Set bit in a register. For Advanced users
     * @param addr address of register
     * @param n nth bit
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns value read from register
     */
    uint32_t setBit(uint32_t addr, int n, int detPos = -1); //

    /**
     * Clear bit in a register. For Advanced users
     * @param addr address of register
     * @param n nth bit
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns value read from register
     */
    uint32_t clearBit(uint32_t addr, int n, int detPos = -1); //

    /**
     * Validates and sets the receiver.
     * Also updates the receiver with all the shared memory parameters
     * significant for the receiver Also configures the detector to the receiver
     * as UDP destination
     * @param receiver receiver hostname or IP address
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver IP address from shared memory
     */
    std::string setReceiverHostname(const std::string &receiver,
                                    int detPos = -1); //

    /**
     * Returns the receiver IP address
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver IP address
     */
    std::string getReceiverHostname(int detPos = -1) const; //

    /**
     * ets the number of UDP interfaces to stream data from detector (Jungfrau
     * only)
     * @param n number of interfaces. Options 1 or 2.
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setNumberofUDPInterfaces(int n, int detPos = -1); //

    /**
     * Returns the number of UDP interfaces to stream data from detector
     * (Jungfrau only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the number of interfaces
     */
    int getNumberofUDPInterfaces(int detPos = -1) ; //

    /**
     * Selects the UDP interfaces to stream data from detector. Effective only
     * when number of interfaces is 1. (Jungfrau only)
     * @param n selected interface. Options 1 or 2.
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void selectUDPInterface(int n, int detPos = -1);

    /**
     * Returns the UDP interfaces to stream data from detector. Effective only
     * when number of interfaces is 1. (Jungfrau only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the interface selected
     */
    int getSelectedUDPInterface(int detPos = -1) ; //

    /**
     * (advanced users)
     * Set/Get client streaming in  ZMQ port and restarts client sockets
     * @param i sets, -1 gets
     * If detPos is -1(multi module), port calculated (increments) for all the
     * individual detectors using i
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setClientDataStreamingInPort(int i = -1, int detPos = -1);

    /**
     * Returns the client zmq port
     * If detPos is -1(multi module), port returns client streaming port of
     * first module
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the client zmq port
     */
    int getClientStreamingPort(int detPos = -1); //

    /**
     * (advanced users)
     * Set/Get receiver streaming out ZMQ port and restarts receiver sockets
     * @param i sets, -1 gets
     * If detPos is -1(multi module), port calculated (increments) for all the
     * individual detectors using i
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReceiverDataStreamingOutPort(int i = -1, int detPos = -1); //

    /**
     * Returns the receiver zmq port
     * If detPos is -1(multi module), port returns receiver streaming port of
     * first module
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver zmq port
     */
    int getReceiverStreamingPort(int detPos = -1); //

     /**
     * Sets the transmission delay for left, right or entire frame
     * (Eiger, Jungfrau(only entire frame))
     * @param index type of delay
     * @param delay delay
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns transmission delay
     */
    int setDetectorNetworkParameter(networkParameter index, int delay,
                                    int detPos = -1); // maybe not needed in API

    /**
     * Sets the additional json header
     * @param jsonheader additional json header
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns additional json header, default is empty
     */
    std::string setAdditionalJsonHeader(const std::string &jsonheader,
                                        int detPos = -1); //

    /**
     * Returns the additional json header
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the additional json header, default is empty
     */
    std::string getAdditionalJsonHeader(int detPos = -1); //

    /**
     * Sets the value for the additional json header parameter if found, else
     * append it
     * @param key additional json header parameter
     * @param value additional json header parameter value (cannot be empty)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the additional json header parameter value,
     * empty if no parameter found in additional json header
     */
    std::string setAdditionalJsonParameter(const std::string &key,
                                           const std::string &value,
                                           int detPos = -1); //

    /**
     * Returns the additional json header parameter value
     * @param key additional json header parameter
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the additional json header parameter value,
     * empty if no parameter found in additional json header
     */
    std::string getAdditionalJsonParameter(const std::string &key,
                                           int detPos = -1); //

    /**
     * Sets the detector minimum/maximum energy threshold in processor (for
     * Moench only)
     * @param index 0 for emin, antyhing else for emax
     * @param v value to set (-1 gets)
     * @returns detector minimum/maximum energy threshold (-1 for not found or
     * error in computing json parameter value)
     */
    int setDetectorMinMaxEnergyThreshold(const int index, int value,
                                         int detPos = -1); //

    /**
     * Sets the frame mode in processor (Moench only)
     * @param value frameModeType (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns frame mode (-1 for not found or error in computing json
     * parameter value)
     */
    int setFrameMode(frameModeType value, int detPos = -1);

    /**
     * Sets the detector mode in processor (Moench only)
     * @param value detectorModetype (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns detector mode (-1 for not found or error in computing json
     * parameter value)
     */
    int setDetectorMode(detectorModeType value, int detPos = -1);

    /**
     * Sets the receiver UDP socket buffer size
     * @param udpsockbufsize additional json header
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns receiver udp socket buffer size
     */
    int64_t setReceiverUDPSocketBufferSize(int64_t udpsockbufsize = -1,
                                           int detPos = -1); //

    /**
     * Returns the receiver UDP socket buffer size
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver UDP socket buffer size
     */
    int64_t getReceiverUDPSocketBufferSize(int detPos = -1); //

    /**
     * Returns the receiver real UDP socket buffer size
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver real UDP socket buffer size
     */
    int64_t getReceiverRealUDPSocketBufferSize(int detPos = -1); //

    /** (users only)
     * Set 10GbE Flow Control (Eiger)
     * @param enable 1 to set, 0 to unset, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 10GbE flow Control
     */
    int setFlowControl10G(int enable = -1, int detPos = -1); //

    /**
     * Execute a digital test (Gotthard, Jungfrau, CTB)
     * @param mode testmode type
     * @param value 1 to set or 0 to clear the image test bit (Gotthard)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns result of test
     */
    int digitalTest(digitalTestMode mode, int ival = -1, int detPos = -1);

    /**
     * Set/get counter bit in detector (Eiger)
     * @param i is -1 to get, 0 to reset and any other value to set the counter
     * bit
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the counter bit in detector
     */
    int setCounterBit(int i = -1, int detPos = -1); //

    /**
     * Clear ROI (Gotthard)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void clearROI(int detPos = -1);

    /**
     * Set ROI (Gotthard)
     * At the moment only one set allowed per module
     * Only allowed to set one ROI per module
     * @param arg  roi
     * @param detPos specific detector position
     */
    void setROI(slsDetectorDefs::ROI arg, int detPos = -1);

    /**
     * Get ROI  (Gotthard)
     * Only allowed to set one ROI per module
     * @param detPos specific detector position
     * @returns roi
     */
    slsDetectorDefs::ROI getROI(int detPos) const;

    /**
     * Set ADC Enable Mask (CTB, Moench)
     * @param mask ADC Enable mask
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setADCEnableMask(uint32_t mask, int detPos = -1); //

    /**
     * Get ADC Enable Mask (CTB, Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns ADC Enable mask
     */
    uint32_t getADCEnableMask(int detPos = -1); //

    /**
     * Set ADC invert register (CTB, Moench)
     * @param value ADC invert value
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setADCInvert(uint32_t value, int detPos = -1); //

    /**
     * Get ADC invert register (CTB, Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns ADC invert value
     */
    uint32_t getADCInvert(int detPos = -1); //

    /**
     * Set external sampling source (CTB only)
     * @param value external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setExternalSamplingSource(int value, int detPos = -1); //

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling source
     */
    int getExternalSamplingSource(int detPos = -1); //

    /**
     * Set external sampling enable (CTB only)
     * @param value external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setExternalSampling(bool value, int detPos = -1); //

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling enable
     */
    int getExternalSampling(int detPos = -1); //

    /**
     * Set external sampling enable (CTB only)
     * @param list external sampling source (Option: 0-63)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReceiverDbitList(std::vector<int> list, int detPos = -1); //

    /**
     * Get external sampling source (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns external sampling enable
     */
    std::vector<int> getReceiverDbitList(int detPos = -1); //

    /**
     * Set digital data offset in bytes (CTB only)
     * @param value digital data offset in bytes
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setReceiverDbitOffset(int value, int detPos = -1); //

    /**
     * Get digital data offset in bytes (CTB only)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns digital data offset in bytes
     */
    int getReceiverDbitOffset(int detPos = -1); //

    /**
     * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert
     * users
     * @param addr address of adc register
     * @param val value
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void writeAdcRegister(uint32_t addr, uint32_t val, int detPos = -1); //

    /**
     * Activates/Deactivates the detector (Eiger only)
     * @param enable active (1) or inactive (0), -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 0 (inactive) or 1 (active)for activate mode
     */
    int activate(int const enable = -1, int detPos = -1); //

    /**
     * Set deactivated Receiver padding mode (Eiger only)
     * @param padding padding option for deactivated receiver. Can be 1
     * (padding), 0 (no padding), -1 (gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 1 (padding), 0 (no padding), -1 (inconsistent values) for
     * padding option
     */
    int setDeactivatedRxrPaddingMode(int padding = -1, int detPos = -1);

    /**
     * Returns the enable if data will be flipped across x axis (Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 1 for flipped, else 0
     */
    int getFlippedDataX(int detPos = -1); //

    /**
     * Sets the enable which determines if
     * data will be flipped across x axis (Eiger)
     * @param value 0 or 1 to reset/set or -1 to get value
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns enable flipped data across x or y axis
     */
    int setFlippedDataX(int value = -1, int detPos = -1); //

    /**
     * Sets all the trimbits to a particular value (Eiger)
     * @param val trimbit value
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns OK or FAIL
     */
    int setAllTrimbits(int val, int detPos = -1); //

    /**
     * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
     * 4 bit mode gap pixels only in gui call back
     * @param val 1 sets, 0 unsets, -1 gets
     * @returns gap pixel enable or -1 for error
     */
    int enableGapPixels(int val = -1, int detPos = -1); //

    void setGapPixelsEnable(bool enable, sls::Positions pos = {});
    /**
     * Sets the number of trim energies and their value  (Eiger)
     *
     * @param nen number of energies
     * @param en array of energies
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns number of trim energies
     */
    int setTrimEn(std::vector<int> energies, int detPos = -1); //

    /**
     * Returns the number of trim energies and their value  (Eiger)
     *
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns vector of trim energies
     */
    std::vector<int> getTrimEn(int detPos = -1); //

    /**
     * Pulse Pixel (Eiger)
     * @param n is number of times to pulse
     * @param x is x coordinate
     * @param y is y coordinate
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void pulsePixel(int n = 0, int x = 0, int y = 0, int detPos = -1); //

    /**
     * Pulse Pixel and move by a relative value (Eiger)
     * @param n is number of times to pulse
     * @param x is relative x value
     * @param y is relative y value
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void pulsePixelNMove(int n = 0, int x = 0, int y = 0, int detPos = -1); //

    /**
     * Pulse Chip (Eiger)
     * @param n is number of times to pulse
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void pulseChip(int n = 0, int detPos = -1); //

    /**
     * Set/gets threshold temperature (Jungfrau)
     * @param val value in millidegrees, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns threshold temperature in millidegrees
     */
    int setThresholdTemperature(int val = -1, int detPos = -1); //

    /**
     * Enables/disables temperature control (Jungfrau)
     * @param val value, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns temperature control enable
     */
    int setTemperatureControl(int val = -1, int detPos = -1); //

    /**
     * Resets/ gets over-temperature event (Jungfrau)
     * @param val value, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns over-temperature event
     */
    int setTemperatureEvent(int val = -1, int detPos = -1); //

    /**
     * Set storage cell that stores first acquisition of the series (Jungfrau)
     * @param value storage cell index. Value can be 0 to 15. (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the storage cell that stores the first acquisition of the series
     */
    int setStoragecellStart(int pos = -1, int detPos = -1); //

    /**
     * Programs FPGA with pof file (Jungfrau, CTB, Moench)
     * @param fname file name
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void programFPGA(const std::string &fname, int detPos = -1); //

    /**
     * Resets FPGA (Jungfrau, CTB, Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void resetFPGA(int detPos = -1); //

    /**
     * Copies detector server from tftp and changes respawn server (Not Eiger)
     * @param fname name of detector server binary
     * @param hostname name of pc to tftp from
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname, int detPos = -1); //

    /**
     * Reboot detector controller (Not Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void rebootController(int detPos = -1); //

    /**
     * Updates the firmware, detector server and then reboots detector
     * controller blackfin. (Not Eiger)
     * @param sname name of detector server binary
     * @param hostname name of pc to tftp from
     * @param fname programming file name
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void update(const std::string &sname, const std::string &hostname,
                const std::string &fname, int detPos = -1); //

    /**
     * Power on/off Chip (Jungfrau)
     * @param ival on is 1, off is 0, -1 to get
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns OK or FAIL
     */
    int powerChip(int ival = -1, int detPos = -1); //

    /**
     * Automatic comparator disable (Jungfrau)
     * @param ival on is 1, off is 0, -1 to get
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns OK or FAIL
     */
    int setAutoComparatorDisableMode(int ival = -1, int detPos = -1); //

    /**
     * Set Default Rate correction from trimbit file ( Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setDefaultRateCorrection(int detPos = -1); //

    /**
     * Set Rate correction ( Eiger)
     * @param t dead time in ns - if 0 disable correction,
     * if >0 set dead time to t, cannot be < 0
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setRateCorrection(int64_t t = 0, int detPos = -1); //

    /**
     * Get rate correction ( Eiger)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns 0 if rate correction disabled, > 0 otherwise (ns)
     */
    int64_t getRateCorrection(int detPos = -1); //

    /**
     * Prints receiver configuration
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns receiver configuration
     */
    std::string printReceiverConfiguration(int detPos = -1); //

    /**
     * Get receiver online status
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns use receiver flag
     */
    bool getUseReceiverFlag(int detPos = -1); //

    /**
     * Locks/Unlocks the connection to the receiver
     * @param lock sets (1), usets (0), gets (-1) the lock
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns lock status of the receiver
     */
    int lockReceiver(int lock = -1, int detPos = -1); //

    /**
     * Turns off the receiver server!
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void exitReceiver(int detPos = -1);

    /**
     * Executes a system command on the receiver server
     * e.g. mount an nfs disk, reboot and returns answer etc.
     * @param cmd command to be executed
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void execReceiverCommand(const std::string &cmd, int detPos = -1);

    /**
     * Returns output file directory
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns output file directory
     */
    std::string getFilePath(int detPos = -1); //

    /**
     * Sets up the file directory
     * @param detPos -1 for all detectors in  list or specific detector position
     * @param s file directory
     * @returns file dir
     */
    std::string setFilePath(const std::string &path, int detPos = -1); //

    /**
     * Returns file name prefix
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file name prefix
     */
    std::string getFileName(int detPos = -1); //

    /**
     * Sets up the file name prefix
     * @param detPos -1 for all detectors in  list or specific detector position
     * @param s file name prefix
     * @returns file name prefix
     */
    std::string setFileName(const std::string &fname, int detPos = -1); //

    /**
     * Sets the max frames per file in receiver
     * @param f max frames per file
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns max frames per file in receiver
     */
    int setFramesPerFile(int f = -1, int detPos = -1); //

    /**
     * Gets the max frames per file in receiver
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns max frames per file in receiver
     */
    int getFramesPerFile(int detPos = -1) const; //

    /**
     * Sets the frames discard policy in receiver
     * @param f frames discard policy
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns frames discard policy set in receiver
     */
    frameDiscardPolicy setReceiverFramesDiscardPolicy(
        frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY, int detPos = -1); //

    /**
     * Sets the partial frames padding enable in receiver
     * @param f partial frames padding enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns partial frames padding enable in receiver
     */
    int setPartialFramesPadding(bool padding, int detPos = -1); //

    int getPartialFramesPadding(int detPos = -1) const; //

    /**
     * Returns file format
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file name
     */
    fileFormat getFileFormat(int detPos = -1); //

    /**
     * Sets up the file format
     * @param f file format
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file format
     */
    fileFormat setFileFormat(fileFormat f, int detPos = -1); //

    /**
     * Sets up the file index
     * @param i file index
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file index
     */
    int64_t setFileIndex(int64_t i, int detPos = -1); //

    /**
     * Get File index
     * @param  detPos -1 for all detectors in  list or specific detector
     * position
     * @returns file index
     */
    int64_t getFileIndex(int detPos = -1) const; //

    /**
     * Gets the number of frames caught by receiver
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns number of frames caught by receiver
     */
    int getFramesCaughtByReceiver(int detPos = -1); //

    /**
     * Gets the current frame index of receiver
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns average of all current frame index of receiver
     */
    uint64_t getReceiverCurrentFrameIndex(int detPos = -1); //

    /**
     * Resets framescaught in receiver
     * Use this when using startAcquisition instead of acquire
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void resetFramesCaught(int detPos = -1); //

    /**
     * Sets/Gets receiver file write enable
     * @param value 1 or 0 to set/reset file write enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file write enable
     */
    int setFileWrite(bool value, int detPos = -1); //

    /**
     * Gets file write enable
     * @returns file write enable
     */
    int getFileWrite(int detPos = -1) const; //

    /**
     * Sets/Gets receiver master file write enable
     * @param value 1 or 0 to set/reset master file write enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns master file write enable
     */
    int setMasterFileWrite(bool value, int detPos = -1); //

    /**
     * Gets master file write enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns master file write enable
     */
    int getMasterFileWrite(int detPos = -1) const; //

    /**
     * Sets/Gets file overwrite enable
     * @param enable 1 or 0 to set/reset file overwrite enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file overwrite enable
     */
    int setFileOverWrite(bool enable, int detPos = -1); //

    /**
     * Gets file over write enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns file over write enable
     */
    int getFileOverWrite(int detPos = -1) const; //

    /**
     * (previously setReadReceiverFrequency)
     * Sets the receiver streaming frequency
     * @param freq nth frame streamed out, if 0, streamed out at a timer of 200
     * ms
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns receiver streaming frequency
     */
    int setReceiverStreamingFrequency(int freq = -1, int detPos = -1);

    /**
     * (previously setReceiverReadTimer)
     * Sets the receiver streaming timer
     * If receiver streaming frequency is 0, then this timer between each
     * data stream is set. Default is 200 ms.
     * @param time_in_ms timer between frames
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns receiver streaming timer in ms
     */
    int setReceiverStreamingTimer(int time_in_ms = 200, int detPos = -1); //

    /**
     * Enable data streaming to client
     * @param enable 0 to disable, 1 to enable, -1 to get the value
     * @returns data streaming to client enable
     */
    bool enableDataStreamingToClient(int enable = -1);

    /**
     * Enable or disable streaming data from receiver to client
     * @param enable 0 to disable 1 to enable -1 to only get the value
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns data streaming from receiver enable
     */
    int enableDataStreamingFromReceiver(int enable = -1, int detPos = -1);

    /**
     * Enable/disable or 10Gbe
     * @param i is -1 to get, 0 to disable and 1 to enable
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns if 10Gbe is enabled
     */
    int enableTenGigabitEthernet(int i = -1, int detPos = -1); //

    /**
     * Set/get receiver fifo depth
     * @param i is -1 to get, any other value to set the fifo deph
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver fifo depth
     */
    int setReceiverFifoDepth(int i = -1, int detPos = -1); //

    /**
     * Set/get receiver silent mode
     * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the receiver silent mode enable
     */
    int setReceiverSilentMode(int i = -1, int detPos = -1); //

    /**
     * Opens pattern file and sends pattern (CTB/ Moench)
     * @param fname pattern file to open
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setPattern(const std::string &fname, int detPos = -1); //

    /**
     * Executes and saves pattern to file (CTB/ Moench)
     * @param fname pattern file to save to
     */
    void savePattern(const std::string &fname); // 

    /**
     * Sets pattern IO control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns actual value
     */
    uint64_t setPatternIOControl(uint64_t word = -1, int detPos = -1); //

    /**
     * Sets pattern clock control (CTB/ Moench)
     * @param word 64bit word to be written, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns actual value
     */
    uint64_t setPatternClockControl(uint64_t word = -1, int detPos = -1); //

    /**
     * Writes a pattern word (CTB/ Moench)
     * @param addr address of the word
     * @param word 64bit word to be written, -1  reads the addr (same as
     * executing the pattern)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns actual value
     */
    uint64_t setPatternWord(int addr, uint64_t word, int detPos = -1); //

    /**
     * Sets the wait address (CTB/ Moench)
     * @param level  0,1,2, wait level
     * @param addr wait address, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns actual value
     */
    int setPatternWaitAddr(int level, int addr = -1, int detPos = -1); //

    /**
     * Sets the wait time (CTB/ Moench)
     * @param level  0,1,2, wait level
     * @param t wait time, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns actual value
     */
    uint64_t setPatternWaitTime(int level, uint64_t t = -1, int detPos = -1); //

    /**
     * Sets the mask applied to every pattern (CTB/ Moench)
     * @param mask mask to be applied
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setPatternMask(uint64_t mask, int detPos = -1); //

    /**
     * Gets the mask applied to every pattern (CTB/ Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns mask set
     */
    uint64_t getPatternMask(int detPos = -1); //

    /**
     * Selects the bits that the mask will be applied to for every pattern (CTB/
     * Moench)
     * @param mask mask to select bits
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setPatternBitMask(uint64_t mask, int detPos = -1); //

    /**
     * Gets the bits that the mask will be applied to for every pattern (CTB/
     * Moench)
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns mask  of bits selected
     */
    uint64_t getPatternBitMask(int detPos = -1); //

    /**
     * Set LED Enable (Moench, CTB only)
     * @param enable 1 to switch on, 0 to switch off, -1 gets
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns LED enable
     */
    int setLEDEnable(int enable = -1, int detPos = -1); //

    /**
     * Set Digital IO Delay (Moench, CTB only)
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     * @param detPos -1 for all detectors in  list or specific detector position
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, int detPos = -1); //

    /**
     * Loads the detector setup from file
     * @param fname file to read from
     */
    void loadParameters(const std::string &fname);

    /**
     * register callback for accessing acquisition final data
     * @param func function to be called at the end of the acquisition.
     * gets detector status and progress index as arguments
     * @param pArg argument
     */
    void registerAcquisitionFinishedCallback(void (*func)(double, int, void *),
                                             void *pArg);

    /**
     * register calbback for accessing detector final data,
     * also enables data streaming in client and receiver
     * @param userCallback function for plotting/analyzing the data.
     * Its arguments are
     * the data structure d and the frame number f,
     * s is for subframe number for eiger for 32 bit mode
     * @param pArg argument
     */
    void registerDataCallback(void (*userCallback)(detectorData *, uint64_t,
                                                   uint32_t, void *),
                              void *pArg);

    /**
     * Performs a complete acquisition
     * resets frames caught in receiver, starts receiver, starts detector,
     * blocks till detector finished acquisition, stop receiver, increments file
     * index, loops for measurements, calls required call backs.
     * @returns OK or FAIL depending on if it already started
     */
    int acquire(); //

    /**
     * Combines data from all readouts and gives it to the gui
     * or just gives progress of acquisition by polling receivers
     */
    void processData();

    /**
     * Convert raw file
     * @param fname name of pof file
     * @param fpgasrc pointer in memory to read pof to
     * @returns file size
     */
    std::vector<char> readPofFile(const std::string &fname);

  private:
    /**
     * Creates/open shared memory, initializes detector structure and members
     * Called by constructor/ set hostname / read config file
     * @param verify true to verify if shared memory version matches existing
     * one
     * @param update true to update last user pid, date etc
     */
    void setupMultiDetector(bool verify = true, bool update = true);

    /**
     * Initialize (open/create) shared memory for the sharedMultiDetector
     * structure
     * @param verify true to verify if shm size matches existing one
     * @param update true to update last user pid, date etc
     * @returns true if shared memory was created in this call, else false
     */
    void initSharedMemory(bool verify = true);

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
     * Check if acquiring flag is set, set error if set
     * @returns FAIL if not ready, OK if ready
     */
    bool isAcquireReady();

    /**
     * Execute in command line and return result
     * @param cmd command
     * @returns result
     */
    std::string exec(const char *cmd);

    /**
     * Add sls detector
     * @param s hostname of the single detector
     */
    void addSlsDetector(const std::string &hostname);

    /**
     * Updates the channel size in X and Y dimension for all the sls
     * detectors
     */
    void updateDetectorSize();

    /**
     * increments file index
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns the file index
     */
    int64_t incrementFileIndex(int detPos = -1);

    /**
     * add gap pixels to the image (only for Eiger in 4 bit mode)
     * @param image pointer to image without gap pixels
     * @param gpImage poiner to image with gap pixels, if NULL, allocated
     * inside function
     * quadEnable quad enabled
     * @returns number of data bytes of image with gap pixels
     */
    int processImageWithGapPixels(char *image, char *&gpImage, bool quadEnable);

    /**
     * Create Receiving Data Sockets
     * @param destroy is true to destroy all the sockets
     * @returns OK or FAIL
     */
    int createReceivingDataSockets(const bool destroy = false);

    /**
     * Reads frames from receiver through a constant socket
     * Called during acquire() when call back registered or when using gui
     */
    void readFrameFromReceiver();

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
    void setCurrentProgress(int i = 0);

    /**
     * Start data processing thread
     */
    void startProcessingThread();

    /**
     * Check if processing thread is ready to join main thread
     * @returns true if ready, else false
     */
    bool getJoinThreadFlag() const;

    /**
     * Main thread sets if the processing thread should join it
     * @param v true if it should join, else false
     */
    void setJoinThreadFlag(bool value);

    /**
     * Listen to key event to stop acquiring
     * when using acquire command
     */
    int kbhit();

    /**
     * Convert a double holding time in seconds to an int64_t with nano seconds
     * Used for conversion when sending time to detector
     * @param t time in seconds
     * @returns time in nano seconds
     */
    int64_t secondsToNanoSeconds(double t);

    /** Multi detector Id */
    const int multiId{0};

    /** Shared Memory object */
    sls::SharedMemory<sharedMultiSlsDetector> multi_shm{0, -1};

    /** pointers to the slsDetector structures */
    std::vector<std::unique_ptr<slsDetector>> detectors;

    /** data streaming (down stream) enabled in client (zmq sckets created) */
    bool client_downstream{false};

    /** ZMQ Socket - Receiver to Client */
    std::vector<std::unique_ptr<ZmqSocket>> zmqSocket;

    /** semaphore to let postprocessing thread continue for next
     * scan/measurement */
    sem_t sem_newRTAcquisition;

    /** semaphore to let main thread know it got all the dummy packets (also
     * from ext. process) */
    sem_t sem_endRTAcquisition;

    /** Total number of frames/images for next acquisition */
    int totalProgress{0};

    /** Current progress or frames/images processed in current acquisition */
    int progressIndex{0};

    /** mutex to synchronize main and data processing threads */
    mutable std::mutex mp;

    /** sets when the acquisition is finished */
    bool jointhread{false};

    /** the data processing thread */
    // pthread_t dataProcessingThread;
    std::thread dataProcessingThread;

    /** detector data packed for the gui */
    detectorData *thisData{nullptr};

    void (*acquisition_finished)(double, int, void *){nullptr};
    void *acqFinished_p{nullptr};

    void (*dataReady)(detectorData *, uint64_t, uint32_t, void *){nullptr};
    void *pCallbackArg{nullptr};
};

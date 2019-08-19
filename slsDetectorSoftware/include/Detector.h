#pragma once
#include "Result.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <memory>
#include <vector>

class multiSlsDetector;
namespace sls {
using ns = std::chrono::nanoseconds;
class MacAddr;
class IpAddr;

/**
 * \class Detector
 */
class Detector {
    std::unique_ptr<multiSlsDetector> pimpl;

  public:
    /**
     * @param shm_id detector shared memory id
     * Default value is 0. Can be set to more values for 
     * multiple detectors.It is important only if you 
     * are controlling multiple detectors from the same pc.
     */
    Detector(int shm_id = 0);
    ~Detector();

    /**************************************************
     *                                                *
     *    CONFIGURATION                               *
     *                                                *
     * ************************************************/

    /* Free the shared memory of this detector and all modules
     * belonging to it.*/
    void freeSharedMemory();

    void loadConfig(const std::string &fname);

    Result<std::string> getHostname(Positions pos = {}) const;

    /* Frees shared memory, adds detectors to the list
     * and updates local detector cache */
    void setHostname(const std::vector<std::string> &hostname);

    /** Gets shared memory ID */
    int getShmId() const;

    Result<int64_t> getFirmwareVersion(Positions pos = {}) const;

    Result<int64_t> getDetectorServerVersion(Positions pos = {}) const;

    Result<int64_t> getSerialNumber(Positions pos = {}) const;

    int64_t getClientVersion() const;

    Result<int64_t> getReceiverVersion(Positions pos = {}) const;

    Result<defs::detectorType> getDetectorType(Positions pos = {}) const;

    /** Gets the total number of detectors */
    int size() const;

    defs::xy getModuleGeometry() const;

    Result<defs::xy> getModuleSize(Positions pos = {}) const;

    /** Gets the actual full detector size. It is the same even if ROI changes */
    defs::xy getDetectorSize() const;

    /**
     * Sets the detector size in both dimensions.
     * This value is used to calculate row and column positions for each module.
     */
    void setDetectorSize(const defs::xy value);

    /** [Jungfrau][Gotthard] */
    Result<defs::detectorSettings> getSettings(Positions pos = {}) const;

    /** [Jungfrau][Gotthard] */
    void setSettings(defs::detectorSettings value, Positions pos = {});



    /**************************************************
     *                                                *
     *    Acquisition Parameters                      *
     *                                                *
     * ************************************************/

    Result<int64_t> getNumberOfFrames() const;

    void setNumberOfFrames(int64_t value);

    Result<int64_t> getNumberOfTriggers() const;

    void setNumberOfTriggers(int64_t value);

    Result<ns> getExptime(Positions pos = {}) const;

    void setExptime(ns t, Positions pos = {});

    Result<ns> getPeriod(Positions pos = {}) const;

    void setPeriod(ns t, Positions pos = {});

    /** [Gotthard][Jungfrau] */
    Result<ns> getDelayAfterTrigger(Positions pos = {}) const;

    /** [Gotthard][Jungfrau] */
    void setDelayAfterTrigger(ns value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfFramesLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfTriggersLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    Result<ns> getDelayAfterTriggerLeft(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    Result<int> getSpeed(Positions pos = {}) const;

    /** [Eiger][Jungfrau] // TODO: create enum for speed
     * Options: (0 full speed, 1 half speed, 2 quarter speed)
     */
    void setSpeed(int value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int> getADCPhase(bool inDeg, Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    void setADCPhase(int value, bool inDeg, Positions pos = {});

    /** [Jungfrau][CTB] */
    Result<int> getMaxADCPhaseShift(Positions pos = {}) const;

    Result<int> getHighVoltage(Positions pos = {}) const;

    /**
     * [Gotthard Options: 0, 90, 110, 120, 150, 180, 200]
     * [Jungfrau, CTB Options: 0, 60 - 200]
     * [Eiger Options: 0 - 200]
     */
    void setHighVoltage(int value, Positions pos = {});

    /**
     * (Degrees)
     * [Gotthard] Options: TEMPERATURE_ADC, TEMPERATURE_FPGA
     * [Jungfrau] Options: TEMPERATURE_ADC, TEMPERATURE_FPGA
     * [Eiger] Options: TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE,
     * TEMPERATURE_DCDC, TEMPERATURE_SODL, TEMPERATURE_SODR, TEMPERATURE_FPGA2,
     * TEMPERATURE_FPGA3
     * [CTB] Options: SLOW_ADC_TEMP
     */
    Result<int> getTemperature(defs::dacIndex index, Positions pos = {}) const;

    Result<int> getDAC(defs::dacIndex index, bool mV, Positions pos = {}) const;

    void setDAC(int value, defs::dacIndex index, bool mV, Positions pos = {});

    Result<defs::timingMode> getTimingMode(Positions pos = {}) const;

    /**
     * [Gotthard, Jungfrau, CTB Options: AUTO_TIMING, TRIGGER_EXPOSURE]
     * [Eiger Options: AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER]
     */
    void setTimingMode(defs::timingMode value,
                       Positions pos = {});


    /**************************************************
     *                                                *
     *    ACQUISITION                                 *
     *                                                *
     * ************************************************/
    /**
     * Blocking call, starts the receiver and detector.
     * Increments file index if file write enabled.
     * Acquired the number of frames set.
     */
    void acquire();

    /** Non blocking 
     * Starts the reciever (if enabled) and then the detector
     * You have to check detector status until it is idle before you call stopACquisition
     * 
    */
    void startAcquisition();

    /**
     * Stops detector acquisition and then receiver (if enabled)
     * If no receiver enabled, you can skip this for normal acquisition (no abort)
     */
    void stopAcquisition();//TODO: cannot do this. for acquire, to stop acquisition, must not also do stop receiver(mutex)
    // TODO: stopAcquire??

    /**
     * Clears the acquiring flag. This has to be done manually
     * after an acquisition was aborted.
     */
    void clearAcquiringFlag();

    Result<defs::runStatus> getDetectorStatus(Positions pos = {}) const;

    Result<defs::runStatus> getReceiverStatus(Positions pos = {}) const;

    Result<int> getFramesCaught(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    void setStartingFrameNumber(uint64_t value, Positions pos);

    /** [Eiger] Sends an internal software trigger to the detector */
    void sendSoftwareTrigger(Positions pos = {});

    //TODO: remove resetframescaught in receiver


    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/
    
    /** Configures the destination for UDP packets in the detector 
     * Needed only if you use a custom receiver (not slsReceiver)
     * as it is already included in setReceiverHostname.
    */
    void configureMAC(Positions pos = {});//TODO: find a reasonable name

    /** [Jungfrau] */
    Result<int> getNumberofUDPInterfaces(Positions pos = {}) const;

    /** [Jungfrau] Also restarts client and receiver sockets */
    void setNumberofUDPInterfaces(int n, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getSelectedUDPInterface(Positions pos = {}) const;

    /**
     * [Jungfrau:
     * Effective only when number of interfaces is 1.
     * Options: 0 (outer, default), 1(inner)] //TODO: enum?
     */
    void selectUDPInterface(int interface, Positions pos = {});

    Result<IpAddr> getSourceUDPIP(Positions pos = {}) const;

    /* For Eiger 1G, the detector will replace with its own DHCP IP
     * 10G Eiger and other detectors, the source UDP IP must be in the 
     * same subnet of the destination UDP IP
     */
    void setSourceUDPIP(const std::string &ip, Positions pos = {});

   /** [Jungfrau] bottom half */
    Result<IpAddr> getSourceUDPIP2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setSourceUDPIP2(const std::string &ip, Positions pos = {});

    Result<MacAddr> getSourceUDPMAC(Positions pos = {}) const;

    /* For Eiger 1G, the detector will replace with its own DHCP MAC
     * For Eiger 10G, the detector will replace with its own DHCP MAC + 1
     * Others can be anything (beware of certain bits)
     */
    
    void setSourceUDPMAC(const std::string &mac, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<MacAddr> getSourceUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setSourceUDPMAC2(const std::string &mac, Positions pos = {});

    Result<IpAddr> getDestinationUDPIP(Positions pos = {}) const;

    /** IP of the interface in receiver that the detector sends data to */
    void setDestinationUDPIP(const std::string &ip, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<IpAddr> getDestinationUDPIP2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setDestinationUDPIP2(const std::string &ip, Positions pos = {});
    
    Result<MacAddr> getDestinationUDPMAC(Positions pos = {}) const;

    /** MAC of the interface in receiver that the detector sends data to 
     * Only needed if you use a custom receiver (not slsReceiver)
     * Must be followed by configuremac.
    */
    void setDestinationUDPMAC(const std::string &mac, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<MacAddr> getDestinationUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setDestinationUDPMAC2(const std::string &mac, Positions pos = {});

    Result<int> getDestinationUDPPort(Positions pos = {}) const;

    /** module_id is -1 for all detectors, ports for each module is calculated (increments) */
    //TODO if Parallel takes a vector, can send multiple vaues to set in slsdetector.cp
    void setDestinationUDPPort(int port, int module_id = -1);

    /** [Eiger right port][Jungfrau bottom half] */
    Result<int> getDestinationUDPPort2(Positions pos = {}) const;

    /** [Eiger right port][Jungfrau bottom half]
     * module_id is -1 for all detectors, ports for each module is calculated (increments)
     */
    void setDestinationUDPPort2(int port, int module_id = -1);

    Result<std::string> printRxConfiguration(Positions pos = {}) const;

    /** [Eiger][CTB] */
    Result<bool> getTenGiga(Positions pos = {}) const;

    /** [Eiger][CTB] */
    void setTenGiga(bool enable, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<bool> getTenGigaGFlowControl(Positions pos = {}) const;

    /** [Eiger, Jungfrau] */
    void setTenGigaGFlowControl(bool enable, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<int> getTransmissionDelayFrame(Positions pos = {}) const;

    /**
     * [Jungfrau]: Sets the transmission delay of the first UDP packet being
     * streamed out of the module. Options: 0 - 31, each value represenets 1 ms 
     * [Eiger]: Sets the transmission delay of entire frame streamed out for both
     * left and right UDP ports. Options: //TODO possible values
     */
    void setTransmissionDelayFrame(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayLeft(Positions pos = {}) const;

    /**
     * [Eiger]
     * Sets the transmission delay of first packet streamed out of the left UDP
     * port
     */
    void setTransmissionDelayLeft(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayRight(Positions pos = {}) const;

    /**
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the right UDP
     * port
     */
    void setTransmissionDelayRight(int value, Positions pos = {});


    /**************************************************
     *                                                *
     *    RECEIVER CONFIG                             *
     *                                                *
     * ************************************************/

    /** true when slsReceiver is used */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    Result<std::string> getRxHostname(Positions pos = {}) const;

    /**
     * Validates and sets the receiver.
     * Updates local receiver cache parameters
     * Configures the detector to the receiver as UDP destination
     * @param receiver receiver hostname or IP address
     */
    void setRxHostname(const std::string &receiver, Positions pos = {});

    Result<int> getRxPort(Positions pos = {}) const;

    /** Receiver TCP port (for client communication with Receiver)  */
    void setRxPort(int value, Positions pos = {});

    Result<int> getRxFifoDepth(Positions pos = {}) const;

    /** fifo between udp listening and processing threads */
    void setRxFifoDepth(int nframes, Positions pos = {});

    Result<bool> getRxSilentMode(Positions pos = {}) const;

    /** receiver prints hardly any information while acquiring */
    void setRxSilentMode(bool value, Positions pos = {});

    Result<defs::frameDiscardPolicy>
    getRxFrameDiscardPolicy(Positions pos = {}) const;

    /**
     * default NO_DISCARD
     * Options: NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
     * discard partial frames is the fastest
     */
    void setRxFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos = {});
                                       
    Result<bool> getPartialFramesPadding(Positions pos = {}) const;

    /** padding enabled. Disabling padding is the fastest */
    void setPartialFramesPadding(bool value, Positions pos = {});

    Result<int64_t> getRxUDPSocketBufferSize(Positions pos = {}) const;

    void setRxUDPSocketBufferSize(int64_t udpsockbufsize,
                                        Positions pos = {});
    /** TODO: 
     * Linux kernel allocates twice the amount you set for bookkeeping purposes */
    Result<int64_t>
    getRxRealUDPSocketBufferSize(Positions pos = {}) const;

    Result<bool> getRxLock(Positions pos = {});

    /** locks receiver server to client IP */
    void setRxLock(bool value, Positions pos = {});

    Result<std::string> getRxLastClientIP(Positions pos = {}) const;


    /**************************************************
     *                                                *
     *    FILE                                        *
     *                                                *
     * ************************************************/
    Result<defs::fileFormat> getFileFormat(Positions pos = {}) const;

    /** default binary, Options: BINARY, HDF5 (library must be compiled with
     * this option) */
    void setFileFormat(defs::fileFormat f, Positions pos = {});

    Result<std::string> getFilePath(Positions pos = {}) const;

    void setFilePath(const std::string &fname, Positions pos = {});

    Result<std::string> getFileNamePrefix(Positions pos = {}) const;

    /** default run 
     * File Name: [file name prefix]_d[module index]_f[file index]_[acquisition index].[file format]
     * eg. run_d0_f0_5.raw
    */
    void setFileNamePrefix(const std::string &fname, Positions pos = {});

    Result<int> getAcquisitonIndex(Positions pos = {}) const;

    void setAcquisitionIndex(int i, Positions pos = {});

    Result<bool> getFileWrite(Positions pos = {}) const;

    /** default writes */
    void setFileWrite(bool value, Positions pos = {});

    Result<bool> getMasterFileWrite(Positions pos = {}) const;

    /* default writes */
    void setMasterFileWrite(bool value, Positions pos = {});

    Result<bool> getFileOverWrite(Positions pos = {}) const;

    /** default overwites */
    void setFileOverWrite(bool value, Positions pos = {});

    Result<int> getFramesPerFile(Positions pos = {}) const;

    /** -1 will set frames per file to unlimited //TODO check if it is -1 or 0 */
    void setFramesPerFile(int n, Positions pos = {});


    /**************************************************
     *                                                *
     *    ZMQ Streaming Parameters (Receiver<->Client)*
     *                                                *
     * ************************************************/
    //TODO callback functions

    Result<bool> getRxZmqDataStream(Positions pos = {}) const;

    void setRxZmqDataStream(bool value, Positions pos = {});

    Result<int> getRxZmqFrequency(Positions pos = {}) const;

    /** @param freq nth frame streamed out of receiver.
     * If 0, streaming timer is the timeout,
     * after which current frame sent out. Default is 0 at 200 ms.
     * Default is 0. This is more for gui purposes to not send every frame.
     * If you want every frame, set freq to 1.
     */
    void setRxZmqFrequency(int freq, Positions pos = {});

    Result<int> getRxZmqTimer(Positions pos = {}) const;

    /**
     * If receiver streaming frequency is 0 (default), then this timer between
     * each data stream is set. Default is 200 ms.
     */
    void setRxZmqTimer(int time_in_ms, Positions pos = {});

    Result<int> getRxZmqPort(Positions pos = {}) const;

    /**
     * module_id is -1 for all detectors, ports for each module is calculated (increments)
     * Restarts receiver zmq sockets only if it was already enabled
     */
    void setRxZmqPort(int port, int module_id = -1);

    Result<std::string> getRxZmqIP(Positions pos = {}) const;

    void setRxZmqIP(const std::string &ip,
                                       Positions pos = {});

    Result<int> getClientZmqPort(Positions pos = {}) const;
    
    /**
     * Needed only when using the client call back to get reconstructed data from multi modules
     * module_id is -1 for all detectors, ports for each module is calculated (increments)
     * Restarts client zmq sockets oonly if it was already enabled
     */
    void setClientZmqPort(int port, int module_id = -1);

    Result<std::string> getClientZmqIp(Positions pos = {}) const;

    void setClientZmqIp(const std::string &ip, Positions pos = {});


    /**************************************************
     *                                                *
     *    Eiger Specific                              *
     *                                                *
     * ************************************************/

    Result<int> getDynamicRange(Positions pos = {}) const;

    /**
     * [Eiger]
     * Options: 4, 8, 16, 32
     * If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1
     */
    void setDynamicRange(int value);

    /** [Eiger] in 32 bit mode */
    Result<ns> getSubExptime(Positions pos = {}) const;

    /** [Eiger] in 32 bit mode */
    void setSubExptime(ns t, Positions pos = {});

    /** [Eiger] in 32 bit mode */
    Result<ns> getSubDeadTime(Positions pos = {}) const;

    /** [Eiger] in 32 bit mode */
    void setSubDeadTime(ns value, Positions pos = {});

    /** [Eiger] */
    Result<int> getThresholdEnergy(Positions pos = {}) const;

    /** [Eiger] */
    void setThresholdEnergy(int threshold_ev,
                            defs::detectorSettings settings = defs::STANDARD,
                            bool trimbits = true, Positions pos = {});

    /** [Eiger] */
    Result<std::string> getSettingsDir(Positions pos = {}) const;

    /** [Eiger] */
    void setSettingsDir(const std::string &value, Positions pos = {});

    /** [Eiger] */
    void loadTrimbits(const std::string &fname, Positions pos = {});

    /**[Eiger] */
    Result<bool> getRxAddGapPixels(Positions pos = {}) const;

    /**
     * [Eiger]
     * 4 bit mode not implemented in Receiver, but in client data call back
     * Fills in gap pixels in data
     */
    void setRxAddGapPixels(bool enable);

    /** [Eiger] */
    Result<bool> getParallelMode(Positions pos = {}) const;

    /** [Eiger] */
    void setParallelMode(bool value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getOverFlowMode(Positions pos = {}) const;

    /** [Eiger] */
    void setOverFlowMode(bool value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getBottom(Positions pos = {}) const;

    /** [Eiger] for client call back (gui) purposes */
    void setBottom(bool value, Positions pos = {});

    /** [Eiger] -1 if they are all different */
    Result<int> getAllTrimbits(Positions pos = {}) const;

    /**[Eiger] */
    void setAllTrimbits(int value, Positions pos = {});

    /**[Eiger] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEnergies(Positions pos = {}) const;

    /** [Eiger] Set the energies where the detector is trimmed */
    void setTrimEnergies(std::vector<int> energies, Positions pos = {});

    /** [Eiger] deadtime in ns, 0 = disabled */
    Result<ns> getRateCorrection(Positions pos = {}) const;

    /** //TODO: default, get, set
     * [Eiger] Set Rate correction
     * 0 disable correction, < 0: default dead time from trimbit file, > 0 custom deadtime (advanced)
     */
    void setRateCorrection(ns dead_time, Positions pos = {});

    /** [Eiger] */
    Result<int> getPartialReadout(Positions pos = {}) const;

    /** [Eiger] Number of lines to read out per half module
     * Options: 0 - 256. Depending on dynamic range and
     * 10 GbE enabled, only specific values are accepted.
     */
    void setPartialReadout(const int lines, Positions pos = {});

    /** [Eiger] */
    Result<bool> getInterruptSubframe(Positions pos = {}) const;

    /** [Eiger] when set, the last subframe is interrupted at end of acq */
    void setInterruptSubframe(const bool enable, Positions pos = {});

    /** [Eiger] minimum two frames */
    Result<ns> getMeasuredPeriod(Positions pos = {}) const;

    /** [Eiger] */
    Result<ns> getMeasuredSubFramePeriod(Positions pos = {}) const;

    /** [Eiger] */
    Result<bool> getActive(Positions pos = {}) const;

    /** [Eiger] */
    void setActive(bool active, Positions pos = {});

    /** [Eiger] */
    Result<bool> getRxPadDeactivatedMode(Positions pos = {}) const;

    /** [Eiger] Pad deactivated modules in receiver */
    void setRxPadDeactivatedMode(bool pad, Positions pos = {});

    /** [Eiger] Advanced */
    Result<bool> getPartialReset(Positions pos = {}) const;

    /** [Eiger] Advanced 
     * used for pulsing chips */
    void setPartialReset(bool enable, Positions pos = {});

    /** [Eiger] Advanced 
     * Pulse Pixel n times at x and y coordinates */
    void pulsePixel(int n, defs::xy pixel, Positions pos = {});

    /** [Eiger] Advanced 
     * Pulse Pixel n times and move by a relative value of x and y
     * coordinates */
    void pulsePixelNMove(int n, defs::xy pixel, Positions pos = {});

    /** [Eiger] Advanced
     * Pulse chip n times */
    void pulseChip(int n, Positions pos = {});

    /** [Eiger] with specific quad hardware */
    Result<bool> getQuad(Positions pos = {}) const;

    /** [Eiger] with specific quad hardware */
    void setQuad(const bool enable);


    /**************************************************
     *                                                *
     *    Jungfrau Specific                           *
     *                                                *
     * ************************************************/

    /** [Jungfrau] */
    Result<int> getThresholdTemperature(Positions pos = {}) const;

    /**
     * [Jungfrau]Set threshold temperature
     * If temperature crosses threshold temperature
     * and temperature control is enabled,
     * power to chip will be switched off and
     * temperature event will be set
     * @param val value in degrees
     */
    void setThresholdTemperature(int temp, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getTemperatureControl(Positions pos = {}) const;

    /** [Jungfrau] */
    void setTemperatureControl(bool enable, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getTemperatureEvent(Positions pos = {}) const;

    /** [Jungfrau] */
    void resetTemperatureEvent(Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getPowerChip(Positions pos = {}) const;

    /** [Jungfrau] */
    void setPowerChip(bool on, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getAutoCompDisable(Positions pos = {}) const;

    /** [Jungfrau] Advanced
     * //TODO naming
     * By default, the on-chip gain switching is active during the entire exposure.
     * This mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us). */
    void setAutoCompDisable(bool value, Positions pos = {});
    
    /** [Jungfrau] Advanced TODO naming */
    Result<int64_t> getNumberOfAdditionalStorageCells() const;

    /** [Jungfrau] Advanced */
    void setNumberOfAdditionalStorageCells(int64_t value);

    /** [Jungfrau] Advanced */
    Result<int> getStorageCellStart(Positions pos = {}) const;

    /** [Jungfrau] Advanced. Sets the storage cell storing the first acquisition of the
     * series. Options: 0-15
     */
    void setStoragecellStart(int cell, Positions pos = {});

    /** [Jungfrau] Advanced*/
    Result<ns> getStorageCellDelay(Positions pos = {}) const;

    /** [Jungfrau] Advanced
     * Options: (0-1638375 ns (resolution of 25ns) */
    void setStorageCellDelay(ns value, Positions pos = {});


    /**************************************************
     *                                                *
     *    Gotthard Specific                           *
     *                                                *
     * ************************************************/

    /** [Gotthard]*/
    Result<defs::ROI> getROI(Positions pos = {}) const;

    /**
     * [Gotthard]
     * Options: Only a single ROI per module
     * Can set only a single ROI at a time
     * @param module position index
     */
    void setROI(defs::ROI value, int moduleId);

    /** [Gotthard] TODO: check with jiaguo if he needs any of these functions */
    Result<ns> getExptimeLeft(Positions pos = {}) const;

    /** [Gotthard] */
    Result<ns> getPeriodLeft(Positions pos = {}) const;

    /** [Gotthard] */
    Result<defs::externalSignalFlag>
    getExternalSignalFlags(Positions pos = {}) const;

    /** [Gotthard] Options: TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE */
    void setExternalSignalFlags(defs::externalSignalFlag value,
                                Positions pos = {});

    /** [Gotthard] */
    void loadDarkImage(const std::string &fname, int module_id = -1);

    /** [Gotthard] */
    void loadGainImage(const std::string &fname, int module_id = -1);

    /**
     * [Gotthard] subset modules not allowed
     * @param startACQ if start acq after reading counter
     */
    void getCounterMemoryBlock(const std::string &fname, bool startACQ,
                               Positions pos = {});

    /**
     * [Gotthard]
     * @param startACQ if start acq after resetting counter
     * TODO! does it make sense to call one detector?
     */
    void resetCounterBlock(bool startACQ, Positions pos = {});

    /** [Gotthard] */
    Result<int> getDigitalTestBit(Positions pos = {});

    /** [Gotthard] */
    Result<int> setDigitalTestBit(const int value, Positions pos = {});

    /**************************************************
     *                                                *
     *    CTB Specific                                *
     *                                                *
     * ************************************************/

    /** [CTB] */
    Result<int64_t> getNumberOfAnalogSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfAnalogSamples(int64_t value, Positions pos = {});

    /** [CTB] */
    Result<int64_t> getNumberOfDigitalSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfDigitalSamples(int64_t value, Positions pos = {});

    /** [CTB] */
    Result<int> getSignalType(Positions pos = {}) const;

    /** [CTB] Options: NORMAL_READOUT = 0, DIGITAL_ONLY = 1, ANALOG_AND_DIGITAL
     * = 2 */
    void setSignalType(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITPhase(bool inDeg, Positions pos = {}) const;

    /** [CTB] */
    void setDBITPhase(int value, bool inDeg, Positions pos = {});

    /** [CTB] */
    Result<int> getMaxDBITPhaseShift(Positions pos = {}) const;

    /** [CTB] */
    Result<int> getADCClock(Positions pos = {}) const;

    /** [CTB] */
    void setADCClock(int value_in_MHz, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITClock(Positions pos = {}) const;

    /** [CTB] */
    void setDBITClock(int value_in_MHz, Positions pos = {});

    /** [CTB] */
    Result<int> getRUNClock(Positions pos = {}) const;

    /** [CTB] */
    void setRUNClock(int value_in_MHz, Positions pos = {});

    /** [CTB] */
    Result<int> getSYNCClock(Positions pos = {}) const;

    /** [CTB] */
    Result<int> getADCPipeline(Positions pos = {}) const;

    /** [CTB] */
    void setADCPipeline(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITPipeline(Positions pos = {}) const;

    /** [CTB] */
    void setDBITPipeline(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getVrefVoltage(bool mV, Positions pos = {}) const;

    /** [CTB] */
    void setVrefVoltage(int value, bool mV, Positions pos = {});

    /** [CTB] */
    Result<int> getVoltage(defs::dacIndex index, Positions pos = {}) const;

    /**
     * [CTB] mV
     * Options: V_LIMIT, V_POWER_A, V_POWER_B, V_POWER_C,
     * V_POWER_D, V_POWER_IO, V_POWER_CHIP
     */
    void setVoltage(int value, defs::dacIndex index, Positions pos = {});

    /**
     * [CTB] mV
     * Options: V_POWER_A, V_POWER_B, V_POWER_C, V_POWER_D, V_POWER_IO,
     * V_POWER_CHIP
     */
    Result<int> getMeasuredVoltage(defs::dacIndex index,
                                   Positions pos = {}) const;

    /**
     * [CTB] mA
     * Options: I_POWER_A, I_POWER_B, I_POWER_C, I_POWER_D, I_POWER_IO
     */
    Result<int> getMeasuredCurrent(defs::dacIndex index,
                                   Positions pos = {}) const;

    /** [CTB] Options: SLOW_ADC0 - SLOW_ADC7 */
    Result<int> getSlowADC(defs::dacIndex index, Positions pos = {}) const;

    /** [CTB]*/
    Result<uint32_t> getADCEnableMask(Positions pos = {}) const;

    /** [CTB]*/
    void setADCEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB] */
    Result<uint32_t> getADCInvert(Positions pos = {}) const;

    /** [CTB]*/
    void setADCInvert(uint32_t value, Positions pos = {});

    /** [CTB] */
    Result<int> getExternalSamplingSource(Positions pos = {}) const;

    /** [CTB] Value between 0-63 */
    void setExternalSamplingSource(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getExternalSampling(Positions pos = {}) const;

    /** [CTB] */
    void setExternalSampling(bool value, Positions pos = {});

    /** [CTB] */
    Result<std::vector<int>> getRxDbitList(Positions pos = {}) const;

    /** [CTB] list contains the set of bits (0-63) to save */
    void setRxDbitList(std::vector<int> list, Positions pos = {});

    /** [CTB] */
    Result<int> getRxDbitOffset(Positions pos = {}) const;

    /** [CTB] Set number of bytes of digital data to skip in the Receiver */
    void setRxDbitOffset(int value, Positions pos = {});

    /**
     * [CTB] Set Digital IO Delay
     * cannot get
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});

    /** [CTB] */
    Result<bool> getLEDEnable(Positions pos = {}) const;

    /** [CTB] */
    void setLEDEnable(bool enable, Positions pos = {});


    /**************************************************
     *                                                *
     *    PATTERN                                     *
     *                                                *
     * ************************************************/

    /** [CTB] */
    void setPattern(const std::string &fname, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternIOControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternIOControl(uint64_t word, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternClockControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternClockControl(uint64_t word, Positions pos = {});

    /** [CTB] Caution: If word is  -1  reads the addr (same as
     * executing the pattern) */
    void setPatternWord(int addr, uint64_t word, Positions pos = {});

    /**[CTB] Options: level: -1 (complete pattern) and 0-2 levels
     * @returns array of start address, stop address and number of loops
     */
    Result<std::array<int, 3>> getPatternLoops(int level,
                                               Positions pos = {}) const;

    /** [CTB] Options: start, stop, n : 0-2
     * level: -1 (complete pattern) and 0-2 levels */
    void setPatternLoops(int level, int start, int stop, int n,
                         Positions pos = {});

    /* [CTB] */
    Result<int> getPatternWaitAddr(int level, Positions pos = {}) const;

    /** [CTB] Options: level 0-2 */
    void setPatternWaitAddr(int level, int addr, Positions pos = {});

    /** [CTB]  */
    Result<uint64_t> getPatternWaitTime(int level, Positions pos = {}) const;

    /** [CTB] Options: level 0-2 */
    void setPatternWaitTime(int level, uint64_t t, Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /** [CTB] Sets the mask applied to every pattern to the selected bit mask */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** [CTB]  */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /** [CTB] Sets the bitmask that the mask will be applied to for every
     * pattern
     */
    void setPatternBitMask(uint64_t mask, Positions pos = {});

    /**************************************************
     *                                                *
     *    Moench                                      *
     *                                                *
     * ************************************************/


    /** [Moench] */
    Result<std::string> getAdditionalJsonHeader(Positions pos = {}) const;

    /** [Moench] */
    void setAdditionalJsonHeader(const std::string &jsonheader,
                                 Positions pos = {});

    /** [Moench] */
    Result<std::string> getAdditionalJsonParameter(const std::string &key,
                                                   Positions pos = {}) const;
    /**
     * [Moench]
     * Sets the value for additional json header parameter if found,
     * else appends the parameter key and value
     * The value cannot be empty
     */
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value,
                                    Positions pos = {});

        /** [Moench] TODO! How do we do this best??? Can be refactored to something
     * else? Use a generic zmq message passing system...
     * For now limiting to all detectors working the same*/
    /** [Moench: -1 if not found or cannot convert to int] */
    Result<int> getDetectorMinMaxEnergyThreshold(const bool isEmax,
                                                 Positions pos = {}) const;

    /** [Moench] */
    void setDetectorMinMaxEnergyThreshold(const bool isEmax, const int value,
                                          Positions pos = {});

    /** [Moench: -1 if unknown mode] */
    Result<int> getFrameMode(Positions pos = {}) const;

    /** [Moench] */
    void setFrameMode(defs::frameModeType value, Positions pos = {});

    /** [Moench: -1 if unknown mode] */
    Result<int> getDetectorMode(Positions pos = {}) const;

    /** [Moench] */
    void setDetectorMode(defs::detectorModeType value, Positions pos = {});

    /**************************************************
     *                                                *
     *    Advanced                                    *
     *                                                *
     * ************************************************/

    /** [Jungfrau][CTB] */
    void programFPGA(const std::string &fname, Positions pos = {});

    /** [Jungfrau][CTB] */
    void resetFPGA(Positions pos = {});

        /** [Jungfrau][Gotthard][CTB]
     * Copy detector server fname from tftp folder of hostname to detector
     * Also changes respawn server, which is effective after a reboot.
     */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname, Positions pos = {});

    /** [Jungfrau][Gotthard][CTB] */
    void rebootController(Positions pos = {});

    /**
     * [Jungfrau][Gotthard][CTB]
     * Updates the firmware, detector server and then reboots detector
     * controller blackfin.
     * @param sname name of detector server binary found on tftp folder of host
     * pc
     * @param hostname name of pc to tftp from
     * @param fname programming file name
     * @param pos detector positions
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &hostname,
                                 const std::string &fname, Positions pos = {});

    Result<uint32_t> readRegister(uint32_t addr, Positions pos = {}) const;

    void writeRegister(uint32_t addr, uint32_t val, Positions pos = {});

    void setBit(uint32_t addr, int bitnr, Positions pos = {});

    void clearBit(uint32_t addr, int bitnr, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int> executeFirmwareTest(Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int> executeBusTest(Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] not possible to read back*/
    void writeAdcRegister(uint32_t addr, uint32_t value, Positions pos = {});


    /**************************************************
     *                                                *
     *    Insignificant                               *
     *                                                *
     * ************************************************/



    Result<int> getControlPort(Positions pos = {}) const;

    /** Detector Control TCP port (for client communication with Detector
     * control server) */
    void setControlPort(int value, Positions pos = {});

    Result<int> getStopPort(Positions pos = {}) const;

    /** Detector Stop TCP port (for client communication with Detector Stop
     * server) */
    void setStopPort(int value, Positions pos = {});

    Result<bool> getDetectorLock(Positions pos = {}) const;

    void setDetectorLock(bool lock, Positions pos = {});

    /** Get last client IP saved on detector server */
    Result<std::string> getLastClientIP(Positions pos = {}) const;

    /** Execute a command on the detector server */
    void execCommand(const std::string &value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfFramesFromStart(Positions pos = {}) const;

    /** [Jungfrau][CTB] Get time from detector start */
    Result<ns> getActualTime(Positions pos = {}) const;

    /** [Jungfrau][CTB] Get timestamp at a frame start */
    Result<ns> getMeasurementTime(Positions pos = {}) const;

    std::string getUserDetails() const;

   Result<uint64_t> getRxCurrentFrameIndex(Positions pos = {}) const;

private:
std::vector<int> getPortNumbers(int start_port);

};

} // namespace sls
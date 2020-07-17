#pragma once
#include "Result.h"
#include "network_utils.h"
#include "sls_detector_defs.h"
#include <chrono>
#include <map>
#include <memory>
#include <vector>

class detectorData;

namespace sls {
using ns = std::chrono::nanoseconds;
class DetectorImpl;
class MacAddr;
class IpAddr;

// Free function to avoid dependence on class
// and avoid the option to free another objects
// shm by mistake
void freeSharedMemory(int multiId, int detPos = -1);

/**
 * \class Detector
 */
class Detector {
    std::unique_ptr<DetectorImpl> pimpl;

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
     *    Configuration                               *
     *                                                *
     * ************************************************/

    /* Free the shared memory of this detector and all modules
     * belonging to it */
    void freeSharedMemory();

    void loadConfig(const std::string &fname);

    void loadParameters(const std::string &fname);

    Result<std::string> getHostname(Positions pos = {}) const;

    /* Frees shared memory, adds detectors to the list
     * and updates local detector cache */
    void setHostname(const std::vector<std::string> &hostname);

    /** connects to n servers at local host starting at specific control port */
    void setVirtualDetectorServers(int numServers, int startingPort);

    /** Gets shared memory ID */
    int getShmId() const;

    /** package git branch */
    std::string getPackageVersion() const;

    int64_t getClientVersion() const;

    Result<int64_t> getFirmwareVersion(Positions pos = {}) const;

    Result<int64_t> getDetectorServerVersion(Positions pos = {}) const;

    Result<int64_t> getSerialNumber(Positions pos = {}) const;

    Result<int64_t> getReceiverVersion(Positions pos = {}) const;

    Result<defs::detectorType> getDetectorType(Positions pos = {}) const;

    /** Gets the total number of detectors */
    int size() const;

    bool empty() const;

    defs::xy getModuleGeometry() const;

    Result<defs::xy> getModuleSize(Positions pos = {}) const;

    /** Gets the actual full detector size. It is the same even if ROI changes
     */
    defs::xy getDetectorSize() const;

    /**
     * Sets the detector size in both dimensions.
     * This value is used to calculate row and column positions for each module.
     */
    void setDetectorSize(const defs::xy value);

    /** list of possible settings for this detector */
    std::vector<defs::detectorSettings> getSettingsList() const;

    /** [Jungfrau][Gotthard][Gotthard2] */
    Result<defs::detectorSettings> getSettings(Positions pos = {}) const;

    /** [Jungfrau] Options:DYNAMICGAIN, DYNAMICHG0, FIXGAIN1, FIXGAIN2,
     * FORCESWITCHG1, FORCESWITCHG2 [Gotthard] Options: DYNAMICGAIN, HIGHGAIN,
     * LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN [Gotthard2] Options: DYNAMICGAIN,
     * FIXGAIN1, FIXGAIN2 [Moench] Options: G1_HIGHGAIN, G1_LOWGAIN,
     * G2_HIGHCAP_HIGHGAIN, G2_HIGHCAP_LOWGAIN, G2_LOWCAP_HIGHGAIN,
     * G2_LOWCAP_LOWGAIN, G4_HIGHGAIN, G4_LOWGAIN
     */
    void setSettings(defs::detectorSettings value, Positions pos = {});

    /** [Eiger][Mythen3] */
    void loadTrimbits(const std::string &fname, Positions pos = {});

    /** [Eiger][Mythen3] -1 if they are all different */
    Result<int> getAllTrimbits(Positions pos = {}) const;

    /**[Eiger][Mythen3] */
    void setAllTrimbits(int value, Positions pos = {});

    /**[Eiger][Jungfrau] */
    bool getGapPixelsinCallback() const;

    /**
     * [Eiger][Jungfrau]
     * Only in client data call back
     * Fills in gap pixels in data
     */
    void setGapPixelsinCallback(const bool enable);

    /**************************************************
     *                                                *
     *    Callbacks                                   *
     *                                                *
     * ************************************************/

    /**
     * register callback for end of acquisition
     * @param func function to be called with parameters:
     * current progress in percentage, detector status, pArg pointer
     * @param pArg pointer that is returned in call back
     */
    void registerAcquisitionFinishedCallback(void (*func)(double, int, void *),
                                             void *pArg);

    /**
     * register callback for accessing reconstructed complete images
     * Receiver sends out images via zmq, the client reconstructs them into
     * complete images. Therefore, it also enables zmq streaming from receiver
     * and the client.
     * @param func function to be called for each image with parameters:
     * detector data structure, frame number, sub frame number (for eiger in 32
     * bit mode), pArg pointer
     * @param pArg pointer that is returned in call back
     */
    void registerDataCallback(void (*func)(detectorData *, uint64_t, uint32_t,
                                           void *),
                              void *pArg);

    /**************************************************
     *                                                *
     *    Acquisition Parameters                      *
     *                                                *
     * ************************************************/

    Result<int64_t> getNumberOfFrames(Positions pos = {}) const;

    void setNumberOfFrames(int64_t value);

    Result<int64_t> getNumberOfTriggers(Positions pos = {}) const;

    void setNumberOfTriggers(int64_t value);

    Result<ns> getExptime(Positions pos = {}) const;

    void setExptime(ns t, Positions pos = {});

    Result<ns> getPeriod(Positions pos = {}) const;

    void setPeriod(ns t, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3][Gotthard2] */
    Result<ns> getDelayAfterTrigger(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3][Gotthard2] */
    void setDelayAfterTrigger(ns value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3]
     * [Gotthard2] only in continuous mode */
    Result<int64_t> getNumberOfFramesLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3]
     * [Gotthard2] only in continuous mode */
    Result<int64_t> getNumberOfTriggersLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3]
     * [Gotthard2] only in continuous mode */
    Result<ns> getPeriodLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench][Mythen3]
     * [Gotthard2] only in continuous mode */
    Result<ns> getDelayAfterTriggerLeft(Positions pos = {}) const;

    Result<defs::timingMode> getTimingMode(Positions pos = {}) const;

    /**
     * [Gotthard][Jungfrau][CTB][Moench][Mythen3] Options:
     * AUTO_TIMING, TRIGGER_EXPOSURE
     * [Gotthard2] Options: AUTO_TIMING, TRIGGER_EXPOSURE, GATED, TRIGGER_GATED
     * [Eiger] Options: AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER
     */
    void setTimingMode(defs::timingMode value, Positions pos = {});

    /** [Eiger][Jungfrau] */
    Result<defs::speedLevel> getSpeed(Positions pos = {}) const;

    /** [Eiger][Jungfrau]
     * Options: FULL_SPEED, HALF_SPEED, QUARTER_SPEED */
    void setSpeed(defs::speedLevel value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB][Moench] */
    Result<int> getADCPhase(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench] */
    void setADCPhase(int value, Positions pos = {});

    /** [Jungfrau][CTB][Moench] */
    Result<int> getMaxADCPhaseShift(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench] */
    Result<int> getADCPhaseInDegrees(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB][Moench] */
    void setADCPhaseInDegrees(int value, Positions pos = {});

    /** [CTB][Jungfrau] */
    Result<int> getDBITPhase(Positions pos = {}) const;

    /** [CTB][Jungfrau] */
    void setDBITPhase(int value, Positions pos = {});

    /** [CTB][Jungfrau] */
    Result<int> getMaxDBITPhaseShift(Positions pos = {}) const;

    /** [CTB][Jungfrau] */
    Result<int> getDBITPhaseInDegrees(Positions pos = {}) const;

    /** [CTB][Jungfrau] */
    void setDBITPhaseInDegrees(int value, Positions pos = {});

    /** [Mythen3][Gotthard2] Hz */
    Result<int> getClockFrequency(int clkIndex, Positions pos = {});

    /** [not implemented] Hz */
    void setClockFrequency(int clkIndex, int value, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockPhase(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    void setClockPhase(int clkIndex, int value, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getMaxClockPhaseShift(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockPhaseinDegrees(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    void setClockPhaseinDegrees(int clkIndex, int value, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockDivider(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    void setClockDivider(int clkIndex, int value, Positions pos = {});

    Result<int> getHighVoltage(Positions pos = {}) const;

    /**
     * [Gotthard] Options: 0, 90, 110, 120, 150, 180, 200
     * [Jungfrau][CTB][Moench] Options: 0, 60 - 200
     * [Eiger][Mythen3][Gotthard2] Options: 0 - 200
     */
    void setHighVoltage(int value, Positions pos = {});

    /** [Jungfrau][Mythen3][Gotthard2][Moench] */
    Result<bool> getPowerChip(Positions pos = {}) const;

    /** [Jungfrau][Mythen3][Gotthard2][Moench] */
    void setPowerChip(bool on, Positions pos = {});

    /** [Gotthard][Eiger virtual] */
    Result<int> getImageTestMode(Positions pos = {});

    /** [Gotthard] If 1, adds channel intensity with precalculated values.
     * Default is 0
     * [Eiger virtual] If 1, pixels are saturated. If 0, increasing intensity
     * Only for virtual servers */
    void setImageTestMode(const int value, Positions pos = {});

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

    /** gets list of dac indices for this detector */
    std::vector<defs::dacIndex> getDacList() const;

    Result<int> getDAC(defs::dacIndex index, bool mV, Positions pos = {}) const;

    void setDAC(defs::dacIndex index, int value, bool mV, Positions pos = {});

    /* [Gotthard2] */
    Result<int> getOnChipDAC(defs::dacIndex index, int chipIndex,
                             Positions pos = {}) const;

    /* [Gotthard2] */
    void setOnChipDAC(defs::dacIndex index, int chipIndex, int value,
                      Positions pos = {});

    /** [Gotthard] signal index is 0
     * [Mythen3] signal index 0-3 for master input, 4-7 master output signals */
    Result<defs::externalSignalFlag>
    getExternalSignalFlags(int signalIndex, Positions pos = {}) const;

    /** [Gotthard]  signal index is 0
     * Options: TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE
     * [Mythen3] signal index 0 is master input trigger signal, 1-3 for master
     * input gate signals, 4 is busy out signal, 5-7 is master output gate
     * signals.
     * Options: TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE (for
     * master input trigger only), INVERSION_ON, INVERSION_OFF */
    void setExternalSignalFlags(int signalIndex, defs::externalSignalFlag value,
                                Positions pos = {});

    /**************************************************
     *                                                *
     *    Acquisition                                 *
     *                                                *
     * ************************************************/
    /**
     * Blocking call: Acquire the number of frames set
     * - sets acquiring flag
     * - starts the receiver listener
     * - starts detector acquisition for number of frames set
     * - monitors detector status from running to idle
     * - stops the receiver listener
     * - increments file index if file write enabled
     * - resets acquiring flag
     */
    void acquire();

    /** If acquisition aborted, use this to clear before starting next
     * acquisition */
    void clearAcquiringFlag();

    /** Non Blocking: Start receiver listener*/
    void startReceiver();

    /** Non Blocking: Stop receiver listener */
    void stopReceiver();

    /** Non blocking: start detector acquisition
     * detector status changes from RUNNING to IDLE when finished */
    void startDetector();

    /** Non blocking: abort detector acquisition */
    void stopDetector();

    Result<defs::runStatus> getDetectorStatus(Positions pos = {}) const;

    Result<defs::runStatus> getReceiverStatus(Positions pos = {}) const;

    Result<int64_t> getFramesCaught(Positions pos = {}) const;

    Result<std::vector<uint64_t>>
    getNumMissingPackets(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    void setStartingFrameNumber(uint64_t value, Positions pos = {});

    /** [Eiger] Sends an internal software trigger to the detector */
    void sendSoftwareTrigger(Positions pos = {});

    Result<defs::scanParameters> getScan(Positions pos = {}) const;

    /** enables/ disables scans for  dac, trimbits [Eiger/ Mythen3]
     * TRIMBIT_SCAN. Enabling scan sets number of frames to number of steps in
     * receiver. Disabling scan sets number of frames to 1 */
    void setScan(const defs::scanParameters t);

    /** gets scan error message in case of error during scan in case of non
     * blocking acquisition (startDetector, not acquire) */
    Result<std::string> getScanErrorMessage(Positions pos = {}) const;

    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/

    /** [Jungfrau][Gotthard2] */
    Result<int> getNumberofUDPInterfaces(Positions pos = {}) const;

    /** [Jungfrau][Gotthard2] Also restarts client and receiver zmq sockets
     * [Gotthard2] second interface enabled to send veto information via 10gbps
     * for debugging. By default it is sent via 2.5gbps if veto enabled
     * n can be 1 or 2 */
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
    void setSourceUDPIP(const IpAddr ip, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<IpAddr> getSourceUDPIP2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setSourceUDPIP2(const IpAddr ip, Positions pos = {});

    Result<MacAddr> getSourceUDPMAC(Positions pos = {}) const;

    /* For Eiger 1G, the detector will replace with its own DHCP MAC
     * For Eiger 10G, the detector will replace with its own DHCP MAC + 1
     * Others can be anything (beware of certain bits)
     */

    void setSourceUDPMAC(const MacAddr mac, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<MacAddr> getSourceUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setSourceUDPMAC2(const MacAddr mac, Positions pos = {});

    Result<IpAddr> getDestinationUDPIP(Positions pos = {}) const;

    /** IP of the interface in receiver that the detector sends data to */
    void setDestinationUDPIP(const IpAddr ip, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<IpAddr> getDestinationUDPIP2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setDestinationUDPIP2(const IpAddr ip, Positions pos = {});

    Result<MacAddr> getDestinationUDPMAC(Positions pos = {}) const;

    /** MAC of the interface in receiver that the detector sends data to
     * Only needed if you use a custom receiver (not slsReceiver)
     * Must be followed by configuremac.
     */
    void setDestinationUDPMAC(const MacAddr mac, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<MacAddr> getDestinationUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setDestinationUDPMAC2(const MacAddr mac, Positions pos = {});

    Result<int> getDestinationUDPPort(Positions pos = {}) const;

    /** module_id is -1 for all detectors, ports for each module is calculated
     * (increments) */
    void setDestinationUDPPort(int port, int module_id = -1);

    /** [Eiger right port][Jungfrau bottom half] */
    Result<int> getDestinationUDPPort2(Positions pos = {}) const;

    /** [Eiger right port][Jungfrau bottom half]
     * module_id is -1 for all detectors, ports for each module is calculated
     * (increments)
     */
    void setDestinationUDPPort2(int port, int module_id = -1);

    Result<std::string> printRxConfiguration(Positions pos = {}) const;

    /** [Eiger][CTB][Moench] */
    Result<bool> getTenGiga(Positions pos = {}) const;

    /** [Eiger][CTB][Moench] */
    void setTenGiga(bool value, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<bool> getTenGigaFlowControl(Positions pos = {}) const;

    /** [Eiger, Jungfrau] */
    void setTenGigaFlowControl(bool enable, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<int> getTransmissionDelayFrame(Positions pos = {}) const;

    /**
     * [Jungfrau]: Sets the transmission delay of the first UDP packet being
     * streamed out of the module. Options: 0 - 31, each value represenets 1 ms
     * [Eiger]: Sets the transmission delay of entire frame streamed out for
     * both left and right UDP ports. Options: //TODO possible values
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
     *    Receiver Config                             *
     *                                                *
     * ************************************************/

    /** true when slsReceiver is used */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    Result<std::string> getRxHostname(Positions pos = {}) const;

    /**
     * Validates and sets the receiver.
     * Updates local receiver cache parameters
     * Configures the detector to the receiver as UDP destination
     * receiver is receiver hostname or IP address, can include tcp port eg.
     * hostname:port
     */
    void setRxHostname(const std::string &receiver, Positions pos = {});

    /** multiple rx hostnames (same as setRxHostname) */
    void setRxHostname(const std::vector<std::string> &name);

    Result<int> getRxPort(Positions pos = {}) const;

    /** Receiver TCP port (for client communication with Receiver)
     *  module_id is -1 for all detectors, ports for each module is calculated
     * (increments) */
    void setRxPort(int port, int module_id = -1);

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

    void setRxUDPSocketBufferSize(int64_t udpsockbufsize, Positions pos = {});
    /** TODO:
     * Linux kernel allocates twice the amount you set for bookkeeping purposes
     */
    Result<int64_t> getRxRealUDPSocketBufferSize(Positions pos = {}) const;

    Result<bool> getRxLock(Positions pos = {});

    /** locks receiver server to client IP */
    void setRxLock(bool value, Positions pos = {});

    Result<sls::IpAddr> getRxLastClientIP(Positions pos = {}) const;

    Result<std::array<pid_t, NUM_RX_THREAD_IDS>>
    getRxThreadIds(Positions pos = {}) const;

    /**************************************************
     *                                                *
     *    File                                        *
     *                                                *
     * ************************************************/
    Result<defs::fileFormat> getFileFormat(Positions pos = {}) const;

    /** default binary, Options: BINARY, HDF5 (library must be compiled with
     * this option) */
    void setFileFormat(defs::fileFormat f, Positions pos = {});

    Result<std::string> getFilePath(Positions pos = {}) const;

    void setFilePath(const std::string &fpath, Positions pos = {});

    Result<std::string> getFileNamePrefix(Positions pos = {}) const;

    /** default run
     * File Name: [file name prefix]_d[module index]_f[file index]_[acquisition
     * index].[file format] eg. run_d0_f0_5.raw
     */
    void setFileNamePrefix(const std::string &fname, Positions pos = {});

    Result<int64_t> getAcquisitionIndex(Positions pos = {}) const;

    void setAcquisitionIndex(int64_t i, Positions pos = {});

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

    /** 0 will set frames per file to unlimited */
    void setFramesPerFile(int n, Positions pos = {});

    /**************************************************
     *                                                *
     *    ZMQ Streaming Parameters (Receiver<->Client)*
     *                                                *
     * ************************************************/
    // TODO callback functions

    Result<bool> getRxZmqDataStream(Positions pos = {}) const;

    void setRxZmqDataStream(bool value, Positions pos = {});

    Result<int> getRxZmqFrequency(Positions pos = {}) const;

    /** freq is nth frame streamed out of receiver.
     * If 0, streaming timer is the timeout,
     * after which current frame sent out. Default is 0 at 200 ms.
     * Default is 1: send every frame.
     * If you want just to see some frames for gui purposes, set to 0 (200ms
     * default timer).
     */
    void setRxZmqFrequency(int freq, Positions pos = {});

    Result<int> getRxZmqTimer(Positions pos = {}) const;

    /**
     * If receiver streaming frequency is 0 (default), then this timer between
     * each data stream is set. Default is 200 ms.
     */
    void setRxZmqTimer(int time_in_ms, Positions pos = {});

    Result<int> getRxZmqStartingFrame(Positions pos = {}) const;

    /**
     * The starting frame index to stream out. 0 by default, which streams 
     * the first frame in an acquisition, and then depending on the rx zmq 
     * frequency/ timer.
     */
    void setRxZmqStartingFrame(int fnum, Positions pos = {});

    Result<int> getRxZmqPort(Positions pos = {}) const;

    /**
     * module_id is -1 for all detectors, ports for each module is calculated
     * (increments) Restarts receiver zmq sockets only if it was already enabled
     */
    void setRxZmqPort(int port, int module_id = -1);

    Result<IpAddr> getRxZmqIP(Positions pos = {}) const;

    void setRxZmqIP(const IpAddr ip, Positions pos = {});

    Result<int> getClientZmqPort(Positions pos = {}) const;

    /**
     * Modified only when using an intermediate process between receiver and
     * gui/client. Module_id is -1 for all detectors, ports for each module is
     * calculated (increments) Restarts client zmq sockets only if it was
     * already enabled
     */
    void setClientZmqPort(int port, int module_id = -1);

    Result<IpAddr> getClientZmqIp(Positions pos = {}) const;

    void setClientZmqIp(const IpAddr ip, Positions pos = {});

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
    Result<std::string> getSettingsPath(Positions pos = {}) const;

    /** [Eiger] */
    void setSettingsPath(const std::string &value, Positions pos = {});

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

    /**[Eiger] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEnergies(Positions pos = {}) const;

    /** [Eiger] Set the energies where the detector is trimmed */
    void setTrimEnergies(std::vector<int> energies, Positions pos = {});

    /** [Eiger] deadtime in ns, 0 = disabled */
    Result<ns> getRateCorrection(Positions pos = {}) const;

    /** [Eiger] Sets default rate correction from trimbit file */
    void setDefaultRateCorrection(Positions pos = {});

    /** //TODO: default, get, set
     * [Eiger] Set Rate correction
     * 0 disable correction, > 0 custom deadtime, cannot be -1
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
    void setActive(const bool active, Positions pos = {});

    /** [Eiger] */
    Result<bool> getRxPadDeactivatedMode(Positions pos = {}) const;

    /** [Eiger] Pad deactivated modules in receiver */
    void setRxPadDeactivatedMode(bool pad, Positions pos = {});

    /** [Eiger] Advanced */
    Result<bool> getPartialReset(Positions pos = {}) const;

    /** [Eiger] Advanced
     * used for pulsing chips */
    void setPartialReset(bool value, Positions pos = {});

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
     * val is value in degrees
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
    Result<bool> getAutoCompDisable(Positions pos = {}) const;

    /** [Jungfrau] Advanced
     * //TODO naming
     * By default, the on-chip gain switching is active during the entire
     * exposure. This mode disables the on-chip gain switching comparator
     * automatically after 93.75% of exposure time (only for longer than 100us).
     */
    void setAutoCompDisable(bool value, Positions pos = {});

    /** [Jungfrau] Advanced TODO naming */
    Result<int> getNumberOfAdditionalStorageCells(Positions pos = {}) const;

    /** [Jungfrau] Advanced */
    void setNumberOfAdditionalStorageCells(int value);

    /** [Jungfrau] Advanced */
    Result<int> getStorageCellStart(Positions pos = {}) const;

    /** [Jungfrau] Advanced. Sets the storage cell storing the first acquisition
     * of the series. Options: 0-15
     */
    void setStorageCellStart(int cell, Positions pos = {});

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
     * module_id is position index
     */
    void setROI(defs::ROI value, int module_id);

    /** [Gotthard] Clear ROI */
    void clearROI(Positions pos = {});

    /** [Gotthard] */
    Result<ns> getExptimeLeft(Positions pos = {}) const;

    /**************************************************
     *                                                *
     *    Gotthard2 Specific                          *
     *                                                *
     * ************************************************/

    /** [Gotthard2] only in burst mode and auto timing mode */
    Result<int64_t> getNumberOfBursts(Positions pos = {}) const;

    /** [Gotthard2] only in burst mode and auto timing mode */
    void setNumberOfBursts(int64_t value);

    /** [Gotthard2] only in burst mode and auto timing mode */
    Result<ns> getBurstPeriod(Positions pos = {}) const;

    /** [Gotthard2] only in burst mode and auto timing mode */
    void setBurstPeriod(ns value, Positions pos = {});

    /** [Gotthard2] offset channel, increment channel */
    Result<std::array<int, 2>> getInjectChannel(Positions pos = {});

    /** [Gotthard2]
     * offsetChannel is starting channel to be injected
     * incrementChannel is determines succeeding channels to be injected */
    void setInjectChannel(const int offsetChannel, const int incrementChannel,
                          Positions pos = {});

    /** [Gotthard2] adu values for each channel */
    Result<std::vector<int>> getVetoPhoton(const int chipIndex,
                                           Positions pos = {});

    /** [Gotthard2] energy in keV */
    void setVetoPhoton(const int chipIndex, const int numPhotons,
                       const int energy, const std::string &fname,
                       Positions pos = {});

    /** [Gotthard2]  */
    void setVetoReference(const int gainIndex, const int value,
                          Positions pos = {});

    /** [Gotthard2]  */
    void setVetoFile(const int chipIndex, const std::string &fname,
                     Positions pos = {});

    /** [Gotthard2]  */
    Result<defs::burstMode> getBurstMode(Positions pos = {});

    /** [Gotthard2]  BURST_OFF, BURST_INTERNAL (default), BURST_EXTERNAL */
    void setBurstMode(defs::burstMode value, Positions pos = {});

    /** [Gotthard2] */
    Result<bool> getCDSGain(Positions pos = {}) const;

    /** default disabled */
    void setCDSGain(bool value, Positions pos = {});

    /** [Gotthard2] */
    Result<int> getFilter(Positions pos = {}) const;

    /** default 0 */
    void setFilter(int value, Positions pos = {});

    /** [Gotthard2] */
    Result<bool> getCurrentSource(Positions pos = {}) const;

    /** default disabled */
    void setCurrentSource(bool value, Positions pos = {});

    /** [Gotthard2] */
    Result<defs::timingSourceType> getTimingSource(Positions pos = {}) const;

    /** [Gotthard2] Options: TIMING_INTERNAL, TIMING_EXTERNAL */
    void setTimingSource(defs::timingSourceType value, Positions pos = {});

    /** [Gotthard2] */
    Result<bool> getVeto(Positions pos = {}) const;

    /** [Gotthard2] */
    void setVeto(const bool enable, Positions pos = {});

    /** [Gotthard2] */
    Result<int> getADCConfiguration(const int chipIndex, const int adcIndex,
                                    Positions pos = {}) const;

    /** [Gotthard2] */
    void setADCConfiguration(const int chipIndex, const int adcIndex,
                             const int value, Positions pos = {});

    /** [Gotthard2] */
    void getBadChannels(const std::string &fname, Positions pos = {}) const;

    /** [Gotthard2] */
    void setBadChannels(const std::string &fname, Positions pos = {});

    /**************************************************
     *                                                *
     *    Mythen3 Specific                            *
     *                                                *
     * ************************************************/
    /** [Mythen3] */
    Result<uint32_t> getCounterMask(Positions pos = {}) const;

    /** [Mythen3] countermask bit set for each counter enabled */
    void setCounterMask(uint32_t countermask, Positions pos = {});

    Result<int> getNumberOfGates(Positions pos = {}) const;

    /** [Mythen3] external gates in gating or trigger_gating mode (external
     * gating) */
    void setNumberOfGates(int value, Positions pos = {});

    /** [Mythen3] exptime for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2 */
    Result<ns> getExptime(int gateIndex, Positions pos = {}) const;

    /** [Mythen3] exptime for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2, -1 for all */
    void setExptime(int gateIndex, ns t, Positions pos = {});

    /** [Mythen3] exptime for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2, -1 for all */
    Result<std::array<ns, 3>> getExptimeForAllGates(Positions pos = {}) const;

    /** [Mythen3] gate delay for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2 */
    Result<ns> getGateDelay(int gateIndex, Positions pos = {}) const;

    /** [Mythen3] gate delay for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2, -1 for all */
    void setGateDelay(int gateIndex, ns t, Positions pos = {});

    /** [Mythen3] gate delay for each gate signal in auto or trigger timing mode
     * (internal gating). Gate index: 0-2, -1 for all */
    Result<std::array<ns, 3>> getGateDelayForAllGates(Positions pos = {}) const;

    /**************************************************
     *                                                *
     *    CTB / Moench Specific                       *
     *                                                *
     * ************************************************/
    /** [CTB][Moench] */
    Result<int> getNumberOfAnalogSamples(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setNumberOfAnalogSamples(int value, Positions pos = {});

    /** [CTB][Moench] */
    Result<int> getADCClock(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setADCClock(int value_in_MHz, Positions pos = {});

    /** [CTB][Moench] */
    Result<int> getRUNClock(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setRUNClock(int value_in_MHz, Positions pos = {});

    /** [CTB][Moench] */
    Result<int> getSYNCClock(Positions pos = {}) const;

    /** [CTB][Moench] */
    Result<int> getADCPipeline(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setADCPipeline(int value, Positions pos = {});

    /** [CTB][Moench] */
    Result<int> getVoltage(defs::dacIndex index, Positions pos = {}) const;

    /**
     * [CTB][Moench] mV
     * [Ctb] Options: V_LIMIT, V_POWER_A, V_POWER_B, V_POWER_C,
     * V_POWER_D, V_POWER_IO, V_POWER_CHIP
     * [Moench] Options: V_LIMIT
     */
    void setVoltage(defs::dacIndex index, int value, Positions pos = {});

    /** [CTB][Moench] */
    Result<uint32_t> getADCEnableMask(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setADCEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB][Moench] */
    Result<uint32_t> getTenGigaADCEnableMask(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setTenGigaADCEnableMask(uint32_t mask, Positions pos = {});

    /**************************************************
     *                                                *
     *    CTB Specific                                *
     *                                                *
     * ************************************************/

    /** [CTB] */
    Result<int> getNumberOfDigitalSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfDigitalSamples(int value, Positions pos = {});

    /** [CTB] */
    Result<defs::readoutMode> getReadoutMode(Positions pos = {}) const;

    /** [CTB] Options: ANALOG_ONLY = 0, DIGITAL_ONLY = 1, ANALOG_AND_DIGITAL */
    void setReadoutMode(defs::readoutMode value, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITClock(Positions pos = {}) const;

    /** [CTB] */
    void setDBITClock(int value_in_MHz, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITPipeline(Positions pos = {}) const;

    /** [CTB] */
    void setDBITPipeline(int value, Positions pos = {});

    /**
     * [CTB] mV
     * Options: V_POWER_A, V_POWER_B, V_POWER_C, V_POWER_D, V_POWER_IO */
    Result<int> getMeasuredVoltage(defs::dacIndex index,
                                   Positions pos = {}) const;

    /**
     * [CTB] mA
     * Options: I_POWER_A, I_POWER_B, I_POWER_C, I_POWER_D, I_POWER_IO  */
    Result<int> getMeasuredCurrent(defs::dacIndex index,
                                   Positions pos = {}) const;

    /** [CTB] Options: SLOW_ADC0 - SLOW_ADC7  in uV */
    Result<int> getSlowADC(defs::dacIndex index, Positions pos = {}) const;

    /** [CTB] */
    Result<int> getExternalSamplingSource(Positions pos = {}) const;

    /** [CTB] Value between 0-63 */
    void setExternalSamplingSource(int value, Positions pos = {});

    /** [CTB] */
    Result<bool> getExternalSampling(Positions pos = {}) const;

    /** [CTB] */
    void setExternalSampling(bool value, Positions pos = {});

    /** [CTB] */
    Result<std::vector<int>> getRxDbitList(Positions pos = {}) const;

    /** [CTB] list contains the set of bits (0-63) to save */
    void setRxDbitList(const std::vector<int> &list, Positions pos = {});

    /** [CTB] */
    Result<int> getRxDbitOffset(Positions pos = {}) const;

    /** [CTB] Set number of bytes of digital data to skip in the Receiver */
    void setRxDbitOffset(int value, Positions pos = {});

    /**
     * [CTB] Set Digital IO Delay
     * cannot get
     * pinMask is IO mask to select the pins
     * delay is delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});

    /** [CTB] */
    Result<bool> getLEDEnable(Positions pos = {}) const;

    /** [CTB] */
    void setLEDEnable(bool enable, Positions pos = {});

    /**************************************************
     *                                                *
     *    Pattern                                     *
     *                                                *
     * ************************************************/

    /** [CTB][Moench][Mythen3] */
    void setPattern(const std::string &fname, Positions pos = {});

    /** [CTB][Moench][Mythen3] */
    void savePattern(const std::string &fname);

    /** [CTB][Moench] */
    Result<uint64_t> getPatternIOControl(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setPatternIOControl(uint64_t word, Positions pos = {});

    /** [CTB][Moench][Mythen3] same as executing for ctb and moench */
    Result<uint64_t> getPatternWord(int addr, Positions pos = {});

    /** [CTB] Caution: If word is  -1  reads the addr (same as
     * executing the pattern)
     * [Mythen3][Moench] */
    void setPatternWord(int addr, uint64_t word, Positions pos = {});

    /**[CTB][Moench][Mythen3] Options: level: -1 (complete pattern) and 0-2
     * levels
     * @returns array of start address and stop address
     */
    Result<std::array<int, 2>>
    getPatternLoopAddresses(int level, Positions pos = {}) const;

    /** [CTB][Moench][Mythen3] Options: level: -1 (complete pattern) and 0-2
     * levels */
    void setPatternLoopAddresses(int level, int start, int stop,
                                 Positions pos = {});

    /**[CTB][Moench][Mythen3] Options: level: -1 (complete pattern) and 0-2
     * levels
     * @returns number of loops
     */
    Result<int> getPatternLoopCycles(int level, Positions pos = {}) const;

    /** [CTB][Moench][Mythen3] n: 0-2, level: -1 (complete pattern) and 0-2
     * levels */
    void setPatternLoopCycles(int level, int n, Positions pos = {});

    /* [CTB][Moench][Mythen3] */
    Result<int> getPatternWaitAddr(int level, Positions pos = {}) const;

    /** [CTB][Moench][Mythen3] Options: level 0-2 */
    void setPatternWaitAddr(int level, int addr, Positions pos = {});

    /** [CTB][Moench][Mythen3]  */
    Result<uint64_t> getPatternWaitTime(int level, Positions pos = {}) const;

    /** [CTB][Moench][Mythen3] Options: level 0-2 */
    void setPatternWaitTime(int level, uint64_t t, Positions pos = {});

    /** [CTB][Moench][Mythen3] */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /** [CTB][Moench][Mythen3] Sets the mask applied to every pattern to the
     * selected bit mask */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** [CTB][Moench][Mythen3]  */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /** [CTB][Moench][Mythen3] Sets the bitmask that the mask will be applied to
     * for every pattern
     */
    void setPatternBitMask(uint64_t mask, Positions pos = {});

    /** [Mythen3] */
    void startPattern(Positions pos = {});

    /**************************************************
     *                                                *
     *    Moench                                      *
     *                                                *
     * ************************************************/

    /** [Moench] */
    Result<std::map<std::string, std::string>>
    getAdditionalJsonHeader(Positions pos = {}) const;

    /** [Moench] If empty, reset additional json header. Max 20 characters for
     * each key/value */
    void setAdditionalJsonHeader(
        const std::map<std::string, std::string> &jsonHeader,
        Positions pos = {});

    /** [Moench] */
    Result<std::string> getAdditionalJsonParameter(const std::string &key,
                                                   Positions pos = {}) const;
    /**
     * [Moench]
     * Sets the value for additional json header parameters if found,
     * else appends the parameter key and value
     * If empty, deletes parameter. Max 20 characters for each key/value
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
    Result<defs::frameModeType> getFrameMode(Positions pos = {}) const;

    /** [Moench] */
    void setFrameMode(defs::frameModeType value, Positions pos = {});

    /** [Moench: -1 if unknown mode] */
    Result<defs::detectorModeType> getDetectorMode(Positions pos = {}) const;

    /** [Moench] */
    void setDetectorMode(defs::detectorModeType value, Positions pos = {});

    /**************************************************
     *                                                *
     *    Advanced                                    *
     *                                                *
     * ************************************************/

    /** [Jungfrau][CTB][Moench] fname is a pof file
     * [Mythen3][Gotthard2] fname is an rbf file
     */
    void programFPGA(const std::string &fname, Positions pos = {});

    /** [Jungfrau][CTB][Moench] */
    void resetFPGA(Positions pos = {});

    /** [Jungfrau][Gotthard][CTB][Moench][Mythen3][Gotthard2]
     * Copy detector server fname from tftp folder of hostname to detector
     * Also changes respawn server, which is effective after a reboot.
     */
    void copyDetectorServer(const std::string &fname,
                            const std::string &hostname, Positions pos = {});

    /** [Jungfrau][Gotthard][CTB][Moench][Mythen3][Gotthard2] */
    void rebootController(Positions pos = {});

    /**
     * [Jungfrau][Gotthard][CTB][Moench]
     * Updates the firmware, detector server and then reboots detector
     * controller blackfin.
     * sname is name of detector server binary found on tftp folder of host
     * pc
     * hostname is name of pc to tftp from
     * fname is programming file name
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &hostname,
                                 const std::string &fname, Positions pos = {});

    Result<uint32_t> readRegister(uint32_t addr, Positions pos = {}) const;

    void writeRegister(uint32_t addr, uint32_t val, Positions pos = {});

    void setBit(uint32_t addr, int bitnr, Positions pos = {});

    void clearBit(uint32_t addr, int bitnr, Positions pos = {});

    /** [Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench] */
    void executeFirmwareTest(Positions pos = {});

    /** [Gotthard][Jungfrau][Mythen3][Gotthard2][CTB][Moench] */
    void executeBusTest(Positions pos = {});

    /** [Gotthard][Jungfrau][CTB][Moench] not possible to read back*/
    void writeAdcRegister(uint32_t addr, uint32_t value, Positions pos = {});

    bool getInitialChecks() const;

    /** initial compaibility and other server start up checks
     * default enabled */
    void setInitialChecks(const bool value);

    /** [CTB][Moench][Jungfrau] */
    Result<uint32_t> getADCInvert(Positions pos = {}) const;

    /** [CTB][Moench][Jungfrau] */
    void setADCInvert(uint32_t value, Positions pos = {});

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
    Result<sls::IpAddr> getLastClientIP(Positions pos = {}) const;

    /** Execute a command on the detector server console */
    Result<std::string> executeCommand(const std::string &value,
                                       Positions pos = {});

    /** [Jungfrau][Mythen3][CTB][Moench]
     * [Gotthard2] only in continuous mode */
    Result<int64_t> getNumberOfFramesFromStart(Positions pos = {}) const;

    /** [Jungfrau][Mythen3][CTB][Moench] Get time from detector start
     * [Gotthard2] only in continuous mode */
    Result<ns> getActualTime(Positions pos = {}) const;

    /** [Jungfrau][Mythen3][CTB][Moench] Get timestamp at a frame start
     * [Gotthard2] only in continuous mode */
    Result<ns> getMeasurementTime(Positions pos = {}) const;

    std::string getUserDetails() const;

    Result<uint64_t> getRxCurrentFrameIndex(Positions pos = {}) const;

  private:
    std::vector<int> getPortNumbers(int start_port);
};

} // namespace sls
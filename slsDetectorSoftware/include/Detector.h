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
     * @param multi_id multi detector shared memory id
     */
    Detector(int multi_id = 0);
    ~Detector();


    /**************************************************
     *                                                *
     *    CONFIGURATIOn                               *
     *                                                *
     * ************************************************/
    /**
     * Frees the shared memory of this detector and all modules
     * belonging to it.
     */
    void freeSharedMemory();
    void setConfig(const std::string &fname);
    Result<std::string> getHostname(Positions pos = {}) const;
    /**
     * Frees shared memory and adds detectors to the list
     * Also updates local detector cache */
    void setHostname(const std::vector<std::string> &value);

    int getMultiId() const;

    void checkDetectorVersionCompatibility(Positions pos = {}) const;

    void checkReceiverVersionCompatibility(Positions pos = {}) const;

    Result<int64_t> getDetectorFirmwareVersion(Positions pos = {}) const;

    Result<int64_t> getDetectorServerVersion(Positions pos = {}) const;

    Result<int64_t> getDetectorSerialNumber(Positions pos = {}) const;

    int64_t getClientSoftwareVersion() const;

    Result<int64_t> getReceiverSoftwareVersion(Positions pos = {}) const;

    /** Get user details of shared memory */
    std::string getUserDetailsFromSharedMemory() const;

    defs::detectorType getDetectorType() const;

    Result<defs::detectorType> getDetectorTypeAsEnum(Positions pos = {}) const;

    Result<std::string> getDetectorTypeAsString(Positions pos = {}) const;

    /** @returns the total number of detectors in the multidetector structure */
    int size() const;

    defs::coordinates getNumberOfDetectors() const;

    Result<defs::coordinates> getNumberOfChannels(Positions pos = {}) const;

     Result<defs::coordinates>
    getNumberOfChannelsInclGapPixels(Positions pos = {}) const;

    defs::coordinates getMaxNumberOfChannels() const;

    /**
     * Sets the maximum number of channels of complete detector in both
     * dimensions. -1 means no limit in this dimension. This value is used to
     * calculate row and column offsets for each module.
     */
    void setMaxNumberOfChannels(const defs::coordinates value);

    /** [Gotthard] */
    Result<defs::coordinates> getDetectorOffsets(Positions pos = {}) const;

    /** [Gotthard] */
    void setDetectorOffsets(defs::coordinates value, Positions pos = {});

    /** [Eiger with specific quad hardware] */
    Result<bool> getQuad(Positions pos = {}) const;

    /** [Eiger with specific quad hardware] */
    void setQuad(const bool enable, Positions pos = {});

    /** [Eiger] */
    Result<int> getReadNLines(Positions pos = {}) const;

   /** [Eiger] Number of lines to read out per half module 
    * Options: 0 - 256. Depending on dynamic range and 
    * 10 GbE enabled, only specific values are accepted
   */
    void setReadNLines(const int value, Positions pos = {});

     Result<int> getControlPort(Positions pos = {}) const;

    /** Detector Control TCP port (for client communication with Detector
     * control server) */
    void setControlPort(int value, Positions pos = {});

    Result<int> getStopPort(Positions pos = {}) const;

    /** Detector Stop TCP port (for client communication with Detector Stop
     * server) */
    void setStopPort(int value, Positions pos = {});

    Result<bool> getLockServer(Positions pos = {}) const;

    /** Lock for detector control server to this client IP */
    void setLockServer(bool value, Positions pos = {});

    /** Get last client IP saved on detector server */
    Result<std::string> getLastClientIP(Positions pos = {}) const;

    void exitServer(Positions pos = {});

    /** Execute a command on the detector server */
    void execCommand(const std::string &value, Positions pos = {});

    void writeConfigurationFile(const std::string &value);

    /** [Not CTB] */
    Result<defs::detectorSettings> getSettings(Positions pos = {}) const;

    /**
     * [Not CTB]
     * Load detector settings from the settings file picked from the
     * trimdir/settingsdir
     * Eiger only stores in shared memory ( a get will
     * overwrite this) For Eiger, one must use setThresholdEnergy
     */
    void setSettings(defs::detectorSettings value, Positions pos = {});

    /** [Eiger] */
    Result<int> getThresholdEnergy(Positions pos = {}) const;

    /**
     * [Eiger]
     * Set threshold energy
     * @param value threshold in eV
     * @param sett ev. change settings
     * @param tb 1 to include trimbits, 0 to exclude
     */
    void setThresholdEnergy(int value,
                            defs::detectorSettings sett = defs::GET_SETTINGS,
                            int tb = 1, Positions pos = {});

    Result<std::string> getSettingsDir(Positions pos = {}) const;

    /** [Not CTB] Sets the detector trimbit/settings directory */
    void setSettingsDir(const std::string &value, Positions pos = {});

    /** [Not CTB] Loads the modules settings/trimbits reading from a specific file
     * file name extension is automatically generated from detector serial number */
    void loadSettingsFile(const std::string &value, Positions pos = {});

    /** [Not CTB] Saves the modules settings/trimbits to a specific file
     * file name extension is automatically generated from detector serial number */
    void saveSettingsFile(const std::string &value, Positions pos = {});

    /** Configures in detector the destination for UDP packets */
    void configureMAC(Positions pos = {});

    Result<int64_t> getNumberOfFrames() const;

    void setNumberOfFrames(int64_t value);

    Result<int64_t> getNumberOfCycles() const;

    void setNumberOfCycles(int64_t value);

    /** [Jungfrau] */
    Result<int64_t> getNumberOfAdditionalStorageCells() const;

    /** [Jungfrau] */
    void setNumberOfStorageCells(int64_t value);

    /** [Jungfrau] */
    Result<int> getStorageCellStart(Positions pos = {}) const;

    /**
     * [Jungfrau] Sets the storage cell storing the first acquisition of the series
     * Options: 0-15
     */
    void setStoragecellStart(int cell, Positions pos = {});

    /** [CTB] */
    Result<int64_t> getNumberOfAnalogSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfAnalogSamples(int64_t value, Positions pos = {});

    /** [CTB] */
    Result<int64_t> getNumberOfDigitalSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfDigitalSamples(int64_t value, Positions pos = {});

    Result<ns> getExptime(Positions pos = {}) const;

    void setExptime(ns t, Positions pos = {});

    Result<ns> getPeriod(Positions pos = {}) const;

    void setPeriod(ns t, Positions pos = {});
   
    /** [Gotthard][Jungfrau] */
    Result<ns> getDelayAfterTrigger(Positions pos = {}) const;

    /** [Gotthard][Jungfrau] */
    void setDelayAfterTrigger(ns value, Positions pos = {});

    /** [Eiger in 32 bit mode] */
    Result<ns> getSubExptime(Positions pos = {}) const;

    /** [Eiger in 32 bit mode] */
    void setSubExptime(ns t, Positions pos = {});

    /** [Eiger in 32 bit mode] */
    Result<ns> getSubDeadTime(Positions pos = {}) const;

    /** [Eiger in 32 bit mode] */
    void setSubDeadTime(ns value, Positions pos = {});

    /** [Jungfrau] */
    Result<ns> getStorageCellDelay(Positions pos = {}) const;

    /** [Jungfrau]
     * Options: (0-1638375 ns (resolution of 25ns) */
    void setStorageCellDelay(ns value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfFramesLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfCyclesLeft(Positions pos = {}) const;
    
    /** [Gotthard] */
    Result<ns> getExptimeLeft(Positions pos = {}) const;

    /** [Gotthard] */
    Result<ns> getPeriodLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    Result<ns> getDelayAfterTriggerLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    Result<int64_t> getNumberOfFramesFromStart(Positions pos = {}) const;

    /** [Jungfrau][CTB] Get time from detector start */
    Result<ns> getActualTime(Positions pos = {}) const;

     /** [Jungfrau][CTB] Get timestamp at a frame start */
    Result<ns> getMeasurementTime(Positions pos = {}) const;

    /** [Eiger] Get measured period between previous two frames */
    Result<ns> getMeasuredPeriod(Positions pos = {}) const;

    /** [Eiger] Get measured sub frame period between previous two frames */
    Result<ns> getMeasuredSubFramePeriod(Positions pos = {}) const;

    /** [Eiger][Jungfrau] */
    Result<int> getSpeed(Positions pos = {}) const;

    /** [Eiger][Jungfrau] 
     * @param value speed (0 full speed, 1 half speed, 2 quarter speed)
     */
    void setSpeed(int value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] */
    Result<int> getADCPhase(bool inDeg, Positions pos = {}) const;

    /** [Gotthard][Jungfrau][CTB] */
    void setADCPhase(int value, bool inDeg, Positions pos = {});

    /** [Jungfrau][CTB] */
    Result<int> getMaxADCPhaseShift(Positions pos = {}) const;

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

    Result<int> getDynamicRange(Positions pos = {}) const;

    /**
     * [Eiger]
     * Options: 4, 8, 16, 32
     * If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1
     */
    void setDynamicRange(int value);

    Result<int> getHighVoltage(Positions pos = {}) const;

    /**
     * [Gotthard Options: 0, 90, 110, 120, 150, 180, 200]
     * [Jungfrau, CTB Options: 0, 60 - 200]
     * [Eiger Options: 0 - 200]
     */
    void setHighVoltage(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getIODelay(Positions pos = {}) const;

    /** [Eiger] */
    void setIODelay(int value, Positions pos = {});

    /**
     * (Degrees)
     * [Gotthard Options: TEMPERATURE_ADC, TEMPERATURE_FPGA]
     * [Jungfrau Options: TEMPERATURE_ADC, TEMPERATURE_FPGA]
     * [Eiger Options: TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE,
     * TEMPERATURE_DCDC, TEMPERATURE_SODL, TEMPERATURE_SODR, TEMPERATURE_FPGA2,
     * TEMPERATURE_FPGA3) (CTB Options: SLOW_ADC_TEMP]
     */
    Result<int> getTemp(defs::dacIndex index, Positions pos = {}) const;

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

    Result<int> getDAC(defs::dacIndex index, bool mV, Positions pos = {}) const;

    void setDAC(int value, defs::dacIndex index, bool mV, Positions pos = {});

    Result<defs::externalCommunicationMode> getTimingMode(Positions pos = {}) const;

    /**
     * [Gotthard, Jungfrau, CTB Options: AUTO_TIMING, TRIGGER_EXPOSURE]
     * [Eiger Options: AUTO_TIMING, TRIGGER_EXPOSURE, GATED, BURST_TRIGGER]
     */
    void setTimingMode(defs::externalCommunicationMode value, Positions pos = {});

    /** [Gotthard] */
    Result<defs::externalSignalFlag> getExternalSignalFlags(Positions pos = {}) const;

    /** [Gotthard] Options: TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE */
    void setExternalSignalFlags(defs::externalSignalFlag value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getParallelMode(Positions pos = {}) const;

    /** [Eiger] */
    void setParallelMode(bool value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getOverFlowMode(Positions pos = {}) const;

    /** [Eiger] */
    void setOverFlowMode(bool value, Positions pos = {});

    /** [CTB] */
    Result<int> getSignalType(Positions pos = {}) const;

    /** [CTB] Options: NORMAL_READOUT = 0, DIGITAL_ONLY = 1, ANALOG_AND_DIGITAL = 2 */
    void setSignalType(int value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getInterruptSubframe(Positions pos = {}) const;

    /** [Eiger] when set,  the last subframe is interrupted at end of acq */
    void setInterruptSubframe(const bool enable, Positions pos = {});

    Result<uint32_t> readRegister(uint32_t addr, Positions pos = {}) const;

    void writeRegister(uint32_t addr, uint32_t val, Positions pos = {});

    void setBit(uint32_t addr, int bitnr, Positions pos = {});

    void clearBit(uint32_t addr, int bitnr, Positions pos = {});

    Result<MacAddr> getDetectorMAC(Positions pos = {}) const;

    void setDetectorMAC(const std::string &detectorMAC, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<MacAddr> getDetectorMAC2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setDetectorMAC2(const std::string &detectorMAC, Positions pos = {});

    Result<IpAddr> getDetectorIP(Positions pos = {}) const;

    void setDetectorIP(const std::string &detectorIP, Positions pos = {});

    /** [Jungfrau] bottom half */
    Result<IpAddr> getDetectorIP2(Positions pos = {}) const;

    /** [Jungfrau] bottom half */
    void setDetectorIP2(const std::string &detectorIP, Positions pos = {});

    Result<std::string> getReceiverHostname(Positions pos = {}) const;

    /**
     * Validates and sets the receiver.
     * Updates local receiver cache parameters
     * Configures the detector to the receiver as UDP destination
     * @param receiver receiver hostname or IP address 
     */
    void setReceiverHostname(const std::string &receiver, Positions pos = {});

    Result<IpAddr> getReceiverUDPIP(Positions pos = {}) const;

    void setReceiverUDPIP(const std::string &udpip, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<IpAddr> getReceiverUDPIP2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setReceiverUDPIP2(const std::string &udpip, Positions pos = {});

    Result<MacAddr> getReceiverUDPMAC(Positions pos = {}) const;

    void setReceiverUDPMAC(const std::string &udpmac, Positions pos = {});

    /** [Jungfrau bottom half] */
    Result<MacAddr> getReceiverUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau bottom half] */
    void setReceiverUDPMAC2(const std::string &udpmac, Positions pos = {});

    Result<int> getReceiverUDPPort(Positions pos = {}) const;
    
    void setReceiverUDPPort(int udpport, Positions pos = {});

    /** [Eiger right port][Jungfrau bottom half] */
    Result<int> getReceiverUDPPort2(Positions pos = {}) const;

    /** [Eiger right port][Jungfrau bottom half] */
    void setReceiverUDPPort2(int udpport, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getNumberofUDPInterfaces(Positions pos = {}) const;
    
    /** [Jungfrau] Also restarts client and receiver sockets */
    void setNumberofUDPInterfaces(int n, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getSelectedUDPInterface(Positions pos = {}) const;
    
    /** 
     * [Jungfrau:
     * Effective only when number of interfaces is 1.
     * Options: 0 (outer, default), 1(inner)] 
     */
    void selectUDPInterface(int interface, Positions pos = {});

    Result<int> getClientStreamingPort(Positions pos = {}) const;
    /** 
     * pos can be for a single module or all modules, not a subset
     * If pos for all modules, ports for each module is calculated (increments)
     * Restarts client zmq sockets
     */
    void setClientDataStreamingInPort(int port, Positions pos = {});
 
    Result<int> getReceiverStreamingPort(Positions pos = {}) const;
    /** 
     * pos can be for a single module or all modules, not a subset
     * If pos for all modules, ports for each module is calculated (increments)
     * Restarts receiver zmq sockets
     */
    void setReceiverDataStreamingOutPort(int port, Positions pos = {});

    Result<std::string> getClientStreamingIP(Positions pos = {}) const;
   
    // TODO these should probably be the same ?? same as what?
    void setClientDataStreamingInIP(const std::string &ip, Positions pos = {});

    Result<std::string> getReceiverStreamingIP(Positions pos = {}) const;
    
    void setReceiverDataStreamingOutIP(const std::string &ip,
                                       Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<bool> getFlowControl10G(Positions pos = {}) const;
   
    /** [Eiger, Jungfrau] */
    void setFlowControl10G(bool enable, Positions pos = {});

    /** [Eiger, Jungfrau] */
    Result<int> getTransmissionDelayFrame(Positions pos = {}) const;
   
    /** 
     * [Jungfrau: Sets the transmission delay of the first UDP packet being streamed out of the module
     * Options: 0 - 31, each value represenets 1 ms ] 
     * [Eiger: Sets the transmission delay of entire frame streamed out for both left and right UDP ports]
     */
    void setTransmissionDelayFrame(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayLeft(Positions pos = {}) const;
    /** 
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the left UDP port
     */
    void setTransmissionDelayLeft(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayRight(Positions pos = {}) const;
    
    /** 
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the right UDP port
     */
    void setTransmissionDelayRight(int value, Positions pos = {});
    
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
    Result<int> getDetectorMinMaxEnergyThreshold(const bool isEmax, Positions pos = {}) const;
    
    /** [Moench] */
    void setDetectorMinMaxEnergyThreshold(const bool isEmax, const int value, Positions pos = {});
    
    /** [Moench: -1 if unknown mode] */
    Result<int> getFrameMode(Positions pos = {}) const;
   
    /** [Moench] */    
    void setFrameMode(defs::frameModeType value, Positions pos = {});
    
    /** [Moench: -1 if unknown mode] */
    Result<int> getDetectorMode(Positions pos = {}) const;    
   
    /** [Moench] */
    void setDetectorMode(defs::detectorModeType value, Positions pos = {});

    /** [Gotthard] */
    Result<int> getDigitalTestBit(Positions pos = {});
    
    /** [Gotthard] */
    Result<int> setDigitalTestBit(const int value, Positions pos = {});
   
    /** [Gotthard][Jungfrau][CTB] */
    Result<int> executeFirmwareTest(Positions pos = {});
    
    /** [Gotthard][Jungfrau][CTB] */
    Result<int> executeBusTest(Positions pos = {});
    
    /** [Gotthard] subset modules not allowed */
    void loadDarkImage(const std::string &fname, Positions pos = {});
    
    /** [Gotthard] subset modules not allowed */
    void loadGainImage(const std::string &fname, Positions pos = {});
   
    /**
     * [Gotthard] subset modules not allowed 
     * @param startACQ if start acq after reading counter
     */
    void getCounterMemoryBlock(const std::string &fname, bool startACQ, Positions pos = {});
   
    /**
     * [Gotthard] 
     * @param startACQ if start acq after resetting counter
     * TODO! does it make sense to call one detector?
     */
    void resetCounterBlock(bool startACQ, Positions pos = {});

    /** [Eiger] */
    Result<bool> getCounterBit(Positions pos = {}) const;
   
    /** [Eiger] If it is set, it resets chips completely (else partially) before an acquisition TODO: if it makes sense */
    void setCounterBit(bool value, Positions pos = {});

    /** [Gotthard, CTB]*/
    Result<std::vector<defs::ROI>> getROI(Positions pos = {}) const;
    
    /** 
     * [Gotthard Options: Only a single chip or all chips, only 1 ROI allowed]
     * [CTB: multiple ROIs allowed] 
     * subset modules not allowed 
     */
    void setROI(std::vector<defs::ROI> value, Positions pos = {});

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
    Result<std::vector<int>> getReceiverDbitList(Positions pos = {}) const;

    /** [CTB] list contains the set of bits (0-63) to save */
    void setReceiverDbitList(std::vector<int> list, Positions pos = {});

    /** [CTB] */
    Result<int> getReceiverDbitOffset(Positions pos = {}) const;

    /** [CTB] Set number of bytes of digital data to skip in the Receiver */
    void setReceiverDbitOffset(int value, Positions pos = {});

    /** [Gotthard][Jungfrau][CTB] not possible to read back*/
    void writeAdcRegister(uint32_t addr, uint32_t value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getActive(Positions pos = {}) const;

    /** [Eiger] */
    void setActive(bool active, Positions pos = {});

    /** [Eiger] */
    Result<bool> getBottom(Positions pos = {}) const;

    /** [Eiger] for gui purposes */
    void setBottom(bool value, Positions pos = {});   

    /** [Eiger] 
     * @returns -1 if they are all different
    */
    Result<int> getAllTrimbits(Positions pos = {}) const;

    /**[Eiger] */
    void setAllTrimbits(int value, Positions pos = {});

    /**[Eiger] */
    Result<bool> getGapPixelsEnable(Positions pos = {}) const;

    /**
     * [Eiger] 
     * 4 bit mode not implemented in Receiver, but in client data call back
     * Fills in gap pixels in data
     */
    void setGapPixelsEnable(bool enable);

    /**[Eiger] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEnergies(Positions pos = {}) const;

    /** [Eiger] Set the energies where the detector is trimmed */
    void setTrimEnergies(std::vector<int> energies, Positions pos = {});

    /**
     * [Eiger] Pulse Pixel n times at x and y coordinates
     */
    void pulsePixel(int n, int x, int y, Positions pos = {});

    /** [Eiger] Pulse Pixel n times and move by a relative value of x and y coordinates */
    void pulsePixelNMove(int n, int x, int y, Positions pos = {});

    /** [Eiger] Pulse chip n times */
    void pulseChip(int n, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getThresholdTemperature(Positions pos = {}) const;

    /**
     * [Jungfrau]Set threshold temperature
     * If temperature crosses threshold temperature
     * and temperature control is enabled, 
     * power to chip will be switched off and 
     * temperature event will be set
     * @param val value in millidegrees TODO! Verify 
     */
    void setThresholdTemperature(int temp, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getTemperatureControl(Positions pos = {}) const;

    /** [Jungfrau] */
    void setTemperatureControl(bool enable, Positions pos = {});

    /** [Jungfrau] */
    Result<int> getTemperatureEvent(Positions pos = {}) const;

    /** [Jungfrau] */
    void ResetTemperatureEvent(Positions pos = {});

    /** [Jungfrau][CTB] */
    void programFPGA(const std::string &fname, Positions pos = {});

    /** [Jungfrau][CTB] */
    void resetFPGA(Positions pos = {});

    /** 
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
     * @param sname name of detector server binary found on tftp folder of host pc
     * @param hostname name of pc to tftp from
     * @param fname programming file name
     * @param pos detector positions
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &hostname,
                                 const std::string &fname, Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getPowerChip(Positions pos = {}) const;

    /** [Jungfrau] */
    void setPowerChip(bool on, Positions pos = {});    

    /** [Jungfrau] */
    Result<bool> getAutoCompDisable(Positions pos = {}) const;

    /** [Jungfrau] TODO??? fix docs ? */
    void setAutoCompDisable(bool value, Positions pos = {});

    /** [Eiger]
     * @returns deadtime in ns, 0 = disabled
     */
    Result<int64_t> getRateCorrection(Positions pos = {}) const;

    /**
     * [Eiger] Set Rate correction
     * 0 disable correction, <0 set to default, >0 deadtime in ns
     */
    void setRateCorrection(int64_t dead_time_ns, Positions pos = {});

    /** [Eiger][Jungfrau] */
    Result<uint64_t> getStartingFrameNumber(Positions pos = {}) const;
   
    /** [Eiger][Jungfrau] */
    void setStartingFrameNumber(uint64_t value, Positions pos);

    /** [Eiger] */
    Result<bool> getTenGigaEnabled(Positions pos = {}) const;
   
    /** [Eiger] */
    void setTenGigaEnabled(bool value, Positions pos = {});

    /** [CTB] */
    Result<bool> getLEDEnable(Positions pos = {}) const;
    
    /** [CTB] */
    void setLEDEnable(bool enable, Positions pos = {});

    /**
     * [CTB] Set Digital IO Delay
     * cannot get
     * @param digital IO mask to select the pins
     * @param delay delay in ps(1 bit=25ps, max of 775 ps)
     */
    void setDigitalIODelay(uint64_t pinMask, int delay, Positions pos = {});

    /**************************************************
     *                                                *
     *    FILE, anything concerning file writing or   *
     *    reading goes here                           *
     *                                                *
     * ************************************************/
    Result<defs::fileFormat> getFileFormat(Positions pos = {}) const;
   
    /** default binary, Options: BINARY, HDF5 (library must be compiled with this option) */
    void setFileFormat(defs::fileFormat f, Positions pos = {});
    
    Result<std::string> getFilePath(Positions pos = {}) const;
    
    void setFilePath(const std::string &fname, Positions pos = {});
    
    Result<std::string> getFileNamePrefix(Positions pos = {}) const;
   
    /** default run */
    void setFileNamePrefix(const std::string &fname, Positions pos = {});

    Result<int> getFileIndex(Positions pos = {}) const;
   
    void setFileIndex(int i, Positions pos = {});
    
    Result<bool> getFileWrite(Positions pos = {}) const;
    
    /** default writes */ 
    
    void setFileWrite(bool value, Positions pos = {});
    
    Result<bool> getMasterFileWrite(Positions pos = {}) const;
    
    void setMasterFileWrite(bool value, Positions pos = {});
    
    Result<bool> getFileOverWrite(Positions pos = {}) const;
    
    /** default overwites */
    void setFileOverWrite(bool value, Positions pos = {});
    
    Result<int> getFramesPerFile(Positions pos = {}) const;
    
    void setFramesPerFile(int n, Positions pos = {});


    /**************************************************
     *                                                *
     *    RECEIVER CONFIG                             *
     *                                                *
     * ************************************************/
    /** true when slsReceiver is used */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    Result<std::string> printReceiverConfiguration(Positions pos = {}) const;

    Result<int> getReceiverPort(Positions pos = {}) const;

    /** Receiver TCP port (for client communication with Receiver)  */
    void setReceiverPort(int value, Positions pos = {});

    Result<bool> getReceiverLock(Positions pos = {});

    /** locks receiver server to client IP */
    void setReceiverLock(bool value, Positions pos = {});

    Result<std::string> getReceiverLastClientIP(Positions pos = {}) const;

    void exitReceiver(Positions pos = {});

    void execReceiverCommand(const std::string &cmd, Positions pos = {});

    Result<int> getReceiverStreamingFrequency(Positions pos = {}) const;
    
    /** @param freq nth frame streamed out of receiver. 
     * If 0, streaming timer is the timeout,
     * after which current frame sent out. Default is 0 at 200 ms.
     * For every frame, set freq to 1.
     */
    void setReceiverStreamingFrequency(int freq, Positions pos = {});

    Result<int> getReceiverStreamingTimer(Positions pos = {}) const;
    
    /**
     * If receiver streaming frequency is 0 (default), then this timer between each
     * data stream is set. Default is 200 ms.
     */
    void setReceiverStreamingTimer(int time_in_ms, Positions pos = {});

    bool getDataStreamingToClient() const;
    
    void setDataStreamingToClient(bool value);
    
    Result<bool> getDataStreamingFromReceiver(Positions pos = {}) const;
    
    void setDataStreamingFromReceiver(bool value, Positions pos = {});

    Result<int> getReceiverFifoDepth(Positions pos = {}) const;
    
    void setReceiverFifoDepth(int nframes, Positions pos = {});
    
    Result<bool> getReceiverSilentMode(Positions pos = {}) const;
    
    void setReceiverSilentMode(bool value, Positions pos = {});

    Result<defs::frameDiscardPolicy>
    getReceiverFrameDiscardPolicy(Positions pos = {}) const;
    
    /** 
     * default NO_DISCARD 
     * Options: NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
     */
    void setReceiverFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                       Positions pos = {});
    Result<bool> getPartialFramesPadding(Positions pos = {}) const;
    
    /** padding enabled */
    void setPartialFramesPadding(bool value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getRxPadDeactivatedMod(Positions pos = {}) const;

    /**
     * [Eiger] Set deactivated Receiver padding mode
     */
    void setRxPadDeactivatedMod(bool pad, Positions pos = {});

    Result<int64_t> getReceiverUDPSocketBufferSize(Positions pos = {}) const;

    void setReceiverUDPSocketBufferSize(int64_t udpsockbufsize,
                                        Positions pos = {});
    Result<int64_t>
    getReceiverRealUDPSocketBufferSize(Positions pos = {}) const;

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

    /**
     * Get the acquiring flag. When true the detector blocks
     * any attempt to start a new acquisition.
     */
    bool getAcquiringFlag() const;

    /**
     * Set the acquiring flag. This might have to done manually
     * after an acquisition was aborted.
     */
    void setAcquiringFlag(bool value);

    Result<defs::runStatus> getRunStatus(Positions pos = {});

    /** Start detector acquisition (Non blocking) */
    void startAcquisition();

    void stopAcquisition();

    /** [Eiger] Sends an internal software trigger to the detector */
    void sendSoftwareTrigger(Positions pos = {});

    /**  Receiver starts listening to UDP packets from detector */
    void startReceiver(Positions pos = {});
    
    /**  Receiver stops listening to UDP packets from detector */
    void stopReceiver(Positions pos = {});
    
    /** Read back the run status of the receiver */
    Result<defs::runStatus> getReceiverStatus(Positions pos = {});

    Result<int> getFramesCaughtByReceiver(Positions pos = {}) const;
    
    Result<uint64_t> getReceiverCurrentFrameIndex(Positions pos = {}) const;
    
    void resetFramesCaught(Positions pos = {});

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

};

} // namespace sls
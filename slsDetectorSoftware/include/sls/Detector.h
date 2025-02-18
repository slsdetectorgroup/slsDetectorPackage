// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Pattern.h"
#include "sls/Result.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"
#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace sls {
using ns = std::chrono::nanoseconds;
class detectorData;
class DetectorImpl;
class MacAddr;
class IpAddr;

// Free function to avoid dependence on class
// and avoid the option to free another objects
// shm by mistake
void freeSharedMemory(const int detectorIndex = 0, const int moduleIndex = -1);

/**
 * \class Detector
 */
class Detector {
    std::unique_ptr<DetectorImpl> pimpl;

  public:
    /** @name Configuration */
    ///@{
    /**************************************************
     *                                                *
     *    Configuration                               *
     *                                                *
     * ************************************************/

    /**
     * @param shm_id detector shared memory id
     * Default value is 0. Can be set to more values for
     * multiple detectors.It is important only if you
     * are controlling multiple detectors from the same pc.
     */
    Detector(int shm_id = 0);
    ~Detector();

    // Disable copy since SharedMemory object is unique in DetectorImpl
    Detector(const Detector &other) = delete;
    Detector &operator=(const Detector &other) = delete;

    // Move constructor and assignment operator
    Detector(Detector &&other) noexcept;
    Detector &operator=(Detector &&other) noexcept;

    /** Frees shared memory before loading configuration file. Set up once
    normally */
    void loadConfig(const std::string &fname);

    /** Shared memory not freed prior. Set up per measurement. */
    void loadParameters(const std::string &fname);

    void loadParameters(const std::vector<std::string> &parameters);

    Result<std::string> getHostname(Positions pos = {}) const;

    /**Frees shared memory, adds detectors to the list. The row and column
     * values in the udp/zmq header are affected by the order in this command
     * and the setDetectorSize function. The modules are stacked row by row
     * until they reach the y-axis limit set by detsize (if specified). Then,
     * stacking continues in the next column and so on. This only affects row
     * and column in udp/zmq header.*/
    void setHostname(const std::vector<std::string> &hostname);

    /** connects to n servers at local host starting at specific control port.
     * Every virtual server will have a stop port (control port + 1) */
    void setVirtualDetectorServers(int numServers, uint16_t startingPort);

    /** Gets shared memory ID */
    int getShmId() const;

    std::string getPackageVersion() const;

    std::string getClientVersion() const;

    Result<int64_t> getFirmwareVersion(Positions pos = {}) const;

    /** [Eiger] Options:  FRONT_LEFT, FRONT_RIGHT */
    Result<int64_t>
    getFrontEndFirmwareVersion(const defs::fpgaPosition fpgaPosition,
                               Positions pos = {}) const;

    Result<std::string> getDetectorServerVersion(Positions pos = {}) const;

    Result<std::string> getHardwareVersion(Positions pos = {}) const;

    Result<std::string> getKernelVersion(Positions pos = {}) const;

    /* [Jungfrau][Moench][Gotthard][Mythen3][Gotthard2][CTB] */
    Result<int64_t> getSerialNumber(Positions pos = {}) const;

    /** [Eiger][Gotthard2][Mythen3][Jungfrau][Moench] 6 bit value (ideally
     * unique) that is streamed out in the UDP header of the detector.*/
    Result<int> getModuleId(Positions pos = {}) const;

    Result<std::string> getReceiverVersion(Positions pos = {}) const;

    /** Options: EIGER, JUNGFRAU, GOTTHARD, MOENCH, MYTHEN3, GOTTHARD2,
     * CHIPTESTBOARD, XILINX_CHIPTESTBOARD */
    Result<defs::detectorType> getDetectorType(Positions pos = {}) const;

    /** Gets the total number of modules in shared memory */
    int size() const;

    bool empty() const;

    defs::xy getModuleGeometry() const;

    Result<defs::xy> getModuleSize(Positions pos = {}) const;

    /** Gets the actual full detector size. It is the same even if ROI changes
     */
    defs::xy getDetectorSize() const;

    /**
     * Sets the detector size in both dimensions (number of channels). \n
     * This value is used to calculate row and column positions for each module
     * and included into udp data packet header. \n By default, it adds modules
     * in y dimension for 2d detectors and in x dimension for 1d detectors.
     */
    void setDetectorSize(const defs::xy value);

    /** list of possible settings for this detector */
    std::vector<defs::detectorSettings> getSettingsList() const;

    /** [Jungfrau][Moench][Gotthard][Gotthard2][Mythen3] */
    Result<defs::detectorSettings> getSettings(Positions pos = {}) const;

    /** [Jungfrau] GAIN0, HIGHGAIN0 \n [Gotthard] DYNAMICGAIN, HIGHGAIN,
     * LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN \n [Gotthard2] DYNAMICGAIN,
     * FIXGAIN1, FIXGAIN2 \n [Mythen3] STANDARD, FAST,
     * HIGHGAIN. Also changes vrshaper and vrpreamp \n [Eiger] Use threshold
     * command. Settings loaded from file found in settingspath \n
     * [Moench] G1_HIGHGAIN, G1_LOWGAIN, G2_HIGHCAP_HIGHGAIN,
     * G2_HIGHCAP_LOWGAIN, G2_LOWCAP_HIGHGAIN, G2_LOWCAP_LOWGAIN, G4_HIGHGAIN,
     * G4_LOWGAIN
     */
    void setSettings(defs::detectorSettings value, Positions pos = {});

    /** [Eiger] */
    Result<int> getThresholdEnergy(Positions pos = {}) const;

    /** Mythen3] threshold energy for the three counters */
    Result<std::array<int, 3>> getAllThresholdEnergy(Positions pos = {}) const;

    /** [Eiger][Mythen3] It loads trim files from settingspath */
    void setThresholdEnergy(int threshold_ev,
                            defs::detectorSettings settings = defs::STANDARD,
                            bool trimbits = true, Positions pos = {});

    /** [Mythen3] It loads trim files from settingspath. An energy of -1 will
     * pick up values from detector */
    void setThresholdEnergy(std::array<int, 3> threshold_ev,
                            defs::detectorSettings settings = defs::STANDARD,
                            bool trimbits = true, Positions pos = {});

    /** [Eiger][Mythen3] */
    Result<std::string> getSettingsPath(Positions pos = {}) const;

    /** [Eiger][Mythen3] Directory where settings files are loaded from/to */
    void setSettingsPath(const std::string &value, Positions pos = {});

    /** [Eiger][Mythen3] If no extension specified, serial number of each module
     * is attached. */
    void loadTrimbits(const std::string &fname, Positions pos = {});

    /** [Eiger][Mythen3] If no extension specified, serial number of each module
     * is attached. */
    void saveTrimbits(const std::string &fname, Positions pos = {});

    /** [Eiger][Mythen3] -1 if they are all different */
    Result<int> getAllTrimbits(Positions pos = {}) const;

    /**[Eiger][Mythen3] */
    void setAllTrimbits(int value, Positions pos = {});

    /**[Eiger][Mythen3] Returns energies in eV where the module is trimmed */
    Result<std::vector<int>> getTrimEnergies(Positions pos = {}) const;

    /** [Eiger][Mythen3] List of trim energies, where corresponding default trim
     * files exist in corresponding trim folders */
    void setTrimEnergies(std::vector<int> energies, Positions pos = {});

    /**[Eiger][Jungfrau][Moench] */
    bool getGapPixelsinCallback() const;

    /**
     * [Eiger][Jungfrau][Moench]
     * Include gap pixels in client data call back. Will not be in detector
     * streaming, receiver file or streaming. Default is disabled.
     */
    void setGapPixelsinCallback(const bool enable);

    /** [Eiger][Jungfrau][Moench] */
    Result<bool> getFlipRows(Positions pos = {}) const;

    /** [Eiger] flips rows paramater sent to slsreceiver to stream as json
     * parameter to flip rows in gui \n[Jungfrau][Moench] flips rows in the
     * detector itself.  For bottom module and number of interfaces must be set
     * to 2. slsReceiver and slsDetectorGui does not handle.slsReceiver and
     * slsDetectorGui does not handle
     */
    void setFlipRows(bool value, Positions pos = {});

    /** [Eiger][Mythen3][Gotthard1][Gotthard2][Jungfrau][Moench] via stop server
     * **/
    Result<bool> getMaster(Positions pos = {}) const;

    /** [Eiger][Gotthard2][Jungfrau][Moench] Set (half) module to master and the
     * other(s) to slaves */
    void setMaster(bool value, int pos);

    /** [Jungfrau][Moench] **/
    Result<bool> getSynchronization(Positions pos = {}) const;

    /** [Jungfrau][Moench]  Sync mode requires at least one master configured.
       Also requires flatband cabling between master and slave with
       termination board.
     */
    void setSynchronization(bool value);

    /** [Gotthard2][Mythen3] */
    void getBadChannels(const std::string &fname, Positions pos = {}) const;

    /** [Gotthard2][Mythen3]
     * [Mythen3] Also does trimming
     */
    void setBadChannels(const std::string &fname, Positions pos = {});

    /** [Gotthard2][Mythen3] */
    Result<std::vector<int>> getBadChannels(Positions pos = {}) const;

    /** [Gotthard2][Mythen3] Empty list resets bad channel list */
    void setBadChannels(const std::vector<int> list, Positions pos = {});

    /** [Gotthard2][Mythen3] Size of list should match number of modules. Each
     * value is at module level and can start at 0. Empty vector resets bad
     * channel list. */
    void setBadChannels(const std::vector<std::vector<int>> list);

    Result<int> getRow(Positions pos = {}) const;

    /** Set it in udp header. Gui uses it to rearrange for complete image */
    void setRow(const int value, Positions pos = {});

    Result<int> getColumn(Positions pos = {}) const;

    /** Set it in udp header. Gui uses it to rearrange for complete image */
    void setColumn(const int value, Positions pos = {});

    Result<bool> isVirtualDetectorServer(Positions pos = {}) const;
    ///@}

    /** @name Callbacks */
    ///@{
    /**************************************************
     *                                                *
     *    Callbacks                                   *
     *                                                *
     * ************************************************/

    /** register callback for end of acquisition
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
    ///@}

    /** @name Acquisition Parameters */
    ///@{
    /**************************************************
     *                                                *
     *    Acquisition Parameters                      *
     *                                                *
     * ************************************************/

    Result<int64_t> getNumberOfFrames(Positions pos = {}) const;

    /** In trigger mode, number of frames per trigger. In scan mode, number of
     * frames is set to number of steps \n [Gotthard2] Burst mode has a maximum
     * of 2720 frames. */
    void setNumberOfFrames(int64_t value);

    Result<int64_t> getNumberOfTriggers(Positions pos = {}) const;

    void setNumberOfTriggers(int64_t value);

    /** [Gotthard][Jungfrau][Moench][Eiger][CTB][Xilinx CTB][Gotthard2]  \n
     * [Mythen3] use function with gate index **/
    Result<ns> getExptime(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][Eiger][CTB][Xilinx CTB][Gotthard2]  \n
     * [Mythen3] sets exptime for all gate signals. To specify gate index, use
     * function with gate index **/
    void setExptime(ns t, Positions pos = {});

    Result<ns> getPeriod(Positions pos = {}) const;

    void setPeriod(ns t, Positions pos = {});

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx CTB] */
    Result<ns> getDelayAfterTrigger(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx CTB] */
    void setDelayAfterTrigger(ns value, Positions pos = {});

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Xilinx CTB]
     * [Gotthard2] only in continuous auto mode */
    Result<int64_t> getNumberOfFramesLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Xilinx CTB]
     * Only when external trigger used */
    Result<int64_t> getNumberOfTriggersLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Gotthard2][Xilinx CTB]
     * [Gotthard2] only in continuous mode */
    Result<ns> getPeriodLeft(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB][Mythen3][Xilinx CTB]
     * [Gotthard2] only in continuous mode */
    Result<ns> getDelayAfterTriggerLeft(Positions pos = {}) const;

    Result<int> getDynamicRange(Positions pos = {}) const;

    /**
     * [Eiger] Options: 4, 8, 12, 16, 32. If i is 32, also sets clkdivider to 2,
     * else sets clkdivider to 1 \n [Mythen3] Options: 8, 16, 32 \n
     * [Jungfrau][Moench][Gotthard][CTB][Mythen3][Gotthard2][Xilinx CTB] 16
     */
    void setDynamicRange(int value);

    /** list of possible dynamic ranges for this detector */
    std::vector<int> getDynamicRangeList() const;

    Result<defs::timingMode> getTimingMode(Positions pos = {}) const;

    /**
     * [Gotthard][Jungfrau][Moench][Gotthard][CTB][Gotthard2][Xilinx CTB]
     * Options: AUTO_TIMING, TRIGGER_EXPOSURE \n [Mythen3] Options: AUTO_TIMING,
     * TRIGGER_EXPOSURE, GATED, TRIGGER_GATED \n [Eiger] Options: AUTO_TIMING,
     * TRIGGER_EXPOSURE, GATED, BURST_TRIGGER
     */
    void setTimingMode(defs::timingMode value, Positions pos = {});

    /** list of possible timing modes for this detector */
    std::vector<defs::timingMode> getTimingModeList() const;

    /** [Eiger][Jungfrau][Moench][Gotthard2][Mythen3] */
    Result<defs::speedLevel> getReadoutSpeed(Positions pos = {}) const;

    /** [Eiger][Jungfrau][Moench][Gotthard2]
     * [Jungfrau][Mythen3] Options: FULL_SPEED, HALF_SPEED (Default),
     * QUARTER_SPEED \n [Moench] Options: FULL_SPEED (Default) \n [Eiger]
     * Options: FULL_SPEED (Default), HALF_SPEED, QUARTER_SPEED \n [Gotthard2]
     * Options: G2_108MHZ (Default), G2_144MHZ \n [Jungfrau][Moench] FULL_SPEED
     * option only available from v2.0 boards and is recommended to set number
     * of interfaces to 2. \n Also overwrites adcphase to recommended default.
     */
    void setReadoutSpeed(defs::speedLevel value, Positions pos = {});

    /** list of possible readoutspeed modes for this detector */
    std::vector<defs::speedLevel> getReadoutSpeedList() const;

    /** [Jungfrau][Moench][CTB] */
    Result<int> getADCPhase(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB]
     * [Jungfrau][Moench] Absolute phase shift. Changing Speed also resets
     * adcphase to recommended defaults. \n [Ctb] Absolute phase shift. Changing
     * adcclk also resets adcphase and sets it to previous values. \n [Gotthard]
     * Relative phase shift
     */
    void setADCPhase(int value, Positions pos = {});

    /** [Jungfrau][Moench][CTB] */
    Result<int> getMaxADCPhaseShift(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB] */
    Result<int> getADCPhaseInDegrees(Positions pos = {}) const;

    /** [Gotthard][Jungfrau][Moench][CTB]
     * [Jungfrau][Moench] Absolute phase shift. Changing Speed also resets
     * adcphase to recommended defaults. \n [Ctb] Absolute phase shift. Changing
     * adcclk also resets adcphase and sets it to previous values. \n [Gotthard]
     * Relative phase shift
     */
    void setADCPhaseInDegrees(int value, Positions pos = {});

    /** [CTB][Jungfrau] */
    Result<int> getDBITPhase(Positions pos = {}) const;

    /** [CTB][Jungfrau] Absolute phase shift \n
     * [CTB] changing dbitclk also resets dbitphase and sets to previous values.
     */
    void setDBITPhase(int value, Positions pos = {});

    /** [CTB][Jungfrau] */
    Result<int> getMaxDBITPhaseShift(Positions pos = {}) const;

    /** [CTB][Jungfrau] */
    Result<int> getDBITPhaseInDegrees(Positions pos = {}) const;

    /** [CTB][Jungfrau] Absolute phase shift \n
     * [CTB] changing dbitclk also resets dbitphase and sets to previous values.
     */
    void setDBITPhaseInDegrees(int value, Positions pos = {});

    /** [Mythen3][Gotthard2] Hz */
    Result<int> getClockFrequency(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockPhase(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] absolute phase shift  \n
     * [Gotthard2] clkIndex: 0-5, [Mythen3] clkIndex 0 only */
    void setClockPhase(int clkIndex, int value, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getMaxClockPhaseShift(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockPhaseinDegrees(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2]  \n
     * [Gotthard2] clkIndex: 0-5, [Mythen3] clkIndex 0 only */
    void setClockPhaseinDegrees(int clkIndex, int value, Positions pos = {});

    /** [Mythen3][Gotthard2] */
    Result<int> getClockDivider(int clkIndex, Positions pos = {});

    /** [Mythen3][Gotthard2] Must be greater than 1. \n
     * [Gotthard2] clkIndex: 0-5, [Mythen3] clkIndex 0 only */
    void setClockDivider(int clkIndex, int value, Positions pos = {});

    Result<int> getHighVoltage(Positions pos = {}) const;

    /**
     * [Gotthard] Options: 0, 90, 110, 120, 150, 180, 200
     * [Jungfrau][Moench][CTB] Options: 0, 60 - 200
     * [Eiger][Mythen3][Gotthard2] Options: 0 - 200
     */
    void setHighVoltage(int value, Positions pos = {});

    /** [Jungfrau][Moench][Mythen3][Gotthard2][Xilinx Ctb] */
    Result<bool> getPowerChip(Positions pos = {}) const;

    /** [Jungfrau][Moench][Mythen3][Gotthard2][Xilinx Ctb] Power the chip. \n
     *  Default is disabled. \n
     * [Jungfrau][Moench] Default is disabled. Get will return power status. Can
     * be off if temperature event occured (temperature over temp_threshold with
     * temp_control enabled. Will configure chip (only chip v1.1)\n
     * [Mythen3][Gotthard2] Default is 1. If module not connected or wrong
     * module, powerchip will fail.\n
     * [Xilinx CTB] Default is 0. Also configures chip if powered on.
     */
    void setPowerChip(bool on, Positions pos = {});

    /** [Gotthard][Eiger virtual] */
    Result<int> getImageTestMode(Positions pos = {});

    /** [Gotthard] If 1, adds channel intensity with precalculated values.
     * Default is 0 \n
     * [Eiger][Jungfrau][Moench] Only for virtual servers, if 1, pixels are
     * saturated. If 0, increasing intensity */
    void setImageTestMode(const int value, Positions pos = {});

    /** gets list of temperature indices for this detector */
    std::vector<defs::dacIndex> getTemperatureList() const;

    /**
     * (Degrees)
     * [Mythen3][Gotthard2][Xilinx Ctb] Options: TEMPERATURE_FPGA
     * [Gotthard] Options: TEMPERATURE_ADC, TEMPERATURE_FPGA \n
     * [Jungfrau][Moench] Options: TEMPERATURE_ADC, TEMPERATURE_FPGA \n
     * [Eiger] Options: TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT, TEMPERATURE_10GE,
     * TEMPERATURE_DCDC, TEMPERATURE_SODL, TEMPERATURE_SODR, TEMPERATURE_FPGA2,
     * TEMPERATURE_FPGA3 \n [CTB] Options: SLOW_ADC_TEMP
     */
    Result<int> getTemperature(defs::dacIndex index, Positions pos = {}) const;

    /** gets list of dac enums for this detector */
    std::vector<defs::dacIndex> getDacList() const;

    /** [Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Mythen3] */
    Result<int> getDefaultDac(defs::dacIndex index, Positions pos = {});

    /** [Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Mythen3] */
    void setDefaultDac(defs::dacIndex index, int defaultValue,
                       Positions pos = {});

    /** [Jungfrau][Moench][Mythen3] */
    Result<int> getDefaultDac(defs::dacIndex index, defs::detectorSettings sett,
                              Positions pos = {});

    /** [Jungfrau][Moench][Mythen3] */
    void setDefaultDac(defs::dacIndex index, int defaultValue,
                       defs::detectorSettings sett, Positions pos = {});

    /** [Eiger][Jungfrau][Moench][Gotthard][Gotthard2][Mythen3]
    reset to defaults, hardReset will reset to hardcoded defaults on on-board
    server */
    void resetToDefaultDacs(const bool hardReset, Positions pos = {});

    Result<int> getDAC(defs::dacIndex index, bool mV = false,
                       Positions pos = {}) const;

    void setDAC(defs::dacIndex index, int value, bool mV = false,
                Positions pos = {});

    /**[Gotthard2] */
    Result<int> getOnChipDAC(defs::dacIndex index, int chipIndex,
                             Positions pos = {}) const;

    /**[Gotthard2] */
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

    /** [Eiger][Mythen3][Gotthard2][Moench] */
    Result<bool> getParallelMode(Positions pos = {}) const;

    /** [Eiger][Mythen3][Gotthard2][Moench]
     * [Mythen3] If exposure time is too short, acquisition will return with an
     * ERROR and take fewer frames than expected \n
     * [Mythen3][Eiger][Moench] Default: Non parallel \n
     * [Gotthard2] Default: Parallel. Non parallel mode works only in continuous
     * mode.*/
    void setParallelMode(bool value, Positions pos = {});

    /** [Gotthard2][Jungfrau] */
    Result<int> getFilterResistor(Positions pos = {}) const;

    /** [Gotthard2][Jungfrau] Set filter resistor. Increasing values for
     * increasing resistance.\n[Gotthard2] Options: [0|1|2|3]. Default is
     * 0.\n[Jungfrau] Options: [0|1]. Default is 1.*/
    void setFilterResistor(int value, Positions pos = {});

    /** [Gotthard2][Jungfrau] */
    Result<defs::currentSrcParameters>
    getCurrentSource(Positions pos = {}) const;

    /** [Gotthard2][Jungfrau] Please refer documentation on
     * currentSrcParameters (sls_detector_defs.h) on the structure and its
     * members */
    void setCurrentSource(defs::currentSrcParameters par, Positions pos = {});

    /** [CTB][Gotthard2] */
    Result<int> getDBITPipeline(Positions pos = {}) const;

    /** [CTB] Options: 0-255 \n [Gotthard2] Options: 0-7 */
    void setDBITPipeline(int value, Positions pos = {});

    /** [Eiger][Jungfrau][Moench] */
    Result<int> getReadNRows(Positions pos = {}) const;

    /** [Eiger] Number of rows to read out per half module
     * Options: 0 - 256. 256 is default. The permissible values depend on
     * dynamic range and 10Gbe enabled. \n[Jungfrau][Moench] Number of rows per
     * module starting from the centre. Options: 8 - 512, must be multiples
     * of 8. Default is 512.
     */
    void setReadNRows(const int lines, Positions pos = {});

    ///@}

    /** @name Acquisition */
    ///@{
    /**************************************************
     *                                                *
     *    Acquisition                                 *
     *                                                *
     * ************************************************/
    /**
     * Blocking call: Acquire the number of frames set
     * - sets acquiring flag
     * - starts the receiver listener (if enabled)
     * - starts detector acquisition for number of frames set
     * - monitors detector status from running to idle
     * - stops the receiver listener (if enabled)
     * - increments file index if file write enabled
     * - resets acquiring flag
     * Control server is blocked and cannot accept other commands until
     * acquisition is done.
     */
    void acquire();

    /** If acquisition aborted during blocking acquire, use this to clear
     * acquiring flag in shared memory before starting next acquisition */
    void clearAcquiringFlag();

    /** Non Blocking: Start receiver listener and create data file if file write
     * enabled */
    void startReceiver();

    /** Non Blocking: Stops receiver listener for detector data packets and
       closes current data file (if file write enabled). */
    void stopReceiver();

    /** Non blocking: start detector acquisition. Status changes to RUNNING or
     * WAITING and automatically returns to idle at the end of acquisition.
     [Mythen3] Master starts acquisition first */
    void startDetector(Positions pos = {});

    /** [Mythen3] Non blocking: start detector readout of counters in chip.
     * Status changes to TRANSMITTING and automatically returns to idle at the
     * end of readout.
     [Eiger] Master stops acquisition last */
    void startDetectorReadout();

    /** Non blocking: Abort detector acquisition. Status changes to IDLE or
     * STOPPED. Goes to stop server. */
    void stopDetector(Positions pos = {});

    /** IDLE, ERROR, WAITING, RUN_FINISHED, TRANSMITTING, RUNNING, STOPPED \n
     * Goes to stop server */
    Result<defs::runStatus> getDetectorStatus(Positions pos = {}) const;

    /** Options: IDLE, TRANSMITTING, RUNNING */
    Result<defs::runStatus> getReceiverStatus(Positions pos = {}) const;

    /** Gets the number of frames caught for each port in receiver. */
    Result<std::vector<int64_t>> getFramesCaught(Positions pos = {}) const;

    /** Gets the number of missing packets for each port in receiver. Negative
     * number denotes extra packets. */
    Result<std::vector<int64_t>> getNumMissingPackets(Positions pos = {}) const;

    /** Gets frame index for each port in receiver. */
    Result<std::vector<int64_t>>
    getRxCurrentFrameIndex(Positions pos = {}) const;

    /** [Eiger][Jungfrau][Moench][CTB][Xilinx CTB][Gotthard2] */
    Result<uint64_t> getNextFrameNumber(Positions pos = {}) const;

    /** [Eiger][Jungfrau][Moench][CTB][Xilinx CTB][Gotthard2] Stopping
     * acquisition might result in different frame numbers for different
     * modules. So, after stopping, next frame number (max + 1) is set for all
     * the modules afterwards.*/
    void setNextFrameNumber(uint64_t value, Positions pos = {});

    /** [Eiger][Mythen3][Jungfrau][Moench] Sends an internal software trigger to
     * the detector block true if command blocks till frames are sent out from
     * that trigger [Eiger][Jungfrau][Moench] Block can be true
     */
    void sendSoftwareTrigger(const bool block = false, Positions pos = {});

    Result<defs::scanParameters> getScan(Positions pos = {}) const;

    /** enables/ disables scans for dac and trimbits \n
     * Enabling scan sets number of frames to number of steps in
     * receiver. \n To cancel scan configuration, set dac to '0', which also
     * sets number of frames to 1 \n [Eiger/ Mythen3] Trimbits using
     * TRIMBIT_SCAN*/
    void setScan(const defs::scanParameters t);

    /** Gets Scan error message if scan ended in error for non blocking
     * acquisitions.*/
    Result<std::string> getScanErrorMessage(Positions pos = {}) const;
    ///@}

    /** @name Network Configuration (Detector<->Receiver) */
    ///@{
    /**************************************************
     *                                                 *
     *    Network Configuration (Detector<->Receiver)  *
     *                                                 *
     * ************************************************/

    /** [Jungfrau][Moench][Gotthard2][Eiger] */
    Result<int> getNumberofUDPInterfaces(Positions pos = {}) const;

    /** [Jungfrau][Moench][Gotthard2]  Number of udp interfaces to stream data
     * from detector. Default is 1. \n Also enables second interface in receiver
     * for listening (Writes a file per interface if writing enabled). \n Also
     * restarts client and receiver zmq sockets if zmq streaming enabled. \n
     * [Gotthard2] second interface enabled to send veto information via 10Gbps
     * for debugging. By default, if veto enabled, it is sent via 2.5 gbps
     * interface. */
    void setNumberofUDPInterfaces(int n, Positions pos = {});

    /** [Jungfrau][Moench] */
    Result<int> getSelectedUDPInterface(Positions pos = {}) const;

    /**
     * [Jungfrau][Moench]
     * Effective only when number of interfaces is 1.
     * Options: 0 (outer, default), 1(inner)] //TODO: enum?
     */
    void selectUDPInterface(int interface, Positions pos = {});

    Result<IpAddr> getSourceUDPIP(Positions pos = {}) const;

    /**For Eiger 1G, the detector will replace with its own DHCP IP
     * 10G Eiger and other detectors. The source UDP IP must be in the
     * same subnet of the destination UDP IP
     */
    void setSourceUDPIP(const IpAddr ip, Positions pos = {});

    /** [Jungfrau][Moench] bottom half [Gotthard2] veto debugging */
    Result<IpAddr> getSourceUDPIP2(Positions pos = {}) const;

    /** [Jungfrau][Moench] bottom half [Gotthard2] veto debugging. \n The source
     * UDP IP must be in the same subnet of the destination UDP IP2 */
    void setSourceUDPIP2(const IpAddr ip, Positions pos = {});

    Result<MacAddr> getSourceUDPMAC(Positions pos = {}) const;

    /**For Eiger 1G, the detector will replace with its own DHCP MAC
     * For Eiger 10G, the detector will replace with its own DHCP MAC + 1
     * Others can be anything (beware of certain bits)
     */
    void setSourceUDPMAC(const MacAddr mac, Positions pos = {});

    /** [Jungfrau][Moench] bottom half [Gotthard2] veto debugging */
    Result<MacAddr> getSourceUDPMAC2(Positions pos = {}) const;

    /** [Jungfrau][Moench] bottom half [Gotthard2] veto debugging */
    void setSourceUDPMAC2(const MacAddr mac, Positions pos = {});

    Result<UdpDestination> getDestinationUDPList(const uint32_t entry,
                                                 Positions pos = {}) const;

    void setDestinationUDPList(const UdpDestination, const int module_id);

    /** [Jungfrau][Moench][Eiger][Mythen3][Gotthard2] */
    Result<int> getNumberofUDPDestinations(Positions pos = {}) const;

    void clearUDPDestinations(Positions pos = {});

    /** [Jungfrau][Moench][Mythen3][Gotthard2] */
    Result<int> getFirstUDPDestination(Positions pos = {}) const;

    /**[Jungfrau][Moench][Gotthard2] Options 0-31 (or number of udp
     * destinations)\n [Mythen3] Options 0-63 (or number of udp destinations)
     */
    void setFirstUDPDestination(const int value, Positions pos = {});

    Result<IpAddr> getDestinationUDPIP(Positions pos = {}) const;

    /** IP of the interface in receiver that the detector sends data to */
    void setDestinationUDPIP(const IpAddr ip, Positions pos = {});

    /** [Jungfrau][Moench] bottom half \n [Gotthard2] veto debugging */
    Result<IpAddr> getDestinationUDPIP2(Positions pos = {}) const;

    /** [Jungfrau][Moench] bottom half \n [Gotthard2] veto debugging */
    void setDestinationUDPIP2(const IpAddr ip, Positions pos = {});

    Result<MacAddr> getDestinationUDPMAC(Positions pos = {}) const;

    /** Mac address of the receiver (destination) udp interface. Not mandatory
     * to set as setDestinationUDPIP (udp_dstip) retrieves it from slsReceiver
     * process but must be set if you use a custom receiver (not slsReceiver).\n
     * Use router mac address if router in between detector and receiver.
     */
    void setDestinationUDPMAC(const MacAddr mac, Positions pos = {});

    /** [Jungfrau][Moench] bottom half \n [Gotthard2] veto debugging */
    Result<MacAddr> getDestinationUDPMAC2(Positions pos = {}) const;

    /* [Jungfrau][Moench][Gotthard2] Mac address of the receiver (destination)
     * udp interface 2. \n Not mandatory to set as udp_dstip2 retrieves it from
     * slsReceiver process but must be set if you use a custom receiver (not
     * slsReceiver). \n [Jungfrau][Moench] bottom half \n [Gotthard2] veto
     * debugging \n Use router mac address if router in between detector and
     * receiver.
     */
    void setDestinationUDPMAC2(const MacAddr mac, Positions pos = {});

    Result<uint16_t> getDestinationUDPPort(Positions pos = {}) const;

    /** Default is 50001. \n If module_id is -1, ports for each module is
     * calculated (incremented by 1 if no 2nd interface) */
    void setDestinationUDPPort(uint16_t port, int module_id = -1);

    /** [Eiger] right port[Jungfrau][Moench] bottom half [Gotthard2] veto
     * debugging */
    Result<uint16_t> getDestinationUDPPort2(Positions pos = {}) const;

    /** [Eiger] right port[Jungfrau][Moench] bottom half [Gotthard2] veto
     * debugging \n Default is 50002. \n If module_id is -1, ports for each
     * module is calculated (incremented by 1 if no 2nd interface)*/
    void setDestinationUDPPort2(uint16_t port, int module_id = -1);

    /** Reconfigures Detector with UDP destination. More for debugging as the
     * configuration is done automatically when the detector has sufficient UDP
     * details. */
    void reconfigureUDPDestination(Positions pos = {});

    /** Validates that UDP configuration in the detector is valid. If not
     * configured, it will throw with error message requesting missing udp
     * information */
    void validateUDPConfiguration(Positions pos = {});

    Result<std::string> printRxConfiguration(Positions pos = {}) const;

    /** [Eiger][CTB][Mythen3] */
    Result<bool> getTenGiga(Positions pos = {}) const;

    /** [Eiger][CTB][Mythen3] */
    void setTenGiga(bool value, Positions pos = {});

    /** [Eiger][Jungfrau][Moench] */
    Result<bool> getTenGigaFlowControl(Positions pos = {}) const;

    /** [Eiger][Jungfrau][Moench] */
    void setTenGigaFlowControl(bool enable, Positions pos = {});

    /** [Eiger][Jungfrau][Moench][Mythen3] */
    Result<int> getTransmissionDelayFrame(Positions pos = {}) const;

    /**
     * Eiger][Jungfrau][Moench][Mythen3] Transmission delay of first udp packet
     * being streamed out of the module.\n[Jungfrau][Moench] [0-31] Each value
     * represents 1 ms\n[Eiger] Additional delay to txndelay_left and
     * txndelay_right. Each value represents 10ns. Typical value is
     * 50000.\n[Mythen3] [0-16777215] Each value represents 8 ns (125 MHz
     * clock), max is 134 ms.
     */
    void setTransmissionDelayFrame(int value, Positions pos = {});

    /** [Eiger] */
    Result<int> getTransmissionDelayLeft(Positions pos = {}) const;

    /**[Eiger] Transmission delay of first packet in an image being streamed out
     * of the module's left UDP port. Each value represents 10ns. Typical value
     * is 50000.
     */
    void setTransmissionDelayLeft(int value, Positions pos = {});

    /** [Eiger] Transmission delay of first packet in an image being streamed
     * out of the module's right UDP port. Each value represents 10ns. Typical
     * value is 50000. */
    Result<int> getTransmissionDelayRight(Positions pos = {}) const;

    /**
     * [Eiger]
     * Sets the transmission delay of first packet streamed ut of the right UDP
     * port
     */
    void setTransmissionDelayRight(int value, Positions pos = {});

    /** [Eiger][Jungfrau][Moench] */
    int getTransmissionDelay() const;

    /**
     * [Eiger][Jungfrau][Moench][Mythen3] Set transmission delay for all modules
     * in the detector using the step size provided.Sets up \n\t\t[Eiger]
     * txdelay_left to (2 * mod_index * n_delay), \n\t\t[Eiger] txdelay_right to
     * ((2 * mod_index + 1) * n_delay) and \n\t\t[Eiger] txdelay_frame to (2
     * *num_modules * n_delay) \n\t\t[Jungfrau][Moench][Mythen3] txdelay_frame
     * to (num_modules * n_delay) \nfor every module.
     */
    void setTransmissionDelay(int step);

    ///@}

    /** @name Receiver Configuration */
    ///@{
    /**************************************************
     *                                                *
     *    Receiver Configuration                      *
     *                                                *
     * ************************************************/

    /** true when slsReceiver is used */
    Result<bool> getUseReceiverFlag(Positions pos = {}) const;

    Result<std::string> getRxHostname(Positions pos = {}) const;

    /**
     * Sets receiver hostname or IP address for each module. \n Used for TCP
     * control communication between client and receiver to configure receiver.
     * Also updates receiver with detector parameters. \n Also resets any prior
     * receiver property (not on detector). \n receiver is receiver hostname or
     * IP address, can include tcp port eg. hostname:port
     */
    void setRxHostname(const std::string &receiver, Positions pos = {});

    /** multiple rx hostnames. Single element will set it for all */
    void setRxHostname(const std::vector<std::string> &name);

    Result<uint16_t> getRxPort(Positions pos = {}) const;

    /** TCP port for client-receiver communication. \n
     *  Default is 1954. \n  Must be different if multiple receivers on same pc.
     * \n Must be first command to set a receiver parameter to be able to
     * communicate. \n Multi command will automatically increment port for
     * individual modules.*/
    void setRxPort(uint16_t port, int module_id = -1);

    Result<int> getRxFifoDepth(Positions pos = {}) const;

    /** Number of frames in fifo between udp listening and processing threads */
    void setRxFifoDepth(int nframes, Positions pos = {});

    Result<bool> getRxSilentMode(Positions pos = {}) const;

    /** Switch on or off receiver text output during acquisition */
    void setRxSilentMode(bool value, Positions pos = {});

    Result<defs::frameDiscardPolicy>
    getRxFrameDiscardPolicy(Positions pos = {}) const;

    /**
     * Options: NO_DISCARD, DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES
     * Default: NO_DISCARD
     * discard partial frames is the fastest
     */
    void setRxFrameDiscardPolicy(defs::frameDiscardPolicy f,
                                 Positions pos = {});

    Result<bool> getPartialFramesPadding(Positions pos = {}) const;

    /** Default: padding enabled. Disabling padding is the fastest */
    void setPartialFramesPadding(bool value, Positions pos = {});

    Result<int> getRxUDPSocketBufferSize(Positions pos = {}) const;

    /** UDP socket buffer size in receiver. Tune rmem_default and rmem_max
     * accordingly. Max value is INT_MAX/2. */
    void setRxUDPSocketBufferSize(int udpsockbufsize, Positions pos = {});

    /** TODO:
     * Gets actual udp socket buffer size. Double the size of rx_udpsocksize due
     * to kernel bookkeeping.
     */
    Result<int> getRxRealUDPSocketBufferSize(Positions pos = {}) const;

    Result<bool> getRxLock(Positions pos = {});

    /** Lock receiver to one client IP, 1 locks, 0 unlocks. Default is unlocked.
     */
    void setRxLock(bool value, Positions pos = {});

    /** Client IP Address that last communicated with the receiver */
    Result<IpAddr> getRxLastClientIP(Positions pos = {}) const;

    /** Get kernel thread ids from the receiver in order of [parent, tcp,
     * listener 0, processor 0, streamer 0, listener 1, processor 1, streamer 1,
     * arping]. If no streamer yet or there is no second interface, it gives 0
     * in its place.
     */
    Result<std::array<pid_t, NUM_RX_THREAD_IDS>>
    getRxThreadIds(Positions pos = {}) const;

    Result<bool> getRxArping(Positions pos = {}) const;

    /** Starts a thread in slsReceiver to arping the interface it is listening
     * every minute. Useful in 10G mode. */
    void setRxArping(bool value, Positions pos = {});

    /** at module level */
    Result<defs::ROI> getIndividualRxROIs(Positions pos) const;

    defs::ROI getRxROI() const;

    /** only at multi module level without gap pixels */
    void setRxROI(const defs::ROI value);

    void clearRxROI();

    ///@}

    /** @name File */
    ///@{
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

    /** Default is "/". If path does not exist and fwrite enabled, it will try
     * to create it at start of acquisition. */
    void setFilePath(const std::string &fpath, Positions pos = {});

    Result<std::string> getFileNamePrefix(Positions pos = {}) const;

    /** default run
     * File Name: [file name prefix]_d[module index]_f[file index]_[acquisition
     * index].[file format] eg. run_d0_f0_5.raw
     */
    void setFileNamePrefix(const std::string &fname, Positions pos = {});

    Result<int64_t> getAcquisitionIndex(Positions pos = {}) const;

    /** file or Acquisition index in receiver \n
     * File name: [file name prefix]_d[detector index]_f[sub file
     * index]_[acquisition/file index].[raw/h5].
     */
    void setAcquisitionIndex(int64_t i, Positions pos = {});

    Result<bool> getFileWrite(Positions pos = {}) const;

    /** default disabled */
    void setFileWrite(bool value, Positions pos = {});

    bool getMasterFileWrite() const;

    /**default enabled */
    void setMasterFileWrite(bool value);

    Result<bool> getFileOverWrite(Positions pos = {}) const;

    /** default overwites */
    void setFileOverWrite(bool value, Positions pos = {});

    Result<int> getFramesPerFile(Positions pos = {}) const;

    /** Default depends on detector type. \n 0 will set frames per file in an
     * acquisition to unlimited */
    void setFramesPerFile(int n, Positions pos = {});
    ///@}

    /** @name ZMQ Streaming Parameters (Receiver<->Client) */
    ///@{
    /**************************************************
     *                                                *
     *    ZMQ Streaming Parameters (Receiver<->Client)*
     *                                                *
     * ************************************************/
    // TODO callback functions

    Result<bool> getRxZmqDataStream(Positions pos = {}) const;

    /** Enable/ disable data streaming from receiver via zmq (eg. to GUI or to
     * another process for further processing). \n This creates/ destroys zmq
     * streamer threads in receiver. \n Switching to Gui automatically enables
     * data streaming in receiver. \n Switching back to command line or API
     * acquire will require disabling data streaming in receiver for fast
     * applications (if not needed for client data call backs).
     */
    void setRxZmqDataStream(bool value, Positions pos = {});

    Result<int> getRxZmqFrequency(Positions pos = {}) const;

    /** Frequency of frames streamed out from receiver via zmq. \n Default: 1,
     * Means every frame is streamed out. \n If 2, every second frame is
     * streamed out. \n If 0, streaming timer is the timeout, after which
     * current frame is sent out. (default timeout is 500 ms). Usually used for
     * gui purposes.
     */
    void setRxZmqFrequency(int freq, Positions pos = {});

    Result<int> getRxZmqTimer(Positions pos = {}) const;

    /**
     * If receiver streaming frequency is 0 (default), then this timer between
     * each data stream is set. Default is 500 ms.
     */
    void setRxZmqTimer(int time_in_ms, Positions pos = {});

    Result<int> getRxZmqStartingFrame(Positions pos = {}) const;

    /**
     * The starting frame index to stream out. 0 by default, which streams
     * the first frame in an acquisition, and then depending on the rx zmq
     * frequency/ timer.
     */
    void setRxZmqStartingFrame(int fnum, Positions pos = {});

    Result<uint16_t> getRxZmqPort(Positions pos = {}) const;

    /** Zmq port for data to be streamed out of the receiver. \n
     * Also restarts receiver zmq streaming if enabled. \n Default is 30001. \n
     * Must be different for every detector (and udp port). \n module_id is -1
     * for all detectors, ports for each module is calculated (increment by 1 if
     * no 2nd interface). \n Restarts receiver zmq sockets only if it was
     * already enabled
     */
    void setRxZmqPort(uint16_t port, int module_id = -1);

    Result<uint16_t> getClientZmqPort(Positions pos = {}) const;

    /** Port number to listen to zmq data streamed out from receiver or
     * intermediate process. \n Must be different for every detector (and udp
     * port). \n Module_id is -1 for all detectors, ports for each module is
     * calculated (increment by 1 if no 2nd interface). \n Restarts client zmq
     * sockets only if it was already enabled \n Default connects to receiver
     * zmq streaming out port (30001).
     */
    void setClientZmqPort(uint16_t port, int module_id = -1);

    Result<IpAddr> getClientZmqIp(Positions pos = {}) const;

    /** Ip Address to listen to zmq data streamed out from receiver or
     * intermediate process. \n Default connects to receiver zmq Ip Address
     * (from rx_hostname). \n Modified only when using an intermediate process
     * between receiver and client(gui). \n Also restarts client zmq streaming
     * if enabled.
     */
    void setClientZmqIp(const IpAddr ip, Positions pos = {});

    int getClientZmqHwm() const;

    /** Client's zmq receive high water mark. \n Default is the zmq library's
     * default (1000), can also be set here using -1. \n This is a high number
     * and can be set to 2 for gui purposes. \n One must also set the receiver's
     * send high water mark to similar value. Final effect is sum of them.
     */
    void setClientZmqHwm(const int limit);

    Result<int> getRxZmqHwm(Positions pos = {}) const;

    /** Receiver's zmq send high water mark. \n Default is the zmq library's
     * default (1000) \n This is a high number and can be set to 2 for gui
     * purposes. \n One must also set the client's receive high water mark to
     * similar value. Final effect is sum of them. Also restarts receiver zmq
     * streaming if enabled. \n Can set to -1 to set default.
     */
    void setRxZmqHwm(const int limit);

    ///@}

    /** @name Eiger Specific */
    ///@{
    /**************************************************
     *                                                *
     *    Eiger Specific                              *
     *                                                *
     * ************************************************/

    /** [Eiger] in 32 bit mode */
    Result<ns> getSubExptime(Positions pos = {}) const;

    /** [Eiger] in 32 bit mode */
    void setSubExptime(ns t, Positions pos = {});

    /** [Eiger] in 32 bit mode */
    Result<ns> getSubDeadTime(Positions pos = {}) const;

    /** [Eiger] in 32 bit mode */
    void setSubDeadTime(ns value, Positions pos = {});

    /** [Eiger] */
    Result<bool> getOverFlowMode(Positions pos = {}) const;

    /** [Eiger] Overflow in 32 bit mode. Default is disabled.*/
    void setOverFlowMode(bool value, Positions pos = {});

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
    Result<bool> getInterruptSubframe(Positions pos = {}) const;

    /** [Eiger] Enable last subframe interrupt at required exposure time.
     * Disabling will wait for last sub frame to finish exposing. Default is
     * disabled. */
    void setInterruptSubframe(const bool enable, Positions pos = {});

    /** [Eiger] minimum two frames */
    Result<ns> getMeasuredPeriod(Positions pos = {}) const;

    /** [Eiger] */
    Result<ns> getMeasuredSubFramePeriod(Positions pos = {}) const;

    /** [Eiger] */
    Result<bool> getActive(Positions pos = {}) const;

    /** [Eiger] activated by default at hostname command. Deactivated does not
     * send data or communicated with FEB or BEB */
    void setActive(const bool active, Positions pos = {});

    /** [Eiger] Advanced */
    Result<bool> getPartialReset(Positions pos = {}) const;

    /** [Eiger] Advanced used for pulsing chips. Default is Complete reset */
    void setPartialReset(bool value, Positions pos = {});

    /** [Eiger] Advanced
     * Pulse Pixel n times at x and y coordinates */
    void pulsePixel(int n, defs::xy pixel, Positions pos = {});

    /** [Eiger] Advanced
     * Pulse Pixel n times and move by a relative value of x and y
     * coordinates */
    void pulsePixelNMove(int n, defs::xy pixel, Positions pos = {});

    /** [Eiger] Advanced
     * Pulse chip n times. \n
     * If n is -1, resets to normal mode (reset chip completely at start of
     * acquisition, where partialreset = 0).  */
    void pulseChip(int n, Positions pos = {});

    /** [Eiger] with specific quad hardware */
    Result<bool> getQuad(Positions pos = {}) const;

    /** [Eiger] Sets detector size to a quad. 0 (disabled) is default. (Specific
     * hardware required). */
    void setQuad(const bool enable);

    /** [Eiger] */
    Result<bool> getDataStream(const defs::portPosition port,
                               Positions pos = {}) const;

    /** [Eiger] enable or disable data streaming from left or right of detector
     * for 10GbE. Default: enabled
     */
    void setDataStream(const defs::portPosition port, const bool enable,
                       Positions pos = {});

    /** [Eiger] Advanced */
    Result<bool> getTop(Positions pos = {}) const;

    /** [Eiger] Advanced. Default is hardware default */
    void setTop(bool value, Positions pos = {});

    ///@}

    /** @name Jungfrau/Moench Specific */
    ///@{
    /**************************************************
     *                                                *
     *    Jungfrau/Moench Specific                           *
     *                                                *
     * ************************************************/

    /** [Jungfrau] */
    Result<double> getChipVersion(Positions pos = {}) const;

    /** [Jungfrau][Moench] */
    Result<int> getThresholdTemperature(Positions pos = {}) const;

    /**
     * [Jungfrau][Moench]Set threshold temperature in degrees.
     * If temperature crosses threshold temperature
     * and temperature control is enabled (default is disabled), power to chip
     * will be switched off and temperature event will be set. \n To power on
     * chip again, temperature has to be less than threshold temperature and
     * temperature event has to be cleared.
     */
    void setThresholdTemperature(int temp, Positions pos = {});

    /** [Jungfrau][Moench] */
    Result<bool> getTemperatureControl(Positions pos = {}) const;

    /** [Jungfrau][Moench]  refer to setThresholdTemperature
     * Default is disabled */
    void setTemperatureControl(bool enable, Positions pos = {});

    /** [Jungfrau][Moench] refer to setThresdholdTemperature */
    Result<int> getTemperatureEvent(Positions pos = {}) const;

    /** [Jungfrau][Moench] refer to setThresdholdTemperature */
    void resetTemperatureEvent(Positions pos = {});

    /** [Jungfrau] */
    Result<bool> getAutoComparatorDisable(Positions pos = {}) const;

    /** [Jungfrau] Advanced
     * //TODO naming
     * By default, the on-chip gain switching is active during the
     * entire exposure. This mode disables the on-chip gain switching comparator
     * automatically and the duration is set using setComparatorDisableTime\n
     * Default is false or this mode disabled(comparator enabled throughout).
     * true enables mode. 0 disables mode.
     */
    void setAutoComparatorDisable(bool value, Positions pos = {});

    /** [Jungfrau] */
    Result<ns> getComparatorDisableTime(Positions pos = {}) const;

    /** [Jungfrau] Time before end of exposure when comparator is
     * disabled.*/
    void setComparatorDisableTime(ns t, Positions pos = {});

    /** [Jungfrau] Advanced TODO naming */
    Result<int> getNumberOfAdditionalStorageCells(Positions pos = {}) const;

    /** [Jungfrau] Advanced \n
     * Only for chipv1.0. Options: 0 - 15. Default: 0. \n
     * The #images = #frames x #triggers x (#storagecells + 1) */
    void setNumberOfAdditionalStorageCells(int value);

    /** [Jungfrau] Advanced */
    Result<int> getStorageCellStart(Positions pos = {}) const;

    /** [Jungfrau] Advanced. Sets the storage cell storing the first
     * acquisition of the series. Options: 0-max. max is 15 (default) for
     * chipv1.0 and 3 (default) for chipv1.1.
     */
    void setStorageCellStart(int cell, Positions pos = {});

    /** [Jungfrau] Advanced*/
    Result<ns> getStorageCellDelay(Positions pos = {}) const;

    /** [Jungfrau] Advanced \n Additional time delay between 2
     * consecutive exposures in burst mode. \n Options: (0-1638375 ns
     * (resolution of 25ns)\n Only applicable for chipv1.0.
     */
    void setStorageCellDelay(ns value, Positions pos = {});

    /** list of possible gainmode  */
    std::vector<defs::gainMode> getGainModeList() const;

    /** [Jungfrau]*/
    Result<defs::gainMode> getGainMode(Positions pos = {}) const;

    /** [Jungfrau] Options: DYNAMIC, FORCE_SWITCH_G1, FORCE_SWITCH_G2,
     * FIX_G1, FIX_G2, FIX_G0 \n\CAUTION: Do not use FIX_G0 without caution, you
     * can damage the detector!!!\n
     */
    void setGainMode(const defs::gainMode mode, Positions pos = {});

    /** [Jungfrau] Advanced */
    Result<int> getNumberOfFilterCells(Positions pos = {}) const;

    /** [Jungfrau] Advanced Options[0-12], only for chip v1.1
     */
    void setNumberOfFilterCells(int cell, Positions pos = {});

    /** [Jungfrau] */
    Result<defs::pedestalParameters> getPedestalMode(Positions pos = {}) const;

    /** [Jungfrau] In pedestal mode, the number of frames or triggers is
     * overwritten by \n(#pedestal_frames x #pedestal_loops x 2). \nIn
     * auto timing mode or in trigger mode with #frames > 1, #frames is
     * overwritten and #triggers = 1, \nelse #triggers is overwritten and
     * #frames = 1. One cannot set #frames, #triggers or timing mode in pedestal
     * mode (it will throw an exception). Disabling pedestal mode will set back
     * the original values of #frames and #triggers
     */
    void setPedestalMode(const defs::pedestalParameters par,
                         Positions pos = {});

    /** [Jungfrau] */
    Result<defs::timingInfoDecoder>
    getTimingInfoDecoder(Positions pos = {}) const;

    /** [Jungfrau] Advanced Command! Only for pcb v2.0 */
    void setTimingInfoDecoder(defs::timingInfoDecoder value,
                              Positions pos = {});

    /** [Jungfrau] */
    Result<defs::collectionMode> getCollectionMode(Positions pos = {}) const;

    /** [Jungfrau] */
    void setCollectionMode(defs::collectionMode value, Positions pos = {});

    ///@}

    /** @name Gotthard Specific */
    ///@{
    /**************************************************
     *                                                *
     *    Gotthard Specific                           *
     *                                                *
     * ************************************************/

    /** [Gotthard]*/
    Result<defs::ROI> getROI(Positions pos = {}) const;

    /**
     * [Gotthard] Region of interest in detector \n
     * Options: Only a single ROI per module \n
     * Either all channels or a single adc or 2 chips (256 channels). Default is
     * all channels enabled (-1 -1). \n module_id is position index
     */
    void setROI(defs::ROI value, int module_id);

    /** [Gotthard] Clear ROI to all channels enabled. Default is all channels
     * enabled. */
    void clearROI(Positions pos = {});

    /** [Gotthard] */
    Result<ns> getExptimeLeft(Positions pos = {}) const;
    ///@}

    /** @name Gotthard2 Specific */
    ///@{
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

    /** [Gotthard2] Period between 2 bursts. Only in burst mode and auto timing
     * mode */
    void setBurstPeriod(ns value, Positions pos = {});

    /** [Gotthard2] only in burst auto mode */
    Result<int64_t> getNumberOfBurstsLeft(Positions pos = {}) const;

    /** [Gotthard2] offset channel, increment channel */
    Result<std::array<int, 2>> getInjectChannel(Positions pos = {});

    /** [Gotthard2]
     * Inject channels with current source for calibration.
     * offsetChannel is starting channel to be injected
     * incrementChannel is determines succeeding channels to be injected */
    void setInjectChannel(const int offsetChannel, const int incrementChannel,
                          Positions pos = {});

    /** [Gotthard2] gain indices and adu values for each channel */
    void getVetoPhoton(const int chipIndex, const std::string &fname,
                       Positions pos = {});

    /** [Gotthard2] energy in keV */
    void setVetoPhoton(const int chipIndex, const int numPhotons,
                       const int energy, const std::string &fname,
                       Positions pos = {});

    /** [Gotthard2] for all chips */
    void setVetoReference(const int gainIndex, const int value,
                          Positions pos = {});

    /** [Gotthard2] Set veto reference for each 128 channels for specific chip.
     * The file should have 128 rows of gain index and 12 bit value in dec"*/
    void setVetoFile(const int chipIndex, const std::string &fname,
                     Positions pos = {});

    /** [Gotthard2]  */
    Result<defs::burstMode> getBurstMode(Positions pos = {});

    /** [Gotthard2]  BURST_INTERNAL (default), BURST_EXTERNAL,
     * CONTINUOUS_INTERNAL, CONTINUOUS_EXTERNAL. Also changes clkdiv 2, 3, 4 */
    void setBurstMode(defs::burstMode value, Positions pos = {});

    /** [Gotthard2] */
    Result<bool> getCDSGain(Positions pos = {}) const;

    /** default disabled */
    void setCDSGain(bool value, Positions pos = {});

    /** [Gotthard2] */
    Result<defs::timingSourceType> getTimingSource(Positions pos = {}) const;

    /** [Gotthard2] Options: TIMING_INTERNAL (default), TIMING_EXTERNAL */
    void setTimingSource(defs::timingSourceType value, Positions pos = {});

    /** [Gotthard2] */
    Result<bool> getVeto(Positions pos = {}) const;

    /** [Gotthard2] Veto data in chip, Default disabled */
    void setVeto(const bool enable, Positions pos = {});

    /** [Gotthard2] */
    Result<defs::streamingInterface> getVetoStream(Positions pos = {}) const;

    /** [Gotthard2] Options: NONE (Default), LOW_LATENCY_LINK, ETHERNET_10GB
     * (debugging), ALL Enable or disable the 2 veto streaming interfaces
     * available. Can concatenate more than one interface. \nLOW_LATENCY_LINK is
     * the default interface to work with. \nETHERNET_10GB is for debugging and
     * also enables second interface in receiver for listening to veto packets
     * (writes a separate file if writing enabled). Also restarts client and
     * receiver zmq sockets if zmq streaming enabled.*/
    void setVetoStream(const defs::streamingInterface value,
                       Positions pos = {});

    /** [Gotthard2] */
    Result<defs::vetoAlgorithm>
    getVetoAlgorithm(const defs::streamingInterface value,
                     Positions pos = {}) const;

    /** [Gotthard2] Options(vetoAlgorithm): ALG_HITS (default), ALG_RAW.
     * Options(streamingInterface): LOW_LATENCY_LINK, ETHERNET_10GB */
    void setVetoAlgorithm(const defs::vetoAlgorithm alg,
                          const defs::streamingInterface value,
                          Positions pos = {});

    /** [Gotthard2] */
    Result<int> getADCConfiguration(const int chipIndex, const int adcIndex,
                                    Positions pos = {}) const;

    /** [Gotthard2] configures one chip at a time for specific adc, chipIndex.
     * -1 for all. Setting specific chip index not implemented in hardware yet
     */
    void setADCConfiguration(const int chipIndex, const int adcIndex,
                             const int value, Positions pos = {});

    ///@}

    /** @name Mythen3 Specific */
    ///@{
    /**************************************************
     *                                                *
     *    Mythen3 Specific                            *
     *                                                *
     * ************************************************/
    /** [Mythen3] */
    Result<uint32_t> getCounterMask(Positions pos = {}) const;

    /** [Mythen3] countermask bit set for each counter index enabled. Enabling
     * counters sets vth dacs to remembered values and disabling sets them to
     * disabled values. Setting vth dacs explicitly overwrites them. */
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

    /** [Mythen3] gate delay for all gates in auto or trigger timing mode
     * (internal gating). Gate index: 0-2, -1 for all */
    Result<std::array<ns, 3>> getGateDelayForAllGates(Positions pos = {}) const;

    // TODO! check if we really want to expose this !!!!!
    Result<int> getChipStatusRegister(Positions pos = {}) const;

    void setGainCaps(int caps, Positions pos = {});

    Result<int> getGainCaps(Positions pos = {});

    /** [Mythen3] */
    Result<defs::polarity> getPolarity(Positions pos = {}) const;

    /** [Mythen3] */
    void setPolarity(defs::polarity value, Positions pos = {});

    /** [Mythen3] */
    Result<bool> getInterpolation(Positions pos = {}) const;

    /** [Mythen3] interpolation mode enables all counters and disables vth3.
     * Disabling sets back counter mask and vth3. */
    void setInterpolation(bool value, Positions pos = {});

    /** [Mythen3] */
    Result<bool> getPumpProbe(Positions pos = {}) const;

    /** [Mythen3] pump probe mode only enables vth2. Disabling sets back to
     * previous value */
    void setPumpProbe(bool value, Positions pos = {});

    /** [Mythen3] */
    Result<bool> getAnalogPulsing(Positions pos = {}) const;

    /** [Mythen3] */
    void setAnalogPulsing(bool value, Positions pos = {});

    /** [Mythen3] */
    Result<bool> getDigitalPulsing(Positions pos = {}) const;

    /** [Mythen3] */
    void setDigitalPulsing(bool value, Positions pos = {});

    ///@}

    /** @name CTB Specific */
    ///@{
    /**************************************************
     *                                                *
     *    CTB / Xilinx CTB Specific                   *
     *                                                *
     * ************************************************/
    /** [CTB] */
    Result<int> getNumberOfAnalogSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfAnalogSamples(int value, Positions pos = {});

    /** [CTB] */
    Result<int> getADCClock(Positions pos = {}) const;

    /** [CTB] */
    void setADCClock(int value_in_MHz, Positions pos = {});

    /** [CTB] */
    Result<int> getRUNClock(Positions pos = {}) const;

    /** [CTB] */
    void setRUNClock(int value_in_MHz, Positions pos = {});

    /** [CTB]  in MHZ */
    Result<int> getSYNCClock(Positions pos = {}) const;

    /** gets list of power enums */
    std::vector<defs::dacIndex> getPowerList() const;

    /** gets list of slow adc enums */
    std::vector<defs::dacIndex> getSlowADCList() const;

    /** [CTB][Xilinx CTB] */
    Result<int> getPower(defs::dacIndex index, Positions pos = {}) const;

    /**
     * [CTB][Xilinx CTB] mV
     * [Ctb][Xilinx CTB] Options: V_LIMIT, V_POWER_A, V_POWER_B, V_POWER_C,
     * V_POWER_D, V_POWER_IO, V_POWER_CHIP
     */
    void setPower(defs::dacIndex index, int value, Positions pos = {});

    /**
     * [CTB] Options: [0- 4] or [1V, 1.14V, 1.33V, 1.6V, 2V]
     */
    Result<int> getADCVpp(bool mV = false, Positions pos = {}) const;

    /** [CTB] */
    void setADCVpp(int value, bool mV = false, Positions pos = {});

    /** [CTB] */
    Result<uint32_t> getADCEnableMask(Positions pos = {}) const;

    /** [CTB] */
    void setADCEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB] */
    Result<uint32_t> getTenGigaADCEnableMask(Positions pos = {}) const;

    /** [CTB] If any of a consecutive 4 bits are enabled, the "
        "complete 4 bits are enabled */
    void setTenGigaADCEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB][Xilinx CTB] */
    Result<uint32_t> getTransceiverEnableMask(Positions pos = {}) const;

    /** [CTB][Xilinx CTB] */
    void setTransceiverEnableMask(uint32_t mask, Positions pos = {});

    /** [CTB] */
    Result<int> getNumberOfDigitalSamples(Positions pos = {}) const;

    /** [CTB] */
    void setNumberOfDigitalSamples(int value, Positions pos = {});

    /** [CTB][Xilinx CTB] */
    Result<int> getNumberOfTransceiverSamples(Positions pos = {}) const;

    /** [CTB][Xilinx CTB] */
    void setNumberOfTransceiverSamples(int value, Positions pos = {});

    /** [CTB][Xilinx CTB] */
    Result<defs::readoutMode> getReadoutMode(Positions pos = {}) const;

    /** [CTB] Options: ANALOG_ONLY (default), DIGITAL_ONLY, ANALOG_AND_DIGITAL,
     * TRANSCEIVER_ONLY, DIGITAL_AND_TRANSCEIVER
     * [Xilinx CTB] Options: TRANSCEIVER_ONLY (default)
     */
    void setReadoutMode(defs::readoutMode value, Positions pos = {});

    /** [CTB] */
    Result<int> getDBITClock(Positions pos = {}) const;

    /** [CTB] */
    void setDBITClock(int value_in_MHz, Positions pos = {});

    /**
     * [CTB] mV
     * Options: V_POWER_A, V_POWER_B, V_POWER_C, V_POWER_D, V_POWER_IO */
    Result<int> getMeasuredPower(defs::dacIndex index,
                                 Positions pos = {}) const;

    /**
     * [CTB] mA
     * Options: I_POWER_A, I_POWER_B, I_POWER_C, I_POWER_D, I_POWER_IO  */
    Result<int> getMeasuredCurrent(defs::dacIndex index,
                                   Positions pos = {}) const;

    /** [CTB][Xilinx CTB] Options: SLOW_ADC0 - SLOW_ADC7  in uV */
    Result<int> getSlowADC(defs::dacIndex index, Positions pos = {}) const;

    /** [CTB] */
    Result<int> getExternalSamplingSource(Positions pos = {}) const;

    /** [CTB] Value between 0-63 \n For advanced users only.*/
    void setExternalSamplingSource(int value, Positions pos = {});

    /** [CTB] */
    Result<bool> getExternalSampling(Positions pos = {}) const;

    /** [CTB] For advanced users only. */
    void setExternalSampling(bool value, Positions pos = {});

    /** [CTB] */
    Result<std::vector<int>> getRxDbitList(Positions pos = {}) const;

    /** [CTB] list contains the set of digital signal bits (0-63) to save, must
     * be non repetitive. Note: data will be rearranged according to signal bits
     */
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

    /** [CTB] Default is enabled. */
    void setLEDEnable(bool enable, Positions pos = {});

    /** [CTB][Xilinx CTB] */
    void setDacNames(const std::vector<std::string> names);

    /** [CTB][Xilinx CTB] */
    std::vector<std::string> getDacNames() const;

    /** [CTB][Xilinx CTB] */
    defs::dacIndex getDacIndex(const std::string &name) const;

    /** [CTB][Xilinx CTB] */
    void setDacName(const defs::dacIndex i, const std::string &name);

    /** [CTB][Xilinx CTB] */
    std::string getDacName(const defs::dacIndex i) const;

    /** [CTB][Xilinx CTB] */
    void setAdcNames(const std::vector<std::string> names);

    /** [CTB][Xilinx CTB] */
    std::vector<std::string> getAdcNames() const;

    /** [CTB][Xilinx CTB] */
    int getAdcIndex(const std::string &name) const;

    /** [CTB][Xilinx CTB] */
    void setAdcName(const int i, const std::string &name);

    /** [CTB][Xilinx CTB] */
    std::string getAdcName(const int i) const;

    /** [CTB][Xilinx CTB] */
    void setSignalNames(const std::vector<std::string> names);

    /** [CTB][Xilinx CTB] */
    std::vector<std::string> getSignalNames() const;

    /** [CTB][Xilinx CTB] */
    int getSignalIndex(const std::string &name) const;

    /** [CTB][Xilinx CTB] */
    void setSignalName(const int i, const std::string &name);

    /** [CTB][Xilinx CTB] */
    std::string getSignalName(const int i) const;

    /** [CTB][Xilinx CTB] */
    void setPowerNames(const std::vector<std::string> names);

    /** [CTB][Xilinx CTB] */
    std::vector<std::string> getPowerNames() const;

    /** [CTB][Xilinx CTB] */
    defs::dacIndex getPowerIndex(const std::string &name) const;

    /** [CTB][Xilinx CTB] */
    void setPowerName(const defs::dacIndex i, const std::string &name);

    /** [CTB][Xilinx CTB] */
    std::string getPowerName(const defs::dacIndex i) const;

    /** [CTB][Xilinx CTB] */
    void setSlowADCNames(const std::vector<std::string> names);

    /** [CTB][Xilinx CTB] */
    std::vector<std::string> getSlowADCNames() const;

    /** [CTB][Xilinx CTB] */
    defs::dacIndex getSlowADCIndex(const std::string &name) const;

    /** [CTB][Xilinx CTB] */
    void setSlowADCName(const defs::dacIndex i, const std::string &name);

    /** [CTB][Xilinx CTB] */
    std::string getSlowADCName(const defs::dacIndex i) const;

    ///@}

    /** @name Xilinx CTB Specific */
    ///@{
    /**************************************************
     *                                                *
     *    Xilinx CTB Specific                         *
     *                                                *
     * ************************************************/
    ///@}

    /** [Xilinx Ctb] */
    void configureTransceiver(Positions pos = {});

    /** @name Pattern */
    ///@{
    /**************************************************
     *                                                *
     *    Pattern                                     *
     *                                                *
     * ************************************************/
    /** [CTB][Mythen3][Xilinx CTB] Gets the pattern file name including path of
     * the last pattern uploaded. \n Returns an empty if nothing was uploaded or
     * via a server default file*/
    Result<std::string> getPatterFileName(Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] Loads ASCII pattern file directly to server
     * (instead of executing line by line)*/
    void setPattern(const std::string &fname, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB] Loads pattern parameters structure directly
     * to server */
    void setPattern(const Pattern &pat, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB] Saves pattern to file
     * (ascii). \n [Ctb] Also executes pattern.*/
    void savePattern(const std::string &fname);

    /** [Mythen3] Loads and runs default pattern */
    void loadDefaultPattern(Positions pos = {});

    /** [CTB] */
    Result<uint64_t> getPatternIOControl(Positions pos = {}) const;

    /** [CTB] */
    void setPatternIOControl(uint64_t word, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB] same as executing for ctb */
    Result<uint64_t> getPatternWord(int addr, Positions pos = {});

    /** [CTB][Xilinx CTB] Caution: If word is  -1  reads the addr (same as
     * executing the pattern)
     * [Mythen3] */
    void setPatternWord(int addr, uint64_t word, Positions pos = {});

    /**[CTB][Mythen3][Xilinx CTB] Options: level: -1 (complete pattern) and 0-2
     * levels
     * @returns array of start address and stop address
     */
    Result<std::array<int, 2>>
    getPatternLoopAddresses(int level, Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] Options: level: -1 (complete pattern) and 0-2
     * levels */
    void setPatternLoopAddresses(int level, int start, int stop,
                                 Positions pos = {});

    /**[CTB][Mythen3][Xilinx CTB] Options: level: -1 (complete pattern) and 0-2
     * levels  */
    Result<int> getPatternLoopCycles(int level, Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] n: 0-2, level: -1 (complete pattern) and 0-2
     * levels */
    void setPatternLoopCycles(int level, int n, Positions pos = {});

    /**[CTB][Mythen3][Xilinx CTB] */
    Result<int> getPatternWaitAddr(int level, Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] Options: level 0-2 */
    void setPatternWaitAddr(int level, int addr, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB]  */
    Result<uint64_t> getPatternWaitTime(int level, Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] Options: level 0-2 */
    void setPatternWaitTime(int level, uint64_t t, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB] */
    Result<uint64_t> getPatternMask(Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB] Selects the bits that will have a pattern
     * mask applied to the selected patmask for every pattern. */
    void setPatternMask(uint64_t mask, Positions pos = {});

    /** [CTB][Mythen3][Xilinx CTB]  */
    Result<uint64_t> getPatternBitMask(Positions pos = {}) const;

    /** [CTB][Mythen3][Xilinx CTB] Sets the mask applied to every pattern to the
     * selected bits */
    void setPatternBitMask(uint64_t mask, Positions pos = {});

    /** [Mythen3] */
    void startPattern(Positions pos = {});
    ///@}

    /** @name Json Header specific */
    ///@{
    /**************************************************
     *                                                *
     *    Json Header specific                        *
     *                                                *
     * ************************************************/

    Result<std::map<std::string, std::string>>
    getAdditionalJsonHeader(Positions pos = {}) const;

    /**  If empty, reset additional json header. Default is empty. Max
     * 20 characters for each key/value. Empty value deletes header. Use only if
     * to be processed by an intermediate user process listening to receiver zmq
     * packets such as in Moench */
    void setAdditionalJsonHeader(
        const std::map<std::string, std::string> &jsonHeader,
        Positions pos = {});

    Result<std::string> getAdditionalJsonParameter(const std::string &key,
                                                   Positions pos = {}) const;
    /**
     * Sets the value for additional json header parameters. If not found,
     * the pair is appended. Empty value deletes parameter. Max 20 characters
     * for each key/value.
     */
    void setAdditionalJsonParameter(const std::string &key,
                                    const std::string &value,
                                    Positions pos = {});
    ///@}

    /** @name Advanced */
    ///@{
    /**************************************************
     *                                                *
     *    Advanced                                    *
     *                                                *
     * ************************************************/

    /** [CTB][Moench] */
    Result<int> getADCPipeline(Positions pos = {}) const;

    /** [CTB][Moench] */
    void setADCPipeline(int value, Positions pos = {});

    /**  [Jungfrau][Moench][Gotthard][CTB][Mythen3][Gotthard2]
     * Advanced user Function!
     * Program firmware from command line, after which detector controller is
     * rebooted. forceDeleteNormalFile is true, if normal file found
     * in device tree, it must be deleted, a new device drive created and
     * programming continued.[Jungfrau][Moench][CTB] fname is a pof file (full
     * path) \n [Mythen3][Gotthard2] fname is an rbf file (full path)
     */
    void programFPGA(const std::string &fname, const bool forceDeleteNormalFile,
                     Positions pos = {});

    /** [Jungfrau][Moench][CTB][Xilinx CTB]  Advanced user Function!  */
    void resetFPGA(Positions pos = {});

    /** [Jungfrau][Moench][Eiger][Ctb][Mythen3][Gotthard2] Copies detector
     * server via TCP (without tftp).\nMakes a symbolic link with a shorter
     * name (without vx.x.x).\nThen, detector controller reboots (except
     * Eiger).\n[Jungfrau][Moench][Ctb] Also deletes old server binary and
     *  changes respawn server to the link, which is effective after a reboot.
     */
    void updateDetectorServer(const std::string &fname, Positions pos = {});

    /** [Jungfrau][Moench][Ctb][Mythen3][Gotthard2] \n
     * Advanced Command!! You could damage the detector. Please use with
     * caution.\nUpdates the kernel image. Then, detector controller reboots
     *  with new kernel
     */
    void updateKernel(const std::string &fname, Positions pos = {});

    /** [Jungfrau][Moench][Gotthard][CTB][Mythen3][Gotthard2][Xilinx CTB]
     * Advanced user Function! */
    void rebootController(Positions pos = {});

    /**
     * Advanced user Function!\n [Jungfrau][Moench][Gotthard][CTB] Updates the
     * firmware, detector server, make a soft link and then reboots detector
     * controller. \n [Mythen3][Gotthard2] Will require a script to start up the
     * shorter named server link at start up \n sname is full path name of
     * detector server  \n fname is programming file name with full path to it
     */
    void updateFirmwareAndServer(const std::string &sname,
                                 const std::string &fname, Positions pos = {});

    Result<bool> getUpdateMode(Positions pos = {}) const;

    /** Restarts detector server in update mode. This is useful when
     * server-firmware compatibility is at its worst and server cannot start up
     * normally */
    void setUpdateMode(const bool updatemode, Positions pos = {});

    /** Advanced user Function! \n
     * Goes to stop server. Hence, can be called while calling blocking
     * acquire(). \n [Eiger] Address is +0x100 for only left, +0x200 for only
     * right. */
    Result<uint32_t> readRegister(uint32_t addr, Positions pos = {}) const;

    /** Advanced user Function! \n
     * Goes to stop server. Hence, can be called while calling blocking
     * acquire(). \n [Eiger] Address is +0x100 for only left, +0x200 for only
     * right. */
    void writeRegister(uint32_t addr, uint32_t val, bool validate = false,
                       Positions pos = {});

    /** Advanced user Function!  */
    void setBit(uint32_t addr, int bitnr, bool validate = false,
                Positions pos = {});

    /** Advanced user Function!  */
    void clearBit(uint32_t addr, int bitnr, bool validate = false,
                  Positions pos = {});

    /** Advanced user Function!  */
    Result<int> getBit(uint32_t addr, int bitnr, Positions pos = {});

    /** [Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] Advanced user
     * Function! */
    void executeFirmwareTest(Positions pos = {});

    /** [Gotthard][Jungfrau][Moench][Mythen3][Gotthard2][CTB] Advanced user
     * Function! Writes different values in a R/W register and confirms the
     * writes to check bus */
    void executeBusTest(Positions pos = {});

    /** [Gotthard][Jungfrau][Moench][CTB] Advanced user Function! not possible
     * to read back */
    void writeAdcRegister(uint32_t addr, uint32_t value, Positions pos = {});

    /** Advanced user Function!  */
    bool getInitialChecks() const;

    /** Enables/disabled initial compaibility and other server start up checks.
     * \n Default is enabled. Must come before 'hostname' command to take
     * effect. \n Can be used to reprogram fpga when current firmware is
     * incompatible. \n Advanced user Function! */
    void setInitialChecks(const bool value);

    /** [CTB][Jungfrau][Moench] Advanced user Function! */
    Result<uint32_t> getADCInvert(Positions pos = {}) const;

    /** [CTB][Jungfrau][Moench] Advanced user Function! \n
    [Jungfrau][Moench] Inversions on top of default mask */
    void setADCInvert(uint32_t value, Positions pos = {});
    ///@}

    /** @name Insignificant */
    ///@{
    /**************************************************
     *                                                *
     *    Insignificant                               *
     *                                                *
     * ************************************************/

    Result<uint16_t> getControlPort(Positions pos = {}) const;

    /** Detector Control TCP port (for client communication with Detector
     * control server) Default is 1952. Normally unchanged. Set different ports
     * for virtual servers on same pc */
    void setControlPort(uint16_t value, Positions pos = {});

    Result<uint16_t> getStopPort(Positions pos = {}) const;

    /** Port number of the stop server on detector for detector-client tcp
     * interface. Default is 1953. Normally unchanged. */
    void setStopPort(uint16_t value, Positions pos = {});

    Result<bool> getDetectorLock(Positions pos = {}) const;

    /** lock detector to one client IP. default is unlocked */
    void setDetectorLock(bool lock, Positions pos = {});

    /** Client IP Address that last communicated with the detector */
    Result<IpAddr> getLastClientIP(Positions pos = {}) const;

    /** Execute a command on the detector server console */
    Result<std::string> executeCommand(const std::string &value,
                                       Positions pos = {});

    /** [Jungfrau][Moench][Mythen3][CTB][Xilinx CTB]
     * [Gotthard2] only in continuous mode */
    Result<int64_t> getNumberOfFramesFromStart(Positions pos = {}) const;

    /** [Jungfrau][Moench][Mythen3][CTB][Xilinx CTB] Get time from detector
     * start [Gotthard2] not in burst and auto mode */
    Result<ns> getActualTime(Positions pos = {}) const;

    /** [Jungfrau][Moench][Mythen3][CTB][Xilinx CTB] Get timestamp at a frame
     * start [Gotthard2] not in burst and auto mode */
    Result<ns> getMeasurementTime(Positions pos = {}) const;

    /** get user details from shared memory  (hostname, type, PID, User, Date)
     */
    std::string getUserDetails() const;

    ///@}

  private:
    std::vector<uint16_t> getValidPortNumbers(uint16_t start_port);
    void updateRxRateCorrections();
    void setNumberofUDPInterfaces_(int n, Positions pos);
};

} // namespace sls

#ifndef DETECTOR_H
#define DETECTOR_H
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "error_defs.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
// #include "slsDetectorUtils.h"
#include "sls_detector_defs.h"
// #include "sls_receiver_defs.h"

class Detector {
  public:
    Detector(int i) : det(i), multi_detector_id(i) {
        // Disable output from std::cout
        // std::cout.setstate(std::ios_base::failbit);
    }

    int getMultiDetectorId() { return multi_detector_id; }

    // get image size as [nrow, ncols] return as a pair of ints
    std::pair<int, int> getImageSize() {
        std::pair<int, int> image_size{0, 0};
        image_size.first = det.getMaxNumberOfChannelsPerDetector(
            slsDetectorDefs::dimension::Y);
        image_size.second = det.getMaxNumberOfChannelsPerDetector(
            slsDetectorDefs::dimension::X);
        return image_size;
    }

    void setImageSize(const int rows, const int cols) {
        det.setMaxNumberOfChannelsPerDetector(slsDetectorDefs::dimension::Y,
                                              rows);
        det.setMaxNumberOfChannelsPerDetector(slsDetectorDefs::dimension::X,
                                              cols);
    }

    // blocking command, acquire set number of frames
    void acquire() { det.acquire(); }

    // for Eiger check status of  the module
    // true active false deactivated
    bool getActive(int i) { return det.activate(-1, i); }
    // activate or deactivate a module
    void setActive(int i, bool value) { det.activate(value, i); }

    int getFramesCaughtByReceiver() {
        return det.getFramesCaughtByReceiver();
        // return det.getFramesCaughtByReceiver();
    }
    int getFramesCaughtByReceiver(int i) {
        return det.getFramesCaughtByReceiver(i);
    }

    void setReceiverFifoDepth(int n_frames) {
        det.setReceiverFifoDepth(n_frames);
    }

    void setNumberOfStorageCells(const int64_t num) {
        det.setTimer(slsDetectorDefs::timerIndex::STORAGE_CELL_NUMBER, num);
    }
    int getNumberOfStorageCells() {
        return det.setTimer(slsDetectorDefs::timerIndex::STORAGE_CELL_NUMBER,
                            -1);
    }

    void setStoragecellStart(int cell) { det.setStoragecellStart(cell); }

    int getStoragecellStart() { return det.setStoragecellStart(); }

    int getReceiverFifoDepth() { return det.setReceiverFifoDepth(); }

    void resetFramesCaught() { det.resetFramesCaught(); }

    int getReceiverCurrentFrameIndex() {
        return det.getReceiverCurrentFrameIndex();
    }

    std::string getReceiverHostname(int det_id = -1) const {
        return det.getReceiverHostname(det_id);
    }

    void setReceiverHostname(std::string hostname, int det_id = -1) {
        det.setReceiverHostname(hostname, det_id);
    }

    std::string getReceiverUDPIP(int det_id = -1) const {
        return det.getReceiverUDPIP(det_id);
    }

    void setReceiverUDPIP(std::string ip, int det_id = -1) {
        det.setReceiverUDPIP(ip, det_id);
    }

    std::string getReceiverUDPMAC(int det_id = -1) {
        return det.getReceiverUDPMAC(det_id);
    }

    void setReceiverUDPMAC(std::string mac, int det_id = -1) {
        det.setReceiverUDPMAC(mac, det_id);
    }

    void startReceiver() { det.startReceiver(); }
    void stopReceiver() { det.stopReceiver(); }

    bool getTenGigabitEthernet() { return det.enableTenGigabitEthernet(); }
    void setTenGigabitEthernet(bool value) {
        det.enableTenGigabitEthernet(value);
    }

    void setFileFormat(const std::string &format);
    std::string getFileFormat();

    std::string checkOnline() { return det.checkOnline(); }

    bool getReceiverOnline() { return det.setReceiverOnline(); }
    void setReceiverOnline(const bool status) { det.setReceiverOnline(status); }

    bool getOnline() { return det.setOnline(); }
    void setOnline(const bool status) { det.setOnline(status); }

    bool isChipPowered() { return det.powerChip(); }
    void powerChip(const bool value) { det.powerChip(value); }

    // read register from readout system, used for low level control
    uint32_t readRegister(const uint32_t addr) {
        return det.readRegister(addr);
    }

    // directly write to register in readout system
    void writeRegister(const uint32_t addr, const uint32_t value) {
        det.writeRegister(addr, value);
    }

    // directly write to the ADC register
    // should this also be unsigned? Probably...
    void writeAdcRegister(const int addr, const int value) {
        det.writeAdcRegister(addr, value);
    }

    void setBitInRegister(const uint32_t reg_addr, const int bit_number) {
        det.setBit(reg_addr, bit_number);
    }
    void clearBitInRegister(const uint32_t reg_addr, const int bit_number) {
        det.clearBit(reg_addr, bit_number);
    }

    bool getAcquiringFlag() { return det.getAcquiringFlag(); }

    void setAcquiringFlag(const bool flag) { det.setAcquiringFlag(flag); }

    bool getCounterBit() { return det.setCounterBit(); }
    void setCounterBit(bool b) { det.setCounterBit(b); }

    slsDetectorDefs::dacIndex dacNameToEnum(std::string dac_name);

    std::pair<int, int> getDetectorGeometry() {
        std::pair<int, int> g;
        det.getNumberOfDetectors(g.first, g.second);
        return g;
    }

    int getNumberOfDetectors() { return det.getNumberOfDetectors(); }

    std::string getRunStatus() {
        auto s = det.getRunStatus();
        return det.runStatusType(s);
    }

    void startAcquisition() { det.startAcquisition(); }
    void stopAcquisition() { det.stopAcquisition(); }

    std::string getHostname() { return det.getHostname(); }

    void setHostname(std::string hostname) {
        det.setHostname(hostname.c_str());
    }

    int getDynamicRange() { return det.setDynamicRange(-1); }
    void setDynamicRange(const int dr) { det.setDynamicRange(dr); }

    void pulseChip(const int n) { det.pulseChip(n); }
    void pulseAllPixels(const int n);
    void pulseDiagonal(const int n);

    void readConfigurationFile(std::string fname) {
        det.readConfigurationFile(fname);
    }
    void readParametersFile(std::string fname) {
        det.retrieveDetectorSetup(fname);
    }

    int64_t getFirmwareVersion() {
        return det.getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION);
    }
    int64_t getServerVersion() {
        return det.getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION);
    }
    int64_t getClientVersion() {
        return det.getId(slsDetectorDefs::THIS_SOFTWARE_VERSION);
    }
    int64_t getReceiverVersion() {
        return det.getId(slsDetectorDefs::RECEIVER_VERSION);
    }

    std::vector<int64_t> getDetectorNumber() { return det.getDetectorNumber(); }

    int getReadoutClockSpeed() {
        return det.setSpeed(slsDetectorDefs::CLOCK_DIVIDER, -1);
    }
    void setReadoutClockSpeed(const int speed) {
        det.setSpeed(slsDetectorDefs::CLOCK_DIVIDER, speed);
    }

    void setDbitPipeline(const int value) {
        det.setSpeed(slsDetectorDefs::DBIT_PIPELINE, value);
    }
    int getDbitPipeline() {
        return det.setSpeed(slsDetectorDefs::DBIT_PIPELINE, -1);
    }
    void setDbitPhase(const int value) {
        det.setSpeed(slsDetectorDefs::DBIT_PHASE, value);
    }
    int getDbitPhase() { return det.setSpeed(slsDetectorDefs::DBIT_PHASE, -1); }
    void setDbitClock(const int value) {
        det.setSpeed(slsDetectorDefs::DBIT_CLOCK, value);
    }
    int getDbitClock() { return det.setSpeed(slsDetectorDefs::DBIT_CLOCK, -1); }
    std::vector<int> getReceiverPort() const { return det.getReceiverPort(); }

    void setReceiverPort(int det_id, int value) {
        det.setReceiverPort(value, det_id);
    }

    void setRateCorrection(std::vector<double> tau) {
        for (int i = 0; i < det.getNumberOfDetectors(); ++i)
            det.setRateCorrection(tau[i], i);
    }

    std::vector<double> getRateCorrection();

    void setPatternLoops(int level, int start, int stop,
                         int n, int detPos) {
        det.setPatternLoops(level, start, stop, n, detPos);
    }


    std::array<uint64_t, 3> getPatternLoops(uint64_t level, int detPos) {
        return det.getPatternLoops(level, detPos);
    }

    void setPatternWord(int addr, uint64_t word, int detPos) {
        det.setPatternWord(addr, word, detPos);
    }

    bool getFlippedDataX(int i) {
        return det.getFlippedData(slsDetectorDefs::dimension::X, i);
    }

    bool getFlippedDataY(int i) {
        return det.getFlippedData(slsDetectorDefs::dimension::Y, i);
    }

    void setFlippedDataX(int i, bool value) {
        det.setFlippedData(slsDetectorDefs::dimension::X, value, i);
    }

    void setFlippedDataY(int i, bool value) {
        det.setFlippedData(slsDetectorDefs::dimension::Y, value, i);
    }

    /*** Frame and file settings ***/
    void setFileName(std::string fname) { det.setFileName(fname); }
    std::string getFileName() { return det.getFileName(); }
    void setFilePath(std::string path) { det.setFilePath(path); }
    void setFilePath(std::string path, int i) { det.setFilePath(path, i); }
    std::string getFilePath() { return det.getFilePath(); }
    std::string getFilePath(int i) { return det.getFilePath(i); }

    std::string getUserDetails() { return det.getUserDetails(); }

    void setFramesPerFile(const int n_frames) {
        det.setFramesPerFile(n_frames);
    }
    int getFramesPerFile() { return det.setFramesPerFile(); }

    std::string getReceiverFrameDiscardPolicy() {
        return det.getReceiverFrameDiscardPolicy(
            det.setReceiverFramesDiscardPolicy());
    }
    void setReceiverFramesDiscardPolicy(std::string f) {
        auto fdp = det.getReceiverFrameDiscardPolicy(f);
        if (fdp == slsDetectorDefs::GET_FRAME_DISCARD_POLICY) {
            throw std::invalid_argument("Coult not decode policy: nodiscard, "
                                        "discardempty, discardpartial");
        }
        det.setReceiverFramesDiscardPolicy(fdp);
    }

    void setPartialFramesPadding(bool padding) {
        det.setPartialFramesPadding(padding);
    }

    bool getPartialFramesPadding() { return det.getPartialFramesPadding(); }

    std::vector<double> getMeasuredPeriod() {
        std::vector<double> mp;
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            auto t = det.getTimeLeft(slsDetectorDefs::MEASURED_PERIOD, i);
            mp.push_back(static_cast<double>(t) * 1E-9);
        }
        return mp;
    }
    std::vector<double> getMeasuredSubPeriod() {
        std::vector<double> mp;
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            auto t = det.getTimeLeft(slsDetectorDefs::MEASURED_SUBPERIOD, i);
            mp.push_back(static_cast<double>(t) * 1E-9);
        }
        return mp;
    }

    bool isClientAndDetectorCompatible() {
        auto r = det.checkDetectorVersionCompatibility();
        if (r == 0)
            return true;
        else
            return false;
    }
    bool isClientAndReceiverCompatible() {
        auto r = det.checkReceiverVersionCompatibility();
        if (r == 0)
            return true;
        else
            return false;
    }

    /*** END Frame and file settings ***/

    void loadTrimbitFile(std::string fname, const int idet) {
        det.loadSettingsFile(fname, idet);
    }

    // Eiger: set the energies where the detector is trimmed
    void setTrimEnergies(std::vector<int> energy) { det.setTrimEn(energy); }

    std::vector<int> getTrimEnergies() { return det.getTrimEn(); }

    /*** Temperature control functions for Jungfrau ***/
    void setThresholdTemperature(float t) {
        det.setThresholdTemperature(static_cast<int>(t * 1000), -1);
    }

    float getThresholdTemperature() {
        return static_cast<double>(det.setThresholdTemperature(-1, -1)) /
               1000.0;
    }

    void setTemperatureControl(bool v) { det.setTemperatureControl(v); }
    bool getTemperatureControl() { return det.setTemperatureControl(); }

    bool getTemperatureEvent() { return det.setTemperatureEvent(); }
    void resetTemperatureEvent() { det.setTemperatureEvent(0); }
    /*** END Temperature control functions for Jungfrau ***/

    void setThresholdEnergy(const int eV) { det.setThresholdEnergy(eV); }

    std::string getSettingsDir() { return det.getSettingsDir(); }
    void setSettingsDir(std::string dir) { det.setSettingsDir(dir); }

    int getThresholdEnergy() { return det.getThresholdEnergy(); }

    std::string getSettings() {
        return det.getDetectorSettings(det.getSettings());
    }

    void setSettings(std::string s) {
        det.setSettings(det.getDetectorSettings(s));
    }

    // name to enum translation on the c++ side
    // should we instead expose the enum to Python?
    int getDac(std::string dac_name, const int mod_id) {
        int val = -1;
        auto dac = dacNameToEnum(dac_name);
        return det.setDAC(val, dac, 0, mod_id);
    }

    void setDac(std::string dac_name, const int mod_id, int val) {
        auto dac = dacNameToEnum(dac_name);
        det.setDAC(val, dac, 0, mod_id);
    }

    int getDac_mV(std::string dac_name, const int mod_id) {
        int val = -1;
        auto dac = dacNameToEnum(dac_name);
        return det.setDAC(val, dac, 1, mod_id);
    }

    void setDac_mV(std::string dac_name, const int mod_id, int value) {
        auto dac = dacNameToEnum(dac_name);
        det.setDAC(value, dac, 1, mod_id);
    }

    // Intended for the JungfrauCTB should we name dacs instead
    int getDacFromIndex(const int index, const int mod_id) {
        int val = -1;
        auto dac = static_cast<slsDetectorDefs::dacIndex>(0);
        return det.setDAC(val, dac, 0, mod_id);
    }
    // Intended for the JungfrauCTB should we name dacs instead
    int setDacFromIndex(const int index, const int mod_id, int value) {
        auto dac = static_cast<slsDetectorDefs::dacIndex>(0);
        return det.setDAC(value, dac, 0, mod_id);
    }

    // Calling multi do we have a need to lock/unlock a single det?
    bool getServerLock() { return det.lockServer(-1); }
    void setServerLock(const bool value) { det.lockServer(value); }
    bool getReceiverLock() { return det.lockReceiver(-1); }
    void setReceiverLock(const bool value) { det.lockReceiver(value); }

    int getAdc(std::string adc_name, int mod_id) {
        auto adc = dacNameToEnum(adc_name);
        return det.getADC(adc, mod_id);
    }

    std::vector<std::string> getReadoutFlags();

    // note singular
    void setReadoutFlag(const std::string flag_name);

    // name to enum transltion of dac
    int getDacVthreshold() {
        int val = -1;
        auto dac = slsDetectorDefs::dacIndex::THRESHOLD;
        return det.setDAC(val, dac, 0, -1);
    }

    void setDacVthreshold(const int val) {
        auto dac = slsDetectorDefs::dacIndex::THRESHOLD;
        det.setDAC(val, dac, 0, -1);
    }

    void setFileIndex(const int i) { det.setFileIndex(i); }

    int getFileIndex() { return det.getFileIndex(); }

    // time in ns
    void setExposureTime(const int64_t t) {
        det.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, t);
    }

    // time in ns
    int64_t getExposureTime() {
        return det.setTimer(slsDetectorDefs::timerIndex::ACQUISITION_TIME, -1);
    }

    void setSubExposureTime(const int64_t t) {
        det.setTimer(slsDetectorDefs::timerIndex::SUBFRAME_ACQUISITION_TIME, t);
    }

    int64_t getSubExposureTime() {
        // time in ns
        return det.setTimer(
            slsDetectorDefs::timerIndex::SUBFRAME_ACQUISITION_TIME, -1);
    }

    void setSubExposureDeadTime(const int64_t t) {
        det.setTimer(slsDetectorDefs::timerIndex::SUBFRAME_DEADTIME, t);
    }

    int64_t getSubExposureDeadTime() {
        // time in ns
        return det.setTimer(slsDetectorDefs::timerIndex::SUBFRAME_DEADTIME, -1);
    }

    int64_t getCycles() {
        return det.setTimer(slsDetectorDefs::timerIndex::CYCLES_NUMBER, -1);
    }

    void setCycles(const int64_t n_cycles) {
        det.setTimer(slsDetectorDefs::timerIndex::CYCLES_NUMBER, n_cycles);
    }

    void setNumberOfMeasurements(const int n_measurements) {
        det.setTimer(slsDetectorDefs::timerIndex::MEASUREMENTS_NUMBER,
                     n_measurements);
    }
    int getNumberOfMeasurements() {
        return det.setTimer(slsDetectorDefs::timerIndex::MEASUREMENTS_NUMBER,
                            -1);
    }

    int getNumberOfGates() {
        return det.setTimer(slsDetectorDefs::timerIndex::GATES_NUMBER, -1);
    }
    void setNumberOfGates(const int t) {
        det.setTimer(slsDetectorDefs::timerIndex::GATES_NUMBER, t);
    }

    // time in ns
    int64_t getDelay() {
        return det.setTimer(slsDetectorDefs::timerIndex::DELAY_AFTER_TRIGGER,
                            -1);
    }
    // time in ns
    void setDelay(const int64_t t) {
        det.setTimer(slsDetectorDefs::timerIndex::DELAY_AFTER_TRIGGER, t);
    }
    // time in ns
    int64_t getPeriod() {
        return det.setTimer(slsDetectorDefs::timerIndex::FRAME_PERIOD, -1);
    }
    // time in ns
    void setPeriod(const int64_t t) {
        det.setTimer(slsDetectorDefs::timerIndex::FRAME_PERIOD, t);
    }

    int64_t getNumberOfFrames() {
        return det.setTimer(slsDetectorDefs::timerIndex::FRAME_NUMBER, -1);
    }

    void setNumberOfFrames(const int64_t nframes) {
        det.setTimer(slsDetectorDefs::timerIndex::FRAME_NUMBER, nframes);
    }

    std::string getTimingMode() {
        return det.externalCommunicationType(
            det.setExternalCommunicationMode());
    }
    void setTimingMode(const std::string mode) {
        det.setExternalCommunicationMode(det.externalCommunicationType(mode));
    }

    void freeSharedMemory() { det.freeSharedMemory(); }

    std::vector<std::string> getDetectorType() {
        std::vector<std::string> detector_type;
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            detector_type.push_back(det.getDetectorTypeAsString(i));
        }
        return detector_type;
    }

    void setFileWrite(bool value) { det.setFileWrite(value); }
    bool getFileWrite() { return det.getFileWrite(); }

    void setFileOverWrite(bool value) { det.setFileOverWrite(value); }

    bool getFileOverWrite() { return det.getFileOverWrite(); }

    void setAllTrimbits(int tb) { det.setAllTrimbits(tb); }
    int getAllTrimbits() { return det.setAllTrimbits(-1); }
    bool getRxDataStreamStatus() {
        return det.enableDataStreamingFromReceiver();
    }

    void setRxDataStreamStatus(bool state) {
        det.enableDataStreamingFromReceiver(state);
    }

    // Get a network parameter for all detectors, looping over individual
    // detectors return a vector of strings
    std::vector<int> getReceiverStreamingPort() {
        std::vector<int> vec;
        vec.reserve(det.getNumberOfDetectors());
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            vec.push_back(det.getReceiverStreamingPort(i));
        }
        return vec;
    }

    void setReceiverStreamingPort(int value, int det_id) {
        det.setReceiverDataStreamingOutPort(value, det_id);
    }

    std::vector<int> getReceiverUDPPort() {
        std::vector<int> vec;
        vec.reserve(det.getNumberOfDetectors());
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            vec.push_back(det.getReceiverUDPPort(i));
        }
        return vec;
    }

    std::vector<int> getReceiverUDPPort2() {
        std::vector<int> vec;
        vec.reserve(det.getNumberOfDetectors());
        for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
            vec.push_back(det.getReceiverUDPPort2(i));
        }
        return vec;
    }

    void setReceiverUDPPort(int port, int det_id) {
        det.setReceiverUDPPort(port, det_id);
    }
    void setReceiverUDPPort2(int port, int det_id) {
        det.setReceiverUDPPort2(port, det_id);
    }

    // //Set network parameter for all modules if det_id == -1 otherwise the
    // module
    // //specified with det_id.
    // void setDetectorNetworkParameter(std::string par_name, std::string par,
    // const int det_id) {
    //     auto p = networkNameToEnum(par_name);
    //     if (det_id == -1) {
    //         det.setDetectorNetworkParameter(p, par);
    //     } else {
    //         det.setDetectorNetworkParameter(p, par, det_id);
    //     }
    // }

    void configureNetworkParameters() { det.configureMAC(); }

    std::string getLastClientIP() { return det.getLastClientIP(); }
    std::string getReceiverLastClientIP() {
        return det.getReceiverLastClientIP();
    }

    // get frame delay of module (det_id) in ns
    int getDelayFrame(int det_id) {
        auto r = det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_FRAME, -1,
            det_id);
        return r;
    }
    // set frame delay of module (det_id) in ns
    void setDelayFrame(int det_id, int delay) {
        // auto delay_str = std::to_string(delay);
        det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_FRAME, delay,
            det_id);
    }

    // get delay left of module (det_id) in ns
    int getDelayLeft(int det_id) {
        auto r = det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_LEFT, -1,
            det_id);
        return r;
    }
    // set delay left of module (det_id) in ns
    void setDelayLeft(int det_id, int delay) {
        // auto delay_str = std::to_string(delay);
        det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_LEFT, delay,
            det_id);
    }

    // get delay right of module (det_id) in ns
    int getDelayRight(const int det_id) {
        auto r = det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_RIGHT, -1,
            det_id);
        return r;
    }

    // set delay right of module (det_id) in ns
    void setDelayRight(int det_id, int delay) {
        // auto delay_str = std::to_string(delay);
        det.setDetectorNetworkParameter(
            slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_RIGHT, delay,
            det_id);
    }

    // Check if detector if filling in gap pixels in module
    // return true if so, currently only in developer
    bool getGapPixels() { return det.enableGapPixels(-1); }

    // Set to true to have the detector filling in gap pixels
    // false to disable, currently only in developer
    void setGapPixels(bool val) { det.enableGapPixels(val); }

    slsDetectorDefs::networkParameter networkNameToEnum(std::string par_name);

  private:
    multiSlsDetector det;
    slsDetector *getSlsDetector(int i) const;
    int multi_detector_id = 0;
};

void Detector::setFileFormat(const std::string &format) {
    if (format == "binary") {
        det.setFileFormat(slsDetectorDefs::fileFormat::BINARY);
    } else if (format == "hdf5") {
        det.setFileFormat(slsDetectorDefs::fileFormat::HDF5);
    }
}

std::string Detector::getFileFormat() {
    auto format =
        det.setFileFormat(slsDetectorDefs::fileFormat::GET_FILE_FORMAT, -1);
    switch (format) {
    case slsDetectorDefs::fileFormat::BINARY:
        return "binary";
    case slsDetectorDefs::fileFormat::HDF5:
        return "hdf5";
    default:
        return "unknown";
    }
}

slsDetectorDefs::networkParameter
Detector::networkNameToEnum(std::string par_name) {

    if (par_name == "detectormac") {
        return slsDetectorDefs::networkParameter::DETECTOR_MAC;
    } else if (par_name == "detectorip") {
        return slsDetectorDefs::networkParameter::DETECTOR_IP;
    } else if (par_name == "rx_hostname") {
        return slsDetectorDefs::networkParameter::RECEIVER_HOSTNAME;
    } else if (par_name == "rx_udpip") {
        return slsDetectorDefs::networkParameter::RECEIVER_UDP_IP;
    } else if (par_name == "rx_udpport") {
        return slsDetectorDefs::networkParameter::RECEIVER_UDP_PORT;
    } else if (par_name == "rx_udpmac") {
        return slsDetectorDefs::networkParameter::RECEIVER_UDP_MAC;
    } else if (par_name == "rx_udpport2") {
        return slsDetectorDefs::networkParameter::RECEIVER_UDP_PORT2;
    } else if (par_name == "rx_udpsocksize") {
        return slsDetectorDefs::networkParameter::RECEIVER_UDP_SCKT_BUF_SIZE;
    } else if (par_name == "rx_realudpsocksize") {
        return slsDetectorDefs::networkParameter::
            RECEIVER_REAL_UDP_SCKT_BUF_SIZE;
    } else if (par_name == "rx_jsonaddheader") {
        return slsDetectorDefs::networkParameter::ADDITIONAL_JSON_HEADER;
    } else if (par_name == "delay_left") {
        return slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_LEFT;
    } else if (par_name == "delay_right") {
        return slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_RIGHT;
    } else if (par_name == "delay_frame") {
        return slsDetectorDefs::networkParameter::DETECTOR_TXN_DELAY_FRAME;
    } else if (par_name == "flow_control_10g") {
        return slsDetectorDefs::networkParameter::FLOW_CONTROL_10G;
    } else if (par_name == "client_zmqport") {
        return slsDetectorDefs::networkParameter::CLIENT_STREAMING_PORT;
    } else if (par_name == "rx_zmqport") {
        return slsDetectorDefs::networkParameter::RECEIVER_STREAMING_PORT;
    } else if (par_name == "rx_zmqip") {
        return slsDetectorDefs::networkParameter::RECEIVER_STREAMING_SRC_IP;
    } else if (par_name == "client_zmqip") {
        return slsDetectorDefs::networkParameter::CLIENT_STREAMING_SRC_IP;
    }

    throw std::runtime_error("Could not decode network parameter");
};

// slsDetectorDefs::fileFormat Detector::file///

slsDetectorDefs::dacIndex Detector::dacNameToEnum(std::string dac_name) {
    // to avoid uninitialised
    slsDetectorDefs::dacIndex dac = slsDetectorDefs::dacIndex::E_SvP;

    if (dac_name == "vsvp") {
        dac = slsDetectorDefs::dacIndex::E_SvP;
    } else if (dac_name == "vtr") {
        dac = slsDetectorDefs::dacIndex::E_Vtr;
    } else if (dac_name == "vthreshold") {
        dac = slsDetectorDefs::dacIndex::THRESHOLD;
    } else if (dac_name == "vrf") {
        dac = slsDetectorDefs::dacIndex::E_Vrf;
    } else if (dac_name == "vrs") {
        dac = slsDetectorDefs::dacIndex::E_Vrs;
    } else if (dac_name == "vsvn") {
        dac = slsDetectorDefs::dacIndex::E_SvN;
    } else if (dac_name == "vtgstv") {
        dac = slsDetectorDefs::dacIndex::E_Vtgstv;
    } else if (dac_name == "vcmp_ll") {
        dac = slsDetectorDefs::dacIndex::E_Vcmp_ll;
    } else if (dac_name == "vcmp_lr") {
        dac = slsDetectorDefs::dacIndex::E_Vcmp_lr;
    } else if (dac_name == "vcall") {
        dac = slsDetectorDefs::dacIndex::E_cal;
    } else if (dac_name == "vcmp_rl") {
        dac = slsDetectorDefs::dacIndex::E_Vcmp_rl;
    } else if (dac_name == "rxb_rb") {
        dac = slsDetectorDefs::dacIndex::E_rxb_rb;
    } else if (dac_name == "rxb_lb") {
        dac = slsDetectorDefs::dacIndex::E_rxb_lb;
    } else if (dac_name == "vcmp_rr") {
        dac = slsDetectorDefs::dacIndex::E_Vcmp_rr;
    } else if (dac_name == "vcp") {
        dac = slsDetectorDefs::dacIndex::E_Vcp;
    } else if (dac_name == "vcn") {
        dac = slsDetectorDefs::dacIndex::E_Vcn;
    } else if (dac_name == "vis") {
        dac = slsDetectorDefs::dacIndex::E_Vis;
    } else if (dac_name == "iodelay") {
        dac = slsDetectorDefs::dacIndex::IO_DELAY;
    } else if (dac_name == "v_a") {
        dac = slsDetectorDefs::dacIndex::V_POWER_A;
    } else if (dac_name == "v_b") {
        dac = slsDetectorDefs::dacIndex::V_POWER_B;
    } else if (dac_name == "v_c") {
        dac = slsDetectorDefs::dacIndex::V_POWER_C;
    } else if (dac_name == "v_d") {
        dac = slsDetectorDefs::dacIndex::V_POWER_D;
    } else if (dac_name == "v_io") {
        dac = slsDetectorDefs::dacIndex::V_POWER_IO;
    } else if (dac_name == "v_chip") {
        dac = slsDetectorDefs::dacIndex::V_POWER_CHIP;
    } else if (dac_name == "v_limit") {
        dac = slsDetectorDefs::dacIndex::V_LIMIT;
    } else if (dac_name == "temp_fpga") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_FPGA;
    } else if (dac_name == "temp_fpgaext") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_FPGAEXT;
    } else if (dac_name == "temp_10ge") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_10GE;
    } else if (dac_name == "temp_dcdc") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_DCDC;
    } else if (dac_name == "temp_sodl") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_SODL;
    } else if (dac_name == "temp_sodr") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_SODR;
    } else if (dac_name == "temp_fpgafl") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_FPGA2;
    } else if (dac_name == "temp_fpgafr") {
        dac = slsDetectorDefs::dacIndex::TEMPERATURE_FPGA3;
    } else if (dac_name == "vhighvoltage") {
        dac = slsDetectorDefs::dacIndex::HIGH_VOLTAGE;
    } else if (dac_name == "vb_comp") {
        dac = static_cast<slsDetectorDefs::dacIndex>(0);
    } else if (dac_name == "vdd_prot") {
        dac = static_cast<slsDetectorDefs::dacIndex>(1);
    } else if (dac_name == "vin_com") {
        dac = static_cast<slsDetectorDefs::dacIndex>(2);
    } else if (dac_name == "vref_prech") {
        dac = static_cast<slsDetectorDefs::dacIndex>(3);
    } else if (dac_name == "vb_pixbuff") {
        dac = static_cast<slsDetectorDefs::dacIndex>(4);
    } else if (dac_name == "vb_ds") {
        dac = static_cast<slsDetectorDefs::dacIndex>(5);
    } else if (dac_name == "vref_ds") {
        dac = static_cast<slsDetectorDefs::dacIndex>(6);
    } else if (dac_name == "vref_comp") {
        dac = static_cast<slsDetectorDefs::dacIndex>(7);
    } else if (dac_name == "dac0") {
        dac = static_cast<slsDetectorDefs::dacIndex>(0);
    } else if (dac_name == "dac1") {
        dac = static_cast<slsDetectorDefs::dacIndex>(1);
    } else if (dac_name == "dac2") {
        dac = static_cast<slsDetectorDefs::dacIndex>(2);
    } else if (dac_name == "dac3") {
        dac = static_cast<slsDetectorDefs::dacIndex>(3);
    } else if (dac_name == "dac4") {
        dac = static_cast<slsDetectorDefs::dacIndex>(4);
    } else if (dac_name == "dac5") {
        dac = static_cast<slsDetectorDefs::dacIndex>(5);
    } else if (dac_name == "dac6") {
        dac = static_cast<slsDetectorDefs::dacIndex>(6);
    } else if (dac_name == "dac7") {
        dac = static_cast<slsDetectorDefs::dacIndex>(7);
    } else if (dac_name == "dac8") {
        dac = static_cast<slsDetectorDefs::dacIndex>(8);
    } else if (dac_name == "dac9") {
        dac = static_cast<slsDetectorDefs::dacIndex>(9);
    } else if (dac_name == "dac10") {
        dac = static_cast<slsDetectorDefs::dacIndex>(10);
    } else if (dac_name == "dac11") {
        dac = static_cast<slsDetectorDefs::dacIndex>(11);
    } else if (dac_name == "dac12") {
        dac = static_cast<slsDetectorDefs::dacIndex>(12);
    } else if (dac_name == "dac13") {
        dac = static_cast<slsDetectorDefs::dacIndex>(13);
    } else if (dac_name == "dac14") {
        dac = static_cast<slsDetectorDefs::dacIndex>(14);
    } else if (dac_name == "dac15") {
        dac = static_cast<slsDetectorDefs::dacIndex>(15);
    } else if (dac_name == "dac16") {
        dac = static_cast<slsDetectorDefs::dacIndex>(16);
    } else if (dac_name == "dac17") {
        dac = static_cast<slsDetectorDefs::dacIndex>(17);
    }

    return dac;
}

std::vector<std::string> Detector::getReadoutFlags() {
    std::vector<std::string> flags;
    auto r = det.setReadOutFlags();
    if (r & slsDetectorDefs::readOutFlags::STORE_IN_RAM)
        flags.push_back("storeinram");
    if (r & slsDetectorDefs::readOutFlags::TOT_MODE)
        flags.push_back("tot");
    if (r & slsDetectorDefs::readOutFlags::CONTINOUS_RO)
        flags.push_back("continous");
    if (r & slsDetectorDefs::readOutFlags::PARALLEL)
        flags.push_back("parallel");
    if (r & slsDetectorDefs::readOutFlags::NONPARALLEL)
        flags.push_back("nonparallel");
    if (r & slsDetectorDefs::readOutFlags::SAFE)
        flags.push_back("safe");
    if (r & slsDetectorDefs::readOutFlags::DIGITAL_ONLY)
        flags.push_back("digital");
    if (r & slsDetectorDefs::readOutFlags::ANALOG_AND_DIGITAL)
        flags.push_back("analog_digital");
    if (r & slsDetectorDefs::readOutFlags::NOOVERFLOW)
        flags.push_back("nooverflow");
    if (r & slsDetectorDefs::readOutFlags::SHOW_OVERFLOW)
        flags.push_back("overflow");
    return flags;
}

// note singular
void Detector::setReadoutFlag(const std::string flag_name) {
    if (flag_name == "none")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::NORMAL_READOUT);
    else if (flag_name == "storeinram")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::STORE_IN_RAM);
    else if (flag_name == "tot")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::TOT_MODE);
    else if (flag_name == "continous")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::CONTINOUS_RO);
    else if (flag_name == "parallel")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::PARALLEL);
    else if (flag_name == "nonparallel")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::NONPARALLEL);
    else if (flag_name == "safe")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::SAFE);
    else if (flag_name == "digital")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::DIGITAL_ONLY);
    else if (flag_name == "analog_digital")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::ANALOG_AND_DIGITAL);
    else if (flag_name == "nooverflow")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::NOOVERFLOW);
    else if (flag_name == "overflow")
        det.setReadOutFlags(slsDetectorDefs::readOutFlags::SHOW_OVERFLOW);
    else
        throw std::runtime_error("Flag name not recognized");
}

std::vector<double> Detector::getRateCorrection() {
    std::vector<double> rate_corr;
    for (int i = 0; i < det.getNumberOfDetectors(); ++i) {
        rate_corr.push_back(det.getRateCorrection(i));
    }
    return rate_corr;
}

void Detector::pulseAllPixels(int n) {
    //  int pulsePixelNMove(int n=0,int x=0,int y=0);
    //  int pulsePixel(int n=0,int x=0,int y=0);

    for (int j = 0; j < 8; ++j) {
        det.pulsePixel(0, -255 + j, 0);
        for (int i = 0; i < 256; ++i) {
            det.pulsePixelNMove(n, 0, 1);
        }
    }
    return;
}
void Detector::pulseDiagonal(int n) {
    //  int pulsePixelNMove(int n=0,int x=0,int y=0);
    //  int pulsePixel(int n=0,int x=0,int y=0);

    for (int j = 20; j < 232; j += 16) {
        det.pulsePixel(0, -255, j);
        for (int i = 0; i < 8; ++i) {
            det.pulsePixelNMove(n, 1, 1);
        }
    }
    return;
}

#endif // DETECTOR_H

// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "Module.h"
#include "SharedMemory.h"
#include "sls/ClientSocket.h"
#include "sls/ToString.h"
#include "sls/Version.h"
#include "sls/bit_utils.h"
#include "sls/container_utils.h"
#include "sls/file_utils.h"
#include "sls/md5_helper.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_exceptions.h"
#include "sls/sls_detector_funcs.h"
#include "sls/string_utils.h"
#include "sls/versionAPI.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <thread>

namespace sls {

// creating new shm
Module::Module(detectorType type, int det_id, int module_index, bool verify)
    : moduleIndex(module_index), shm(det_id, module_index) {

    // ensure shared memory was not created before
    if (shm.exists()) {
        LOG(logWARNING) << "This shared memory should have been "
                           "deleted before! "
                        << shm.getName() << ". Freeing it again";
        shm.removeSharedMemory();
    }

    initSharedMemory(type, det_id, verify);
}

// opening existing shm
Module::Module(int det_id, int module_index, bool verify)
    : moduleIndex(module_index), shm(det_id, module_index) {

    // getDetectorType From shm will check if existing
    detectorType type = getDetectorTypeFromShm(det_id, verify);
    initSharedMemory(type, det_id, verify);
}

bool Module::isFixedPatternSharedMemoryCompatible() const {
    return (shm()->shmversion >= MODULE_SHMAPIVERSION);
}

std::string Module::getHostname() const { return shm()->hostname; }

void Module::setHostname(const std::string &hostname,
                         const bool initialChecks) {
    strcpy_safe(shm()->hostname, hostname.c_str());
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.close();
    try {
        checkDetectorVersionCompatibility();
        initialDetectorServerChecks();
        LOG(logINFO) << "Module Version Compatibility - Success";
    } catch (const RuntimeError &e) {
        if (!initialChecks) {
            LOG(logWARNING) << "Bypassing Initial Checks at your own risk!";
        } else {
            throw;
        }
    }
    if (shm()->detType == EIGER) {
        setActivate(true);
    }
}

int64_t Module::getFirmwareVersion() const {
    return sendToDetector<int64_t>(F_GET_FIRMWARE_VERSION);
}

int64_t
Module::getFrontEndFirmwareVersion(const fpgaPosition fpgaPosition) const {
    return sendToDetector<int64_t>(F_GET_FRONTEND_FIRMWARE_VERSION,
                                   fpgaPosition);
}

std::string Module::getControlServerLongVersion() const {
    try {
        char retval[MAX_STR_LENGTH]{};
        sendToDetector(F_GET_SERVER_VERSION, nullptr, retval);
        return retval;
    }
    // throw with old server version (sends 8 bytes)
    catch (RuntimeError &e) {
        std::string emsg = std::string(e.what());
        if (emsg.find(F_GET_SERVER_VERSION) && emsg.find("8 bytes")) {
            throwDeprecatedServerVersion();
        }
        throw;
    }
}

void Module::throwDeprecatedServerVersion() const {
    uint64_t res = sendToDetectorStop<int64_t>(F_GET_SERVER_VERSION);
    std::cout << std::endl;
    std::ostringstream os;
    os << "Detector Server (Control) version (0x" << std::hex << res
       << ") is incompatible with this client. Please update detector server!";
    throw RuntimeError(os.str());
}

std::string Module::getStopServerLongVersion() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetectorStop(F_GET_SERVER_VERSION, nullptr, retval);
    return retval;
}

std::string Module::getDetectorServerVersion() const {
    Version v(getControlServerLongVersion());
    return v.concise();
}

std::string Module::getHardwareVersion() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetector(F_GET_HARDWARE_VERSION, nullptr, retval);
    return retval;
}

std::string Module::getKernelVersion() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetector(F_GET_KERNEL_VERSION, nullptr, retval);
    return retval;
}

int64_t Module::getSerialNumber() const {
    return sendToDetector<int64_t>(F_GET_SERIAL_NUMBER);
}

int Module::getModuleId() const { return sendToDetector<int>(F_GET_MODULE_ID); }

std::string Module::getReceiverLongVersion() const {
    char retval[MAX_STR_LENGTH]{};
    sendToReceiver(F_GET_RECEIVER_VERSION, nullptr, retval);
    return retval;
}

std::string Module::getReceiverSoftwareVersion() const {
    Version v(getReceiverLongVersion());
    return v.concise();
}

// static function
slsDetectorDefs::detectorType
Module::getTypeFromDetector(const std::string &hostname, uint16_t cport) {
    LOG(logDEBUG1) << "Getting Module type ";
    ClientSocket socket("Detector", hostname, cport);
    socket.Send(F_GET_DETECTOR_TYPE);
    socket.setFnum(F_GET_DETECTOR_TYPE);
    if (socket.Receive<int>() == FAIL) {
        throw RuntimeError("Detector (" + hostname + ", " +
                           std::to_string(cport) +
                           ") returned error at getting detector type");
    }
    auto retval = socket.Receive<detectorType>();
    LOG(logDEBUG1) << "Module type is " << retval;
    return retval;
}

slsDetectorDefs::detectorType Module::getDetectorType() const {
    return shm()->detType;
}

void Module::updateNumberOfChannels() {
    if (shm()->detType == CHIPTESTBOARD ||
        shm()->detType == XILINX_CHIPTESTBOARD) {
        std::array<int, 2> retvals{};
        sendToDetector(F_GET_NUM_CHANNELS, nullptr, retvals);
        shm()->nChan.x = retvals[0];
        shm()->nChan.y = retvals[1];
    }
}

slsDetectorDefs::xy Module::getNumberOfChannels() const {
    slsDetectorDefs::xy coord{};
    coord.x = (shm()->nChan.x * shm()->nChip.x);
    coord.y = (shm()->nChan.y * shm()->nChip.y);
    return coord;
}

void Module::updateNumberOfModule(slsDetectorDefs::xy det) {
    shm()->numberOfModule = det;
    int args[2] = {shm()->numberOfModule.y, moduleIndex};
    sendToDetector(F_SET_POSITION, args, nullptr);
}

slsDetectorDefs::detectorSettings Module::getSettings() const {
    return sendToDetector<detectorSettings>(F_SET_SETTINGS, GET_FLAG);
}

void Module::setSettings(detectorSettings isettings) {
    if (shm()->detType == EIGER) {
        throw RuntimeError(
            "Cannot set settings for Eiger. Use threshold energy.");
    }
    sendToDetector<int>(F_SET_SETTINGS, isettings);
}

int Module::getThresholdEnergy() const {
    return sendToDetector<int>(F_GET_THRESHOLD_ENERGY);
}

std::array<int, 3> Module::getAllThresholdEnergy() const {
    return sendToDetector<std::array<int, 3>>(F_GET_ALL_THRESHOLD_ENERGY);
}

void Module::setThresholdEnergy(int e_eV, detectorSettings isettings,
                                bool trimbits) {
    if (shm()->detType == MYTHEN3) {
        throw RuntimeError("Mythen3 should have called with 3 energies");
    }
    // verify e_eV exists in trimEneregies[]
    if (shm()->trimEnergies.empty() || (e_eV < shm()->trimEnergies.front()) ||
        (e_eV > shm()->trimEnergies.back())) {
        throw RuntimeError("This energy " + std::to_string(e_eV) +
                           " not defined for this module!");
    }
    bool interpolate =
        std::all_of(shm()->trimEnergies.begin(), shm()->trimEnergies.end(),
                    [e_eV](const int &e) { return e != e_eV; });

    sls_detector_module myMod{shm()->detType};

    if (!interpolate) {
        std::string settingsfname = getTrimbitFilename(isettings, e_eV);
        LOG(logDEBUG1) << "Settings File is " << settingsfname;
        myMod = readSettingsFile(settingsfname, trimbits);
    } else {
        // find the trim values
        int trim1 = -1, trim2 = -1;
        for (size_t i = 0; i < shm()->trimEnergies.size(); ++i) {
            if (e_eV < shm()->trimEnergies[i]) {
                trim2 = shm()->trimEnergies[i];
                trim1 = shm()->trimEnergies[i - 1];
                break;
            }
        }
        std::string settingsfname1 = getTrimbitFilename(isettings, trim1);
        std::string settingsfname2 = getTrimbitFilename(isettings, trim2);
        LOG(logDEBUG1) << "Settings Files are " << settingsfname1 << " and "
                       << settingsfname2;
        auto myMod1 = readSettingsFile(settingsfname1, trimbits);
        auto myMod2 = readSettingsFile(settingsfname2, trimbits);
        if (myMod1.iodelay != myMod2.iodelay) {
            throw RuntimeError("setThresholdEnergyAndSettings: Iodelays do not "
                               "match between files");
        }
        myMod = interpolateTrim(&myMod1, &myMod2, e_eV, trim1, trim2, trimbits);
        myMod.iodelay = myMod1.iodelay;
        myMod.tau =
            linearInterpolation(e_eV, trim1, trim2, myMod1.tau, myMod2.tau);
    }

    myMod.reg = isettings;
    myMod.eV[0] = e_eV;
    setModule(myMod, trimbits);
    if (getSettings() != isettings) {
        throw RuntimeError("setThresholdEnergyAndSettings: Could not set "
                           "settings in Module");
    }

    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_THRESHOLD, e_eV, nullptr);
    }
}

void Module::setAllThresholdEnergy(std::array<int, 3> e_eV,
                                   detectorSettings isettings, bool trimbits) {
    if (shm()->detType != MYTHEN3) {
        throw RuntimeError("This detector should have called with 3 energies");
    }
    if (shm()->trimEnergies.empty()) {
        throw RuntimeError("Trim energies have not been defined for this "
                           "module yet! Use trimen.");
    }

    std::vector<int> energy(e_eV.begin(), e_eV.end());
    // if all energies are same
    if (allEqualTo(energy, energy[0])) {
        if (energy[0] == -1) {
            throw RuntimeError(
                "Every energy provided to set threshold energy is -1. Typo?");
        }
        energy.resize(1);
    }

    // for each threshold
    std::vector<sls_detector_module> myMods;
    for (size_t i = 0; i < energy.size(); ++i) {
        if (energy[i] == -1) {
            sls_detector_module mod = getModule();
            myMods.push_back(mod);
            continue;
        }

        sls_detector_module mod{shm()->detType};
        myMods.push_back(mod);

        // don't interpolate
        if (shm()->trimEnergies.anyEqualTo(energy[i])) {
            std::string settingsfname =
                getTrimbitFilename(isettings, energy[i]);
            LOG(logDEBUG1) << "Settings File is " << settingsfname;
            myMods[i] = readSettingsFile(settingsfname, trimbits);
        }
        // interpolate
        else {
            if (shm()->trimEnergies.size() < 2) {
                throw RuntimeError(
                    "Cannot interpolate with less than 2 trim energies");
            }
            // find the trim values
            int trim1 = -1, trim2 = -1;
            if (energy[i] < shm()->trimEnergies[0]) {
                trim1 = shm()->trimEnergies[0];
                trim2 = shm()->trimEnergies[1];
            } else if (energy[i] >
                       shm()->trimEnergies[shm()->trimEnergies.size() - 1]) {
                trim1 = shm()->trimEnergies[shm()->trimEnergies.size() - 2];
                trim2 = shm()->trimEnergies[shm()->trimEnergies.size() - 1];
            } else {
                for (size_t j = 0; j < shm()->trimEnergies.size(); ++j) {
                    // std::cout << "checking " << energy[i] << " and  "
                    //         << shm()->trimEnergies[j] << std::endl;
                    if (energy[i] < shm()->trimEnergies[j]) {
                        trim1 = shm()->trimEnergies[j - 1];
                        trim2 = shm()->trimEnergies[j];
                        break;
                    }
                }
            }
            LOG(logINFO) << "e_eV:" << energy[i] << " [" << trim1 << ", "
                         << trim2 << "]";

            std::string settingsfname1 = getTrimbitFilename(isettings, trim1);
            std::string settingsfname2 = getTrimbitFilename(isettings, trim2);
            LOG(logDEBUG1) << "Settings Files are " << settingsfname1 << " and "
                           << settingsfname2;
            auto myMod1 = readSettingsFile(settingsfname1, trimbits);
            auto myMod2 = readSettingsFile(settingsfname2, trimbits);

            myMods[i] = interpolateTrim(&myMod1, &myMod2, energy[i], trim1,
                                        trim2, trimbits);
            // csr
            if (myMod1.reg != myMod2.reg) {
                throw RuntimeError(
                    "setAllThresholdEnergy: chip shift register values do not "
                    "match between files for energy (eV) " +
                    std::to_string(energy[i]));
            }
            myMods[i].reg = myMod1.reg;
        }
    }

    sls_detector_module myMod{shm()->detType};
    myMod = myMods[0];

    // if multiple thresholds, combine
    if (myMods.size() > 1) {
        auto counters = getSetBits(getCounterMask());

        // average vtrim of enabled counters
        int sum = 0;
        for (size_t i = 0; i < counters.size(); ++i) {
            sum += myMods[counters[i]].dacs[M_VTRIM];
        }
        myMod.dacs[M_VTRIM] = sum / counters.size();

        // copy vth1, vth2 and vth3 from the correct threshold mods
        myMod.dacs[M_VTH1] = myMods[0].dacs[M_VTH1];
        myMod.dacs[M_VTH2] = myMods[1].dacs[M_VTH2];
        myMod.dacs[M_VTH3] = myMods[2].dacs[M_VTH3];

        // check if dacs are different
        for (size_t j = 0; j < 16; ++j) {
            if (j == M_VTRIM || j == M_VTH1 || j == M_VTH2 || j == M_VTH3)
                continue;

            if (myMods[0].dacs[j] != myMods[1].dacs[j]) {
                throw RuntimeError("Dac Index " + std::to_string(j) +
                                   " differs for threshold[0]:[" +
                                   std::to_string(myMods[0].dacs[j]) +
                                   "] and threshold[1]:" +
                                   std::to_string(myMods[1].dacs[j]) + "]");
            }
            if (myMods[1].dacs[j] != myMods[2].dacs[j]) {
                throw RuntimeError("Dac Index " + std::to_string(j) +
                                   " differs for threshold[1]:[" +
                                   std::to_string(myMods[1].dacs[j]) +
                                   "] and threshold[2]:" +
                                   std::to_string(myMods[2].dacs[j]) + "]");
            }
        }

        // replace correct trim values (interleaved)
        for (int i = 0; i < myMod.nchan; ++i) {
            myMod.chanregs[i] = myMods[i % 3].chanregs[i];
        }
        // csr
        if (myMods[0].reg != myMods[1].reg || myMods[1].reg != myMods[2].reg) {
            throw RuntimeError(
                "setAllThresholdEnergy: chip shift register values do not "
                "match between files for all energies");
        }
    }

    std::copy(e_eV.begin(), e_eV.end(), myMod.eV);
    LOG(logDEBUG) << "ev:" << ToString(myMod.eV);

    setModule(myMod, trimbits);
    if (getSettings() != isettings) {
        throw RuntimeError("setThresholdEnergyAndSettings: Could not set "
                           "settings in Module");
    }

    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_ALL_THRESHOLD, e_eV, nullptr);
    }
}

std::string Module::getSettingsDir() const {
    return std::string(shm()->settingsDir);
}

std::string Module::setSettingsDir(const std::string &dir) {
    strcpy_safe(shm()->settingsDir, dir.c_str());
    return shm()->settingsDir;
}

void Module::loadTrimbits(const std::string &fname) {
    // find specific file if it has detid in file name (.snxxx)
    if (shm()->detType == EIGER || shm()->detType == MYTHEN3) {
        std::ostringstream ostfn;
        ostfn << fname;
        int moduleIdWidth = 3;
        if (shm()->detType == MYTHEN3) {
            moduleIdWidth = 4;
        }
        if ((fname.find(".sn") == std::string::npos) &&
            (fname.find(".trim") == std::string::npos)) {
            ostfn << ".sn" << std::setfill('0') << std::setw(moduleIdWidth)
                  << std::dec << getModuleId();
        }
        auto myMod = readSettingsFile(ostfn.str());
        setModule(myMod);
    } else {
        throw RuntimeError("not implemented for this detector");
    }
}

void Module::saveTrimbits(const std::string &fname) {
    // find specific file if it has detid in file name (.snxxx)
    if (shm()->detType == EIGER || shm()->detType == MYTHEN3) {
        std::ostringstream ostfn;
        ostfn << fname;
        int moduleIdWidth = 3;
        if (shm()->detType == MYTHEN3) {
            moduleIdWidth = 4;
        }
        if ((fname.find(".sn") == std::string::npos) &&
            (fname.find(".trim") == std::string::npos)) {
            ostfn << ".sn" << std::setfill('0') << std::setw(moduleIdWidth)
                  << std::dec << getModuleId();
        }
        auto myMod = getModule();
        saveSettingsFile(myMod, ostfn.str());
    } else {
        throw RuntimeError("not implemented for this detector");
    }
}

int Module::getAllTrimbits() const {
    return sendToDetector<int>(F_SET_ALL_TRIMBITS, GET_FLAG);
}

void Module::setAllTrimbits(int val) {
    sendToDetector<int>(F_SET_ALL_TRIMBITS, val);
}

std::vector<int> Module::getTrimEn() const {
    if (shm()->detType != EIGER && shm()->detType != MYTHEN3) {
        throw RuntimeError("getTrimEn not implemented for this detector.");
    }
    return std::vector<int>(shm()->trimEnergies.begin(),
                            shm()->trimEnergies.end());
}

int Module::setTrimEn(const std::vector<int> &energies) {
    if (shm()->detType != EIGER && shm()->detType != MYTHEN3) {
        throw RuntimeError("setTrimEn not implemented for this detector.");
    }
    if (energies.size() > MAX_TRIMEN) {
        std::ostringstream os;
        os << "Size of trim energies: " << energies.size()
           << " exceeds what can be stored in shared memory: " << MAX_TRIMEN
           << "\n";
        throw RuntimeError(os.str());
    }
    shm()->trimEnergies = energies;
    std::sort(shm()->trimEnergies.begin(), shm()->trimEnergies.end());
    return shm()->trimEnergies.size();
}

bool Module::getFlipRows() const {
    if (shm()->detType == EIGER) {
        return sendToReceiver<int>(F_GET_FLIP_ROWS_RECEIVER);
    }
    return sendToDetector<int>(F_GET_FLIP_ROWS);
}

void Module::setFlipRows(bool value) {
    if (shm()->detType == EIGER) {
        sendToReceiver<int>(F_SET_FLIP_ROWS_RECEIVER, static_cast<int>(value));
    } else {
        sendToDetector(F_SET_FLIP_ROWS, static_cast<int>(value), nullptr);
    }
}

bool Module::isMaster() const { return sendToDetectorStop<int>(F_GET_MASTER); }

void Module::setMaster(const bool master) {
    sendToDetector(F_SET_MASTER, static_cast<int>(master), nullptr);
    sendToDetectorStop(F_SET_MASTER, static_cast<int>(master), nullptr);
}

bool Module::getSynchronization() const {
    return sendToDetector<int>(F_GET_SYNCHRONIZATION);
}

bool Module::getSynchronizationFromStopServer() const {
    return sendToDetectorStop<int>(F_GET_SYNCHRONIZATION);
}

void Module::setSynchronization(const bool value) {
    sendToDetector(F_SET_SYNCHRONIZATION, static_cast<int>(value), nullptr);
    // to deal with virtual servers as well
    // (get sync from stop server during blocking acquisition)
    sendToDetectorStop(F_SET_SYNCHRONIZATION, static_cast<int>(value), nullptr);
}

std::vector<int> Module::getBadChannels() const {
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_GET_BAD_CHANNELS);
    client.setFnum(F_GET_BAD_CHANNELS);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Detector " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
    // receive badchannels
    auto nch = client.Receive<int>();
    std::vector<int> badchannels(nch);
    if (nch > 0) {
        client.Receive(badchannels);
        for (size_t i = 0; i < badchannels.size(); ++i) {
            LOG(logDEBUG1) << i << ":" << badchannels[i];
        }
    }
    return badchannels;
}

void Module::setBadChannels(std::vector<int> list) {
    auto nch = static_cast<int>(list.size());
    LOG(logDEBUG1) << "Sending bad channels to detector, nch:" << nch;
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_BAD_CHANNELS);
    client.setFnum(F_SET_BAD_CHANNELS);
    client.Send(nch);
    if (nch > 0) {
        client.Send(list);
    }
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Detector " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
}

int Module::getRow() const { return sendToDetector<int>(F_GET_ROW); }

void Module::setRow(int value) {
    sendToDetector(F_SET_ROW, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_ROW, value, nullptr);
    }
}

int Module::getColumn() const { return sendToDetector<int>(F_GET_COLUMN); }

void Module::setColumn(int value) {
    sendToDetector(F_SET_COLUMN, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_COLUMN, value, nullptr);
    }
}

bool Module::isVirtualDetectorServer() const {
    return sendToDetector<int>(F_IS_VIRTUAL);
}

int64_t Module::getNumberOfFrames() const {
    return sendToDetector<int64_t>(F_GET_NUM_FRAMES);
}

void Module::setNumberOfFrames(int64_t value) {
    sendToDetector(F_SET_NUM_FRAMES, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_FRAMES, value, nullptr);
    }
}

int64_t Module::getNumberOfTriggers() const {
    return sendToDetector<int64_t>(F_GET_NUM_TRIGGERS);
}

void Module::setNumberOfTriggers(int64_t value) {
    sendToDetector(F_SET_NUM_TRIGGERS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_TRIGGERS, value, nullptr);
    }
}

int64_t Module::getExptime(int gateIndex) const {
    return sendToDetector<int64_t>(F_GET_EXPTIME, gateIndex);
}

void Module::setExptime(int gateIndex, int64_t value) {
    int64_t prevVal = value;
    if (shm()->detType == EIGER) {
        prevVal = getExptime(-1);
    }
    int64_t args[]{static_cast<int64_t>(gateIndex), value};
    sendToDetector(F_SET_EXPTIME, args, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_EXPTIME, args, nullptr);
    }
    if (prevVal != value) {
        updateRateCorrection();
    }
}

int64_t Module::getPeriod() const {
    return sendToDetector<int64_t>(F_GET_PERIOD);
}

void Module::setPeriod(int64_t value) {
    sendToDetector(F_SET_PERIOD, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_PERIOD, value, nullptr);
    }
}

int64_t Module::getDelayAfterTrigger() const {
    return sendToDetector<int64_t>(F_GET_DELAY_AFTER_TRIGGER);
}

void Module::setDelayAfterTrigger(int64_t value) {
    sendToDetector(F_SET_DELAY_AFTER_TRIGGER, value, nullptr);
}

int64_t Module::getNumberOfFramesLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_FRAMES_LEFT);
}

int64_t Module::getNumberOfTriggersLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_TRIGGERS_LEFT);
}

int64_t Module::getDelayAfterTriggerLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_DELAY_AFTER_TRIGGER_LEFT);
}

int64_t Module::getPeriodLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_PERIOD_LEFT);
}

int Module::getDynamicRange() const {
    return sendToDetector<int>(F_SET_DYNAMIC_RANGE, GET_FLAG);
}

void Module::setDynamicRange(int dr) {
    int prev_val = dr;
    if (shm()->detType == EIGER) {
        prev_val = getDynamicRange();
    }

    auto retval = sendToDetector<int>(F_SET_DYNAMIC_RANGE, dr);
    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_SET_RECEIVER_DYNAMIC_RANGE, retval);
    }

    // update speed
    if (shm()->detType == EIGER) {
        if (dr == 32) {
            LOG(logINFO) << "Setting Clock to Quarter Speed to cope with "
                            "Dynamic Range of 32";
            setReadoutSpeed(defs::QUARTER_SPEED);
        } else {
            LOG(logINFO) << "Setting Clock to Full Speed for Dynamic Range of "
                         << dr;
            setReadoutSpeed(defs::FULL_SPEED);
        }
        // EIGER only, update speed and rate correction when dr changes
        if (dr != prev_val) {
            updateRateCorrection();
        }
    }
}

slsDetectorDefs::timingMode Module::getTimingMode() const {
    return sendToDetector<timingMode>(F_SET_TIMING_MODE, GET_FLAG);
}

void Module::setTimingMode(timingMode value) {
    sendToDetector<int>(F_SET_TIMING_MODE, value);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_TIMING_MODE, value, nullptr);
    }
}

slsDetectorDefs::speedLevel Module::getReadoutSpeed() const {
    return sendToDetector<speedLevel>(F_GET_READOUT_SPEED);
}

void Module::setReadoutSpeed(speedLevel value) {
    sendToDetector(F_SET_READOUT_SPEED, value, nullptr);
}

int Module::getClockDivider(int clkIndex) const {
    return sendToDetector<int>(F_GET_CLOCK_DIVIDER, clkIndex);
}

void Module::setClockDivider(int clkIndex, int value) {
    int args[]{clkIndex, value};
    sendToDetector(F_SET_CLOCK_DIVIDER, args, nullptr);
}

int Module::getClockPhase(int clkIndex, bool inDegrees) const {
    int args[]{clkIndex, static_cast<int>(inDegrees)};
    return sendToDetector<int>(F_GET_CLOCK_PHASE, args);
}

void Module::setClockPhase(int clkIndex, int value, bool inDegrees) {
    int args[]{clkIndex, value, static_cast<int>(inDegrees)};
    sendToDetector(F_SET_CLOCK_PHASE, args, nullptr);
}

int Module::getMaxClockPhaseShift(int clkIndex) const {
    return sendToDetector<int>(F_GET_MAX_CLOCK_PHASE_SHIFT, clkIndex);
}

int Module::getClockFrequency(int clkIndex) const {
    return sendToDetector<int>(F_GET_CLOCK_FREQUENCY, clkIndex);
}

void Module::setClockFrequency(int clkIndex, int value) {
    int args[]{clkIndex, value};
    sendToDetector(F_SET_CLOCK_FREQUENCY, args, nullptr);
}

int Module::getDAC(dacIndex index, bool mV) const {
    int args[]{static_cast<int>(index), static_cast<int>(mV), GET_FLAG};
    return sendToDetector<int>(F_SET_DAC, args);
}
int Module::getDefaultDac(slsDetectorDefs::dacIndex index,
                          slsDetectorDefs::detectorSettings sett) {
    int args[]{static_cast<int>(index), static_cast<int>(sett)};
    return sendToDetector<int>(F_GET_DEFAULT_DAC, args);
}
void Module::setDefaultDac(slsDetectorDefs::dacIndex index, int defaultValue,
                           defs::detectorSettings sett) {
    int args[]{static_cast<int>(index), static_cast<int>(sett), defaultValue};
    return sendToDetector(F_SET_DEFAULT_DAC, args, nullptr);
}

void Module::resetToDefaultDacs(const bool hardReset) {
    sendToDetector(F_RESET_TO_DEFAULT_DACS, static_cast<int>(hardReset),
                   nullptr);
}

void Module::setDAC(int val, dacIndex index, bool mV) {
    int args[]{static_cast<int>(index), static_cast<int>(mV), val};
    sendToDetector<int>(F_SET_DAC, args);
}

bool Module::getPowerChip() const {
    return sendToDetector<int>(F_POWER_CHIP, GET_FLAG);
}

void Module::setPowerChip(bool on) {
    sendToDetector<int>(F_POWER_CHIP, static_cast<int>(on));
}

int Module::getImageTestMode() const {
    return sendToDetector<int>(F_GET_IMAGE_TEST_MODE);
}

void Module::setImageTestMode(const int value) {
    sendToDetector(F_SET_IMAGE_TEST_MODE, value, nullptr);
}

int Module::getADC(dacIndex index) const {
    switch (index) {
    case TEMPERATURE_ADC:
    case TEMPERATURE_FPGA:
    case TEMPERATURE_FPGAEXT:
    case TEMPERATURE_10GE:
    case TEMPERATURE_DCDC:
    case TEMPERATURE_SODL:
    case TEMPERATURE_SODR:
    case TEMPERATURE_FPGA2:
    case TEMPERATURE_FPGA3:
        // only the temperatures go to the control server, others need
        // configuration of adc in control server
        return sendToDetectorStop<int>(F_GET_ADC, index);

    default:
        return sendToDetector<int>(F_GET_ADC, index);
    }
}

int Module::getOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex) const {
    int args[]{static_cast<int>(index), chipIndex};
    return sendToDetector<int>(F_GET_ON_CHIP_DAC, args);
}

void Module::setOnChipDAC(slsDetectorDefs::dacIndex index, int chipIndex,
                          int value) {
    int args[]{static_cast<int>(index), chipIndex, value};
    sendToDetector(F_SET_ON_CHIP_DAC, args, nullptr);
}

slsDetectorDefs::externalSignalFlag
Module::getExternalSignalFlags(int signalIndex) const {
    return sendToDetector<slsDetectorDefs::externalSignalFlag>(
        F_GET_EXTERNAL_SIGNAL_FLAG, signalIndex);
}

void Module::setExternalSignalFlags(int signalIndex, externalSignalFlag type) {
    int args[]{signalIndex, static_cast<int>(type)};
    sendToDetector(F_SET_EXTERNAL_SIGNAL_FLAG, args, nullptr);
}

bool Module::getParallelMode() const {
    return sendToDetector<int>(F_GET_PARALLEL_MODE);
}

void Module::setParallelMode(const bool enable) {
    sendToDetector(F_SET_PARALLEL_MODE, static_cast<int>(enable), nullptr);
}

int Module::getFilterResistor() const {
    return sendToDetector<int>(F_GET_FILTER_RESISTOR);
}

void Module::setFilterResistor(int value) {
    sendToDetector(F_SET_FILTER_RESISTOR, value, nullptr);
}

defs::currentSrcParameters Module::getCurrentSource() const {
    return sendToDetector<defs::currentSrcParameters>(F_GET_CURRENT_SOURCE);
}

void Module::setCurrentSource(defs::currentSrcParameters par) {
    sendToDetector(F_SET_CURRENT_SOURCE, par, nullptr);
}

int Module::getDBITPipeline() const {
    return sendToDetector<int>(F_GET_DBIT_PIPELINE);
}

void Module::setDBITPipeline(int value) {
    sendToDetector(F_SET_DBIT_PIPELINE, value, nullptr);
}

int Module::getReadNRows() const {
    return sendToDetector<int>(F_GET_READ_N_ROWS);
}

void Module::setReadNRows(const int value) {
    sendToDetector(F_SET_READ_N_ROWS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_READ_N_ROWS, value, nullptr);
    }
}

// Acquisition

void Module::startReceiver() {
    shm()->stoppedFlag = false;
    sendToReceiver(F_START_RECEIVER);
}

void Module::stopReceiver() {
    auto rxStatusPrior = getReceiverStatus();
    sendToReceiver(F_STOP_RECEIVER, static_cast<int>(shm()->stoppedFlag),
                   nullptr);

    if (rxStatusPrior == IDLE && getReceiverStreaming()) {
        restreamStopFromReceiver();
    }
}

void Module::startAcquisition() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_ACQUISITION);
}

void Module::startReadout() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_READOUT);
}

void Module::stopAcquisition() {
    sendToDetectorStop(F_STOP_ACQUISITION);
    shm()->stoppedFlag = true;
}

void Module::restreamStopFromReceiver() {
    sendToReceiver(F_RESTREAM_STOP_FROM_RECEIVER);
}

void Module::startAndReadAll() {
    shm()->stoppedFlag = false;
    sendToDetector(F_START_AND_READ_ALL);
}

slsDetectorDefs::runStatus Module::getRunStatus() const {
    return sendToDetectorStop<runStatus>(F_GET_RUN_STATUS);
}

slsDetectorDefs::runStatus Module::getReceiverStatus() const {
    return sendToReceiver<runStatus>(F_GET_RECEIVER_STATUS);
}

double Module::getReceiverProgress() const {
    return sendToReceiver<double>(F_GET_RECEIVER_PROGRESS);
}

std::vector<int64_t> Module::getFramesCaughtByReceiver() const {
    // TODO!(Erik) Refactor
    LOG(logDEBUG1) << "Getting frames caught";
    if (shm()->useReceiverFlag) {
        auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
        client.Send(F_GET_RECEIVER_FRAMES_CAUGHT);
        client.setFnum(F_GET_RECEIVER_FRAMES_CAUGHT);
        if (client.Receive<int>() == FAIL) {
            throw ReceiverError(
                "Receiver " + std::to_string(moduleIndex) +
                " returned error: " + client.readErrorMessage());
        } else {
            auto nports = client.Receive<int>();
            std::vector<int64_t> retval(nports);
            client.Receive(retval);
            LOG(logDEBUG1) << "Frames caught of Receiver" << moduleIndex << ": "
                           << ToString(retval);
            return retval;
        }
    }
    throw RuntimeError("No receiver to get frames caught.");
}

std::vector<int64_t> Module::getNumMissingPackets() const {
    // TODO!(Erik) Refactor
    LOG(logDEBUG1) << "Getting num missing packets";
    if (shm()->useReceiverFlag) {
        auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
        client.Send(F_GET_NUM_MISSING_PACKETS);
        client.setFnum(F_GET_NUM_MISSING_PACKETS);
        if (client.Receive<int>() == FAIL) {
            throw ReceiverError(
                "Receiver " + std::to_string(moduleIndex) +
                " returned error: " + client.readErrorMessage());
        } else {
            auto nports = client.Receive<int>();
            std::vector<int64_t> retval(nports);
            client.Receive(retval);
            LOG(logDEBUG1) << "Missing packets of Receiver" << moduleIndex
                           << ": " << ToString(retval);
            return retval;
        }
    }
    throw RuntimeError("No receiver to get missing packets.");
}

std::vector<int64_t> Module::getReceiverCurrentFrameIndex() const {
    // TODO!(Erik) Refactor
    LOG(logDEBUG1) << "Getting frame index";
    if (shm()->useReceiverFlag) {
        auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
        client.Send(F_GET_RECEIVER_FRAME_INDEX);
        client.setFnum(F_GET_RECEIVER_FRAME_INDEX);
        if (client.Receive<int>() == FAIL) {
            throw ReceiverError(
                "Receiver " + std::to_string(moduleIndex) +
                " returned error: " + client.readErrorMessage());
        } else {
            auto nports = client.Receive<int>();
            std::vector<int64_t> retval(nports);
            client.Receive(retval);
            LOG(logDEBUG1) << "Frame index of Receiver" << moduleIndex << ": "
                           << ToString(retval);
            return retval;
        }
    }
    throw RuntimeError("No receiver to get frame index.");
}

uint64_t Module::getNextFrameNumber() const {
    return sendToDetector<uint64_t>(F_GET_NEXT_FRAME_NUMBER);
}

void Module::setNextFrameNumber(uint64_t value) {
    sendToDetector(F_SET_NEXT_FRAME_NUMBER, value, nullptr);
}

void Module::sendSoftwareTrigger(const bool block) {
    sendToDetectorStop(F_SOFTWARE_TRIGGER, static_cast<int>(block), nullptr);
}

defs::scanParameters Module::getScan() const {
    return sendToDetector<defs::scanParameters>(F_GET_SCAN);
}

void Module::setScan(const defs::scanParameters t) {
    auto retval = sendToDetector<int64_t>(F_SET_SCAN, t);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_SCAN, t, nullptr);
    }
    // if disabled, retval is 1, else its number of steps
    setNumberOfFrames(retval);
}

std::string Module::getScanErrorMessage() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetector(F_GET_SCAN_ERROR_MESSAGE, nullptr, retval);
    return retval;
}

// Network Configuration (Detector<->Receiver)

int Module::getNumberofUDPInterfacesFromShm() const {
    return shm()->numUDPInterfaces;
}

void Module::updateNumberofUDPInterfaces() {
    shm()->numUDPInterfaces = sendToDetector<int>(F_GET_NUM_INTERFACES);
}

void Module::setNumberofUDPInterfaces(int n) {
    sendToDetector(F_SET_NUM_INTERFACES, n, nullptr);
    shm()->numUDPInterfaces = n;
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_INTERFACES, n, nullptr);
    }
}

int Module::getSelectedUDPInterface() const {
    return sendToDetector<int>(F_GET_INTERFACE_SEL);
}

void Module::selectUDPInterface(int n) {
    sendToDetector(F_SET_INTERFACE_SEL, n, nullptr);
}

IpAddr Module::getSourceUDPIP() const {
    return sendToDetector<IpAddr>(F_GET_SOURCE_UDP_IP);
}

void Module::setSourceUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address");
    }
    sendToDetector(F_SET_SOURCE_UDP_IP, ip, nullptr);
}

IpAddr Module::getSourceUDPIP2() const {
    return sendToDetector<IpAddr>(F_GET_SOURCE_UDP_IP2);
}

void Module::setSourceUDPIP2(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid source udp ip address2");
    }
    sendToDetector(F_SET_SOURCE_UDP_IP2, ip, nullptr);
}

MacAddr Module::getSourceUDPMAC() const {
    return sendToDetector<MacAddr>(F_GET_SOURCE_UDP_MAC);
}

void Module::setSourceUDPMAC(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC, mac, nullptr);
}

MacAddr Module::getSourceUDPMAC2() const {
    return sendToDetector<MacAddr>(F_GET_SOURCE_UDP_MAC2);
}

void Module::setSourceUDPMAC2(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid source udp mac address2");
    }
    sendToDetector(F_SET_SOURCE_UDP_MAC2, mac, nullptr);
}

UdpDestination Module::getDestinationUDPList(const uint32_t entry) const {
    return sendToDetector<UdpDestination>(F_GET_DEST_UDP_LIST, entry);
}

void Module::setDestinationUDPList(const UdpDestination dest) {
    // set them in the default way so the receivers are also set up
    if (dest.entry == 0) {
        if (dest.port != 0) {
            setDestinationUDPPort(dest.port);
        }
        if (dest.ip != 0) {
            setDestinationUDPIP(dest.ip);
        }
        if (dest.mac != 0) {
            setDestinationUDPMAC(dest.mac);
        }
        if (dest.port2 != 0) {
            setDestinationUDPPort2(dest.port2);
        }
        if (dest.ip2 != 0) {
            setDestinationUDPIP2(dest.ip2);
        }
        if (dest.mac2 != 0) {
            setDestinationUDPMAC2(dest.mac2);
        }
    } else {
        sendToDetector(F_SET_DEST_UDP_LIST, dest, nullptr);
    }
}

int Module::getNumberofUDPDestinations() const {
    return sendToDetector<int>(F_GET_NUM_DEST_UDP);
}

void Module::clearUDPDestinations() { sendToDetector(F_CLEAR_ALL_UDP_DEST); }

int Module::getFirstUDPDestination() const {
    return sendToDetector<int>(F_GET_UDP_FIRST_DEST);
}

void Module::setFirstUDPDestination(const int value) {
    sendToDetector(F_SET_UDP_FIRST_DEST, value, nullptr);
}

IpAddr Module::getDestinationUDPIP() const {
    return sendToDetector<IpAddr>(F_GET_DEST_UDP_IP);
}

void Module::setDestinationUDPIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address");
    }
    if (ip.str() == LOCALHOST_IP && !isVirtualDetectorServer()) {
        throw RuntimeError("Invalid destination udp ip. Change rx_hostname "
                           "from localhost or change udp_dstip from auto?");
    }
    sendToDetector(F_SET_DEST_UDP_IP, ip, nullptr);
    if (shm()->useReceiverFlag) {
        MacAddr retval(0LU);
        sendToReceiver(F_SET_RECEIVER_UDP_IP, ip, retval);
        LOG(logINFO) << "Setting destination udp mac of Module " << moduleIndex
                     << " to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC, retval, nullptr);
    }
}

IpAddr Module::getDestinationUDPIP2() const {
    return sendToDetector<IpAddr>(F_GET_DEST_UDP_IP2);
}

void Module::setDestinationUDPIP2(const IpAddr ip) {
    LOG(logDEBUG1) << "Setting destination udp ip2 to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address2");
    }
    if (ip.str() == LOCALHOST_IP && !isVirtualDetectorServer()) {
        throw RuntimeError("Invalid destination udp ip2. Change rx_hostname "
                           "from localhost or change udp_dstip from auto?");
    }
    sendToDetector(F_SET_DEST_UDP_IP2, ip, nullptr);
    if (shm()->useReceiverFlag) {
        MacAddr retval(0LU);
        sendToReceiver(F_SET_RECEIVER_UDP_IP2, ip, retval);
        LOG(logINFO) << "Setting destination udp mac2 of Module " << moduleIndex
                     << " to " << retval;
        sendToDetector(F_SET_DEST_UDP_MAC2, retval, nullptr);
    }
}

MacAddr Module::getDestinationUDPMAC() const {
    return sendToDetector<MacAddr>(F_GET_DEST_UDP_MAC);
}

void Module::setDestinationUDPMAC(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid destination udp mac address");
    }
    sendToDetector(F_SET_DEST_UDP_MAC, mac, nullptr);
}

MacAddr Module::getDestinationUDPMAC2() const {
    return sendToDetector<MacAddr>(F_GET_DEST_UDP_MAC2);
}

void Module::setDestinationUDPMAC2(const MacAddr mac) {
    if (mac == 0) {
        throw RuntimeError("Invalid desinaion udp mac address2");
    }
    sendToDetector(F_SET_DEST_UDP_MAC2, mac, nullptr);
}

uint16_t Module::getDestinationUDPPort() const {
    return sendToDetector<uint16_t>(F_GET_DEST_UDP_PORT);
}

void Module::setDestinationUDPPort(const uint16_t port) {
    sendToDetector(F_SET_DEST_UDP_PORT, port, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT, port, nullptr);
    }
}

uint16_t Module::getDestinationUDPPort2() const {
    return sendToDetector<uint16_t>(F_GET_DEST_UDP_PORT2);
}

void Module::setDestinationUDPPort2(const uint16_t port) {
    sendToDetector(F_SET_DEST_UDP_PORT2, port, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_UDP_PORT2, port, nullptr);
    }
}

void Module::reconfigureUDPDestination() { sendToDetector(F_RECONFIGURE_UDP); }

void Module::validateUDPConfiguration() {
    sendToDetector(F_VALIDATE_UDP_CONFIG);
}

std::string Module::printReceiverConfiguration() {
    std::ostringstream os;
    os << "\n\nModule " << moduleIndex << "\nReceiver Hostname:\t"
       << getReceiverHostname();

    if (shm()->detType == JUNGFRAU || shm()->detType == MOENCH) {
        os << "\nNumber of Interfaces:\t" << getNumberofUDPInterfacesFromShm()
           << "\nSelected Interface:\t" << getSelectedUDPInterface();
    }

    os << "\nSource UDP IP:\t" << getSourceUDPIP() << "\nSource UDP MAC:\t"
       << getSourceUDPMAC() << "\nDestination UDP IP:\t"
       << getDestinationUDPIP() << "\nDestination UDP MAC:\t"
       << getDestinationUDPMAC();

    if (shm()->detType == JUNGFRAU || shm()->detType == MOENCH) {
        os << "\nSource UDP IP2:\t" << getSourceUDPIP2()
           << "\nSource UDP MAC2:\t" << getSourceUDPMAC2()
           << "\nDestination UDP IP2:\t" << getDestinationUDPIP2()
           << "\nDestination UDP MAC2:\t" << getDestinationUDPMAC2();
    }
    os << "\nDestination UDP Port:\t" << getDestinationUDPPort();
    if (shm()->detType == JUNGFRAU || shm()->detType == MOENCH ||
        shm()->detType == EIGER) {
        os << "\nDestination UDP Port2:\t" << getDestinationUDPPort2();
    }
    os << "\n";
    return os.str();
}

bool Module::getTenGiga() const {
    return sendToDetector<int>(F_ENABLE_TEN_GIGA, GET_FLAG);
}

void Module::setTenGiga(bool value) {
    auto arg = static_cast<int>(value);
    auto retval = sendToDetector<int>(F_ENABLE_TEN_GIGA, arg);
    sendToDetectorStop<int>(F_ENABLE_TEN_GIGA, arg);
    arg = retval;
    if (shm()->useReceiverFlag && arg != GET_FLAG) {
        sendToReceiver<int>(F_ENABLE_RECEIVER_TEN_GIGA, arg);
    }
}

bool Module::getTenGigaFlowControl() const {
    return sendToDetector<int>(F_GET_TEN_GIGA_FLOW_CONTROL);
}

void Module::setTenGigaFlowControl(bool enable) {
    sendToDetector(F_SET_TEN_GIGA_FLOW_CONTROL, static_cast<int>(enable),
                   nullptr);
}

int Module::getTransmissionDelayFrame() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_FRAME);
}

void Module::setTransmissionDelayFrame(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_FRAME, value, nullptr);
}

int Module::getTransmissionDelayLeft() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_LEFT);
}

void Module::setTransmissionDelayLeft(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_LEFT, value, nullptr);
}

int Module::getTransmissionDelayRight() const {
    return sendToDetector<int>(F_GET_TRANSMISSION_DELAY_RIGHT);
}

void Module::setTransmissionDelayRight(int value) {
    sendToDetector(F_SET_TRANSMISSION_DELAY_RIGHT, value, nullptr);
}

// Receiver Config

bool Module::getUseReceiverFlag() const { return shm()->useReceiverFlag; }

std::string Module::getReceiverHostname() const {
    return std::string(shm()->rxHostname);
}

void Module::setReceiverHostname(const std::string &hostname,
                                 const uint16_t port,
                                 const bool initialChecks) {
    {
        std::ostringstream oss;
        oss << "Setting up Receiver hostname with " << hostname;
        if (port != 0) {
            oss << " at port " << port;
        }
        LOG(logDEBUG1) << oss.str();
    }

    if (getRunStatus() == RUNNING) {
        throw RuntimeError("Cannot set receiver hostname. Acquisition already "
                           "running. Stop it first.");
    }

    if (hostname == "none") {
        memset(shm()->rxHostname, 0, MAX_STR_LENGTH);
        strcpy_safe(shm()->rxHostname, "none");
        shm()->useReceiverFlag = false;
        return;
    }

    strcpy_safe(shm()->rxHostname, hostname.c_str());
    if (port != 0) {
        shm()->rxTCPPort = port;
    }
    shm()->useReceiverFlag = true;

    try {
        checkReceiverVersionCompatibility();
        LOG(logINFO) << "Receiver Version Compatibility - Success";
    } catch (const RuntimeError &e) {
        if (!initialChecks) {
            LOG(logWARNING) << "Bypassing Initial Checks at your own risk!";
        } else {
            throw;
        }
    }
    // populate parameters from detector
    rxParameters retval;
    sendToDetector(F_GET_RECEIVER_PARAMETERS, nullptr, retval);

    // populate from shared memory
    retval.detType = shm()->detType;
    retval.numberOfModule.x = shm()->numberOfModule.x;
    retval.numberOfModule.y = shm()->numberOfModule.y;
    retval.moduleIndex = moduleIndex;
    memset(retval.hostname, 0, sizeof(retval.hostname));
    strcpy_safe(retval.hostname, shm()->hostname);

    MacAddr retvals[2];
    sendToReceiver(F_SETUP_RECEIVER, retval, retvals);
    // update Modules with dest mac
    if (retvals[0] != 0) {
        LOG(logINFO) << "Setting destination udp mac of Module " << moduleIndex
                     << " to " << retvals[0]
                     << ". Use udp_dstmac for custom mac.";
        sendToDetector(F_SET_DEST_UDP_MAC, retvals[0], nullptr);
    }
    if (retvals[1] != 0) {
        LOG(logINFO) << "Setting destination udp mac2 of Module " << moduleIndex
                     << " to " << retvals[1]
                     << ". Use udp_dstmac2 for custom mac.";
        sendToDetector(F_SET_DEST_UDP_MAC2, retvals[1], nullptr);
    }

    shm()->numUDPInterfaces = retval.udpInterfaces;

    // to use rx_hostname if empty
    updateClientStreamingIP();
}

uint16_t Module::getReceiverPort() const { return shm()->rxTCPPort; }

void Module::setReceiverPort(uint16_t port_number) {
    shm()->rxTCPPort = port_number;
}

int Module::getReceiverFifoDepth() const {
    return sendToReceiver<int>(F_SET_RECEIVER_FIFO_DEPTH, GET_FLAG);
}

void Module::setReceiverFifoDepth(int n_frames) {
    sendToReceiver<int>(F_SET_RECEIVER_FIFO_DEPTH, n_frames);
}

bool Module::getReceiverSilentMode() const {
    return sendToReceiver<int>(F_GET_RECEIVER_SILENT_MODE);
}

void Module::setReceiverSilentMode(bool enable) {
    sendToReceiver(F_SET_RECEIVER_SILENT_MODE, static_cast<int>(enable),
                   nullptr);
}

slsDetectorDefs::frameDiscardPolicy
Module::getReceiverFramesDiscardPolicy() const {
    return sendToReceiver<frameDiscardPolicy>(F_GET_RECEIVER_DISCARD_POLICY);
}

void Module::setReceiverFramesDiscardPolicy(frameDiscardPolicy f) {
    sendToReceiver(F_SET_RECEIVER_DISCARD_POLICY, static_cast<int>(f), nullptr);
}

bool Module::getPartialFramesPadding() const {
    return sendToReceiver<int>(F_GET_RECEIVER_PADDING);
}

void Module::setPartialFramesPadding(bool padding) {
    sendToReceiver(F_SET_RECEIVER_PADDING, static_cast<int>(padding), nullptr);
}

int Module::getReceiverUDPSocketBufferSize() const {
    int arg = GET_FLAG;
    return sendToReceiver<int>(F_RECEIVER_UDP_SOCK_BUF_SIZE, arg);
}

int Module::getReceiverRealUDPSocketBufferSize() const {
    return sendToReceiver<int>(F_RECEIVER_REAL_UDP_SOCK_BUF_SIZE);
}

void Module::setReceiverUDPSocketBufferSize(int udpsockbufsize) {
    sendToReceiver<int>(F_RECEIVER_UDP_SOCK_BUF_SIZE, udpsockbufsize);
}

bool Module::getReceiverLock() const {
    return sendToReceiver<int>(F_LOCK_RECEIVER, GET_FLAG);
}

void Module::setReceiverLock(bool lock) {
    sendToReceiver<int>(F_LOCK_RECEIVER, static_cast<int>(lock));
}

IpAddr Module::getReceiverLastClientIP() const {
    return sendToReceiver<IpAddr>(F_GET_LAST_RECEIVER_CLIENT_IP);
}

std::array<pid_t, NUM_RX_THREAD_IDS> Module::getReceiverThreadIds() const {
    return sendToReceiver<std::array<pid_t, NUM_RX_THREAD_IDS>>(
        F_GET_RECEIVER_THREAD_IDS);
}

bool Module::getRxArping() const {
    return sendToReceiver<int>(F_GET_RECEIVER_ARPING);
}

void Module::setRxArping(bool enable) {
    sendToReceiver(F_SET_RECEIVER_ARPING, static_cast<int>(enable), nullptr);
}

defs::ROI Module::getRxROI() const {
    return sendToReceiver<slsDetectorDefs::ROI>(F_RECEIVER_GET_RECEIVER_ROI);
}

void Module::setRxROI(const slsDetectorDefs::ROI arg) {
    LOG(logDEBUG) << moduleIndex << ": " << arg;
    sendToReceiver(F_RECEIVER_SET_RECEIVER_ROI, arg, nullptr);
}

void Module::setRxROIMetadata(const slsDetectorDefs::ROI arg) {
    sendToReceiver(F_RECEIVER_SET_RECEIVER_ROI_METADATA, arg, nullptr);
}

// File
slsDetectorDefs::fileFormat Module::getFileFormat() const {
    return sendToReceiver<fileFormat>(F_GET_RECEIVER_FILE_FORMAT);
}

void Module::setFileFormat(fileFormat f) {
    sendToReceiver(F_SET_RECEIVER_FILE_FORMAT, f, nullptr);
}

std::string Module::getFilePath() const {
    char ret[MAX_STR_LENGTH]{};
    sendToReceiver(F_GET_RECEIVER_FILE_PATH, nullptr, ret);
    return ret;
}

void Module::setFilePath(const std::string &path) {
    if (path.empty()) {
        throw RuntimeError("Cannot set empty file path");
    }
    char args[MAX_STR_LENGTH]{};
    strcpy_safe(args, path.c_str());
    sendToReceiver(F_SET_RECEIVER_FILE_PATH, args, nullptr);
}

std::string Module::getFileName() const {
    char buff[MAX_STR_LENGTH]{};
    sendToReceiver(F_GET_RECEIVER_FILE_NAME, nullptr, buff);
    return buff;
}

void Module::setFileName(const std::string &fname) {
    if (fname.empty()) {
        throw RuntimeError("Cannot set empty file name prefix");
    }
    char args[MAX_STR_LENGTH]{};
    strcpy_safe(args, fname.c_str());
    sendToReceiver(F_SET_RECEIVER_FILE_NAME, args, nullptr);
}

int64_t Module::getFileIndex() const {
    return sendToReceiver<int64_t>(F_GET_RECEIVER_FILE_INDEX);
}

void Module::setFileIndex(int64_t file_index) {
    sendToReceiver(F_SET_RECEIVER_FILE_INDEX, file_index, nullptr);
}

void Module::incrementFileIndex() { sendToReceiver(F_INCREMENT_FILE_INDEX); }

bool Module::getFileWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_FILE_WRITE);
}

void Module::setFileWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_FILE_WRITE, static_cast<int>(value), nullptr);
}

bool Module::getMasterFileWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_MASTER_FILE_WRITE);
}

void Module::setMasterFileWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_MASTER_FILE_WRITE, static_cast<int>(value),
                   nullptr);
}

bool Module::getFileOverWrite() const {
    return sendToReceiver<int>(F_GET_RECEIVER_OVERWRITE);
}

void Module::setFileOverWrite(bool value) {
    sendToReceiver(F_SET_RECEIVER_OVERWRITE, static_cast<int>(value), nullptr);
}

int Module::getFramesPerFile() const {
    return sendToReceiver<int>(F_GET_RECEIVER_FRAMES_PER_FILE);
}

void Module::setFramesPerFile(int n_frames) {
    sendToReceiver(F_SET_RECEIVER_FRAMES_PER_FILE, n_frames, nullptr);
}

// ZMQ Streaming Parameters (Receiver<->Client)

bool Module::getReceiverStreaming() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING);
}

void Module::setReceiverStreaming(bool enable) {
    sendToReceiver(F_SET_RECEIVER_STREAMING, static_cast<int>(enable), nullptr);
}

int Module::getReceiverStreamingFrequency() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_FREQUENCY);
}

void Module::setReceiverStreamingFrequency(int freq) {
    if (freq < 0) {
        throw RuntimeError("Invalid streaming frequency " +
                           std::to_string(freq));
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_FREQUENCY, freq, nullptr);
}

int Module::getReceiverStreamingTimer() const {
    return sendToReceiver<int>(F_RECEIVER_STREAMING_TIMER, GET_FLAG);
}

void Module::setReceiverStreamingTimer(int time_in_ms) {
    sendToReceiver<int>(F_RECEIVER_STREAMING_TIMER, time_in_ms);
}

int Module::getReceiverStreamingStartingFrame() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_START_FNUM);
}

void Module::setReceiverStreamingStartingFrame(int fnum) {
    if (fnum < 0) {
        throw RuntimeError("Invalid streaming starting frame number " +
                           std::to_string(fnum));
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_START_FNUM, fnum, nullptr);
}

uint16_t Module::getReceiverStreamingPort() const {
    return sendToReceiver<uint16_t>(F_GET_RECEIVER_STREAMING_PORT);
}

void Module::setReceiverStreamingPort(uint16_t port) {
    sendToReceiver(F_SET_RECEIVER_STREAMING_PORT, port, nullptr);
}

uint16_t Module::getClientStreamingPort() const { return shm()->zmqport; }

void Module::setClientStreamingPort(uint16_t port) { shm()->zmqport = port; }

IpAddr Module::getClientStreamingIP() const { return shm()->zmqip; }

void Module::setClientStreamingIP(const IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid client zmq ip address");
    }
    shm()->zmqip = ip;
}

int Module::getReceiverStreamingHwm() const {
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_HWM);
}

void Module::setReceiverStreamingHwm(const int limit) {
    sendToReceiver(F_SET_RECEIVER_STREAMING_HWM, limit, nullptr);
}

//  Eiger Specific

int64_t Module::getSubExptime() const {
    return sendToDetector<int64_t>(F_GET_SUB_EXPTIME);
}

void Module::setSubExptime(int64_t value) {
    int64_t prevVal = value;
    if (shm()->detType == EIGER) {
        prevVal = getSubExptime();
    }
    sendToDetector(F_SET_SUB_EXPTIME, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_SUB_EXPTIME, value, nullptr);
    }
    if (prevVal != value) {
        updateRateCorrection();
    }
}

int64_t Module::getSubDeadTime() const {
    return sendToDetector<int64_t>(F_GET_SUB_DEADTIME);
}

void Module::setSubDeadTime(int64_t value) {
    sendToDetector(F_SET_SUB_DEADTIME, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_SUB_DEADTIME, value, nullptr);
    }
}

bool Module::getOverFlowMode() const {
    return sendToDetector<int>(F_GET_OVERFLOW_MODE);
}

void Module::setOverFlowMode(const bool enable) {
    sendToDetector(F_SET_OVERFLOW_MODE, static_cast<int>(enable), nullptr);
}

int64_t Module::getRateCorrection() const {
    return sendToDetector<int64_t>(F_GET_RATE_CORRECT);
}

void Module::setDefaultRateCorrection() {
    int64_t arg = -1;
    sendToDetector(F_SET_RATE_CORRECT, arg, nullptr);
}

void Module::setRateCorrection(int64_t t) {
    sendToDetector(F_SET_RATE_CORRECT, t, nullptr);
}

void Module::sendReceiverRateCorrections(const std::vector<int64_t> &t) {
    LOG(logDEBUG) << "Sending to receiver 0 [rate corrections: " << ToString(t)
                  << ']';
    auto receiver = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    receiver.Send(F_SET_RECEIVER_RATE_CORRECT);
    receiver.setFnum(F_SET_RECEIVER_RATE_CORRECT);
    receiver.Send(static_cast<int>(t.size()));
    receiver.Send(t);
    if (receiver.Receive<int>() == FAIL) {
        throw ReceiverError("Receiver " + std::to_string(moduleIndex) +
                            " returned error: " + receiver.readErrorMessage());
    }
}

bool Module::getInterruptSubframe() const {
    return sendToDetector<int>(F_GET_INTERRUPT_SUBFRAME);
}

void Module::setInterruptSubframe(const bool enable) {
    sendToDetector(F_SET_INTERRUPT_SUBFRAME, static_cast<int>(enable), nullptr);
}

int64_t Module::getMeasuredPeriod() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASURED_PERIOD);
}

int64_t Module::getMeasuredSubFramePeriod() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASURED_SUBPERIOD);
}

bool Module::getActivate() const {
    auto retval = sendToDetector<int>(F_ACTIVATE, GET_FLAG);
    auto retval2 = sendToDetectorStop<int>(F_ACTIVATE, GET_FLAG);
    if (retval != retval2) {
        std::ostringstream oss;
        oss << "Inconsistent activate state. Control Server: " << retval
            << ". Stop Server: " << retval2;
        throw RuntimeError(oss.str());
    }
    return retval;
}

void Module::setActivate(const bool enable) {
    int arg = static_cast<int>(enable);
    auto retval = sendToDetector<int>(F_ACTIVATE, arg);
    sendToDetectorStop<int>(F_ACTIVATE, arg);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_ACTIVATE, retval, nullptr);
    }
}

bool Module::getCounterBit() const {
    return (
        !static_cast<bool>(sendToDetector<int>(F_SET_COUNTER_BIT, GET_FLAG)));
}

void Module::setCounterBit(bool cb) {
    sendToDetector<int>(F_SET_COUNTER_BIT, static_cast<int>(!cb));
}

void Module::pulsePixel(int n, int x, int y) {
    const int args[]{n, x, y};
    sendToDetector(F_PULSE_PIXEL, args, nullptr);
}

void Module::pulsePixelNMove(int n, int x, int y) {
    const int args[]{n, x, y};
    sendToDetector(F_PULSE_PIXEL_AND_MOVE, args, nullptr);
}

void Module::pulseChip(int n_pulses) {
    sendToDetector(F_PULSE_CHIP, n_pulses, nullptr);
}

bool Module::getQuad() const { return sendToDetector<int>(F_GET_QUAD) != 0; }

void Module::setQuad(const bool enable) {
    int value = enable ? 1 : 0;
    sendToDetector(F_SET_QUAD, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_QUAD, value, nullptr);
    }
}

bool Module::getDataStream(const portPosition port) const {
    return sendToDetector<int>(F_GET_DATASTREAM, static_cast<int>(port));
}

void Module::setDataStream(const portPosition port, const bool enable) {
    int args[]{static_cast<int>(port), static_cast<int>(enable)};
    sendToDetector(F_SET_DATASTREAM, args, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_DATASTREAM, args, nullptr);
    }
}

bool Module::getTop() const {
    return (static_cast<bool>(sendToDetector<int>(F_GET_TOP)));
}

void Module::setTop(bool value) {
    sendToDetector(F_SET_TOP, static_cast<int>(value), nullptr);
}

// Jungfrau/Moench Specific
double Module::getChipVersion() const {
    return (sendToDetector<int>(F_GET_CHIP_VERSION)) / 10.00;
}

int Module::getThresholdTemperature() const {
    auto retval = sendToDetectorStop<int>(F_THRESHOLD_TEMP, GET_FLAG);
    return retval / 1000;
}

void Module::setThresholdTemperature(int val) {
    if (val <= 0) {
        throw RuntimeError("Invalid threshold temperature " +
                           std::to_string(val));
    }
    val *= 1000;
    sendToDetectorStop<int>(F_THRESHOLD_TEMP, val);
}

bool Module::getTemperatureControl() const {
    return sendToDetectorStop<int>(F_TEMP_CONTROL, GET_FLAG);
}

void Module::setTemperatureControl(bool val) {
    sendToDetectorStop<int>(F_TEMP_CONTROL, static_cast<int>(val));
}

int Module::getTemperatureEvent() const {
    return sendToDetectorStop<int>(F_TEMP_EVENT, GET_FLAG);
}

void Module::resetTemperatureEvent() {
    sendToDetectorStop<int>(F_TEMP_EVENT, 0);
}

bool Module::getAutoComparatorDisableMode() const {
    return sendToDetector<int>(F_AUTO_COMP_DISABLE, GET_FLAG);
}

void Module::setAutoComparatorDisableMode(bool val) {
    sendToDetector<int>(F_AUTO_COMP_DISABLE, static_cast<int>(val));
}

int64_t Module::getComparatorDisableTime() const {
    return sendToDetector<int64_t>(F_GET_COMP_DISABLE_TIME);
}

void Module::setComparatorDisableTime(int64_t value) {
    sendToDetector(F_SET_COMP_DISABLE_TIME, value, nullptr);
}

int Module::getNumberOfAdditionalStorageCells() const {
    return sendToDetector<int>(F_GET_NUM_ADDITIONAL_STORAGE_CELLS);
}

void Module::setNumberOfAdditionalStorageCells(int value) {
    sendToDetector(F_SET_NUM_ADDITIONAL_STORAGE_CELLS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_ADD_STORAGE_CELLS, value, nullptr);
    }
}

int Module::getStorageCellStart() const {
    return sendToDetector<int>(F_STORAGE_CELL_START, GET_FLAG);
}

void Module::setStorageCellStart(int pos) {
    sendToDetector<int>(F_STORAGE_CELL_START, pos);
}

int64_t Module::getStorageCellDelay() const {
    return sendToDetector<int64_t>(F_GET_STORAGE_CELL_DELAY);
}

void Module::setStorageCellDelay(int64_t value) {
    sendToDetector(F_SET_STORAGE_CELL_DELAY, value, nullptr);
}

slsDetectorDefs::gainMode Module::getGainMode() const {
    return sendToDetector<gainMode>(F_GET_GAIN_MODE);
}

void Module::setGainMode(const slsDetectorDefs::gainMode mode) {
    sendToDetector(F_SET_GAIN_MODE, mode, nullptr);
}

int Module::getNumberOfFilterCells() const {
    return sendToDetector<int>(F_GET_NUM_FILTER_CELLS);
}

void Module::setNumberOfFilterCells(int value) {
    sendToDetector(F_SET_NUM_FILTER_CELLS, value, nullptr);
}

defs::pedestalParameters Module::getPedestalMode() const {
    return sendToDetector<defs::pedestalParameters>(F_GET_PEDESTAL_MODE);
}

void Module::setPedestalMode(const defs::pedestalParameters par) {
    sendToDetector(F_SET_PEDESTAL_MODE, par, nullptr);
    if (shm()->useReceiverFlag) {
        auto value = getNumberOfFrames();
        sendToReceiver(F_RECEIVER_SET_NUM_FRAMES, value, nullptr);
        value = getNumberOfTriggers();
        sendToReceiver(F_SET_RECEIVER_NUM_TRIGGERS, value, nullptr);
    }
}

defs::timingInfoDecoder Module::getTimingInfoDecoder() const {
    return sendToDetector<defs::timingInfoDecoder>(F_GET_TIMING_INFO_DECODER);
}

void Module::setTimingInfoDecoder(const defs::timingInfoDecoder value) {
    sendToDetector(F_SET_TIMING_INFO_DECODER, static_cast<int>(value), nullptr);
}

defs::collectionMode Module::getCollectionMode() const {
    return sendToDetector<defs::collectionMode>(F_GET_COLLECTION_MODE);
}

void Module::setCollectionMode(const defs::collectionMode value) {
    sendToDetector(F_SET_COLLECTION_MODE, static_cast<int>(value), nullptr);
}

// Gotthard Specific

slsDetectorDefs::ROI Module::getROI() const {
    return sendToDetector<slsDetectorDefs::ROI>(F_GET_ROI);
}

void Module::setROI(slsDetectorDefs::ROI arg) {
    if (arg.xmin < 0 || arg.xmax >= getNumberOfChannels().x) {
        arg.xmin = -1;
        arg.xmax = -1;
    }
    sendToDetector(F_SET_ROI, arg, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_DETECTOR_ROI, arg, nullptr);
    }
}

void Module::clearROI() { setROI(slsDetectorDefs::ROI{}); }

int64_t Module::getExptimeLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_EXPTIME_LEFT);
}

// Gotthard2 Specific

int64_t Module::getNumberOfBursts() const {
    return sendToDetector<int64_t>(F_GET_NUM_BURSTS);
}

void Module::setNumberOfBursts(int64_t value) {
    sendToDetector(F_SET_NUM_BURSTS, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_BURSTS, value, nullptr);
    }
}

int64_t Module::getBurstPeriod() const {
    return sendToDetector<int64_t>(F_GET_BURST_PERIOD);
}

void Module::setBurstPeriod(int64_t value) {
    sendToDetector(F_SET_BURST_PERIOD, value, nullptr);
}

int64_t Module::getNumberOfBurstsLeft() const {
    return sendToDetectorStop<int64_t>(F_GET_BURSTS_LEFT);
}

std::array<int, 2> Module::getInjectChannel() const {
    return sendToDetector<std::array<int, 2>>(F_GET_INJECT_CHANNEL);
}

void Module::setInjectChannel(const int offsetChannel,
                              const int incrementChannel) {
    int args[]{offsetChannel, incrementChannel};
    sendToDetector(F_SET_INJECT_CHANNEL, args, nullptr);
}

void Module::sendVetoPhoton(const int chipIndex,
                            const std::vector<int> &gainIndices,
                            const std::vector<int> &values) {
    const int nch = gainIndices.size();
    if (gainIndices.size() != values.size()) {
        throw RuntimeError("Number of Gain Indices and values do not match! "
                           "Gain Indices size: " +
                           std::to_string(gainIndices.size()) +
                           ", values size: " + std::to_string(values.size()));
    }
    LOG(logDEBUG1) << "Sending veto photon/file to detector [chip:" << chipIndex
                   << ", nch:" << nch << "]";

    const int args[]{chipIndex, nch};
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_VETO_PHOTON);
    client.setFnum(F_SET_VETO_PHOTON);
    client.Send(args);
    client.Send(gainIndices);
    client.Send(values);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Detector " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
}

void Module::getVetoPhoton(const int chipIndex,
                           const std::string &fname) const {
    LOG(logDEBUG1) << "Getting veto photon [" << chipIndex << "]\n";
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_GET_VETO_PHOTON);
    client.setFnum(F_GET_VETO_PHOTON);
    client.Send(chipIndex);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Detector " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }

    auto nch = client.Receive<int>();
    if (nch != shm()->nChan.x) {
        throw DetectorError("Could not get veto photon. Expected " +
                            std::to_string(shm()->nChan.x) + " channels, got " +
                            std::to_string(nch));
    }
    std::vector<int> gainIndices(nch);
    std::vector<int> values(nch);
    client.Receive(gainIndices);
    client.Receive(values);

    // save to file
    std::ofstream outfile(fname);
    if (!outfile) {
        throw RuntimeError("Could not create file to save veto photon");
    }
    for (int i = 0; i < nch; ++i) {
        outfile << gainIndices[i] << ' ' << values[i] << '\n';
    }
    LOG(logDEBUG1) << nch << " veto photon saved to file";
}

void Module::setVetoPhoton(const int chipIndex, const int numPhotons,
                           const int energy, const std::string &fname) {
    if (shm()->detType != GOTTHARD2) {
        throw RuntimeError(
            "Set Veto reference is not implemented for this detector");
    }
    if (chipIndex < -1 || chipIndex >= shm()->nChip.x) {
        throw RuntimeError("Could not set veto photon. Invalid chip index: " +
                           std::to_string(chipIndex));
    }
    if (numPhotons < 1) {
        throw RuntimeError(
            "Could not set veto photon. Invalid number of photons: " +
            std::to_string(numPhotons));
    }
    if (energy < 1) {
        throw RuntimeError("Could not set veto photon. Invalid energy: " +
                           std::to_string(energy));
    }
    std::ifstream infile(fname.c_str());
    if (!infile) {
        throw RuntimeError("Could not set veto photon. Could not open file: " +
                           fname);
    }
    LOG(logDEBUG1) << "Setting veto photon. Reading Gain values from file";

    const int totalEnergy = numPhotons * energy;
    std::vector<int> gainIndices;
    std::vector<int> values;

    while (infile.good()) {
        std::string line;
        getline(infile, line);
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        if (line.length() < 1) {
            continue;
        }
        std::istringstream ss(line);

        // read pedestal, gain values, gain thresholds
        double gainPedestal[3] = {-1, -1, -1};
        double gainValue[3] = {-1, -1, -1};
        int gainThreshold[2] = {-1, -1};
        ss >> gainPedestal[0] >> gainPedestal[1] >> gainPedestal[2] >>
            gainValue[0] >> gainValue[1] >> gainValue[2] >> gainThreshold[0] >>
            gainThreshold[1];
        if (ss.fail()) {
            throw RuntimeError("Could not set veto photon. Invalid pedestals, "
                               "gain values or gain thresholds for channel " +
                               std::to_string(gainIndices.size()));
        }

        // caluclate gain index from gain thresholds and threshold energy
        int gainIndex = 2;
        if (totalEnergy < gainThreshold[0]) {
            gainIndex = 0;
        } else if (totalEnergy < gainThreshold[1]) {
            gainIndex = 1;
        }
        gainIndices.push_back(gainIndex);
        // calculate ADU value
        values.push_back(gainPedestal[gainIndex] +
                         (gainValue[gainIndex] * totalEnergy));
    }
    // check size
    if ((int)gainIndices.size() != shm()->nChan.x) {
        throw RuntimeError("Could not set veto photon. Invalid number of "
                           "entries in file. Expected " +
                           std::to_string(shm()->nChan.x) + ", read " +
                           std::to_string(gainIndices.size()));
    }

    sendVetoPhoton(chipIndex, gainIndices, values);
}

void Module::setVetoReference(const int gainIndex, const int value) {
    const int args[]{gainIndex, value};
    sendToDetector(F_SET_VETO_REFERENCE, args, nullptr);
}

void Module::setVetoFile(const int chipIndex, const std::string &fname) {
    if (shm()->detType != GOTTHARD2) {
        throw RuntimeError(
            "Set Veto file is not implemented for this detector");
    }
    if (chipIndex < -1 || chipIndex >= shm()->nChip.x) {
        throw RuntimeError("Could not set veto file. Invalid chip index: " +
                           std::to_string(chipIndex));
    }

    std::ifstream input_file(fname);
    if (!input_file) {
        throw RuntimeError("Could not set veto file for chip " +
                           std::to_string(chipIndex) +
                           ". Could not open file: " + fname);
    }

    std::vector<int> gainIndices;
    std::vector<int> values;

    for (std::string line; std::getline(input_file, line);) {
        if (line.find('#') != std::string::npos) {
            line.erase(line.find('#'));
        }
        LOG(logDEBUG1) << "line after removing comments:\n\t" << line;
        if (line.length() > 1) {
            // convert command and string to a vector
            std::istringstream iss(line);
            std::string val;
            int gainIndex = -1;
            int value = -1;
            iss >> gainIndex >> val;
            if (iss.fail()) {
                throw RuntimeError("Could not set veto file. Invalid gain "
                                   "or reference value for channel " +
                                   std::to_string(gainIndices.size()));
            }
            try {
                value = StringTo<int>(val);
            } catch (...) {
                throw RuntimeError("Could not set veto file. Invalid value " +
                                   val + " for channel " +
                                   std::to_string(gainIndices.size()));
            }
            gainIndices.push_back(gainIndex);
            values.push_back(value);
        }
    }
    // check size
    if ((int)gainIndices.size() != shm()->nChan.x) {
        throw RuntimeError("Could not set veto file. Invalid number of "
                           "entries in file. Expected " +
                           std::to_string(shm()->nChan.x) + ", read " +
                           std::to_string(gainIndices.size()));
    }

    sendVetoPhoton(chipIndex, gainIndices, values);
}

slsDetectorDefs::burstMode Module::getBurstMode() const {
    return sendToDetector<burstMode>(F_GET_BURST_MODE);
}

void Module::setBurstMode(slsDetectorDefs::burstMode value) {
    sendToDetector(F_SET_BURST_MODE, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_BURST_MODE, value, nullptr);
    }
}

bool Module::getCDSGain() const { return sendToDetector<int>(F_GET_CDS_GAIN); }

void Module::setCDSGain(bool value) {
    sendToDetector(F_SET_CDS_GAIN, static_cast<int>(value), nullptr);
}

slsDetectorDefs::timingSourceType Module::getTimingSource() const {
    return sendToDetector<timingSourceType>(F_GET_TIMING_SOURCE);
}

void Module::setTimingSource(slsDetectorDefs::timingSourceType value) {
    sendToDetector(F_SET_TIMING_SOURCE, static_cast<int>(value), nullptr);
}

bool Module::getVeto() const { return sendToDetector<int>(F_GET_VETO); }

void Module::setVeto(bool enable) {
    sendToDetector(F_SET_VETO, static_cast<int>(enable), nullptr);
}

bool Module::getVetoStream() const {
    return (sendToDetector<int>(F_GET_VETO_STREAM));
}

void Module::setVetoStream(const bool value) {
    sendToDetector(F_SET_VETO_STREAM, static_cast<int>(value), nullptr);
}

slsDetectorDefs::vetoAlgorithm Module::getVetoAlgorithm(
    const slsDetectorDefs::streamingInterface interface) const {
    return sendToDetector<vetoAlgorithm>(F_GET_VETO_ALGORITHM,
                                         static_cast<int>(interface));
}

void Module::setVetoAlgorithm(
    const slsDetectorDefs::vetoAlgorithm alg,
    const slsDetectorDefs::streamingInterface interface) {
    int args[]{static_cast<int>(alg), static_cast<int>(interface)};
    sendToDetector(F_SET_VETO_ALGORITHM, args, nullptr);
}

int Module::getADCConfiguration(const int chipIndex, const int adcIndex) const {
    int args[]{chipIndex, adcIndex};
    return sendToDetector<int>(F_GET_ADC_CONFIGURATION, args);
}

void Module::setADCConfiguration(const int chipIndex, const int adcIndex,
                                 int value) {
    int args[]{chipIndex, adcIndex, value};
    sendToDetector(F_SET_ADC_CONFIGURATION, args, nullptr);
}

// Mythen3 Specific

uint32_t Module::getCounterMask() const {
    return sendToDetector<uint32_t>(F_GET_COUNTER_MASK);
}

void Module::setCounterMask(uint32_t countermask) {
    sendToDetector(F_SET_COUNTER_MASK, countermask, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_COUNTER_MASK, countermask, nullptr);
    }
}

int Module::getNumberOfGates() const {
    return sendToDetector<int>(F_GET_NUM_GATES);
}

void Module::setNumberOfGates(int value) {
    sendToDetector(F_SET_NUM_GATES, value, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_NUM_GATES, value, nullptr);
    }
}

std::array<time::ns, 3> Module::getExptimeForAllGates() const {
    return sendToDetector<std::array<time::ns, 3>>(F_GET_EXPTIME_ALL_GATES);
}

int64_t Module::getGateDelay(int gateIndex) const {
    return sendToDetector<int64_t>(F_GET_GATE_DELAY, gateIndex);
}

void Module::setGateDelay(int gateIndex, int64_t value) {
    int64_t args[]{static_cast<int64_t>(gateIndex), value};
    sendToDetector(F_SET_GATE_DELAY, args, nullptr);
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_SET_RECEIVER_GATE_DELAY, args, nullptr);
    }
}

std::array<time::ns, 3> Module::getGateDelayForAllGates() const {
    return sendToDetector<std::array<time::ns, 3>>(F_GET_GATE_DELAY_ALL_GATES);
}

int Module::getChipStatusRegister() const {
    return sendToDetector<int>(F_GET_CSR);
}

void Module::setGainCaps(int caps) {
    sendToDetector(F_SET_GAIN_CAPS, caps, nullptr);
}

int Module::getGainCaps() { return sendToDetector<int>(F_GET_GAIN_CAPS); }

defs::polarity Module::getPolarity() const {
    return sendToDetector<defs::polarity>(F_GET_POLARITY);
}

void Module::setPolarity(const defs::polarity value) {
    sendToDetector(F_SET_POLARITY, static_cast<int>(value), nullptr);
}

bool Module::getInterpolation() const {
    return sendToDetector<int>(F_GET_INTERPOLATION);
}

void Module::setInterpolation(const bool enable) {
    sendToDetector(F_SET_INTERPOLATION, static_cast<int>(enable), nullptr);
    int mask = getCounterMask();
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_COUNTER_MASK, mask, nullptr);
    }
}

bool Module::getPumpProbe() const {
    return sendToDetector<int>(F_GET_PUMP_PROBE);
}

void Module::setPumpProbe(const bool enable) {
    sendToDetector(F_SET_PUMP_PROBE, static_cast<int>(enable), nullptr);
}

bool Module::getAnalogPulsing() const {
    return sendToDetector<int>(F_GET_ANALOG_PULSING);
}

void Module::setAnalogPulsing(const bool enable) {
    sendToDetector(F_SET_ANALOG_PULSING, static_cast<int>(enable), nullptr);
}

bool Module::getDigitalPulsing() const {
    return sendToDetector<int>(F_GET_DIGITAL_PULSING);
}

void Module::setDigitalPulsing(const bool enable) {
    sendToDetector(F_SET_DIGITAL_PULSING, static_cast<int>(enable), nullptr);
}

// CTB Specific
int Module::getNumberOfAnalogSamples() const {
    return sendToDetector<int>(F_GET_NUM_ANALOG_SAMPLES);
}

void Module::setNumberOfAnalogSamples(int value) {
    sendToDetector(F_SET_NUM_ANALOG_SAMPLES, value, nullptr);
    // update #nchan, as it depends on #samples, adcmask
    updateNumberOfChannels();
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_ANALOG_SAMPLES, value, nullptr);
    }
}

uint32_t Module::getADCEnableMask() const {
    return sendToDetector<uint32_t>(F_GET_ADC_ENABLE_MASK);
}

void Module::setADCEnableMask(uint32_t mask) {
    sendToDetector(F_SET_ADC_ENABLE_MASK, mask, nullptr);
    // update #nchan, as it depends on #samples, adcmask,
    updateNumberOfChannels();

    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_RECEIVER_SET_ADC_MASK, mask);
    }
}

uint32_t Module::getTenGigaADCEnableMask() const {
    return sendToDetector<uint32_t>(F_GET_ADC_ENABLE_MASK_10G);
}

void Module::setTenGigaADCEnableMask(uint32_t mask) {
    sendToDetector(F_SET_ADC_ENABLE_MASK_10G, mask, nullptr);
    updateNumberOfChannels(); // depends on samples and adcmask

    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_RECEIVER_SET_ADC_MASK_10G, mask);
    }
}

uint32_t Module::getTransceiverEnableMask() const {
    return sendToDetector<uint32_t>(F_GET_TRANSCEIVER_ENABLE_MASK);
}

void Module::setTransceiverEnableMask(uint32_t mask) {
    sendToDetector(F_SET_TRANSCEIVER_ENABLE_MASK, mask, nullptr);
    // update #nchan, as it depends on #samples, adcmask,
    updateNumberOfChannels();

    if (shm()->useReceiverFlag) {
        sendToReceiver<int>(F_RECEIVER_SET_TRANSCEIVER_MASK, mask);
    }
}
// CTB Specific

int Module::getNumberOfDigitalSamples() const {
    return sendToDetector<int>(F_GET_NUM_DIGITAL_SAMPLES);
}

void Module::setNumberOfDigitalSamples(int value) {
    sendToDetector(F_SET_NUM_DIGITAL_SAMPLES, value, nullptr);
    updateNumberOfChannels(); // depends on samples and adcmask
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_DIGITAL_SAMPLES, value, nullptr);
    }
}

int Module::getNumberOfTransceiverSamples() const {
    return sendToDetector<int>(F_GET_NUM_TRANSCEIVER_SAMPLES);
}

void Module::setNumberOfTransceiverSamples(int value) {
    sendToDetector(F_SET_NUM_TRANSCEIVER_SAMPLES, value, nullptr);
    updateNumberOfChannels(); // depends on samples and adcmask
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_NUM_TRANSCEIVER_SAMPLES, value, nullptr);
    }
}

slsDetectorDefs::readoutMode Module::getReadoutMode() const {
    return sendToDetector<readoutMode>(F_GET_READOUT_MODE);
}

void Module::setReadoutMode(const slsDetectorDefs::readoutMode mode) {
    auto arg = static_cast<uint32_t>(mode); // TODO! unit?
    sendToDetector(F_SET_READOUT_MODE, arg, nullptr);
    sendToDetectorStop(F_SET_READOUT_MODE, arg, nullptr);
    // update #nchan, as it depends on #samples, adcmask,
    if (shm()->detType == CHIPTESTBOARD ||
        shm()->detType == XILINX_CHIPTESTBOARD) {
        updateNumberOfChannels();
    }
    if (shm()->useReceiverFlag) {
        sendToReceiver(F_RECEIVER_SET_READOUT_MODE, mode, nullptr);
    }
}

int Module::getExternalSamplingSource() {
    return setExternalSamplingSource(GET_FLAG);
}

int Module::setExternalSamplingSource(int value) {
    return sendToDetector<int>(F_EXTERNAL_SAMPLING_SOURCE, value);
}

bool Module::getExternalSampling() const {
    return sendToDetector<int>(F_EXTERNAL_SAMPLING, GET_FLAG);
}

void Module::setExternalSampling(bool value) {
    sendToDetector<int>(F_EXTERNAL_SAMPLING, static_cast<int>(value));
}

std::vector<int> Module::getReceiverDbitList() const {
    return sendToReceiver<StaticVector<int, MAX_RX_DBIT>>(
        F_GET_RECEIVER_DBIT_LIST);
}

void Module::setReceiverDbitList(std::vector<int> list) {
    LOG(logDEBUG1) << "Setting Receiver Dbit List";
    if (list.size() > 64) {
        throw RuntimeError("Dbit list size cannot be greater than 64\n");
    }
    for (auto &it : list) {
        if (it < 0 || it > 63) {
            throw RuntimeError("Dbit list value must be between 0 and 63\n");
        }
    }
    std::sort(begin(list), end(list));
    auto last = std::unique(begin(list), end(list));
    list.erase(last, list.end());

    StaticVector<int, MAX_RX_DBIT> arg = list;
    sendToReceiver(F_SET_RECEIVER_DBIT_LIST, arg, nullptr);
}

int Module::getReceiverDbitOffset() const {
    return sendToReceiver<int>(F_GET_RECEIVER_DBIT_OFFSET);
}

void Module::setReceiverDbitOffset(int value) {
    sendToReceiver(F_SET_RECEIVER_DBIT_OFFSET, value, nullptr);
}

void Module::setDigitalIODelay(uint64_t pinMask, int delay) {
    uint64_t args[]{pinMask, static_cast<uint64_t>(delay)};
    sendToDetector(F_DIGITAL_IO_DELAY, args, nullptr);
}

bool Module::getLEDEnable() const {
    return sendToDetector<int>(F_LED, GET_FLAG);
}

void Module::setLEDEnable(bool enable) {
    sendToDetector<int>(F_LED, static_cast<int>(enable));
}

// Xilinx Ctb Specific
void Module::configureTransceiver() {
    sendToDetector(F_CONFIG_TRANSCEIVER);
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): Transceiver configured successfully!";
}

// Pattern
std::string Module::getPatterFileName() const {
    char retval[MAX_STR_LENGTH]{};
    sendToDetector(F_GET_PATTERN_FILE_NAME, nullptr, retval);
    return retval;
}

void Module::setPattern(const Pattern &pat, const std::string &fname) {
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_PATTERN);
    client.setFnum(F_SET_PATTERN);
    client.Send(pat.data(), pat.size());
    char args[MAX_STR_LENGTH]{};
    strcpy_safe(args, fname.c_str());
    client.Send(args);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Detector " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
}

Pattern Module::getPattern() {
    Pattern pat;
    sendToDetector(F_GET_PATTERN, nullptr, 0, pat.data(), pat.size());
    return pat;
}

void Module::loadDefaultPattern() { sendToDetector(F_LOAD_DEFAULT_PATTERN); }

uint64_t Module::getPatternIOControl() const {
    return sendToDetector<uint64_t>(F_GET_PATTERN_IO_CONTROL);
}

void Module::setPatternIOControl(uint64_t word) {
    sendToDetector(F_SET_PATTERN_IO_CONTROL, word, nullptr);
}

uint64_t Module::getPatternWord(int addr) const {
    uint64_t args[]{static_cast<uint64_t>(addr),
                    static_cast<uint64_t>(GET_FLAG)};
    return sendToDetector<uint64_t>(F_SET_PATTERN_WORD, args);
}

void Module::setPatternWord(int addr, uint64_t word) {
    uint64_t args[]{static_cast<uint64_t>(addr), word};
    sendToDetector<uint64_t>(F_SET_PATTERN_WORD, args);
}

std::array<int, 2> Module::getPatternLoopAddresses(int level) const {
    int args[]{level, GET_FLAG, GET_FLAG};
    std::array<int, 2> retvals{};
    sendToDetector(F_SET_PATTERN_LOOP_ADDRESSES, args, retvals);
    return retvals;
}

void Module::setPatternLoopAddresses(int level, int start, int stop) {
    int args[]{level, start, stop};
    std::array<int, 2> retvals{};
    sendToDetector(F_SET_PATTERN_LOOP_ADDRESSES, args, retvals);
}

int Module::getPatternLoopCycles(int level) const {
    int args[]{level, GET_FLAG};
    return sendToDetector<int>(F_SET_PATTERN_LOOP_CYCLES, args);
}

void Module::setPatternLoopCycles(int level, int n) {
    int args[]{level, n};
    sendToDetector<int>(F_SET_PATTERN_LOOP_CYCLES, args);
}

int Module::getPatternWaitAddr(int level) const {
    int args[]{level, GET_FLAG};
    return sendToDetector<int>(F_SET_PATTERN_WAIT_ADDR, args);
}

void Module::setPatternWaitAddr(int level, int addr) {
    int args[]{level, addr};
    sendToDetector<int>(F_SET_PATTERN_WAIT_ADDR, args);
}

uint64_t Module::getPatternWaitTime(int level) const {
    uint64_t args[]{static_cast<uint64_t>(level),
                    static_cast<uint64_t>(GET_FLAG)};
    return sendToDetector<uint64_t>(F_SET_PATTERN_WAIT_TIME, args);
}

void Module::setPatternWaitTime(int level, uint64_t t) {
    uint64_t args[]{static_cast<uint64_t>(level), t};
    sendToDetector<uint64_t>(F_SET_PATTERN_WAIT_TIME, args);
}

uint64_t Module::getPatternMask() const {
    return sendToDetector<uint64_t>(F_GET_PATTERN_MASK);
}

void Module::setPatternMask(uint64_t mask) {
    sendToDetector(F_SET_PATTERN_MASK, mask, nullptr);
}

uint64_t Module::getPatternBitMask() const {
    return sendToDetector<uint64_t>(F_GET_PATTERN_BIT_MASK);
}

void Module::setPatternBitMask(uint64_t mask) {
    sendToDetector(F_SET_PATTERN_BIT_MASK, mask, nullptr);
}

void Module::startPattern() { sendToDetector(F_START_PATTERN); }

// Json Header specific

std::map<std::string, std::string> Module::getAdditionalJsonHeader() const {
    // TODO, refactor this function with a more robust sending.
    // Now assuming whitespace separated key value
    if (!shm()->useReceiverFlag) {
        throw RuntimeError("Set rx_hostname first to use receiver parameters "
                           "(zmq json header)");
    }
    auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    client.Send(F_GET_ADDITIONAL_JSON_HEADER);
    client.setFnum(F_GET_ADDITIONAL_JSON_HEADER);
    if (client.Receive<int>() == FAIL) {
        throw ReceiverError("Receiver " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    } else {
        auto size = client.Receive<int>();
        std::string buff(size, '\0');
        std::map<std::string, std::string> retval;
        if (size > 0) {
            client.Receive(&buff[0], buff.size());
            std::istringstream iss(buff);
            std::string key, value;
            while (iss >> key) {
                iss >> value;
                retval[key] = value;
            }
        }
        LOG(logDEBUG) << "Getting additional json header " << ToString(retval);
        return retval;
    }
}

void Module::setAdditionalJsonHeader(
    const std::map<std::string, std::string> &jsonHeader) {
    if (!shm()->useReceiverFlag) {
        throw RuntimeError("Set rx_hostname first to use receiver parameters "
                           "(zmq json header)");
    }
    for (auto &it : jsonHeader) {
        if (it.first.empty() || it.first.length() > SHORT_STR_LENGTH ||
            it.second.length() > SHORT_STR_LENGTH) {
            throw RuntimeError(
                it.first + " or " + it.second +
                " pair has invalid size. "
                "Key cannot be empty. Both can have max 20 characters");
        }
    }
    std::ostringstream oss;
    for (auto &it : jsonHeader)
        oss << it.first << ' ' << it.second << ' ';
    auto buff = oss.str();
    const auto size = static_cast<int>(buff.size());
    LOG(logDEBUG) << "Sending to receiver additional json header "
                  << ToString(jsonHeader);
    auto client = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    client.Send(F_SET_ADDITIONAL_JSON_HEADER);
    client.setFnum(F_SET_ADDITIONAL_JSON_HEADER);
    client.Send(size);
    if (size > 0)
        client.Send(&buff[0], buff.size());

    if (client.Receive<int>() == FAIL) {
        throw ReceiverError("Receiver " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
}

std::string Module::getAdditionalJsonParameter(const std::string &key) const {
    char arg[SHORT_STR_LENGTH]{};
    strcpy_safe(arg, key.c_str());
    char retval[SHORT_STR_LENGTH]{};
    sendToReceiver(F_GET_ADDITIONAL_JSON_PARAMETER, arg, retval);
    return retval;
}

void Module::setAdditionalJsonParameter(const std::string &key,
                                        const std::string &value) {
    if (key.empty() || key.length() > SHORT_STR_LENGTH ||
        value.length() > SHORT_STR_LENGTH) {
        throw RuntimeError(
            key + " or " + value +
            " pair has invalid size. "
            "Key cannot be empty. Both can have max 2 characters");
    }
    char args[2][SHORT_STR_LENGTH]{};
    strcpy_safe(args[0], key.c_str());
    strcpy_safe(args[1], value.c_str());
    sendToReceiver(F_SET_ADDITIONAL_JSON_PARAMETER, args, nullptr);
}

// Advanced

int Module::getADCPipeline() const {
    return sendToDetector<int>(F_GET_ADC_PIPELINE);
}

void Module::setADCPipeline(int value) {
    sendToDetector(F_SET_ADC_PIPELINE, value, nullptr);
}

void Module::programFPGA(std::vector<char> buffer,
                         const bool forceDeleteNormalFile) {
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case CHIPTESTBOARD:
        sendProgram(true, buffer, F_PROGRAM_FPGA, "Update Firmware", "",
                    forceDeleteNormalFile);
        break;
    case MYTHEN3:
    case GOTTHARD2:
        sendProgram(false, buffer, F_PROGRAM_FPGA, "Update Firmware");
        break;
    default:
        throw RuntimeError("Updating Firmware via the package is not "
                           "implemented for this detector");
    }
}

void Module::resetFPGA() { sendToDetector(F_RESET_FPGA); }

void Module::updateDetectorServer(std::vector<char> buffer,
                                  const std::string &serverName) {
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case CHIPTESTBOARD:
        sendProgram(true, buffer, F_UPDATE_DETECTOR_SERVER,
                    "Update Detector Server (no tftp)", serverName);
        break;
    case MYTHEN3:
    case GOTTHARD2:
    case EIGER:
        sendProgram(false, buffer, F_UPDATE_DETECTOR_SERVER,
                    "Update Detector Server (no tftp)", serverName);
        break;
    default:
        throw RuntimeError(
            "Updating DetectorServer via the package is not implemented "
            "for this detector");
    }
}

void Module::updateKernel(std::vector<char> buffer) {
    switch (shm()->detType) {
    case JUNGFRAU:
    case MOENCH:
    case CHIPTESTBOARD:
        sendProgram(true, buffer, F_UPDATE_KERNEL, "Update Kernel");
        break;
    case MYTHEN3:
    case GOTTHARD2:
        sendProgram(false, buffer, F_UPDATE_KERNEL, "Update Kernel");
        break;
    default:
        throw RuntimeError("Updating Kernel via the package is not implemented "
                           "for this detector");
    }
}

void Module::rebootController() {
    sendToDetector(F_REBOOT_CONTROLLER);
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): Controller rebooted successfully!";
}

bool Module::getUpdateMode() const {
    return sendToDetector<int>(F_GET_UPDATE_MODE);
}

void Module::setUpdateMode(const bool updatemode) {
    sendToDetector(F_SET_UPDATE_MODE, static_cast<int>(updatemode), nullptr);
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): Update Mode set to " << updatemode << "!";
}

uint32_t Module::readRegister(uint32_t addr) const {
    return sendToDetectorStop<uint32_t>(F_READ_REGISTER, addr);
}

void Module::writeRegister(uint32_t addr, uint32_t val, bool validate) {
    uint32_t args[]{addr, val, static_cast<uint32_t>(validate)};
    return sendToDetectorStop(F_WRITE_REGISTER, args, nullptr);
}

void Module::setBit(uint32_t addr, int n, bool validate) {
    uint32_t args[] = {addr, static_cast<uint32_t>(n),
                       static_cast<uint32_t>(validate)};
    sendToDetectorStop(F_SET_BIT, args, nullptr);
}

void Module::clearBit(uint32_t addr, int n, bool validate) {
    uint32_t args[] = {addr, static_cast<uint32_t>(n),
                       static_cast<uint32_t>(validate)};
    sendToDetectorStop(F_CLEAR_BIT, args, nullptr);
}

int Module::getBit(uint32_t addr, int n) {
    uint32_t args[2] = {addr, static_cast<uint32_t>(n)};
    return sendToDetectorStop<int>(F_GET_BIT, args);
}

void Module::executeFirmwareTest() { sendToDetector(F_SET_FIRMWARE_TEST); }

void Module::executeBusTest() { sendToDetector(F_SET_BUS_TEST); }

void Module::writeAdcRegister(uint32_t addr, uint32_t val) {
    uint32_t args[]{addr, val};
    sendToDetector(F_WRITE_ADC_REG, args, nullptr);
}

uint32_t Module::getADCInvert() const {
    return sendToDetector<uint32_t>(F_GET_ADC_INVERT);
}

void Module::setADCInvert(uint32_t value) {
    sendToDetector(F_SET_ADC_INVERT, value, nullptr);
}

// Insignificant
uint16_t Module::getControlPort() const { return shm()->controlPort; }

void Module::setControlPort(uint16_t port_number) {
    shm()->controlPort = port_number;
}

uint16_t Module::getStopPort() const { return shm()->stopPort; }

void Module::setStopPort(uint16_t port_number) {
    shm()->stopPort = port_number;
}

bool Module::getLockDetector() const {
    return sendToDetector<int>(F_LOCK_SERVER, GET_FLAG);
}

void Module::setLockDetector(bool lock) {
    sendToDetector<int>(F_LOCK_SERVER, static_cast<int>(lock));
}

IpAddr Module::getLastClientIP() const {
    return sendToDetector<IpAddr>(F_GET_LAST_CLIENT_IP);
}

std::string Module::executeCommand(const std::string &cmd) {
    char arg[MAX_STR_LENGTH]{};
    char retval[MAX_STR_LENGTH]{};
    strcpy_safe(arg, cmd.c_str());
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): Sending command " << cmd;
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_EXEC_COMMAND);
    client.setFnum(F_EXEC_COMMAND);
    client.Send(arg);
    if (client.Receive<int>() == FAIL) {
        std::cout << '\n';
        std::ostringstream os;
        os << "Module " << moduleIndex << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw DetectorError(os.str());
    }
    client.Receive(retval);
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): command executed";
    return retval;
}

int64_t Module::getNumberOfFramesFromStart() const {
    return sendToDetectorStop<int64_t>(F_GET_FRAMES_FROM_START);
}

int64_t Module::getActualTime() const {
    return sendToDetectorStop<int64_t>(F_GET_ACTUAL_TIME);
}

int64_t Module::getMeasurementTime() const {
    return sendToDetectorStop<int64_t>(F_GET_MEASUREMENT_TIME);
}

// private

void Module::checkArgs(const void *args, size_t args_size, void *retval,
                       size_t retval_size) const {
    if (args == nullptr && args_size != 0)
        throw RuntimeError(
            "Passed nullptr as args to Send function but size is not 0");
    if (args != nullptr && args_size == 0)
        throw RuntimeError(
            "Passed size 0 to Send function but args is not nullptr");
    if (retval == nullptr && retval_size != 0)
        throw RuntimeError(
            "Passed nullptr as retval to Send function but size is not 0");
    if (retval != nullptr && retval_size == 0)
        throw RuntimeError(
            "Passed size 0 to Send function but retval is not nullptr");
}

// Macro to check arguments passed to send to receiver and send to detector
// Should detect and fail on pointer types and on nullptr_t
#define STATIC_ASSERT_ARG(ARG, DST)                                            \
    static_assert(!std::is_pointer<ARG>::value,                                \
                  "Pointer type is incompatible with templated " DST);         \
    static_assert(!std::is_same<ARG, std::nullptr_t>::value,                   \
                  "nullptr_t type is incompatible with templated " DST);

void Module::sendToDetector(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the detector
    // the other versions use templates to deduce sizes and create
    // the return type
    checkArgs(args, args_size, retval, retval_size);
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    client.close();
}

void Module::sendToDetector(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, args_size,
                                                      retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToDetector(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    sendToDetector(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Arg, typename Ret>
void Module::sendToDetector(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, retval);
}

template <typename Arg>
void Module::sendToDetector(int fnum, const Arg &args, std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    sendToDetector(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToDetector(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToDetector(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToDetector(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    sendToDetector(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Ret>
void Module::sendToDetector(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetector(fnum, nullptr, retval);
}

void Module::sendToDetector(int fnum) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << "]";
    sendToDetector(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToDetector(int fnum) {
    static_cast<const Module &>(*this).sendToDetector(fnum);
}

template <typename Ret> Ret Module::sendToDetector(int fnum) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    Ret retval{};
    sendToDetector(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret> Ret Module::sendToDetector(int fnum) {
    return static_cast<const Module &>(*this).sendToDetector<Ret>(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToDetector(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetector")
    STATIC_ASSERT_ARG(Ret, "sendToDetector")
    Ret retval{};
    sendToDetector(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToDetector(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToDetector<Ret>(fnum, args);
}

//---------------------------------------------------------- sendToDetectorStop

void Module::sendToDetectorStop(int fnum, const void *args, size_t args_size,
                                void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the detector stop
    // the other versions use templates to deduce sizes and create
    // the return type
    checkArgs(args, args_size, retval, retval_size);
    auto stop = DetectorSocket(shm()->hostname, shm()->stopPort);
    stop.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    stop.close();
}

void Module::sendToDetectorStop(int fnum, const void *args, size_t args_size,
                                void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, args_size,
                                                          retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToDetectorStop(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Arg, typename Ret>
void Module::sendToDetectorStop(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, retval);
}

template <typename Arg>
void Module::sendToDetectorStop(int fnum, const Arg &args,
                                std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    sendToDetectorStop(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToDetectorStop(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
}

template <typename Ret>
void Module::sendToDetectorStop(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum, nullptr,
                                                          retval);
}

void Module::sendToDetectorStop(int fnum) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, nullptr, 0]";
    sendToDetectorStop(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToDetectorStop(int fnum) {
    static_cast<const Module &>(*this).sendToDetectorStop(fnum);
}

template <typename Ret> Ret Module::sendToDetectorStop(int fnum) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    Ret retval{};
    sendToDetectorStop(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret> Ret Module::sendToDetectorStop(int fnum) {
    return static_cast<const Module &>(*this).sendToDetectorStop<Ret>(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToDetectorStop(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending to Stop: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToDetectorStop")
    STATIC_ASSERT_ARG(Ret, "sendToDetectorStop")
    Ret retval{};
    sendToDetectorStop(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToDetectorStop(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToDetectorStop<Ret>(fnum,
                                                                      args);
}

//-------------------------------------------------------------- sendToReceiver

void Module::sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const {
    // This is the only function that actually sends data to the receiver
    // the other versions use templates to deduce sizes and create
    // the return type
    if (!shm()->useReceiverFlag) {
        std::ostringstream oss;
        oss << "Set rx_hostname first to use receiver parameters, ";
        oss << getFunctionNameFromEnum(static_cast<detFuncs>(fnum));
        throw RuntimeError(oss.str());
    }
    checkArgs(args, args_size, retval, retval_size);
    auto receiver = ReceiverSocket(shm()->rxHostname, shm()->rxTCPPort);
    receiver.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    receiver.close();
}

void Module::sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, args_size,
                                                      retval, retval_size);
}

template <typename Arg, typename Ret>
void Module::sendToReceiver(int fnum, const Arg &args, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
}

template <typename Arg, typename Ret>
void Module::sendToReceiver(int fnum, const Arg &args, Ret &retval) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, retval);
}

template <typename Arg>
void Module::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << typeid(Arg).name() << ", " << sizeof(Arg)
                   << ", nullptr, 0 ]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Module::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, args, nullptr);
}

template <typename Ret>
void Module::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
}

template <typename Ret>
void Module::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) {
    static_cast<const Module &>(*this).sendToReceiver(fnum, nullptr, retval);
}

template <typename Ret> Ret Module::sendToReceiver(int fnum) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, " << typeid(Ret).name() << ", "
                   << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    Ret retval{};
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << ToString(retval);
    return retval;
}

template <typename Ret> Ret Module::sendToReceiver(int fnum) {
    return static_cast<const Module &>(*this).sendToReceiver<Ret>(fnum);
}

void Module::sendToReceiver(int fnum) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", nullptr, 0, nullptr, 0]";
    sendToReceiver(fnum, nullptr, 0, nullptr, 0);
}

void Module::sendToReceiver(int fnum) {
    static_cast<const Module &>(*this).sendToReceiver(fnum);
}

template <typename Ret, typename Arg>
Ret Module::sendToReceiver(int fnum, const Arg &args) const {
    LOG(logDEBUG1) << "Sending to Receiver: ["
                   << getFunctionNameFromEnum(static_cast<detFuncs>(fnum))
                   << ", " << args << ", " << sizeof(args) << ", "
                   << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    STATIC_ASSERT_ARG(Arg, "sendToReceiver")
    STATIC_ASSERT_ARG(Ret, "sendToReceiver")
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret, typename Arg>
Ret Module::sendToReceiver(int fnum, const Arg &args) {
    return static_cast<const Module &>(*this).sendToReceiver<Ret>(fnum, args);
}

slsDetectorDefs::detectorType Module::getDetectorTypeFromShm(int det_id,
                                                             bool verify) {
    if (!shm.exists()) {
        throw SharedMemoryError("Shared memory " + shm.getName() +
                                " does not exist.\n Corrupted Multi Shared "
                                "memory. Please free shared memory.");
    }

    shm.openSharedMemory(verify);
    if (verify && shm()->shmversion != MODULE_SHMVERSION) {
        std::ostringstream ss;
        ss << "Single shared memory (" << det_id << "-" << moduleIndex
           << ":)version mismatch (expected 0x" << std::hex << MODULE_SHMVERSION
           << " but got 0x" << shm()->shmversion << ")" << std::dec
           << ". Clear Shared memory to continue.";
        shm.unmapSharedMemory();
        throw SharedMemoryError(ss.str());
    }
    return shm()->detType;
}

void Module::initSharedMemory(detectorType type, int det_id, bool verify) {
    shm = SharedMemory<sharedModule>(det_id, moduleIndex);
    if (!shm.exists()) {
        shm.createSharedMemory();
        initializeModuleStructure(type);
    } else {
        shm.openSharedMemory(verify);
        if (verify && shm()->shmversion != MODULE_SHMVERSION) {
            std::ostringstream ss;
            ss << "Single shared memory (" << det_id << "-" << moduleIndex
               << ":) version mismatch (expected 0x" << std::hex
               << MODULE_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
               << std::dec << ". Clear Shared memory to continue.";
            throw SharedMemoryError(ss.str());
        }
    }
}

void Module::initializeModuleStructure(detectorType type) {
    shm()->shmversion = MODULE_SHMVERSION;
    memset(shm()->hostname, 0, MAX_STR_LENGTH);
    shm()->detType = type;
    shm()->numberOfModule.x = 0;
    shm()->numberOfModule.y = 0;
    shm()->controlPort = DEFAULT_TCP_CNTRL_PORTNO;
    shm()->stopPort = DEFAULT_TCP_STOP_PORTNO;
    char *home_directory = getenv("HOME");
    if (home_directory != nullptr)
        strcpy_safe(shm()->settingsDir, home_directory);
    else {
        strcpy_safe(shm()->settingsDir, "");
        LOG(logWARNING) << "HOME directory not set";
    }
    strcpy_safe(shm()->rxHostname, "none");
    shm()->rxTCPPort = DEFAULT_TCP_RX_PORTNO + moduleIndex;
    shm()->useReceiverFlag = false;
    shm()->numUDPInterfaces = 1;
    shm()->zmqport =
        DEFAULT_ZMQ_CL_PORTNO + moduleIndex * shm()->numUDPInterfaces;
    shm()->zmqip = IpAddr{};
    shm()->stoppedFlag = false;

    // get the Module parameters based on type
    detParameters parameters{type};
    shm()->nChan.x = parameters.nChanX;
    shm()->nChan.y = parameters.nChanY;
    shm()->nChip.x = parameters.nChipX;
    shm()->nChip.y = parameters.nChipY;
    shm()->nDacs = parameters.nDacs;
}

void Module::initialDetectorServerChecks() {
    sendToDetector(F_INITIAL_CHECKS);
    sendToDetectorStop(F_INITIAL_CHECKS);
}

void Module::checkDetectorVersionCompatibility() {
    std::string detServers[2] = {getControlServerLongVersion(),
                                 getStopServerLongVersion()};
    for (int i = 0; i != 2; ++i) {
        // det and client (sem. versioning)
        Version det(detServers[i]);
        Version client(APILIB);
        if (det.hasSemanticVersioning() && client.hasSemanticVersioning()) {
            if (!det.isBackwardCompatible(client)) {
                std::ostringstream oss;
                oss << "Detector (" << (i == 0 ? "Control" : "Stop")
                    << ") version (" << det.concise()
                    << ") is incompatible with client version ("
                    << client.concise() << "). Please update "
                    << (det <= client ? "detector" : "client");
                throw sls::RuntimeError(oss.str());
            }
        }
        // comparing dates(exact match to expected)
        else {
            Version expectedDetector(getDetectorAPI());
            if (det != expectedDetector) {
                std::ostringstream oss;
                oss << "Detector (" << (i == 0 ? "Control" : "Stop")
                    << ") version (" << det.getDate()
                    << ") is incompatible with client-detector API version ("
                    << expectedDetector.getDate() << "). Please update "
                    << (det <= expectedDetector ? "detector" : "client");
                throw sls::RuntimeError(oss.str());
            }
        }
        LOG(logDEBUG) << "Detector compatible";
    }
}

const std::string Module::getDetectorAPI() const {
    switch (shm()->detType) {
    case EIGER:
        return APIEIGER;
    case JUNGFRAU:
        return APIJUNGFRAU;
    case GOTTHARD:
        return APIGOTTHARD;
    case CHIPTESTBOARD:
        return APICTB;
    case MOENCH:
        return APIMOENCH;
    case MYTHEN3:
        return APIMYTHEN3;
    case GOTTHARD2:
        return APIGOTTHARD2;
    case XILINX_CHIPTESTBOARD:
        return APIXILINXCTB;
    default:
        throw NotImplementedError(
            "Detector type not implemented to get Detector API");
    }
}

void Module::checkReceiverVersionCompatibility() {
    // rxr and client (sem. versioning)
    Version rxr(getReceiverLongVersion());
    Version client(APILIB);
    if (rxr.hasSemanticVersioning() && client.hasSemanticVersioning()) {
        if (!rxr.isBackwardCompatible(client)) {
            std::ostringstream oss;
            oss << "Receiver version (" << rxr.concise()
                << ") is incompatible with client version (" << client.concise()
                << "). Please update "
                << (rxr <= client ? "receiver" : "client");
            throw sls::RuntimeError(oss.str());
        }
    }
    // comparing dates(exact match to expected)
    else {
        Version expectedReceiver(APIRECEIVER);
        if (rxr != expectedReceiver) {
            std::ostringstream oss;
            oss << "Receiver version (" << rxr.getDate()
                << ") is incompatible with client-receiver API version ("
                << expectedReceiver.getDate() << "). Please update "
                << (rxr <= expectedReceiver ? "receiver" : "client");
            throw sls::RuntimeError(oss.str());
        }
    }
    LOG(logDEBUG) << "Receiver compatible";
}

void Module::setModule(sls_detector_module &module, bool trimbits) {
    LOG(logDEBUG1) << "Setting module with trimbits:" << trimbits;
    // to exclude trimbits
    if (!trimbits) {
        module.nchan = 0;
        module.nchip = 0;
    }
    // validate dacs and trimbits
    if (shm()->detType == MYTHEN3) {
        // check for trimbits that are out of range
        bool out_of_range = false;
        for (int i = 0; i != module.nchan; ++i) {
            if (module.chanregs[i] < 0) {
                module.chanregs[i] = 0;
                out_of_range = true;
            } else if (module.chanregs[i] > 63) {
                module.chanregs[i] = 63;
                out_of_range = true;
            }
        }
        if (out_of_range) {
            LOG(logWARNING) << "Some trimbits were out of range, these have "
                               "been replaced with 0 or 63.";
        }
        // check dacs
        out_of_range = false;
        for (int i = 0; i != module.ndac; ++i) {
            int dacMin = 0;
            int dacMax = 2800;
            if (i == M_VTH1 || i == M_VTH2 || i == M_VTH3) {
                dacMin = 200;
                dacMax = 2400;
            }
            if (module.dacs[i] < dacMin) {
                module.dacs[i] = dacMin;
                out_of_range = true;
            } else if (module.dacs[i] > dacMax) {
                module.dacs[i] = dacMax;
                out_of_range = true;
            }
        }
        if (out_of_range) {
            LOG(logWARNING)
                << "Some dacs were out of range, "
                   "these have been replaced with 0/200 or 2800/2400.";
        }
    }
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_SET_MODULE);
    client.setFnum(F_SET_MODULE);
    sendModule(&module, client);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Module " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
}

sls_detector_module Module::getModule() {
    LOG(logDEBUG1) << "Getting module";
    sls_detector_module module(shm()->detType);
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(F_GET_MODULE);
    client.setFnum(F_GET_MODULE);
    if (client.Receive<int>() == FAIL) {
        throw DetectorError("Module " + std::to_string(moduleIndex) +
                            " returned error: " + client.readErrorMessage());
    }
    receiveModule(&module, client);
    return module;
}

void Module::sendModule(sls_detector_module *myMod, ClientSocket &client) {
    constexpr TLogLevel level = logDEBUG1;
    LOG(level) << "Sending Module";
    int ts = 0;
    int n = 0;
    n = client.Send(&(myMod->serialnumber), sizeof(myMod->serialnumber));
    ts += n;
    LOG(level) << "Serial number sent. " << n
               << " bytes. serialno: " << myMod->serialnumber;

    n = client.Send(&(myMod->nchan), sizeof(myMod->nchan));
    ts += n;
    LOG(level) << "nchan sent. " << n << " bytes. nchan: " << myMod->nchan;

    n = client.Send(&(myMod->nchip), sizeof(myMod->nchip));
    ts += n;
    LOG(level) << "nchip sent. " << n << " bytes. nchip: " << myMod->nchip;

    n = client.Send(&(myMod->ndac), sizeof(myMod->ndac));
    ts += n;
    LOG(level) << "ndac sent. " << n << " bytes. ndac: " << myMod->ndac;

    n = client.Send(&(myMod->reg), sizeof(myMod->reg));
    ts += n;
    LOG(level) << "reg sent. " << n << " bytes. reg: " << myMod->reg;

    n = client.Send(&(myMod->iodelay), sizeof(myMod->iodelay));
    ts += n;
    LOG(level) << "iodelay sent. " << n
               << " bytes. iodelay: " << myMod->iodelay;

    n = client.Send(&(myMod->tau), sizeof(myMod->tau));
    ts += n;
    LOG(level) << "tau sent. " << n << " bytes. tau: " << myMod->tau;

    n = client.Send(myMod->eV, sizeof(myMod->eV));
    ts += n;
    LOG(level) << "ev sent. " << n << " bytes. ev: " << ToString(myMod->eV);

    n = client.Send(myMod->dacs, sizeof(int) * (myMod->ndac));
    ts += n;
    LOG(level) << "dacs sent. " << n << " bytes";

    n = client.Send(myMod->chanregs, sizeof(int) * (myMod->nchan));
    ts += n;
    LOG(level) << "channels sent. " << n << " bytes";

    int expectedBytesSent = sizeof(sls_detector_module) - sizeof(myMod->dacs) -
                            sizeof(myMod->chanregs) +
                            (myMod->ndac * sizeof(int)) +
                            (myMod->nchan * sizeof(int));

    if (expectedBytesSent != ts) {
        throw RuntimeError("Module size " + std::to_string(ts) +
                           " sent does not match expected size to be sent " +
                           std::to_string(expectedBytesSent));
    }
}

void Module::receiveModule(sls_detector_module *myMod, ClientSocket &client) {
    constexpr TLogLevel level = logDEBUG1;
    LOG(level) << "Receiving Module";
    myMod->serialnumber = client.Receive<int>();
    LOG(level) << "serialno: " << myMod->serialnumber;
    myMod->nchan = client.Receive<int>();
    LOG(level) << "nchan: " << myMod->nchan;
    myMod->nchip = client.Receive<int>();
    LOG(level) << "nchip: " << myMod->nchip;
    myMod->ndac = client.Receive<int>();
    LOG(level) << "ndac: " << myMod->ndac;
    myMod->reg = client.Receive<int>();
    LOG(level) << "reg: " << myMod->reg;
    myMod->iodelay = client.Receive<int>();
    LOG(level) << "iodelay: " << myMod->iodelay;
    myMod->tau = client.Receive<int>();
    LOG(level) << "tau: " << myMod->tau;
    client.Receive(myMod->eV);
    LOG(level) << "eV: " << ToString(myMod->eV);
    client.Receive(myMod->dacs, sizeof(int) * (myMod->ndac));
    LOG(level) << myMod->ndac << " dacs received";
    client.Receive(myMod->chanregs, sizeof(int) * (myMod->nchan));
    LOG(level) << myMod->nchan << " chans received";
}

void Module::updateClientStreamingIP() {
    auto ip = getClientStreamingIP();
    if (ip == 0) {
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = IpAddr{shm()->rxHostname};
        if (ip == 0) {
            ip = HostnameToIp(shm()->rxHostname);
        }
        LOG(logINFO) << "Setting default module " << moduleIndex
                     << " zmq ip to " << ip;

        setClientStreamingIP(ip);
    }
}

void Module::updateRateCorrection() {
    sendToDetector(F_UPDATE_RATE_CORRECTION);
}

sls_detector_module Module::interpolateTrim(sls_detector_module *a,
                                            sls_detector_module *b,
                                            const int energy, const int e1,
                                            const int e2, bool trimbits) {
    // dacs specified only for eiger and mythen3
    if (shm()->detType != EIGER && shm()->detType != MYTHEN3) {
        throw NotImplementedError(
            "Interpolation of Trim values not implemented for this detector!");
    }

    sls_detector_module myMod{shm()->detType};

    // create copy and interpolate dac lists
    std::vector<int> dacs_to_copy, dacs_to_interpolate;
    if (shm()->detType == EIGER) {
        dacs_to_copy.insert(
            dacs_to_copy.end(),
            {E_SVP, E_SVN, E_VTGSTV, E_RXB_RB, E_RXB_LB, E_VCN, E_VIS});
        // interpolate vrf, vcmp, vcp
        dacs_to_interpolate.insert(dacs_to_interpolate.end(),
                                   {E_VTR, E_VRF, E_VCMP_LL, E_VCMP_LR,
                                    E_VCMP_RL, E_VCMP_RR, E_VCP, E_VRS});
    } else {
        dacs_to_copy.insert(dacs_to_copy.end(),
                            {M_VCASSH, M_VRSHAPER, M_VRSHAPER_N, M_VIPRE_OUT,
                             M_VICIN, M_VCAS, M_VRPREAMP, M_VCAL_N, M_VIPRE,
                             M_VISHAPER, M_VCAL_P, M_VDCSH});
        // interpolate vtrim, vth1, vth2, vth3
        dacs_to_interpolate.insert(dacs_to_interpolate.end(),
                                   {M_VTH1, M_VTH2, M_VTH3, M_VTRIM});
    }

    // Copy Dacs
    for (size_t i = 0; i < dacs_to_copy.size(); ++i) {
        if (a->dacs[dacs_to_copy[i]] != b->dacs[dacs_to_copy[i]]) {
            throw RuntimeError("Interpolate module: dacs " + std::to_string(i) +
                               " different");
        }
        myMod.dacs[dacs_to_copy[i]] = a->dacs[dacs_to_copy[i]];
    }

    // Interpolate Dacs
    for (size_t i = 0; i < dacs_to_interpolate.size(); ++i) {
        myMod.dacs[dacs_to_interpolate[i]] =
            linearInterpolation(energy, e1, e2, a->dacs[dacs_to_interpolate[i]],
                                b->dacs[dacs_to_interpolate[i]]);
    }

    // Copy irrelevant dacs (without failing)
    if (shm()->detType == EIGER) {
        // CAL
        if (a->dacs[E_CAL] != b->dacs[E_CAL]) {
            LOG(logWARNING)
                << "DAC CAL differs in both energies (" << a->dacs[E_CAL] << ","
                << b->dacs[E_CAL] << ")!\nTaking first: " << a->dacs[E_CAL];
        }
        myMod.dacs[E_CAL] = a->dacs[E_CAL];
    }

    // Interpolate all trimbits
    if (trimbits) {
        for (int i = 0; i < myMod.nchan; ++i) {
            myMod.chanregs[i] = linearInterpolation(
                energy, e1, e2, a->chanregs[i], b->chanregs[i]);
        }
    }
    return myMod;
}

std::string Module::getTrimbitFilename(detectorSettings s, int e_eV) {
    std::string ssettings;
    switch (s) {
    case STANDARD:
        ssettings = "/standard";
        break;
    case FAST:
        ssettings = "/fast";
        break;
    case HIGHGAIN:
        ssettings = "/highgain";
        break;
    case LOWGAIN:
        ssettings = "/lowgain";
        break;
    case VERYHIGHGAIN:
        ssettings = "/veryhighgain";
        break;
    case VERYLOWGAIN:
        ssettings = "/verylowgain";
        break;
    default:
        std::ostringstream ss;
        ss << "Unknown settings " << ToString(s) << " for this detector!";
        throw RuntimeError(ss.str());
    }
    std::ostringstream ostfn;
    ostfn << shm()->settingsDir << ssettings << "/" << e_eV << "eV";
    if (shm()->detType == EIGER) {
        ostfn << "/noise.sn";
    } else if (shm()->detType == MYTHEN3) {
        ostfn << "/trim.sn";
    } else {
        throw RuntimeError(
            "Settings or trimbit files not defined for this detector.");
    }
    int moduleIdWidth = 3;
    if (shm()->detType == MYTHEN3) {
        moduleIdWidth = 4;
    }
    ostfn << std::setfill('0') << std::setw(moduleIdWidth) << std::dec
          << getModuleId() << std::setbase(10);
    return ostfn.str();
}

sls_detector_module Module::readSettingsFile(const std::string &fname,
                                             bool trimbits) {
    LOG(logDEBUG1) << "Read settings file " << fname;
    sls_detector_module myMod(shm()->detType);
    // open file
    std::ifstream infile;
    if (shm()->detType == EIGER || shm()->detType == MYTHEN3) {
        infile.open(fname.c_str(), std::ifstream::binary);
    } else {
        infile.open(fname.c_str(), std::ios_base::in);
    }
    if (!infile) {
        throw RuntimeError("Could not open settings file: " + fname);
    }

    auto file_size = getFileSize(infile);

    // eiger
    if (shm()->detType == EIGER) {
        infile.read(reinterpret_cast<char *>(myMod.dacs),
                    sizeof(int) * (myMod.ndac));
        infile.read(reinterpret_cast<char *>(&myMod.iodelay),
                    sizeof(myMod.iodelay));
        infile.read(reinterpret_cast<char *>(&myMod.tau), sizeof(myMod.tau));
        for (int i = 0; i < myMod.ndac; ++i) {
            LOG(logDEBUG1) << "dac " << i << ":" << myMod.dacs[i];
        }
        LOG(logDEBUG1) << "iodelay:" << myMod.iodelay;
        LOG(logDEBUG1) << "tau:" << myMod.tau;
        if (trimbits) {
            infile.read(reinterpret_cast<char *>(myMod.chanregs),
                        sizeof(int) * (myMod.nchan));
        }
        if (!infile) {
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
    }

    // mythen3 (dacs, trimbits)
    else if (shm()->detType == MYTHEN3) {
        int expected_size = sizeof(int) * myMod.ndac +
                            sizeof(int) * myMod.nchan + sizeof(myMod.reg);
        if (file_size != expected_size) {
            throw RuntimeError("The size of the settings file: " + fname +
                               " differs from the expected size, " +
                               std::to_string(file_size) + " instead of " +
                               std::to_string(expected_size) + " bytes");
        }
        infile.read(reinterpret_cast<char *>(&myMod.reg), sizeof(myMod.reg));
        infile.read(reinterpret_cast<char *>(myMod.dacs),
                    sizeof(int) * (myMod.ndac));
        for (int i = 0; i < myMod.ndac; ++i) {
            LOG(logDEBUG) << "dac " << i << ":" << myMod.dacs[i];
        }
        if (trimbits) {
            infile.read(reinterpret_cast<char *>(myMod.chanregs),
                        sizeof(int) * (myMod.nchan));
        }
        if (!infile) {
            throw RuntimeError("readSettingsFile: Could not load all values "
                               "for settings for " +
                               fname);
        }
    }

    else {
        throw RuntimeError("Not implemented for this detector");
    }
    LOG(logINFO) << "Settings file loaded: " << fname;
    return myMod;
}

void Module::saveSettingsFile(sls_detector_module &myMod,
                              const std::string &fname) {
    LOG(logDEBUG1) << moduleIndex << ": Saving settings to " << fname;
    std::ofstream outfile(fname);
    if (!outfile) {
        throw RuntimeError("Could not write settings file: " + fname);
    }
    switch (shm()->detType) {
    case MYTHEN3:
        outfile.write(reinterpret_cast<char *>(&myMod.reg), sizeof(myMod.reg));
        outfile.write(reinterpret_cast<char *>(myMod.dacs),
                      sizeof(int) * (myMod.ndac));
        outfile.write(reinterpret_cast<char *>(myMod.chanregs),
                      sizeof(int) * (myMod.nchan));
        break;
    case EIGER:
        outfile.write(reinterpret_cast<char *>(myMod.dacs),
                      sizeof(int) * (myMod.ndac));
        outfile.write(reinterpret_cast<char *>(&myMod.iodelay),
                      sizeof(myMod.iodelay));
        outfile.write(reinterpret_cast<char *>(&myMod.tau), sizeof(myMod.tau));
        outfile.write(reinterpret_cast<char *>(myMod.chanregs),
                      sizeof(int) * (myMod.nchan));
        break;
    default:
        throw RuntimeError(
            "Saving settings file is not implemented for this detector.");
    }
    LOG(logINFO) << "Settings for " << shm()->hostname << " written to "
                 << fname;
}

void Module::sendProgram(bool blackfin, std::vector<char> buffer,
                         const int functionEnum,
                         const std::string &functionType,
                         const std::string serverName,
                         const bool forceDeleteNormalFile) {
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): Sending " << functionType;

    // send fnum and filesize
    auto client = DetectorSocket(shm()->hostname, shm()->controlPort);
    client.Send(functionEnum);
    uint64_t filesize = buffer.size();
    client.Send(filesize);

    // send checksum
    std::string checksum = md5_calculate_checksum(buffer.data(), filesize);
    LOG(logDEBUG1) << "Checksum:" << checksum;
    char cChecksum[MAX_STR_LENGTH] = {0};
    strcpy(cChecksum, checksum.c_str());
    client.Send(cChecksum);

    // send server name
    if (functionEnum == F_UPDATE_DETECTOR_SERVER) {
        char sname[MAX_STR_LENGTH] = {0};
        strcpy(sname, serverName.c_str());
        client.Send(sname);
    }

    // send forceDeleteNormalFile flag
    if (blackfin) {
        client.Send(static_cast<int>(forceDeleteNormalFile));
    }

    // validate memory allocation etc in detector
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Module " << moduleIndex << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw DetectorError(os.str());
    }

    // send program
    if (blackfin) {
        uint64_t unitprogramsize = 0;
        int currentPointer = 0;
        while (filesize > 0) {
            unitprogramsize = MAX_BLACKFIN_PROGRAM_SIZE;
            if (unitprogramsize > filesize) {
                unitprogramsize = filesize;
            }
            LOG(logDEBUG) << "unitprogramsize:" << unitprogramsize
                          << "\t filesize:" << filesize;

            client.Send(&buffer[currentPointer], unitprogramsize);
            if (client.Receive<int>() == FAIL) {
                std::cout << '\n';
                std::ostringstream os;
                os << "Module " << moduleIndex << " (" << shm()->hostname << ")"
                   << " returned error: " << client.readErrorMessage();
                throw DetectorError(os.str());
            }
            filesize -= unitprogramsize;
            currentPointer += unitprogramsize;
        }
    } else {
        client.Send(buffer);
    }

    // tmp checksum verified in detector
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Module " << moduleIndex << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw DetectorError(os.str());
    }
    LOG(logINFO) << "Checksum verified for module " << moduleIndex << " ("
                 << shm()->hostname << ")";

    // simulating erasing and writing to
    if (blackfin) {
        if (functionEnum == F_PROGRAM_FPGA) {
            simulatingActivityinDetector("Erasing Flash",
                                         BLACKFIN_ERASE_FLASH_TIME);
            simulatingActivityinDetector("Writing to Flash",
                                         BLACKFIN_WRITE_TO_FLASH_TIME);
        }
    } else {
        if (functionEnum == F_PROGRAM_FPGA) {
            simulatingActivityinDetector("Erasing Flash",
                                         NIOS_ERASE_FLASH_TIME_FPGA);
            simulatingActivityinDetector("Writing to Flash",
                                         NIOS_WRITE_TO_FLASH_TIME_FPGA);
        } else if (functionEnum == F_UPDATE_KERNEL) {
            simulatingActivityinDetector("Erasing Flash",
                                         NIOS_ERASE_FLASH_TIME_KERNEL);
            simulatingActivityinDetector("Writing to Flash",
                                         NIOS_WRITE_TO_FLASH_TIME_KERNEL);
        }
    }

    // update verified
    if (client.Receive<int>() == FAIL) {
        std::ostringstream os;
        os << "Module " << moduleIndex << " (" << shm()->hostname << ")"
           << " returned error: " << client.readErrorMessage();
        throw DetectorError(os.str());
    }
    LOG(logINFO) << "Module " << moduleIndex << " (" << shm()->hostname
                 << "): " << functionType << " successful";
}

void Module::simulatingActivityinDetector(const std::string &functionType,
                                          const int timeRequired) {
    LOG(logINFO) << functionType << " for module " << moduleIndex << " ("
                 << shm()->hostname << ")";
    printf("%d%%\r", 0);
    std::cout << std::flush;
    const int ERASE_TIME = timeRequired;
    int count = ERASE_TIME + 1;
    while (count > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --count;
        printf(
            "%d%%\r",
            static_cast<int>(
                (static_cast<double>(ERASE_TIME - count) / ERASE_TIME) * 100));
        std::cout << std::flush;
    }
    printf("\n");
}
} // namespace sls

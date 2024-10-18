/* WARINING This file is auto generated any edits might be overwritten without
 * warning */

// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "py_headers.h"

#include "sls/Detector.h"
#include "sls/TimeHelper.h"
#include "sls/ToString.h"
#include "sls/network_utils.h"
#include "sls/sls_detector_defs.h"

#include <array>
#include <chrono>
namespace py = pybind11;
void init_det(py::module &m) {
    using sls::defs;
    using sls::Detector;
    using sls::ns;
    using sls::Positions;
    using sls::Result;

    m.def("freeSharedMemory",
          (void (*)(const int, const int)) & sls::freeSharedMemory,
          py::arg() = 0, py::arg() = -1);

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi.def(py::init<int>());

    CppDetectorApi.def("loadConfig",
                       (void (Detector::*)(const std::string &)) &
                           Detector::loadConfig,
                       py::arg());
    CppDetectorApi.def("loadParameters",
                       (void (Detector::*)(const std::string &)) &
                           Detector::loadParameters,
                       py::arg());
    CppDetectorApi.def("loadParameters",
                       (void (Detector::*)(const std::vector<std::string> &)) &
                           Detector::loadParameters,
                       py::arg());
    CppDetectorApi.def(
        "getHostname",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getHostname,
        py::arg() = Positions{});
    CppDetectorApi.def("setHostname",
                       (void (Detector::*)(const std::vector<std::string> &)) &
                           Detector::setHostname,
                       py::arg());
    CppDetectorApi.def("setVirtualDetectorServers",
                       (void (Detector::*)(int, uint16_t)) &
                           Detector::setVirtualDetectorServers,
                       py::arg(), py::arg());
    CppDetectorApi.def("getShmId",
                       (int (Detector::*)() const) & Detector::getShmId);
    CppDetectorApi.def("getPackageVersion", (std::string(Detector::*)() const) &
                                                Detector::getPackageVersion);
    CppDetectorApi.def("getClientVersion", (std::string(Detector::*)() const) &
                                               Detector::getClientVersion);
    CppDetectorApi.def("getFirmwareVersion",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getFirmwareVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getFrontEndFirmwareVersion",
                       (Result<int64_t>(Detector::*)(const defs::fpgaPosition,
                                                     sls::Positions) const) &
                           Detector::getFrontEndFirmwareVersion,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDetectorServerVersion",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getDetectorServerVersion,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getHardwareVersion",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getHardwareVersion,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getKernelVersion",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getKernelVersion,
        py::arg() = Positions{});
    CppDetectorApi.def("getSerialNumber",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getSerialNumber,
                       py::arg() = Positions{});
    CppDetectorApi.def("getModuleId",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getModuleId,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getReceiverVersion",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getReceiverVersion,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getDetectorType",
        (Result<defs::detectorType>(Detector::*)(sls::Positions) const) &
            Detector::getDetectorType,
        py::arg() = Positions{});
    CppDetectorApi.def("size", (int (Detector::*)() const) & Detector::size);
    CppDetectorApi.def("empty", (bool (Detector::*)() const) & Detector::empty);
    CppDetectorApi.def("getModuleGeometry", (defs::xy(Detector::*)() const) &
                                                Detector::getModuleGeometry);
    CppDetectorApi.def("getModuleSize",
                       (Result<defs::xy>(Detector::*)(sls::Positions) const) &
                           Detector::getModuleSize,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDetectorSize", (defs::xy(Detector::*)() const) &
                                              Detector::getDetectorSize);
    CppDetectorApi.def("setDetectorSize",
                       (void (Detector::*)(const defs::xy)) &
                           Detector::setDetectorSize,
                       py::arg());
    CppDetectorApi.def("getSettingsList", (std::vector<defs::detectorSettings>(
                                              Detector::*)() const) &
                                              Detector::getSettingsList);
    CppDetectorApi.def(
        "getSettings",
        (Result<defs::detectorSettings>(Detector::*)(sls::Positions) const) &
            Detector::getSettings,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setSettings",
        (void (Detector::*)(defs::detectorSettings, sls::Positions)) &
            Detector::setSettings,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getThresholdEnergy",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getThresholdEnergy,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getAllThresholdEnergy",
        (Result<std::array<int, 3>>(Detector::*)(sls::Positions) const) &
            Detector::getAllThresholdEnergy,
        py::arg() = Positions{});
    CppDetectorApi.def("setThresholdEnergy",
                       (void (Detector::*)(int, defs::detectorSettings, bool,
                                           sls::Positions)) &
                           Detector::setThresholdEnergy,
                       py::arg(), py::arg() = defs::STANDARD, py::arg() = true,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setThresholdEnergy",
        (void (Detector::*)(std::array<int, 3>, defs::detectorSettings, bool,
                            sls::Positions)) &
            Detector::setThresholdEnergy,
        py::arg(), py::arg() = defs::STANDARD, py::arg() = true,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getSettingsPath",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getSettingsPath,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setSettingsPath",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setSettingsPath,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "loadTrimbits",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::loadTrimbits,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "saveTrimbits",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::saveTrimbits,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAllTrimbits",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getAllTrimbits,
                       py::arg() = Positions{});
    CppDetectorApi.def("setAllTrimbits",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setAllTrimbits,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getTrimEnergies",
        (Result<std::vector<int>>(Detector::*)(sls::Positions) const) &
            Detector::getTrimEnergies,
        py::arg() = Positions{});
    CppDetectorApi.def("setTrimEnergies",
                       (void (Detector::*)(std::vector<int>, sls::Positions)) &
                           Detector::setTrimEnergies,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getGapPixelsinCallback",
                       (bool (Detector::*)() const) &
                           Detector::getGapPixelsinCallback);
    CppDetectorApi.def("setGapPixelsinCallback",
                       (void (Detector::*)(const bool)) &
                           Detector::setGapPixelsinCallback,
                       py::arg());
    CppDetectorApi.def("getFlipRows",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getFlipRows,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFlipRows",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setFlipRows,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMaster",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getMaster,
                       py::arg() = Positions{});
    CppDetectorApi.def("setMaster",
                       (void (Detector::*)(bool, int)) & Detector::setMaster,
                       py::arg(), py::arg());
    CppDetectorApi.def("getSynchronization",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getSynchronization,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSynchronization",
        (void (Detector::*)(bool)) & Detector::setSynchronization, py::arg());
    CppDetectorApi.def(
        "getBadChannels",
        (void (Detector::*)(const std::string &, sls::Positions) const) &
            Detector::getBadChannels,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setBadChannels",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setBadChannels,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getBadChannels",
        (Result<std::vector<int>>(Detector::*)(sls::Positions) const) &
            Detector::getBadChannels,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setBadChannels",
        (void (Detector::*)(const std::vector<int>, sls::Positions)) &
            Detector::setBadChannels,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setBadChannels",
        (void (Detector::*)(const std::vector<std::vector<int>>)) &
            Detector::setBadChannels,
        py::arg());
    CppDetectorApi.def("getRow",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRow,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRow",
                       (void (Detector::*)(const int, sls::Positions)) &
                           Detector::setRow,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getColumn",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getColumn,
                       py::arg() = Positions{});
    CppDetectorApi.def("setColumn",
                       (void (Detector::*)(const int, sls::Positions)) &
                           Detector::setColumn,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("isVirtualDetectorServer",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::isVirtualDetectorServer,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "registerAcquisitionFinishedCallback",
        (void (Detector::*)(void (*)(double, int, void *), void *)) &
            Detector::registerAcquisitionFinishedCallback,
        py::arg(), py::arg());
    CppDetectorApi.def("registerDataCallback",
                       (void (Detector::*)(void (*)(sls::detectorData *,
                                                    uint64_t, uint32_t, void *),
                                           void *)) &
                           Detector::registerDataCallback,
                       py::arg(), py::arg());
    CppDetectorApi.def("getNumberOfFrames",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfFrames,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfFrames",
        (void (Detector::*)(int64_t)) & Detector::setNumberOfFrames, py::arg());
    CppDetectorApi.def("getNumberOfTriggers",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfTriggers,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfTriggers",
                       (void (Detector::*)(int64_t)) &
                           Detector::setNumberOfTriggers,
                       py::arg());
    CppDetectorApi.def("getExptime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getExptime,
                       py::arg() = Positions{});
    CppDetectorApi.def("setExptime",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setExptime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPeriod",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPeriod",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setPeriod,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDelayAfterTrigger",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getDelayAfterTrigger,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDelayAfterTrigger",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setDelayAfterTrigger,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFramesLeft",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfFramesLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfTriggersLeft",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfTriggersLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPeriodLeft",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getPeriodLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDelayAfterTriggerLeft",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getDelayAfterTriggerLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDynamicRange",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getDynamicRange,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDynamicRange",
                       (void (Detector::*)(int)) & Detector::setDynamicRange,
                       py::arg());
    CppDetectorApi.def("getDynamicRangeList",
                       (std::vector<int>(Detector::*)() const) &
                           Detector::getDynamicRangeList);
    CppDetectorApi.def(
        "getTimingMode",
        (Result<defs::timingMode>(Detector::*)(sls::Positions) const) &
            Detector::getTimingMode,
        py::arg() = Positions{});
    CppDetectorApi.def("setTimingMode",
                       (void (Detector::*)(defs::timingMode, sls::Positions)) &
                           Detector::setTimingMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTimingModeList",
                       (std::vector<defs::timingMode>(Detector::*)() const) &
                           Detector::getTimingModeList);
    CppDetectorApi.def(
        "getReadoutSpeed",
        (Result<defs::speedLevel>(Detector::*)(sls::Positions) const) &
            Detector::getReadoutSpeed,
        py::arg() = Positions{});
    CppDetectorApi.def("setReadoutSpeed",
                       (void (Detector::*)(defs::speedLevel, sls::Positions)) &
                           Detector::setReadoutSpeed,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getReadoutSpeedList",
                       (std::vector<defs::speedLevel>(Detector::*)() const) &
                           Detector::getReadoutSpeedList);
    CppDetectorApi.def("getADCPhase",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getADCPhase,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCPhase",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setADCPhase,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMaxADCPhaseShift",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getMaxADCPhaseShift,
                       py::arg() = Positions{});
    CppDetectorApi.def("getADCPhaseInDegrees",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getADCPhaseInDegrees,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCPhaseInDegrees",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setADCPhaseInDegrees,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITPhase",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getDBITPhase,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDBITPhase",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setDBITPhase,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMaxDBITPhaseShift",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getMaxDBITPhaseShift,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDBITPhaseInDegrees",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getDBITPhaseInDegrees,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDBITPhaseInDegrees",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setDBITPhaseInDegrees,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClockFrequency",
                       (Result<int>(Detector::*)(int, sls::Positions)) &
                           Detector::getClockFrequency,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClockPhase",
                       (Result<int>(Detector::*)(int, sls::Positions)) &
                           Detector::getClockPhase,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setClockPhase",
                       (void (Detector::*)(int, int, sls::Positions)) &
                           Detector::setClockPhase,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMaxClockPhaseShift",
                       (Result<int>(Detector::*)(int, sls::Positions)) &
                           Detector::getMaxClockPhaseShift,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClockPhaseinDegrees",
                       (Result<int>(Detector::*)(int, sls::Positions)) &
                           Detector::getClockPhaseinDegrees,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setClockPhaseinDegrees",
                       (void (Detector::*)(int, int, sls::Positions)) &
                           Detector::setClockPhaseinDegrees,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClockDivider",
                       (Result<int>(Detector::*)(int, sls::Positions)) &
                           Detector::getClockDivider,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setClockDivider",
                       (void (Detector::*)(int, int, sls::Positions)) &
                           Detector::setClockDivider,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getHighVoltage",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getHighVoltage,
                       py::arg() = Positions{});
    CppDetectorApi.def("setHighVoltage",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setHighVoltage,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPowerChip",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getPowerChip,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPowerChip",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setPowerChip,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getImageTestMode",
                       (Result<int>(Detector::*)(sls::Positions)) &
                           Detector::getImageTestMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setImageTestMode",
                       (void (Detector::*)(const int, sls::Positions)) &
                           Detector::setImageTestMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureList",
                       (std::vector<defs::dacIndex>(Detector::*)() const) &
                           Detector::getTemperatureList);
    CppDetectorApi.def(
        "getTemperature",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions) const) &
            Detector::getTemperature,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDacList",
                       (std::vector<defs::dacIndex>(Detector::*)() const) &
                           Detector::getDacList);
    CppDetectorApi.def(
        "getDefaultDac",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions)) &
            Detector::getDefaultDac,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDefaultDac",
        (void (Detector::*)(defs::dacIndex, int, sls::Positions)) &
            Detector::setDefaultDac,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDefaultDac",
        (Result<int>(Detector::*)(defs::dacIndex, defs::detectorSettings,
                                  sls::Positions)) &
            Detector::getDefaultDac,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDefaultDac",
        (void (Detector::*)(defs::dacIndex, int, defs::detectorSettings,
                            sls::Positions)) &
            Detector::setDefaultDac,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("resetToDefaultDacs",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::resetToDefaultDacs,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDAC",
        (Result<int>(Detector::*)(defs::dacIndex, bool, sls::Positions) const) &
            Detector::getDAC,
        py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "setDAC",
        (void (Detector::*)(defs::dacIndex, int, bool, sls::Positions)) &
            Detector::setDAC,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "getOnChipDAC",
        (Result<int>(Detector::*)(defs::dacIndex, int, sls::Positions) const) &
            Detector::getOnChipDAC,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setOnChipDAC",
        (void (Detector::*)(defs::dacIndex, int, int, sls::Positions)) &
            Detector::setOnChipDAC,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExternalSignalFlags",
                       (Result<defs::externalSignalFlag>(Detector::*)(
                           int, sls::Positions) const) &
                           Detector::getExternalSignalFlags,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setExternalSignalFlags",
        (void (Detector::*)(int, defs::externalSignalFlag, sls::Positions)) &
            Detector::setExternalSignalFlags,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getParallelMode",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getParallelMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setParallelMode",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setParallelMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFilterResistor",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getFilterResistor,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFilterResistor",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setFilterResistor,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getCurrentSource",
                       (Result<defs::currentSrcParameters>(Detector::*)(
                           sls::Positions) const) &
                           Detector::getCurrentSource,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setCurrentSource",
        (void (Detector::*)(defs::currentSrcParameters, sls::Positions)) &
            Detector::setCurrentSource,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITPipeline",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getDBITPipeline,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDBITPipeline",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setDBITPipeline,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getReadNRows",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getReadNRows,
                       py::arg() = Positions{});
    CppDetectorApi.def("setReadNRows",
                       (void (Detector::*)(const int, sls::Positions)) &
                           Detector::setReadNRows,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("acquire", (void (Detector::*)()) & Detector::acquire);
    CppDetectorApi.def("clearAcquiringFlag",
                       (void (Detector::*)()) & Detector::clearAcquiringFlag);
    CppDetectorApi.def("startReceiver",
                       (void (Detector::*)()) & Detector::startReceiver);
    CppDetectorApi.def("stopReceiver",
                       (void (Detector::*)()) & Detector::stopReceiver);
    CppDetectorApi.def("startDetector",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::startDetector,
                       py::arg() = Positions{});
    CppDetectorApi.def("startDetectorReadout",
                       (void (Detector::*)()) & Detector::startDetectorReadout);
    CppDetectorApi.def("stopDetector",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::stopDetector,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getDetectorStatus",
        (Result<defs::runStatus>(Detector::*)(sls::Positions) const) &
            Detector::getDetectorStatus,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getReceiverStatus",
        (Result<defs::runStatus>(Detector::*)(sls::Positions) const) &
            Detector::getReceiverStatus,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getFramesCaught",
        (Result<std::vector<int64_t>>(Detector::*)(sls::Positions) const) &
            Detector::getFramesCaught,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getNumMissingPackets",
        (Result<std::vector<int64_t>>(Detector::*)(sls::Positions) const) &
            Detector::getNumMissingPackets,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxCurrentFrameIndex",
        (Result<std::vector<int64_t>>(Detector::*)(sls::Positions) const) &
            Detector::getRxCurrentFrameIndex,
        py::arg() = Positions{});
    CppDetectorApi.def("getNextFrameNumber",
                       (Result<uint64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNextFrameNumber,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNextFrameNumber",
                       (void (Detector::*)(uint64_t, sls::Positions)) &
                           Detector::setNextFrameNumber,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("sendSoftwareTrigger",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::sendSoftwareTrigger,
                       py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "getScan",
        (Result<defs::scanParameters>(Detector::*)(sls::Positions) const) &
            Detector::getScan,
        py::arg() = Positions{});
    CppDetectorApi.def("setScan",
                       (void (Detector::*)(const defs::scanParameters)) &
                           Detector::setScan,
                       py::arg());
    CppDetectorApi.def(
        "getScanErrorMessage",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getScanErrorMessage,
        py::arg() = Positions{});
    CppDetectorApi.def("getNumberofUDPInterfaces",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberofUDPInterfaces,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberofUDPInterfaces",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberofUDPInterfaces,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSelectedUDPInterface",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getSelectedUDPInterface,
                       py::arg() = Positions{});
    CppDetectorApi.def("selectUDPInterface",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::selectUDPInterface,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSourceUDPIP",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getSourceUDPIP,
        py::arg() = Positions{});
    CppDetectorApi.def("setSourceUDPIP",
                       (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                           Detector::setSourceUDPIP,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSourceUDPIP2",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getSourceUDPIP2,
        py::arg() = Positions{});
    CppDetectorApi.def("setSourceUDPIP2",
                       (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                           Detector::setSourceUDPIP2,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSourceUDPMAC",
        (Result<sls::MacAddr>(Detector::*)(sls::Positions) const) &
            Detector::getSourceUDPMAC,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setSourceUDPMAC",
        (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
            Detector::setSourceUDPMAC,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSourceUDPMAC2",
        (Result<sls::MacAddr>(Detector::*)(sls::Positions) const) &
            Detector::getSourceUDPMAC2,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setSourceUDPMAC2",
        (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
            Detector::setSourceUDPMAC2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPList",
                       (Result<sls::UdpDestination>(Detector::*)(
                           const uint32_t, sls::Positions) const) &
                           Detector::getDestinationUDPList,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPList",
        (void (Detector::*)(const sls::UdpDestination, const int)) &
            Detector::setDestinationUDPList,
        py::arg(), py::arg());
    CppDetectorApi.def("getNumberofUDPDestinations",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberofUDPDestinations,
                       py::arg() = Positions{});
    CppDetectorApi.def("clearUDPDestinations",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::clearUDPDestinations,
                       py::arg() = Positions{});
    CppDetectorApi.def("getFirstUDPDestination",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getFirstUDPDestination,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFirstUDPDestination",
                       (void (Detector::*)(const int, sls::Positions)) &
                           Detector::setFirstUDPDestination,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDestinationUDPIP",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getDestinationUDPIP,
        py::arg() = Positions{});
    CppDetectorApi.def("setDestinationUDPIP",
                       (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                           Detector::setDestinationUDPIP,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDestinationUDPIP2",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getDestinationUDPIP2,
        py::arg() = Positions{});
    CppDetectorApi.def("setDestinationUDPIP2",
                       (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                           Detector::setDestinationUDPIP2,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDestinationUDPMAC",
        (Result<sls::MacAddr>(Detector::*)(sls::Positions) const) &
            Detector::getDestinationUDPMAC,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPMAC",
        (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
            Detector::setDestinationUDPMAC,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDestinationUDPMAC2",
        (Result<sls::MacAddr>(Detector::*)(sls::Positions) const) &
            Detector::getDestinationUDPMAC2,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPMAC2",
        (void (Detector::*)(const sls::MacAddr, sls::Positions)) &
            Detector::setDestinationUDPMAC2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getDestinationUDPPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDestinationUDPPort",
                       (void (Detector::*)(uint16_t, int)) &
                           Detector::setDestinationUDPPort,
                       py::arg(), py::arg() = -1);
    CppDetectorApi.def("getDestinationUDPPort2",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getDestinationUDPPort2,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDestinationUDPPort2",
                       (void (Detector::*)(uint16_t, int)) &
                           Detector::setDestinationUDPPort2,
                       py::arg(), py::arg() = -1);
    CppDetectorApi.def("reconfigureUDPDestination",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::reconfigureUDPDestination,
                       py::arg() = Positions{});
    CppDetectorApi.def("validateUDPConfiguration",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::validateUDPConfiguration,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "printRxConfiguration",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::printRxConfiguration,
        py::arg() = Positions{});
    CppDetectorApi.def("getTenGiga",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getTenGiga,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTenGiga",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setTenGiga,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTenGigaFlowControl",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getTenGigaFlowControl,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTenGigaFlowControl",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setTenGigaFlowControl,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayFrame",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getTransmissionDelayFrame,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransmissionDelayFrame",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setTransmissionDelayFrame,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayLeft",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getTransmissionDelayLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransmissionDelayLeft",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setTransmissionDelayLeft,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayRight",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getTransmissionDelayRight,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransmissionDelayRight",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setTransmissionDelayRight,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelay",
                       (int (Detector::*)() const) &
                           Detector::getTransmissionDelay);
    CppDetectorApi.def(
        "setTransmissionDelay",
        (void (Detector::*)(int)) & Detector::setTransmissionDelay, py::arg());
    CppDetectorApi.def("getUseReceiverFlag",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getUseReceiverFlag,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxHostname",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getRxHostname,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxHostname",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setRxHostname,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setRxHostname",
                       (void (Detector::*)(const std::vector<std::string> &)) &
                           Detector::setRxHostname,
                       py::arg());
    CppDetectorApi.def("getRxPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getRxPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxPort", (void (Detector::*)(uint16_t, int)) & Detector::setRxPort,
        py::arg(), py::arg() = -1);
    CppDetectorApi.def("getRxFifoDepth",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxFifoDepth,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxFifoDepth",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxFifoDepth,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxSilentMode",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getRxSilentMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxSilentMode",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setRxSilentMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxFrameDiscardPolicy",
        (Result<defs::frameDiscardPolicy>(Detector::*)(sls::Positions) const) &
            Detector::getRxFrameDiscardPolicy,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxFrameDiscardPolicy",
        (void (Detector::*)(defs::frameDiscardPolicy, sls::Positions)) &
            Detector::setRxFrameDiscardPolicy,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPartialFramesPadding",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getPartialFramesPadding,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPartialFramesPadding",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setPartialFramesPadding,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxUDPSocketBufferSize",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxUDPSocketBufferSize,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxUDPSocketBufferSize",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxUDPSocketBufferSize,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxRealUDPSocketBufferSize",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxRealUDPSocketBufferSize,
                       py::arg() = Positions{});
    CppDetectorApi.def("getRxLock",
                       (Result<bool>(Detector::*)(sls::Positions)) &
                           Detector::getRxLock,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxLock",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setRxLock,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxLastClientIP",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getRxLastClientIP,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxThreadIds",
        (Result<std::array<pid_t, 9>>(Detector::*)(sls::Positions) const) &
            Detector::getRxThreadIds,
        py::arg() = Positions{});
    CppDetectorApi.def("getRxArping",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getRxArping,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxArping",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setRxArping,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getIndividualRxROIs",
                       (Result<defs::ROI>(Detector::*)(sls::Positions) const) &
                           Detector::getIndividualRxROIs,
                       py::arg());
    CppDetectorApi.def("getRxROI",
                       (defs::ROI(Detector::*)() const) & Detector::getRxROI);
    CppDetectorApi.def(
        "setRxROI", (void (Detector::*)(const defs::ROI)) & Detector::setRxROI,
        py::arg());
    CppDetectorApi.def("clearRxROI",
                       (void (Detector::*)()) & Detector::clearRxROI);
    CppDetectorApi.def(
        "getFileFormat",
        (Result<defs::fileFormat>(Detector::*)(sls::Positions) const) &
            Detector::getFileFormat,
        py::arg() = Positions{});
    CppDetectorApi.def("setFileFormat",
                       (void (Detector::*)(defs::fileFormat, sls::Positions)) &
                           Detector::setFileFormat,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getFilePath",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getFilePath,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setFilePath",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setFilePath,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getFileNamePrefix",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getFileNamePrefix,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setFileNamePrefix",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setFileNamePrefix,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAcquisitionIndex",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getAcquisitionIndex,
                       py::arg() = Positions{});
    CppDetectorApi.def("setAcquisitionIndex",
                       (void (Detector::*)(int64_t, sls::Positions)) &
                           Detector::setAcquisitionIndex,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFileWrite",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getFileWrite,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFileWrite",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setFileWrite,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMasterFileWrite", (bool (Detector::*)() const) &
                                                 Detector::getMasterFileWrite);
    CppDetectorApi.def(
        "setMasterFileWrite",
        (void (Detector::*)(bool)) & Detector::setMasterFileWrite, py::arg());
    CppDetectorApi.def("getFileOverWrite",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getFileOverWrite,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFileOverWrite",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setFileOverWrite,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFramesPerFile",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getFramesPerFile,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFramesPerFile",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setFramesPerFile,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqDataStream",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqDataStream,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqDataStream",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setRxZmqDataStream,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqFrequency",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqFrequency,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqFrequency",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxZmqFrequency,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqTimer",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqTimer,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqTimer",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxZmqTimer,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqStartingFrame",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqStartingFrame,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqStartingFrame",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxZmqStartingFrame,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqPort",
                       (void (Detector::*)(uint16_t, int)) &
                           Detector::setRxZmqPort,
                       py::arg(), py::arg() = -1);
    CppDetectorApi.def("getClientZmqPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getClientZmqPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setClientZmqPort",
                       (void (Detector::*)(uint16_t, int)) &
                           Detector::setClientZmqPort,
                       py::arg(), py::arg() = -1);
    CppDetectorApi.def(
        "getClientZmqIp",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getClientZmqIp,
        py::arg() = Positions{});
    CppDetectorApi.def("setClientZmqIp",
                       (void (Detector::*)(const sls::IpAddr, sls::Positions)) &
                           Detector::setClientZmqIp,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClientZmqHwm",
                       (int (Detector::*)() const) & Detector::getClientZmqHwm);
    CppDetectorApi.def(
        "setClientZmqHwm",
        (void (Detector::*)(const int)) & Detector::setClientZmqHwm, py::arg());
    CppDetectorApi.def("getRxZmqHwm",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxZmqHwm,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqHwm",
                       (void (Detector::*)(const int)) & Detector::setRxZmqHwm,
                       py::arg());
    CppDetectorApi.def("getSubExptime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getSubExptime,
                       py::arg() = Positions{});
    CppDetectorApi.def("setSubExptime",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setSubExptime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSubDeadTime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getSubDeadTime,
                       py::arg() = Positions{});
    CppDetectorApi.def("setSubDeadTime",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setSubDeadTime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getOverFlowMode",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getOverFlowMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setOverFlowMode",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setOverFlowMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRateCorrection",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getRateCorrection,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDefaultRateCorrection",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::setDefaultRateCorrection,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRateCorrection",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setRateCorrection,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInterruptSubframe",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getInterruptSubframe,
                       py::arg() = Positions{});
    CppDetectorApi.def("setInterruptSubframe",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::setInterruptSubframe,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMeasuredPeriod",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getMeasuredPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("getMeasuredSubFramePeriod",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getMeasuredSubFramePeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("getActive",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getActive,
                       py::arg() = Positions{});
    CppDetectorApi.def("setActive",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::setActive,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPartialReset",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getPartialReset,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPartialReset",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setPartialReset,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("pulsePixel",
                       (void (Detector::*)(int, defs::xy, sls::Positions)) &
                           Detector::pulsePixel,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("pulsePixelNMove",
                       (void (Detector::*)(int, defs::xy, sls::Positions)) &
                           Detector::pulsePixelNMove,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("pulseChip",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::pulseChip,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getQuad",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getQuad,
                       py::arg() = Positions{});
    CppDetectorApi.def("setQuad",
                       (void (Detector::*)(const bool)) & Detector::setQuad,
                       py::arg());
    CppDetectorApi.def("getDataStream",
                       (Result<bool>(Detector::*)(const defs::portPosition,
                                                  sls::Positions) const) &
                           Detector::getDataStream,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setDataStream",
                       (void (Detector::*)(const defs::portPosition, const bool,
                                           sls::Positions)) &
                           Detector::setDataStream,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTop",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getTop,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTop", (void (Detector::*)(bool, sls::Positions)) & Detector::setTop,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getChipVersion",
                       (Result<double>(Detector::*)(sls::Positions) const) &
                           Detector::getChipVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getThresholdTemperature",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getThresholdTemperature,
                       py::arg() = Positions{});
    CppDetectorApi.def("setThresholdTemperature",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setThresholdTemperature,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureControl",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getTemperatureControl,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTemperatureControl",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setTemperatureControl,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureEvent",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getTemperatureEvent,
                       py::arg() = Positions{});
    CppDetectorApi.def("resetTemperatureEvent",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::resetTemperatureEvent,
                       py::arg() = Positions{});
    CppDetectorApi.def("getAutoComparatorDisable",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getAutoComparatorDisable,
                       py::arg() = Positions{});
    CppDetectorApi.def("setAutoComparatorDisable",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setAutoComparatorDisable,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getComparatorDisableTime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getComparatorDisableTime,
                       py::arg() = Positions{});
    CppDetectorApi.def("setComparatorDisableTime",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setComparatorDisableTime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfAdditionalStorageCells",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfAdditionalStorageCells,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfAdditionalStorageCells",
                       (void (Detector::*)(int)) &
                           Detector::setNumberOfAdditionalStorageCells,
                       py::arg());
    CppDetectorApi.def("getStorageCellStart",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getStorageCellStart,
                       py::arg() = Positions{});
    CppDetectorApi.def("setStorageCellStart",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setStorageCellStart,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getStorageCellDelay",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getStorageCellDelay,
                       py::arg() = Positions{});
    CppDetectorApi.def("setStorageCellDelay",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setStorageCellDelay,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getGainModeList",
                       (std::vector<defs::gainMode>(Detector::*)() const) &
                           Detector::getGainModeList);
    CppDetectorApi.def(
        "getGainMode",
        (Result<defs::gainMode>(Detector::*)(sls::Positions) const) &
            Detector::getGainMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setGainMode",
        (void (Detector::*)(const defs::gainMode, sls::Positions)) &
            Detector::setGainMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFilterCells",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfFilterCells,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfFilterCells",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberOfFilterCells,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPedestalMode",
        (Result<defs::pedestalParameters>(Detector::*)(sls::Positions) const) &
            Detector::getPedestalMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setPedestalMode",
        (void (Detector::*)(const defs::pedestalParameters, sls::Positions)) &
            Detector::setPedestalMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getTimingInfoDecoder",
        (Result<defs::timingInfoDecoder>(Detector::*)(sls::Positions) const) &
            Detector::getTimingInfoDecoder,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setTimingInfoDecoder",
        (void (Detector::*)(defs::timingInfoDecoder, sls::Positions)) &
            Detector::setTimingInfoDecoder,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getCollectionMode",
        (Result<defs::collectionMode>(Detector::*)(sls::Positions) const) &
            Detector::getCollectionMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setCollectionMode",
        (void (Detector::*)(defs::collectionMode, sls::Positions)) &
            Detector::setCollectionMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getROI",
                       (Result<defs::ROI>(Detector::*)(sls::Positions) const) &
                           Detector::getROI,
                       py::arg() = Positions{});
    CppDetectorApi.def("setROI",
                       (void (Detector::*)(defs::ROI, int)) & Detector::setROI,
                       py::arg(), py::arg());
    CppDetectorApi.def(
        "clearROI", (void (Detector::*)(sls::Positions)) & Detector::clearROI,
        py::arg() = Positions{});
    CppDetectorApi.def("getExptimeLeft",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getExptimeLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfBursts",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfBursts,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfBursts",
        (void (Detector::*)(int64_t)) & Detector::setNumberOfBursts, py::arg());
    CppDetectorApi.def("getBurstPeriod",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getBurstPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("setBurstPeriod",
                       (void (Detector::*)(sls::ns, sls::Positions)) &
                           Detector::setBurstPeriod,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfBurstsLeft",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfBurstsLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getInjectChannel",
        (Result<std::array<int, 2>>(Detector::*)(sls::Positions)) &
            Detector::getInjectChannel,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setInjectChannel",
        (void (Detector::*)(const int, const int, sls::Positions)) &
            Detector::setInjectChannel,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getVetoPhoton",
        (void (Detector::*)(const int, const std::string &, sls::Positions)) &
            Detector::getVetoPhoton,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoPhoton",
        (void (Detector::*)(const int, const int, const int,
                            const std::string &, sls::Positions)) &
            Detector::setVetoPhoton,
        py::arg(), py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoReference",
        (void (Detector::*)(const int, const int, sls::Positions)) &
            Detector::setVetoReference,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoFile",
        (void (Detector::*)(const int, const std::string &, sls::Positions)) &
            Detector::setVetoFile,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getBurstMode",
                       (Result<defs::burstMode>(Detector::*)(sls::Positions)) &
                           Detector::getBurstMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setBurstMode",
                       (void (Detector::*)(defs::burstMode, sls::Positions)) &
                           Detector::setBurstMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getCDSGain",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getCDSGain,
                       py::arg() = Positions{});
    CppDetectorApi.def("setCDSGain",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setCDSGain,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getTimingSource",
        (Result<defs::timingSourceType>(Detector::*)(sls::Positions) const) &
            Detector::getTimingSource,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setTimingSource",
        (void (Detector::*)(defs::timingSourceType, sls::Positions)) &
            Detector::setTimingSource,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getVeto",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getVeto,
                       py::arg() = Positions{});
    CppDetectorApi.def("setVeto",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::setVeto,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getVetoStream",
        (Result<defs::streamingInterface>(Detector::*)(sls::Positions) const) &
            Detector::getVetoStream,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoStream",
        (void (Detector::*)(const defs::streamingInterface, sls::Positions)) &
            Detector::setVetoStream,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getVetoAlgorithm",
        (Result<defs::vetoAlgorithm>(Detector::*)(
            const defs::streamingInterface, sls::Positions) const) &
            Detector::getVetoAlgorithm,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoAlgorithm",
        (void (Detector::*)(const defs::vetoAlgorithm,
                            const defs::streamingInterface, sls::Positions)) &
            Detector::setVetoAlgorithm,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getADCConfiguration",
        (Result<int>(Detector::*)(const int, const int, sls::Positions) const) &
            Detector::getADCConfiguration,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCConfiguration",
        (void (Detector::*)(const int, const int, const int, sls::Positions)) &
            Detector::setADCConfiguration,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getCounterMask",
                       (Result<uint32_t>(Detector::*)(sls::Positions) const) &
                           Detector::getCounterMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setCounterMask",
                       (void (Detector::*)(uint32_t, sls::Positions)) &
                           Detector::setCounterMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfGates",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfGates,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfGates",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberOfGates,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getExptime",
        (Result<sls::ns>(Detector::*)(int, sls::Positions) const) &
            Detector::getExptime,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setExptime",
                       (void (Detector::*)(int, sls::ns, sls::Positions)) &
                           Detector::setExptime,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getExptimeForAllGates",
        (Result<std::array<ns, 3>>(Detector::*)(sls::Positions) const) &
            Detector::getExptimeForAllGates,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getGateDelay",
        (Result<sls::ns>(Detector::*)(int, sls::Positions) const) &
            Detector::getGateDelay,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setGateDelay",
                       (void (Detector::*)(int, sls::ns, sls::Positions)) &
                           Detector::setGateDelay,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getGateDelayForAllGates",
        (Result<std::array<ns, 3>>(Detector::*)(sls::Positions) const) &
            Detector::getGateDelayForAllGates,
        py::arg() = Positions{});
    CppDetectorApi.def("getChipStatusRegister",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getChipStatusRegister,
                       py::arg() = Positions{});
    CppDetectorApi.def("setGainCaps",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setGainCaps,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getGainCaps",
                       (Result<int>(Detector::*)(sls::Positions)) &
                           Detector::getGainCaps,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getPolarity",
        (Result<defs::polarity>(Detector::*)(sls::Positions) const) &
            Detector::getPolarity,
        py::arg() = Positions{});
    CppDetectorApi.def("setPolarity",
                       (void (Detector::*)(defs::polarity, sls::Positions)) &
                           Detector::setPolarity,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInterpolation",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getInterpolation,
                       py::arg() = Positions{});
    CppDetectorApi.def("setInterpolation",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setInterpolation,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPumpProbe",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getPumpProbe,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPumpProbe",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setPumpProbe,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAnalogPulsing",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getAnalogPulsing,
                       py::arg() = Positions{});
    CppDetectorApi.def("setAnalogPulsing",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setAnalogPulsing,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDigitalPulsing",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getDigitalPulsing,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDigitalPulsing",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setDigitalPulsing,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfAnalogSamples",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfAnalogSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfAnalogSamples",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberOfAnalogSamples,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCClock",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getADCClock,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCClock",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setADCClock,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRUNClock",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRUNClock,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRUNClock",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRUNClock,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSYNCClock",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getSYNCClock,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPowerList",
                       (std::vector<defs::dacIndex>(Detector::*)() const) &
                           Detector::getPowerList);
    CppDetectorApi.def("getSlowADCList",
                       (std::vector<defs::dacIndex>(Detector::*)() const) &
                           Detector::getSlowADCList);
    CppDetectorApi.def(
        "getPower",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions) const) &
            Detector::getPower,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPower",
        (void (Detector::*)(defs::dacIndex, int, sls::Positions)) &
            Detector::setPower,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCVpp",
                       (Result<int>(Detector::*)(bool, sls::Positions) const) &
                           Detector::getADCVpp,
                       py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def("setADCVpp",
                       (void (Detector::*)(int, bool, sls::Positions)) &
                           Detector::setADCVpp,
                       py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def("getADCEnableMask",
                       (Result<uint32_t>(Detector::*)(sls::Positions) const) &
                           Detector::getADCEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCEnableMask",
                       (void (Detector::*)(uint32_t, sls::Positions)) &
                           Detector::setADCEnableMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTenGigaADCEnableMask",
                       (Result<uint32_t>(Detector::*)(sls::Positions) const) &
                           Detector::getTenGigaADCEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTenGigaADCEnableMask",
                       (void (Detector::*)(uint32_t, sls::Positions)) &
                           Detector::setTenGigaADCEnableMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransceiverEnableMask",
                       (Result<uint32_t>(Detector::*)(sls::Positions) const) &
                           Detector::getTransceiverEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransceiverEnableMask",
                       (void (Detector::*)(uint32_t, sls::Positions)) &
                           Detector::setTransceiverEnableMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfDigitalSamples",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfDigitalSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfDigitalSamples",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberOfDigitalSamples,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfTransceiverSamples",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfTransceiverSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfTransceiverSamples",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setNumberOfTransceiverSamples,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getReadoutMode",
        (Result<defs::readoutMode>(Detector::*)(sls::Positions) const) &
            Detector::getReadoutMode,
        py::arg() = Positions{});
    CppDetectorApi.def("setReadoutMode",
                       (void (Detector::*)(defs::readoutMode, sls::Positions)) &
                           Detector::setReadoutMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITClock",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getDBITClock,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDBITClock",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setDBITClock,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getMeasuredPower",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions) const) &
            Detector::getMeasuredPower,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getMeasuredCurrent",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions) const) &
            Detector::getMeasuredCurrent,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSlowADC",
        (Result<int>(Detector::*)(defs::dacIndex, sls::Positions) const) &
            Detector::getSlowADC,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExternalSamplingSource",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getExternalSamplingSource,
                       py::arg() = Positions{});
    CppDetectorApi.def("setExternalSamplingSource",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setExternalSamplingSource,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExternalSampling",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getExternalSampling,
                       py::arg() = Positions{});
    CppDetectorApi.def("setExternalSampling",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setExternalSampling,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxDbitList",
        (Result<std::vector<int>>(Detector::*)(sls::Positions) const) &
            Detector::getRxDbitList,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxDbitList",
        (void (Detector::*)(const std::vector<int> &, sls::Positions)) &
            Detector::setRxDbitList,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxDbitOffset",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getRxDbitOffset,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxDbitOffset",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setRxDbitOffset,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setDigitalIODelay",
                       (void (Detector::*)(uint64_t, int, sls::Positions)) &
                           Detector::setDigitalIODelay,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getLEDEnable",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getLEDEnable,
                       py::arg() = Positions{});
    CppDetectorApi.def("setLEDEnable",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setLEDEnable,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setDacNames",
                       (void (Detector::*)(const std::vector<std::string>)) &
                           Detector::setDacNames,
                       py::arg());
    CppDetectorApi.def("getDacNames",
                       (std::vector<std::string>(Detector::*)() const) &
                           Detector::getDacNames);
    CppDetectorApi.def(
        "getDacIndex",
        (defs::dacIndex(Detector::*)(const std::string &) const) &
            Detector::getDacIndex,
        py::arg());
    CppDetectorApi.def(
        "setDacName",
        (void (Detector::*)(const defs::dacIndex, const std::string &)) &
            Detector::setDacName,
        py::arg(), py::arg());
    CppDetectorApi.def("getDacName",
                       (std::string(Detector::*)(const defs::dacIndex) const) &
                           Detector::getDacName,
                       py::arg());
    CppDetectorApi.def("setAdcNames",
                       (void (Detector::*)(const std::vector<std::string>)) &
                           Detector::setAdcNames,
                       py::arg());
    CppDetectorApi.def("getAdcNames",
                       (std::vector<std::string>(Detector::*)() const) &
                           Detector::getAdcNames);
    CppDetectorApi.def("getAdcIndex",
                       (int (Detector::*)(const std::string &) const) &
                           Detector::getAdcIndex,
                       py::arg());
    CppDetectorApi.def("setAdcName",
                       (void (Detector::*)(const int, const std::string &)) &
                           Detector::setAdcName,
                       py::arg(), py::arg());
    CppDetectorApi.def("getAdcName",
                       (std::string(Detector::*)(const int) const) &
                           Detector::getAdcName,
                       py::arg());
    CppDetectorApi.def("setSignalNames",
                       (void (Detector::*)(const std::vector<std::string>)) &
                           Detector::setSignalNames,
                       py::arg());
    CppDetectorApi.def("getSignalNames",
                       (std::vector<std::string>(Detector::*)() const) &
                           Detector::getSignalNames);
    CppDetectorApi.def("getSignalIndex",
                       (int (Detector::*)(const std::string &) const) &
                           Detector::getSignalIndex,
                       py::arg());
    CppDetectorApi.def("setSignalName",
                       (void (Detector::*)(const int, const std::string &)) &
                           Detector::setSignalName,
                       py::arg(), py::arg());
    CppDetectorApi.def("getSignalName",
                       (std::string(Detector::*)(const int) const) &
                           Detector::getSignalName,
                       py::arg());
    CppDetectorApi.def("setPowerNames",
                       (void (Detector::*)(const std::vector<std::string>)) &
                           Detector::setPowerNames,
                       py::arg());
    CppDetectorApi.def("getPowerNames",
                       (std::vector<std::string>(Detector::*)() const) &
                           Detector::getPowerNames);
    CppDetectorApi.def(
        "getPowerIndex",
        (defs::dacIndex(Detector::*)(const std::string &) const) &
            Detector::getPowerIndex,
        py::arg());
    CppDetectorApi.def(
        "setPowerName",
        (void (Detector::*)(const defs::dacIndex, const std::string &)) &
            Detector::setPowerName,
        py::arg(), py::arg());
    CppDetectorApi.def("getPowerName",
                       (std::string(Detector::*)(const defs::dacIndex) const) &
                           Detector::getPowerName,
                       py::arg());
    CppDetectorApi.def("setSlowADCNames",
                       (void (Detector::*)(const std::vector<std::string>)) &
                           Detector::setSlowADCNames,
                       py::arg());
    CppDetectorApi.def("getSlowADCNames",
                       (std::vector<std::string>(Detector::*)() const) &
                           Detector::getSlowADCNames);
    CppDetectorApi.def(
        "getSlowADCIndex",
        (defs::dacIndex(Detector::*)(const std::string &) const) &
            Detector::getSlowADCIndex,
        py::arg());
    CppDetectorApi.def(
        "setSlowADCName",
        (void (Detector::*)(const defs::dacIndex, const std::string &)) &
            Detector::setSlowADCName,
        py::arg(), py::arg());
    CppDetectorApi.def("getSlowADCName",
                       (std::string(Detector::*)(const defs::dacIndex) const) &
                           Detector::getSlowADCName,
                       py::arg());
    CppDetectorApi.def("configureTransceiver",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::configureTransceiver,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getPatterFileName",
        (Result<std::string>(Detector::*)(sls::Positions) const) &
            Detector::getPatterFileName,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setPattern",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::setPattern,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPattern",
        (void (Detector::*)(const sls::Pattern &, sls::Positions)) &
            Detector::setPattern,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("savePattern",
                       (void (Detector::*)(const std::string &)) &
                           Detector::savePattern,
                       py::arg());
    CppDetectorApi.def("loadDefaultPattern",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::loadDefaultPattern,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPatternIOControl",
                       (Result<uint64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getPatternIOControl,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPatternIOControl",
                       (void (Detector::*)(uint64_t, sls::Positions)) &
                           Detector::setPatternIOControl,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWord",
                       (Result<uint64_t>(Detector::*)(int, sls::Positions)) &
                           Detector::getPatternWord,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternWord",
                       (void (Detector::*)(int, uint64_t, sls::Positions)) &
                           Detector::setPatternWord,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPatternLoopAddresses",
        (Result<std::array<int, 2>>(Detector::*)(int, sls::Positions) const) &
            Detector::getPatternLoopAddresses,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternLoopAddresses",
                       (void (Detector::*)(int, int, int, sls::Positions)) &
                           Detector::setPatternLoopAddresses,
                       py::arg(), py::arg(), py::arg(),
                       py::arg() = Positions{});
    CppDetectorApi.def("getPatternLoopCycles",
                       (Result<int>(Detector::*)(int, sls::Positions) const) &
                           Detector::getPatternLoopCycles,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternLoopCycles",
                       (void (Detector::*)(int, int, sls::Positions)) &
                           Detector::setPatternLoopCycles,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWaitAddr",
                       (Result<int>(Detector::*)(int, sls::Positions) const) &
                           Detector::getPatternWaitAddr,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternWaitAddr",
                       (void (Detector::*)(int, int, sls::Positions)) &
                           Detector::setPatternWaitAddr,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPatternWaitTime",
        (Result<uint64_t>(Detector::*)(int, sls::Positions) const) &
            Detector::getPatternWaitTime,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternWaitTime",
                       (void (Detector::*)(int, uint64_t, sls::Positions)) &
                           Detector::setPatternWaitTime,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternMask",
                       (Result<uint64_t>(Detector::*)(sls::Positions)) &
                           Detector::getPatternMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPatternMask",
                       (void (Detector::*)(uint64_t, sls::Positions)) &
                           Detector::setPatternMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternBitMask",
                       (Result<uint64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getPatternBitMask,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPatternBitMask",
                       (void (Detector::*)(uint64_t, sls::Positions)) &
                           Detector::setPatternBitMask,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("startPattern",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::startPattern,
                       py::arg() = Positions{});
    CppDetectorApi.def("getAdditionalJsonHeader",
                       (Result<std::map<std::string, std::string>>(Detector::*)(
                           sls::Positions) const) &
                           Detector::getAdditionalJsonHeader,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setAdditionalJsonHeader",
        (void (Detector::*)(const std::map<std::string, std::string> &,
                            sls::Positions)) &
            Detector::setAdditionalJsonHeader,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAdditionalJsonParameter",
                       (Result<std::string>(Detector::*)(
                           const std::string &, sls::Positions) const) &
                           Detector::getAdditionalJsonParameter,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setAdditionalJsonParameter",
        (void (Detector::*)(const std::string &, const std::string &,
                            sls::Positions)) &
            Detector::setAdditionalJsonParameter,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCPipeline",
                       (Result<int>(Detector::*)(sls::Positions) const) &
                           Detector::getADCPipeline,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCPipeline",
                       (void (Detector::*)(int, sls::Positions)) &
                           Detector::setADCPipeline,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "programFPGA",
        (void (Detector::*)(const std::string &, const bool, sls::Positions)) &
            Detector::programFPGA,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "resetFPGA", (void (Detector::*)(sls::Positions)) & Detector::resetFPGA,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "updateDetectorServer",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::updateDetectorServer,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "updateKernel",
        (void (Detector::*)(const std::string &, sls::Positions)) &
            Detector::updateKernel,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("rebootController",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::rebootController,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "updateFirmwareAndServer",
        (void (Detector::*)(const std::string &, const std::string &,
                            sls::Positions)) &
            Detector::updateFirmwareAndServer,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getUpdateMode",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getUpdateMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setUpdateMode",
                       (void (Detector::*)(const bool, sls::Positions)) &
                           Detector::setUpdateMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "readRegister",
        (Result<uint32_t>(Detector::*)(uint32_t, sls::Positions) const) &
            Detector::readRegister,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "writeRegister",
        (void (Detector::*)(uint32_t, uint32_t, bool, sls::Positions)) &
            Detector::writeRegister,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "setBit",
        (void (Detector::*)(uint32_t, int, bool, sls::Positions)) &
            Detector::setBit,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "clearBit",
        (void (Detector::*)(uint32_t, int, bool, sls::Positions)) &
            Detector::clearBit,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "getBit",
        (Result<int>(Detector::*)(uint32_t, int, sls::Positions)) &
            Detector::getBit,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("executeFirmwareTest",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::executeFirmwareTest,
                       py::arg() = Positions{});
    CppDetectorApi.def("executeBusTest",
                       (void (Detector::*)(sls::Positions)) &
                           Detector::executeBusTest,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "writeAdcRegister",
        (void (Detector::*)(uint32_t, uint32_t, sls::Positions)) &
            Detector::writeAdcRegister,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInitialChecks", (bool (Detector::*)() const) &
                                               Detector::getInitialChecks);
    CppDetectorApi.def("setInitialChecks",
                       (void (Detector::*)(const bool)) &
                           Detector::setInitialChecks,
                       py::arg());
    CppDetectorApi.def("getADCInvert",
                       (Result<uint32_t>(Detector::*)(sls::Positions) const) &
                           Detector::getADCInvert,
                       py::arg() = Positions{});
    CppDetectorApi.def("setADCInvert",
                       (void (Detector::*)(uint32_t, sls::Positions)) &
                           Detector::setADCInvert,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getControlPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getControlPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setControlPort",
                       (void (Detector::*)(uint16_t, sls::Positions)) &
                           Detector::setControlPort,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getStopPort",
                       (Result<uint16_t>(Detector::*)(sls::Positions) const) &
                           Detector::getStopPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setStopPort",
                       (void (Detector::*)(uint16_t, sls::Positions)) &
                           Detector::setStopPort,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDetectorLock",
                       (Result<bool>(Detector::*)(sls::Positions) const) &
                           Detector::getDetectorLock,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDetectorLock",
                       (void (Detector::*)(bool, sls::Positions)) &
                           Detector::setDetectorLock,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getLastClientIP",
        (Result<sls::IpAddr>(Detector::*)(sls::Positions) const) &
            Detector::getLastClientIP,
        py::arg() = Positions{});
    CppDetectorApi.def("executeCommand",
                       (Result<std::string>(Detector::*)(const std::string &,
                                                         sls::Positions)) &
                           Detector::executeCommand,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFramesFromStart",
                       (Result<int64_t>(Detector::*)(sls::Positions) const) &
                           Detector::getNumberOfFramesFromStart,
                       py::arg() = Positions{});
    CppDetectorApi.def("getActualTime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getActualTime,
                       py::arg() = Positions{});
    CppDetectorApi.def("getMeasurementTime",
                       (Result<sls::ns>(Detector::*)(sls::Positions) const) &
                           Detector::getMeasurementTime,
                       py::arg() = Positions{});
    CppDetectorApi.def("getUserDetails", (std::string(Detector::*)() const) &
                                             Detector::getUserDetails);
    ;
}

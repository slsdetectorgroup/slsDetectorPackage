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
    using namespace sls; // TODO! qualify arguments and return types to avoid
                         // this

    m.def("freeSharedMemory",
          (void (*)(const int, const int))&sls::freeSharedMemory, py::arg() = 0,
          py::arg() = -1);

    m.def("getUserDetails", (std::string (*)(const int))&sls::getUserDetails,
          py::arg() = 0);

    py::class_<Detector> CppDetectorApi(m, "CppDetectorApi");
    CppDetectorApi.def(py::init<int>());

    CppDetectorApi.def(
        "loadConfig",
        (void (Detector::*)(const std::string &))&Detector::loadConfig,
        py::arg());
    CppDetectorApi.def(
        "loadParameters",
        (void (Detector::*)(const std::string &))&Detector::loadParameters,
        py::arg());
    CppDetectorApi.def(
        "loadParameters",
        (void (Detector::*)(
            const std::vector<std::string> &))&Detector::loadParameters,
        py::arg());
    CppDetectorApi.def("getHostname",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getHostname,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setHostname",
        (void (Detector::*)(
            const std::vector<std::string> &))&Detector::setHostname,
        py::arg());
    CppDetectorApi.def(
        "setVirtualDetectorServers",
        (void (Detector::*)(int, uint16_t))&Detector::setVirtualDetectorServers,
        py::arg(), py::arg());
    CppDetectorApi.def("getShmId",
                       (int (Detector::*)() const) & Detector::getShmId);
    CppDetectorApi.def("getPackageVersion",
                       (std::string (Detector::*)() const) &
                           Detector::getPackageVersion);
    CppDetectorApi.def("getClientVersion", (std::string (Detector::*)() const) &
                                               Detector::getClientVersion);
    CppDetectorApi.def("getFirmwareVersion",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getFirmwareVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getFrontEndFirmwareVersion",
                       (Result<int64_t> (Detector::*)(const defs::fpgaPosition,
                                                      Positions) const) &
                           Detector::getFrontEndFirmwareVersion,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDetectorServerVersion",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getDetectorServerVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getHardwareVersion",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getHardwareVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getKernelVersion",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getKernelVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getSerialNumber",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getSerialNumber,
                       py::arg() = Positions{});
    CppDetectorApi.def("getModuleId",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getModuleId,
                       py::arg() = Positions{});
    CppDetectorApi.def("getReceiverVersion",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getReceiverVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getDetectorType",
        (Result<defs::detectorType> (Detector::*)(Positions) const) &
            Detector::getDetectorType,
        py::arg() = Positions{});
    CppDetectorApi.def("size", (int (Detector::*)() const) & Detector::size);
    CppDetectorApi.def("empty", (bool (Detector::*)() const) & Detector::empty);
    CppDetectorApi.def("getModuleGeometry", (defs::xy (Detector::*)() const) &
                                                Detector::getModuleGeometry);
    CppDetectorApi.def("getModuleSize",
                       (Result<defs::xy> (Detector::*)(Positions) const) &
                           Detector::getModuleSize,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPortPerModuleGeometry",
                       (defs::xy (Detector::*)() const) &
                           Detector::getPortPerModuleGeometry);
    CppDetectorApi.def("getPortSize",
                       (Result<defs::xy> (Detector::*)(Positions) const) &
                           Detector::getPortSize,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDetectorSize", (defs::xy (Detector::*)() const) &
                                              Detector::getDetectorSize);
    CppDetectorApi.def(
        "setDetectorSize",
        (void (Detector::*)(const defs::xy))&Detector::setDetectorSize,
        py::arg());
    CppDetectorApi.def("getSettingsList", (std::vector<defs::detectorSettings> (
                                              Detector::*)() const) &
                                              Detector::getSettingsList);
    CppDetectorApi.def(
        "getSettings",
        (Result<defs::detectorSettings> (Detector::*)(Positions) const) &
            Detector::getSettings,
        py::arg() = Positions{});
    CppDetectorApi.def("setSettings",
                       (void (Detector::*)(defs::detectorSettings,
                                           Positions))&Detector::setSettings,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getThresholdEnergy",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getThresholdEnergy,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getAllThresholdEnergy",
        (Result<std::array<int, 3>> (Detector::*)(Positions) const) &
            Detector::getAllThresholdEnergy,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setThresholdEnergy",
        (void (Detector::*)(int, defs::detectorSettings, bool,
                            Positions))&Detector::setThresholdEnergy,
        py::arg(), py::arg() = defs::STANDARD, py::arg() = true,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setThresholdEnergy",
        (void (Detector::*)(std::array<int, 3>, defs::detectorSettings, bool,
                            Positions))&Detector::setThresholdEnergy,
        py::arg(), py::arg() = defs::STANDARD, py::arg() = true,
        py::arg() = Positions{});
    CppDetectorApi.def("getSettingsPath",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getSettingsPath,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSettingsPath",
        (void (Detector::*)(const std::string &,
                            Positions))&Detector::setSettingsPath,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("loadTrimbits",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::loadTrimbits,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("saveTrimbits",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::saveTrimbits,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAllTrimbits",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getAllTrimbits,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setAllTrimbits",
        (void (Detector::*)(int, Positions))&Detector::setAllTrimbits,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getTrimEnergies",
        (Result<std::vector<int>> (Detector::*)(Positions) const) &
            Detector::getTrimEnergies,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setTrimEnergies",
        (void (Detector::*)(std::vector<int>,
                            Positions))&Detector::setTrimEnergies,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getGapPixelsinCallback",
                       (bool (Detector::*)() const) &
                           Detector::getGapPixelsinCallback);
    CppDetectorApi.def(
        "setGapPixelsinCallback",
        (void (Detector::*)(const bool))&Detector::setGapPixelsinCallback,
        py::arg());
    CppDetectorApi.def("getFlipRows",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getFlipRows,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFlipRows",
        (void (Detector::*)(bool, Positions))&Detector::setFlipRows, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getMaster",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getMaster,
                       py::arg() = Positions{});
    CppDetectorApi.def("setMaster",
                       (void (Detector::*)(bool, int))&Detector::setMaster,
                       py::arg(), py::arg());
    CppDetectorApi.def("getSynchronization",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getSynchronization,
                       py::arg() = Positions{});
    CppDetectorApi.def("setSynchronization",
                       (void (Detector::*)(bool))&Detector::setSynchronization,
                       py::arg());
    CppDetectorApi.def(
        "getBadChannels",
        (void (Detector::*)(const std::string &, Positions) const) &
            Detector::getBadChannels,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setBadChannels",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::setBadChannels,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getBadChannels",
        (Result<std::vector<int>> (Detector::*)(Positions) const) &
            Detector::getBadChannels,
        py::arg() = Positions{});
    CppDetectorApi.def("setBadChannels",
                       (void (Detector::*)(const std::vector<int>,
                                           Positions))&Detector::setBadChannels,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setBadChannels",
        (void (Detector::*)(
            const std::vector<std::vector<int>>))&Detector::setBadChannels,
        py::arg());
    CppDetectorApi.def("getRow",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRow,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRow", (void (Detector::*)(const int, Positions))&Detector::setRow,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getColumn",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getColumn,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setColumn",
        (void (Detector::*)(const int, Positions))&Detector::setColumn,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("isVirtualDetectorServer",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::isVirtualDetectorServer,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "registerAcquisitionFinishedCallback",
        (void (Detector::*)(
            void (*)(double, int, void *),
            void *))&Detector::registerAcquisitionFinishedCallback,
        py::arg(), py::arg());
    CppDetectorApi.def("registerDataCallback",
                       (void (Detector::*)(
                           void (*)(detectorData *, uint64_t, uint32_t, void *),
                           void *))&Detector::registerDataCallback,
                       py::arg(), py::arg());
    CppDetectorApi.def("getNumberOfFrames",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfFrames,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfFrames",
        (void (Detector::*)(int64_t))&Detector::setNumberOfFrames, py::arg());
    CppDetectorApi.def("getNumberOfTriggers",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfTriggers,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfTriggers",
        (void (Detector::*)(int64_t))&Detector::setNumberOfTriggers, py::arg());
    CppDetectorApi.def("getExptime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getExptime,
                       py::arg() = Positions{});
    CppDetectorApi.def("setExptime",
                       (void (Detector::*)(ns, Positions))&Detector::setExptime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPeriod",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPeriod",
                       (void (Detector::*)(ns, Positions))&Detector::setPeriod,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDelayAfterTrigger",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getDelayAfterTrigger,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDelayAfterTrigger",
        (void (Detector::*)(ns, Positions))&Detector::setDelayAfterTrigger,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFramesLeft",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfFramesLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfTriggersLeft",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfTriggersLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPeriodLeft",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getPeriodLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDelayAfterTriggerLeft",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getDelayAfterTriggerLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDynamicRange",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getDynamicRange,
                       py::arg() = Positions{});
    CppDetectorApi.def("setDynamicRange",
                       (void (Detector::*)(int))&Detector::setDynamicRange,
                       py::arg());
    CppDetectorApi.def("getDynamicRangeList",
                       (std::vector<int> (Detector::*)() const) &
                           Detector::getDynamicRangeList);
    CppDetectorApi.def(
        "getTimingMode",
        (Result<defs::timingMode> (Detector::*)(Positions) const) &
            Detector::getTimingMode,
        py::arg() = Positions{});
    CppDetectorApi.def("setTimingMode",
                       (void (Detector::*)(defs::timingMode,
                                           Positions))&Detector::setTimingMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTimingModeList",
                       (std::vector<defs::timingMode> (Detector::*)() const) &
                           Detector::getTimingModeList);
    CppDetectorApi.def(
        "getReadoutSpeed",
        (Result<defs::speedLevel> (Detector::*)(Positions) const) &
            Detector::getReadoutSpeed,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setReadoutSpeed",
        (void (Detector::*)(defs::speedLevel,
                            Positions))&Detector::setReadoutSpeed,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getReadoutSpeedList",
                       (std::vector<defs::speedLevel> (Detector::*)() const) &
                           Detector::getReadoutSpeedList);
    CppDetectorApi.def("getADCPhase",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getADCPhase,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCPhase",
        (void (Detector::*)(int, Positions))&Detector::setADCPhase, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getMaxADCPhaseShift",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getMaxADCPhaseShift,
                       py::arg() = Positions{});
    CppDetectorApi.def("getADCPhaseInDegrees",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getADCPhaseInDegrees,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCPhaseInDegrees",
        (void (Detector::*)(int, Positions))&Detector::setADCPhaseInDegrees,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITPhase",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getDBITPhase,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDBITPhase",
        (void (Detector::*)(int, Positions))&Detector::setDBITPhase, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getMaxDBITPhaseShift",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getMaxDBITPhaseShift,
                       py::arg() = Positions{});
    CppDetectorApi.def("getDBITPhaseInDegrees",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getDBITPhaseInDegrees,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDBITPhaseInDegrees",
        (void (Detector::*)(int, Positions))&Detector::setDBITPhaseInDegrees,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getClockFrequency",
        (Result<int> (Detector::*)(int, Positions))&Detector::getClockFrequency,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getClockPhase",
        (Result<int> (Detector::*)(int, Positions))&Detector::getClockPhase,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setClockPhase",
        (void (Detector::*)(int, int, Positions))&Detector::setClockPhase,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMaxClockPhaseShift",
                       (Result<int> (Detector::*)(
                           int, Positions))&Detector::getMaxClockPhaseShift,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClockPhaseinDegrees",
                       (Result<int> (Detector::*)(
                           int, Positions))&Detector::getClockPhaseinDegrees,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setClockPhaseinDegrees",
        (void (Detector::*)(int, int,
                            Positions))&Detector::setClockPhaseinDegrees,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getClockDivider",
        (Result<int> (Detector::*)(int, Positions))&Detector::getClockDivider,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setClockDivider",
        (void (Detector::*)(int, int, Positions))&Detector::setClockDivider,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getHighVoltage",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getHighVoltage,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setHighVoltage",
        (void (Detector::*)(int, Positions))&Detector::setHighVoltage,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPowerChip",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getPowerChip,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPowerChip",
        (void (Detector::*)(bool, Positions))&Detector::setPowerChip, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getImageTestMode",
        (Result<int> (Detector::*)(Positions))&Detector::getImageTestMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setImageTestMode",
        (void (Detector::*)(const int, Positions))&Detector::setImageTestMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureList",
                       (std::vector<defs::dacIndex> (Detector::*)() const) &
                           Detector::getTemperatureList);
    CppDetectorApi.def(
        "getTemperature",
        (Result<int> (Detector::*)(defs::dacIndex, Positions) const) &
            Detector::getTemperature,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDacList",
                       (std::vector<defs::dacIndex> (Detector::*)() const) &
                           Detector::getDacList);
    CppDetectorApi.def("getDefaultDac",
                       (Result<int> (Detector::*)(
                           defs::dacIndex, Positions))&Detector::getDefaultDac,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setDefaultDac",
                       (void (Detector::*)(defs::dacIndex, int,
                                           Positions))&Detector::setDefaultDac,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDefaultDac",
        (Result<int> (Detector::*)(defs::dacIndex, defs::detectorSettings,
                                   Positions))&Detector::getDefaultDac,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDefaultDac",
        (void (Detector::*)(defs::dacIndex, int, defs::detectorSettings,
                            Positions))&Detector::setDefaultDac,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("resetToDefaultDacs",
                       (void (Detector::*)(
                           const bool, Positions))&Detector::resetToDefaultDacs,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getDAC",
        (Result<int> (Detector::*)(defs::dacIndex, bool, Positions) const) &
            Detector::getDAC,
        py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def("setDAC",
                       (void (Detector::*)(defs::dacIndex, int, bool,
                                           Positions))&Detector::setDAC,
                       py::arg(), py::arg(), py::arg() = false,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getOnChipDAC",
        (Result<int> (Detector::*)(defs::dacIndex, int, Positions) const) &
            Detector::getOnChipDAC,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setOnChipDAC",
                       (void (Detector::*)(defs::dacIndex, int, int,
                                           Positions))&Detector::setOnChipDAC,
                       py::arg(), py::arg(), py::arg(),
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getExternalSignalFlags",
        (Result<defs::externalSignalFlag> (Detector::*)(int, Positions) const) &
            Detector::getExternalSignalFlags,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setExternalSignalFlags",
        (void (Detector::*)(int, defs::externalSignalFlag,
                            Positions))&Detector::setExternalSignalFlags,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getParallelMode",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getParallelMode,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setParallelMode",
        (void (Detector::*)(bool, Positions))&Detector::setParallelMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFilterResistor",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getFilterResistor,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFilterResistor",
        (void (Detector::*)(int, Positions))&Detector::setFilterResistor,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getCurrentSource",
        (Result<defs::currentSrcParameters> (Detector::*)(Positions) const) &
            Detector::getCurrentSource,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setCurrentSource",
        (void (Detector::*)(defs::currentSrcParameters,
                            Positions))&Detector::setCurrentSource,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITPipeline",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getDBITPipeline,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDBITPipeline",
        (void (Detector::*)(int, Positions))&Detector::setDBITPipeline,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getReadNRows",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getReadNRows,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setReadNRows",
        (void (Detector::*)(const int, Positions))&Detector::setReadNRows,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("acquire", (void (Detector::*)())&Detector::acquire);
    CppDetectorApi.def("clearAcquiringFlag",
                       (void (Detector::*)())&Detector::clearAcquiringFlag);
    CppDetectorApi.def("startReceiver",
                       (void (Detector::*)())&Detector::startReceiver);
    CppDetectorApi.def("stopReceiver",
                       (void (Detector::*)())&Detector::stopReceiver);
    CppDetectorApi.def("startDetector",
                       (void (Detector::*)(Positions))&Detector::startDetector,
                       py::arg() = Positions{});
    CppDetectorApi.def("startDetectorReadout",
                       (void (Detector::*)())&Detector::startDetectorReadout);
    CppDetectorApi.def("stopDetector",
                       (void (Detector::*)(Positions))&Detector::stopDetector,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getDetectorStatus",
        (Result<defs::runStatus> (Detector::*)(Positions) const) &
            Detector::getDetectorStatus,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getReceiverStatus",
        (Result<defs::runStatus> (Detector::*)(Positions) const) &
            Detector::getReceiverStatus,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getFramesCaught",
        (Result<std::vector<int64_t>> (Detector::*)(Positions) const) &
            Detector::getFramesCaught,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getNumMissingPackets",
        (Result<std::vector<int64_t>> (Detector::*)(Positions) const) &
            Detector::getNumMissingPackets,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxCurrentFrameIndex",
        (Result<std::vector<int64_t>> (Detector::*)(Positions) const) &
            Detector::getRxCurrentFrameIndex,
        py::arg() = Positions{});
    CppDetectorApi.def("getNextFrameNumber",
                       (Result<uint64_t> (Detector::*)(Positions) const) &
                           Detector::getNextFrameNumber,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNextFrameNumber",
        (void (Detector::*)(uint64_t, Positions))&Detector::setNextFrameNumber,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "sendSoftwareTrigger",
        (void (Detector::*)(const bool,
                            Positions))&Detector::sendSoftwareTrigger,
        py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "getScan",
        (Result<defs::scanParameters> (Detector::*)(Positions) const) &
            Detector::getScan,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setScan",
        (void (Detector::*)(const defs::scanParameters))&Detector::setScan,
        py::arg());
    CppDetectorApi.def("getScanErrorMessage",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getScanErrorMessage,
                       py::arg() = Positions{});
    CppDetectorApi.def("getNumberofUDPInterfaces",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberofUDPInterfaces,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberofUDPInterfaces",
        (void (Detector::*)(int, Positions))&Detector::setNumberofUDPInterfaces,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSelectedUDPInterface",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getSelectedUDPInterface,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "selectUDPInterface",
        (void (Detector::*)(int, Positions))&Detector::selectUDPInterface,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSourceUDPIP",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getSourceUDPIP,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSourceUDPIP",
        (void (Detector::*)(const IpAddr, Positions))&Detector::setSourceUDPIP,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSourceUDPIP2",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getSourceUDPIP2,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSourceUDPIP2",
        (void (Detector::*)(const IpAddr, Positions))&Detector::setSourceUDPIP2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSourceUDPMAC",
                       (Result<MacAddr> (Detector::*)(Positions) const) &
                           Detector::getSourceUDPMAC,
                       py::arg() = Positions{});
    CppDetectorApi.def("setSourceUDPMAC",
                       (void (Detector::*)(
                           const MacAddr, Positions))&Detector::setSourceUDPMAC,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getSourceUDPMAC2",
                       (Result<MacAddr> (Detector::*)(Positions) const) &
                           Detector::getSourceUDPMAC2,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSourceUDPMAC2",
        (void (Detector::*)(const MacAddr,
                            Positions))&Detector::setSourceUDPMAC2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPList",
                       (Result<UdpDestination> (Detector::*)(const uint32_t,
                                                             Positions) const) &
                           Detector::getDestinationUDPList,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPList",
        (void (Detector::*)(const UdpDestination,
                            const int))&Detector::setDestinationUDPList,
        py::arg(), py::arg());
    CppDetectorApi.def("getNumberofUDPDestinations",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberofUDPDestinations,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "clearUDPDestinations",
        (void (Detector::*)(Positions))&Detector::clearUDPDestinations,
        py::arg() = Positions{});
    CppDetectorApi.def("getFirstUDPDestination",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getFirstUDPDestination,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFirstUDPDestination",
        (void (Detector::*)(const int,
                            Positions))&Detector::setFirstUDPDestination,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPIP",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPIP,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPIP",
        (void (Detector::*)(const IpAddr,
                            Positions))&Detector::setDestinationUDPIP,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPIP2",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPIP2,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPIP2",
        (void (Detector::*)(const IpAddr,
                            Positions))&Detector::setDestinationUDPIP2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPMAC",
                       (Result<MacAddr> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPMAC,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPMAC",
        (void (Detector::*)(const MacAddr,
                            Positions))&Detector::setDestinationUDPMAC,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPMAC2",
                       (Result<MacAddr> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPMAC2,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPMAC2",
        (void (Detector::*)(const MacAddr,
                            Positions))&Detector::setDestinationUDPMAC2,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDestinationUDPPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPPort",
        (void (Detector::*)(uint16_t, int))&Detector::setDestinationUDPPort,
        py::arg(), py::arg() = -1);
    CppDetectorApi.def("getDestinationUDPPort2",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getDestinationUDPPort2,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDestinationUDPPort2",
        (void (Detector::*)(uint16_t, int))&Detector::setDestinationUDPPort2,
        py::arg(), py::arg() = -1);
    CppDetectorApi.def(
        "reconfigureUDPDestination",
        (void (Detector::*)(Positions))&Detector::reconfigureUDPDestination,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "validateUDPConfiguration",
        (void (Detector::*)(Positions))&Detector::validateUDPConfiguration,
        py::arg() = Positions{});
    CppDetectorApi.def("printRxConfiguration",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::printRxConfiguration,
                       py::arg() = Positions{});
    CppDetectorApi.def("getTenGiga",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getTenGiga,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTenGiga",
        (void (Detector::*)(bool, Positions))&Detector::setTenGiga, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getTenGigaFlowControl",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getTenGigaFlowControl,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTenGigaFlowControl",
        (void (Detector::*)(bool, Positions))&Detector::setTenGigaFlowControl,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayFrame",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getTransmissionDelayFrame,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransmissionDelayFrame",
                       (void (Detector::*)(
                           int, Positions))&Detector::setTransmissionDelayFrame,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayLeft",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getTransmissionDelayLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTransmissionDelayLeft",
        (void (Detector::*)(int, Positions))&Detector::setTransmissionDelayLeft,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelayRight",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getTransmissionDelayRight,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTransmissionDelayRight",
                       (void (Detector::*)(
                           int, Positions))&Detector::setTransmissionDelayRight,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransmissionDelay",
                       (int (Detector::*)() const) &
                           Detector::getTransmissionDelay);
    CppDetectorApi.def("setTransmissionDelay",
                       (void (Detector::*)(int))&Detector::setTransmissionDelay,
                       py::arg());
    CppDetectorApi.def("getUseReceiverFlag",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getUseReceiverFlag,
                       py::arg() = Positions{});
    CppDetectorApi.def("getRxHostname",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getRxHostname,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxHostname",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::setRxHostname,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxHostname",
        (void (Detector::*)(
            const std::vector<std::string> &))&Detector::setRxHostname,
        py::arg());
    CppDetectorApi.def("getRxPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getRxPort,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxPort",
                       (void (Detector::*)(uint16_t, int))&Detector::setRxPort,
                       py::arg(), py::arg() = -1);
    CppDetectorApi.def("getRxFifoDepth",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxFifoDepth,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxFifoDepth",
        (void (Detector::*)(int, Positions))&Detector::setRxFifoDepth,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxSilentMode",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getRxSilentMode,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxSilentMode",
        (void (Detector::*)(bool, Positions))&Detector::setRxSilentMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxFrameDiscardPolicy",
        (Result<defs::frameDiscardPolicy> (Detector::*)(Positions) const) &
            Detector::getRxFrameDiscardPolicy,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxFrameDiscardPolicy",
        (void (Detector::*)(defs::frameDiscardPolicy,
                            Positions))&Detector::setRxFrameDiscardPolicy,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPartialFramesPadding",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getPartialFramesPadding,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPartialFramesPadding",
        (void (Detector::*)(bool, Positions))&Detector::setPartialFramesPadding,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxUDPSocketBufferSize",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxUDPSocketBufferSize,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxUDPSocketBufferSize",
        (void (Detector::*)(int, Positions))&Detector::setRxUDPSocketBufferSize,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxRealUDPSocketBufferSize",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxRealUDPSocketBufferSize,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxLock",
        (Result<bool> (Detector::*)(Positions))&Detector::getRxLock,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxLock", (void (Detector::*)(bool, Positions))&Detector::setRxLock,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxLastClientIP",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getRxLastClientIP,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxThreadIds",
        (Result<std::array<pid_t, 9>> (Detector::*)(Positions) const) &
            Detector::getRxThreadIds,
        py::arg() = Positions{});
    CppDetectorApi.def("getRxArping",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getRxArping,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxArping",
        (void (Detector::*)(bool, Positions))&Detector::setRxArping, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getRxROI",
                       (std::vector<defs::ROI> (Detector::*)(int) const) &
                           Detector::getRxROI,
                       py::arg() = -1);
    CppDetectorApi.def(
        "setRxROI",
        (void (Detector::*)(const std::vector<defs::ROI> &))&Detector::setRxROI,
        py::arg());
    CppDetectorApi.def("clearRxROI",
                       (void (Detector::*)())&Detector::clearRxROI);
    CppDetectorApi.def(
        "getFileFormat",
        (Result<defs::fileFormat> (Detector::*)(Positions) const) &
            Detector::getFileFormat,
        py::arg() = Positions{});
    CppDetectorApi.def("setFileFormat",
                       (void (Detector::*)(defs::fileFormat,
                                           Positions))&Detector::setFileFormat,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFilePath",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getFilePath,
                       py::arg() = Positions{});
    CppDetectorApi.def("setFilePath",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::setFilePath,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFileNamePrefix",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getFileNamePrefix,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFileNamePrefix",
        (void (Detector::*)(const std::string &,
                            Positions))&Detector::setFileNamePrefix,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAcquisitionIndex",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getAcquisitionIndex,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setAcquisitionIndex",
        (void (Detector::*)(int64_t, Positions))&Detector::setAcquisitionIndex,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFileWrite",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getFileWrite,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFileWrite",
        (void (Detector::*)(bool, Positions))&Detector::setFileWrite, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getMasterFileWrite", (bool (Detector::*)() const) &
                                                 Detector::getMasterFileWrite);
    CppDetectorApi.def("setMasterFileWrite",
                       (void (Detector::*)(bool))&Detector::setMasterFileWrite,
                       py::arg());
    CppDetectorApi.def("getFileOverWrite",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getFileOverWrite,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFileOverWrite",
        (void (Detector::*)(bool, Positions))&Detector::setFileOverWrite,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getFramesPerFile",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getFramesPerFile,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setFramesPerFile",
        (void (Detector::*)(int, Positions))&Detector::setFramesPerFile,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqDataStream",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getRxZmqDataStream,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxZmqDataStream",
        (void (Detector::*)(bool, Positions))&Detector::setRxZmqDataStream,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqFrequency",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxZmqFrequency,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxZmqFrequency",
        (void (Detector::*)(int, Positions))&Detector::setRxZmqFrequency,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqTimer",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxZmqTimer,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxZmqTimer",
        (void (Detector::*)(int, Positions))&Detector::setRxZmqTimer, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqStartingFrame",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxZmqStartingFrame,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxZmqStartingFrame",
        (void (Detector::*)(int, Positions))&Detector::setRxZmqStartingFrame,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxZmqPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getRxZmqPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxZmqPort",
        (void (Detector::*)(uint16_t, int))&Detector::setRxZmqPort, py::arg(),
        py::arg() = -1);
    CppDetectorApi.def("getClientZmqPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getClientZmqPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setClientZmqPort",
        (void (Detector::*)(uint16_t, int))&Detector::setClientZmqPort,
        py::arg(), py::arg() = -1);
    CppDetectorApi.def("getClientZmqIp",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getClientZmqIp,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setClientZmqIp",
        (void (Detector::*)(const IpAddr, Positions))&Detector::setClientZmqIp,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getClientZmqHwm",
                       (int (Detector::*)() const) & Detector::getClientZmqHwm);
    CppDetectorApi.def(
        "setClientZmqHwm",
        (void (Detector::*)(const int))&Detector::setClientZmqHwm, py::arg());
    CppDetectorApi.def("getRxZmqHwm",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxZmqHwm,
                       py::arg() = Positions{});
    CppDetectorApi.def("setRxZmqHwm",
                       (void (Detector::*)(const int))&Detector::setRxZmqHwm,
                       py::arg());
    CppDetectorApi.def("getSubExptime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getSubExptime,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSubExptime",
        (void (Detector::*)(ns, Positions))&Detector::setSubExptime, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getSubDeadTime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getSubDeadTime,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setSubDeadTime",
        (void (Detector::*)(ns, Positions))&Detector::setSubDeadTime, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getOverFlowMode",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getOverFlowMode,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setOverFlowMode",
        (void (Detector::*)(bool, Positions))&Detector::setOverFlowMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRateCorrection",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getRateCorrection,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDefaultRateCorrection",
        (void (Detector::*)(Positions))&Detector::setDefaultRateCorrection,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setRateCorrection",
        (void (Detector::*)(ns, Positions))&Detector::setRateCorrection,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInterruptSubframe",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getInterruptSubframe,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setInterruptSubframe",
        (void (Detector::*)(const bool,
                            Positions))&Detector::setInterruptSubframe,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getMeasuredPeriod",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getMeasuredPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("getMeasuredSubFramePeriod",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getMeasuredSubFramePeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def("getActive",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getActive,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setActive",
        (void (Detector::*)(const bool, Positions))&Detector::setActive,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPartialReset",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getPartialReset,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPartialReset",
        (void (Detector::*)(bool, Positions))&Detector::setPartialReset,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "pulsePixel",
        (void (Detector::*)(int, defs::xy, Positions))&Detector::pulsePixel,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("pulsePixelNMove",
                       (void (Detector::*)(
                           int, defs::xy, Positions))&Detector::pulsePixelNMove,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("pulseChip",
                       (void (Detector::*)(int, Positions))&Detector::pulseChip,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getQuad",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getQuad,
                       py::arg() = Positions{});
    CppDetectorApi.def("setQuad",
                       (void (Detector::*)(const bool))&Detector::setQuad,
                       py::arg());
    CppDetectorApi.def("getDataStream",
                       (Result<bool> (Detector::*)(const defs::portPosition,
                                                   Positions) const) &
                           Detector::getDataStream,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setDataStream",
                       (void (Detector::*)(const defs::portPosition, const bool,
                                           Positions))&Detector::setDataStream,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTop",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getTop,
                       py::arg() = Positions{});
    CppDetectorApi.def("setTop",
                       (void (Detector::*)(bool, Positions))&Detector::setTop,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getChipVersion",
                       (Result<double> (Detector::*)(Positions) const) &
                           Detector::getChipVersion,
                       py::arg() = Positions{});
    CppDetectorApi.def("getThresholdTemperature",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getThresholdTemperature,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setThresholdTemperature",
        (void (Detector::*)(int, Positions))&Detector::setThresholdTemperature,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureControl",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getTemperatureControl,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTemperatureControl",
        (void (Detector::*)(bool, Positions))&Detector::setTemperatureControl,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTemperatureEvent",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getTemperatureEvent,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "resetTemperatureEvent",
        (void (Detector::*)(Positions))&Detector::resetTemperatureEvent,
        py::arg() = Positions{});
    CppDetectorApi.def("getAutoComparatorDisable",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getAutoComparatorDisable,
                       py::arg() = Positions{});
    CppDetectorApi.def("setAutoComparatorDisable",
                       (void (Detector::*)(
                           bool, Positions))&Detector::setAutoComparatorDisable,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getComparatorDisableTime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getComparatorDisableTime,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setComparatorDisableTime",
        (void (Detector::*)(ns, Positions))&Detector::setComparatorDisableTime,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfAdditionalStorageCells",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfAdditionalStorageCells,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfAdditionalStorageCells",
        (void (Detector::*)(int))&Detector::setNumberOfAdditionalStorageCells,
        py::arg());
    CppDetectorApi.def("getStorageCellStart",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getStorageCellStart,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setStorageCellStart",
        (void (Detector::*)(int, Positions))&Detector::setStorageCellStart,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getStorageCellDelay",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getStorageCellDelay,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setStorageCellDelay",
        (void (Detector::*)(ns, Positions))&Detector::setStorageCellDelay,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getGainModeList",
                       (std::vector<defs::gainMode> (Detector::*)() const) &
                           Detector::getGainModeList);
    CppDetectorApi.def("getGainMode",
                       (Result<defs::gainMode> (Detector::*)(Positions) const) &
                           Detector::getGainMode,
                       py::arg() = Positions{});
    CppDetectorApi.def("setGainMode",
                       (void (Detector::*)(const defs::gainMode,
                                           Positions))&Detector::setGainMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFilterCells",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfFilterCells,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfFilterCells",
        (void (Detector::*)(int, Positions))&Detector::setNumberOfFilterCells,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPedestalMode",
        (Result<defs::pedestalParameters> (Detector::*)(Positions) const) &
            Detector::getPedestalMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setPedestalMode",
        (void (Detector::*)(const defs::pedestalParameters,
                            Positions))&Detector::setPedestalMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getTimingInfoDecoder",
        (Result<defs::timingInfoDecoder> (Detector::*)(Positions) const) &
            Detector::getTimingInfoDecoder,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setTimingInfoDecoder",
        (void (Detector::*)(defs::timingInfoDecoder,
                            Positions))&Detector::setTimingInfoDecoder,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getCollectionMode",
        (Result<defs::collectionMode> (Detector::*)(Positions) const) &
            Detector::getCollectionMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setCollectionMode",
        (void (Detector::*)(defs::collectionMode,
                            Positions))&Detector::setCollectionMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfBursts",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfBursts,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfBursts",
        (void (Detector::*)(int64_t))&Detector::setNumberOfBursts, py::arg());
    CppDetectorApi.def("getBurstPeriod",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getBurstPeriod,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setBurstPeriod",
        (void (Detector::*)(ns, Positions))&Detector::setBurstPeriod, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfBurstsLeft",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfBurstsLeft,
                       py::arg() = Positions{});
    CppDetectorApi.def("getInjectChannel",
                       (Result<std::array<int, 2>> (Detector::*)(
                           Positions))&Detector::getInjectChannel,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setInjectChannel",
        (void (Detector::*)(const int, const int,
                            Positions))&Detector::setInjectChannel,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getVetoPhoton",
                       (void (Detector::*)(const int, const std::string &,
                                           Positions))&Detector::getVetoPhoton,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setVetoPhoton",
                       (void (Detector::*)(const int, const int, const int,
                                           const std::string &,
                                           Positions))&Detector::setVetoPhoton,
                       py::arg(), py::arg(), py::arg(), py::arg(),
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoReference",
        (void (Detector::*)(const int, const int,
                            Positions))&Detector::setVetoReference,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setVetoFile",
                       (void (Detector::*)(const int, const std::string &,
                                           Positions))&Detector::setVetoFile,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getBurstMode",
        (Result<defs::burstMode> (Detector::*)(Positions) const) &
            Detector::getBurstMode,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setBurstMode",
        (void (Detector::*)(defs::burstMode, Positions))&Detector::setBurstMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getCDSGain",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getCDSGain,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setCDSGain",
        (void (Detector::*)(bool, Positions))&Detector::setCDSGain, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getTimingSource",
        (Result<defs::timingSourceType> (Detector::*)(Positions) const) &
            Detector::getTimingSource,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setTimingSource",
        (void (Detector::*)(defs::timingSourceType,
                            Positions))&Detector::setTimingSource,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getVeto",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getVeto,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setVeto",
        (void (Detector::*)(const bool, Positions))&Detector::setVeto,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getVetoStream",
        (Result<defs::streamingInterface> (Detector::*)(Positions) const) &
            Detector::getVetoStream,
        py::arg() = Positions{});
    CppDetectorApi.def("setVetoStream",
                       (void (Detector::*)(const defs::streamingInterface,
                                           Positions))&Detector::setVetoStream,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getVetoAlgorithm",
                       (Result<defs::vetoAlgorithm> (Detector::*)(
                           const defs::streamingInterface, Positions) const) &
                           Detector::getVetoAlgorithm,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setVetoAlgorithm",
        (void (Detector::*)(const defs::vetoAlgorithm,
                            const defs::streamingInterface,
                            Positions))&Detector::setVetoAlgorithm,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getADCConfiguration",
        (Result<int> (Detector::*)(const int, const int, Positions) const) &
            Detector::getADCConfiguration,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCConfiguration",
        (void (Detector::*)(const int, const int, const int,
                            Positions))&Detector::setADCConfiguration,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getCounterMask",
                       (Result<uint32_t> (Detector::*)(Positions) const) &
                           Detector::getCounterMask,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setCounterMask",
        (void (Detector::*)(uint32_t, Positions))&Detector::setCounterMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfGates",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfGates,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfGates",
        (void (Detector::*)(int, Positions))&Detector::setNumberOfGates,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExptime",
                       (Result<ns> (Detector::*)(int, Positions) const) &
                           Detector::getExptime,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setExptime",
        (void (Detector::*)(int, ns, Positions))&Detector::setExptime,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getExptimeForAllGates",
        (Result<std::array<ns, 3>> (Detector::*)(Positions) const) &
            Detector::getExptimeForAllGates,
        py::arg() = Positions{});
    CppDetectorApi.def("getGateDelay",
                       (Result<ns> (Detector::*)(int, Positions) const) &
                           Detector::getGateDelay,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setGateDelay",
        (void (Detector::*)(int, ns, Positions))&Detector::setGateDelay,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getGateDelayForAllGates",
        (Result<std::array<ns, 3>> (Detector::*)(Positions) const) &
            Detector::getGateDelayForAllGates,
        py::arg() = Positions{});
    CppDetectorApi.def("getChipStatusRegister",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getChipStatusRegister,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setGainCaps",
        (void (Detector::*)(int, Positions))&Detector::setGainCaps, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getGainCaps",
        (Result<int> (Detector::*)(Positions))&Detector::getGainCaps,
        py::arg() = Positions{});
    CppDetectorApi.def("getPolarity",
                       (Result<defs::polarity> (Detector::*)(Positions) const) &
                           Detector::getPolarity,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPolarity",
        (void (Detector::*)(defs::polarity, Positions))&Detector::setPolarity,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInterpolation",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getInterpolation,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setInterpolation",
        (void (Detector::*)(bool, Positions))&Detector::setInterpolation,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPumpProbe",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getPumpProbe,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPumpProbe",
        (void (Detector::*)(bool, Positions))&Detector::setPumpProbe, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getAnalogPulsing",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getAnalogPulsing,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setAnalogPulsing",
        (void (Detector::*)(bool, Positions))&Detector::setAnalogPulsing,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDigitalPulsing",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getDigitalPulsing,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDigitalPulsing",
        (void (Detector::*)(bool, Positions))&Detector::setDigitalPulsing,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfAnalogSamples",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfAnalogSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfAnalogSamples",
        (void (Detector::*)(int, Positions))&Detector::setNumberOfAnalogSamples,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCClock",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getADCClock,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCClock",
        (void (Detector::*)(int, Positions))&Detector::setADCClock, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getRUNClock",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRUNClock,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRUNClock",
        (void (Detector::*)(int, Positions))&Detector::setRUNClock, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def("getSYNCClock",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getSYNCClock,
                       py::arg() = Positions{});
    CppDetectorApi.def("getPowerList",
                       (std::vector<defs::dacIndex> (Detector::*)() const) &
                           Detector::getPowerList);
    CppDetectorApi.def("getSlowADCList",
                       (std::vector<defs::dacIndex> (Detector::*)() const) &
                           Detector::getSlowADCList);
    CppDetectorApi.def(
        "getPower",
        (Result<int> (Detector::*)(defs::dacIndex, Positions) const) &
            Detector::getPower,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPower",
        (void (Detector::*)(defs::dacIndex, int, Positions))&Detector::setPower,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCVpp",
                       (Result<int> (Detector::*)(bool, Positions) const) &
                           Detector::getADCVpp,
                       py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCVpp",
        (void (Detector::*)(int, bool, Positions))&Detector::setADCVpp,
        py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def("getADCEnableMask",
                       (Result<uint32_t> (Detector::*)(Positions) const) &
                           Detector::getADCEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCEnableMask",
        (void (Detector::*)(uint32_t, Positions))&Detector::setADCEnableMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTenGigaADCEnableMask",
                       (Result<uint32_t> (Detector::*)(Positions) const) &
                           Detector::getTenGigaADCEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTenGigaADCEnableMask",
        (void (Detector::*)(uint32_t,
                            Positions))&Detector::setTenGigaADCEnableMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getTransceiverEnableMask",
                       (Result<uint32_t> (Detector::*)(Positions) const) &
                           Detector::getTransceiverEnableMask,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setTransceiverEnableMask",
        (void (Detector::*)(uint32_t,
                            Positions))&Detector::setTransceiverEnableMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfDigitalSamples",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfDigitalSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def("setNumberOfDigitalSamples",
                       (void (Detector::*)(
                           int, Positions))&Detector::setNumberOfDigitalSamples,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfTransceiverSamples",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getNumberOfTransceiverSamples,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setNumberOfTransceiverSamples",
        (void (Detector::*)(int,
                            Positions))&Detector::setNumberOfTransceiverSamples,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getReadoutMode",
        (Result<defs::readoutMode> (Detector::*)(Positions) const) &
            Detector::getReadoutMode,
        py::arg() = Positions{});
    CppDetectorApi.def("setReadoutMode",
                       (void (Detector::*)(defs::readoutMode,
                                           Positions))&Detector::setReadoutMode,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDBITClock",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getDBITClock,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDBITClock",
        (void (Detector::*)(int, Positions))&Detector::setDBITClock, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def(
        "getMeasuredPower",
        (Result<int> (Detector::*)(defs::dacIndex, Positions) const) &
            Detector::getMeasuredPower,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getMeasuredCurrent",
        (Result<int> (Detector::*)(defs::dacIndex, Positions) const) &
            Detector::getMeasuredCurrent,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getSlowADC",
        (Result<int> (Detector::*)(defs::dacIndex, Positions) const) &
            Detector::getSlowADC,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExternalSamplingSource",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getExternalSamplingSource,
                       py::arg() = Positions{});
    CppDetectorApi.def("setExternalSamplingSource",
                       (void (Detector::*)(
                           int, Positions))&Detector::setExternalSamplingSource,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getExternalSampling",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getExternalSampling,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setExternalSampling",
        (void (Detector::*)(bool, Positions))&Detector::setExternalSampling,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getRxDbitList",
        (Result<std::vector<int>> (Detector::*)(Positions) const) &
            Detector::getRxDbitList,
        py::arg() = Positions{});
    CppDetectorApi.def("setRxDbitList",
                       (void (Detector::*)(const std::vector<int> &,
                                           Positions))&Detector::setRxDbitList,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxDbitOffset",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getRxDbitOffset,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxDbitOffset",
        (void (Detector::*)(int, Positions))&Detector::setRxDbitOffset,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getRxDbitReorder",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getRxDbitReorder,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setRxDbitReorder",
        (void (Detector::*)(bool, Positions))&Detector::setRxDbitReorder,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setDigitalIODelay",
        (void (Detector::*)(uint64_t, int,
                            Positions))&Detector::setDigitalIODelay,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getLEDEnable",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getLEDEnable,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setLEDEnable",
        (void (Detector::*)(bool, Positions))&Detector::setLEDEnable, py::arg(),
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setDacNames",
        (void (Detector::*)(
            const std::vector<std::string>))&Detector::setDacNames,
        py::arg());
    CppDetectorApi.def("getDacNames",
                       (std::vector<std::string> (Detector::*)() const) &
                           Detector::getDacNames);
    CppDetectorApi.def(
        "getDacIndex",
        (defs::dacIndex (Detector::*)(const std::string &) const) &
            Detector::getDacIndex,
        py::arg());
    CppDetectorApi.def(
        "setDacName",
        (void (Detector::*)(const defs::dacIndex,
                            const std::string &))&Detector::setDacName,
        py::arg(), py::arg());
    CppDetectorApi.def("getDacName",
                       (std::string (Detector::*)(const defs::dacIndex) const) &
                           Detector::getDacName,
                       py::arg());
    CppDetectorApi.def(
        "setAdcNames",
        (void (Detector::*)(
            const std::vector<std::string>))&Detector::setAdcNames,
        py::arg());
    CppDetectorApi.def("getAdcNames",
                       (std::vector<std::string> (Detector::*)() const) &
                           Detector::getAdcNames);
    CppDetectorApi.def("getAdcIndex",
                       (int (Detector::*)(const std::string &) const) &
                           Detector::getAdcIndex,
                       py::arg());
    CppDetectorApi.def(
        "setAdcName",
        (void (Detector::*)(const int,
                            const std::string &))&Detector::setAdcName,
        py::arg(), py::arg());
    CppDetectorApi.def("getAdcName",
                       (std::string (Detector::*)(const int) const) &
                           Detector::getAdcName,
                       py::arg());
    CppDetectorApi.def(
        "setSignalNames",
        (void (Detector::*)(
            const std::vector<std::string>))&Detector::setSignalNames,
        py::arg());
    CppDetectorApi.def("getSignalNames",
                       (std::vector<std::string> (Detector::*)() const) &
                           Detector::getSignalNames);
    CppDetectorApi.def("getSignalIndex",
                       (int (Detector::*)(const std::string &) const) &
                           Detector::getSignalIndex,
                       py::arg());
    CppDetectorApi.def(
        "setSignalName",
        (void (Detector::*)(const int,
                            const std::string &))&Detector::setSignalName,
        py::arg(), py::arg());
    CppDetectorApi.def("getSignalName",
                       (std::string (Detector::*)(const int) const) &
                           Detector::getSignalName,
                       py::arg());
    CppDetectorApi.def(
        "setPowerNames",
        (void (Detector::*)(
            const std::vector<std::string>))&Detector::setPowerNames,
        py::arg());
    CppDetectorApi.def("getPowerNames",
                       (std::vector<std::string> (Detector::*)() const) &
                           Detector::getPowerNames);
    CppDetectorApi.def(
        "getPowerIndex",
        (defs::dacIndex (Detector::*)(const std::string &) const) &
            Detector::getPowerIndex,
        py::arg());
    CppDetectorApi.def(
        "setPowerName",
        (void (Detector::*)(const defs::dacIndex,
                            const std::string &))&Detector::setPowerName,
        py::arg(), py::arg());
    CppDetectorApi.def("getPowerName",
                       (std::string (Detector::*)(const defs::dacIndex) const) &
                           Detector::getPowerName,
                       py::arg());
    CppDetectorApi.def(
        "setSlowADCNames",
        (void (Detector::*)(
            const std::vector<std::string>))&Detector::setSlowADCNames,
        py::arg());
    CppDetectorApi.def("getSlowADCNames",
                       (std::vector<std::string> (Detector::*)() const) &
                           Detector::getSlowADCNames);
    CppDetectorApi.def(
        "getSlowADCIndex",
        (defs::dacIndex (Detector::*)(const std::string &) const) &
            Detector::getSlowADCIndex,
        py::arg());
    CppDetectorApi.def(
        "setSlowADCName",
        (void (Detector::*)(const defs::dacIndex,
                            const std::string &))&Detector::setSlowADCName,
        py::arg(), py::arg());
    CppDetectorApi.def("getSlowADCName",
                       (std::string (Detector::*)(const defs::dacIndex) const) &
                           Detector::getSlowADCName,
                       py::arg());
    CppDetectorApi.def(
        "configureTransceiver",
        (void (Detector::*)(Positions))&Detector::configureTransceiver,
        py::arg() = Positions{});
    CppDetectorApi.def("getPatterFileName",
                       (Result<std::string> (Detector::*)(Positions) const) &
                           Detector::getPatterFileName,
                       py::arg() = Positions{});
    CppDetectorApi.def("setPattern",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::setPattern,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPattern",
        (void (Detector::*)(const Pattern &, Positions))&Detector::setPattern,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "savePattern",
        (void (Detector::*)(const std::string &))&Detector::savePattern,
        py::arg());
    CppDetectorApi.def(
        "loadDefaultPattern",
        (void (Detector::*)(Positions))&Detector::loadDefaultPattern,
        py::arg() = Positions{});
    CppDetectorApi.def("getPatternIOControl",
                       (Result<uint64_t> (Detector::*)(Positions) const) &
                           Detector::getPatternIOControl,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternIOControl",
        (void (Detector::*)(uint64_t, Positions))&Detector::setPatternIOControl,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWord",
                       (Result<uint64_t> (Detector::*)(
                           int, Positions))&Detector::getPatternWord,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternWord",
        (void (Detector::*)(int, uint64_t, Positions))&Detector::setPatternWord,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPatternLoopAddresses",
        (Result<std::array<int, 2>> (Detector::*)(int, Positions) const) &
            Detector::getPatternLoopAddresses,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternLoopAddresses",
        (void (Detector::*)(int, int, int,
                            Positions))&Detector::setPatternLoopAddresses,
        py::arg(), py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternLoopCycles",
                       (Result<int> (Detector::*)(int, Positions) const) &
                           Detector::getPatternLoopCycles,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("setPatternLoopCycles",
                       (void (Detector::*)(
                           int, int, Positions))&Detector::setPatternLoopCycles,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWaitAddr",
                       (Result<int> (Detector::*)(int, Positions) const) &
                           Detector::getPatternWaitAddr,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternWaitAddr",
        (void (Detector::*)(int, int, Positions))&Detector::setPatternWaitAddr,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWaitClocks",
                       (Result<uint64_t> (Detector::*)(int, Positions) const) &
                           Detector::getPatternWaitClocks,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternWaitClocks",
        (void (Detector::*)(int, uint64_t,
                            Positions))&Detector::setPatternWaitClocks,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternWaitInterval",
                       (Result<ns> (Detector::*)(int, Positions) const) &
                           Detector::getPatternWaitInterval,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternWaitInterval",
        (void (Detector::*)(int, ns,
                            Positions))&Detector::setPatternWaitInterval,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "getPatternMask",
        (Result<uint64_t> (Detector::*)(Positions))&Detector::getPatternMask,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternMask",
        (void (Detector::*)(uint64_t, Positions))&Detector::setPatternMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getPatternBitMask",
                       (Result<uint64_t> (Detector::*)(Positions) const) &
                           Detector::getPatternBitMask,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setPatternBitMask",
        (void (Detector::*)(uint64_t, Positions))&Detector::setPatternBitMask,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("startPattern",
                       (void (Detector::*)(Positions))&Detector::startPattern,
                       py::arg() = Positions{});
    CppDetectorApi.def("getAdditionalJsonHeader",
                       (Result<std::map<std::string, std::string>> (
                           Detector::*)(Positions) const) &
                           Detector::getAdditionalJsonHeader,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setAdditionalJsonHeader",
        (void (Detector::*)(const std::map<std::string, std::string> &,
                            Positions))&Detector::setAdditionalJsonHeader,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getAdditionalJsonParameter",
                       (Result<std::string> (Detector::*)(const std::string &,
                                                          Positions) const) &
                           Detector::getAdditionalJsonParameter,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "setAdditionalJsonParameter",
        (void (Detector::*)(const std::string &, const std::string &,
                            Positions))&Detector::setAdditionalJsonParameter,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getADCPipeline",
                       (Result<int> (Detector::*)(Positions) const) &
                           Detector::getADCPipeline,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCPipeline",
        (void (Detector::*)(int, Positions))&Detector::setADCPipeline,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("programFPGA",
                       (void (Detector::*)(const std::string &, const bool,
                                           Positions))&Detector::programFPGA,
                       py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("resetFPGA",
                       (void (Detector::*)(Positions))&Detector::resetFPGA,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "updateDetectorServer",
        (void (Detector::*)(const std::string &,
                            Positions))&Detector::updateDetectorServer,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("updateKernel",
                       (void (Detector::*)(const std::string &,
                                           Positions))&Detector::updateKernel,
                       py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "rebootController",
        (void (Detector::*)(Positions))&Detector::rebootController,
        py::arg() = Positions{});
    CppDetectorApi.def(
        "updateFirmwareAndServer",
        (void (Detector::*)(const std::string &, const std::string &,
                            Positions))&Detector::updateFirmwareAndServer,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getUpdateMode",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getUpdateMode,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setUpdateMode",
        (void (Detector::*)(const bool, Positions))&Detector::setUpdateMode,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "readRegister",
        (Result<uint32_t> (Detector::*)(uint32_t, Positions) const) &
            Detector::readRegister,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("writeRegister",
                       (void (Detector::*)(uint32_t, uint32_t, bool,
                                           Positions))&Detector::writeRegister,
                       py::arg(), py::arg(), py::arg() = false,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setBit",
        (void (Detector::*)(uint32_t, int, bool, Positions))&Detector::setBit,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "clearBit",
        (void (Detector::*)(uint32_t, int, bool, Positions))&Detector::clearBit,
        py::arg(), py::arg(), py::arg() = false, py::arg() = Positions{});
    CppDetectorApi.def(
        "getBit",
        (Result<int> (Detector::*)(uint32_t, int, Positions))&Detector::getBit,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def(
        "executeFirmwareTest",
        (void (Detector::*)(Positions))&Detector::executeFirmwareTest,
        py::arg() = Positions{});
    CppDetectorApi.def("executeBusTest",
                       (void (Detector::*)(Positions))&Detector::executeBusTest,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "writeAdcRegister",
        (void (Detector::*)(uint32_t, uint32_t,
                            Positions))&Detector::writeAdcRegister,
        py::arg(), py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getInitialChecks", (bool (Detector::*)() const) &
                                               Detector::getInitialChecks);
    CppDetectorApi.def(
        "setInitialChecks",
        (void (Detector::*)(const bool))&Detector::setInitialChecks, py::arg());
    CppDetectorApi.def("getADCInvert",
                       (Result<uint32_t> (Detector::*)(Positions) const) &
                           Detector::getADCInvert,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setADCInvert",
        (void (Detector::*)(uint32_t, Positions))&Detector::setADCInvert,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getControlPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getControlPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setControlPort",
        (void (Detector::*)(uint16_t, Positions))&Detector::setControlPort,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getStopPort",
                       (Result<uint16_t> (Detector::*)(Positions) const) &
                           Detector::getStopPort,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setStopPort",
        (void (Detector::*)(uint16_t, Positions))&Detector::setStopPort,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getDetectorLock",
                       (Result<bool> (Detector::*)(Positions) const) &
                           Detector::getDetectorLock,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "setDetectorLock",
        (void (Detector::*)(bool, Positions))&Detector::setDetectorLock,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getLastClientIP",
                       (Result<IpAddr> (Detector::*)(Positions) const) &
                           Detector::getLastClientIP,
                       py::arg() = Positions{});
    CppDetectorApi.def(
        "executeCommand",
        (Result<std::string> (Detector::*)(const std::string &,
                                           Positions))&Detector::executeCommand,
        py::arg(), py::arg() = Positions{});
    CppDetectorApi.def("getNumberOfFramesFromStart",
                       (Result<int64_t> (Detector::*)(Positions) const) &
                           Detector::getNumberOfFramesFromStart,
                       py::arg() = Positions{});
    CppDetectorApi.def("getActualTime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getActualTime,
                       py::arg() = Positions{});
    CppDetectorApi.def("getMeasurementTime",
                       (Result<ns> (Detector::*)(Positions) const) &
                           Detector::getMeasurementTime,
                       py::arg() = Positions{});
    ;
}

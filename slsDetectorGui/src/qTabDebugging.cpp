// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabDebugging.h"
#include "qDefs.h"
#include "sls/ToString.h"

namespace sls {

qTabDebugging::qTabDebugging(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Debugging ready";
}

qTabDebugging::~qTabDebugging() {}

void qTabDebugging::SetupWidgetWindow() {
    if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
        groupTest->setEnabled(false);
    } else {
        EnableTest();
    }
    PopulateDetectors();
    Initialization();
    Refresh();
}

void qTabDebugging::Initialization() {
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetInfo()));
    connect(chkDetectorFirmware, SIGNAL(toggled(bool)), this,
            SLOT(EnableTest()));
    connect(chkDetectorBus, SIGNAL(toggled(bool)), this, SLOT(EnableTest()));
    if (groupTest->isEnabled()) {
        connect(btnTest, SIGNAL(clicked()), this, SLOT(TestDetector()));
    }
}

void qTabDebugging::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    try {
        comboDetector->clear();
        comboDetector->addItem("All");
        auto res = det->getHostname();
        if (det->size() > 1) {
            for (auto &it : res) {
                comboDetector->addItem(QString(it.c_str()));
            }
        }
        comboDetector->setCurrentIndex(0);
    }
    CATCH_DISPLAY("Could not populate readouts for debugging",
                  "qTabDebugging::PopulateDetectors")
}

void qTabDebugging::GetFirmwareVersion() {
    LOG(logDEBUG) << "Firmware Version";
    try {
        auto retval =
            det->getFirmwareVersion({comboDetector->currentIndex() - 1})
                .squash(-1);
        std::string s = "inconsistent";
        if (retval != -1) {
            if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
                s = ToString(retval);
            } else {
                s = ToStringHex(retval);
            }
        }
        dispFirmwareVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not get firmware version.",
                  "qTabDebugging::GetFirmwareVersion")
}

void qTabDebugging::GetServerSoftwareVersion() {
    LOG(logDEBUG) << "Server Software Version";
    try {
        std::string s =
            det->getDetectorServerVersion({comboDetector->currentIndex() - 1})
                .squash("inconsistent");
        dispSoftwareVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not get on-board software version.",
                  "qTabDebugging::GetServerSoftwareVersion")
}

void qTabDebugging::GetReceiverVersion() {
    LOG(logDEBUG) << "Server Receiver Version";
    try {
        std::string s =
            det->getReceiverVersion({comboDetector->currentIndex() - 1})
                .squash("inconsistent");
        dispReceiverVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not receiver version.",
                  "qTabDebugging::GetReceiverVersion")
}

void qTabDebugging::GetDetectorStatus() {
    LOG(logDEBUG) << "Getting Status";

    try {
        std::string s =
            ToString(det->getDetectorStatus({comboDetector->currentIndex() - 1})
                         .squash(defs::runStatus::ERROR));
        lblStatus->setText(QString(s.c_str()).toUpper());
    }
    CATCH_DISPLAY("Could not get detector status.",
                  "qTabDebugging::GetDetectorStatus")
}

void qTabDebugging::GetInfo() {
    LOG(logDEBUG) << "Getting Readout Info";
    GetFirmwareVersion();
    GetServerSoftwareVersion();
    GetReceiverVersion();
    GetDetectorStatus();
}

void qTabDebugging::EnableTest() {
    btnTest->setEnabled(chkDetectorFirmware->isChecked() ||
                        chkDetectorBus->isChecked());
    lblBusTestOk->hide();
    lblBusTestFail->hide();
    lblFwTestOk->hide();
    lblFwTestFail->hide();
}

void qTabDebugging::TestDetector() {
    LOG(logINFO) << "Testing Readout";

    // hide results if clicking button again
    EnableTest();

    // detector firmware
    if (chkDetectorFirmware->isChecked()) {
        try {
            det->executeFirmwareTest({comboDetector->currentIndex() - 1});
            LOG(logINFO) << "Detector Firmware Test: Pass";
            lblFwTestOk->show();
        } catch (std::exception &e) {
            LOG(logWARNING)
                << "Detector Firmware Test: Fail (" << e.what() << ")";
            lblFwTestFail->show();
        }
    }

    // detector CPU-FPGA bus
    if (chkDetectorBus->isChecked()) {
        try {
            det->executeBusTest({comboDetector->currentIndex() - 1});
            LOG(logINFO) << "Detector Bus Test: Pass";
            lblBusTestOk->show();
        } catch (std::exception &e) {
            LOG(logWARNING) << "Detector Bus Test: Fail (" << e.what() << ")";
            lblBusTestFail->show();
        }
    }
}

void qTabDebugging::Refresh() { GetInfo(); }

} // namespace sls

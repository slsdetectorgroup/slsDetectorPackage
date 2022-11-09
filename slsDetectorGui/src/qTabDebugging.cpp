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
        chkDetectorFirmware->setEnabled(false);
        chkDetectorBus->setEnabled(false);
        btnTest->setEnabled(false);
    }
    PopulateDetectors();
    Initialization();
    Refresh();
}

void qTabDebugging::Initialization() {
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetInfo()));
    if (btnTest->isEnabled()) {
        connect(btnTest, SIGNAL(clicked()), this, SLOT(TestDetector()));
    }
}

void qTabDebugging::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    comboDetector->clear();
    auto res = det->getHostname();
    for (auto &it : res) {
        comboDetector->addItem(QString(it.c_str()));
    }
}

void qTabDebugging::GetFirmwareVersion() {
    LOG(logDEBUG) << "Firmware Version";
/*std::string("0x") +
                          std::to_string((unsigned long)det->getFirmwareVersion(
                              {comboDetector->currentIndex()})[0]);*/
    try {
        auto retval = det->getFirmwareVersion({comboDetector->currentIndex()})[0];
        std::string s;
        if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
            s = ToString(retval);
        } else {
            s = ToStringHex(retval);
        }
        dispFirmwareVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not get firmware version.",
                  "qTabDebugging::GetFirmwareVersion")
}

void qTabDebugging::GetServerSoftwareVersion() {
    LOG(logDEBUG) << "Server Software Version";
    try {
        std::string s =  det->getDetectorServerVersion({comboDetector->currentIndex()})[0];
        dispSoftwareVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not get on-board software version.",
                  "qTabDebugging::GetServerSoftwareVersion")
}

void qTabDebugging::GetReceiverVersion() {
    LOG(logDEBUG) << "Server Receiver Version";
    try {
        std::string s =  det->getReceiverVersion({comboDetector->currentIndex()})[0];
        dispSoftwareVersion->setText(s.c_str());
    }
    CATCH_DISPLAY("Could not receiver version.",
                  "qTabDebugging::GetReceiverVersion")
}

void qTabDebugging::GetDetectorStatus() {
    LOG(logDEBUG) << "Getting Status";

    try {
        std::string s = ToString(
            det->getDetectorStatus({comboDetector->currentIndex()})[0]);
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


void qTabDebugging::TestDetector() {
    LOG(logINFO) << "Testing Readout";

    try {
        QString moduleName = "Module";
        if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
            moduleName = "Half Module";
        }

        // construct message
        QString message = QString("<nobr>Test Results for %1:</nobr><br><br>")
                              .arg(comboDetector->currentText());

        // detector firmware
        if (chkDetectorFirmware->isChecked()) {
            try {
                det->executeFirmwareTest({comboDetector->currentIndex()});
                message.append(QString("<nobr>%1 Firmware: PASS</nobr><br>")
                                   .arg(moduleName));
                LOG(logINFO) << "Detector Firmware Test: Pass";
            }
            CATCH_DISPLAY("Firmware test failed.",
                          "qTabDebugging::TestDetector")
        }

        // detector CPU-FPGA bus
        if (chkDetectorBus->isChecked()) {
            try {
                det->executeBusTest({comboDetector->currentIndex()});
                message.append(
                    QString("<nobr>%1 Bus: PASS</nobr><br>").arg(moduleName));
                LOG(logINFO) << "Detector Bus Test: Pass";
            }
            CATCH_DISPLAY("Bus test failed.", "qTabDebugging::TestDetector")
        }

        // display message
        qDefs::Message(qDefs::INFORMATION, message.toLatin1().constData(),
                       "qTabDebugging::TestDetector");
    }
    CATCH_DISPLAY("Could not execute digital test.",
                  "qTabDebugging::TestDetector")
}

void qTabDebugging::Refresh() {
    GetInfo();
}

} // namespace sls

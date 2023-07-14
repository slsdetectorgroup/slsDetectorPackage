// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabDataOutput.h"
#include "qDefs.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QString>
#include <string>

#include <unistd.h>

namespace sls {

qTabDataOutput::qTabDataOutput(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector), btnGroupRate(nullptr) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "DataOutput ready";
}

qTabDataOutput::~qTabDataOutput() { delete btnGroupRate; }

void qTabDataOutput::SetupWidgetWindow() {
    // button group for rate
    btnGroupRate = new QButtonGroup(this);
    btnGroupRate->addButton(radioDefaultDeadtime, 0);
    btnGroupRate->addButton(radioCustomDeadtime, 1);

    // enabling according to det type
    switch (det->getDetectorType().squash()) {
    case slsDetectorDefs::EIGER:
        chkTenGiga->setEnabled(true);
        chkRate->setEnabled(true);
        radioDefaultDeadtime->setEnabled(true);
        radioCustomDeadtime->setEnabled(true);
        // flags and speed
        lblClkDivider->setEnabled(true);
        comboClkDivider->setEnabled(true);
        chkParallel->setEnabled(true);
        break;
    case slsDetectorDefs::MYTHEN3:
        chkParallel->setEnabled(true);
        break;
    case slsDetectorDefs::MOENCH:
    case slsDetectorDefs::JUNGFRAU:
        lblClkDivider->setEnabled(true);
        comboClkDivider->setEnabled(true);
        break;
    default:
        break;
    }
    PopulateDetectors();

    Initialization();

    Refresh();
}

void qTabDataOutput::Initialization() {
    // ourdir, fileformat, overwrite enable
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetOutputDir()));
    connect(dispOutputDir, SIGNAL(editingFinished()), this,
            SLOT(SetOutputDir()));
    connect(dispOutputDir, SIGNAL(returnPressed()), this,
            SLOT(ForceSetOutputDir()));
    connect(btnOutputBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutputDir()));
    connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetFileFormat(int)));
    connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
            SLOT(SetOverwriteEnable(bool)));
    if (chkTenGiga->isEnabled()) {
        connect(chkTenGiga, SIGNAL(toggled(bool)), this,
                SLOT(SetTenGigaEnable(bool)));
    }
    // rate
    if (chkRate->isEnabled()) {
        connect(chkRate, SIGNAL(toggled(bool)), this,
                SLOT(EnableRateCorrection()));
        connect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
                SLOT(SetRateCorrection()));
        connect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
                SLOT(SetRateCorrection()));
    }
    // parallel
    if (chkParallel->isEnabled()) {
        connect(chkParallel, SIGNAL(toggled(bool)), this,
                SLOT(SetParallel(bool)));
    }
    // speed
    if (comboClkDivider->isEnabled()) {
        connect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSpeed(int)));
    }
}

void qTabDataOutput::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    comboDetector->clear();
    comboDetector->addItem("All");
    if (det->size() > 1) {
        auto res = det->getHostname();
        for (auto &it : res) {
            comboDetector->addItem(QString(it.c_str()));
        }
    }
}

void qTabDataOutput::EnableBrowse() {
    LOG(logDEBUG) << "Getting browse enable";
    try {
        btnOutputBrowse->setEnabled(false); // exception default
        std::string rxHostname =
            det->getRxHostname({comboDetector->currentIndex() - 1})
                .squash("none");
        if (rxHostname == "none") {
            btnOutputBrowse->setEnabled(false);
        } else if (rxHostname == "localhost") {
            btnOutputBrowse->setEnabled(true);
        } else {
            std::string hostname;
            const size_t len = 15;
            char host[len]{};
            if (gethostname(host, len) == 0) {
                hostname.assign(host);
            }
            // client pc (hostname) same as reciever hostname
            if (hostname == rxHostname) {
                btnOutputBrowse->setEnabled(true);
            } else {
                btnOutputBrowse->setEnabled(false);
            }
        }
    }
    CATCH_DISPLAY("Could not get receiver hostname.",
                  "qTabDataOutput::EnableBrowse")
}

void qTabDataOutput::GetFileWrite() {
    LOG(logDEBUG) << "Getting file write enable";
    try {
        boxFileWriteEnabled->setEnabled(true); // exception default
        auto retval = det->getFileWrite().tsquash(
            "File write is inconsistent for all detectors.");
        boxFileWriteEnabled->setEnabled(retval);
    }
    CATCH_DISPLAY("Could not get file enable.", "qTabDataOutput::GetFileWrite")
}

void qTabDataOutput::GetFileName() {
    LOG(logDEBUG) << "Getting file name";
    try {
        auto retval = det->getFileNamePrefix().tsquash(
            "File name is inconsistent for all detectors.");
        dispFileName->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get file name prefix.",
                  "qTabDataOutput::GetFileName")
}

void qTabDataOutput::GetOutputDir() {
    LOG(logDEBUG) << "Getting file path";
    disconnect(dispOutputDir, SIGNAL(editingFinished()), this,
               SLOT(SetOutputDir()));
    try {
        std::string path =
            det->getFilePath({comboDetector->currentIndex() - 1})
                .tsquash("File path is different for all detectors.");
        dispOutputDir->setText(QString(path.c_str()));
    }
    CATCH_DISPLAY("Could not get file path.", "qTabDataOutput::GetOutputDir")
    connect(dispOutputDir, SIGNAL(editingFinished()), this,
            SLOT(SetOutputDir()));
}

void qTabDataOutput::BrowseOutputDir() {
    LOG(logDEBUG) << "Browsing output directory";
    QString directory = QFileDialog::getExistingDirectory(
        this, tr("Choose Output Directory "), dispOutputDir->text());
    if (!directory.isEmpty()) {
        dispOutputDir->setText(directory);
        ForceSetOutputDir();
    }
}

void qTabDataOutput::SetOutputDir(bool force) {
    // return forces modification (inconsistency from command line)
    if (dispOutputDir->isModified() || force) {
        dispOutputDir->setModified(false);
        QString path = dispOutputDir->text();
        LOG(logDEBUG) << "Setting output directory to "
                      << path.toLatin1().constData();

        // empty
        if (path.isEmpty()) {
            qDefs::Message(qDefs::WARNING,
                           "Invalid Output Path. Must not be empty.",
                           "qTabDataOutput::SetOutputDir");
            LOG(logWARNING) << "Invalid Output Path. Must not be empty.";
            GetOutputDir();
        } else {
            // chop off trailing '/'
            if (path.endsWith('/')) {
                while (path.endsWith('/')) {
                    path.chop(1);
                }
            }
            std::string spath = std::string(path.toLatin1().constData());
            try {
                det->setFilePath(spath, {comboDetector->currentIndex() - 1});
            }
            CATCH_HANDLE("Could not set output file path.",
                         "qTabDataOutput::SetOutputDir", this,
                         &qTabDataOutput::GetOutputDir)
        }
    }
}

void qTabDataOutput::ForceSetOutputDir() { SetOutputDir(true); };

void qTabDataOutput::GetFileFormat() {
    LOG(logDEBUG) << "Getting File Format";
    disconnect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetFileFormat(int)));
    try {
        auto retval = det->getFileFormat().tsquash(
            "File format is inconsistent for all detectors.");
        switch (retval) {
        case slsDetectorDefs::BINARY:
        case slsDetectorDefs::HDF5:
            comboFileFormat->setCurrentIndex(static_cast<int>(retval));
            break;
        default:
            throw RuntimeError(std::string("Unknown file format: ") +
                               std::to_string(static_cast<int>(retval)));
        }
    }
    CATCH_DISPLAY("Could not get file format.", "qTabDataOutput::GetFileFormat")
    connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetFileFormat(int)));
}

void qTabDataOutput::SetFileFormat(int format) {
    LOG(logINFO) << "Setting File Format to "
                 << comboFileFormat->currentText().toLatin1().data();
    try {
        det->setFileFormat(static_cast<slsDetectorDefs::fileFormat>(
            comboFileFormat->currentIndex()));
    }
    CATCH_HANDLE("Could not set file format.", "qTabDataOutput::SetFileFormat",
                 this, &qTabDataOutput::GetFileFormat)
}

void qTabDataOutput::GetFileOverwrite() {
    LOG(logDEBUG) << "Getting File Over Write Enable";
    disconnect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
               SLOT(SetOverwriteEnable(bool)));
    try {
        auto retval = det->getFileOverWrite().tsquash(
            "File over write is inconsistent for all detectors.");
        chkOverwriteEnable->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get file over write enable.",
                  "qTabDataOutput::GetFileOverwrite")

    connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
            SLOT(SetOverwriteEnable(bool)));
}

void qTabDataOutput::SetOverwriteEnable(bool enable) {
    LOG(logINFO) << "Setting File Over Write Enable to " << enable;
    try {
        det->setFileOverWrite(enable);
    }
    CATCH_HANDLE("Could not set file over write enable.",
                 "qTabDataOutput::SetOverwriteEnable", this,
                 &qTabDataOutput::GetFileOverwrite)
}

void qTabDataOutput::GetTenGigaEnable() {
    LOG(logDEBUG) << "Getting 10GbE enable";
    disconnect(chkTenGiga, SIGNAL(toggled(bool)), this,
               SLOT(SetTenGigaEnable(bool)));
    try {
        auto retval = det->getTenGiga().tsquash(
            "10GbE enable is inconsistent for all detectors.");
        chkTenGiga->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get 10GbE enable.",
                  "qTabDataOutput::GetTenGigaEnable")
    connect(chkTenGiga, SIGNAL(toggled(bool)), this,
            SLOT(SetTenGigaEnable(bool)));
}

void qTabDataOutput::SetTenGigaEnable(bool enable) {
    LOG(logINFO) << "Setting 10GbE to " << enable;
    try {
        det->setTenGiga(enable);
    }
    CATCH_HANDLE("Could not set 10GbE enable.",
                 "qTabDataOutput::SetTenGigaEnable", this,
                 &qTabDataOutput::GetTenGigaEnable)
}

void qTabDataOutput::GetRateCorrection() {
    LOG(logDEBUG) << "Getting Rate Correction";
    disconnect(chkRate, SIGNAL(toggled(bool)), this,
               SLOT(EnableRateCorrection()));
    disconnect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
               SLOT(SetRateCorrection()));
    disconnect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
               SLOT(SetRateCorrection()));
    try {
        spinCustomDeadTime->setValue(-1);
        int64_t retval = det->getRateCorrection()
                             .tsquash("Rate correction (enable/tau) is "
                                      "inconsistent for all detectors.")
                             .count();
        chkRate->setChecked(retval == 0 ? false : true);
        if (retval != 0)
            spinCustomDeadTime->setValue(retval);
    }
    CATCH_DISPLAY("Could not get rate correction.",
                  "qTabDataOutput::GetRateCorrection")
    connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(EnableRateCorrection()));
    connect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
            SLOT(SetRateCorrection()));
    connect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
            SLOT(SetRateCorrection()));
}

void qTabDataOutput::EnableRateCorrection() {
    // enable
    if (chkRate->isChecked()) {
        SetRateCorrection();
        return;
    }
    LOG(logINFO) << "Disabling Rate correction";
    // disable
    try {
        det->setRateCorrection(ns(0));
    }
    CATCH_HANDLE("Could not switch off rate correction.",
                 "qTabDataOutput::EnableRateCorrection", this,
                 &qTabDataOutput::GetRateCorrection)
}

void qTabDataOutput::SetRateCorrection() {
    // do nothing if rate correction is disabled
    if (!chkRate->isChecked()) {
        return;
    }
    try {
        // custom dead time
        if (radioCustomDeadtime->isChecked()) {
            int64_t deadtime = spinCustomDeadTime->value();
            LOG(logINFO) << "Setting Rate Correction with custom dead time: "
                         << deadtime;
            det->setRateCorrection(ns(deadtime));
        }
        // default dead time
        else {
            LOG(logINFO) << "Setting Rate Correction with default dead time";
            det->setDefaultRateCorrection();
        }
    }
    CATCH_HANDLE("Could not set rate correction.",
                 "qTabDataOutput::SetRateCorrection", this,
                 &qTabDataOutput::GetRateCorrection)
}

void qTabDataOutput::GetSpeed() {
    LOG(logDEBUG) << "Getting Readout Speed";
    disconnect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetSpeed(int)));
    try {
        auto retval = det->getReadoutSpeed().tsquash(
            "Readout Speed is inconsistent for all detectors.");
        comboClkDivider->setCurrentIndex(static_cast<int>(retval));
    }
    CATCH_DISPLAY("Could not get readout speed.", "qTabDataOutput::GetSpeed")
    connect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSpeed(int)));
}

void qTabDataOutput::SetSpeed(int speed) {
    LOG(logINFO) << "Setting Readout Speed to "
                 << comboClkDivider->currentText().toLatin1().data();
    try {
        det->setReadoutSpeed(static_cast<slsDetectorDefs::speedLevel>(speed));
    }
    CATCH_HANDLE("Could not set readout speed.", "qTabDataOutput::SetSpeed",
                 this, &qTabDataOutput::GetSpeed)
}

void qTabDataOutput::GetParallel() {
    LOG(logDEBUG) << "Getting parallel readout";
    disconnect(chkParallel, SIGNAL(toggled(bool)), this,
               SLOT(SetParallel(bool)));
    try {
        auto retval = det->getParallelMode().tsquash(
            "Parallel Flag is inconsistent for all detectors.");
        chkParallel->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get parallel readout.",
                  "qTabDataOutput::GetParallel")
    connect(chkParallel, SIGNAL(toggled(bool)), this, SLOT(SetParallel(bool)));
}

void qTabDataOutput::SetParallel(bool enable) {
    LOG(logINFO) << "Setting PArallel readout to " << enable;
    try {
        det->setParallelMode(enable);
    }
    CATCH_HANDLE("Could not set parallel readout.",
                 "qTabDataOutput::SetParallel", this,
                 &qTabDataOutput::GetParallel)
}

void qTabDataOutput::Refresh() {
    LOG(logDEBUG) << "**Updating DataOutput Tab";

    EnableBrowse();
    GetFileWrite();
    GetFileName();
    GetOutputDir();
    GetFileOverwrite();
    GetFileFormat();
    if (chkRate->isEnabled()) {
        GetRateCorrection();
    }
    if (chkTenGiga->isEnabled()) {
        GetTenGigaEnable();
    }
    if (chkParallel->isEnabled()) {
        GetParallel();
    }
    if (comboClkDivider->isEnabled()) {
        GetSpeed();
    }

    LOG(logDEBUG) << "**Updated DataOutput Tab";
}

} // namespace sls

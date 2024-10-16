// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include "sls/string_utils.h"
#include <QStandardItemModel>
#include <QTimer>

namespace sls {

qTabMeasurement::qTabMeasurement(QWidget *parent, Detector *detector,
                                 qDrawPlot *p)
    : QWidget(parent), det(detector), plot(p), progressTimer(nullptr) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Measurement ready";
}

qTabMeasurement::~qTabMeasurement() { delete progressTimer; }

void qTabMeasurement::SetupWidgetWindow() {
    setFont(QFont("Carlito", 9, QFont::Normal));
    // palette
    red = QPalette();
    red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
    acqPeriodTip = spinPeriod->toolTip();
    errPeriodTip =
        QString("<nobr>Frame period between exposures.</nobr><br>"
                "<nobr> #period#</nobr><br><br>") +
        QString(
            "<nobr><font color=\"red\"><b>Acquisition Period</b> should be"
            " greater than or equal to <b>Exposure Time</b>.</font></nobr>");

    // timer to update the progress bar
    progressTimer = new QTimer(this);

    gateImplemented = false;
    delayImplemented = true;
    startingFnumImplemented = false;
    // by default, delay and starting fnum is disabled in form
    lblDelay->setEnabled(true);
    spinDelay->setEnabled(true);
    comboDelayUnit->setEnabled(true);

    // default is triggers and delay (not #bursts and burst period for gotthard2
    // in auto mode)
    ShowTriggerDelay();

    // enabling according to det type
    lblBurstMode->hide();
    comboBurstMode->hide();
    switch (det->getDetectorType().squash()) {
    case slsDetectorDefs::EIGER:
        delayImplemented = false;
        lblNextFrameNumber->setEnabled(true);
        spinNextFrameNumber->setEnabled(true);
        startingFnumImplemented = true;
        break;
    case slsDetectorDefs::JUNGFRAU:
    case slsDetectorDefs::MOENCH:
        lblNextFrameNumber->setEnabled(true);
        spinNextFrameNumber->setEnabled(true);
        startingFnumImplemented = true;
        break;
    case slsDetectorDefs::GOTTHARD2:
        lblBurstMode->show();
        comboBurstMode->show();
        lblNumBursts->setEnabled(true);
        spinNumBursts->setEnabled(true);
        lblBurstPeriod->setEnabled(true);
        spinBurstPeriod->setEnabled(true);
        comboBurstPeriodUnit->setEnabled(true);
        break;
    case slsDetectorDefs::MYTHEN3:
        gateImplemented = true;
        break;
    default:
        break;
    }

    SetupTimingMode();

    Initialization();

    Refresh();
    // normally called only if different
    EnableWidgetsforTimingMode();
}

void qTabMeasurement::Initialization() {
    connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetTimingMode(int)));
    if (comboBurstMode->isVisible()) {
        connect(comboBurstMode, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetBurstMode(int)));
    }
    connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumMeasurements(int)));
    connect(spinNumFrames, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumFrames(int)));
    connect(spinNumTriggers, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumTriggers(int)));
    if (spinNumBursts->isEnabled()) {
        connect(spinNumBursts, SIGNAL(valueChanged(int)), this,
                SLOT(SetNumBursts(int)));
    }
    if (gateImplemented) {
        connect(spinNumGates, SIGNAL(valueChanged(int)), this,
                SLOT(SetNumGates(int)));
    }
    connect(spinExpTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetExposureTime()));
    connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetExposureTime()));
    connect(spinPeriod, SIGNAL(valueChanged(double)), this,
            SLOT(SetAcquisitionPeriod()));
    connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetAcquisitionPeriod()));
    if (spinDelay->isEnabled()) {
        connect(spinDelay, SIGNAL(valueChanged(double)), this,
                SLOT(SetDelay()));
        connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetDelay()));
    }
    if (spinBurstPeriod->isEnabled()) {
        connect(spinBurstPeriod, SIGNAL(valueChanged(double)), this,
                SLOT(SetBurstPeriod()));
        connect(comboBurstPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetBurstPeriod()));
    }
    connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
    connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
    connect(dispFileName, SIGNAL(returnPressed()), this,
            SLOT(ForceSetFileName()));
    connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
    if (startingFnumImplemented) {
        connect(spinNextFrameNumber, SIGNAL(valueChanged(int)), this,
                SLOT(SetNextFrameNumber(int)));
    }
    connect(progressTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));
    connect(btnStart, SIGNAL(clicked()), this, SLOT(StartAcquisition()));
    connect(btnStop, SIGNAL(clicked()), this, SLOT(StopAcquisition()));
}

void qTabMeasurement::ShowTriggerDelay() {
    bool showTrigger = true;
    if (det->getDetectorType().squash() == slsDetectorDefs::GOTTHARD2) {
        // burst and auto
        if ((comboBurstMode->currentIndex() ==
                 slsDetectorDefs::BURST_INTERNAL ||
             comboBurstMode->currentIndex() ==
                 slsDetectorDefs::BURST_EXTERNAL) &&
            (comboTimingMode->currentIndex() == AUTO)) {
            // show burst, burstperiod, not trigger or delay
            showTrigger = false;
        }

        // frame and period are disabled for cont trigger in g2
        bool enableFramePeriod = true;
        if ((comboBurstMode->currentIndex() ==
                 slsDetectorDefs::CONTINUOUS_INTERNAL ||
             comboBurstMode->currentIndex() ==
                 slsDetectorDefs::CONTINUOUS_EXTERNAL) &&
            (comboTimingMode->currentIndex() == TRIGGER)) {
            enableFramePeriod = false;
        }
        lblNumFrames->setEnabled(enableFramePeriod);
        spinNumFrames->setEnabled(enableFramePeriod);
        lblPeriod->setEnabled(enableFramePeriod);
        spinPeriod->setEnabled(enableFramePeriod);
        comboPeriodUnit->setEnabled(enableFramePeriod);
    }

    if (showTrigger) {
        stackedLblTriggerBurst->setCurrentWidget(pageLblTrigger);
        stackedSpinTriggerBurst->setCurrentWidget(pageSpinTrigger);
        stackedLblDelayBurstPeriod->setCurrentWidget(pageLblDelay);
        stackedSpinDelayBurstPeriod->setCurrentWidget(pageSpinDelay);
        stackedComboDelayBurstPeriod->setCurrentWidget(pageComboDelay);
    } else {
        stackedLblTriggerBurst->setCurrentWidget(pageLblBurst);
        stackedSpinTriggerBurst->setCurrentWidget(pageSpinBurst);
        stackedLblDelayBurstPeriod->setCurrentWidget(pageLblBurstPeriod);
        stackedSpinDelayBurstPeriod->setCurrentWidget(pageSpinBurstPeriod);
        stackedComboDelayBurstPeriod->setCurrentWidget(pageComboBurstPeriod);
    }
}

void qTabMeasurement::SetupTimingMode() {
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(comboTimingMode->model());
    QModelIndex index[NUMTIMINGMODES];
    QStandardItem *item[NUMTIMINGMODES];
    if (model) {
        for (int i = 0; i < NUMTIMINGMODES; i++) {
            index[i] = model->index(i, comboTimingMode->modelColumn(),
                                    comboTimingMode->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
            item[i]->setEnabled(false);
        }
        try {
            auto res = det->getTimingModeList();
            for (auto it : res) {
                item[(int)it]->setEnabled(true);
            }
        }
        CATCH_DISPLAY(std::string("Could not setup timing mode"),
                      "qTabMeasurement::SetupTimingMode")
    }
}

void qTabMeasurement::EnableWidgetsforTimingMode() {
    LOG(logDEBUG) << "Enabling Widgets for Timing Mode";

    // default
    lblNumFrames->setEnabled(false);
    spinNumFrames->setEnabled(false);
    lblNumTriggers->setEnabled(false);
    spinNumTriggers->setEnabled(false);
    lblExpTime->setEnabled(false);
    spinExpTime->setEnabled(false);
    comboExpUnit->setEnabled(false);
    lblPeriod->setEnabled(false);
    spinPeriod->setEnabled(false);
    comboPeriodUnit->setEnabled(false);
    lblDelay->setEnabled(false);
    spinDelay->setEnabled(false);
    comboDelayUnit->setEnabled(false);
    lblNumGates->setEnabled(false);
    spinNumGates->setEnabled(false);

    switch (comboTimingMode->currentIndex()) {
    case AUTO:
        // #frames, exptime, period
        if (det->getDetectorType().squash() != slsDetectorDefs::GOTTHARD2) {
            spinNumTriggers->setValue(1);
        }
        lblNumFrames->setEnabled(true);
        spinNumFrames->setEnabled(true);
        lblExpTime->setEnabled(true);
        spinExpTime->setEnabled(true);
        comboExpUnit->setEnabled(true);
        lblPeriod->setEnabled(true);
        spinPeriod->setEnabled(true);
        comboPeriodUnit->setEnabled(true);
        if (det->getDetectorType().squash() == slsDetectorDefs::GOTTHARD2) {
            GetBurstMode(); // also decides to show trigger or burst mode
        }
        break;
    case TRIGGER:
        // #triggers, exptime
        lblNumTriggers->setEnabled(true);
        spinNumTriggers->setEnabled(true);
        lblExpTime->setEnabled(true);
        spinExpTime->setEnabled(true);
        comboExpUnit->setEnabled(true);
        // not implemented  in FW to have multiple frames for eiger
        if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
            spinNumFrames->setValue(1);
        } else {
            // #frames, period, delay
            lblNumFrames->setEnabled(true);
            spinNumFrames->setEnabled(true);
            lblPeriod->setEnabled(true);
            spinPeriod->setEnabled(true);
            comboPeriodUnit->setEnabled(true);
            lblDelay->setEnabled(true);
            spinDelay->setEnabled(true);
            comboDelayUnit->setEnabled(true);
            if (det->getDetectorType().squash() == slsDetectorDefs::GOTTHARD2) {
                GetBurstMode(); // also decides to show trigger or burst mode
            }
        }
        break;
    case GATED:
        // #frames, #gates(mythen3)
        spinNumTriggers->setValue(1);
        if (det->getDetectorType().squash() == slsDetectorDefs::MYTHEN3) {
            lblNumGates->setEnabled(true);
            spinNumGates->setEnabled(true);
        }
        lblNumFrames->setEnabled(true);
        spinNumFrames->setEnabled(true);
        break;
    case BURST_TRIGGER:
        // #frames, exptime, period
        spinNumTriggers->setValue(1);
        lblNumFrames->setEnabled(true);
        spinNumFrames->setEnabled(true);
        lblExpTime->setEnabled(true);
        spinExpTime->setEnabled(true);
        comboExpUnit->setEnabled(true);
        lblPeriod->setEnabled(true);
        spinPeriod->setEnabled(true);
        comboPeriodUnit->setEnabled(true);
        break;
    case TRIGGER_GATED:
        // #triggers, delay, #frames, #gates
        lblNumTriggers->setEnabled(true);
        spinNumTriggers->setEnabled(true);
        lblDelay->setEnabled(true);
        spinDelay->setEnabled(true);
        comboDelayUnit->setEnabled(true);
        lblNumFrames->setEnabled(true);
        spinNumFrames->setEnabled(true);
        lblNumGates->setEnabled(true);
        spinNumGates->setEnabled(true);
        break;
    default:
        break;
    }

    CheckAcqPeriodGreaterThanExp();
}

void qTabMeasurement::GetTimingMode() {
    LOG(logDEBUG) << "Getting timing mode";
    disconnect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetTimingMode(int)));
    try {

        slsDetectorDefs::timingMode retval{slsDetectorDefs::AUTO_TIMING};
        // m3: remove slave modes (always trigger) before squashing
        if (det->getDetectorType().squash() == slsDetectorDefs::MYTHEN3) {
            auto retvals = det->getTimingMode();
            auto is_master = det->getMaster();
            Result<slsDetectorDefs::timingMode> masterRetvals;
            for (size_t i = 0; i != is_master.size(); ++i) {
                if (is_master[i]) {
                    masterRetvals.push_back(retvals[i]);
                }
            }
            retval = masterRetvals.tsquash(
                "Inconsistent timing mode for all detectors.");
        } else {
            retval = det->getTimingMode().tsquash(
                "Inconsistent timing mode for all detectors.");
        }

        auto oldMode = comboTimingMode->currentIndex();
        switch (retval) {
        case slsDetectorDefs::AUTO_TIMING:
        case slsDetectorDefs::TRIGGER_EXPOSURE:
        case slsDetectorDefs::GATED:
        case slsDetectorDefs::BURST_TRIGGER:
        case slsDetectorDefs::TRIGGER_GATED:
            comboTimingMode->setCurrentIndex((int)retval);
            // update widget enable only if different
            if (oldMode != comboTimingMode->currentIndex()) {
                EnableWidgetsforTimingMode();
            }
            break;
        default:
            throw RuntimeError(std::string("Unknown timing mode: ") +
                               std::to_string(retval));
        }
    }
    CATCH_DISPLAY("Could not get timing mode.",
                  "qTabMeasurement::GetTimingMode")
    connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetTimingMode(int)));
}

void qTabMeasurement::SetTimingMode(int val) {
    LOG(logINFO) << "Setting timing mode:"
                 << comboTimingMode->currentText().toLatin1().data();
    try {
        det->setTimingMode(static_cast<slsDetectorDefs::timingMode>(val));
        EnableWidgetsforTimingMode();
    }
    CATCH_HANDLE("Could not set timing mode.", "qTabMeasurement::SetTimingMode",
                 this, &qTabMeasurement::GetTimingMode)
}

void qTabMeasurement::GetBurstMode() {
    LOG(logDEBUG) << "Getting burst mode";
    disconnect(comboBurstMode, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetBurstMode(int)));
    try {
        auto retval = det->getBurstMode().tsquash(
            "Inconsistent burst mode for all detectors.");
        switch (retval) {
        case slsDetectorDefs::BURST_INTERNAL:
        case slsDetectorDefs::BURST_EXTERNAL:
        case slsDetectorDefs::CONTINUOUS_INTERNAL:
        case slsDetectorDefs::CONTINUOUS_EXTERNAL:
            comboBurstMode->setCurrentIndex((int)retval);
            ShowTriggerDelay();
            break;
        default:
            throw RuntimeError(std::string("Unknown burst mode: ") +
                               std::to_string(retval));
        }
    }
    CATCH_DISPLAY("Could not get burst mode.", "qTabMeasurement::GetBurstMode")
    connect(comboBurstMode, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetBurstMode(int)));
}

void qTabMeasurement::SetBurstMode(int val) {
    LOG(logINFO) << "Setting burst mode:"
                 << comboBurstMode->currentText().toLatin1().data();
    try {
        det->setBurstMode(static_cast<slsDetectorDefs::burstMode>(val));
        ShowTriggerDelay();
    }
    CATCH_HANDLE("Could not set burst mode.", "qTabMeasurement::SetBurstMode",
                 this, &qTabMeasurement::GetBurstMode)
}

void qTabMeasurement::SetNumMeasurements(int val) {
    LOG(logINFO) << "Setting Number of Measurements to " << val;
    numMeasurements = val;
}

void qTabMeasurement::GetNumFrames() {
    LOG(logDEBUG) << "Getting number of frames";
    disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumFrames(int)));
    try {
        auto retval = det->getNumberOfFrames().tsquash(
            "Inconsistent number of frames for all detectors.");
        spinNumFrames->setValue(retval);
    }
    CATCH_DISPLAY("Could not get number of frames.",
                  "qTabMeasurement::GetNumFrames")
    connect(spinNumFrames, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumFrames(int)));
}

void qTabMeasurement::SetNumFrames(int val) {
    LOG(logINFO) << "Setting number of frames to " << val;
    try {
        det->setNumberOfFrames(val);
    }
    CATCH_HANDLE("Could not set number of frames.",
                 "qTabMeasurement::SetNumFrames", this,
                 &qTabMeasurement::GetNumFrames)
}

void qTabMeasurement::GetNumTriggers() {
    LOG(logDEBUG) << "Getting number of triggers";
    disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumTriggers(int)));
    try {
        auto retval = det->getNumberOfTriggers().tsquash(
            "Inconsistent number of triggers for all detectors.");
        spinNumTriggers->setValue(retval);
    }
    CATCH_DISPLAY("Could not get number of frames.",
                  "qTabMeasurement::GetNumTriggers")
    connect(spinNumTriggers, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumTriggers(int)));
}

void qTabMeasurement::SetNumTriggers(int val) {
    LOG(logINFO) << "Setting number of triggers to " << val;
    try {
        det->setNumberOfTriggers(val);
    }
    CATCH_HANDLE("Could not set number of triggers.",
                 "qTabMeasurement::SetNumTriggers", this,
                 &qTabMeasurement::GetNumTriggers)
}

void qTabMeasurement::GetNumBursts() {
    LOG(logDEBUG) << "Getting number of bursts";
    disconnect(spinNumBursts, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumBursts(int)));
    try {
        auto retval = det->getNumberOfBursts().tsquash(
            "Inconsistent number of bursts for all detectors.");
        spinNumBursts->setValue(retval);
    }
    CATCH_DISPLAY("Could not get number of frames.",
                  "qTabMeasurement::GetNumBursts")
    connect(spinNumBursts, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumBursts(int)));
}

void qTabMeasurement::SetNumBursts(int val) {
    LOG(logINFO) << "Setting number of bursts to " << val;
    try {
        det->setNumberOfBursts(val);
    }
    CATCH_HANDLE("Could not set number of bursts.",
                 "qTabMeasurement::SetNumBursts", this,
                 &qTabMeasurement::GetNumBursts)
}

void qTabMeasurement::GetNumGates() {
    LOG(logDEBUG) << "Getting number of gates";
    disconnect(spinNumGates, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumGates(int)));
    try {
        auto retval = det->getNumberOfGates().tsquash(
            "Inconsistent number of gates for all detectors.");
        spinNumGates->setValue(retval);
    }
    CATCH_DISPLAY("Could not get number of gates.",
                  "qTabMeasurement::GetNumGates")
    connect(spinNumGates, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumGates(int)));
}

void qTabMeasurement::SetNumGates(int val) {
    LOG(logINFO) << "Setting number of external gates to " << val;
    try {
        det->setNumberOfGates(val);
    }
    CATCH_HANDLE("Could not set number of gates.",
                 "qTabMeasurement::SetNumGates", this,
                 &qTabMeasurement::GetNumGates)
}

void qTabMeasurement::GetExposureTime() {
    LOG(logDEBUG) << "Getting exposure time";
    disconnect(spinExpTime, SIGNAL(valueChanged(double)), this,
               SLOT(SetExposureTime()));
    disconnect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetExposureTime()));
    try {
        spinExpTime->setValue(-1);

        bool inconsistentGateValues = false;
        std::chrono::nanoseconds retval;
        if (det->getDetectorType().squash() == slsDetectorDefs::MYTHEN3) {
            auto retvals = det->getExptimeForAllGates().tsquash(
                "Inconsistent exposure time for all detectors.");
            // all gates have same value
            if (retvals[0] == retvals[1] && retvals[1] == retvals[2]) {
                retval = retvals[0];
            } else {
                // dont throw, just leave it as -1
                inconsistentGateValues = true;
            }
        } else {
            retval = det->getExptime().tsquash(
                "Inconsistent exposure time for all detectors.");
        }

        if (!inconsistentGateValues) {
            auto time = qDefs::getUserFriendlyTime(retval);
            spinExpTime->setValue(time.first);
            comboExpUnit->setCurrentIndex(static_cast<int>(time.second));
            CheckAcqPeriodGreaterThanExp();
        }
    }
    CATCH_DISPLAY("Could not get exposure time.",
                  "qTabMeasurement::GetExposureTime")
    connect(spinExpTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetExposureTime()));
    connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetExposureTime()));
}

void qTabMeasurement::SetExposureTime() {
    auto val = spinExpTime->value();
    auto unit = static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex());
    LOG(logINFO) << "Setting exposure time to " << val << " "
                 << qDefs::getUnitString(unit);
    try {
        auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
        det->setExptime(timeNS);
        CheckAcqPeriodGreaterThanExp();
    }
    CATCH_HANDLE("Could not set exposure time.",
                 "qTabMeasurement::SetExposureTime", this,
                 &qTabMeasurement::GetExposureTime)
}

void qTabMeasurement::GetAcquisitionPeriod() {
    LOG(logDEBUG) << "Getting acquisition period";
    disconnect(spinPeriod, SIGNAL(valueChanged(double)), this,
               SLOT(SetAcquisitionPeriod()));
    disconnect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetAcquisitionPeriod()));
    try {
        spinPeriod->setValue(-1);
        auto retval = det->getPeriod().tsquash(
            "Inconsistent acquisition period for all detectors.");
        auto time = qDefs::getUserFriendlyTime(retval);
        spinPeriod->setValue(time.first);
        comboPeriodUnit->setCurrentIndex(static_cast<int>(time.second));
        CheckAcqPeriodGreaterThanExp();
    }
    CATCH_DISPLAY("Could not get acquisition period.",
                  "qTabMeasurement::GetAcquisitionPeriod")
    connect(spinPeriod, SIGNAL(valueChanged(double)), this,
            SLOT(SetAcquisitionPeriod()));
    connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetAcquisitionPeriod()));
}

void qTabMeasurement::SetAcquisitionPeriod() {
    auto val = spinPeriod->value();
    auto unit = static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex());
    LOG(logINFO) << "Setting acquisition period to " << val << " "
                 << qDefs::getUnitString(unit);
    try {
        auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
        det->setPeriod(timeNS);
        CheckAcqPeriodGreaterThanExp();
    }
    CATCH_HANDLE("Could not set acquisition period.",
                 "qTabMeasurement::SetAcquisitionPeriod", this,
                 &qTabMeasurement::GetAcquisitionPeriod)
}

void qTabMeasurement::CheckAcqPeriodGreaterThanExp() {
    LOG(logDEBUG) << "Checking period >= exptime";
    bool error = false;
    if (lblPeriod->isEnabled()) {
        auto exptimeNS = qDefs::getNSTime(std::make_pair(
            spinExpTime->value(),
            static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex())));
        auto acqtimeNS = qDefs::getNSTime(std::make_pair(
            spinPeriod->value(),
            static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex())));
        if (exptimeNS > acqtimeNS) {
            error = true;
            spinPeriod->setToolTip(errPeriodTip);
            lblPeriod->setToolTip(errPeriodTip);
            lblPeriod->setPalette(red);
            lblPeriod->setText("Acquisition Period:*");
        }
    }

    if (!error) {
        spinPeriod->setToolTip(acqPeriodTip);
        lblPeriod->setToolTip(acqPeriodTip);
        lblPeriod->setPalette(lblTimingMode->palette());
        lblPeriod->setText("Acquisition Period:");
    }
}

void qTabMeasurement::GetDelay() {
    LOG(logDEBUG) << "Getting delay";
    disconnect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
    disconnect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetDelay()));
    try {
        spinDelay->setValue(-1);
        auto retval = det->getDelayAfterTrigger().tsquash(
            "Inconsistent delay for all detectors.");
        auto time = qDefs::getUserFriendlyTime(retval);
        spinDelay->setValue(time.first);
        comboDelayUnit->setCurrentIndex(static_cast<int>(time.second));
    }
    CATCH_DISPLAY("Could not get delay.", "qTabMeasurement::GetDelay")
    connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
    connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetDelay()));
}

void qTabMeasurement::SetDelay() {
    auto val = spinDelay->value();
    auto unit = static_cast<qDefs::timeUnit>(comboDelayUnit->currentIndex());
    LOG(logINFO) << "Setting delay to " << val << " "
                 << qDefs::getUnitString(unit);
    try {
        auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
        det->setDelayAfterTrigger(timeNS);
    }
    CATCH_HANDLE("Could not set delay.", "qTabMeasurement::SetDelay", this,
                 &qTabMeasurement::GetDelay)
}

void qTabMeasurement::GetBurstPeriod() {
    LOG(logDEBUG) << "Getting Burst Period";
    disconnect(spinBurstPeriod, SIGNAL(valueChanged(double)), this,
               SLOT(SetBurstPeriod()));
    disconnect(comboBurstPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetBurstPeriod()));
    try {
        spinBurstPeriod->setValue(-1);
        auto retval = det->getBurstPeriod().tsquash(
            "Inconsistent burst period for all detectors.");
        auto time = qDefs::getUserFriendlyTime(retval);
        spinBurstPeriod->setValue(time.first);
        comboBurstPeriodUnit->setCurrentIndex(static_cast<int>(time.second));
    }
    CATCH_DISPLAY("Could not get burst period.",
                  "qTabMeasurement::GetBurstPeriod")
    connect(spinBurstPeriod, SIGNAL(valueChanged(double)), this,
            SLOT(SetBurstPeriod()));
    connect(comboBurstPeriodUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetBurstPeriod()));
}

void qTabMeasurement::SetBurstPeriod() {
    auto val = spinBurstPeriod->value();
    auto unit =
        static_cast<qDefs::timeUnit>(comboBurstPeriodUnit->currentIndex());
    LOG(logINFO) << "Setting burst period to " << val << " "
                 << qDefs::getUnitString(unit);
    try {
        auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
        det->setBurstPeriod(timeNS);
    }
    CATCH_HANDLE("Could not set burst period.",
                 "qTabMeasurement::SetBurstPeriod", this,
                 &qTabMeasurement::GetBurstPeriod)
}

void qTabMeasurement::GetFileWrite() {
    LOG(logDEBUG) << "Getting File Write Enable";
    disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
    try {
        dispFileName->setEnabled(true); // default, even when exception
        lblIndex->setEnabled(true);
        spinIndex->setEnabled(true);
        auto retval = det->getFileWrite().tsquash(
            "Inconsistent file write for all detectors.");
        chkFile->setChecked(retval);
        dispFileName->setEnabled(retval);
        lblIndex->setEnabled(retval);
        spinIndex->setEnabled(retval);
    }
    CATCH_DISPLAY("Could not get file over write enable.",
                  "qTabMeasurement::GetFileWrite")
    connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
}

void qTabMeasurement::SetFileWrite(bool val) {
    LOG(logINFO) << "Set File Write to " << val;
    try {
        det->setFileWrite(val);
        dispFileName->setEnabled(val);
        lblIndex->setEnabled(val);
        spinIndex->setEnabled(val);
    }
    CATCH_HANDLE("Could not set file write enable.",
                 "qTabMeasurement::SetFileWrite", this,
                 &qTabMeasurement::GetFileWrite)
}

void qTabMeasurement::GetFileName() {
    LOG(logDEBUG) << "Getting file name prefix";
    disconnect(dispFileName, SIGNAL(editingFinished()), this,
               SLOT(SetFileName()));
    try {
        auto retval = det->getFileNamePrefix().tsquash(
            "Inconsistent file name prefix for all detectors.");
        dispFileName->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get file name prefix.",
                  "qTabMeasurement::GetFileName")
    connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
}

void qTabMeasurement::SetFileName(bool force) {
    // return forces modification (inconsistency from command line)
    if (dispFileName->isModified() || force) {
        dispFileName->setModified(false);
        std::string val =
            std::string(dispFileName->text().toLatin1().constData());
        LOG(logINFO) << "Setting File Name Prefix:" << val;
        try {
            det->setFileNamePrefix(val);
        }
        CATCH_HANDLE("Could not set file name prefix.",
                     "qTabMeasurement::SetFileName", this,
                     &qTabMeasurement::GetFileName)

        emit FileNameChangedSignal(dispFileName->text());
    }
}

void qTabMeasurement::ForceSetFileName() { SetFileName(true); }

void qTabMeasurement::GetRunIndex() {
    LOG(logDEBUG) << "Getting Acquisition File index";
    disconnect(spinIndex, SIGNAL(valueChanged(int)), this,
               SLOT(SetRunIndex(int)));
    try {
        auto retval = det->getAcquisitionIndex().tsquash(
            "Inconsistent file index for all detectors.");
        spinIndex->setValue(retval);
    }
    CATCH_DISPLAY("Could not get acquisition file index.",
                  "qTabMeasurement::GetRunIndex")
    connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
}

void qTabMeasurement::SetRunIndex(int val) {
    LOG(logINFO) << "Setting Acquisition File Index to " << val;
    try {
        det->setAcquisitionIndex(val);
    }
    CATCH_HANDLE("Could not set acquisition file index.",
                 "qTabMeasurement::SetRunIndex", this,
                 &qTabMeasurement::GetRunIndex)
}

void qTabMeasurement::GetNextFrameNumber() {
    LOG(logDEBUG) << "Getting Starting Frame Number";
    disconnect(spinNextFrameNumber, SIGNAL(valueChanged(int)), this,
               SLOT(SetNextFrameNumber(int)));
    try {
        auto retval = det->getNextFrameNumber().tsquash(
            "Inconsistent starting frame number for all detectors.");
        spinNextFrameNumber->setValue(retval);
    }
    CATCH_HANDLE("Could not get starting frame number.",
                 "qTabMeasurement::GetNextFrameNumber", spinNextFrameNumber,
                 &QSpinBox::setValue, -1)
    connect(spinNextFrameNumber, SIGNAL(valueChanged(int)), this,
            SLOT(SetNextFrameNumber(int)));
}

void qTabMeasurement::SetNextFrameNumber(int val) {
    LOG(logINFO) << "Setting Starting frame number to " << val;
    try {
        det->setNextFrameNumber(val);
    }
    CATCH_HANDLE("Could not set starting frame number.",
                 "qTabMeasurement::SetNextFrameNumber", this,
                 &qTabMeasurement::GetNextFrameNumber)
}

void qTabMeasurement::ResetProgress() {
    std::lock_guard<std::mutex> lock(mProgress);
    LOG(logDEBUG) << "Resetting progress";
    lblCurrentFrame->setText("0");
    lblCurrentMeasurement->setText("0");
    progressBar->setValue(0);
}

void qTabMeasurement::UpdateProgress() {
    LOG(logDEBUG) << "Updating progress";
    std::lock_guard<std::mutex> lock(mProgress);
    progressBar->setValue(plot->GetProgress());
    lblCurrentFrame->setText(QString::number(plot->GetCurrentFrameIndex()));
    lblCurrentMeasurement->setText(QString::number(currentMeasurement));
}

int qTabMeasurement::VerifyOutputDirectoryError() {
    try {
        auto retval = det->getFilePath();
        for (int i = 0; i < static_cast<int>(retval.size()); ++i) {
            det->setFilePath(retval[i], {i});
        }
        return slsDetectorDefs::OK;
    }
    CATCH_DISPLAY("Could not set path.",
                  "qTabMeasurement::VerifyOutputDirectoryError")
    return slsDetectorDefs::FAIL; // for exception
}

void qTabMeasurement::StartAcquisition() {
    btnStart->setEnabled(false);
    // if file write enabled and output dir doesnt exist
    if ((chkFile->isChecked()) &&
        (VerifyOutputDirectoryError() == slsDetectorDefs::FAIL)) {
        if (qDefs::Message(
                qDefs::QUESTION,
                "<nobr>Your data will not be saved.</nobr><br><nobr>Disable "
                "File write and Proceed with acquisition anyway?</nobr>",
                "qTabMeasurement::StartAcquisition") == slsDetectorDefs::FAIL) {
            btnStart->setEnabled(true);
            return;
        } else {
            disconnect(chkFile, SIGNAL(toggled(bool)), this,
                       SLOT(SetFileWrite(bool)));
            chkFile->setChecked(false);
            // cannot wait for signals from chkFile
            SetFileWrite(false);
            connect(chkFile, SIGNAL(toggled(bool)), this,
                    SLOT(SetFileWrite(bool)));
        }
    }

    LOG(logINFOBLUE) << "Starting Acquisition";
    plot->SetRunning(true);
    isAcquisitionStopped = false;
    currentMeasurement = 0;
    ResetProgress();
    Enable(0);
    progressTimer->start(100);
    emit EnableTabsSignal(false);
}

void qTabMeasurement::StopAcquisition() {
    LOG(logINFORED) << "Stopping Acquisition";
    try {
        isAcquisitionStopped = true;
        det->stopDetector();
    }
    CATCH_DISPLAY("Could not stop acquisition.",
                  "qTabMeasurement::StopAcquisition")
}

void qTabMeasurement::AcquireFinished() {
    // to catch only once (if abort acquire also calls acq finished call back)
    if (!btnStart->isEnabled()) {
        LOG(logDEBUG) << "Acquire Finished";
        UpdateProgress();
        GetRunIndex();
        if (startingFnumImplemented) {
            GetNextFrameNumber();
        }
        LOG(logDEBUG) << "Measurement " << currentMeasurement << " finished";
        // next measurement if acq is not stopped
        if (!isAcquisitionStopped &&
            ((currentMeasurement + 1) < numMeasurements)) {
            ++currentMeasurement;
            plot->StartAcquisition();
        }
        // end of acquisition
        else {
            progressTimer->stop();
            Enable(1);
            plot->SetRunning(false);
            btnStart->setEnabled(true);
            emit EnableTabsSignal(true);
        }
    }
}

void qTabMeasurement::AbortAcquire(QString exmsg) {
    LOG(logINFORED) << "Abort Acquire";
    qDefs::ExceptionMessage("Acquire unsuccessful.",
                            exmsg.toLatin1().constData(),
                            "qDrawPlot::AcquireFinished");
    isAcquisitionStopped = true;
    AcquireFinished();
}

void qTabMeasurement::Enable(bool enable) {
    frameTimeResolved->setEnabled(enable);
    frameNotTimeResolved->setEnabled(enable);

    // shortcut each time, else it doesnt work a second time
    btnStart->setShortcut(QApplication::translate("TabMeasurementObject",
                                                  "Shift+Space", nullptr));
}

void qTabMeasurement::Refresh() {
    LOG(logDEBUG) << "**Updating Measurement Tab";

    if (!plot->GetIsRunning()) {
        GetTimingMode();
        if (comboBurstMode->isVisible()) {
            GetBurstMode();
        }
        GetNumFrames();
        GetExposureTime();
        GetAcquisitionPeriod();
        GetNumTriggers();
        if (spinNumBursts->isEnabled()) {
            GetNumBursts();
        }
        if (delayImplemented) {
            GetDelay();
        }
        if (spinBurstPeriod->isEnabled()) {
            GetBurstPeriod();
        }
        if (gateImplemented) {
            GetNumGates();
        }
        GetFileWrite();
        GetFileName();
        GetRunIndex();
        if (startingFnumImplemented) {
            GetNextFrameNumber();
        }
        ResetProgress();
    }

    LOG(logDEBUG) << "**Updated Measurement Tab";
}

} // namespace sls

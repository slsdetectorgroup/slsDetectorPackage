// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabSettings.h"
#include "qDefs.h"
#include "sls/ToString.h"
#include "sls/bit_utils.h"
#include <QStandardItemModel>

namespace sls {

qTabSettings::qTabSettings(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Settings ready";
}

qTabSettings::~qTabSettings() {}

void qTabSettings::SetupWidgetWindow() {
    comboHV->hide();
    lblComboHV->hide();
    lblSpinHV->hide();
    spinHV->hide();
    hvmin = HV_MIN;

    counters = std::vector<QCheckBox *>{chkCounter1, chkCounter2, chkCounter3};

    spinThreshold2->hide();
    spinThreshold3->hide();
    btnSetThreshold->hide();
    btnSetThreshold->setEnabled(false);
    lblCounter->hide();
    lblCounter->setEnabled(false);
    chkCounter1->setEnabled(false);
    chkCounter2->setEnabled(false);
    chkCounter3->setEnabled(false);
    chkCounter1->hide();
    chkCounter2->hide();
    chkCounter3->hide();

    // enabling according to det type
    slsDetectorDefs::detectorType detType = det->getDetectorType().squash();
    if (detType == slsDetectorDefs::MYTHEN3) {
        lblSpinHV->show();
        spinHV->show();
        hvmin = 0;
        lblDynamicRange->setEnabled(true);
        comboDynamicRange->setEnabled(true);

        spinThreshold2->show();
        spinThreshold3->show();
        lblThreshold->setEnabled(true);
        spinThreshold->setEnabled(true);
        spinThreshold2->setEnabled(true);
        spinThreshold3->setEnabled(true);
        btnSetThreshold->setEnabled(true);
        btnSetThreshold->show();

        lblCounter->show();
        lblCounter->setEnabled(true);
        chkCounter1->setEnabled(true);
        chkCounter2->setEnabled(true);
        chkCounter3->setEnabled(true);
        chkCounter1->show();
        chkCounter2->show();
        chkCounter3->show();

        // disable dr
        QStandardItemModel *model =
            qobject_cast<QStandardItemModel *>(comboDynamicRange->model());
        if (model) {
            QStandardItem *item;
            int dr = DYNAMICRANGE_4;
            for (int i = 0; i != 2; ++i) {
                // disable dr 4
                QModelIndex index =
                    model->index(dr, comboDynamicRange->modelColumn(),
                                 comboDynamicRange->rootModelIndex());
                item = model->itemFromIndex(index);
                item->setEnabled(false);

                // disable dr 12
                dr = DYNAMICRANGE_12;
            }
        }
    } else if (detType == slsDetectorDefs::EIGER) {
        lblSpinHV->show();
        spinHV->show();
        hvmin = 0;
        lblDynamicRange->setEnabled(true);
        comboDynamicRange->setEnabled(true);
        lblThreshold->setEnabled(true);
        spinThreshold->setEnabled(true);
    } else if (detType == slsDetectorDefs::JUNGFRAU) {
        lblSpinHV->show();
        spinHV->show();
        lblGainMode->setEnabled(true);
        comboGainMode->setEnabled(true);
    } else if (detType == slsDetectorDefs::MOENCH) {
        lblSpinHV->show();
        spinHV->show();
    } else if (detType == slsDetectorDefs::GOTTHARD) {
        comboHV->show();
        lblComboHV->show();
    } else if (detType == slsDetectorDefs::GOTTHARD2) {
        lblSpinHV->show();
        spinHV->show();
        hvmin = 0;
    }

    // default settings for the disabled
    comboSettings->setCurrentIndex(UNINITIALIZED);
    if (comboSettings->isEnabled()) {
        SetupDetectorSettings();
    }
    spinThreshold->setValue(-1);
    if (detType == slsDetectorDefs::MYTHEN3) {
        spinThreshold2->setValue(-1);
        spinThreshold3->setValue(-1);
    }

    // default for gain mode
    if (comboGainMode->isEnabled()) {
        SetupGainMode();
    }

    Initialization();
    // default for the disabled
    GetDynamicRange();
    Refresh();
}

void qTabSettings::SetExportMode(bool exportMode) {
    if (comboGainMode->isEnabled()) {
        ShowFixG0(exportMode);
    }
}

void qTabSettings::SetupDetectorSettings() {
    comboSettings->setCurrentIndex(UNINITIALIZED);

    // enable only those available to detector
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(comboSettings->model());
    const int numSettings = comboSettings->count();
    if (model) {
        std::vector<QModelIndex> index(numSettings);
        std::vector<QStandardItem *> item(numSettings);
        for (size_t i = 0; i < index.size(); ++i) {
            index[i] = model->index(i, comboSettings->modelColumn(),
                                    comboSettings->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
            item[i]->setEnabled(false);
        }
        try {
            auto res = det->getSettingsList();
            for (auto it : res) {
                item[(int)it]->setEnabled(true);
            }
        }
        CATCH_DISPLAY(std::string("Could not setup settings"),
                      "qTabSettings::SetupDetectorSettings")
    }
}

void qTabSettings::SetupGainMode() {
    comboGainMode->setCurrentIndex(DYNAMIC);
    ShowFixG0(false);
}

void qTabSettings::ShowFixG0(bool expertMode) {
    LOG(logINFO) << (expertMode ? "Showing" : "Hiding") << " FIX_G0";

    // enable.disable Fix G0
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(comboGainMode->model());
    const int numSettings = comboGainMode->count();
    if (model) {
        std::vector<QModelIndex> index(numSettings);
        std::vector<QStandardItem *> item(numSettings);
        index[FIX_G0] = model->index(FIX_G0, comboGainMode->modelColumn(),
                                     comboGainMode->rootModelIndex());
        item[FIX_G0] = model->itemFromIndex(index[FIX_G0]);
        item[FIX_G0]->setEnabled(expertMode);
    }
    isVisibleFixG0 = expertMode;
}

void qTabSettings::Initialization() {
    // High voltage
    connect(comboHV, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetHighVoltage()));
    connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));

    // Settings
    if (comboSettings->isEnabled())
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSettings(int)));
    // Gain mode
    if (comboGainMode->isEnabled())
        connect(comboGainMode, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetGainMode(int)));

    // Dynamic Range
    if (comboDynamicRange->isEnabled())
        connect(comboDynamicRange, SIGNAL(activated(int)), this,
                SLOT(SetDynamicRange(int)));

    // Threshold
    // m3
    if (btnSetThreshold->isEnabled()) {
        connect(btnSetThreshold, SIGNAL(clicked()), this,
                SLOT(SetThresholdEnergies()));
    }
    // eiger
    else if (spinThreshold->isEnabled())
        connect(spinThreshold, SIGNAL(valueChanged(int)), this,
                SLOT(SetThresholdEnergy(int)));

    // counters
    if (lblCounter->isEnabled()) {
        connect(chkCounter1, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
        connect(chkCounter2, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
        connect(chkCounter3, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
    }
}

void qTabSettings::GetHighVoltage() {
    // not enabled for eiger
    if (!comboHV->isVisible() && !spinHV->isVisible())
        return;
    LOG(logDEBUG) << "Getting High Voltage";
    disconnect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));
    disconnect(comboHV, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetHighVoltage()));
    try {
        Result<int> retvals = det->getHighVoltage();

        int retval = 0;
        if (det->getDetectorType().squash() != slsDetectorDefs::EIGER) {
            retval = retvals.tsquash("Inconsistent values for high voltage.");
        }
        // eiger slaves return -999
        else {

            auto is_master = det->getMaster();
            Result<int> master_retvals;
            for (size_t i = 0; i != retvals.size(); ++i) {
                if (is_master[i]) {
                    master_retvals.push_back(retvals[i]);
                }
            }
            retval =
                master_retvals.tsquash("Inconsistent values for high voltage.");
        }

        // spinHV
        if (spinHV->isVisible()) {
            if (retval != 0 && retval < hvmin && retval > HV_MAX) {
                throw RuntimeError(std::string("Unknown High Voltage: ") +
                                   std::to_string(retval));
            }
            spinHV->setValue(retval);
        }
        // combo HV
        else {
            switch (retval) {
            case 0:
                comboHV->setCurrentIndex(HV_0);
                break;
            case 90:
                comboHV->setCurrentIndex(HV_90);
                break;
            case 110:
                comboHV->setCurrentIndex(HV_110);
                break;
            case 120:
                comboHV->setCurrentIndex(HV_120);
                break;
            case 150:
                comboHV->setCurrentIndex(HV_150);
                break;
            case 180:
                comboHV->setCurrentIndex(HV_180);
                break;
            case 200:
                comboHV->setCurrentIndex(HV_200);
                break;
            default:
                throw RuntimeError(std::string("Unknown High Voltage: ") +
                                   std::to_string(retval));
            }
        }
    }
    CATCH_DISPLAY("Could not get high voltage.", "qTabSettings::GetHighVoltage")
    connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));
    connect(comboHV, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetHighVoltage()));
}

void qTabSettings::SetHighVoltage() {
    int val = (comboHV->isVisible() ? comboHV->currentText().toInt()
                                    : spinHV->value());
    LOG(logINFO) << "Setting high voltage:" << val;

    try {
        det->setHighVoltage(val);
    }
    CATCH_HANDLE("Could not set high voltage.", "qTabSettings::SetHighVoltage",
                 this, &qTabSettings::GetHighVoltage)
}

void qTabSettings::GetSettings() {
    LOG(logDEBUG) << "Getting settings";
    disconnect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetSettings(int)));
    try {
        auto retval = det->getSettings().tsquash(
            "Inconsistent settings for all detectors.");
        switch (retval) {
        case slsDetectorDefs::UNDEFINED:
            comboSettings->setCurrentIndex(UNDEFINED);
            break;
        case slsDetectorDefs::UNINITIALIZED:
            comboSettings->setCurrentIndex(UNINITIALIZED);
            break;
        default:
            if ((int)retval < -1 || (int)retval >= comboSettings->count()) {
                throw RuntimeError(std::string("Unknown settings: ") +
                                   std::to_string(retval));
            }
            comboSettings->setCurrentIndex(retval);
            break;
        }
    }
    CATCH_DISPLAY("Could not get settings.", "qTabSettings::GetSettings")
    connect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSettings(int)));
}

void qTabSettings::SetSettings(int index) {
    // settings
    auto val = static_cast<slsDetectorDefs::detectorSettings>(index);
    try {
        LOG(logINFO) << "Setting Settings to " << ToString(val);
        det->setSettings(val);
    }
    CATCH_HANDLE("Could not set settings.", "qTabSettings::SetSettings", this,
                 &qTabSettings::GetSettings)
    // threshold
    if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
        SetThresholdEnergy(spinThreshold->value());
    }
}

void qTabSettings::GetGainMode() {
    LOG(logDEBUG) << "Getting gain mode";
    disconnect(comboGainMode, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetGainMode(int)));
    try {
        auto retval = det->getGainMode().tsquash(
            "Inconsistent gain mode for all detectors.");
        if ((int)retval < 0 || (int)retval >= comboGainMode->count()) {
            throw RuntimeError(std::string("Unknown gain mode: ") +
                               std::to_string(retval));
        }
        // warning when using fix_g0 and not in export mode
        if ((int)retval == FIX_G0 && !isVisibleFixG0) {
            std::string message =
                "<nobr>You are not in Expert Mode and Gain Mode is in FIX_G0. "
                "</nobr><br><nobr>Could damage the detector when used without "
                "caution! </nobr>";
            qDefs::Message(qDefs::WARNING, message,
                           "qTabSettings::GetGainMode");
            LOG(logWARNING) << message;
        }
        comboGainMode->setCurrentIndex((int)retval);
    }
    CATCH_DISPLAY("Could not get gain mode.", "qTabSettings::GetGainMode")
    connect(comboGainMode, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetGainMode(int)));
}

void qTabSettings::SetGainMode(int index) {
    // warning for fix_G0 even in export mode
    if (index == FIX_G0) {
        if (qDefs::Message(
                qDefs::QUESTION,
                "<nobr>You are in Export mode, "
                "</nobr><br><nobr>but setting Gain Mode to FIX_G0 could "
                "damage the detector! </nobr><br><nobr>Proceed and set "
                "gainmode to FIX_G0 anyway?</nobr>",
                "qTabSettings::SetGainMode") == slsDetectorDefs::FAIL) {
            GetGainMode();
            return;
        }
    }

    LOG(logINFO) << "Setting Gain Mode to "
                 << comboGainMode->currentText().toLatin1().data();
    auto val = static_cast<slsDetectorDefs::gainMode>(index);
    try {

        det->setGainMode(val);
    }
    CATCH_HANDLE("Could not set gain mode.", "qTabSettings::SetGainMode", this,
                 &qTabSettings::GetGainMode)
}

void qTabSettings::GetDynamicRange() {
    LOG(logDEBUG) << "Getting dynamic range";
    disconnect(comboDynamicRange, SIGNAL(activated(int)), this,
               SLOT(SetDynamicRange(int)));
    try {
        auto retval = det->getDynamicRange().tsquash(
            "Inconsistent dynamic range for all detectors.");
        // set the final value on gui
        switch (retval) {
        case 32:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_32);
            break;
        case 16:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_16);
            break;
        case 12:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_12);
            break;
        case 8:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_8);
            break;
        case 4:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_4);
            break;
        default:
            throw RuntimeError(std::string("Unknown dynamic range: ") +
                               std::to_string(retval));
        }
    }
    CATCH_DISPLAY("Could not get dynamic range.",
                  "qTabSettings::GetDynamicRange")
    connect(comboDynamicRange, SIGNAL(activated(int)), this,
            SLOT(SetDynamicRange(int)));
}

void qTabSettings::SetDynamicRange(int index) {
    LOG(logINFO) << "Setting dynamic range to "
                 << comboDynamicRange->currentText().toLatin1().data();
    try {
        switch (index) {
        case DYNAMICRANGE_32:
            det->setDynamicRange(32);
            break;
        case DYNAMICRANGE_16:
            det->setDynamicRange(16);
            break;
        case DYNAMICRANGE_12:
            det->setDynamicRange(12);
            break;
        case DYNAMICRANGE_8:
            det->setDynamicRange(8);
            break;
        case DYNAMICRANGE_4:
            det->setDynamicRange(4);
            break;
        default:
            throw RuntimeError(std::string("Unknown dynamic range: ") +
                               std::to_string(index));
        }
    }
    CATCH_HANDLE("Could not set dynamic range.",
                 "qTabSettings::SetDynamicRange", this,
                 &qTabSettings::GetDynamicRange)
}

void qTabSettings::GetThresholdEnergies() {
    LOG(logDEBUG) << "Getting theshold energies";
    disconnect(btnSetThreshold, SIGNAL(clicked()), this,
               SLOT(SetThresholdEnergies()));
    try {
        auto retval = det->getAllThresholdEnergy().tsquash(
            "Inconsistent threhsold energies for all detectors.");
        spinThreshold->setValue(retval[0]);
        spinThreshold2->setValue(retval[1]);
        spinThreshold3->setValue(retval[2]);
    }
    CATCH_DISPLAY("Could not get threshold energy.",
                  "qTabSettings::GetThresholdEnergies")
    connect(btnSetThreshold, SIGNAL(clicked()), this,
            SLOT(SetThresholdEnergies()));
}

void qTabSettings::GetThresholdEnergy() {
    LOG(logDEBUG) << "Getting theshold energy";
    disconnect(spinThreshold, SIGNAL(valueChanged(int)), this,
               SLOT(SetThresholdEnergy(int)));
    try {
        auto retval = det->getThresholdEnergy().tsquash(
            "Inconsistent threhsold energy for all detectors.");
        spinThreshold->setValue(retval);
    }
    CATCH_DISPLAY("Could not get threshold energy.",
                  "qTabSettings::GetThresholdEnergy")
    connect(spinThreshold, SIGNAL(valueChanged(int)), this,
            SLOT(SetThresholdEnergy(int)));
}

void qTabSettings::SetThresholdEnergies() {
    std::array<int, 3> eV = {spinThreshold->value(), spinThreshold2->value(),
                             spinThreshold3->value()};
    slsDetectorDefs::detectorSettings sett =
        static_cast<slsDetectorDefs::detectorSettings>(
            comboSettings->currentIndex());
    LOG(logINFO) << "Setting Threshold Energies to " << ToString(eV) << " (eV)";
    try {
        det->setThresholdEnergy(eV, sett);
    }
    CATCH_DISPLAY("Could not get threshold energies.",
                  "qTabSettings::SetThresholdEnergies")
    // set the right value anyway (due to tolerance)
    GetThresholdEnergies();
}

void qTabSettings::SetThresholdEnergy(int index) {
    LOG(logINFO) << "Setting Threshold Energy to " << index << " eV";
    try {
        det->setThresholdEnergy(index);
    }
    CATCH_DISPLAY("Could not get threshold energy.",
                  "qTabSettings::SetThresholdEnergy")
    // set the right value anyway (due to tolerance)
    GetThresholdEnergy();
}

void qTabSettings::GetCounterMask() {
    LOG(logDEBUG) << "Getting counter mask";
    disconnect(chkCounter1, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    disconnect(chkCounter2, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    disconnect(chkCounter3, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    try {
        auto retval = getSetBits(det->getCounterMask().tsquash(
            "Counter mask is inconsistent for all detectors."));
        // default to unchecked
        for (auto p : counters) {
            p->setChecked(false);
        }
        // if retval[i] = 2, chkCounter2 is checked
        for (auto i : retval) {
            if (i > 3) {
                throw RuntimeError(std::string("Unknown counter index : ") +
                                   std::to_string(static_cast<int>(i)));
            }
            counters[i]->setChecked(true);
        }
    }
    CATCH_DISPLAY("Could not get counter mask.", "qTabSettings::GetCounterMask")
    connect(chkCounter1, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
    connect(chkCounter2, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
    connect(chkCounter3, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
}

void qTabSettings::SetCounterMask() {
    uint32_t mask = 0;
    for (unsigned int i = 0; i < counters.size(); ++i) {
        if (counters[i]->isChecked()) {
            mask |= (1 << i);
        }
    }
    LOG(logINFO) << "Setting counter mask to " << mask;
    try {
        det->setCounterMask(mask);
    }
    CATCH_HANDLE("Could not set counter mask.", "qTabSettings::SetCounterMask",
                 this, &qTabSettings::GetCounterMask)
}

void qTabSettings::Refresh() {
    LOG(logDEBUG) << "**Updating Settings Tab";

    GetHighVoltage();

    if (comboSettings->isEnabled()) {
        GetSettings();
    }

    if (comboGainMode->isEnabled()) {
        GetGainMode();
    }

    if (comboDynamicRange->isEnabled()) {
        GetDynamicRange();
    }

    // m3
    if (btnSetThreshold->isEnabled())
        GetThresholdEnergies();
    // eiger
    else if (spinThreshold->isEnabled()) {
        GetThresholdEnergy();
    }

    if (lblCounter->isEnabled()) {
        GetCounterMask();
    }

    LOG(logDEBUG) << "**Updated Settings Tab";
}

} // namespace sls

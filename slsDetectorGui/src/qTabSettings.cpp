#include "qTabSettings.h"
#include "qDefs.h"
#include "sls/ToString.h"
#include <QStandardItemModel>

qTabSettings::qTabSettings(QWidget *parent, sls::Detector *detector)
    : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Settings ready";
}

qTabSettings::~qTabSettings() {}

void qTabSettings::SetupWidgetWindow() {

    spinThreshold2->hide();
    spinThreshold3->hide();
    btnSetThreshold->hide();
    btnSetThreshold->setEnabled(false);
    // enabling according to det type
    slsDetectorDefs::detectorType detType = det->getDetectorType().squash();
    if (detType == slsDetectorDefs::MYTHEN3) {
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
        // disable dr
        QStandardItemModel *model =
            qobject_cast<QStandardItemModel *>(comboDynamicRange->model());
        if (model) {
            QModelIndex index;
            QStandardItem *item;
            index =
                model->index(DYNAMICRANGE_4, comboDynamicRange->modelColumn(),
                             comboDynamicRange->rootModelIndex());
            item = model->itemFromIndex(index);
            item->setEnabled(false);
        }
    } else if (detType == slsDetectorDefs::EIGER) {
        lblDynamicRange->setEnabled(true);
        comboDynamicRange->setEnabled(true);
        lblThreshold->setEnabled(true);
        spinThreshold->setEnabled(true);
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
    Initialization();
    // default for the disabled
    GetDynamicRange();
    Refresh();
}

void qTabSettings::SetupDetectorSettings() {
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(comboSettings->model());
    if (model) {
        QModelIndex index[NUMSETTINGS];
        QStandardItem *item[NUMSETTINGS];
        for (int i = 0; i < NUMSETTINGS; ++i) {
            index[i] = model->index(i, comboSettings->modelColumn(),
                                    comboSettings->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
            item[i]->setEnabled(false);
        }
        switch (det->getDetectorType().squash()) {
        case slsDetectorDefs::EIGER:
            item[(int)STANDARD]->setEnabled(true);
            item[(int)HIGHGAIN]->setEnabled(true);
            item[(int)LOWGAIN]->setEnabled(true);
            item[(int)VERYHIGHGAIN]->setEnabled(true);
            item[(int)VERLOWGAIN]->setEnabled(true);
            break;
        case slsDetectorDefs::GOTTHARD:
            item[(int)HIGHGAIN]->setEnabled(true);
            item[(int)DYNAMICGAIN]->setEnabled(true);
            item[(int)LOWGAIN]->setEnabled(true);
            item[(int)MEDIUMGAIN]->setEnabled(true);
            item[(int)VERYHIGHGAIN]->setEnabled(true);
            break;
        case slsDetectorDefs::JUNGFRAU:
            item[(int)DYNAMICGAIN]->setEnabled(true);
            item[(int)DYNAMICHG0]->setEnabled(true);
            item[(int)FIXGAIN1]->setEnabled(true);
            item[(int)FIXGAIN2]->setEnabled(true);
            item[(int)FORCESWITCHG1]->setEnabled(true);
            item[(int)FORCESWITCHG2]->setEnabled(true);
            break;
        case slsDetectorDefs::GOTTHARD2:
            item[(int)DYNAMICGAIN]->setEnabled(true);
            item[(int)FIXGAIN1]->setEnabled(true);
            item[(int)FIXGAIN2]->setEnabled(true);
            break;
        case slsDetectorDefs::MOENCH:
            item[(int)G1_HIGHGAIN]->setEnabled(true);
            item[(int)G1_LOWGAIN]->setEnabled(true);
            item[(int)G2_HIGHCAP_HIGHGAIN]->setEnabled(true);
            item[(int)G2_HIGHCAP_LOWGAIN]->setEnabled(true);
            item[(int)G2_LOWCAP_HIGHGAIN]->setEnabled(true);
            item[(int)G2_LOWCAP_LOWGAIN]->setEnabled(true);
            item[(int)G4_HIGHGAIN]->setEnabled(true);
            item[(int)G4_LOWGAIN]->setEnabled(true);
            break;
        case slsDetectorDefs::MYTHEN3:
            item[(int)STANDARD]->setEnabled(true);
            item[(int)FAST]->setEnabled(true);
            item[(int)HIGHGAIN]->setEnabled(true);
            break;
        default:
            LOG(logDEBUG) << "Unknown detector type. Exiting GUI.";
            qDefs::Message(qDefs::CRITICAL,
                           "Unknown detector type. Exiting GUI.",
                           "qTabSettings::SetupDetectorSettings");
            exit(-1);
        }
    }
}

void qTabSettings::Initialization() {
    // Settings
    if (comboSettings->isEnabled())
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSettings(int)));

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
            if ((int)retval < -1 || (int)retval >= NUMSETTINGS) {
                throw sls::RuntimeError(std::string("Unknown settings: ") +
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
        LOG(logINFO) << "Setting Settings to " << sls::ToString(val);
        det->setSettings(val);
    }
    CATCH_HANDLE("Could not set settings.", "qTabSettings::SetSettings", this,
                 &qTabSettings::GetSettings)
    // threshold
    if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
        SetThresholdEnergy(spinThreshold->value());
    }
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
        case 8:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_8);
            break;
        case 4:
            comboDynamicRange->setCurrentIndex(DYNAMICRANGE_4);
            break;
        default:
            throw sls::RuntimeError(std::string("Unknown dynamic range: ") +
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
                 << comboDynamicRange->currentText().toAscii().data();
    try {
        switch (index) {
        case DYNAMICRANGE_32:
            det->setDynamicRange(32);
            break;
        case DYNAMICRANGE_16:
            det->setDynamicRange(16);
            break;
        case DYNAMICRANGE_8:
            det->setDynamicRange(8);
            break;
        case DYNAMICRANGE_4:
            det->setDynamicRange(4);
            break;
        default:
            throw sls::RuntimeError(std::string("Unknown dynamic range: ") +
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
                  "qTabDataOutput::GetThresholdEnergies")
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
                  "qTabDataOutput::GetThresholdEnergy")
    connect(spinThreshold, SIGNAL(valueChanged(int)), this,
            SLOT(SetThresholdEnergy(int)));
}

void qTabSettings::SetThresholdEnergies() {
    std::array<int, 3> eV = {spinThreshold->value(), spinThreshold2->value(),
                             spinThreshold3->value()};
    slsDetectorDefs::detectorSettings sett =
        static_cast<slsDetectorDefs::detectorSettings>(
            comboSettings->currentIndex());
    LOG(logINFO) << "Setting Threshold Energies to " << sls::ToString(eV)
                 << " (eV)";
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

void qTabSettings::Refresh() {
    LOG(logDEBUG) << "**Updating Settings Tab";

    if (comboSettings->isEnabled()) {
        GetSettings();
    }

    if (comboDynamicRange->isEnabled()) {
        GetDynamicRange();
    }

    // m3
    if (btnSetThreshold->isEnabled())
        GetThresholdEnergies();
    // eiger
    else if (spinThreshold->isEnabled()) {
        LOG(logINFOBLUE) << "calling it!";
        GetThresholdEnergy();
    }

    LOG(logDEBUG) << "**Updated Settings Tab";
}

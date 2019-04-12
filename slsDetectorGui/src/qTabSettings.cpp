#include "qTabSettings.h"

#include "multiSlsDetector.h"

#include <QStandardItemModel>

#include <cmath>
#include <iostream>

qTabSettings::qTabSettings(QWidget *parent, multiSlsDetector *detector)
    : QWidget(parent), myDet(detector), detType(slsDetectorDefs::GENERIC) {

    setupUi(this);
    SetupWidgetWindow();
    Initialization();
    FILE_LOG(logDEBUG) << "Settings ready";
}

qTabSettings::~qTabSettings() {}

void qTabSettings::SetupWidgetWindow() {
    // Detector Type
    detType = myDet->getDetectorTypeAsEnum();

    // Settings
    comboSettings->setCurrentIndex(UNINITIALIZED);
    if (detType == slsDetectorDefs::MOENCH) {
        lblSettings->setEnabled(false);
        comboSettings->setEnabled(false);
    } else {
        SetupDetectorSettings();
        GetSettings();
    }

    // Dynamic Range
    GetDynamicRange();
    // cannot change dr for other types
    if (detType != slsDetectorDefs::EIGER) {
        lblDynamicRange->setEnabled(false);
        comboDynamicRange->setEnabled(false);
    }

    // threshold energy
    if (detType == slsDetectorDefs::EIGER) {
			qDefs::IgnoreNonCriticalExceptions<QSpinBox>(
                spinThreshold,
                &QSpinBox::setValue,
                myDet,
                std::string("qTabSettings::SetupWidgetWindow"),
                &multiSlsDetector::getThresholdEnergy, -1);
    } else {
        lblThreshold->setEnabled(false);
        spinThreshold->setEnabled(false);
    }
}

void qTabSettings::SetupDetectorSettings() {

    // To be able to index items on a combo box
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(comboSettings->model());
    if (model) {
        QModelIndex index[NUMSETTINGS];
        QStandardItem *item[NUMSETTINGS];
        for (int i = 0; i < NUMSETTINGS; ++i) {
            index[i] = model->index(i, comboSettings->modelColumn(),
                                    comboSettings->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
        }

        item[(int)UNDEFINED]->setEnabled(false);
        item[(int)UNINITIALIZED]->setEnabled(false);

        switch (detType) {
        case slsDetectorDefs::EIGER:
            item[(int)STANDARD]->setEnabled(true);
            item[(int)HIGHGAIN]->setEnabled(true);
            item[(int)LOWGAIN]->setEnabled(true);
            item[(int)VERYHIGHGAIN]->setEnabled(true);
            item[(int)VERLOWGAIN]->setEnabled(true);

            item[(int)FAST]->setEnabled(false);
            item[(int)DYNAMICGAIN]->setEnabled(false);
            item[(int)MEDIUMGAIN]->setEnabled(false);
            item[(int)LOWNOISE]->setEnabled(false);
            item[(int)DYNAMICHG0]->setEnabled(false);
            item[(int)FIXGAIN1]->setEnabled(false);
            item[(int)FIXGAIN2]->setEnabled(false);
            item[(int)FORCESWITCHG1]->setEnabled(false);
            item[(int)FORCESWITCHG2]->setEnabled(false);
            break;

        case slsDetectorDefs::GOTTHARD:
            item[(int)HIGHGAIN]->setEnabled(true);
            item[(int)DYNAMICGAIN]->setEnabled(true);
            item[(int)LOWGAIN]->setEnabled(true);
            item[(int)MEDIUMGAIN]->setEnabled(true);
            item[(int)VERYHIGHGAIN]->setEnabled(true);

            item[(int)STANDARD]->setEnabled(false);
            item[(int)FAST]->setEnabled(false);
            item[(int)LOWNOISE]->setEnabled(false);
            item[(int)DYNAMICHG0]->setEnabled(false);
            item[(int)FIXGAIN1]->setEnabled(false);
            item[(int)FIXGAIN2]->setEnabled(false);
            item[(int)FORCESWITCHG1]->setEnabled(false);
            item[(int)FORCESWITCHG2]->setEnabled(false);
            item[(int)VERLOWGAIN]->setEnabled(false);
            break;

        case slsDetectorDefs::JUNGFRAU:
            item[(int)DYNAMICGAIN]->setEnabled(true);
            item[(int)DYNAMICHG0]->setEnabled(true);
            item[(int)FIXGAIN1]->setEnabled(true);
            item[(int)FIXGAIN2]->setEnabled(true);
            item[(int)FORCESWITCHG1]->setEnabled(true);
            item[(int)FORCESWITCHG2]->setEnabled(true);

            item[(int)STANDARD]->setEnabled(false);
            item[(int)FAST]->setEnabled(false);
            item[(int)HIGHGAIN]->setEnabled(false);
            item[(int)LOWGAIN]->setEnabled(false);
            item[(int)MEDIUMGAIN]->setEnabled(false);
            item[(int)VERYHIGHGAIN]->setEnabled(false);
            item[(int)LOWNOISE]->setEnabled(false);
            item[(int)VERLOWGAIN]->setEnabled(false);
            break;

        default:
            FILE_LOG(logDEBUG) << "Unknown detector type. Exiting GUI.";
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
    if (spinThreshold->isEnabled())
        connect(spinThreshold, SIGNAL(valueChanged(int)), this,
                SLOT(SetEnergy()));
}

void qTabSettings::GetSettings() {
    FILE_LOG(logDEBUG) << "Getting settings";

    int sett = qDefs::IgnoreNonCriticalExceptionsandReturn(
        myDet, std::string("qTabSettings::SetupWidgetWindow"),
        &multiSlsDetector::getSettings, -1);

    FILE_LOG(logDEBUG) << "Settings from Detector:" << sett;

    if (sett == -1)
        sett = UNDEFINED;
    if (sett == slsDetectorDefs::UNDEFINED)
        sett = UNDEFINED;
    else if (sett == slsDetectorDefs::UNINITIALIZED)
        sett = UNINITIALIZED;

    comboSettings->setCurrentIndex(sett);
}

void qTabSettings::GetDynamicRange() {
    FILE_LOG(logDEBUG) << "Getting dynamic range";

    int ret = qDefs::IgnoreNonCriticalExceptionsandReturn(
        myDet, std::string("qTabSettings::GetDynamicRange"),
        &multiSlsDetector::setDynamicRange, -1, -1);

    // set the final value on gui
    switch (ret) {
    case 32:
        comboDynamicRange->setCurrentIndex(0);
        break;
    case 16:
        comboDynamicRange->setCurrentIndex(1);
        break;
    case 8:
        comboDynamicRange->setCurrentIndex(2);
        break;
    case 4:
        comboDynamicRange->setCurrentIndex(3);
        break;
    default:
        if (ret != -1) {
            qDefs::Message(qDefs::WARNING,
                       "Unknown Dyanmic Range " + std::to_string(ret) + ".",
                       "qTabSettings::SetupDetectorSettings");
        }
        break;
    }
}

void qTabSettings::SetSettings(int index) {
    slsDetectorDefs::detectorSettings sett =
        myDet->setSettings((slsDetectorDefs::detectorSettings)index);
    FILE_LOG(logINFO) << "Settings set to "
                      << myDet->slsDetectorDefs::getDetectorSettings(sett);

    // threshold
    if (spinThreshold->isEnabled()) {
        SetEnergy();
    }

    qDefs::checkErrorMessage(myDet, "qTabSettings::SetSettings");
}

void qTabSettings::SetDynamicRange(int index) {
    int dr;
    switch (index) {
    case 0:
        dr = 32;
        break;
    case 1:
        dr = 16;
        break;
    case 2:
        dr = 8;
        break;
    case 3:
        dr = 4;
        break;
    default:
        break;
    }
    int ret = myDet->setDynamicRange(dr);
    FILE_LOG(logINFO) << "Setting dynamic range to " << dr;
    qDefs::checkErrorMessage(myDet, "qTabSettings::SetDynamicRange");

    // check
    if (ret != dr) {
        qDefs::Message(qDefs::WARNING, "Could not set dynamic range.",
                       "qTabSettings::SetDynamicRange");
        disconnect(comboDynamicRange, SIGNAL(activated(int)), this,
                   SLOT(SetDynamicRange(int)));
        GetDynamicRange();
        connect(comboDynamicRange, SIGNAL(activated(int)), this,
                SLOT(SetDynamicRange(int)));
    }
}

void qTabSettings::SetEnergy() {
    int index = spinThreshold->value();
    FILE_LOG(logINFO) << "Settings threshold energy to " << index;

    myDet->setThresholdEnergy(index);
    int ret = myDet->getThresholdEnergy();
    if ((ret - index) > 200) {
        qDefs::Message(qDefs::WARNING,
                       "Threshold energy could not be set (tolerance 200).",
                       "qTabSettings::SetEnergy");
    }
    disconnect(spinThreshold, SIGNAL(valueChanged(int)), this,
               SLOT(SetEnergy()));
    spinThreshold->setValue(ret);
    connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));

    qDefs::checkErrorMessage(myDet, "qTabSettings::SetEnergy");
}

void qTabSettings::Refresh() {
    FILE_LOG(logDEBUG) << "\n**Updating Settings Tab";

    // settings
    if (comboSettings->isEnabled()) {
        disconnect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
                   SLOT(SetSettings(int)));
        GetSettings();
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSettings(int)));
    }

    // threshold
    if (spinThreshold->isEnabled()) {
        disconnect(spinThreshold, SIGNAL(valueChanged(int)), this,
                   SLOT(SetEnergy()));
        spinThreshold->setValue(myDet->getThresholdEnergy());
        connect(spinThreshold, SIGNAL(valueChanged(int)), this,
                SLOT(SetEnergy()));
    }

    // Dynamic Range
    if (comboDynamicRange->isEnabled()) {
        disconnect(comboDynamicRange, SIGNAL(activated(int)), this,
                   SLOT(SetDynamicRange(int)));
        GetDynamicRange();
        connect(comboDynamicRange, SIGNAL(activated(int)), this,
                SLOT(SetDynamicRange(int)));
    }

    FILE_LOG(logDEBUG) << "**Updated Settings Tab";

    qDefs::checkErrorMessage(myDet, "qTabSettings::Refresh");
}

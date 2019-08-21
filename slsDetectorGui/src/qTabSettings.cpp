#include "qTabSettings.h"
#include "qDefs.h"

#include <QStandardItemModel>

#include <cmath>
#include <iostream>

qTabSettings::qTabSettings(QWidget *parent, sls::Detector *detector): QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Settings ready";
}

qTabSettings::~qTabSettings() {}

void qTabSettings::SetupWidgetWindow() {

	// enabling according to det type
    switch(det->getDetectorTypeAsEnum()) {
        case slsDetectorDefs::MOENCH:
            lblSettings->setEnabled(false);
            comboSettings->setEnabled(false);
            break;
        case slsDetectorDefs::EIGER:
            lblDynamicRange->setEnabled(true);
            comboDynamicRange->setEnabled(true);
            lblThreshold->setEnabled(true);
            spinThreshold->setEnabled(true);
            break;
        default:
            break;
    }

    // default settings for the disabled
    comboSettings->setCurrentIndex(UNINITIALIZED);
    if (comboSettings->isEnabled()) {
        SetupDetectorSettings();
    }
    spinThreshold->setValue(-1);

    Initialization();
    
    // default for the disabled
    GetDynamicRange();

    Refresh();
}

void qTabSettings::SetupDetectorSettings() {
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(comboSettings->model());
    if (model) {
        QModelIndex index[NUMSETTINGS];
        QStandardItem *item[NUMSETTINGS];
        for (int i = 0; i < NUMSETTINGS; ++i) {
            index[i] = model->index(i, comboSettings->modelColumn(), comboSettings->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
            item[i]->setEnabled(false); 
        }
        switch (det->getDetectorTypeAsEnum()) {
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
        default:
            FILE_LOG(logDEBUG) << "Unknown detector type. Exiting GUI.";
            qDefs::Message(qDefs::CRITICAL,
                           "Unknown detector type. Exiting GUI.", "qTabSettings::SetupDetectorSettings");
            exit(-1);
        }
    }
}

void qTabSettings::Initialization() {
    // Settings
    if (comboSettings->isEnabled())
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSettings(int)));
   
    // Dynamic Range
    if (comboDynamicRange->isEnabled())
        connect(comboDynamicRange, SIGNAL(activated(int)), this, SLOT(SetDynamicRange(int)));

    // Threshold
    if (spinThreshold->isEnabled())
        connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetThresholdEnergy(int)));
}

void qTabSettings::GetSettings() {
    FILE_LOG(logDEBUG) << "Getting settings";
    disconnect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSettings(int)));

    try{
        auto retval = det->getSettings(-1);
        switch (retval) {
            case -1:
                qDefs::Message(qDefs::WARNING, "Settings are inconsistent for all detectors.", "qTabSettings::GetSettings");
                break;
            case slsDetectorDefs::UNDEFINED:
                comboSettings->setCurrentIndex(UNDEFINED);
                break;
            case slsDetectorDefs::UNINITIALIZED:
                comboSettings->setCurrentIndex(UNINITIALIZED);
                break;
            default:
                if ((int)retval < -1 || (int)retval >= NUMSETTINGS) {
                    qDefs::Message(qDefs::WARNING, std::string("Unknown settings: ") + std::to_string(retval), "qTabSettings::GetSettings");
                } else {
                    comboSettings->setCurrentIndex(retval);
                }
                break;
        }
    } CATCH_DISPLAY ("Could not get settings.", "qTabSettings::GetSettings")

    connect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSettings(int)));
}

void qTabSettings::SetSettings(int index) {
    // settings
    auto val = static_cast<slsDetectorDefs::detectorSettings>(index);
    FILE_LOG(logINFO) << "Setting Settings to " << det->slsDetectorDefs::getDetectorSettings(val);

    try {
        det->setSettings(val);
    } CATCH_HANDLE ("Could not set settings.", "qTabSettings::SetSettings", this, &qTabSettings::GetSettings)

    // threshold
    if (spinThreshold->isEnabled()) {
        SetThresholdEnergy(spinThreshold->value());
    }
}

void qTabSettings::GetDynamicRange() {
    FILE_LOG(logDEBUG) << "Getting dynamic range";
    disconnect(comboDynamicRange, SIGNAL(activated(int)), this, SLOT(SetDynamicRange(int)));
 
    try {
        auto retval = det->setDynamicRange(-1);

        // set the final value on gui
        switch (retval) {
        case -1:
            qDefs::Message(qDefs::WARNING, "Dynamic Range is inconsistent for all detectors.", "qTabSettings::GetDynamicRange");
            break;
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
            qDefs::Message(qDefs::WARNING, std::string("Unknown dynamic range: ") + std::to_string(retval), "qTabSettings::GetDynamicRange");
            break;
        }
    } CATCH_DISPLAY ("Could not get dynamic range.", "qTabSettings::GetDynamicRange")

    connect(comboDynamicRange, SIGNAL(activated(int)), this,SLOT(SetDynamicRange(int))); 
}

void qTabSettings::SetDynamicRange(int index) {
    FILE_LOG(logINFO) << "Setting dynamic range to " << comboDynamicRange->currentText().toAscii().data();
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
            qDefs::Message(qDefs::WARNING, std::string("Unknown dynamic range: ") + std::to_string(index), "qTabSettings::SetDynamicRange");
           break;
        }
    } CATCH_HANDLE ("Could not set dynamic range.", "qTabSettings::SetDynamicRange", this, &qTabSettings::GetDynamicRange)
}

void qTabSettings::GetThresholdEnergy() {
    FILE_LOG(logDEBUG) << "Getting theshold energy";
    disconnect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetThresholdEnergy(int)));

    try {
        auto retval = det->getThresholdEnergy();
		/*if (retval == -1) { commenting out as default is -1, handle this when API changes
			qDefs::Message(qDefs::WARNING, "Threshold Energy is inconsistent for all detectors.", "qTabDataOutput::GetThresholdEnergy");
            spinThreshold->setValue(-1);
		} else {
			spinThreshold->setValue(retval);
		}*/
        spinThreshold->setValue(retval);
    } CATCH_DISPLAY ("Could not get threshold energy.", "qTabDataOutput::GetThresholdEnergy")

    connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetThresholdEnergy(int)));
}

void qTabSettings::SetThresholdEnergy(int index) {
    FILE_LOG(logINFO) << "Setting Threshold Energy to " << index << " eV";
    try {
        det->setThresholdEnergy(index);
    } CATCH_DISPLAY ("Could not get threshold energy.", "qTabSettings::SetThresholdEnergy")

    // set the right value anyway (due to tolerance)
    GetThresholdEnergy(); 
}

void qTabSettings::Refresh() {
    FILE_LOG(logDEBUG) << "**Updating Settings Tab";

    if (comboSettings->isEnabled()) {
        GetSettings();
    }

    if (comboDynamicRange->isEnabled()) {
        GetDynamicRange();
    }
    
    if (spinThreshold->isEnabled())
        GetThresholdEnergy();

    FILE_LOG(logDEBUG) << "**Updated Settings Tab";
}

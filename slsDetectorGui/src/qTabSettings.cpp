/*
 * qTabSettings.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabSettings.h"
// Project Class Headers
#include "multiSlsDetector.h"
#include "slsDetector.h"
// C++ Include Headers
#include <cmath>
#include <iostream>

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabSettings::qTabSettings(QWidget *parent, multiSlsDetector *&detector) : QWidget(parent), myDet(detector), expertMode(false) {

    for (int i = 0; i < NumSettings; i++)
        item[i] = 0;
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Settings ready";
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabSettings::~qTabSettings() {
    delete myDet;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetupWidgetWindow() {
    // Detector Type
    detType = myDet->getDetectorTypeAsEnum();

    // Settings
    if (detType != slsDetectorDefs::CHIPTESTBOARD) {
        SetupDetectorSettings();
    } else
        comboSettings->setEnabled(false);

    //threshold
    if (detType == slsDetectorDefs::EIGER) {
        spinThreshold->setValue(myDet->getThresholdEnergy());
    }

    //expert mode is not enabled initially
    lblThreshold->setEnabled(false);
    spinThreshold->setEnabled(false);

    Initialization();

    // Dynamic Range
    GetDynamicRange();

    qDefs::checkErrorMessage(myDet, "qTabSettings::SetupWidgetWindow");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::GetDynamicRange(int setvalue) {
#ifdef VERBOSE
    std::cout << "Getting dynamic range\n";
#endif
    int ret = myDet->setDynamicRange(-1);
    // if(detType == slsDetectorDefs::MYTHEN) {
    // 	if(ret==24)
    // 		ret=32;
    // 	else if(ret==24)
    // 		std::cout<<"ret:"<<ret<<'\n';
    // }
    //check if the set value is equal to return value
    if ((setvalue != -1) && (setvalue != ret)) {
        qDefs::Message(qDefs::WARNING, "Dynamic Range cannot be set to this value.", "qTabSettings::SetDynamicRange");
#ifdef VERBOSE
        std::cout << "ERROR: Setting dynamic range to " << ret << '\n';
#endif
    }

    //set the final value on gui
    disconnect(comboDynamicRange, SIGNAL(activated(int)), this, SLOT(SetDynamicRange(int)));
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
        comboDynamicRange->setCurrentIndex(0);
        break;
    }
    connect(comboDynamicRange, SIGNAL(activated(int)), this, SLOT(SetDynamicRange(int)));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetupDetectorSettings() {
    // Get detector settings from detector
    int sett = (int)myDet->getSettings();
    std::cout << "sett:" << sett << '\n';
    qDefs::checkErrorMessage(myDet, "qTabSettings::SetupDetectorSettings");
    if (sett == -1)
        sett = Undefined;
    if (sett == slsDetectorDefs::UNDEFINED)
        sett = Undefined;
    else if (sett == slsDetectorDefs::UNINITIALIZED)
        sett = Uninitialized;
    // To be able to index items on a combo box
    model = qobject_cast<QStandardItemModel *>(comboSettings->model());
    if (model) {
        for (int i = 0; i < NumSettings; i++) {
            index[i] = model->index(i, comboSettings->modelColumn(), comboSettings->rootModelIndex());
            item[i] = model->itemFromIndex(index[i]);
        }

        switch (detType) {
        // case slsDetectorDefs::MYTHEN:
        // 	item[(int)Standard]->setEnabled(true);
        // 	item[(int)Fast]->setEnabled(true);
        // 	item[(int)HighGain]->setEnabled(true);
        // 	item[(int)DynamicGain]->setEnabled(false);
        // 	item[(int)LowGain]->setEnabled(false);
        // 	item[(int)MediumGain]->setEnabled(false);
        // 	item[(int)VeryHighGain]->setEnabled(false);
        // 	item[(int)LowNoise]->setEnabled(false);
        // 	item[(int)DynamicHG0]->setEnabled(false);
        // 	item[(int)FixGain1]->setEnabled(false);
        // 	item[(int)FixGain2]->setEnabled(false);
        // 	item[(int)ForceSwitchG1]->setEnabled(false);
        // 	item[(int)ForceSwitchG2]->setEnabled(false);
        // 	item[(int)VeryLowGain]->setEnabled(false);
        // 	break;
        case slsDetectorDefs::EIGER:
            item[(int)Standard]->setEnabled(true);
            item[(int)Fast]->setEnabled(false);
            item[(int)HighGain]->setEnabled(true);
            item[(int)DynamicGain]->setEnabled(false);
            item[(int)LowGain]->setEnabled(true);
            item[(int)MediumGain]->setEnabled(false);
            item[(int)VeryHighGain]->setEnabled(true);
            item[(int)LowNoise]->setEnabled(false);
            item[(int)DynamicHG0]->setEnabled(false);
            item[(int)FixGain1]->setEnabled(false);
            item[(int)FixGain2]->setEnabled(false);
            item[(int)ForceSwitchG1]->setEnabled(false);
            item[(int)ForceSwitchG2]->setEnabled(false);
            item[(int)VeryLowGain]->setEnabled(true);
            break;
        case slsDetectorDefs::MOENCH:
        case slsDetectorDefs::GOTTHARD:
            item[(int)Standard]->setEnabled(false);
            item[(int)Fast]->setEnabled(false);
            item[(int)HighGain]->setEnabled(true);
            item[(int)DynamicGain]->setEnabled(true);
            item[(int)LowGain]->setEnabled(true);
            item[(int)MediumGain]->setEnabled(true);
            item[(int)VeryHighGain]->setEnabled(true);
            item[(int)LowNoise]->setEnabled(false);
            item[(int)DynamicHG0]->setEnabled(false);
            item[(int)FixGain1]->setEnabled(false);
            item[(int)FixGain2]->setEnabled(false);
            item[(int)ForceSwitchG1]->setEnabled(false);
            item[(int)ForceSwitchG2]->setEnabled(false);
            item[(int)VeryLowGain]->setEnabled(false);
            break;
        case slsDetectorDefs::JUNGFRAU:
            item[(int)Standard]->setEnabled(false);
            item[(int)Fast]->setEnabled(false);
            item[(int)HighGain]->setEnabled(false);
            item[(int)DynamicGain]->setEnabled(true);
            item[(int)LowGain]->setEnabled(false);
            item[(int)MediumGain]->setEnabled(false);
            item[(int)VeryHighGain]->setEnabled(false);
            item[(int)LowNoise]->setEnabled(false);
            item[(int)DynamicHG0]->setEnabled(true);
            item[(int)FixGain1]->setEnabled(true);
            item[(int)FixGain2]->setEnabled(true);
            item[(int)ForceSwitchG1]->setEnabled(true);
            item[(int)ForceSwitchG2]->setEnabled(true);
            item[(int)VeryLowGain]->setEnabled(false);
            break;
        default:
            std::cout << "Unknown detector type. Exiting GUI.\n";
            qDefs::Message(qDefs::CRITICAL, "Unknown detector type. Exiting GUI.", "qTabSettings::SetupDetectorSettings");
            exit(-1);
            break;
        }
        // detector settings selected NOT ENABLED.
        // This should not happen -only if the server and gui has a mismatch
        // on which all modes are allowed in detectors
        if (!(item[sett]->isEnabled())) {
            qDefs::Message(qDefs::CRITICAL, "Unknown Detector Settings retrieved from detector. Exiting GUI.", "qTabSettings::SetupDetectorSettings");
#ifdef VERBOSE
            std::cout << "ERROR:  Unknown Detector Settings retrieved from detector.\n";
#endif
            sett = Undefined;
            //	exit(-1);
        }
        // Setting the detector settings
        else
            comboSettings->setCurrentIndex(sett);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::Initialization() {
    // Settings
    if (detType != slsDetectorDefs::CHIPTESTBOARD)
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(setSettings(int)));
    // Dynamic Range
    connect(comboDynamicRange, SIGNAL(activated(int)), this, SLOT(SetDynamicRange(int)));
    // Threshold
    connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::setSettings(int index) {
    //dont set it if settings is set to undefined or uninitialized
    if ((index == Undefined) || (index == Uninitialized)) {
        qDefs::Message(qDefs::WARNING, "Cannot change settings to Undefined or Uninitialized.", "qTabSettings::setSettings");
        disconnect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(setSettings(int)));
        int sett = (int)myDet->getSettings();
        if (sett == -1)
            sett = Undefined;
        if (sett == slsDetectorDefs::UNDEFINED)
            sett = Undefined;
        else if (sett == slsDetectorDefs::UNINITIALIZED)
            sett = Uninitialized;
        comboSettings->setCurrentIndex(sett);
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(setSettings(int)));
    }

    else {
        slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index);
#ifdef VERBOSE
        std::cout << "\nSettings have been set to " << myDet->slsDetectorDefs::getDetectorSettings(sett) << '\n';
#endif

        //threshold
        if (detType == slsDetectorDefs::EIGER) {
            lblThreshold->setEnabled(true);
            spinThreshold->setEnabled(true);
            SetEnergy();
            //also update trimbits plot
            if (expertMode)
                emit UpdateTrimbitSignal(0);
        }
    }

    qDefs::checkErrorMessage(myDet, "qTabSettings::setSettings");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

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
        dr = 32;
        break;
    }
    myDet->setDynamicRange(dr);
#ifdef VERBOSE
    std::cout << "Setting dynamic range to " << dr << '\n';
#endif
    //check
    GetDynamicRange(dr);
    qDefs::checkErrorMessage(myDet, "qTabSettings::SetDynamicRange");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetEnergy() {
    int index = spinThreshold->value();
#ifdef VERBOSE
    std::cout << "Settings threshold energy to " << index << '\n';
#endif
    myDet->setThresholdEnergy(index);
    int ret = (int)myDet->getThresholdEnergy();
    if ((ret - index) > 200) {
        qDefs::Message(qDefs::WARNING, "Threshold energy could not be set. The difference is greater than 200.", "qTabSettings::SetEnergy");
    }
    disconnect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));
    spinThreshold->setValue(ret);
    connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));

    qDefs::checkErrorMessage(myDet, "qTabSettings::SetEnergy");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::Refresh() {
#ifdef VERBOSE
    std::cout << "\n**Updating Settings Tab\n";
#endif

    if (detType != slsDetectorDefs::CHIPTESTBOARD)
        disconnect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(setSettings(int)));
    disconnect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));

    // Dynamic Range
    GetDynamicRange();

    // Settings
    if (detType != slsDetectorDefs::CHIPTESTBOARD) {
#ifdef VERBOSE
        std::cout << "Getting settings\n";
#endif
        int sett = (int)myDet->getSettings();
        if (sett == -1)
            sett = Undefined; //slsDetectorDefs::UNDEFINED;
        if (sett == slsDetectorDefs::UNDEFINED)
            sett = Undefined;
        else if (sett == slsDetectorDefs::UNINITIALIZED)
            sett = Uninitialized;
        comboSettings->setCurrentIndex(sett);

        //threshold
        sett = comboSettings->currentIndex();
        if (detType == slsDetectorDefs::EIGER) {
            if ((sett == Undefined) || (sett == Uninitialized)) {
                lblThreshold->setEnabled(false);
                spinThreshold->setEnabled(false);
            } else {
                lblThreshold->setEnabled(true);
                spinThreshold->setEnabled(true);
#ifdef VERBOSE
                std::cout << "Getting threshold energy\n";
#endif
                spinThreshold->setValue(myDet->getThresholdEnergy());
            }
        }
    }

    if (detType != slsDetectorDefs::CHIPTESTBOARD)
        connect(comboSettings, SIGNAL(currentIndexChanged(int)), this, SLOT(setSettings(int)));
    connect(spinThreshold, SIGNAL(valueChanged(int)), this, SLOT(SetEnergy()));

#ifdef VERBOSE
    std::cout << "**Updated Settings Tab\n\n";
#endif

    qDefs::checkErrorMessage(myDet, "qTabSettings::Refresh");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

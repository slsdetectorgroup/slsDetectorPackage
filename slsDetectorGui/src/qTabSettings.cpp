/*
 * qTabSettings.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabSettings.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// C++ Include Headers
#include<iostream>
#include <cmath>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabSettings::qTabSettings(QWidget *parent,multiSlsDetector*& detector):
		QWidget(parent),myDet(detector),expertMode(false){

	setupUi(this);
	SetupWidgetWindow();
	Initialization();

}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabSettings::~qTabSettings(){
	delete myDet;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetupWidgetWindow(){
	// Detector Type
	detType=myDet->getDetectorsType();

	// Settings
	SetupDetectorSettings();
	int sett = (int)myDet->getSettings();
	if(sett==-1) sett = slsDetectorDefs::UNDEFINED;
	comboSettings->setCurrentIndex(sett);

	//threshold
	spinThreshold->setValue(myDet->getThresholdEnergy());

	//expert mode is not enabled initially
	lblThreshold->setEnabled(false);
	spinThreshold->setEnabled(false);

	// Number of Modules
	spinNumModules->setMaximum(myDet->getMaxNumberOfModules());
	spinNumModules->setValue(myDet->setNumberOfModules());

	// Dynamic Range
	switch(myDet->setDynamicRange(-1)){
	case 32:   	comboDynamicRange->setCurrentIndex(0);	break;
	case 24:   	comboDynamicRange->setCurrentIndex(0);	break;
	case 16:	comboDynamicRange->setCurrentIndex(1);  break;
	case 8:	  	comboDynamicRange->setCurrentIndex(2);	break;
	case 4:	  	comboDynamicRange->setCurrentIndex(3);	break;
	default:	comboDynamicRange->setCurrentIndex(0);	break;
	}

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetupDetectorSettings(){
	// Get detector settings from detector
	slsDetectorDefs::detectorSettings sett = myDet->getSettings();
	qDefs::checkErrorMessage(myDet);
	if(sett==-1) sett = slsDetectorDefs::UNDEFINED;
	// To be able to index items on a combo box
	model = qobject_cast<QStandardItemModel*>(comboSettings->model());
	if (model) {
		for(int i=0;i<NumSettings;i++){
			index[i] = model->index(i,	comboSettings->modelColumn(), comboSettings->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}

		switch(detType){
		case slsDetectorDefs::MYTHEN:
			item[(int)Standard]->setEnabled(true);
			item[(int)Fast]->setEnabled(true);
			item[(int)HighGain]->setEnabled(true);
			item[(int)DynamicGain]->setEnabled(false);
			item[(int)LowGain]->setEnabled(false);
			item[(int)MediumGain]->setEnabled(false);
			item[(int)VeryHighGain]->setEnabled(false);
			break;
		case slsDetectorDefs::EIGER:
			item[(int)Standard]->setEnabled(false);
			item[(int)Fast]->setEnabled(false);
			item[(int)HighGain]->setEnabled(false);
			item[(int)DynamicGain]->setEnabled(false);
			item[(int)LowGain]->setEnabled(false);
			item[(int)MediumGain]->setEnabled(false);
			item[(int)VeryHighGain]->setEnabled(false);
			break;
		case slsDetectorDefs::GOTTHARD:
			item[(int)Standard]->setEnabled(false);
			item[(int)Fast]->setEnabled(false);
			item[(int)HighGain]->setEnabled(true);
			item[(int)DynamicGain]->setEnabled(true);
			item[(int)LowGain]->setEnabled(true);
			item[(int)MediumGain]->setEnabled(true);
			item[(int)VeryHighGain]->setEnabled(true);
			break;
		default:
			qDefs::Message(qDefs::CRITICAL,"Unknown detector type. Exiting GUI.","Settings");
			exit(-1);
			break;
		}
		// detector settings selected NOT ENABLED.
		// This should not happen -only if the server and gui has a mismatch
		// on which all modes are allowed in detectors
		if(!(item[(int)sett]->isEnabled())){
			qDefs::Message(qDefs::CRITICAL,"Unknown Detector Settings retrieved from detector. Exiting GUI.","Settings");
#ifdef VERBOSE
			cout << "ERROR:  Unknown Detector Settings retrieved from detector." << endl;
#endif
			exit(-1);
		}
		// Setting the detector settings
		else	comboSettings->setCurrentIndex((int)sett);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::Initialization(){
	// Settings
	connect(comboSettings, 		SIGNAL(currentIndexChanged(int)),	this, SLOT(setSettings(int)));
	// Number of Modules
	connect(spinNumModules, 	SIGNAL(valueChanged(int)), 			this, SLOT(SetNumberOfModules(int)));
	// Dynamic Range
	connect(comboDynamicRange, 	SIGNAL(activated(int)), 			this, SLOT(SetDynamicRange(int)));
	// Threshold
	connect(spinThreshold,		SIGNAL(valueChanged(int)),			this, SLOT(SetEnergy()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::setSettings(int index){
	//dont set it if settings is set to undefined or uninitialized
	if((index==Undefined)||(index==Uninitialized)){
		qDefs::Message(qDefs::WARNING,"Cannot change settings to Undefined or Uninitialized.","Settings");
		disconnect(comboSettings, 		SIGNAL(currentIndexChanged(int)),	this, SLOT(setSettings(int)));
		int sett = (int)myDet->getSettings();
		if(sett==-1) sett = slsDetectorDefs::UNDEFINED;
		comboSettings->setCurrentIndex(sett);
		connect(comboSettings, 		SIGNAL(currentIndexChanged(int)),	this, SLOT(setSettings(int)));
	}

	else{
		slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index);
	#ifdef VERBOSE
		cout << "Settings have been set to " << myDet->slsDetectorBase::getDetectorSettings(sett) << endl;
	#endif

		//threshold
		if((detType==slsDetectorDefs::MYTHEN)||(detType==slsDetectorDefs::EIGER)){
			lblThreshold->setEnabled(true);
			spinThreshold->setEnabled(true);
			SetEnergy();
			//also update trimbits plot
			if(expertMode)	emit UpdateTrimbitSignal(0);
		}
	}

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetNumberOfModules(int index){
#ifdef VERBOSE
	cout << "Setting number of modules to "<< index << endl;
#endif
	int i = myDet->setNumberOfModules(index);
	if(index!=i)
		qDefs::Message(qDefs::WARNING,"Number of modules cannot be set for this value.","Settings");
#ifdef VERBOSE
	cout << "ERROR: Setting number of modules to "<< i << endl;
#endif
	spinNumModules->setValue(i);

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabSettings::SetDynamicRange(int index){
	int ret,dr;
	switch (index) {
	case 0:    dr=32;		break;
	case 1:    dr=16;  	break;
	case 2:    dr=8;   	break;
	case 3:    dr=4;   	break;
	default:   dr=32;  	break;
	}
	ret=myDet->setDynamicRange(dr);
	if((ret==24)&&(dr==32)) dr = ret;
#ifdef VERBOSE
	cout << "Setting dynamic range to "<< dr << endl;
#endif
	if(ret!=dr){
		qDefs::Message(qDefs::WARNING,"Dynamic Range cannot be set to this value.","Settings");
#ifdef VERBOSE
		cout << "ERROR: Setting dynamic range to "<< ret << endl;
#endif
		switch(ret){
		case 32:  comboDynamicRange->setCurrentIndex(0);	break;
		case 24:  comboDynamicRange->setCurrentIndex(0);	break;
		case 16:	comboDynamicRange->setCurrentIndex(1);  break;
		case 8:	comboDynamicRange->setCurrentIndex(2);	break;
		case 4:	comboDynamicRange->setCurrentIndex(3);	break;
		default:	comboDynamicRange->setCurrentIndex(0);	break;
		}
	}

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabSettings::SetEnergy(){
	int index = spinThreshold->value();
#ifdef VERBOSE
	cout << "Settings threshold energy to "<< index << endl;
#endif
	myDet->setThresholdEnergy(index);
	int ret = (int)myDet->getThresholdEnergy();
	if((ret-index)>200){
		qDefs::Message(qDefs::WARNING,"Threshold energy could not be set. The difference is greater than 200.","Settings");
	}
	disconnect(spinThreshold,	SIGNAL(valueChanged(int)),	this, SLOT(SetEnergy()));
	spinThreshold->setValue(ret);
	connect(spinThreshold,		SIGNAL(valueChanged(int)),	this, SLOT(SetEnergy()));

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabSettings::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating Settings Tab" << endl;
#endif


	// Number of Modules
#ifdef VERBOSE
	cout  << "Getting number of modules" << endl;
#endif
	spinNumModules->setValue(myDet->setNumberOfModules());

	// Dynamic Range
#ifdef VERBOSE
	cout  << "Getting dynamic range" << endl;
#endif
	switch(myDet->setDynamicRange(-1)){
	case 32:   	comboDynamicRange->setCurrentIndex(0);	break;
	case 24:   	comboDynamicRange->setCurrentIndex(0);	break;
	case 16:	comboDynamicRange->setCurrentIndex(1);  break;
	case 8:	  	comboDynamicRange->setCurrentIndex(2);	break;
	case 4:	  	comboDynamicRange->setCurrentIndex(3);	break;
	default:	comboDynamicRange->setCurrentIndex(0);	break;
	}


	// Settings
#ifdef VERBOSE
	cout  << "Getting settings" << endl;
#endif
	disconnect(comboSettings, 		SIGNAL(currentIndexChanged(int)),	this, SLOT(setSettings(int)));
	int sett = (int)myDet->getSettings();
	if(sett==-1) sett = slsDetectorDefs::UNDEFINED;
	comboSettings->setCurrentIndex(sett);
	connect(comboSettings, 		SIGNAL(currentIndexChanged(int)),	this, SLOT(setSettings(int)));


	//threshold
	sett = comboSettings->currentIndex();
	if((detType==slsDetectorDefs::MYTHEN)||(detType==slsDetectorDefs::EIGER)){
		if((sett==Undefined)||(sett==Uninitialized)){
			lblThreshold->setEnabled(false);
			spinThreshold->setEnabled(false);
		}else{
			lblThreshold->setEnabled(true);
			spinThreshold->setEnabled(true);
#ifdef VERBOSE
			cout  << "Getting threshold energy" << endl;
#endif
			SetEnergy();
		}
	}


#ifdef VERBOSE
	cout  << "**Updated Settings Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

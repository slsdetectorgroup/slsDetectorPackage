/*
 * qTabSettings.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabSettings.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabSettings::qTabSettings(QWidget *parent,slsDetectorUtils*& detector,int detID):
		QWidget(parent),myDet(detector),detID(detID){

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
	/** Detector Type*/
	detType=myDet->getDetectorsType();

	/** Settings */
	SetupDetectorSettings();
	comboSettings->setCurrentIndex(myDet->getSettings(detID));


}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::SetupDetectorSettings(){
	/** Get detector settings from detector*/
	slsDetectorDefs::detectorSettings sett = myDet->getSettings(detID);

	/** To be able to index items on a combo box */
	model = qobject_cast<QStandardItemModel*>(comboSettings->model());
	if (model) {
		for(int i=0;i<NumSettings;i++){
			index[i] = model->index(i,	comboSettings->modelColumn(), comboSettings->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}
		/** Enabling/Disabling depending on the detector type
			Undefined and uninitialized are enabled for all detectors*/
		if(sett==slsDetectorDefs::UNDEFINED)
			item[(int)Uninitialized]->setEnabled(false);
		else if(sett==slsDetectorDefs::UNINITIALIZED)
			item[(int)Undefined]->setEnabled(false);
		else{
			item[(int)Uninitialized]->setEnabled(false);
			item[(int)Undefined]->setEnabled(false);
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
			qDefs::ErrorMessage("Unknown detector type.","Settings");
			exit(-1);
			break;
		}
		/** detector settings selected NOT ENABLED.
		 * This should not happen -only if the server and gui has a mismatch
		 * on which all modes are allowed in detectors */
		if(!(item[(int)sett]->isEnabled())){
			qDefs::ErrorMessage("Unknown Detector Settings retrieved from detector. "
					"Exiting GUI.","Settings");
#ifdef VERBOSE
			cout<<"ERROR:  Unknown Detector Settings retrieved from detector."<<endl;
#endif
			exit(-1);
		}
		/** Setting the detector settings */
		else {
			comboSettings->setCurrentIndex((int)sett);

		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::Initialization(){
	/** Settings */
	connect(comboSettings,SIGNAL(currentIndexChanged(int)),this,SLOT(setSettings(int)));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabSettings::setSettings(int index){
	/** The first time settings is changed from undefined or uninitialized to a proper setting,
	 * then undefined/uninitialized should be disabled */
	if(item[(int)Undefined]->isEnabled()){
		/**Do not disable it if this wasnt selected again by mistake*/
		if(index!=(int)Undefined)
			item[(int)Undefined]->setEnabled(false);
	}else if(item[(int)Uninitialized]->isEnabled()){
		/**Do not disable it if this wasnt selected again by mistake*/
		if(index!=(int)Uninitialized)
			item[(int)Uninitialized]->setEnabled(false);
	}
	slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index,detID);
#ifdef VERBOSE
	cout<<"Settings have been set to "<<myDet->slsDetectorBase::getDetectorSettings(sett)<<endl;
#endif

}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabSettings::Refresh(){
	/** Settings */
	SetupDetectorSettings();
	comboSettings->setCurrentIndex(myDet->getSettings(detID));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

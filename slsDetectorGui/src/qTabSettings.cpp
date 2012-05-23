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


#define Detector_Index 0



qTabSettings::qTabSettings(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabSettings::~qTabSettings(){
	delete myDet;
}




void qTabSettings::SetupWidgetWindow(){
	/** Settings */
	comboSettings->setCurrentIndex(myDet->getSettings(Detector_Index));
}



void qTabSettings::Initialization(){
	/** Settings */
	connect(comboSettings,SIGNAL(currentIndexChanged(int)),this,SLOT(setSettings(int)));
}



void qTabSettings::Enable(bool enable){
	comboSettings->setEnabled(enable);
}


void qTabSettings::setSettings(int index){
	slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index,Detector_Index);
#ifdef VERBOSE
	cout<<"Settings have been set to "<<myDet->slsDetectorBase::getDetectorSettings(sett)<<endl;
#endif

}


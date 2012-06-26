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



qTabSettings::qTabSettings(QWidget *parent,slsDetectorUtils*& detector,int detID):
		QWidget(parent),myDet(detector),detID(detID){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();

}




qTabSettings::~qTabSettings(){
	delete myDet;
}




void qTabSettings::SetupWidgetWindow(){
	/** Settings */
	comboSettings->setCurrentIndex(myDet->getSettings(detID));
}



void qTabSettings::Initialization(){
	/** Settings */
	connect(comboSettings,SIGNAL(currentIndexChanged(int)),this,SLOT(setSettings(int)));
}



void qTabSettings::setSettings(int index){
	slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index,detID);
#ifdef VERBOSE
	cout<<"Settings have been set to "<<myDet->slsDetectorBase::getDetectorSettings(sett)<<endl;
#endif

}


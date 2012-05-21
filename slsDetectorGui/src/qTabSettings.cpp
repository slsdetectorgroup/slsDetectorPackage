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
}



void qTabSettings::Initialization(){
}



void qTabSettings::Enable(bool enable){
	//this->setEnabled(enable);

}



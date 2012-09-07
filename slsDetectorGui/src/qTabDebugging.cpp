/*
 * qTabDebugging.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDebugging.h"
#include "qDefs.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// C++ Include Headers
#include<iostream>
using namespace std;



//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDebugging::qTabDebugging(QWidget *parent,multiSlsDetector*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDebugging::~qTabDebugging(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::SetupWidgetWindow(){
	// Detector Type
	detType=myDet->getDetectorsType();

	//status
	int detStatus = (int)myDet->getRunStatus();
	string status = slsDetectorBase::runStatusType(slsDetectorDefs::runStatus(detStatus));
	lblStatus->setText(QString(status.c_str()).toUpper());

	if(detType==slsDetectorDefs::EIGER) lblModule->setText("Half Module Number:");
	else lblModule->setText("Module Number:");

	// loading combo box module numbers
	int max = myDet->setNumberOfModules();


/*	for(int i=0;i<max;i++){
		slsDetector *s = myDet->getSlsDetector(i);
		if(s->setTCPSocket()!=slsDetectorDefs::FAIL){
			comboModule->addItem(QString::number(i));
		}
	}*/
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::Initialization(){
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::Refresh(){

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


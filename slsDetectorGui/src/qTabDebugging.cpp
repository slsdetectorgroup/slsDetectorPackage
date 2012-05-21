/*
 * qTabDebugging.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDebugging.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0



qTabDebugging::qTabDebugging(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabDebugging::~qTabDebugging(){
	delete myDet;
}




void qTabDebugging::SetupWidgetWindow(){
}



void qTabDebugging::Initialization(){
}



void qTabDebugging::Enable(bool enable){
	//this->setEnabled(enable);

}



/*
 * qTabDeveloper.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDeveloper.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0



qTabDeveloper::qTabDeveloper(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabDeveloper::~qTabDeveloper(){
	delete myDet;
}




void qTabDeveloper::SetupWidgetWindow(){
}



void qTabDeveloper::Initialization(){
}



void qTabDeveloper::Enable(bool enable){
	//this->setEnabled(enable);

}



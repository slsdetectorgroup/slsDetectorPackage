/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
/** Qt Project Class Headers */
#include "qTabActions.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0



qTabActions::qTabActions(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabActions::~qTabActions(){
	delete myDet;
}




void qTabActions::SetupWidgetWindow(){
}



void qTabActions::Initialization(){
}



void qTabActions::Enable(bool enable){
	//this->setEnabled(enable);

}





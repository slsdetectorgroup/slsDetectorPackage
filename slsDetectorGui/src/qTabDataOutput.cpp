/*
 * qTabDataOutput.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDataOutput.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0



qTabDataOutput::qTabDataOutput(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabDataOutput::~qTabDataOutput(){
	delete myDet;
}




void qTabDataOutput::SetupWidgetWindow(){
}



void qTabDataOutput::Initialization(){
}



void qTabDataOutput::Enable(bool enable){
	//this->setEnabled(enable);

}



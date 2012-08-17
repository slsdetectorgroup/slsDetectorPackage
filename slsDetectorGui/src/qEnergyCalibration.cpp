/*
 * qEnergyCalibration.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qEnergyCalibration.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;




qEnergyCalibration::qEnergyCalibration(QWidget *parent,multiSlsDetector*& detector):QWizard(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}




qEnergyCalibration::~qEnergyCalibration(){
	delete myDet;
}




void qEnergyCalibration::SetupWidgetWindow(){

}



void qEnergyCalibration::Initialization(){

}





//-------------------------------------------------------------------------------------------------------------------------------------------------


void qEnergyCalibration::Refresh(){

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


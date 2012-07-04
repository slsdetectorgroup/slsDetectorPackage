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




qTabDeveloper::qTabDeveloper(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();

}




qTabDeveloper::~qTabDeveloper(){
	delete myDet;
}




void qTabDeveloper::SetupWidgetWindow(){
}



void qTabDeveloper::Initialization(){
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Refresh(){

}


//-------------------------------------------------------------------------------------------------------------------------------------------------



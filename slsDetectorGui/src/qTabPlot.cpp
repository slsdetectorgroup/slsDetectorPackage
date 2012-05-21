/*
 * qTabPlot.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabPlot.h"
#include "qDefs.h"
#include "qDrawPlot.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0



qTabPlot::qTabPlot(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot):QWidget(parent),myDet(detector),myPlot(plot){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabPlot::~qTabPlot(){
	delete myDet;
	delete myPlot;
}




void qTabPlot::SetupWidgetWindow(){
	box1D->setEnabled(false);
}



void qTabPlot::Initialization(){
/** Plot box*/
	connect(btnClone, 		SIGNAL(clicked()),myPlot, 	SLOT(ClonePlot()));
	connect(btnCloseClones, SIGNAL(clicked()),myPlot, 	SLOT(CloseClones()));
/** 2D Plot box*/
	connect(chkInterpolate, SIGNAL(toggled(bool)),myPlot, SIGNAL(InterpolateSignal(bool)));
	connect(chkContour, 	SIGNAL(toggled(bool)),myPlot, SIGNAL(ContourSignal(bool)));
	connect(chkLogz, 		SIGNAL(toggled(bool)),myPlot, SIGNAL(LogzSignal(bool)));
}



void qTabPlot::Enable(bool enable){
	boxPlot->setEnabled(enable);
	box1D->setEnabled(enable);
	box2D->setEnabled(enable);
	boxPlotAxis->setEnabled(enable);
}



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
#include <iostream>
#include <string>
using namespace std;


#define Detector_Index 0


QString qTabPlot::defaultPlotTitle("Measurement");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");


qTabPlot::qTabPlot(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot):QWidget(parent),myDet(detector),myPlot(plot){
	setupUi(this);
	if(myDet)
	{
		// wherever you choose plot do all these steps
		//This also selects the text if unchecked
		//includes setupwidgetwindow
		//SelectPlot(1);
		Initialization();
	}
}




qTabPlot::~qTabPlot(){
	delete myDet;
	delete myPlot;
}




void qTabPlot::SetupWidgetWindow(){
/** Plot Axis*/
	myPlot->SetPlotTitle(defaultPlotTitle);
	dispTitle->setText(defaultPlotTitle);
	dispTitle->setEnabled(false);
	dispXAxis->setEnabled(false);
	dispYAxis->setEnabled(false);
	dispZAxis->setEnabled(false);
	dispXMin->setEnabled(false);
	dispYMin->setEnabled(false);
	dispZMin->setEnabled(false);
	dispXMax->setEnabled(false);
	dispYMax->setEnabled(false);
	dispZMax->setEnabled(false);
}


void qTabPlot::Select1DPlot(bool b){
	SetupWidgetWindow();
	if(b){
		myPlot->Select1DPlot();
		box1D->setEnabled(true);
		box2D->setEnabled(false);
		chkZAxis->setEnabled(false);
		chkZMin->setEnabled(false);
		chkZMax->setEnabled(false);
		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
		dispXAxis->setText(defaultHistXAxisTitle);
		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
		dispYAxis->setText(defaultHistYAxisTitle);
	}else{
		myPlot->Select2DPlot();
		box1D->setEnabled(false);
		box2D->setEnabled(true);
		chkZAxis->setEnabled(true);
		chkZMin->setEnabled(true);
		chkZMax->setEnabled(true);
		myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
		dispXAxis->setText(defaultImageXAxisTitle);
		myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
		dispYAxis->setText(defaultImageYAxisTitle);
		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
		dispZAxis->setText(defaultImageZAxisTitle);
	}
}


void qTabPlot::Initialization(){
/** Plot box*/
	connect(btnClone, 		SIGNAL(clicked()),myPlot, 	SLOT(ClonePlot()));
	connect(btnCloseClones, SIGNAL(clicked()),myPlot, 	SLOT(CloseClones()));
/** 2D Plot box*/
	connect(chkInterpolate, SIGNAL(toggled(bool)),myPlot, SIGNAL(InterpolateSignal(bool)));
	connect(chkContour, 	SIGNAL(toggled(bool)),myPlot, SIGNAL(ContourSignal(bool)));
	connect(chkLogz, 		SIGNAL(toggled(bool)),myPlot, SIGNAL(LogzSignal(bool)));
/** Plot Axis **/
	connect(chkTitle, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkXAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkYAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkZAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(dispTitle, 		SIGNAL(returnPressed()), this, 	SLOT(SetTitles()));
	connect(dispXAxis, 		SIGNAL(returnPressed()), this, 	SLOT(SetTitles()));
	connect(dispYAxis, 		SIGNAL(returnPressed()), this, 	SLOT(SetTitles()));
	connect(dispZAxis, 		SIGNAL(returnPressed()), this, 	SLOT(SetTitles()));

/** Common Buttons*/
	connect(btnClear, 		SIGNAL(clicked()),myPlot, 	SLOT(Clear1DPlot()));

/** test for 1D*/
	connect(chktest1D,  	SIGNAL(toggled(bool)), this, 	SLOT(Select1DPlot(bool)));
}



void qTabPlot::Enable(bool enable){
	btnClone->setEnabled(enable);
	btnCloseClones->setEnabled(enable);
	box1D->setEnabled(enable);
	box2D->setEnabled(enable);
	boxPlotAxis->setEnabled(enable);
}

void qTabPlot::SetTitles(){
	int oneD = box1D->isEnabled();
	/** Plot Title*/
	if(dispTitle->isEnabled())
		myPlot->SetPlotTitle(dispTitle->text());
	/** X Axis */
	if(dispXAxis->isEnabled()){
		if(oneD)	myPlot->SetHistXAxisTitle(dispXAxis->text());
		else	myPlot->SetImageXAxisTitle(dispXAxis->text());
	}
	/** Y Axis */
	if(dispYAxis->isEnabled()){
		if(oneD)	myPlot->SetHistYAxisTitle(dispYAxis->text());
		else	myPlot->SetImageYAxisTitle(dispYAxis->text());
	}
	/** Z Axis */
	if(dispZAxis->isEnabled())
		myPlot->SetImageZAxisTitle(dispZAxis->text());
}


void qTabPlot::EnableTitles(){
	int oneD = box1D->isEnabled();
	/** Plot Title*/
	dispTitle->setEnabled(chkTitle->isChecked());
	if(!chkTitle->isChecked()){
		myPlot->SetPlotTitle(defaultPlotTitle);
		dispTitle->setText(defaultPlotTitle);
	}
	/** X Axis */
	dispXAxis->setEnabled(chkXAxis->isChecked());
	if(!chkXAxis->isChecked()){
		if(oneD){
			myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
			dispXAxis->setText(defaultHistXAxisTitle);
		}
		else{
			myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
			dispXAxis->setText(defaultImageXAxisTitle);
		}
	}
	/** Y Axis */
	dispYAxis->setEnabled(chkYAxis->isChecked());
	if(!chkYAxis->isChecked()){
		if(oneD){
			myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
			dispYAxis->setText(defaultHistYAxisTitle);
		}else{
			myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
			dispYAxis->setText(defaultImageYAxisTitle);
		}
	}
	/** Z Axis */
	dispZAxis->setEnabled(chkZAxis->isChecked());
	if(!chkZAxis->isChecked()){
		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
		dispZAxis->setText(defaultImageZAxisTitle);
	}
}




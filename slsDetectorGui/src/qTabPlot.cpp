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
		//switch(myDet->detectorytype)
		Select1DPlot(true);
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
/*	dispXMin->setValidator(new QDoubleValidator(dispXMin));
	dispYMin->setValidator(new QDoubleValidator(dispYMin));
	dispZMin->setValidator(new QDoubleValidator(dispZMin));
	dispXMax->setValidator(new QDoubleValidator(dispXMax));
	dispYMax->setValidator(new QDoubleValidator(dispYMax));
	dispZMax->setValidator(new QDoubleValidator(dispZMax));*/
}


void qTabPlot::Select1DPlot(bool b){
	SetupWidgetWindow();
	if(b){
		box1D->setEnabled(true);
		box2D->setEnabled(false);
		chkZAxis->setEnabled(false);
		chkZMin->setEnabled(false);
		chkZMax->setEnabled(false);
		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
		dispXAxis->setText(defaultHistXAxisTitle);
		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
		dispYAxis->setText(defaultHistYAxisTitle);
		myPlot->Select1DPlot();
	}else{
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
		myPlot->Select2DPlot();
	}
}


void qTabPlot::Initialization(){
/** Plot arguments box*/
	connect(chkNoPlot, SIGNAL(toggled(bool)),myPlot, SLOT(EnablePlot(bool)));
/** Snapshot box*/
	connect(btnClone, 		SIGNAL(clicked()),myPlot, 	SLOT(ClonePlot()));
	connect(btnCloseClones, SIGNAL(clicked()),myPlot, 	SLOT(CloseClones()));
/** 1D Plot box*/
	connect(chkSuperimpose, SIGNAL(toggled(bool)),this, SLOT(EnablePersistency(bool)));
	connect(spinPersistency,SIGNAL(valueChanged(int)),myPlot,SLOT(SetPersistency(int)));
/** 2D Plot box*/
	connect(chkInterpolate, SIGNAL(toggled(bool)),myPlot, SIGNAL(InterpolateSignal(bool)));
	connect(chkContour, 	SIGNAL(toggled(bool)),myPlot, SIGNAL(ContourSignal(bool)));
	connect(chkLogz, 		SIGNAL(toggled(bool)),myPlot, SIGNAL(LogzSignal(bool)));
/** Plot Axis **/
	connect(chkTitle, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkXAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkYAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(chkZAxis, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableTitles()));
	connect(dispTitle, 		SIGNAL(textChanged(const QString&)), this, 	SLOT(SetTitles()));
	connect(dispXAxis, 		SIGNAL(textChanged(const QString&)), this, 	SLOT(SetTitles()));
	connect(dispYAxis, 		SIGNAL(textChanged(const QString&)), this, 	SLOT(SetTitles()));
	connect(dispZAxis, 		SIGNAL(textChanged(const QString&)), this, 	SLOT(SetTitles()));

	connect(chkXMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));
	connect(chkXMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));
	connect(chkYMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));
	connect(chkYMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));
	connect(chkZMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));
	connect(chkZMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableRange()));

	connect(dispXMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispXMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispYMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispYMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispZMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispZMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));

/** Common Buttons*/
	connect(btnClear, 		SIGNAL(clicked()),		myPlot, SLOT(Clear1DPlot()));
/** Save */
	connect(btnSave, 		SIGNAL(clicked()),		this, 	SLOT(SavePlot()));

}



void qTabPlot::Enable(bool enable){
	btnClone->setEnabled(enable);
	btnCloseClones->setEnabled(enable);
	box1D->setEnabled(enable);
	box2D->setEnabled(enable);
	boxPlotAxis->setEnabled(enable);
}

void qTabPlot::EnablePersistency(bool enable){
	lblPersistency->setEnabled(enable);
	spinPersistency->setEnabled(enable);
	if(enable)		myPlot->SetPersistency(spinPersistency->value());
	else			myPlot->SetPersistency(0);

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




void qTabPlot::EnableRange(){
	bool disableZoom = false;
	if(!chkXMin->isChecked())	{dispXMin->setText("");	dispXMin->setEnabled(false);}
	else						{disableZoom = true;	dispXMin->setEnabled(true);	}
	if(!chkXMax->isChecked())	{dispXMax->setText("");	dispXMax->setEnabled(false);}
	else 						{disableZoom = true;	dispXMax->setEnabled(true); }
	if(!chkYMin->isChecked())	{dispYMin->setText("");	dispYMin->setEnabled(false);}
	else 						{disableZoom = true;	dispYMin->setEnabled(true);	}
	if(!chkYMax->isChecked())	{dispYMax->setText("");	dispYMax->setEnabled(false);}
	else 						{disableZoom = true;	dispYMax->setEnabled(true); }
	if(!chkZMin->isChecked())	{dispZMin->setText("");	dispZMin->setEnabled(false);}
	else 						{disableZoom = true;	dispZMin->setEnabled(true); }
	if(!chkZMax->isChecked())	{dispZMax->setText("");	dispZMax->setEnabled(false);}
	else 						{disableZoom = true;	dispZMax->setEnabled(true); }
	myPlot->DisableZoom(disableZoom);
	emit DisableZoomSignal(disableZoom);
}



void qTabPlot::SetAxesRange(){
	double xmin,xmax,ymin,ymax,zmin,zmax;
	int oneD = box1D->isEnabled();
	//should be filled for 2d as well
	if(!dispXMin->text().isEmpty())	xmin = dispXMin->text().toDouble();
	else {	if(oneD)				xmin = myPlot->GetHistXAxisLowerBound();}
	if(!dispXMax->text().isEmpty())	xmax = dispXMax->text().toDouble();
	else {	if(oneD)				xmax = myPlot->GetHistXAxisUpperBound();}
	if(!dispYMin->text().isEmpty())	ymin = dispYMin->text().toDouble();
	else {	if(oneD)				ymin = myPlot->GetHistYAxisLowerBound();}
	if(!dispYMax->text().isEmpty())	ymax = dispYMax->text().toDouble();
	else {	if(oneD)				ymax = myPlot->GetHistYAxisUpperBound();}
	if(!dispZMin->text().isEmpty())	zmin = dispZMin->text().toDouble();
	if(!dispZMax->text().isEmpty())	zmax = dispZMax->text().toDouble();
	//should be filled for 2d as well
	if(oneD){
		myPlot->SetHistXAxisScale(xmin,xmax);
		myPlot->SetHistYAxisScale(ymin,ymax);
	}
}


void qTabPlot::SavePlot(){
	QString fullFileName = QString(myDet->getFilePath().c_str())+'/'+dispFName->text()+comboFormat->currentText();
	myPlot->SavePlot(fullFileName);
}






//dispzmin... when unchecked, unzoom and get lower and upper bound... when checked just set lower and upper bound

/*

#include "SlsQtValidators.h"

class QDoubleValidator;
SlsQtDoubleValidator*  validator_double[2];




validator_double = new SlsQtDoubleValidator(num_field);
num_field->setValidator(validator_double);
//default settings
validator_double->setDecimals(3);
double v= num_field->text().toDouble(ok););
is ok 1? for correct conversion

QString s = QString::number(v);
validator_double->fixup(s);
num_field->setText(s);


num_field[i]->setAlignment(Qt::AlignRight);



connect(num_field[i],SIGNAL(lostFocus()),this,SLOT(FirstValueEntered()));

*/




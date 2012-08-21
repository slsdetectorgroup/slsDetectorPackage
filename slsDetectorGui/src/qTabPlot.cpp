/*
 * qTabPlot.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabPlot.h"
#include "qDefs.h"
#include "qDrawPlot.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QStandardItemModel>
// C++ Include Headers
#include <iostream>
#include <string>
#include <math.h>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------


QString qTabPlot::defaultPlotTitle("Measurement");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabPlot::qTabPlot(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot):QWidget(parent),myDet(detector),myPlot(plot){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabPlot::~qTabPlot(){
	delete myDet;
	delete myPlot;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetupWidgetWindow(){
	//check if nth frame should be enabled
	enableNFrame = true;
	//according to timing mode
	slsDetectorDefs::externalCommunicationMode mode = myDet->setExternalCommunicationMode();
	if(	(mode==slsDetectorDefs::GATE_FIX_NUMBER)||
			(mode==slsDetectorDefs::TRIGGER_FRAME) ||
			(mode==slsDetectorDefs::TRIGGER_WINDOW) ||
			((mode==slsDetectorDefs::AUTO_TIMING)&&((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1)==1)&&((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1)==1)) )
		enableNFrame = false;
	//according to if exptime > acq period
	if((myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))>(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-9)))
		enableNFrame = false;



//scan arguments
	btnGroupScan = new QButtonGroup(this);
	btnGroupScan->addButton(radioLevel0,0);
	btnGroupScan->addButton(radioLevel1,1);
	btnGroupScan->addButton(radioFileIndex,2);
	btnGroupScan->addButton(radioAllFrames,3);

// Plot Axis
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
	dispXMin->setValidator(new QDoubleValidator(dispXMin));
	dispYMin->setValidator(new QDoubleValidator(dispYMin));
	dispZMin->setValidator(new QDoubleValidator(dispZMin));
	dispXMax->setValidator(new QDoubleValidator(dispXMax));
	dispYMax->setValidator(new QDoubleValidator(dispYMax));
	dispZMax->setValidator(new QDoubleValidator(dispZMax));

	// Plotting Frequency

	stackedLayout = new QStackedLayout;
	stackedLayout->setSpacing(0);
	spinNthFrame = new QSpinBox;
		spinNthFrame->setMinimum(1);
		spinNthFrame->setMaximum(2000000000);
	spinTimeGap = new QDoubleSpinBox;
		spinTimeGap->setMinimum(0);
		spinTimeGap->setDecimals(3);
		spinTimeGap->setMaximum(999999);
		spinTimeGap->setValue(myPlot->GetMinimumPlotTimer());
	comboTimeGapUnit = new QComboBox;
		comboTimeGapUnit->addItem("hr");
		comboTimeGapUnit->addItem("min");
		comboTimeGapUnit->addItem("s");
		comboTimeGapUnit->addItem("ms");
		comboTimeGapUnit->setCurrentIndex(3);
	QWidget *w = new QWidget;
	QHBoxLayout *h1 = new QHBoxLayout;
	w->setLayout(h1);
	h1->setContentsMargins(0,0,0,0);
		h1->setSpacing(3);
		h1->addWidget(spinTimeGap);
		h1->addWidget(comboTimeGapUnit);

	stackedLayout->addWidget(w);
	stackedLayout->addWidget(spinNthFrame);
	stackWidget->setLayout(stackedLayout);


	// Depending on whether the detector is 1d or 2d
	switch(myDet->getDetectorsType()){
	case slsDetectorDefs::MYTHEN:	isOrginallyOneD = true; 	Select1DPlot(true);	break;
	case slsDetectorDefs::EIGER:	isOrginallyOneD = false;	Select1DPlot(false);break;
	case slsDetectorDefs::GOTTHARD:	isOrginallyOneD = true; 	Select1DPlot(true);break;
	default:
		cout << "ERROR: Detector Type is Generic" << endl;
		exit(-1);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Select1DPlot(bool b){
	isOneD = b;
	if(b){
		box1D->show();
		box2D->hide();
		chkZAxis->setEnabled(false);
		chkZMin->setEnabled(false);
		chkZMax->setEnabled(false);
		dispXAxis->setText(defaultHistXAxisTitle);
		dispYAxis->setText(defaultHistYAxisTitle);
		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
		myPlot->Select1DPlot();
	}else{
		box1D->hide();
		box2D->show();
		chkZAxis->setEnabled(true);
		chkZMin->setEnabled(true);
		chkZMax->setEnabled(true);
		dispXAxis->setText(defaultImageXAxisTitle);
		dispYAxis->setText(defaultImageYAxisTitle);
		dispZAxis->setText(defaultImageZAxisTitle);
		myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
		myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
		myPlot->Select2DPlot();
	}

	boxScan->setEnabled(false);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Initialization(){
// Plot arguments box
	connect(radioNoPlot, 	SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
	connect(radioHistogram, SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
	connect(radioDataGraph, SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
// Scan box
	connect(btnGroupScan, SIGNAL(buttonClicked(QAbstractButton *)),this, SLOT(SetScanArgument()));
// Snapshot box
	connect(btnClone, 		SIGNAL(clicked()),myPlot, 	SLOT(ClonePlot()));
	connect(btnCloseClones, SIGNAL(clicked()),myPlot, 	SLOT(CloseClones()));
// 1D Plot box
	connect(chkSuperimpose, SIGNAL(toggled(bool)),		this, SLOT(EnablePersistency(bool)));
	connect(spinPersistency,SIGNAL(valueChanged(int)),	myPlot,SLOT(SetPersistency(int)));
	connect(chkPoints, 		SIGNAL(toggled(bool)),		myPlot, SLOT(SetDottedPlot(bool)));
// 2D Plot box
	connect(chkInterpolate, SIGNAL(toggled(bool)),myPlot, SIGNAL(InterpolateSignal(bool)));
	connect(chkContour, 	SIGNAL(toggled(bool)),myPlot, SIGNAL(ContourSignal(bool)));
	connect(chkLogz, 		SIGNAL(toggled(bool)),myPlot, SIGNAL(LogzSignal(bool)));
// Plotting frequency box
	connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
	connect(comboTimeGapUnit,SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
	connect(spinTimeGap,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
	connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// Plot Axis *
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
	connect(chkZMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableZRange()));
	connect(chkZMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableZRange()));
	connect(this, 			SIGNAL(EnableZRangeSignal(bool)), myPlot, 	SIGNAL(EnableZRangeSignal(bool)));

	connect(dispXMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispXMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispYMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispYMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetAxesRange()));
	connect(dispZMin, 		SIGNAL(returnPressed()), this, 	SLOT(SetZRange()));
	connect(dispZMax, 		SIGNAL(returnPressed()), this, 	SLOT(SetZRange()));

	connect(this,SIGNAL(SetZRangeSignal(double,double)),myPlot, SIGNAL(SetZRangeSignal(double,double)));

// Common Buttons
// Save
	connect(btnSave, 		SIGNAL(clicked()),		myPlot,	SLOT(SavePlot()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnablePersistency(bool enable){
	lblPersistency->setEnabled(enable);
	spinPersistency->setEnabled(enable);
	if(enable)		myPlot->SetPersistency(spinPersistency->value());
	else			myPlot->SetPersistency(0);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetTitles(){
	// Plot Title
	if(dispTitle->isEnabled())
		myPlot->SetPlotTitle(dispTitle->text());
	// X Axis
	if(dispXAxis->isEnabled()){
		if(isOneD)	myPlot->SetHistXAxisTitle(dispXAxis->text());
		else	myPlot->SetImageXAxisTitle(dispXAxis->text());
	}
	// Y Axis
	if(dispYAxis->isEnabled()){
		if(isOneD)	myPlot->SetHistYAxisTitle(dispYAxis->text());
		else	myPlot->SetImageYAxisTitle(dispYAxis->text());
	}
	// Z Axis
	if(dispZAxis->isEnabled())
		myPlot->SetImageZAxisTitle(dispZAxis->text());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableTitles(){
	// Plot Title
	dispTitle->setEnabled(chkTitle->isChecked());
	if(!chkTitle->isChecked()){
		myPlot->SetPlotTitle(defaultPlotTitle);
		dispTitle->setText(defaultPlotTitle);
	}
	// X Axis
	dispXAxis->setEnabled(chkXAxis->isChecked());
	if(!chkXAxis->isChecked()){
		if(isOneD){
			myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
			dispXAxis->setText(defaultHistXAxisTitle);
		}
		else{
			myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
			dispXAxis->setText(defaultImageXAxisTitle);
		}
	}
	// Y Axis
	dispYAxis->setEnabled(chkYAxis->isChecked());
	if(!chkYAxis->isChecked()){
		if(isOneD){
			myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
			dispYAxis->setText(defaultHistYAxisTitle);
		}else{
			myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
			dispYAxis->setText(defaultImageYAxisTitle);
		}
	}
	// Z Axis
	dispZAxis->setEnabled(chkZAxis->isChecked());
	if(!chkZAxis->isChecked()){
		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
		dispZAxis->setText(defaultImageZAxisTitle);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableRange(){
	bool disableZoom = false;
	if(!chkXMin->isChecked())	dispXMin->setEnabled(false);
	else{disableZoom = true;	dispXMin->setEnabled(true);	}
	if(!chkXMax->isChecked())	dispXMax->setEnabled(false);
	else{disableZoom = true;	dispXMax->setEnabled(true); }
	if(!chkYMin->isChecked())	dispYMin->setEnabled(false);
	else{disableZoom = true;	dispYMin->setEnabled(true);	}
	if(!chkYMax->isChecked())	dispYMax->setEnabled(false);
	else{disableZoom = true;	dispYMax->setEnabled(true); }

	myPlot->DisableZoom(disableZoom);
	emit DisableZoomSignal(disableZoom);
	SetAxesRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetAxesRange(){
	bool changed = false;
	// x min
	changed = (dispXMin->isEnabled())&&(!dispXMin->text().isEmpty());
	if(changed)	myPlot->SetXYRangeValues(dispXMin->text().toDouble(),qDefs::XMINIMUM);
	myPlot->IsXYRangeValues(changed,qDefs::XMINIMUM);

	// x max
	changed = (dispXMax->isEnabled())&&(!dispXMax->text().isEmpty());
	if(changed)	myPlot->SetXYRangeValues(dispXMax->text().toDouble(),qDefs::XMAXIMUM);
	myPlot->IsXYRangeValues(changed,qDefs::XMAXIMUM);

	// y min
	changed = (dispYMin->isEnabled())&&(!dispYMin->text().isEmpty());
	if(changed)	myPlot->SetXYRangeValues(dispYMin->text().toDouble(),qDefs::YMINIMUM);
	myPlot->IsXYRangeValues(changed,qDefs::YMINIMUM);

	// y max
	changed = (dispYMax->isEnabled())&&(!dispYMax->text().isEmpty());
	if(changed)	myPlot->SetXYRangeValues(dispYMax->text().toDouble(),qDefs::YMAXIMUM);
	myPlot->IsXYRangeValues(changed,qDefs::YMAXIMUM);

	//  To remind the updateplot in qdrawplot to set range after updating plot
	myPlot->SetXYRange(true);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetZRange(){
	emit SetZRangeSignal(dispZMin->text().toDouble(),dispZMax->text().toDouble());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableZRange(){
	dispZMin->setEnabled(chkZMin->isChecked());
	dispZMax->setEnabled(chkZMax->isChecked());
	emit EnableZRangeSignal((chkZMin->isChecked())||(chkZMax->isChecked()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetPlot(){
#ifdef VERBOSE
	cout << "Entering Set Plot()" ;
#endif
	if(radioNoPlot->isChecked()){
		cout << " - No Plot" << endl;
		Select1DPlot(isOrginallyOneD);
		myPlot->EnablePlot(false);
		//if enable is true, disable everything
		if(isOrginallyOneD) {box1D->show(); box1D->setEnabled(false); box2D->hide();}
		if(!isOrginallyOneD){box2D->show(); box2D->setEnabled(false); box1D->hide();}
		boxSnapshot->setEnabled(false);
		boxSave->setEnabled(false);
		boxFrequency->setEnabled(false);
		boxPlotAxis->setEnabled(false);
		boxScan->setEnabled(false);
	}else if(radioDataGraph->isChecked()){
		cout << " - DataGraph" << endl;
		myPlot->EnablePlot(true);
		//if enable is true, disable everything
		if(isOrginallyOneD) {box1D->show();box1D->setEnabled(true);} else box1D->hide();
		if(!isOrginallyOneD){box2D->show();box2D->setEnabled(true);} else box2D->hide();
		Select1DPlot(isOrginallyOneD);
		boxSnapshot->setEnabled(true);
		boxSave->setEnabled(true);
		boxFrequency->setEnabled(true);
		boxPlotAxis->setEnabled(true);
		EnableScanBox();
	}else{
		cout << " - Histogram" << endl;
		//select(2d) will set oneD to false, but originallyoneD will remember
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::SetFrequency(){

	disconnect(comboTimeGapUnit,SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
	disconnect(spinTimeGap,		SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
	disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));

	double timeMS,acqPeriodMS;
	double minPlotTimer = myPlot->GetMinimumPlotTimer();
	char cplotms[200];
	sprintf(cplotms,"%f ms",minPlotTimer);

	stackedLayout->setCurrentIndex(comboFrequency->currentIndex());
	switch(comboFrequency->currentIndex()){
	case 0:
		// Get the time interval from gui in ms
		timeMS = (qDefs::getNSTime((qDefs::timeUnit)comboTimeGapUnit->currentIndex(),spinTimeGap->value()))/(1e6);
		if(timeMS<minPlotTimer){
			qDefs::WarningMessage("Interval between Plots - The Time Interval between plots "
					"must be atleast "+string(cplotms)+".","Plot");
			spinTimeGap->setValue(minPlotTimer);
			comboTimeGapUnit->setCurrentIndex(qDefs::MILLISECONDS);
		}
		//This is done so that its known which one was selected
		myPlot->SetFrameFactor(0);
		// Setting the timer value(ms) between plots
		myPlot->SetPlotTimer(timeMS);
#ifdef VERBOSE
	cout << "Plotting Frequency: Time Gap - " << spinTimeGap->value() << qDefs::getUnitString((qDefs::timeUnit)comboTimeGapUnit->currentIndex()) << endl;
#endif
		break;
	case 1:
		acqPeriodMS = (myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-6));
		// gets the acq period * number of nth frames
		timeMS = (spinNthFrame->value())*acqPeriodMS;
		// To make sure the period between plotting is not less than minimum plot timer in  ms
		if(timeMS<minPlotTimer){
			int minFrame = (int)(ceil)(minPlotTimer/acqPeriodMS);
			qDefs::WarningMessage("<b>Plot Tab:</b> Interval between Plots - The nth Image must be larger.<br><br>"
					"Condition to be satisfied:\n(Acquisition Period)*(nth Image) >= 250ms."
					"<br><br>Nth image adjusted to minimum, "
					"for the chosen Acquisition Period.","Plot");
			spinNthFrame->setValue(minFrame);
		}
		// Setting the timer value (nth frames) between plots
		myPlot->SetFrameFactor(spinNthFrame->value());
#ifdef VERBOSE
	cout << "Plotting Frequency: Nth Frame - " << spinNthFrame->value() << endl;
#endif
		break;
	}

	connect(comboTimeGapUnit,SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
	connect(spinTimeGap,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
	connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableScanBox(int mode,int id){
#ifdef VERYVERBOSE
	cout << "Entering Enable Scan Box() \t mode:" << mode << " \t id:" << id << endl;
#endif

	int mode0 = myDet->getScanMode(0);
	int mode1 = myDet->getScanMode(1);

	boxScan->setEnabled(mode0||mode1);

	if(boxScan->isEnabled()){
		//make sure nth frame frequency plot is disabled
		EnablingNthFrameFunction(false);

		//if level0 or 1, check argument to find which scan enabled last from actions tab
		if((btnGroupScan->checkedId()!=2)&&(btnGroupScan->checkedId()!=3)){
			cout<<"mode:"<<mode<<" id:"<<id<<endl;
			//if mode is not none(doesnt check them if called form refresh)
			if(mode>0){
				if(!id) radioLevel0->setChecked(true);
				else radioLevel1->setChecked(true);
			}
		}

		radioLevel0->setEnabled(mode0);
		radioLevel1->setEnabled(mode1);
	}else EnablingNthFrameFunction(enableNFrame);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::EnablingNthFrameFunction(bool enable){
#ifdef VERYVERBOSE
	cout << "Enabling Nth Frame : " << enable << endl;
#endif
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboFrequency->model());
	QStandardItem* item = model->itemFromIndex(model->index(1,	comboFrequency->modelColumn(), comboFrequency->rootModelIndex()));

	//enabling/disabling is easy if it wasnt selected anyway
	if(comboFrequency->currentIndex()!=1)
		item->setEnabled(enable);
	else{
		//only when it was enabled before and now to disable is a problem
		if(!enable){
			spinTimeGap->setValue(myPlot->GetMinimumPlotTimer());
			comboFrequency->setCurrentIndex(0);
			item->setEnabled(false);
		}
	}

}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::SetScanArgument(){
	switch(btnGroupScan->checkedId()){

	//let qdrawplot know which scan argument
	//myPlot->

	//level0
	case 0:
		break;

	//level1
	case 1:
		break;

	//file index
	case 2:
		break;

	//all frames
	case 3:
		break;
	}

}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Refresh(){
	SetFrequency();
	EnableScanBox();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * qTabPlot.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#include "qTabPlot.h"
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

const QString qTabPlot::modeNames[5]={"None","Energy Scan","Threshold Scan","Trimbits Scan","Custom Script Scan"};

QString qTabPlot::defaultPlotTitle("");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabPlot::qTabPlot(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot):
				QWidget(parent),
				myDet(detector),
				myPlot(plot),
				isOneD(false),
				isOriginallyOneD(false),
				wrongInterval(0),
				stackedLayout(0),
				spinNthFrame(0),
				spinTimeGap(0),
				comboTimeGapUnit(0),
				btnGroupScan(0){
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
	//error for interval between plots
	red = new QPalette();;
	red->setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	intervalTip = boxFrequency->toolTip();


//scan arguments
	btnGroupScan = new QButtonGroup(this);
	btnGroupScan->addButton(radioLevel0,0);
	btnGroupScan->addButton(radioLevel1,1);
	btnGroupScan->addButton(radioFileIndex,2);
	btnGroupScan->addButton(radioAllFrames,3);

// Plot Axis
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

	//default titles
	dispTitle->setText("");
	myPlot->SetPlotTitlePrefix("");
	dispXAxis->setText(defaultHistXAxisTitle);
	dispYAxis->setText(defaultHistYAxisTitle);
	myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
	myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
	dispXAxis->setText(defaultImageXAxisTitle);
	dispYAxis->setText(defaultImageYAxisTitle);
	dispZAxis->setText(defaultImageZAxisTitle);
	myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
	myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
	myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);

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

	stackedWidget->setCurrentIndex(0);
	stackedWidget_2->setCurrentIndex(0);

	// Depending on whether the detector is 1d or 2d
	switch(myDet->getDetectorsType()){
	case slsDetectorDefs::MYTHEN:
		isOriginallyOneD = true;
		chkPedestal->setEnabled(false);
		btnRecalPedestal->setEnabled(false);
		chkPedestal_2->setEnabled(false);
		btnRecalPedestal_2->setEnabled(false);
		chkBinary->setEnabled(false);
		chkBinary_2->setEnabled(false);
		break;
	case slsDetectorDefs::EIGER:
		isOriginallyOneD = false;
		chkPedestal->setEnabled(false);
		btnRecalPedestal->setEnabled(false);
		chkPedestal_2->setEnabled(false);
		btnRecalPedestal_2->setEnabled(false);
		chkBinary->setEnabled(false);
		chkBinary_2->setEnabled(false);
		break;
	case slsDetectorDefs::GOTTHARD:
		isOriginallyOneD = true;
		break;
	case slsDetectorDefs::MOENCH:
		isOriginallyOneD = false;
		break;
	default:
		cout << "ERROR: Detector Type is Generic" << endl;
		exit(-1);
	}

	Select1DPlot(isOriginallyOneD);
	if(isOriginallyOneD) myPlot->Select1DPlot();
	else 				myPlot->Select2DPlot();

	//to check if this should be enabled
	EnableScanBox();

	qDefs::checkErrorMessage(myDet,"qTabPlot::SetupWidgetWindow");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetPlotOptionsRightPage(){
	if(isOneD){
		int i = stackedWidget->currentIndex();
		if(i == (stackedWidget->count()-1))
			stackedWidget->setCurrentIndex(0);
		else
			stackedWidget->setCurrentIndex(i+1);
		box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget->currentIndex()+1));
	}
	else{
		int i = stackedWidget_2->currentIndex();
		if(i == (stackedWidget_2->count()-1))
			stackedWidget_2->setCurrentIndex(0);
		else
			stackedWidget_2->setCurrentIndex(i+1);
		box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget_2->currentIndex()+1));
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetPlotOptionsLeftPage(){
	if(isOneD){
		int i = stackedWidget->currentIndex();
		if(i == 0)
			stackedWidget->setCurrentIndex(stackedWidget->count()-1);
		else
			stackedWidget->setCurrentIndex(i-1);
		box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget->currentIndex()+1));
	}
	else{
		int i = stackedWidget_2->currentIndex();
		if(i == 0)
			stackedWidget_2->setCurrentIndex(stackedWidget_2->count()-1);
		else
			stackedWidget_2->setCurrentIndex(i-1);
		box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget_2->currentIndex()+1));
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Select1DPlot(bool b){
	isOneD = b;
	lblFrom->setEnabled(false);
	lblTo->setEnabled(false);
	lblFrom_2->setEnabled(false);
	lblTo_2->setEnabled(false);
	spinFrom->setEnabled(false);
	spinFrom_2->setEnabled(false);
	spinTo->setEnabled(false);
	spinTo_2->setEnabled(false);
	if(b){
		box1D->show();
		box2D->hide();
		chkZAxis->setEnabled(false);
		chkZMin->setEnabled(false);
		chkZMax->setEnabled(false);
	}else{
		box1D->hide();
		box2D->show();
		chkZAxis->setEnabled(true);
		chkZMin->setEnabled(true);
		chkZMax->setEnabled(true);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Initialization(){
// Plot arguments box
	connect(radioNoPlot, 	SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
	connect(radioHistogram, SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
	connect(radioDataGraph, SIGNAL(toggled(bool)),this, SLOT(SetPlot()));
// Scan box
	/*connect(btnGroupScan, SIGNAL(buttonClicked(QAbstractButton *)),this, SLOT(SetScanArgument()));*/
	connect(boxScan,	  SIGNAL(toggled(bool)),				   this, SLOT(EnableScanBox()));
// Snapshot box
	connect(btnClone, 		SIGNAL(clicked()),myPlot, 	SLOT(ClonePlot()));
	connect(btnCloseClones, SIGNAL(clicked()),myPlot, 	SLOT(CloseClones()));
	connect(btnSaveClones,	SIGNAL(clicked()),myPlot, 	SLOT(SaveClones()));
// 1D Plot box
	//to change pages
	connect(btnRight, 		SIGNAL(clicked()),		this, SLOT(SetPlotOptionsRightPage()));
	connect(btnLeft, 		SIGNAL(clicked()),		this, SLOT(SetPlotOptionsLeftPage()));

	connect(chkSuperimpose, SIGNAL(toggled(bool)),		this, SLOT(EnablePersistency(bool)));
	connect(spinPersistency,SIGNAL(valueChanged(int)),	myPlot,SLOT(SetPersistency(int)));
	connect(chkPoints, 		SIGNAL(toggled(bool)),		myPlot, SLOT(SetMarkers(bool)));
	connect(chkLines, 		SIGNAL(toggled(bool)),		myPlot, SLOT(SetLines(bool)));
	connect(chk1DLog, 		SIGNAL(toggled(bool)),		myPlot, SIGNAL(LogySignal(bool)));
	connect(chkStatistics, 	SIGNAL(toggled(bool)),		myPlot, SLOT(DisplayStatistics(bool)));

// 2D Plot box
	connect(chkInterpolate, SIGNAL(toggled(bool)),myPlot, SIGNAL(InterpolateSignal(bool)));
	connect(chkContour, 	SIGNAL(toggled(bool)),myPlot, SIGNAL(ContourSignal(bool)));
	connect(chkLogz, 		SIGNAL(toggled(bool)),myPlot, SIGNAL(LogzSignal(bool)));
	connect(chkStatistics_2,SIGNAL(toggled(bool)),myPlot, SLOT(DisplayStatistics(bool)));
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
	connect(this, 			SIGNAL(ResetZMinZMaxSignal(bool,bool,double,double)), myPlot, 	SIGNAL(ResetZMinZMaxSignal(bool,bool,double,double)));

	connect(dispXMin, 		SIGNAL(editingFinished()), this,	SLOT(SetAxesRange()));
	connect(dispXMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetAxesRange()));
	connect(dispYMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetAxesRange()));
	connect(dispYMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetAxesRange()));
	connect(dispZMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
	connect(dispZMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
// Save
	connect(btnSave, 		SIGNAL(clicked()),		myPlot,	SLOT(SavePlot()));
	connect(chkSaveAll, 	SIGNAL(toggled(bool)),	myPlot,	SLOT(SaveAll(bool)));

	connect(this,SIGNAL(SetZRangeSignal(double,double)),myPlot, SIGNAL(SetZRangeSignal(double,double)));

//pedstal
	connect(chkPedestal, 		SIGNAL(toggled(bool)),	myPlot, 	SLOT(SetPedestal(bool)));
	connect(btnRecalPedestal, 	SIGNAL(clicked()),		myPlot, 	SLOT(RecalculatePedestal()));
	connect(chkPedestal_2, 		SIGNAL(toggled(bool)),	myPlot, 	SLOT(SetPedestal(bool)));
	connect(btnRecalPedestal_2,	SIGNAL(clicked()),		myPlot, 	SLOT(RecalculatePedestal()));

//accumulate
	connect(chkAccumulate, 			SIGNAL(toggled(bool)),	myPlot, 	SLOT(SetAccumulate(bool)));
	connect(btnResetAccumulate, 	SIGNAL(clicked()),		myPlot, 	SLOT(ResetAccumulate()));
	connect(chkAccumulate_2, 		SIGNAL(toggled(bool)),	myPlot, 	SLOT(SetAccumulate(bool)));
	connect(btnResetAccumulate_2,	SIGNAL(clicked()),		myPlot, 	SLOT(ResetAccumulate()));

	//binary
	connect(chkBinary, 			SIGNAL(toggled(bool)),		this,	SLOT(SetBinary()));
	connect(chkBinary_2, 		SIGNAL(toggled(bool)),		this,	SLOT(SetBinary()));
	connect(spinFrom,			SIGNAL(valueChanged(int)),	this,	SLOT(SetBinary()));
	connect(spinFrom_2,			SIGNAL(valueChanged(int)),	this,	SLOT(SetBinary()));
	connect(spinTo,				SIGNAL(valueChanged(int)),	this,	SLOT(SetBinary()));
	connect(spinTo_2,			SIGNAL(valueChanged(int)),	this,	SLOT(SetBinary()));
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
		myPlot->SetPlotTitlePrefix(dispTitle->text());
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
		myPlot->SetPlotTitlePrefix("");
		dispTitle->setText("");
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
#ifdef VERBOSE
	cout << "Setting Range" << endl;
#endif

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
//	emit SetZRangeSignal(dispZMin->text().toDouble(),dispZMax->text().toDouble());
	emit ResetZMinZMaxSignal(chkZMin->isChecked(),chkZMax->isChecked(),dispZMin->text().toDouble(),dispZMax->text().toDouble());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableZRange(){
	dispZMin->setEnabled(chkZMin->isChecked());
	dispZMax->setEnabled(chkZMax->isChecked());
	emit ResetZMinZMaxSignal(chkZMin->isChecked(),chkZMax->isChecked(),dispZMin->text().toDouble(),dispZMax->text().toDouble());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetPlot(){
#ifdef VERBOSE
	cout << "Entering Set Plot()" ;
#endif
	if(radioNoPlot->isChecked()){
		cout << " - No Plot" << endl;

		//Select1DPlot(isOriginallyOneD);
		//if(isOriginallyOneD) {box1D->show(); box2D->hide();}
		//if(!isOriginallyOneD){box2D->show(); box1D->hide();}
		myPlot->EnablePlot(false);
		//if enable is true, disable everything

		boxSnapshot->setEnabled(false);
		boxSave->setEnabled(false);
		boxFrequency->setEnabled(false);
		boxPlotAxis->setEnabled(false);
		boxScan->setEnabled(false);
	}else {//if(radioDataGraph->isChecked()){
		cout << " - DataGraph" << endl;

		myPlot->EnablePlot(true);
		//if enable is true, disable everything
		if(isOriginallyOneD)  {box1D->show(); box2D->hide();}
		if(!isOriginallyOneD) {box2D->show(); box1D->hide();}
		Select1DPlot(isOriginallyOneD);
		if(isOriginallyOneD) myPlot->Select1DPlot();
		else 				myPlot->Select2DPlot();
		boxSnapshot->setEnabled(true);
		boxSave->setEnabled(true);
		boxFrequency->setEnabled(true);
		boxPlotAxis->setEnabled(true);
		if(!myPlot->isRunning()) EnableScanBox();
	}/*else{
		cout << " - Histogram" << endl;
		//select(2d) will set oneD to false, but originallyoneD will remember
	}*/
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::SetFrequency(){

	disconnect(comboTimeGapUnit,SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
	disconnect(spinTimeGap,		SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
	disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
	disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));


	double timeMS,acqPeriodMS;
	double minPlotTimer = myPlot->GetMinimumPlotTimer();
	char cMin[200];
	sprintf(cMin,"%f ms",minPlotTimer);


	acqPeriodMS = (myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-6));
	//if period is 0, check exptime, if that is also 0, give warning and set to min timer
	if(acqPeriodMS==0){
		acqPeriodMS = (myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-6));

		if(acqPeriodMS==0){
			qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots:</nobr><br><nobr>"
					"<b>Every Nth Image</b>: Period betwen Frames and Exposure Time cannot both be 0 ms.</nobr><br><nobr>"
					"Resetting to minimum plotting time interval","qTabPlot::SetFrequency");
			comboFrequency->setCurrentIndex(0);
			stackedLayout->setCurrentIndex(comboFrequency->currentIndex());
			spinTimeGap->setValue(minPlotTimer);
			comboTimeGapUnit->setCurrentIndex(qDefs::MILLISECONDS);
			timeMS=minPlotTimer;
			//This is done so that its known which one was selected
			myPlot->SetFrameFactor(0);
			// Setting the timer value(ms) between plots
			myPlot->SetPlotTimer(timeMS);

			connect(comboTimeGapUnit,SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
			connect(spinTimeGap,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
			connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
			connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
			qDefs::checkErrorMessage(myDet,"qTabPlot::SetFrequency");
			return;
		}
	}


	stackedLayout->setCurrentIndex(comboFrequency->currentIndex());
	switch(comboFrequency->currentIndex()){
	case 0:
		// Get the time interval from gui in ms
		timeMS = (qDefs::getNSTime((qDefs::timeUnit)comboTimeGapUnit->currentIndex(),spinTimeGap->value()))/(1e6);

		if((int)timeMS==0){
			qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots:</nobr><br><nobr>"
					"Time Interval must be atleast >= 1 ms. Resetting to minimum plotting time interval.","qTabPlot::SetFrequency");
			spinTimeGap->setValue(minPlotTimer);
			comboTimeGapUnit->setCurrentIndex(qDefs::MILLISECONDS);
			timeMS=minPlotTimer;
		}


		//show red if min interval<minplottimer
		if(timeMS<minPlotTimer){
			//qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
			boxFrequency->setPalette(*red);
			boxFrequency->setTitle("Interval between Plots*");
			QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
					"<b>Time Interval</b> Condition: min of ")+QString("%1").arg(minPlotTimer)+
							QString("ms.</nobr><br><nobr>You might be losing images!</nobr></font>");
			boxFrequency->setToolTip(errTip);
		}
		//show red if acqPeriod<minInterval
		else if((acqPeriodMS+1)<timeMS){
			cout<<"\nacqPeriodMS:"<<acqPeriodMS<<"\ttimeMS:"<<timeMS<<endl;
			//qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
			boxFrequency->setPalette(*red);
			boxFrequency->setTitle("Interval between Plots*");
			QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
					"<b>Time Interval</b> Acquisition Period should be >= Time Interval between plots.</nobr><br><nobr>"
					"You might be losing images!</nobr></font>");
			boxFrequency->setToolTip(errTip);
		}
		//correct
		else{
			boxFrequency->setPalette(boxSnapshot->palette());
			boxFrequency->setTitle("Interval between Plots");
			boxFrequency->setToolTip(intervalTip);
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

		// gets the acq period * number of nth frames
		timeMS = (spinNthFrame->value())*acqPeriodMS;

		//Show red to make sure the period between plotting is not less than minimum plot timer in  ms
		if(timeMS<minPlotTimer){
			int minFrame = (int)(ceil)(minPlotTimer/acqPeriodMS);
			//qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
			boxFrequency->setPalette(*red);
			boxFrequency->setTitle("Interval between Plots*");
			QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
					"<b>Every nth Image</b> Condition: min nth Image for this time period: ")+QString("%1").arg(minFrame)+
							QString(".</nobr><br><nobr>You might be losing images!</nobr></font>");
			boxFrequency->setToolTip(errTip);
		}else{
			boxFrequency->setPalette(boxSnapshot->palette());
			boxFrequency->setTitle("Interval between Plots");
			boxFrequency->setToolTip(intervalTip);
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
	connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));

	qDefs::checkErrorMessage(myDet,"qTabPlot::SetFrequency");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::EnableScanBox(){
#ifdef VERYVERBOSE
	cout << "Entering Enable Scan Box()" << endl;
#endif

	int mode0 = myDet->getScanMode(0);
	int mode1 = myDet->getScanMode(1);

	//if it was checked before or disabled before, it remembers to check it again
	bool checkedBefore = boxScan->isChecked();//||(!boxScan->isEnabled()));

	int ang;
	//none of these scan plotting options make sense if positions exists
	bool positionsExist = myDet->getAngularConversion(ang);//myDet->getPositions();

	qDefs::checkErrorMessage(myDet,"qTabPlot::EnableScanBox");

	//only now enable/disable
	boxScan->setEnabled((mode0||mode1)&&(!positionsExist));

	//after plotting, enable datagraph if it was disabled while plotting(refresh)
	radioDataGraph->setEnabled(true);


	//if there are scan
	if(boxScan->isEnabled()){
		//disable histogram
		if(radioHistogram->isChecked())
			radioDataGraph->setChecked(true);
		radioHistogram->setEnabled(false);

		//make sure nth frame frequency plot is disabled
		EnablingNthFrameFunction(false);

		//if 2d is chosen or not for scan
		if(boxScan->isChecked()){

			boxScan->setChecked(checkedBefore);
			//make sure nth frame frequency plot is disabled
			EnablingNthFrameFunction(false);

			//
			if(mode0 && mode1){
				if(!radioFileIndex->isChecked())	radioAllFrames->setChecked(true);
				radioLevel0->setEnabled(false);
				radioLevel1->setEnabled(false);
			}else{
				radioLevel0->setEnabled(mode0);
				radioLevel1->setEnabled(mode1);
			}

			//only if level0 or level1 is checked
			if((radioLevel0->isChecked())||(radioLevel1->isChecked())){
				if(mode0)	radioLevel0->setChecked(true);
				if(mode1)	radioLevel1->setChecked(true);
			}
		}
	}
	else{
		//histogram for 1d
		if(isOriginallyOneD){
			radioHistogram->setEnabled(true);
			if(radioHistogram->isChecked())
				EnablingNthFrameFunction(false);
			else
				EnablingNthFrameFunction(true);
		}
	}


	//positions
	if((positionsExist)&&(chkSuperimpose->isChecked())) chkSuperimpose->setChecked(false);
	chkSuperimpose->setEnabled(!positionsExist);
	//box frequency should be enabled cuz its a normal 1d plot
	//boxFrequency->setEnabled(positionsExist);
	myPlot->EnableAnglePlot(positionsExist);


	//sets the scan argument
	/*SetScanArgument();*/
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
	bool histogram = radioHistogram->isChecked();

	//as default from histogram and default titles are set here if scanbox is disabled
	if(isOriginallyOneD){
		dispXAxis->setText(defaultHistXAxisTitle);
		dispYAxis->setText(defaultHistYAxisTitle);
		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
	}else{
		dispXAxis->setText(defaultImageXAxisTitle);
		dispYAxis->setText(defaultImageYAxisTitle);
		dispZAxis->setText(defaultImageZAxisTitle);
		myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
		myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
	}
	Select1DPlot(isOriginallyOneD);
	if(isOriginallyOneD) myPlot->Select1DPlot();
	else 				myPlot->Select2DPlot();


	int ang;
	//if scans(1D or 2D)
	if((boxScan->isEnabled())||(histogram)){
		//setting the title according to the scans
		Select1DPlot(isOriginallyOneD);
		if(isOriginallyOneD) myPlot->Select1DPlot();
		else 				myPlot->Select2DPlot();

	}//angles (1D)
	else if(myDet->getAngularConversion(ang)){
		dispXAxis->setText("Angles");
		myPlot->SetHistXAxisTitle("Angles");
		Select1DPlot(true);
		myPlot->Select1DPlot();

	}

	//histogram
	if(histogram){
		//allFrames
		myPlot->SetScanArgument(qDefs::AllFrames);

		//default titles  for 2d scan
		dispXAxis->setText("Channel Number");
		myPlot->SetImageXAxisTitle("Channel Number");
		dispZAxis->setText("Counts");
		myPlot->SetImageZAxisTitle("Counts");
		dispYAxis->setText("All Frames");
		myPlot->SetImageYAxisTitle("All Frames");

		//set plot to 2d
		Select1DPlot(false);
		myPlot->Select2DPlot();
	}
	//2d
	else if((boxScan->isEnabled())&&(boxScan->isChecked())){

		//let qdrawplot know which scan argument
		myPlot->SetScanArgument(btnGroupScan->checkedId()+1);

		//default titles  for 2d scan
		dispXAxis->setText("Channel Number");
		myPlot->SetImageXAxisTitle("Channel Number");
		dispZAxis->setText("Counts");
		myPlot->SetImageZAxisTitle("Counts");

		//titles for y of 2d scan
		switch(btnGroupScan->checkedId()){
		case 0://level0
			dispYAxis->setText("Scan Level 0");
			myPlot->SetImageYAxisTitle("Scan Level 0");
			break;
		case 1://level1
			dispYAxis->setText("Scan Level 1");
			myPlot->SetImageYAxisTitle("Scan Level 1");
			break;
			break;
		case 2://file index
			dispYAxis->setText("Frame Index");
			myPlot->SetImageYAxisTitle("Frame Index");
			break;
		case 3://all frames
			dispYAxis->setText("All Frames");
			myPlot->SetImageYAxisTitle("All Frames");
			break;
		}

		//set plot to 2d
		Select1DPlot(false);
		myPlot->Select2DPlot();

	}else //done here so that it isnt set by default each time
		myPlot->SetScanArgument(qDefs::None);

	qDefs::checkErrorMessage(myDet,"qTabPlot::SetScanArgument");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating Plot Tab" << endl;
#endif
	if(!myPlot->isRunning()){
		if (!radioNoPlot->isChecked())
			boxFrequency->setEnabled(true);
		connect(boxScan,	  SIGNAL(toggled(bool)),				   this, SLOT(EnableScanBox()));
		EnableScanBox();
		SetFrequency();

	}else{
		boxFrequency->setEnabled(false);
		disconnect(boxScan,	  SIGNAL(toggled(bool)),				   this, SLOT(EnableScanBox()));
		boxScan->setEnabled(false);
		//to toggle between no plot and the plot mode chosen while pltting
		if(radioHistogram->isChecked())
			radioDataGraph->setEnabled(false);
		radioHistogram->setEnabled(false);

	}
#ifdef VERBOSE
	cout  << "**Updated Plot Tab" << endl << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::UpdateAfterCloning(){
#ifdef VERBOSE
	cout  << endl << "**Updating Plot After Cloning" << endl;
#endif
	EnableRange();
	EnableTitles();
	myPlot->UpdateAfterCloning(
			chkPoints->isChecked(),
			chk1DLog->isChecked(),
			chkInterpolate->isChecked(),
			chkContour->isChecked(),
			chkLogz->isChecked());

	//1d
	if(isOneD)
		EnablePersistency(chkSuperimpose->isChecked());
	//2d
	else
		EnableZRange();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetBinary(){
	//1d
	if(isOneD){
		if(chkBinary->isChecked()){
#ifdef VERBOSE
			cout  << endl << "Enabling Binary" << endl;
#endif
			lblFrom->setEnabled(true);
			lblTo->setEnabled(true);
			spinFrom->setEnabled(true);
			spinTo->setEnabled(true);
			myPlot->SetBinary(true,spinFrom->value(),spinTo->value());
		}else{
#ifdef VERBOSE
			cout  << endl << "Disabling Binary" << endl;
#endif
			lblFrom->setEnabled(false);
			lblTo->setEnabled(false);
			spinFrom->setEnabled(false);
			spinTo->setEnabled(false);
			myPlot->SetBinary(false);
		}
	}
	//2d
	else{
		if(chkBinary_2->isChecked()){
#ifdef VERBOSE
			cout  << endl << "Enabling Binary" << endl;
#endif
			lblFrom_2->setEnabled(true);
			lblTo_2->setEnabled(true);
			spinFrom_2->setEnabled(true);
			spinTo_2->setEnabled(true);
			myPlot->SetBinary(true,spinFrom_2->value(),spinTo_2->value());

		}else{
#ifdef VERBOSE
			cout  << endl << "Disabling Binary" << endl;
#endif

			lblFrom_2->setEnabled(false);
			lblTo_2->setEnabled(false);
			spinFrom_2->setEnabled(false);
			spinTo_2->setEnabled(false);
			myPlot->SetBinary(false);
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

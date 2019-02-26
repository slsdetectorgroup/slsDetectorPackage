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
				btnGroupScan(0),
				btnGroupPlotType(0),
				btnGroupHistogram(0){
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

//plot type
	btnGroupPlotType = new QButtonGroup(this);
	btnGroupPlotType->addButton(radioNoPlot,0);
	btnGroupPlotType->addButton(radioDataGraph,1);
	btnGroupPlotType->addButton(radioHistogram,2);

//histogram arguments
	btnGroupHistogram = new QButtonGroup(this);
	btnGroupHistogram->addButton(radioHistIntensity,0);
	btnGroupHistogram->addButton(radioHistLevel0,1);
	btnGroupHistogram->addButton(radioHistLevel1,2);

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
	switch(myDet->getDetectorTypeAsEnum()){
	case slsDetectorDefs::EIGER:
		isOriginallyOneD = false;
		pagePedestal->setEnabled(false);
		pagePedestal_2->setEnabled(false);
		chkBinary->setEnabled(false);
		chkBinary_2->setEnabled(false);
		chkGapPixels->setEnabled(true);
		break;
	case slsDetectorDefs::GOTTHARD:
		isOriginallyOneD = true;
		break;
	case slsDetectorDefs::MOENCH:
		isOriginallyOneD = false;
		break;
	case slsDetectorDefs::JUNGFRAU:
	case slsDetectorDefs::CHIPTESTBOARD:
		isOriginallyOneD = false;
		chkGainPlot->setEnabled(true);
		break;
	default:
		cout << "ERROR: Detector Type is Generic" << endl;
		exit(-1);
	}

	Select1DPlot(isOriginallyOneD);

	//to check if this should be enabled
	EnableScanBox();

	//disable histogram initially
	boxHistogram->hide();

    if (chkGapPixels->isEnabled()) {
        int ret =  myDet->enableGapPixels(-1);
        qDefs::checkErrorMessage(myDet,"qTabPlot::SetupWidgetWindow");
        chkGapPixels->setChecked((ret == 1) ? true : false);
    }

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
#ifdef VERBOSE
	if(b)
		cout << "Selecting 1D Plot" << endl;
	else
		cout << "Selecting 2D Plot" << endl;
#endif
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
		myPlot->Select1DPlot();
	}else{
		box1D->hide();
		box2D->show();
		chkZAxis->setEnabled(true);
		chkZMin->setEnabled(true);
		chkZMax->setEnabled(true);
		myPlot->Select2DPlot();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::Initialization(){
// Plot arguments box
	connect(btnGroupPlotType,SIGNAL(buttonClicked(int)),this, SLOT(SetPlot()));
// Histogram arguments box
	connect(btnGroupHistogram,SIGNAL(buttonClicked(int)),this, SLOT(SetHistogramOptions()));
// Scan box
	connect(boxScan,	  SIGNAL(toggled(bool)),  this, SLOT(EnableScanBox()));
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

	connect(chkXMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableXRange()));
	connect(chkXMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableXRange()));
	connect(chkYMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableYRange()));
	connect(chkYMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableYRange()));
	connect(chkZMin, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableZRange()));
	connect(chkZMax, 		SIGNAL(toggled(bool)), this, 	SLOT(EnableZRange()));
	connect(chkAspectRatio,	SIGNAL(toggled(bool)), this, 	SLOT(checkAspectRatio()));
	connect(this, 			SIGNAL(ResetZMinZMaxSignal(bool,bool,double,double)), myPlot, 	SIGNAL(ResetZMinZMaxSignal(bool,bool,double,double)));

	connect(dispXMin, 		SIGNAL(editingFinished()), this,	SLOT(SetXAxisRange()));
	connect(dispXMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetXAxisRange()));
	connect(dispYMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetYAxisRange()));
	connect(dispYMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetYAxisRange()));
	connect(dispZMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
	connect(dispZMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
// Save
	connect(btnSave, 		SIGNAL(clicked()),		myPlot,	SLOT(SavePlot()));
	connect(chkSaveAll, 	SIGNAL(toggled(bool)),	myPlot,	SLOT(SaveAll(bool)));

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

	//gainplot
	if (chkGainPlot->isEnabled())
		connect(chkGainPlot, SIGNAL(toggled(bool)),myPlot, SIGNAL(GainPlotSignal(bool)));

	// gap pixels
    if (chkGapPixels->isEnabled())
        connect(chkGapPixels, SIGNAL(toggled(bool)),this, SLOT(EnableGapPixels(bool)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnablePersistency(bool enable){
#ifdef VERBOSE
	if(enable)
		cout << "Enabling Persistency" << endl;
	else
		cout << "Disabling Persistency" << endl;
#endif
	lblPersistency->setEnabled(enable);
	spinPersistency->setEnabled(enable);
	if(enable)		myPlot->SetPersistency(spinPersistency->value());
	else			myPlot->SetPersistency(0);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetTitles(){
#ifdef VERBOSE
	cout << "Setting Plot Titles" << endl;
#endif
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


void qTabPlot::checkAspectRatio() {
	if (chkAspectRatio->isChecked()) {
		maintainAspectRatio(-1);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::maintainAspectRatio(int axis) {
#ifdef VERBOSE
	cout << "Maintaining Aspect Ratio" << endl;
#endif
	disconnect(chkXMin, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableXRange()));
	disconnect(chkXMax, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableXRange()));
	disconnect(chkYMin, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableYRange()));
	disconnect(chkYMax, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableYRange()));
	disconnect(dispXMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetXAxisRange()));
	disconnect(dispXMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetXAxisRange()));
	disconnect(dispYMin, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));
	disconnect(dispYMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));

	double ranges[4];
	//get previous plot limits
	ranges[qDefs::XMINIMUM] = myPlot->GetXMinimum();
	ranges[qDefs::XMAXIMUM] = myPlot->GetXMaximum();
	ranges[qDefs::YMINIMUM] = myPlot->GetYMinimum();
	ranges[qDefs::YMAXIMUM] = myPlot->GetYMaximum();
#ifdef VERYVERBOSE
	cprintf(BLUE,"ideal values: xmin:%f, xmax:%f ymin:%f ymax:%f\n",ranges[qDefs::XMINIMUM],ranges[qDefs::XMAXIMUM],ranges[qDefs::YMINIMUM],ranges[qDefs::YMAXIMUM]);
#endif
	double idealAspectratio =  (ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]);

	// enable all
	chkXMin->setChecked(true);	dispXMin->setEnabled(true);
	chkXMax->setChecked(true);	dispXMax->setEnabled(true);
	chkYMin->setChecked(true);	dispYMin->setEnabled(true);
	chkYMax->setChecked(true);	dispYMax->setEnabled(true);

	// if any empty, set it to previous plots boundaries
	if (dispXMin->text().isEmpty()) dispXMin->setText(QString::number(myPlot->GetXMinimum()));
	if (dispXMax->text().isEmpty()) dispXMax->setText(QString::number(myPlot->GetXMaximum()));
	if (dispYMin->text().isEmpty()) dispYMin->setText(QString::number(myPlot->GetYMinimum()));
	if (dispYMax->text().isEmpty()) dispYMax->setText(QString::number(myPlot->GetYMaximum()));

	//get actual limits
	ranges[qDefs::XMINIMUM] = dispXMin->text().toDouble();
	ranges[qDefs::XMAXIMUM] = dispXMax->text().toDouble();
	ranges[qDefs::YMINIMUM] = dispYMin->text().toDouble();
	ranges[qDefs::YMAXIMUM] = dispYMax->text().toDouble();
#ifdef VERYVERBOSE
	cprintf(BLUE,"new limits: xmin:%f, xmax:%f ymin:%f ymax:%f\n",ranges[qDefs::XMINIMUM],ranges[qDefs::XMAXIMUM],ranges[qDefs::YMINIMUM],ranges[qDefs::YMAXIMUM]);
#endif
	// calcualte new aspect ratio
	double newAspectRatio = (ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]);

	// if not ideal aspect ratio
	if (newAspectRatio != idealAspectratio) {

		// find the larger difference
		if (axis == -1) {
			if ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) > (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM])) {
				//change x
				axis = 0;
			} else {
				//change y
				axis = 1;
			}
		}

		// if x changed: y adjusted, y changed: x adjusted, aspect ratio clicked: larger one adjusted
		double newval=0;
		switch(axis) {
		case 0:
			//change x
			newval = idealAspectratio * (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]) + ranges[qDefs::XMINIMUM];
			if (newval <= myPlot->GetXMaximum()) {
				dispXMax->setText(QString::number(newval));
#ifdef VERYVERBOSE
				cprintf(BLUE,"new xmax: %f\n",newval);
#endif
			} else {
				newval = ranges[qDefs::XMAXIMUM] - (idealAspectratio * (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]));
				dispXMin->setText(QString::number(newval));
#ifdef VERYVERBOSE
				cprintf(BLUE,"new xmin: %f\n",newval);
#endif
			}

			break;
		case 1:
			// change y
			newval = ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / idealAspectratio) + ranges[qDefs::YMINIMUM];
			if (newval <= myPlot->GetYMaximum()) {
				dispYMax->setText(QString::number(newval));
				//#ifdef VERYVERBOSE
				cprintf(BLUE,"new ymax: %f\n",newval);
				//#endif
			} else {
				newval = ranges[qDefs::YMAXIMUM] - ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / idealAspectratio);
				dispYMin->setText(QString::number(newval));
#ifdef VERYVERBOSE
				cprintf(BLUE,"new ymin: %f\n",newval);
#endif
			}
			break;
		default:
			break;
		}
	}

	connect(chkXMin, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableXRange()));
	connect(chkXMax, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableXRange()));
	connect(chkYMin, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableYRange()));
	connect(chkYMax, 		SIGNAL(toggled(bool)), 		this, 	SLOT(EnableYRange()));
	connect(dispXMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetXAxisRange()));
	connect(dispXMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetXAxisRange()));
	connect(dispYMin, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));
	connect(dispYMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));

	// disable mouse zooming if any checked
	myPlot->DisableZoom(true);
	emit DisableZoomSignal(true);

	// set XY values in plot
	myPlot->SetXYRangeValues(dispXMin->text().toDouble(),qDefs::XMINIMUM);
	myPlot->SetXYRangeValues(dispXMax->text().toDouble(),qDefs::XMAXIMUM);
	myPlot->SetXYRangeValues(dispYMin->text().toDouble(),qDefs::YMINIMUM);
	myPlot->SetXYRangeValues(dispYMax->text().toDouble(),qDefs::YMAXIMUM);


	myPlot->IsXYRangeValues(true,qDefs::XMINIMUM);
	myPlot->IsXYRangeValues(true,qDefs::XMAXIMUM);
	myPlot->IsXYRangeValues(true,qDefs::YMINIMUM);
	myPlot->IsXYRangeValues(true,qDefs::YMAXIMUM);

	//  To remind the updateplot in qdrawplot to set range after updating plot
	myPlot->SetXYRange(true);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::EnableXRange(){
#ifdef VERBOSE
	cout << "Enable X Axis Range" << endl;
#endif
	// keeping aspect ratio
	if (chkAspectRatio->isChecked()) {
		maintainAspectRatio(1);
		return;
	}
	disconnect(dispXMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetXAxisRange()));
	disconnect(dispXMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetXAxisRange()));

	// enable/disable lineedit
	if(chkXMin->isChecked()) {
		dispXMin->setEnabled(true);
		// if any empty, set it to previous plots boundaries
		if (dispXMin->text().isEmpty())
			dispXMin->setText(QString::number(myPlot->GetXMinimum()));
	} else {
		dispXMin->setEnabled(false);
	}
	// enable/disable lineedit
	if(chkXMax->isChecked()) {
		dispXMax->setEnabled(true);
		// if any empty, set it to previous plots boundaries
		if (dispXMax->text().isEmpty())
			dispXMax->setText(QString::number(myPlot->GetXMaximum()));
	} else {
		dispXMax->setEnabled(false);
	}

	connect(dispXMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetXAxisRange()));
	connect(dispXMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetXAxisRange()));

	EnableRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableYRange(){
#ifdef VERBOSE
	cout << "Enable Y Axis Range" << endl;
#endif
	// keeping aspect ratio
	if (chkAspectRatio->isChecked()) {
		maintainAspectRatio(0);
		return;
	}
	disconnect(dispYMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetYAxisRange()));
	disconnect(dispYMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));

	// enable/disable lineedit
	if(chkYMin->isChecked()) {
		dispYMin->setEnabled(true);
		// if any empty, set it to previous plots boundaries
		if (dispYMin->text().isEmpty())
			dispYMin->setText(QString::number(myPlot->GetYMinimum()));
	} else {
		dispYMin->setEnabled(false);
	}
	// enable/disable lineedit
	if(chkYMax->isChecked()) {
		dispYMax->setEnabled(true);
		// if any empty, set it to previous plots boundaries
		if (dispYMax->text().isEmpty())
			dispYMax->setText(QString::number(myPlot->GetYMaximum()));
	} else {
		dispYMax->setEnabled(false);
	}

	connect(dispYMin, 		SIGNAL(editingFinished()), 	this,	SLOT(SetYAxisRange()));
	connect(dispYMax, 		SIGNAL(editingFinished()), 	this, 	SLOT(SetYAxisRange()));

	EnableRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableRange(){
#ifdef VERBOSE
	cout << "Enable Axes Range" << endl;
#endif
	// disable mouse zooming if any checked
	bool disableZoom= false;
	if(chkYMin->isChecked() || chkYMax->isChecked() || chkYMin->isChecked() || chkYMax->isChecked())
		disableZoom = true;
	emit DisableZoomSignal(disableZoom);
	SetAxesRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetXAxisRange(){
#ifdef VERBOSE
	cout << "Setting X Axis Range" << endl;
#endif

	disconnect(dispXMin, 		SIGNAL(editingFinished()), this,	SLOT(SetXAxisRange()));
	disconnect(dispXMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetXAxisRange()));

	if (dispXMin->text().toDouble() < myPlot->GetXMinimum()) {
		qDefs::Message(qDefs::WARNING,"Outside Plot Range","qTabPlot::CheckZRange");
		dispXMin->setText(QString::number(myPlot->GetXMinimum()));
	}

	if (dispXMax->text().toDouble() > myPlot->GetXMaximum()) {
		qDefs::Message(qDefs::WARNING,"Outside Plot Range","qTabPlot::CheckZRange");
		dispXMax->setText(QString::number(myPlot->GetXMaximum()));
	}

	connect(dispXMin, 		SIGNAL(editingFinished()), this,	SLOT(SetXAxisRange()));
	connect(dispXMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetXAxisRange()));

	// keeping aspect ratio
	if (chkAspectRatio->isChecked()) {
		maintainAspectRatio(1);
		return;
	}

	SetAxesRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetYAxisRange(){
#ifdef VERBOSE
	cout << "Setting Y Axis Range" << endl;
#endif

	disconnect(dispYMin, 		SIGNAL(editingFinished()), this,	SLOT(SetYAxisRange()));
	disconnect(dispYMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetYAxisRange()));

	if (dispYMin->text().toDouble() < myPlot->GetYMinimum()) {
		qDefs::Message(qDefs::WARNING,"Outside Plot Range","qTabPlot::CheckZRange");
		dispYMin->setText(QString::number(myPlot->GetYMinimum()));
	}

	if (dispYMax->text().toDouble() > myPlot->GetYMaximum()) {
		qDefs::Message(qDefs::WARNING,"Outside Plot Range","qTabPlot::CheckZRange");
		dispYMax->setText(QString::number(myPlot->GetYMaximum()));
	}

	connect(dispYMin, 		SIGNAL(editingFinished()), this,	SLOT(SetYAxisRange()));
	connect(dispYMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetYAxisRange()));


	// keeping aspect ratio
	if (chkAspectRatio->isChecked()) {
		maintainAspectRatio(0);
		return;
	}

	SetAxesRange();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetAxesRange(){
#ifdef VERBOSE
	cout << "Setting Axes Range" << endl;
#endif

	// x min
	if (dispXMin->isEnabled()) {
		myPlot->SetXYRangeValues(dispXMin->text().toDouble(),qDefs::XMINIMUM);
		myPlot->IsXYRangeValues(true,qDefs::XMINIMUM);
	}
	else
		myPlot->IsXYRangeValues(false,qDefs::XMINIMUM);
	// x max
	if (dispXMax->isEnabled()) {
		myPlot->SetXYRangeValues(dispXMax->text().toDouble(),qDefs::XMAXIMUM);
		myPlot->IsXYRangeValues(true,qDefs::XMAXIMUM);
	}
	else
		myPlot->IsXYRangeValues(false,qDefs::XMAXIMUM);
	// y min
	if (dispYMin->isEnabled()) {
		myPlot->SetXYRangeValues(dispYMin->text().toDouble(),qDefs::YMINIMUM);
		myPlot->IsXYRangeValues(true,qDefs::YMINIMUM);
	}
	else
		myPlot->IsXYRangeValues(false,qDefs::YMINIMUM);
	// y max
	if (dispYMax->isEnabled()) {
		myPlot->SetXYRangeValues(dispYMax->text().toDouble(),qDefs::YMAXIMUM);
		myPlot->IsXYRangeValues(true,qDefs::YMAXIMUM);
	}
	else
		myPlot->IsXYRangeValues(false,qDefs::YMAXIMUM);

	//  To remind the updateplot in qdrawplot to set range after updating plot
	myPlot->SetXYRange(true);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetZRange(){
	emit ResetZMinZMaxSignal(
			(chkZMin->isChecked() && CheckZRange(dispZMin->text())),
			(chkZMax->isChecked() && CheckZRange(dispZMax->text())),
			dispZMin->text().toDouble(),
			dispZMax->text().toDouble());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableZRange(){

	disconnect(dispZMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
	disconnect(dispZMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));

	dispZMin->setEnabled(chkZMin->isChecked());
	dispZMax->setEnabled(chkZMax->isChecked());
	emit ResetZMinZMaxSignal(
			(chkZMin->isChecked() && CheckZRange(dispZMin->text())),
			(chkZMax->isChecked() && CheckZRange(dispZMax->text())),
			dispZMin->text().toDouble(),
			dispZMax->text().toDouble());

	connect(dispZMin, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
	connect(dispZMax, 		SIGNAL(editingFinished()), this, 	SLOT(SetZRange()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


bool qTabPlot::CheckZRange(QString value){
	if(value.isEmpty())
		return false;

	bool ok;
	value.toDouble(&ok);
	if(!ok) {
		qDefs::Message(qDefs::WARNING,"<nobr>Check Z Range</nobr><br><nobr>"
							"Zmin and Zmax should be in double</nobr>","qTabPlot::CheckZRange");
		return false;
	}

	return true;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::SetPlot(){
#ifdef VERBOSE
	cout << "Entering Set Plot()" ;
#endif
	if(radioNoPlot->isChecked()){
		cout << " - No Plot" << endl;

		boxScan->show();
		boxHistogram->hide();
		myPlot->EnablePlot(false);
		boxSnapshot->setEnabled(false);
		boxSave->setEnabled(false);
		boxFrequency->setEnabled(false);
		boxPlotAxis->setEnabled(false);
		boxScan->setEnabled(false);

	}else if(radioDataGraph->isChecked()){
		cout << " - DataGraph" << endl;

		boxScan->show();
		boxHistogram->hide();
		myPlot->EnablePlot(true);
		Select1DPlot(isOriginallyOneD);
		boxSnapshot->setEnabled(true);
		boxSave->setEnabled(true);
		boxFrequency->setEnabled(true);
		boxPlotAxis->setEnabled(true);
		if(!myPlot->isRunning())
			EnableScanBox();
		//  To remind the updateplot in qdrawplot to set range after updating plot
		myPlot->SetXYRange(true);
	}
	else{
		//histogram and 2d scans dont work
		if(boxScan->isChecked()){
			qDefs::Message(qDefs::WARNING,"<nobr>Histogram cannot be used together with 2D Scan Plots.</nobr><br>"
					"<nobr>Uncheck <b>2D Scan</b> plots to plot <b>Histograms</b></nobr>", "qTabPlot::SetPlot");
			radioDataGraph->setChecked(true);
			boxScan->show();
			boxHistogram->hide();
			return;
		}

		cout << " - Histogram" << endl;

		if(radioHistIntensity->isChecked()){
			pageHistogram->setEnabled(true);
			pageHistogram_2->setEnabled(true);
		}else{
			pageHistogram->setEnabled(false);
			pageHistogram_2->setEnabled(false);
		}
		boxScan->hide();
		boxHistogram->show();
		myPlot->EnablePlot(true);
		Select1DPlot(isOriginallyOneD);
		boxSnapshot->setEnabled(true);
		boxSave->setEnabled(true);
		boxFrequency->setEnabled(true);
		boxPlotAxis->setEnabled(true);
		if(!myPlot->isRunning())
			EnableScanBox();

		//qDefs::Message(qDefs::INFORMATION,"<nobr>Please check the <b>Plot Histogram Options</b> below "
		//		"before <b>Starting Acquitision</b></nobr>","qTabPlot::SetPlot");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabPlot::SetFrequency(){
#ifdef VERBOSE
	cout << "Setting Plot Interval Frequency" << endl;
#endif
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
			//to reduce the warnings displayed
			if((comboFrequency->currentIndex() == 0) && (spinTimeGap->value() == minPlotTimer));
			else
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

// void qTabPlot::EnableScanBox(){
// #ifdef VERBOSE
// 	cout << "Entering Enable Scan Box"<< endl;
// #endif
// 	disconnect(btnGroupPlotType,SIGNAL(buttonClicked(int)),this, SLOT(SetPlot()));
// 	disconnect(boxScan,	  	SIGNAL(toggled(bool)),		this, SLOT(EnableScanBox()));

// 	int oldfreqvalue = myDet->setReadReceiverFrequency();

// 	int mode0 = myDet->getScanMode(0);
// 	int mode1 = myDet->getScanMode(1);

// 	radioHistLevel0->setEnabled(mode0);
// 	radioHistLevel1->setEnabled(mode1);
// 	int ang;
// 	bool angConvert = myDet->getAngularConversion(ang);
// 	myPlot->EnableAnglePlot(angConvert);

// 	radioDataGraph->setEnabled(true);
// 	radioHistogram->setEnabled(true);
// 	chkSuperimpose->setEnabled(true);
// 	pageAccumulate->setEnabled(true);
// 	pageAccumulate_2->setEnabled(true);
// 	if((myDet->getDetectorsType() == slsDetectorDefs::GOTTHARD) ||
// 			(myDet->getDetectorsType() == slsDetectorDefs::PROPIX) ||
// 			(myDet->getDetectorsType() == slsDetectorDefs::JUNGFRAU) ||
// 			(myDet->getDetectorsType() == slsDetectorDefs::CHIPTESTBOARD) ||
// 			(myDet->getDetectorsType() == slsDetectorDefs::MOENCH)){
// 		pagePedestal->setEnabled(true);
// 		pagePedestal_2->setEnabled(true);
// 		chkBinary->setEnabled(true);
// 		chkBinary_2->setEnabled(true);
// 	}




// 	//if angle plot or originally 2d, uncheck and disable scanbox
// 	if ((angConvert) || (!isOriginallyOneD)){
// 		boxScan->setChecked(false);
// 		boxScan->setEnabled(false);

// 		/**Newly added*/
// 		// To remind the updateplot in qdrawplot to set range after updating plot
// 		if(!isOriginallyOneD)
// 			myPlot->SetXYRange(true);

// 		//2d scans read every frame, not compulsory, but for historgrams
// 		if((!isOriginallyOneD) && (mode0 || mode1)){
// 			//read every frame
// 			disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 			disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 			comboFrequency->setCurrentIndex(1);
// 			spinNthFrame->setValue(1);
// 			SetFrequency();
// 			connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 			connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 		}

// 		//persistency, accumulate, pedestal, binary
// 		if(angConvert){
// 			if(chkSuperimpose->isChecked())	chkSuperimpose->setChecked(false);
// 			if(chkPedestal->isChecked())	chkPedestal->setChecked(false);
// 			if(chkPedestal_2->isChecked())	chkPedestal_2->setChecked(false);
// 			if(chkAccumulate->isChecked())	chkAccumulate->setChecked(false);
// 			if(chkAccumulate_2->isChecked())chkAccumulate_2->setChecked(false);
// 			if(chkBinary->isChecked())		chkBinary->setChecked(false);
// 			if(chkBinary_2->isChecked())	chkBinary_2->setChecked(false);
// 			pagePedestal->setEnabled(false);
// 			pagePedestal_2->setEnabled(false);
// 			chkBinary->setEnabled(false);
// 			chkBinary_2->setEnabled(false);
// 			pageAccumulate->setEnabled(false);
// 			pageAccumulate_2->setEnabled(false);
// 		}

// 		if(angConvert){
// 			boxScan->setToolTip("<nobr>Only 1D Plots enabled for Angle Plots</nobr>");
// 			//disable histogram
// 			if(radioHistogram->isChecked()){
// 				radioDataGraph->setChecked(true);
// 				radioHistogram->setEnabled(false);
// 				//  To remind the updateplot in qdrawplot to set range after updating plot
// 				myPlot->SetXYRange(true);
// 				boxScan->show();
// 				boxHistogram->hide();
// 			}
// 		}
// 	}




// 	//originally1d && not angle plot
// 	else{
// 		boxScan->setToolTip("");
// 		boxScan->setEnabled(true);
// 		/*if(mode0 || mode1)
// 			boxScan->setChecked(true);*/

// 		//2d enabled with boxscan
// 		if(boxScan->isChecked()){

// 			//2d for 1d detctors and histogram dont go
// 			if(radioHistogram->isChecked()){
// 				radioDataGraph->setChecked(true);
// 				//  To remind the updateplot in qdrawplot to set range after updating plot
// 				myPlot->SetXYRange(true);
// 				boxScan->show();
// 				boxHistogram->hide();
// 			}

// 			//read every frame
// 			disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 			disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 			comboFrequency->setCurrentIndex(1);
// 			spinNthFrame->setValue(1);
// 			SetFrequency();
// 			connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 			connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));

// 			//enabling options
// 			radioFileIndex->setEnabled(mode0||mode1);
// 			if(mode0 && mode1){
// 				radioLevel0->setEnabled(false);
// 				radioLevel1->setEnabled(false);
// 			}else{
// 				radioLevel0->setEnabled(mode0);
// 				radioLevel1->setEnabled(mode1);
// 			}
// 			//default is allframes if checked button is disabled
// 			if(!btnGroupScan->checkedButton()->isEnabled())
// 				radioAllFrames->setChecked(true);
// 		}
// 	}


// 	//histogram
// 	if(radioHistogram->isChecked()){
// 		if(radioHistIntensity->isChecked()){
// 			pageHistogram->setEnabled(true);
// 			pageHistogram_2->setEnabled(true);
// 		}else{
// 			pageHistogram->setEnabled(false);
// 			pageHistogram_2->setEnabled(false);
// 		}
// 		stackedWidget->setCurrentIndex(stackedWidget->count()-1);
// 		stackedWidget_2->setCurrentIndex(stackedWidget_2->count()-1);
// 		box1D->setTitle(QString("1D Plot Options %1 - Histogram").arg(stackedWidget->currentIndex()+1));
// 		box2D->setTitle(QString("2D Plot Options %1 - Histogram").arg(stackedWidget_2->currentIndex()+1));

// 		if(chkSuperimpose->isChecked())	chkSuperimpose->setChecked(false);
// 		if(chkPedestal->isChecked())	chkPedestal->setChecked(false);
// 		if(chkPedestal_2->isChecked())	chkPedestal_2->setChecked(false);
// 		if(chkAccumulate->isChecked())	chkAccumulate->setChecked(false);
// 		if(chkAccumulate_2->isChecked())chkAccumulate_2->setChecked(false);
// 		if(chkBinary->isChecked())		chkBinary->setChecked(false);
// 		if(chkBinary_2->isChecked())	chkBinary_2->setChecked(false);
// 		pagePedestal->setEnabled(false);
// 		pagePedestal_2->setEnabled(false);
// 		chkBinary->setEnabled(false);
// 		chkBinary_2->setEnabled(false);
// 		pageAccumulate->setEnabled(false);
// 		pageAccumulate_2->setEnabled(false);

// 		//read every frame
// 		disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 		disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 		comboFrequency->setCurrentIndex(1);
// 		spinNthFrame->setValue(1);
// 		SetFrequency();
// 		connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 		connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));


// 	}else{
// 		pageHistogram->setEnabled(false);
// 		pageHistogram_2->setEnabled(false);
// 	}


// 	// if it was set to read every frame
// 	if (oldfreqvalue != 0 && (comboFrequency->currentIndex() != 1 || spinNthFrame->value() != oldfreqvalue)) {
// 		disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 		disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 		comboFrequency->setCurrentIndex(1);
// 		spinNthFrame->setValue(1);
// 		SetFrequency();
// 		connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
// 		connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
// 	}

// 	connect(btnGroupPlotType,SIGNAL(buttonClicked(int)),this, SLOT(SetPlot()));
// 	connect(boxScan,	 	SIGNAL(toggled(bool)),	this, SLOT(EnableScanBox()));

// }


//-------------------------------------------------------------------------------------------------------------------------------------------------

// void qTabPlot::SetScanArgument(){
// #ifdef VERYVERBOSE
// 	cout << "Entering qTabPlot::SetScanArgument()" << endl;
// #endif

// 	//1d
// 	if(isOriginallyOneD){
// 		dispXAxis->setText(defaultHistXAxisTitle);
// 		dispYAxis->setText(defaultHistYAxisTitle);
// 		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
// 		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
// 		Select1DPlot(true);
// 	}
// 	//2d
// 	else{
// 		dispXAxis->setText(defaultImageXAxisTitle);
// 		dispYAxis->setText(defaultImageYAxisTitle);
// 		dispZAxis->setText(defaultImageZAxisTitle);
// 		myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
// 		myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
// 		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
// 		Select1DPlot(false);
// 	}


// 	//histogram default  - set before setscanargument
// 	int min = spinHistFrom->value();
// 	int max = spinHistTo->value();
// 	double size = spinHistSize->value();
// 	int histArg = qDefs::Intensity;
// 	if(radioHistogram->isChecked()){
// 		if(!radioHistIntensity->isChecked()){

// 			int mode = 0;
// 			histArg = qDefs::histLevel0;
// 			if(radioHistLevel1->isChecked()){
// 				mode = 1;
// 				histArg = qDefs::histLevel1;
// 			}


// 			int numSteps = myDet->getScanSteps(mode);
// 			double *values = NULL;
// 			min = 0;max = 1;size = 1;

// 			if(numSteps > 0){
// 				values = new double[numSteps];
// 				myDet->getScanSteps(mode,values);
// 				min = values[0];
// 				max = values[numSteps - 1];
// 				size = (max - min)/(numSteps - 1);
// 			}
// 		}

// 	}

// 	//cout <<"min:"<<min<<" max:"<<max<<" size:"<<size<<endl;
// 	myPlot->SetHistogram(radioHistogram->isChecked(),histArg,min,max,size);


// 	if(radioHistogram->isChecked()){
// 		if(radioHistIntensity->isChecked())
// 			dispXAxis->setText("Intensity");
// 		else if (radioHistLevel0->isChecked())
// 			dispXAxis->setText("Level 0");
// 		else
// 			dispXAxis->setText("Level 1");
// 		dispYAxis->setText("Frequency");
// 		myPlot->SetHistXAxisTitle("Intensity");
// 		myPlot->SetHistYAxisTitle("Frequency");
// 		Select1DPlot(true);
// 	}

// 	//angles (1D)
// 	int ang;
// 	if(myDet->getAngularConversion(ang)){
// 		dispXAxis->setText("Angles");
// 		myPlot->SetHistXAxisTitle("Angles");
// 		Select1DPlot(true);
// 	}


// 	//1d with scan
// 	if(boxScan->isChecked()){
// 		myPlot->SetScanArgument(btnGroupScan->checkedId()+1);

// 		switch(btnGroupScan->checkedId()){
// 		case 0://level0
// 			dispYAxis->setText("Scan Level 0");
// 			myPlot->SetImageYAxisTitle("Scan Level 0");
// 			break;
// 		case 1://level1
// 			dispYAxis->setText("Scan Level 1");
// 			myPlot->SetImageYAxisTitle("Scan Level 1");
// 			break;
// 			break;
// 		case 2://file index
// 			dispYAxis->setText("Frame Index");
// 			myPlot->SetImageYAxisTitle("Frame Index");
// 			break;
// 		case 3://all frames
// 			dispYAxis->setText("All Frames");
// 			myPlot->SetImageYAxisTitle("All Frames");
// 			break;
// 		}
// 		Select1DPlot(false);
// 	}else
// 		myPlot->SetScanArgument(qDefs::None);

// 	//update the from and to labels to be enabled
// 	SetBinary();

// 	qDefs::checkErrorMessage(myDet,"qTabPlot::SetScanArgument");

// }


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


void qTabPlot::SetHistogramOptions(){
	if(radioHistIntensity->isChecked()){
		pageHistogram->setEnabled(true);
		pageHistogram_2->setEnabled(true);
	}else {
		pageHistogram->setEnabled(false);
		pageHistogram_2->setEnabled(false);
	}
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabPlot::EnableGapPixels(bool enable) {
#ifdef VERBOSE
    cout  << "Setting Gap pixels to " << enable << endl;
#endif
    disconnect(chkGapPixels, SIGNAL(toggled(bool)),this, SLOT(EnableGapPixels(bool)));

    myDet->enableGapPixels(enable);
    int ret =  myDet->enableGapPixels(-1);
    qDefs::checkErrorMessage(myDet,"qTabPlot::SetScanArgument");
    chkGapPixels->setChecked((ret == 1) ? true : false);

    connect(chkGapPixels, SIGNAL(toggled(bool)),this, SLOT(EnableGapPixels(bool)));
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
		/*if(myDet->getDetectorsType() == slsDetectorDefs::EIGER)
			comboFrequency->setCurrentIndex(1);*/
		SetFrequency();

	    if (chkGapPixels->isEnabled()) {
	        disconnect(chkGapPixels, SIGNAL(toggled(bool)),this, SLOT(EnableGapPixels(bool)));
	        int ret =  myDet->enableGapPixels(-1);
	        qDefs::checkErrorMessage(myDet,"qTabPlot::Refresh");
	        chkGapPixels->setChecked((ret == 1) ? true : false);
	        connect(chkGapPixels, SIGNAL(toggled(bool)),this, SLOT(EnableGapPixels(bool)));
	    }

	}else{
		boxFrequency->setEnabled(false);
		disconnect(boxScan,	  SIGNAL(toggled(bool)),				   this, SLOT(EnableScanBox()));
		boxScan->setEnabled(false);
		pageHistogram->setEnabled(false);
		pageHistogram_2->setEnabled(false);
		if(radioHistogram->isChecked())
			radioDataGraph->setEnabled(false);
		else
			radioHistogram->setEnabled(false);
	}
#ifdef VERBOSE
	cout  << "**Updated Plot Tab" << endl << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


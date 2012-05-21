/*
 * qTabMeasurement.cpp
 *
 *  Created on: May 2, 2012
 *      Author: l_maliakal_d
 */

/** Qt Project Class Headers */
#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0
#define UndefinedSettings 7



qTabMeasurement::qTabMeasurement(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot):QWidget(parent),myDet(detector),myPlot(plot){
	setupUi(this);
	if(myDet)
	{
		SetupWidgetWindow();
		Initialization();
	}
}




qTabMeasurement::~qTabMeasurement(){
	delete myDet;
	delete myPlot;
}




void qTabMeasurement::SetupWidgetWindow(){
	/** all set initially to reflect the detector's actual parameter values*/

	/** Settings */
	comboSettings->setCurrentIndex(myDet->getSettings(Detector_Index));	//set it to default acc to detector???
	/** Number of Measurements/Frames*/
	setNumFrames(2000);
	/** Acquisition Time */
	setAcquisitionTime();
	/** Frame Period between exposures*/
	setFramePeriod();
	/** File Name*/
	setFileName("run");
	/** File Index*/
	setRunIndex(0);
}



void qTabMeasurement::Initialization(){
	/** Settings */
	connect(comboSettings,SIGNAL(currentIndexChanged(int)),this,SLOT(setSettings(int)));
	/** Number of Measurements/Frames*/
	connect(spinNumMeasurements,SIGNAL(valueChanged(int)),this,SLOT(setNumFrames(int)));
	/** Acquisition Time */
	connect(spinExpTime,SIGNAL(valueChanged(double)),this,SLOT(setAcquisitionTime()));
	connect(comboExpUnit,SIGNAL(currentIndexChanged(int)),this,SLOT(setAcquisitionTime()));
	/** Frame Period between exposures*/
	connect(spinPeriod,SIGNAL(valueChanged(double)),this,SLOT(setFramePeriod()));
	connect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),this,SLOT(setFramePeriod()));
	/** File Name*/
	connect(dispFileName,SIGNAL(textChanged(const QString&)),this,SLOT(setFileName(const QString&)));
	/** File Index*/
	connect(spinIndex,SIGNAL(valueChanged(int)),this,SLOT(setRunIndex(int)));
	/** Start/Stop Acquisition*/
	connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
}




//enabled other tabs as well??
void qTabMeasurement::Enable(bool enable){
	//this->setEnabled(enable);
	comboSettings->setEnabled(enable);
	spinNumMeasurements->setEnabled(enable);
	spinExpTime->setEnabled(enable);
	comboExpUnit->setEnabled(enable);
	spinPeriod->setEnabled(enable);
	comboPeriodUnit->setEnabled(enable);
	dispFileName->setEnabled(enable);
	spinIndex->setEnabled(enable);
	if(!enable) btnStartStop->setEnabled(true);
}




void qTabMeasurement::startStopAcquisition(){
	if(!btnStartStop->text().compare("Start")){
#ifdef VERBOSE
		cout<<endl<<endl<<"Starting Acquisition"<<endl;
#endif
		btnStartStop->setText("Stop");
		Enable(0);
	}else{
#ifdef VERBOSE
		cout<<"Stopping Acquisition"<<endl;
#endif
		btnStartStop->setText("Start");
		Enable(1);
	}
	myPlot->StartStopDaqToggle();
}


void qTabMeasurement::UpdateFinished(){
	disconnect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	btnStartStop->setText("Start");
	Enable(1);
	connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
}

void qTabMeasurement::setSettings(int index){
	slsDetectorDefs::detectorSettings sett = myDet->setSettings((slsDetectorDefs::detectorSettings)index,Detector_Index);
#ifdef VERBOSE
	cout<<"Settings have been set to "<<myDet->slsDetectorBase::getDetectorSettings(sett)<<endl;
#endif

}


void qTabMeasurement::setNumFrames(int val){
	myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,val);
#ifdef VERBOSE
	cout<<"Setting Frame number to " << (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1)<<endl;
#endif
}


void qTabMeasurement::setAcquisitionTime(){
	int64_t exptime64;
	/** Get the 64 bit value of timer*/
	exptime64 = qDefs::get64bTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout<<"Setting acquisition time to " << exptime64 << " clocks" << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,exptime64);

	//float t=exptime64;
	//emit acquisitionTimeChanged(t/(100E+6)); ??????????????????????
}



void qTabMeasurement::setFramePeriod(){
	int64_t exptime64;
	/** Get the 64 bit value of timer*/
	exptime64 = qDefs::get64bTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
#ifdef VERBOSE
	cout<<"Setting frame period between exposures to " << exptime64 << " clocks" << endl;
#endif
	myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,exptime64);

	//float t=exptime64;
	//emit acquisitionTimeChanged(t/(100E+6)); ??????????????????????
}



void qTabMeasurement::setFileName(const QString& fName){
	//  emit fileNameChanged(fName);
	//  thred-->fileName=s;myDet->setFileName(fName.ascii());
#ifdef VERBOSE
	cout<<"Setting File name to " << myDet->getFileName()<<endl;
#endif
	myDet->setFileName(fName.toAscii().data());
}




void qTabMeasurement::setRunIndex(int index){
	myDet->setFileIndex(index);
#ifdef VERBOSE
	cout<<"Setting File Index to " << myDet->getFileIndex()<<endl;
#endif
}




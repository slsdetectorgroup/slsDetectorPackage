/*
 * qTabMeasurement.cpp
 *
 *  Created on: May 2, 2012
 *      Author: l_maliakal_d
 */

//Qt Project Class Headers
#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"
//Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
//Qt Include Headers
#include <QStandardItemModel>
//C++ Include Headers
#include<iostream>
using namespace std;





//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabMeasurement::qTabMeasurement(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot):
								QWidget(parent),myDet(detector),myPlot(plot),expertMode(false){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabMeasurement::~qTabMeasurement(){
	delete myDet;
	delete myPlot;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetupWidgetWindow(){
	//Number of measurements
	numMeasurement=1;

	//Timer to update the progress bar
	progressTimer = new QTimer(this);
	//btnStartStop->setStyleSheet("color:green");
	//Exp Time
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))));
	spinExpTime->setValue(time);
	comboExpUnit->setCurrentIndex((int)unit);
	//Hide the error message
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	acqPeriodTip = spinPeriod->toolTip();
	errPeriodTip = QString("<nobr>Frame period between exposures.</nobr><br>"
			"<nobr> #period#</nobr><br><br>")+
			QString("<nobr><font color=\"red\"><b>Acquisition Period</b> should be"
					" greater than or equal to <b>Exposure Time</b>.</font></nobr>");
	//File Name
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	//File Index
	spinIndex->setValue(myDet->getFileIndex());
	//only initially
	lblProgressIndex->setText(QString::number(myDet->getFileIndex()));
	//ly initially
	progressBar->setValue(0);
	currentMeasurement = 0;

	//timing mode
	SetupTimingMode();

	//file write enabled/disabled
	chkFile->setChecked(myDet->enableWriteToFile());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetupTimingMode(){
	//Get timing mode from detector
	slsDetectorDefs::externalCommunicationMode mode = myDet->setExternalCommunicationMode();

	//To be able to index items on a combo box
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboTimingMode->model());
	QModelIndex index[NumTimingModes];
	QStandardItem* item[NumTimingModes];
	if (model) {
		for(int i=0;i<NumTimingModes;i++){
			index[i] = model->index(i,	comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}
		//Enabling/Disabling depending on the detector type
		switch(myDet->getDetectorsType()){
		case slsDetectorDefs::MYTHEN:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(false);
			item[(int)Trigger_Readout]->setEnabled(true);
			item[(int)Gated]->setEnabled(true);
			item[(int)Gated_Start]->setEnabled(true);
			item[(int)Trigger_Window]->setEnabled(false);
			break;
		case slsDetectorDefs::EIGER:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(true);
			item[(int)Trigger_Readout]->setEnabled(false);
			item[(int)Gated]->setEnabled(false);
			item[(int)Gated_Start]->setEnabled(false);
			item[(int)Trigger_Window]->setEnabled(true);
			break;
		case slsDetectorDefs::GOTTHARD:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(false);
			item[(int)Trigger_Readout]->setEnabled(false);
			item[(int)Gated]->setEnabled(false);
			item[(int)Gated_Start]->setEnabled(false);
			item[(int)Trigger_Window]->setEnabled(false);
			break;
		default:
			qDefs::ErrorMessage("Unknown detector type.","Measurement");
			exit(-1);
			break;
		}
		//Setting the timing mode
		if(item[mode]->isEnabled()){
			//if the timing mode is Auto and
			// number of Frames and number of triggers is 1,
			// then the timing mode is 'None'.
			// This is for the inexperienced user
			if(mode==slsDetectorDefs::AUTO_TIMING){
				int frames = (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1);
				int triggers = (int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1);
				if((frames==1)&&(triggers==1)){
					comboTimingMode->setCurrentIndex((int)None);
					SetTimingMode((int)None);
				}else{
					comboTimingMode->setCurrentIndex((int)Auto);
					SetTimingMode((int)Auto);
				}
			}else{
				//mode +1 since the detector class has no timingmode as "None"
				comboTimingMode->setCurrentIndex((int)mode+1);
				SetTimingMode((int)mode+1);
			}
		}
		// Mode NOT ENABLED.
		// This should not happen -only if the server and gui has a mismatch
		// on which all modes are allowed in detectors
		else{
			qDefs::WarningMessage("Unknown Timing Mode detected from detector."
					"\n\nSetting the following defaults:\nTiming Mode \t: None\n"
					"Number of Frames \t: 1\nNumber of Triggers \t: 1","Measurement");
			comboTimingMode->setCurrentIndex((int)None);
			SetTimingMode((int)None);
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::Initialization(int timingChange){
	//These signals are connected only at start up. The others are reinitialized when changing timing mode
	if(!timingChange){
		//Number of Measurements
		connect(spinNumMeasurements,SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));
		//File Name
		connect(dispFileName,		SIGNAL(textChanged(const QString&)),this,	SLOT(setFileName(const QString&)));
		//File Index
		connect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
		//Start/Stop Acquisition
		connect(btnStartStop,		SIGNAL(clicked()),					this,	SLOT(startStopAcquisition()));
		//Timing Mode
		connect(comboTimingMode,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetTimingMode(int)));//
		//progress bar
		connect(progressTimer, 		SIGNAL(timeout()), 					this, SLOT(UpdateProgress()));
		//enable write to file
		connect(chkFile, 			SIGNAL(toggled(bool)), 				this, SLOT(EnableFileWrite(bool)));
	}
	//Number of Frames
	connect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	//Exposure Time
	connect(spinExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(setExposureTime()));//..myplot
	connect(comboExpUnit,SIGNAL(currentIndexChanged(int)),		this,	SLOT(setExposureTime()));
	//Frame Period between exposures
	connect(spinPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(setAcquisitionPeriod()));//..myplot
	connect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
	//Number of Triggers
	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));//
	//Delay After Trigger
	connect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));//
	connect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	//Number of Gates
	connect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));//
	//Number of Probes
	connect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));//
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::DeInitialization(){
	//Number of Frames
	disconnect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	//Exposure Time
	disconnect(spinExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(setExposureTime()));//..myplot
	disconnect(comboExpUnit,SIGNAL(currentIndexChanged(int)),		this,	SLOT(setExposureTime()));
	//Frame Period between exposures
	disconnect(spinPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(setAcquisitionPeriod()));//..myplot
	disconnect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
	//Number of Triggers
	disconnect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
	//Delay After Trigger
	disconnect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));
	disconnect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	//Number of Gates
	disconnect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));
	//Number of Probes
	disconnect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::Enable(bool enable){
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);
	//Enable this always
	if(!enable) btnStartStop->setEnabled(true);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumMeasurements(int val){
#ifdef VERBOSE
	cout << "Setting Number of Measurements to "  << val << endl;
#endif
	numMeasurement = val;
	myPlot->setNumMeasurements(val);
}
//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setFileName(const QString& fName){
	myDet->setFileName(fName.toAscii().data());
#ifdef VERBOSE
	cout << "Setting File name to "  <<  myDet->getFileName() << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMeasurement::setRunIndex(int index){
	myDet->setFileIndex(index);
	lblProgressIndex->setText(QString::number(index));
#ifdef VERBOSE
	cout << "Setting File Index to " << myDet->getFileIndex() << endl;
#endif
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::startStopAcquisition(){
	if(!btnStartStop->text().compare("Start")){
#ifdef VERBOSE
		cout << endl << endl << "Starting Acquisition" << endl;
#endif
		//btnStartStop->setStyleSheet("color:red");
		btnStartStop->setText("Stop");
		Enable(0);
		progressBar->setValue(0);
		//the progress which keeps adding up for all the measurements
		currentMeasurement = 0;
		progressTimer->start(100);

		emit StartSignal();
		myPlot->StartStopDaqToggle();
	}else{
#ifdef VERBOSE
		cout << "Stopping Acquisition" << endl<< endl;
#endif
		myDet->stopAcquisition();
		//btnStartStop->setStyleSheet("color:green");
		//btnStartStop->setStyleSheet("background:rgb(239,239,239)");
		progressTimer->stop();
		btnStartStop->setText("Start");
		Enable(1);
		emit StopSignal();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::UpdateFinished(){
	disconnect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	btnStartStop->setText("Start");
	//btnStartStop->setStyleSheet("color:green");
	//btnStartStop->setStyleSheet("background:rgb(239,239,239)");
	Enable(1);
	connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	UpdateProgress();
	progressTimer->stop();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetCurrentMeasurement(int val){
	currentMeasurement = val;
	lblCurrentMeasurement->setText(QString::number(val+1));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::UpdateProgress(){
	progressBar->setValue((int)(((currentMeasurement*100)+(myPlot->GetProgress()))/numMeasurement));
	lblProgressIndex->setText(QString::number(myDet->getFileIndex()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumFrames(int val){
	myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,val);
#ifdef VERBOSE
	cout << "Setting number of frames to " << (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1) << endl;
#endif

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setExposureTime(){
	double exptimeNS;
	//Get the value of timer in ns
	exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout << "Setting acquisition time to " << exptimeNS << " clocks" << "/" << spinExpTime->value() << qDefs::getUnitString((qDefs::timeUnit)comboExpUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,(int64_t)exptimeNS);

	if(lblPeriod->isEnabled()){
		double acqtimeNS;
		acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
		if(exptimeNS>acqtimeNS) {
			spinPeriod->setToolTip(errPeriodTip);
			lblPeriod->setToolTip(errPeriodTip);
			lblPeriod->setPalette(red);
			lblPeriod->setText("Acquisition Period:*");
		}
		else {
			spinPeriod->setToolTip(acqPeriodTip);
			lblPeriod->setToolTip(acqPeriodTip);
			lblPeriod->setPalette(lblTimingMode->palette());
			lblPeriod->setText("Acquisition Period:");
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setAcquisitionPeriod(){
	double acqtimeNS;
	//Get the value of timer in ns
	acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
#ifdef VERBOSE
	cout << "Setting frame period between exposures to " << acqtimeNS << " clocks"<< "/" << spinPeriod->value() << qDefs::getUnitString((qDefs::timeUnit)comboPeriodUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,(int64_t)acqtimeNS);

	double exptimeNS;
	exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
	if(exptimeNS>acqtimeNS){
		spinPeriod->setToolTip(errPeriodTip);
		lblPeriod->setToolTip(errPeriodTip);
		lblPeriod->setPalette(red);
		lblPeriod->setText("Acquisition Period:*");
		emit EnableNthFrameSignal(false);
	}
	else {
		spinPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setPalette(lblTimingMode->palette());
		lblPeriod->setText("Acquisition Period:");
		emit EnableNthFrameSignal(true);
	}


	//Check if the interval between plots is ok
	emit CheckPlotIntervalSignal();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumTriggers(int val){
	myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,val);
#ifdef VERBOSE
	cout << "Setting number of triggers to " << (int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1) << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setDelay(){
	double exptimeNS;
	//Get the value of timer in ns
	exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboDelayUnit->currentIndex(),spinDelay->value());
#ifdef VERBOSE
	cout << "Setting delay after trigger to " << exptimeNS << " clocks" <<  "/" << spinDelay->value() << qDefs::getUnitString((qDefs::timeUnit)comboDelayUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,(int64_t)exptimeNS);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumGates(int val){
	myDet->setTimer(slsDetectorDefs::GATES_NUMBER,val);
#ifdef VERBOSE
	cout << "Setting number of gates to " << (int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1) << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumProbes(int val){
	myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,val);
#ifdef VERBOSE
	cout << "Setting number of frames to " << (int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1) << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetTimingMode(int mode){
#ifdef VERBOSE
	cout << "Setting Timing mode to " << comboTimingMode->currentText().toAscii().data() << endl;
#endif
	//Default settings
	lblNumFrames->setEnabled(false);	spinNumFrames->setEnabled(false);
	lblExpTime->setEnabled(false);		spinExpTime->setEnabled(false);			comboExpUnit->setEnabled(false);
	lblPeriod->setEnabled(false);		spinPeriod->setEnabled(false);			comboPeriodUnit->setEnabled(false);
	lblNumTriggers->setEnabled(false);	spinNumTriggers->setEnabled(false);
	lblDelay->setEnabled(false);		spinDelay->setEnabled(false);			comboDelayUnit->setEnabled(false);
	lblNumGates->setEnabled(false);		spinNumGates->setEnabled(false);
	lblNumProbes->setEnabled(false);	spinNumProbes->setEnabled(false);

	bool success = false;
	switch(mode){
	case None://Exposure Time
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		setNumFrames(1);
		setNumTriggers(1);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::AUTO_TIMING)==slsDetectorDefs::AUTO_TIMING)
			success = true;
		break;
	case Auto://#Frames, ExpTime, Period
		setNumTriggers(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::AUTO_TIMING)==slsDetectorDefs::AUTO_TIMING)
			success = true;
		break;
	case Trigger_Exp_Series://#Frames, #Triggers, ExpTime, Period, Delay
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		lblDelay->setEnabled(true);			spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_EXPOSURE)==slsDetectorDefs::TRIGGER_EXPOSURE)
			success = true;
		break;
	case Trigger_Frame://ExpTime, #Triggers
		setNumFrames(1);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_FRAME)==slsDetectorDefs::TRIGGER_FRAME)
			success = true;
		break;
	case Trigger_Readout://#Frames, ExpTime, Period, Delay
		setNumTriggers(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblDelay->setEnabled(true);			spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_READOUT)==slsDetectorDefs::TRIGGER_READOUT)
			success = true;
		break;
	case Gated://#Frames, #Gates
		setNumTriggers(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblNumGates->setEnabled(true);		spinNumGates->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::GATE_FIX_NUMBER)==slsDetectorDefs::GATE_FIX_NUMBER)
			success = true;
		break;
	case Gated_Start://#Frames, #Triggers, #Gates, ExpTime, Period
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		lblNumGates->setEnabled(true);		spinNumGates->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::GATE_WITH_START_TRIGGER)==slsDetectorDefs::GATE_WITH_START_TRIGGER)
			success = true;
		break;
	case Trigger_Window://#Triggers
		setNumFrames(1);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_WINDOW)==slsDetectorDefs::TRIGGER_WINDOW)
			success = true;
		break;
	default:
		//This should never happen
		qDefs::ErrorMessage("Timing mode unknown to GUI","Measurement");
		exit(-1);
	}
	if(!success){
		qDefs::WarningMessage("The detector timing mode could not be set.\n"
				"Please check the external flags."
				"\n\nSetting the following defaults:\nTiming Mode \t: None\n"
				"Number of Frames \t: 1\nNumber of Triggers \t: 1","Measurement");
		comboTimingMode->setCurrentIndex((int)None);
		return;
	}


	if(mode!=None){//Number of Probes
		if((myDet->getDetectorsType()==slsDetectorDefs::MYTHEN)&&(expertMode)){
			lblNumProbes->setEnabled(true);		spinNumProbes->setEnabled(true);
		}
	}

	//To disconnect all the signals before changing their values
	DeInitialization();


	double time;
	int val;
	qDefs::timeUnit unit;
	//Number of Frames
	if(lblNumFrames->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1);
		spinNumFrames->setValue(val);
#ifdef VERBOSE
		cout << "Getting number of frames : " << val <<endl;
#endif
	}

	//Exposure Time
	if(lblExpTime->isEnabled()){
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))));
#ifdef VERBOSE
		cout << "Getting acquisition time : " << time << qDefs::getUnitString(unit) << endl;
#endif
		spinExpTime->setValue(time);
		comboExpUnit->setCurrentIndex((int)unit);
	}

	//Frame Period between exposures
	if(lblPeriod->isEnabled()){
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-9))));
#ifdef VERBOSE
		cout << "Getting frame period between exposures : " << time <<  qDefs::getUnitString(unit) << endl;
#endif
		spinPeriod->setValue(time);
		comboPeriodUnit->setCurrentIndex((int)unit);

		double exptimeNS,acqtimeNS;
		exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
		acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
		if(exptimeNS>acqtimeNS) {

			spinPeriod->setToolTip(errPeriodTip);
			lblPeriod->setToolTip(errPeriodTip);
			lblPeriod->setPalette(red);
			lblPeriod->setText("Acquisition Period:*");
		}
		else {

			spinPeriod->setToolTip(acqPeriodTip);
			lblPeriod->setToolTip(acqPeriodTip);
			lblPeriod->setPalette(lblTimingMode->palette());
			lblPeriod->setText("Acquisition Period:");
		}
	}else	{
		spinPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setPalette(lblTimingMode->palette());
		lblPeriod->setText("Acquisition Period:");
	}

	//Number of Triggers
	if(lblNumTriggers->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1);
		spinNumTriggers->setValue(val);
#ifdef VERBOSE
		cout << "Getting number of triggers : " << val <<endl;
#endif
	}

	//Delay After Trigger
	if(lblDelay->isEnabled()){
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,-1)*(1E-9))));
#ifdef VERBOSE
		cout << "Getting delay after trigger : " << time <<  qDefs::getUnitString(unit) << endl;
#endif
		spinDelay->setValue(time);
		comboDelayUnit->setCurrentIndex((int)unit);
	}

	//Number of Gates
	if(lblNumGates->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1);
		spinNumGates->setValue(val);
#ifdef VERBOSE
		cout << "Getting number of gates : " << val << endl;
#endif
	}

	// Number of Probes
	if(lblNumProbes->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1);
		spinNumProbes->setValue(val);
#ifdef VERBOSE
		cout << "Getting number of probes : " << val << endl;
#endif
	}

	//To reconnect all the signals after changing their values
	Initialization(1);


	// to let qdrawplot know that triggers or frames are used
	myPlot->setFrameEnabled(lblNumFrames->isEnabled());
	myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());


	//check if period is enabled and alright, only then the nth frame frequency is enabled
	if((lblPeriod->isEnabled())&&(lblPeriod->text()=="Acquisition Period:"))
		emit EnableNthFrameSignal(true);
	else emit EnableNthFrameSignal(false);

	return;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::EnableFileWrite(bool enable){
#ifdef VERBOSE
	cout << "Enable File Write:" << enable << endl;
#endif
	myDet->enableWriteToFile(enable);
	dispFileName->setEnabled(enable);
	if(enable) setFileName(dispFileName->text());
};

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::Refresh(){
	//File Name
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	//File Index
	spinIndex->setValue(myDet->getFileIndex());
	//progress label index
	lblProgressIndex->setText(QString::number(myDet->getFileIndex()));
	//Timing mode*
	SetupTimingMode();

	// to let qdrawplot know that triggers or frames are used
	myPlot->setFrameEnabled(lblNumFrames->isEnabled());
	myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());

}


//-------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * qTabMeasurement.cpp
 *
 *  Created on: May 2, 2012
 *      Author: l_maliakal_d
 */

//Qt Project Class Headers
#include "qTabMeasurement.h"
#include "qDetectorMain.h"
//Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
//C++ Include Headers
#include<iostream>
using namespace std;





//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabMeasurement::qTabMeasurement(qDetectorMain *parent,multiSlsDetector*& detector, qDrawPlot*& plot):
		thisParent(parent),myDet(detector),myPlot(plot),expertMode(false){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
	SetupTimingMode();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabMeasurement::~qTabMeasurement(){
	delete myDet;
	delete myPlot;
	delete thisParent;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetupWidgetWindow(){

	detType = myDet->getDetectorsType();

	//Number of measurements
	spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,-1));
	//Number of frames
	spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
	//Exp Time
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))));
	spinExpTime->setValue(time);
	comboExpUnit->setCurrentIndex((int)unit);
	//period
	time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-9))));
	spinPeriod->setValue(time);
	comboPeriodUnit->setCurrentIndex((int)unit);
	//Number of Triggers
	spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
	//delay
	time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,-1)*(1E-9))));
	spinDelay->setValue(time);
	comboDelayUnit->setCurrentIndex((int)unit);
	//gates
	spinNumGates->setValue((int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1));
	//probes
	if(detType == slsDetectorDefs::MYTHEN)
		spinNumProbes->setValue((int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1));
	//File Name
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	//File Index
	spinIndex->setValue(myDet->getFileIndex());
	//only initially
	lblProgressIndex->setText(QString::number(0));
	//ly initially
	progressBar->setValue(0);
	//file write enabled/disabled
	chkFile->setChecked(myDet->enableWriteToFile());
	dispFileName->setEnabled(myDet->enableWriteToFile());


	//creating the icons for the buttons
	iconStart = new QIcon(":/icons/images/start.png");
	iconStop = new QIcon(":/icons/images/stop.png");


	//Timer to update the progress bar
	progressTimer = new QTimer(this);

	//Hide the error message
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	acqPeriodTip = spinPeriod->toolTip();
	errPeriodTip = QString("<nobr>Frame period between exposures.</nobr><br>"
			"<nobr> #period#</nobr><br><br>")+
			QString("<nobr><font color=\"red\"><b>Acquisition Period</b> should be"
					" greater than or equal to <b>Exposure Time</b>.</font></nobr>");

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::SetupWidgetWindow");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetExpertMode(bool enable){
	expertMode = enable;
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::SetExpertMode");

	EnableProbes();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetupTimingMode(){
	//To be able to index items on a combo box
	model = qobject_cast<QStandardItemModel*>(comboTimingMode->model());
	QModelIndex index[NumTimingModes];
	QStandardItem* item[NumTimingModes];
	if (model) {
		for(int i=0;i<NumTimingModes;i++){
			index[i] = model->index(i,	comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}
		//Enabling/Disabling depending on the detector type
		switch(detType){
		case slsDetectorDefs::MYTHEN:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Readout]->setEnabled(true);
			item[(int)Gated]->setEnabled(true);
			item[(int)Gated_Start]->setEnabled(true);
			break;
		case slsDetectorDefs::EIGER:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Readout]->setEnabled(true);
			item[(int)Gated]->setEnabled(true);
			item[(int)Gated_Start]->setEnabled(false);
			break;
		case slsDetectorDefs::MOENCH:
		case slsDetectorDefs::GOTTHARD:
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Readout]->setEnabled(false);
			item[(int)Gated]->setEnabled(false);
			item[(int)Gated_Start]->setEnabled(false);
			break;
		default:
			cout << "Unknown detector type." << endl;
			qDefs::Message(qDefs::CRITICAL,"Unknown detector type.","qTabMeasurement::SetupTimingMode");
			exit(-1);
			break;
		}
	}

	GetModeFromDetector(true);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::GetModeFromDetector(bool startup){
#ifdef VERBOSE
		cout  << "Getting timing mode" << endl;
#endif
	//Get timing mode from detector
	slsDetectorDefs::externalCommunicationMode mode = myDet->setExternalCommunicationMode();

	//Setting the timing mode in gui
	if(model && model->itemFromIndex(model->index(mode,comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex()))->isEnabled()){

		//these are things checked in setuptimingmode

		// to let qdrawplot know that triggers or frames are used
		myPlot->setFrameEnabled(lblNumFrames->isEnabled());
		myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());
		if((!startup) && (comboTimingMode->currentIndex() == (mode+1))){
			if(lblPeriod->isEnabled())
				CheckAcqPeriodGreaterThanExp();
			else{
				spinPeriod->setToolTip(acqPeriodTip);
				lblPeriod->setToolTip(acqPeriodTip);
				lblPeriod->setPalette(lblTimingMode->palette());
				lblPeriod->setText("Acquisition Period:");
			}
			EnableProbes();
			emit CheckPlotIntervalSignal();
			return;
		}

		//if the timing mode is Auto and
		// number of Frames and number of triggers is 1,
		// then the timing mode is 'None'.
		// This is for the inexperienced user
		if(mode==slsDetectorDefs::AUTO_TIMING){
			int frames = spinNumFrames->value();
			int triggers = spinNumTriggers->value();
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
		//check if the detector is not even connected
		string offline = myDet->checkOnline();
		qDefs::checkErrorMessage(myDet,"qTabMeasurement::GetModeFromDetector");

		if(!offline.empty()){
			qDefs::Message(qDefs::CRITICAL,string("<nobr>The detector(s)  <b>")+offline+string(" </b> is/are not connected.  Exiting GUI.</nobr>"),"Main");
			cout << "The detector(s)  " << offline << "  is/are not connected. Exiting GUI." << endl;
			exit(-1);
		}

		qDefs::Message(qDefs::WARNING,"Unknown Timing Mode detected from detector."
				"\n\nSetting the following defaults:\nTiming Mode \t: None\n"
				"Number of Frames \t: 1\nNumber of Triggers \t: 1","qTabMeasurement::GetModeFromDetector");
		comboTimingMode->setCurrentIndex((int)None);
		SetTimingMode((int)None);
	}

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::GetModeFromDetector");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------



void qTabMeasurement::Initialization(){
	//Number of Measurements
	connect(spinNumMeasurements,SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));
	//File Name
	connect(dispFileName,		SIGNAL(editingFinished()),this,	SLOT(setFileName()));
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
	//Number of Frames
	connect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	//Exposure Time
	connect(spinExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(setExposureTime()));
	connect(comboExpUnit,SIGNAL(currentIndexChanged(int)),		this,	SLOT(setExposureTime()));
	//Frame Period between exposures
	connect(spinPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(setAcquisitionPeriod()));
	connect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
	//Number of Triggers
	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
	//Delay After Trigger
	connect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));
	connect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	//Number of Gates
	connect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));
	//Number of Probes
	connect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::Enable(bool enable){
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);

	//shortcut each time, else it doesnt work a second time
	btnStartStop->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::startStopAcquisition(){
	if(btnStartStop->isChecked()){

		//if file write enabled and output dir doesnt exist
		if((chkFile->isChecked())&&(thisParent->DoesOutputDirExist() == slsDetectorDefs::FAIL)){
			if(qDefs::Message(qDefs::QUESTION,
					"<nobr>Your data will not be saved.</nobr><br><nobr>Disable File write and Proceed with acquisition anyway?</nobr>",
					"qTabMeasurement::startStopAcquisition") == slsDetectorDefs::FAIL){
				disconnect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
				btnStartStop->click();
				connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
				return;
			}else{
				//done because for receiver it cant save a file with blank file path and returns without acquiring even to the gui
				disconnect(chkFile, 			SIGNAL(toggled(bool)), 				this, SLOT(EnableFileWrite(bool)));
				chkFile->setChecked(false);
				EnableFileWrite(false);
				connect(chkFile, 			SIGNAL(toggled(bool)), 				this, SLOT(EnableFileWrite(bool)));
			}
		}

#ifdef VERBOSE
		cout << endl << endl << "Starting Acquisition" << endl;
#endif
		//btnStartStop->setStyleSheet("color:red");
		btnStartStop->setText("Stop");
		btnStartStop->setIcon(*iconStop);
		lblProgressIndex->setText(QString::number(0));
		Enable(0);
		progressBar->setValue(0);
		progressTimer->start(100);

		emit StartSignal();
	}else{
#ifdef VERBOSE
		cout << "Stopping Acquisition" << endl<< endl;
#endif
		//emit StopSignal(); commented out to prevent undefined state
		myDet->stopAcquisition();
		/* commented out to prevent undefined state
		myDet->waitForReceiverReadToFinish();
		UpdateProgress();
		//spin index
		disconnect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
		spinIndex->setValue(myDet->getFileIndex());
		connect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
		progressTimer->stop();
		btnStartStop->setText("Start");
		btnStartStop->setIcon(*iconStart);
		btnStartStop->setChecked(false);
		Enable(1);*/
	}
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::startStopAcquisition");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::UpdateFinished(){
	UpdateProgress();
	disconnect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
	spinIndex->setValue(myDet->getFileIndex());
	connect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
	progressTimer->stop();

	disconnect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	btnStartStop->setText("Start");
	btnStartStop->setIcon(*iconStart);
	btnStartStop->setChecked(false);
	Enable(1);
	connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::UpdateFinished");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::SetCurrentMeasurement(int val){
	if((val)<spinNumMeasurements->value())
		lblCurrentMeasurement->setText(QString::number(val));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::UpdateProgress(){
	progressBar->setValue((int)myPlot->GetProgress());
	lblProgressIndex->setText(QString::number(myPlot->GetFrameIndex()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setFileName(){
	QString fName = dispFileName->text();
#ifdef VERBOSE
	cout << "Setting File name to "  <<  fName.toAscii().constData() << endl;
#endif
	myDet->setFileName(fName.toAscii().data());

	disconnect(dispFileName,		SIGNAL(editingFinished()),this,	SLOT(setFileName()));
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	connect(dispFileName,		SIGNAL(editingFinished()),this,	SLOT(setFileName()));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setFileName");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMeasurement::setRunIndex(int index){
#ifdef VERBOSE
	cout << "Setting File Index to " << index << endl;
#endif
	myDet->setFileIndex(index);

	disconnect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
	spinIndex->setValue(myDet->getFileIndex());
	connect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setRunIndex");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumMeasurements(int val){
#ifdef VERBOSE
	cout << "Setting Number of Measurements to "  << val  << endl;
#endif
	myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,val);

	disconnect(spinNumMeasurements,SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));
	spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,-1));
	connect(spinNumMeasurements,SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumMeasurements");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumFrames(int val){
#ifdef VERBOSE
	cout << "Setting number of frames to " << val << endl;
#endif
	myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,val);

	disconnect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
	connect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumFrames");

	EnableProbes();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::CheckAcqPeriodGreaterThanExp(){
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
	double acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
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


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setExposureTime(){
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout << "Setting acquisition time to " << exptimeNS << " clocks" << "/" << spinExpTime->value() << qDefs::getUnitString((qDefs::timeUnit)comboExpUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,(int64_t)exptimeNS);
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setExposureTime");

	if(lblPeriod->isEnabled())
		CheckAcqPeriodGreaterThanExp();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setAcquisitionPeriod(){
	//Get the value of timer in ns
	double acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
#ifdef VERBOSE
	cout << "Setting frame period between exposures to " << acqtimeNS << " clocks"<< "/" << spinPeriod->value() << qDefs::getUnitString((qDefs::timeUnit)comboPeriodUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,(int64_t)acqtimeNS);
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setAcquisitionPeriod");

	CheckAcqPeriodGreaterThanExp();
	//Check if the interval between plots is ok
	emit CheckPlotIntervalSignal();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumTriggers(int val){
#ifdef VERBOSE
	cout << "Setting number of triggers to " << val << endl;
#endif
	myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,val);

	disconnect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
	spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumTriggers");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setDelay(){
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboDelayUnit->currentIndex(),spinDelay->value());
#ifdef VERBOSE
	cout << "Setting delay after trigger to " << exptimeNS << " clocks" <<  "/" << spinDelay->value() << qDefs::getUnitString((qDefs::timeUnit)comboDelayUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,(int64_t)exptimeNS);
/*
	//updating value set
	disconnect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));
	disconnect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,-1)*(1E-9))));
	spinDelay->setValue(time);
	comboDelayUnit->setCurrentIndex((int)unit);
	connect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));
	connect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
*/

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setDelay");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumGates(int val){
#ifdef VERBOSE
	cout << "Setting number of gates to " << val << endl;
#endif
	myDet->setTimer(slsDetectorDefs::GATES_NUMBER,val);

	disconnect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));
	spinNumGates->setValue((int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1));
	connect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumGates");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::setNumProbes(int val){
#ifdef VERBOSE
	cout << "Setting number of probes to " << val << endl;
#endif
	disconnect(spinNumTriggers,SIGNAL(valueChanged(int)),		this,	SLOT(setNumTriggers(int)));
	disconnect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));

	//set probes
	int ret = myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,val);
	if(ret != val)
		ret = myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1);
	spinNumProbes->setValue(ret);
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumProbes");


	//Setting number of probes should reset number of triggers to 1, need to check if enabled, cuz its updated when refresh
	if((spinNumProbes->isEnabled()) && (ret > 0) && (spinNumTriggers->value() != 1)){
		qDefs::Message(qDefs::INFORMATION,"<nobr>Number of Triggers has been reset to 1.</nobr><br>"
				"<nobr>This is mandatory to use probes.</nobr>","qTabMeasurement::setNumProbes");
		cout << "Resetting Number of triggers to 1" << endl;
		spinNumTriggers->setValue(1);
	}
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::setNumProbes");

	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
	connect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));
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
		spinNumTriggers->setValue(1);
		spinNumFrames->setValue(1);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::AUTO_TIMING)==slsDetectorDefs::AUTO_TIMING)
			success = true;
		break;
	case Auto://#Frames, ExpTime, Period
		spinNumTriggers->setValue(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::AUTO_TIMING)==slsDetectorDefs::AUTO_TIMING)
			success = true;
		break;
	case Trigger_Exp_Series://#(Frames), #Triggers, ExpTime, Period, (Delay)
		if(detType == slsDetectorDefs::EIGER) //only 1 frame for each trigger for eiger
		  spinNumFrames->setValue(1);
		else{
		 lblNumFrames->setEnabled(true);	spinNumFrames->setEnabled(true);
		 lblDelay->setEnabled(true);		spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		 lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		}
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_EXPOSURE)==slsDetectorDefs::TRIGGER_EXPOSURE)
			success = true;
		break;
	case Trigger_Readout://#Frames, ExpTime, Period, (Delay)
		if(detType != slsDetectorDefs::EIGER){
		  lblDelay->setEnabled(true);		spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		}
		spinNumTriggers->setValue(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		if(myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_READOUT)==slsDetectorDefs::TRIGGER_READOUT)
			success = true;
		break;
	case Gated://#Frames, #(Gates)
		if(detType != slsDetectorDefs::EIGER){
		  lblNumGates->setEnabled(true);		spinNumGates->setEnabled(true);
		}
		spinNumTriggers->setValue(1);
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);


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
	default:
		cout << "Timing mode unknown to GUI" << endl;
		//This should never happen
		qDefs::Message(qDefs::CRITICAL,"Timing mode unknown to GUI","qTabMeasurement::SetTimingMode");
		qDefs::checkErrorMessage(myDet,"qTabMeasurement::SetTimingMode");
		exit(-1);
	}
	qDefs::checkErrorMessage(myDet,"qTabMeasurement::SetTimingMode");
	if(!success){
		qDefs::Message(qDefs::WARNING,"The detector timing mode could not be set.\n"
				"Please check the external flags."
				"\n\nSetting the following defaults:\nTiming Mode \t: None\n"
				"Number of Frames \t: 1\nNumber of Triggers \t: 1","qTabMeasurement::SetTimingMode");
		spinNumFrames->setValue(1);
		spinNumTriggers->setValue(1);
		comboTimingMode->setCurrentIndex((int)None);
		return;
	}


	//Frame Period between exposures
	qDefs::timeUnit unit;
	if(lblPeriod->isEnabled())
		CheckAcqPeriodGreaterThanExp();
	else{
		spinPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setPalette(lblTimingMode->palette());
		lblPeriod->setText("Acquisition Period:");
	}


	//Check if Number of Probes should be enabled
	EnableProbes();

	// to let qdrawplot know that triggers or frames are used
	myPlot->setFrameEnabled(lblNumFrames->isEnabled());
	myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::SetTimingMode");

	emit CheckPlotIntervalSignal();

	return;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::EnableFileWrite(bool enable){
#ifdef VERBOSE
	cout << "Enable File Write:" << enable << endl;
#endif
	myDet->enableWriteToFile(enable);
	dispFileName->setEnabled(enable);
	if(enable) setFileName();
	myPlot->SetEnableFileWrite(enable);

	disconnect(chkFile, 			SIGNAL(toggled(bool)), 				this, SLOT(EnableFileWrite(bool)));
	chkFile->setChecked(myDet->enableWriteToFile());
	connect(chkFile, 			SIGNAL(toggled(bool)), 				this, SLOT(EnableFileWrite(bool)));

	qDefs::checkErrorMessage(myDet,"qTabMeasurement::EnableFileWrite");
};

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating Measurement Tab" << endl;
#endif

	if(!myPlot->isRunning()){
		//to prevent it from recalculating forever
		disconnect(spinNumMeasurements,	SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));
		disconnect(dispFileName,		SIGNAL(editingFinished()),			this,	SLOT(setFileName()));
		disconnect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
		disconnect(progressTimer, 		SIGNAL(timeout()), 					this, 	SLOT(UpdateProgress()));
		disconnect(chkFile, 			SIGNAL(toggled(bool)), 				this, 	SLOT(EnableFileWrite(bool)));
		disconnect(spinNumFrames,		SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
		disconnect(spinExpTime,			SIGNAL(valueChanged(double)),		this,	SLOT(setExposureTime()));
		disconnect(comboExpUnit,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(setExposureTime()));
		disconnect(spinPeriod,			SIGNAL(valueChanged(double)),		this,	SLOT(setAcquisitionPeriod()));
		disconnect(comboPeriodUnit,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
		disconnect(spinNumTriggers,		SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
		disconnect(spinDelay,			SIGNAL(valueChanged(double)),		this,	SLOT(setDelay()));
		disconnect(comboDelayUnit,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
		disconnect(spinNumGates,		SIGNAL(valueChanged(int)),		 	this,	SLOT(setNumGates(int)));

#ifdef VERBOSE
		cout  << "Getting number of measurements & frames" << endl;
#endif
		//Number of measurements
		spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,-1));
		//Number of frames
		spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));


#ifdef VERBOSE
		cout  << "Getting Exposure time and Acquisition Period" << endl;
#endif
		//Exp Time
		qDefs::timeUnit unit;
		double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))));
		spinExpTime->setValue(time);
		comboExpUnit->setCurrentIndex((int)unit);
		//period
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-9))));
		spinPeriod->setValue(time);
		comboPeriodUnit->setCurrentIndex((int)unit);


#ifdef VERBOSE
		cout  << "Getting delay after trigger, number of triggers and number of gates" << endl;
#endif
		//delay
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,-1)*(1E-9))));
		spinDelay->setValue(time);
		comboDelayUnit->setCurrentIndex((int)unit);
		//Number of Triggers
		spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
		//gates
		spinNumGates->setValue((int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1));


#ifdef VERBOSE
		cout  << "Getting file name prefix, file index, file write enable and progress index" << endl;
#endif
		//File Name
		dispFileName->setText(QString(myDet->getFileName().c_str()));
		//File Index
		spinIndex->setValue(myDet->getFileIndex());
		//file write enabled/disabled
		chkFile->setChecked(myDet->enableWriteToFile());

		//progress label index
		if(myDet->getFrameIndex()==-1)
			lblProgressIndex->setText("0");
		else
			lblProgressIndex->setText(QString::number(myDet->getFrameIndex()));

		connect(spinNumMeasurements,SIGNAL(valueChanged(int)),			this,	SLOT(setNumMeasurements(int)));
		connect(dispFileName,		SIGNAL(editingFinished()),			this,	SLOT(setFileName()));
		connect(spinIndex,			SIGNAL(valueChanged(int)),			this,	SLOT(setRunIndex(int)));
		connect(progressTimer, 		SIGNAL(timeout()), 					this, 	SLOT(UpdateProgress()));
		connect(chkFile, 			SIGNAL(toggled(bool)), 				this, 	SLOT(EnableFileWrite(bool)));
		connect(spinNumFrames,		SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
		connect(spinExpTime,		SIGNAL(valueChanged(double)),		this,	SLOT(setExposureTime()));
		connect(comboExpUnit,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(setExposureTime()));
		connect(spinPeriod,			SIGNAL(valueChanged(double)),		this,	SLOT(setAcquisitionPeriod()));
		connect(comboPeriodUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
		connect(spinNumTriggers,	SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
		connect(spinDelay,			SIGNAL(valueChanged(double)),		this,	SLOT(setDelay()));
		connect(comboDelayUnit,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
		connect(spinNumGates,		SIGNAL(valueChanged(int)),		 	this,	SLOT(setNumGates(int)));

		//timing mode - will also check if exptime>acq period and also enableprobes()
		GetModeFromDetector();

		// to let qdrawplot know that triggers or frames are used
		myPlot->setFrameEnabled(lblNumFrames->isEnabled());
		myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());

		qDefs::checkErrorMessage(myDet,"qTabMeasurement::Refresh");
	}

#ifdef VERBOSE
		cout  << "**Updated Measurement Tab" << endl << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabMeasurement::EnableProbes(){

	//disconnect(spinNumProbes,SIGNAL(valueChanged(int)),		this,	SLOT(setNumProbes(int)));
	disconnect(spinNumTriggers,SIGNAL(valueChanged(int)),	this,	SLOT(setNumTriggers(int)));


	//enabled only in expert mode and if #Frames > 1
	if((expertMode)&&(detType==slsDetectorDefs::MYTHEN)&&(spinNumFrames->value()>1)){
		lblNumProbes->setEnabled(true);
		spinNumProbes->setEnabled(true);
		spinNumProbes->setValue((int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1));
#ifdef VERBOSE
		cout << "Getting number of probes : " << spinNumProbes->value() << endl;
#endif

		//Setting number of probes should reset number of triggers to 1, need to check if enabled, cuz its updated when refresh
		if((spinNumProbes->isEnabled()) && (spinNumProbes->value() > 0) && (spinNumTriggers->value() != 1)){
			qDefs::Message(qDefs::INFORMATION,"<nobr>Number of Triggers has been reset to 1.</nobr><br>"
					"<nobr>This is mandatory to use probes.</nobr>","qTabMeasurement::EnableProbes");
			cout << "Resetting Number of triggers to 1" << endl;
			spinNumTriggers->setValue(1);
		}
		qDefs::checkErrorMessage(myDet,"qTabMeasurement::EnableProbes");

		connect(spinNumProbes,		SIGNAL(valueChanged(int)),	this,	SLOT(setNumProbes(int)));
		connect(spinNumTriggers,	SIGNAL(valueChanged(int)),	this,	SLOT(setNumTriggers(int)));
		return;
	}
	cout << "Probes not enabled" << endl;
	if(detType==slsDetectorDefs::MYTHEN)
		spinNumProbes->setValue(0);
	lblNumProbes->setEnabled(false);
	spinNumProbes->setEnabled(false);

	//connect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));
	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

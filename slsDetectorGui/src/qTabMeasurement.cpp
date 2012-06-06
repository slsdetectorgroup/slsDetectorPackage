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

#include <QStandardItemModel>

/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0
#define UndefinedSettings 7



qTabMeasurement::qTabMeasurement(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot):
								QWidget(parent),myDet(detector),myPlot(plot){
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

	/** File Name **/
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	/** File Index **/
	spinIndex->setValue(myDet->getFileIndex());
	/** only initially **/
	lblProgressIndex->setText(QString::number(myDet->getFileIndex()));

	/** Enabling/Disabling depending on the detector type*/
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboTimingMode->model());
	QModelIndex index[NumTimingModes];
	QStandardItem* item[NumTimingModes];
	if (model) {
		for(int i=0;i<NumTimingModes;i++){
			index[i] = model->index(i,	comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}

		switch(myDet->getDetectorsType()){
		case slsDetectorDefs::MYTHEN:
			item[(int)Gated]->setEnabled(true);
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(false);
			item[(int)Trigger_Readout]->setEnabled(true);
			item[(int)Gated_Start]->setEnabled(true);
			item[(int)Trigger_Window]->setEnabled(false);
			break;
		case slsDetectorDefs::EIGER:
			item[(int)Gated]->setEnabled(false);
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(true);
			item[(int)Trigger_Readout]->setEnabled(false);
			item[(int)Gated_Start]->setEnabled(false);
			item[(int)Trigger_Window]->setEnabled(true);
			break;
		case slsDetectorDefs::GOTTHARD:
			item[(int)Gated]->setEnabled(false);
			item[(int)Trigger_Exp_Series]->setEnabled(true);
			item[(int)Trigger_Frame]->setEnabled(false);
			item[(int)Trigger_Readout]->setEnabled(false);
			item[(int)Gated_Start]->setEnabled(false);
			item[(int)Trigger_Window]->setEnabled(false);
			break;
/*		case slsDetectorDefs::PICASSO:
        case slsDetectorDefs::PILATUS:
		case slsDetectorDefs::AGIPD:*/
		default:
			cout<<"ERROR: Detector Type is Generic"<<endl;
			exit(-1);
		}
	}


	//get timing mode from client


}



void qTabMeasurement::Initialization(int timingChange){
	/** These signals are connected only at start up*/
	if(!timingChange){
		/** Number of Measurements**/
		connect(spinNumMeasurements,SIGNAL(valueChanged(int)),		myPlot,	SLOT(setNumMeasurements(int)));
		/** File Name**/
		connect(dispFileName,SIGNAL(textChanged(const QString&)),	this,	SLOT(setFileName(const QString&)));
		/** File Index**/
		connect(spinIndex,SIGNAL(valueChanged(int)),				this,	SLOT(setRunIndex(int)));
		/** Start/Stop Acquisition**/
		connect(btnStartStop,SIGNAL(clicked()),						this,	SLOT(startStopAcquisition()));
		/** Timing Mode **/
		connect(comboTimingMode,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setTimingMode(int)));//
	}
	/** Number of Frames**/
	connect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	/** Exposure Time **/
	connect(spinExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(setExposureTime()));//..myplot
	connect(comboExpUnit,SIGNAL(currentIndexChanged(int)),		this,	SLOT(setExposureTime()));
	/** Frame Period between exposures**/
	connect(spinPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(setAcquisitionPeriod()));//..myplot
	connect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
	/** Number of Triggers**/
	connect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));//
	/** Delay After Trigger **/
	connect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));//
	connect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	/** Number of Gates**/
	connect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));//
	/** Number of Probes**/
	connect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));//

}





void qTabMeasurement::DeInitialization(){
	/** Number of Frames**/
	disconnect(spinNumFrames,SIGNAL(valueChanged(int)),			this,	SLOT(setNumFrames(int)));
	/** Exposure Time **/
	disconnect(spinExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(setExposureTime()));//..myplot
	disconnect(comboExpUnit,SIGNAL(currentIndexChanged(int)),		this,	SLOT(setExposureTime()));
	/** Frame Period between exposures**/
	disconnect(spinPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(setAcquisitionPeriod()));//..myplot
	disconnect(comboPeriodUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setAcquisitionPeriod()));
	/** Number of Triggers**/
	disconnect(spinNumTriggers,SIGNAL(valueChanged(int)),			this,	SLOT(setNumTriggers(int)));
	/** Delay After Trigger **/
	disconnect(spinDelay,SIGNAL(valueChanged(double)),				this,	SLOT(setDelay()));
	disconnect(comboDelayUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(setDelay()));
	/** Number of Gates**/
	disconnect(spinNumGates,SIGNAL(valueChanged(int)),				this,	SLOT(setNumGates(int)));
	/** Number of Probes**/
	disconnect(spinNumProbes,SIGNAL(valueChanged(int)),			this,	SLOT(setNumProbes(int)));
}





void qTabMeasurement::Enable(bool enable){
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);
	/** Enable this always **/
	if(!enable) btnStartStop->setEnabled(true);
}




void qTabMeasurement::UpdateFinished(){
	disconnect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
	btnStartStop->setText("Start");
	Enable(1);
	connect(btnStartStop,SIGNAL(clicked()),this,SLOT(startStopAcquisition()));
}



void qTabMeasurement::setFileName(const QString& fName){
	//  emit fileNameChanged(fName);
	//  thred-->fileName=s;myDet->setFileName(fName.ascii());
	myDet->setFileName(fName.toAscii().data());
#ifdef VERBOSE
	cout<<"Setting File name to " << myDet->getFileName()<<endl;
#endif
}




void qTabMeasurement::setRunIndex(int index){
	myDet->setFileIndex(index);
#ifdef VERBOSE
	cout<<"Setting File Index to " << myDet->getFileIndex()<<endl;
#endif
}



void qTabMeasurement::startStopAcquisition(){
	if(!btnStartStop->text().compare("Start")){
#ifdef VERBOSE
		cout<<endl<<endl<<"Starting Acquisition"<<endl;
#endif
		btnStartStop->setText("Stop");
		Enable(0);
		emit StartSignal();
	}else{
#ifdef VERBOSE
		cout<<"Stopping Acquisition"<<endl;
#endif
		btnStartStop->setText("Start");
		Enable(1);
		emit StopSignal();
	}
	myPlot->StartStopDaqToggle();
}




void qTabMeasurement::setNumFrames(int val){
	myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,val);
#ifdef VERBOSE
	cout<<"Setting number of frames to " << (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1)<<endl;
#endif

}


void qTabMeasurement::setExposureTime(){
	int64_t exptime64;
	/** Get the 64 bit value of timer**/
	exptime64 = qDefs::get64bTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout<<"Setting acquisition time to " << exptime64 << " clocks" << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,exptime64);

	//float t=exptime64;
	//emit acquisitionTimeChanged(t/(100E+6)); ??????????????????????
}



void qTabMeasurement::setAcquisitionPeriod(){
	int64_t exptime64;
	/** Get the 64 bit value of timer**/
	exptime64 = qDefs::get64bTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(),spinPeriod->value());
#ifdef VERBOSE
	cout<<"Setting frame period between exposures to " << exptime64 << " clocks" << endl;
#endif
	myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,exptime64);

	//float t=exptime64;
	//emit acquisitionTimeChanged(t/(100E+6)); ??????????????????????
}





void qTabMeasurement::setNumTriggers(int val){
	myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,val);
#ifdef VERBOSE
	cout<<"Setting number of triggers to " << (int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1)<<endl;
#endif
}




void qTabMeasurement::setDelay(){
	int64_t exptime64;
	/** Get the 64 bit value of timer**/
	exptime64 = qDefs::get64bTime((qDefs::timeUnit)comboDelayUnit->currentIndex(),spinDelay->value());
#ifdef VERBOSE
	cout<<"Setting delay after trigger to " << exptime64 << " clocks" << endl;
#endif
	myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,exptime64);
}




void qTabMeasurement::setNumGates(int val){
	myDet->setTimer(slsDetectorDefs::GATES_NUMBER,val);
#ifdef VERBOSE
	cout<<"Setting number of gates to " << (int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1)<<endl;
#endif
}




void qTabMeasurement::setNumProbes(int val){
	myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,val);
#ifdef VERBOSE
	cout<<"Setting number of frames to " << (int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1)<<endl;
#endif
}




void qTabMeasurement::setTimingMode(int mode){
#ifdef VERBOSE
	cout<<"Setting Timing mode to " << comboTimingMode->currentText().toAscii().data()<<endl;
#endif
	//need to send to client to set the timing mode

	/** Default settings */
	lblNumFrames->setEnabled(false);	spinNumFrames->setEnabled(false);
	lblExpTime->setEnabled(false);		spinExpTime->setEnabled(false);			comboExpUnit->setEnabled(false);
	lblPeriod->setEnabled(false);		spinPeriod->setEnabled(false);			comboPeriodUnit->setEnabled(false);
	lblNumTriggers->setEnabled(false);	spinNumTriggers->setEnabled(false);
	lblDelay->setEnabled(false);		spinDelay->setEnabled(false);			comboDelayUnit->setEnabled(false);
	lblNumGates->setEnabled(false);		spinNumGates->setEnabled(false);
	lblNumProbes->setEnabled(false);	spinNumProbes->setEnabled(false);


	switch(mode){
	case None:
		break;
	case Auto:
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		break;
	case Gated:
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblNumGates->setEnabled(true);		spinNumGates->setEnabled(true);
		break;
	case Trigger_Exp_Series:
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		lblDelay->setEnabled(true);			spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		break;
	case Trigger_Readout:
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblDelay->setEnabled(true);			spinDelay->setEnabled(true);			comboDelayUnit->setEnabled(true);
		break;
	case Gated_Start:
		lblNumFrames->setEnabled(true);		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);		spinPeriod->setEnabled(true);			comboPeriodUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		lblNumGates->setEnabled(true);		spinNumGates->setEnabled(true);
		break;
	case Trigger_Frame:
		lblExpTime->setEnabled(true);		spinExpTime->setEnabled(true);			comboExpUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		break;
	case Trigger_Window:
		lblNumTriggers->setEnabled(true);	spinNumTriggers->setEnabled(true);
		break;
	default:
		cout<<"ERROR: Timing mode being set to other should never happen"<<endl;
		exit(-1);
	}

	if(mode!=None){
		if(myDet->getDetectorsType()==slsDetectorDefs::MYTHEN){
			lblNumProbes->setEnabled(true);		spinNumProbes->setEnabled(true);
		}
	}

	/** To disconnect all the signals before changing their values*/
	DeInitialization();


	float time;
	int val;
	/**Number of Frames */
	if(lblNumFrames->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1);
		spinNumFrames->setValue(val);
#ifdef VERBOSE
		cout<<"Getting number of frames : " << val <<endl;
#endif
	}

	/**Exposure Time */
	if(lblExpTime->isEnabled()){
		time = (float)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9));
#ifdef VERBOSE
		cout<<"Getting acquisition time : " << time << "s" << endl;
#endif
		spinExpTime->setValue(time);
		comboExpUnit->setCurrentIndex(qDefs::SECONDS);
	}

	/**Frame Period between exposures */
	if(lblPeriod->isEnabled()){
		time = (float)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1)*(1E-9));
#ifdef VERBOSE
		cout<<"Getting frame period between exposures : " << time << "s" << endl;
#endif
		spinPeriod->setValue(time);
		comboPeriodUnit->setCurrentIndex(qDefs::SECONDS);
	}

	/**Number of Triggers */
	if(lblNumTriggers->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1);
		spinNumTriggers->setValue(val);
#ifdef VERBOSE
		cout<<"Getting number of triggers : " << val <<endl;
#endif
	}

	/**Delay After Trigger */
	if(lblDelay->isEnabled()){
		time = (float)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER,-1)*(1E-9));
#ifdef VERBOSE
		cout<<"Getting delay after trigger : " << time << "s" << endl;
#endif
		spinDelay->setValue(time);
		comboDelayUnit->setCurrentIndex(qDefs::SECONDS);
	}

	/**Number of Gates */
	if(lblNumGates->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::GATES_NUMBER,-1);
		spinNumGates->setValue(val);
#ifdef VERBOSE
		cout<<"Getting number of gates : " << val <<endl;
#endif
	}

	/**Number of Probes */
	if(lblNumProbes->isEnabled()){
		val = (int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1);
		spinNumProbes->setValue(val);
#ifdef VERBOSE
		cout<<"Getting number of probes : " << val <<endl;
#endif
	}


	/** To reconnect all the signals after changing their values*/
	Initialization(1);


}

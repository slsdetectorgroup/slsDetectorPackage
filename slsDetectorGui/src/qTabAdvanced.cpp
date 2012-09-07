/*
 * qTabAdvanced.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabAdvanced.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QFileDialog>
/** C++ Include Headers */
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabAdvanced::qTabAdvanced(QWidget *parent,multiSlsDetector*& detector):QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



qTabAdvanced::~qTabAdvanced(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetupWidgetWindow(){
//executed even for non digital, so make sure its necessary
	slsDetectorDefs::detectorType detType = myDet->getDetectorsType();
	if((detType==slsDetectorDefs::MYTHEN)||(detType==slsDetectorDefs::EIGER)){
		outputDirTip = dispFile->toolTip();
		errOutputTip = QString("<br><br><font color=\"red\"><nobr>"
				"<b>Output Trim File</b> should contain both existing directory and a file name.</nobr><br>"
				"<nobr>The extensions are automatically added.</nobr><br><br>"
				"<nobr>Enter valid<b> Output Trim File</b> to enable <b>Start Trimming</b> button.</nobr></font>");
		red = QPalette();
		red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);

		boxTrimming->setChecked(false);
		SetOptimize(false);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Initialization(){
	//energy/angular logs
	connect(chkEnergyLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));
	connect(chkAngularLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));

	//exptime
	connect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
	connect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));

	//threshold dac
	connect(spinThreshold,	SIGNAL(valueChanged(double)),	this,	SLOT(SetThreshold()));

	//output directory
	connect(dispFile,		SIGNAL(editingFinished()),		this, SLOT(SetOutputFile()));
	connect(btnFile,		SIGNAL(clicked()), 				this, SLOT(BrowseOutputFile()));

	//enable trimming method group box
	connect(boxTrimming,	SIGNAL(toggled(bool)),	this, SLOT(EnableTrimming(bool)));

	//trimming method combo
	connect(comboMethod,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetTrimmingMethod(int)));

	//method options
	connect(chkOptimize,	SIGNAL(toggled(bool)),	this, SLOT(SetOptimize(bool)));

	//start Trimming
	connect(btnStart,		SIGNAL(clicked()),	this, SLOT(StartTrimming()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetLogs(){
	QCheckBox *checkedBox = qobject_cast<QCheckBox *>(sender());
	int index = ((!checkedBox->text().compare("Energy Calibration"))?slsDetectorDefs::enCalLog:slsDetectorDefs::angCalLog);
	bool enable = checkedBox->isChecked();
#ifdef VERBOSE
	if(index==slsDetectorDefs::enCalLog)
		cout << "Setting Energy Calibration Logs to " << enable << endl;
	else
		cout << "Setting Angular Calibration Logs to " << enable << endl;
#endif
	//set/unset the log
	myDet->setAction(index,(enable?"set":"none"));
	//verify
	if(myDet->getActionMode(index)!=(enable)){
#ifdef VERBOSE
	cout << "Could not set/reset Log." << endl;
#endif
		qDefs::Message(qDefs::WARNING,"Could not set/reset Log.","Advanced");
		checkedBox->setChecked(!enable);
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetExposureTime(){
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout << "Setting Exposure Time to " << exptimeNS << " clocks" << "/" << spinExpTime->value() << qDefs::getUnitString((qDefs::timeUnit)comboExpUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,(int64_t)exptimeNS);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetThreshold(){
#ifdef VERBOSE
	cout << "Setting Threshold DACu:" << spinThreshold->value() << endl;
#endif
	spinThreshold->setValue((double)myDet->setDAC((dacs_t)spinThreshold->value(),slsDetectorDefs::THRESHOLD));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetOutputFile(){
#ifdef VERBOSE
	cout << "Setting Output File for Trimming" << endl;
#endif
	QString dirPath = dispFile->text().section('/',0,-2,QString::SectionIncludeLeadingSep);
	cout<<"directory:"<<dirPath.toAscii().constData()<<"..."<<endl;
	QString fName = dispFile->text().section('/',-1);
	cout<<"file name:"<<fName.toAscii().constData()<<"..."<<endl;
	//checks if directory exists and file name is not empty
	if((QFile::exists(dirPath))&&(!fName.isEmpty())){

		dispFile->setToolTip(outputDirTip);
		lblFile->setToolTip(outputDirTip);
		lblFile->setPalette(lblExpTime->palette());
		lblFile->setText("Output Trim File: ");
		btnStart->setEnabled(true);


		//check if you're overwriting original trimsettings
		QDir dir;
		//gets the clean absolute path
		dirPath = dir.absoluteFilePath(dirPath);
		dirPath = dir.cleanPath(dirPath);
		QString trimdir = QString(myDet->getSettingsFile()).section('/',0,-2,QString::SectionIncludeLeadingSep);
		trimdir = dir.absoluteFilePath(trimdir);
		trimdir = dir.cleanPath(trimdir);
		if(!dirPath.compare(trimdir)){
			int ret = qDefs::Message(qDefs::QUESTION,string("<nobr>This will possibly overwrite your original trimbits.</nobr><br>"
					"<nobr>Proposed file path:") + string(dirPath.toAscii().constData())+
					string("</nobr><br><nobr>Do you still want to continue?</nobr>"),"Advanced");
			if(ret==qDefs::FAIL){
				dispFile->setText("");
				dispFile->setToolTip(outputDirTip + errOutputTip);
				lblFile->setToolTip(outputDirTip + errOutputTip);
				lblFile->setPalette(red);
				lblFile->setText("Output Trim File:*");
				btnStart->setEnabled(false);
			}
		}

	}//if the directory doesnt exist or if file name is empty
	else{
		cout<<"Invalid Trimming output File"<<endl;
		dispFile->setToolTip(outputDirTip + errOutputTip);
		lblFile->setToolTip(outputDirTip + errOutputTip);
		lblFile->setPalette(red);
		lblFile->setText("Output Trim File:*");
		btnStart->setEnabled(false);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::BrowseOutputFile(){
#ifdef VERBOSE
	cout << "Browsing Output Dir for Trimming:" << endl;
#endif
	QString fName = dispFile->text();
	//dialog
	fName = QFileDialog::getSaveFileName(this,
			tr("Choose file to write the trimbits to"),fName,
			tr("Trimbit files (*.trim noise.sn*) "));
	//if empty, set the file name and it calls SetFileSteps, else ignore
	if (!fName.isEmpty()){
		dispFile->setText(fName);
		SetOutputFile();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::EnableTrimming(bool enable){
#ifdef VERBOSE
	cout << "Enable Trimming set to:" << enable << endl;
#endif
	if(enable){
		//show error label if invalid output dir
		SetOutputFile();
		SetTrimmingMethod(comboMethod->currentIndex());
	}else{
		//error label shouldnt show when disabled
		dispFile->setToolTip(outputDirTip);
		lblFile->setToolTip(outputDirTip);
		lblFile->setPalette(lblExpTime->palette());
		lblFile->setText("Output Trim File: ");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
void qTabAdvanced::SetOptimize(bool enable){
#ifdef VERBOSE
	cout << "Setting Optimize to:" << enable << endl;
#endif
	//trimming method is adjust to count
	if(!comboMethod->currentIndex()){
		lblCounts->setEnabled(true);
		spinCounts->setEnabled(true);
		lblResolution->setEnabled(enable);
		spinResolution->setEnabled(enable);
	}//trimming method is equalize to median
	else{
		lblCounts->setEnabled(false);
		spinCounts->setEnabled(false);
		lblResolution->setEnabled(true);
		spinResolution->setEnabled(true);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetTrimmingMethod(int mode){
#ifdef VERBOSE
	cout << "Setting Trimming method to :" << mode << endl;
#endif
	//make sure the right resolution/Counts is enabled
	SetOptimize(chkOptimize->isChecked());

	//set mode
	switch(mode){
	case 0: trimmingMode = slsDetectorDefs::NOISE_TRIMMING; 	break;
	case 1:	trimmingMode = slsDetectorDefs::IMPROVE_TRIMMING;	break;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::StartTrimming(){

	int parameter1, parameter2;
	//optimize
	bool optimize = chkOptimize->isChecked();

	//set the mode again and also set resolution, counts
	switch(trimmingMode){

	case slsDetectorDefs::NOISE_TRIMMING:
		//define parameters
		parameter1 = spinCounts->value();
		parameter2 = spinResolution->value();
		if(!optimize){
			parameter2 = -1;
			trimmingMode = slsDetectorDefs::FIXEDSETTINGS_TRIMMING;
#ifdef VERBOSE
			cout << "Trimming Mode: FIXEDSETTINGS_TRIMMING" << endl;
#endif
		}else{
#ifdef VERBOSE
			cout << "Trimming Mode: NOISE_TRIMMING" << endl;
#endif
		}
		break;

	case slsDetectorDefs::IMPROVE_TRIMMING:
#ifdef VERBOSE
			cout << "Trimming Mode: IMPROVE_TRIMMING" << endl;
#endif
		//define parameters
		parameter1 = spinResolution->value();
		parameter2 = 1;
		if(!optimize)	parameter2 = 0;
		break;
	default:
		cout << "Should never come here. Start Trimming will have only 2 methods." << endl;
		break;
	}

#ifdef VERBOSE

#endif
	//execute
	int ret = myDet->executeTrimming(trimmingMode,parameter1,parameter2,-1);
	if((ret!=slsDetectorDefs::FAIL)&&(ret!=-1)){
		//save trim file
		ret = myDet->saveSettingsFile(string(dispFile->text().toAscii().constData()),-1);
		if((ret!=slsDetectorDefs::FAIL)&&(ret!=-1))
			qDefs::Message(qDefs::INFORMATION,"The Trimbits have been saved successfully.","Advanced");
		else qDefs::Message(qDefs::WARNING,string("Could not Save the Trimbits to file:\n")+dispFile->text().toAscii().constData(),"Advanced");
	}
	else
		qDefs::Message(qDefs::WARNING,"Atleast 1 channel could not be trimmed.","Advanced");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Refresh(){
	//disconnect
	disconnect(chkEnergyLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
	disconnect(chkAngularLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
	disconnect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
	disconnect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));
	disconnect(spinThreshold,	SIGNAL(valueChanged(double)),		this,	SLOT(SetThreshold()));

	//energy/angular logs
	chkEnergyLog->setChecked(myDet->getActionMode(slsDetectorDefs::enCalLog));
	chkAngularLog->setChecked(myDet->getActionMode(slsDetectorDefs::angCalLog));
#ifdef VERBOSE
	cout << "Energy Calibration Log set to " << chkEnergyLog->isChecked() << endl;
	cout << "Angular Calibration Log set to " << chkAngularLog->isChecked() << endl;
#endif

	//exptime
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1)*(1E-9))));
#ifdef VERBOSE
	cout << "Getting acquisition time : " << time << qDefs::getUnitString(unit) << endl;
#endif
	spinExpTime->setValue(time);
	comboExpUnit->setCurrentIndex((int)unit);


	//threshold
	double threshold = (double)myDet->setDAC(-1,slsDetectorDefs::THRESHOLD);
#ifdef VERBOSE
		cout << "Getting Threshold DACu : " << threshold << endl;
#endif
	spinThreshold->setValue(threshold);

	//connect
	connect(chkEnergyLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
	connect(chkAngularLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
	connect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
	connect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));
	connect(spinThreshold,	SIGNAL(valueChanged(double)),		this,	SLOT(SetThreshold()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


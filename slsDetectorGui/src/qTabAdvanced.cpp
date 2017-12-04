/*
 * qTabAdvanced.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabAdvanced.h"
#include "qDrawPlot.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QFileDialog>
/** C++ Include Headers */
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabAdvanced::qTabAdvanced(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot):
				  QWidget(parent),myDet(detector),det(0),myPlot(plot),btnGroup(NULL),isEnergy(false),isAngular(false),
				  lblFromX(0),
				  spinFromX(0),
				  lblFromY(0),
				  spinFromY(0),
				  lblToX(0),
				  spinToX(0),
				  lblToY(0),
				  spinToY(0),
				  numRois(0){
	setupUi(this);
	SetupWidgetWindow();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



qTabAdvanced::~qTabAdvanced(){
	delete myDet;
	if(det) delete det;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetupWidgetWindow(){

//executed even for non digital, so make sure its necessary

	//Network
	lblIP->setEnabled(false);
	lblMAC->setEnabled(false);
	dispIP->setEnabled(false);
	dispMAC->setEnabled(false);
	boxRxr->setEnabled(false);
	boxSetAllTrimbits->setEnabled(false);


	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	outputDirTip = dispFile->toolTip();
	errOutputTip = QString("<br><br><font color=\"red\"><nobr>"
			"<b>Output Trim File</b> should contain both existing directory and a file name.</nobr><br>"
			"<nobr>The extensions are automatically added.</nobr><br><br>"
			"<nobr>Enter valid<b> Output Trim File</b> to enable <b>Start Trimming</b> button.</nobr></font>");
	detOnlineTip = comboOnline->toolTip();
	rxrOnlineTip = comboRxrOnline->toolTip();
	errOnlineTip = QString("<nobr><br><br><font color=\"red\"><nobr>It is offline!</nobr></font>");

	detType = myDet->getDetectorsType();
	switch(detType){
	case slsDetectorDefs::MYTHEN:
		isEnergy = true;
		isAngular = true;
		break;
	case slsDetectorDefs::EIGER:
		isEnergy = true;
		isAngular = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		boxSetAllTrimbits->setEnabled(true);
		break;
	case slsDetectorDefs::MOENCH:
		isEnergy = false;
		isAngular = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	case slsDetectorDefs::GOTTHARD:
		isEnergy = false;
		isAngular = true;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	case slsDetectorDefs::JUNGFRAU:
		isEnergy = false;
		isAngular = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	default: break;
	}


	//logs and trimming
	if(!isAngular && !isEnergy) boxLogs->setEnabled(false);
	else{
		if(!isAngular) 	chkAngularLog->setEnabled(false);
		if(!isEnergy){
			chkEnergyLog->setEnabled(false);
			boxPlot->setEnabled(false);
			boxTrimming->setEnabled(false);
		}else{
			boxTrimming->setChecked(false);
			SetOptimize(false);

			btnGroup = new QButtonGroup(this);
			btnGroup->addButton(btnRefresh,0);
			btnGroup->addButton(btnGetTrimbits,1);
		}
	}
	trimmingMode = slsDetectorDefs::NOISE_TRIMMING;

	//network

	//add detectors
	for(int i=0;i<myDet->getNumberOfDetectors();i++)
		comboDetector->addItem(QString(myDet->getHostname(i).c_str()));

	comboDetector->setCurrentIndex(0);

	det = myDet->getSlsDetector(comboDetector->currentIndex());

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetupWidgetWindow");
	cout << "Getting ports" << endl;
	spinControlPort->setValue(det->getControlPort());
	spinStopPort->setValue(det->getStopPort());
	spinTCPPort->setValue(det->getReceiverPort());
	spinUDPPort->setValue(atoi(det->getReceiverUDPPort()));

	cout << "Getting network information" << endl;
	dispIP->setText(det->getDetectorIP());
	dispMAC->setText(det->getDetectorMAC());
	dispRxrHostname->setText(det->getReceiver());
	dispUDPIP->setText(det->getReceiverUDPIP());
	dispUDPMAC->setText(det->getReceiverUDPMAC());


	//check if its online and set it to red if offline
#ifdef VERYVERBOSE
	cout << "online" << endl;
#endif
	if(det->setOnline()==slsDetectorDefs::ONLINE_FLAG)
		det->checkOnline();
	if(det->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG)
		det->checkReceiverOnline();
	comboOnline->setCurrentIndex(det->setOnline());
	comboRxrOnline->setCurrentIndex(det->setReceiverOnline());
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}
	if((comboRxrOnline->isEnabled())&&(!comboRxrOnline->currentIndex())){
		comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setPalette(red);
		lblRxrOnline->setText("Online:*");
	}


	//updates roi
	cout << "Getting ROI" << endl;
	updateROIList();

	//  print receiver configurations
	if(myDet->getDetectorsType() != slsDetectorDefs::MYTHEN){
		cout << endl;
		myDet->printReceiverConfiguration();
	}

	Initialization();

	qDefs::checkErrorMessage(det,"qTabAdvanced::SetupWidgetWindow");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Initialization(){

	connect(tabAdvancedSettings,SIGNAL(currentChanged(int)),	this, SLOT(Refresh()));

	//energy/angular logs
	if(isAngular)
		connect(chkEnergyLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));

	if(isEnergy){
		connect(chkAngularLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));

		//exptime
		connect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
		connect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));

		//threshold dac
		connect(spinThreshold,	SIGNAL(valueChanged(double)),	this,	SLOT(SetThreshold()));

		//output directory
		connect(dispFile,		SIGNAL(editingFinished()),		this, SLOT(SetOutputFile()));
		connect(btnFile,		SIGNAL(clicked()), 				this, SLOT(BrowseOutputFile()));

		//setalltrimbits
		if(boxSetAllTrimbits->isEnabled())
			connect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));

		//enable trimming method group box
		connect(boxTrimming,	SIGNAL(toggled(bool)),	this, SLOT(EnableTrimming(bool)));

		//trimming method combo
		connect(comboMethod,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetTrimmingMethod(int)));

		//method options
		connect(chkOptimize,	SIGNAL(toggled(bool)),	this, SLOT(SetOptimize(bool)));

		//start Trimming
		connect(btnStart,		SIGNAL(clicked()),	this, SLOT(StartTrimming()));

		//refresh
		connect(btnGroup,		SIGNAL(buttonClicked(int)),	this, SLOT(UpdateTrimbitPlot(int)));
	}

	//network
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetDetector(int)));
	connect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
	connect(spinStopPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));

	if((detType==slsDetectorDefs::GOTTHARD) ||
			(detType==slsDetectorDefs::MOENCH) ||
			(detType==slsDetectorDefs::PROPIX) ||
			(detType==slsDetectorDefs::PROPIX) ||
			(detType==slsDetectorDefs::JUNGFRAU)){

		//network
		connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
		connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
		connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));

		connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));

		connect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));

	}


	//roi


	connect(btnClearRoi,		SIGNAL(clicked()),			this, SLOT(clearROIinDetector()));
	connect(btnGetRoi,			SIGNAL(clicked()),			this, SLOT(updateROIList()));
	connect(btnSetRoi,			SIGNAL(clicked()),			this, SLOT(setROI()));
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
		qDefs::Message(qDefs::WARNING,"Could not set/reset Log.","qTabAdvanced::SetLogs");
		checkedBox->setChecked(!enable);
	}

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetLogs");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetExposureTime(){
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(),spinExpTime->value());
#ifdef VERBOSE
	cout << "Setting Exposure Time to " << exptimeNS << " clocks" << "/" << spinExpTime->value() << qDefs::getUnitString((qDefs::timeUnit)comboExpUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,(int64_t)exptimeNS);

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetExposureTime");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetThreshold(){
#ifdef VERBOSE
	cout << "Setting Threshold DACu:" << spinThreshold->value() << endl;
#endif
	spinThreshold->setValue((double)myDet->setDAC((dacs_t)spinThreshold->value(),slsDetectorDefs::THRESHOLD,0));
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetThreshold");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetOutputFile(){
#ifdef VERBOSE
	cout << "Setting Output File for Trimming:" << endl;
#endif
	QString dirPath = dispFile->text().section('/',0,-2,QString::SectionIncludeLeadingSep);
	cout << "Directory:" << dirPath.toAscii().constData() << "." << endl;
	QString fName = dispFile->text().section('/',-1);
	cout << "File Name:" << fName.toAscii().constData() << "." << endl;
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
					string("</nobr><br><nobr>Do you still want to continue?</nobr>"),"qTabAdvanced::SetOutputFile");
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

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetOutputFile");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::BrowseOutputFile(){
#ifdef VERBOSE
	cout << "Browsing Output Dir for Trimming" << endl;
#endif
	QString fName = dispFile->text();
	//dialog
	QFileDialog	*fileDialog = new QFileDialog(this,
			tr("Save Trimbits"),fName,
			tr("Trimbit files (*.trim noise.sn*);;All Files(*) "));
	fileDialog->setFileMode(QFileDialog::AnyFile );
    if ( fileDialog->exec() == QDialog::Accepted )
    	fName = fileDialog->selectedFiles()[0];

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


int qTabAdvanced::validateBeforeTrimming(){
#ifdef VERBOSE
	cout << "Validating conditions before Trimming" << endl;
#endif
	char temp[100];
	string message = "<nobr>All conditions satisfied for Trimming.</nobr><br>";
	switch(detType){
	case slsDetectorDefs::MYTHEN:

		//dynamic range
		if(myDet->setDynamicRange(-1) != TRIMMING_DYNAMIC_RANGE){
			sprintf(temp,"%d",TRIMMING_DYNAMIC_RANGE);
			if(myDet->setDynamicRange(TRIMMING_DYNAMIC_RANGE) != TRIMMING_DYNAMIC_RANGE){
				qDefs::Message(qDefs::WARNING,
						string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>"
								"<nobr>Could not set dynamic range to ") + string(temp)+string(".</nobr><br>"
										"Trimming Aborted."),"qTabAdvanced::validateBeforeTrimming");
				return slsDetectorDefs::FAIL;
			}else
				message.append(string("<nobr><b>Dynamic Range</b> has been changed to ") + string(temp) + string(".<nobr><br>"));
		}
		//frames
		if((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1) != TRIMMING_FRAME_NUMBER){
			sprintf(temp,"%d",TRIMMING_FRAME_NUMBER);
			if((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,TRIMMING_FRAME_NUMBER) != TRIMMING_FRAME_NUMBER){
				qDefs::Message(qDefs::WARNING,
						string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>"
								"<nobr>Could not set <b>Number of Frames</b> to ") + string(temp)+string(".</nobr><br>"
										"Trimming Aborted."),"qTabAdvanced::validateBeforeTrimming");
				return slsDetectorDefs::FAIL;
			}else
				message.append(string("<nobr><b>Number of Frames</b> has been changed to ") + string(temp) + string(".<nobr><br>"));
		}
		//trigger
		if((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1) != TRIMMING_TRIGGER_NUMBER){
			sprintf(temp,"%d",TRIMMING_TRIGGER_NUMBER);
			if((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,TRIMMING_TRIGGER_NUMBER) != TRIMMING_TRIGGER_NUMBER){
				qDefs::Message(qDefs::WARNING,
						string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>"
								"<nobr>Could not set <b>Number of Triggers</b> to ") + string(temp)+string(".</nobr><br>"
										"Trimming Aborted."),"qTabAdvanced::validateBeforeTrimming");
				return slsDetectorDefs::FAIL;
			}else
				message.append(string("<nobr><b>Number of Triggers</b> has been changed to ") + string(temp) + string(".<nobr><br>"));
		}
		//probes
		if((int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,-1) != TRIMMING_PROBE_NUMBER){
			sprintf(temp,"%d",TRIMMING_PROBE_NUMBER);
			if((int)myDet->setTimer(slsDetectorDefs::PROBES_NUMBER,TRIMMING_PROBE_NUMBER) != TRIMMING_PROBE_NUMBER){
				qDefs::Message(qDefs::WARNING,
						string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>"
								"<nobr>Could not set <b>Number of Probes</b> to ") + string(temp)+string(".</nobr><br>"
										"Trimming Aborted."),"qTabAdvanced::validateBeforeTrimming");
				return slsDetectorDefs::FAIL;
			}else
				message.append(string("<nobr><b>Number of Probes</b> has been changed to ") + string(temp) + string(".<nobr><br>"));
		}
		//Setting
		if(myDet->getSettings() == slsDetectorDefs::UNINITIALIZED){
			if(qDefs::Message(qDefs::QUESTION,
					string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>")+
					string("<nobr><b>Settings</b> cannot be <b>Uninitialized</b> to start Trimming.</nobr><br>"
							"Change it to <b>Standard</b> and proceed?"),"qTabAdvanced::validateBeforeTrimming") == slsDetectorDefs::FAIL){
				qDefs::Message(qDefs::INFORMATION,
						"<nobr>Please change the <b>Settings</b> in the Settings tab to your choice.</nobr><br>"
						"Aborting Trimming.","qTabAdvanced::validateBeforeTrimming");
				return slsDetectorDefs::FAIL;
			}
			//user asked to change settings to standard
			else{
				if((int)myDet->setSettings(slsDetectorDefs::STANDARD) != slsDetectorDefs::STANDARD){
					qDefs::Message(qDefs::WARNING,
							string("<nobr>Trimming Pre-condition not satisfied:</nobr><br>"
									"<nobr>Could not change <b>Settings</b> to <b>Standard</b></nobr><br>"
									"Trimming Aborted."),"qTabAdvanced::validateBeforeTrimming");
					return slsDetectorDefs::FAIL;
				}else
					message.append(string("<nobr><b>Settings</b> has been changed to Standard.<nobr><br>"));
			}
		}
		break;
	default:
		return slsDetectorDefs::FAIL;

	}

	message.append("<nobr>Initiating Trimming...</nobr>");
	qDefs::Message(qDefs::INFORMATION,message,"qTabAdvanced::validateBeforeTrimming");
	return slsDetectorDefs::OK;

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::StartTrimming(){
	//check a few conditions before trimming
	if(validateBeforeTrimming() == slsDetectorDefs::FAIL)
		return;

#ifdef VERBOSE
	cout << "Starting Trimming" << endl;
#endif
	int parameter1=0, parameter2=0;
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
		cout << "Should never come here. Start Trimming will have only 2 methods. Trimming Method:" << trimmingMode << endl;
		return;
	}

	//execute
	int ret = myDet->executeTrimming(trimmingMode,parameter1,parameter2,-1);

	if((ret!=slsDetectorDefs::FAIL)&&(ret!=-1));
	else
	   qDefs::Message(qDefs::WARNING,"Atleast 1 channel could not be trimmed.","qTabAdvanced::StartTrimming");
	//save trim file
	ret = myDet->saveSettingsFile(string(dispFile->text().toAscii().constData()),-1);
	if((ret!=slsDetectorDefs::FAIL)&&(ret!=-1)){
		qDefs::Message(qDefs::INFORMATION,"The Trimbits have been saved successfully.","qTabAdvanced::StartTrimming");
		//updates plots
		myPlot->UpdateTrimbitPlot(false,radioHistogram->isChecked());
	}
	else qDefs::Message(qDefs::WARNING,string("Could not Save the Trimbits to file:\n")+dispFile->text().toAscii().constData(),"qTabAdvanced::StartTrimming");

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::StartTrimming");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::UpdateTrimbitPlot(int id){
	if(boxPlot->isChecked()){
		//refresh
		if(!id)	myPlot->UpdateTrimbitPlot(false,radioHistogram->isChecked());
		//from detector
		else	myPlot->UpdateTrimbitPlot(true,radioHistogram->isChecked());
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetControlPort(int port){
#ifdef VERBOSE
	cout << "Setting Control Port:" << port << endl;
#endif
	disconnect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
	spinControlPort->setValue(det->setPort(slsDetectorDefs::CONTROL_PORT,port));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetControlPort");
	connect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetStopPort(int port){
#ifdef VERBOSE
	cout << "Setting Stop Port:" << port << endl;
#endif
	disconnect(spinStopPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));
	spinStopPort->setValue(det->setPort(slsDetectorDefs::STOP_PORT,port));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetStopPort");
	connect(spinStopPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetRxrTCPPort(int port){
#ifdef VERBOSE
	cout << "Setting Receiver TCP Port:" << port << endl;
#endif
	disconnect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
	spinTCPPort->setValue(det->setPort(slsDetectorDefs::DATA_PORT,port));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetRxrTCPPort");
	connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetRxrUDPPort(int port){
#ifdef VERBOSE
	cout << "Setting Receiver UDP Port:" << port << endl;
#endif

	disconnect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
	spinUDPPort->setValue(det->setReceiverUDPPort(port));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetRxrUDPPort");
	connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetReceiverOnline(int index){
#ifdef VERBOSE
	cout << "Setting Reciever Online to :" << index << endl;
#endif
	disconnect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	if(index)
		SetReceiver();
	else
		comboRxrOnline->setCurrentIndex(det->setReceiverOnline(index));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetReceiverOnline");
	connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	//highlight in red if offline
	if(!comboRxrOnline->currentIndex()){
		comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setPalette(red);
		lblRxrOnline->setText("Online:*");
	}else{
		comboRxrOnline->setToolTip(rxrOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip);
		lblRxrOnline->setPalette(lblHostname->palette());
		lblRxrOnline->setText("Online:");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetOnline(int index){
#ifdef VERBOSE
	cout << "Setting Detector Online to " << index << endl;
#endif
	disconnect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));
	comboOnline->setCurrentIndex(det->setOnline(index));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetOnline");
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));
	//highlight in red if offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetNetworkParameters(){
#ifdef VERBOSE
	cout << "Setting Network Parametrs" << endl;
#endif
	disconnect(dispIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPIP,		SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPMAC,		SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));

	dispIP->setText(QString(det->setDetectorIP(dispIP->text().toAscii().constData())));
	dispMAC->setText(QString(det->setDetectorMAC(dispMAC->text().toAscii().constData())));
	dispUDPIP->setText(QString(det->setReceiverUDPIP(dispUDPIP->text().toAscii().constData())));
	dispUDPMAC->setText(QString(det->setReceiverUDPMAC(dispUDPMAC->text().toAscii().constData())));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetNetworkParameters");

	connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetReceiver(){
#ifdef VERBOSE
	cout << "Setting Receiver" << endl;
#endif
	string outdir = myDet->getFilePath();
	dispRxrHostname->setText(QString(det->setReceiver(dispRxrHostname->text().toAscii().constData())));
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetReceiver");
	det->setFilePath(outdir);
	qDefs::checkErrorMessage(det,"qTabAdvanced::SetReceiver");
	Refresh();
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::updateROIList(){
#ifdef VERYVERBOSE
	cout<<"in updateROIList() " << endl;
#endif
	clearROI();

	int n,i;
	slsDetectorDefs::ROI* temp = myDet->getROI(n);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::updateROIList");

	if((temp!=NULL)&&(n>0)){
		//assign into array, else it loses values cuz of memory
		slsDetectorDefs::ROI allroi[n];
		for(i=0;i<n;i++)
			allroi[i] =  temp[i];

		//add roi inputs
		AddROIInput(n);
		//populating roi list
		for (i=0;i<n;i++){
			spinFromX[i]->setValue(allroi[i].xmin);
			spinFromY[i]->setValue(allroi[i].ymin);
			spinToX[i]->setValue(allroi[i].xmax);
			spinToY[i]->setValue(allroi[i].ymax);
		}
		cout << "ROIs populated: " << n << endl;
	}


}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::AddROIInput(int num){
#ifdef VERVERBOSE
	cout<<"in AddROIInput() " << num << endl;
#endif
	if((int)lblFromX.size()){
		disconnect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	}

	int exists = numRois+1;
	int total = exists+num;
	//if cleared, addding just one
	if ((num==0) && (numRois==0)){
		exists = 0;
		total = 1;
	}/*else{
		gridRoi->removeWidget
	}*/

	for (int i=exists;i<total;i++){

		if(i >= ((int)lblFromX.size())){
			lblFromX.resize(i+1);	spinFromX.resize(i+1);
			lblFromY.resize(i+1);	spinFromY.resize(i+1);
			lblToX.resize(i+1);	spinToX.resize(i+1);
			lblToY.resize(i+1);	spinToY.resize(i+1);

			lblFromX[i] 	= new QLabel("x min:");
			lblFromY[i] 	= new QLabel("y min:");
			lblToX[i] 		= new QLabel("x max:");
			lblToY[i] 		= new QLabel("y max:");
			spinFromX[i] 	= new QSpinBox();
			spinFromY[i] 	= new QSpinBox();
			spinToX[i] 		= new QSpinBox();
			spinToY[i] 		= new QSpinBox();


			lblFromX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblFromX[i]->setFixedWidth(50);
			lblFromY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblFromY[i]->setFixedWidth(50);
			lblToX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblToX[i]->setFixedWidth(50);
			lblToY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblToY[i]->setFixedWidth(50);
			spinFromX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinFromX[i]->setFixedWidth(80);
			spinFromY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinFromY[i]->setFixedWidth(80);
			spinToX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinToX[i]->setFixedWidth(80);
			spinToY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinToY[i]->setFixedWidth(80);
			spinFromX[i]->setFixedHeight(19);
			spinFromY[i]->setFixedHeight(19);
			spinToX[i]->setFixedHeight(19);
			spinToY[i]->setFixedHeight(19);

			spinFromX[i]->setMaximum(myDet->getMaxNumberOfChannels(slsDetectorDefs::X)-1);
			spinToX[i]->setMaximum(myDet->getMaxNumberOfChannels(slsDetectorDefs::X)-1);
			spinFromY[i]->setMaximum(myDet->getMaxNumberOfChannels(slsDetectorDefs::Y)-1);
			spinToY[i]->setMaximum(myDet->getMaxNumberOfChannels(slsDetectorDefs::Y)-1);
			spinFromX[i]->setMinimum(-1);
			spinToX[i]->setMinimum(-1);
			spinFromY[i]->setMinimum(-1);
			spinToY[i]->setMinimum(-1);
			spinFromX[i]->setValue(-1);
			spinFromY[i]->setValue(-1);
			spinToX[i]->setValue(-1);
			spinToY[i]->setValue(-1);
		}

		gridRoi->addWidget(lblFromX[i],	i,0,Qt::AlignTop);
		gridRoi->addWidget(spinFromX[i],i,1,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,2,Qt::AlignTop);
		gridRoi->addWidget(lblToX[i],	i,3,Qt::AlignTop);
		gridRoi->addWidget(spinToX[i],	i,4,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,5,Qt::AlignTop);
		gridRoi->addWidget(lblFromY[i],	i,6,Qt::AlignTop);
		gridRoi->addWidget(spinFromY[i],i,7,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,8,Qt::AlignTop);
		gridRoi->addWidget(lblToY[i],	i,9,Qt::AlignTop);
		gridRoi->addWidget(spinToY[i],	i,10,Qt::AlignTop);

		lblFromX[i]->show();
		spinFromX[i]->show();
		lblToX[i]->show();
		spinToX[i]->show();
		lblFromY[i]->show();
		spinFromY[i]->show();
		lblToY[i]->show();
		spinToY[i]->show();
	}

	numRois += num;

	connect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));

#ifdef VERYVERBOSE
	cout<<"ROI Inputs added " << num << endl;
#endif

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::AddROIInput");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::clearROI(){
#ifdef VERYVERBOSE
	cout<<"in clearROI() " << endl;
#endif
	if((int)lblFromX.size()){
		disconnect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));

	}


	for (int i=0;i<numRois;i++){
		spinFromX[i]->setValue(-1);
		spinFromY[i]->setValue(-1);
		spinToX[i]->setValue(-1);
		spinToY[i]->setValue(-1);
	}


	//hide widget because they are still visible even when removed and layout deleted
	QLayoutItem *item;
	while((item = gridRoi->takeAt(0))) {
		if (item->widget()){
			item->widget()->hide();
			gridRoi->removeWidget(item->widget());
		}
		//if (item->spacerItem())
	}

	numRois = 0;
	AddROIInput(0);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::setROI(){
#ifdef VERYVERBOSE
	cout<<"in setROI() " << endl;
#endif

	slsDetectorDefs::ROI allroi[MAX_ROIS];

	for (int i=0;i<numRois;i++){
		allroi[i].xmin = spinFromX[i]->value();
		allroi[i].ymin = spinFromY[i]->value();
		allroi[i].xmax = spinToX[i]->value();
		allroi[i].ymax = spinToY[i]->value();
	}

	myDet->setROI(numRois,allroi);
	//qDefs::checkErrorMessage(myDet);
	cout<<"ROIs set" << endl;
	//get the correct list back
	updateROIList();
	//configuremac
	myDet->configureMAC();

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::setROI");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::clearROIinDetector(){
#ifdef VERYVERBOSE
	cout<<"in clearROIinDetector() " << endl;
#endif

	if (QMessageBox::warning(this, "Clear ROI",
			"Are you sure you want to clear all the ROI in detector?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes){

		clearROI();
		setROI();
#ifdef VERBOSE
		cout << "ROIs cleared" << endl;
#endif
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetDetector(int index){
#ifdef VERYVERBOSE
	cout<<"in SetDetector: " << index << endl;
#endif
	det = myDet->getSlsDetector(comboDetector->currentIndex());


	spinControlPort->setValue(det->getControlPort());
	spinStopPort->setValue(det->getStopPort());
	spinTCPPort->setValue(det->getReceiverPort());
	spinUDPPort->setValue(atoi(det->getReceiverUDPPort()));

	dispIP->setText(det->getDetectorIP());
	dispMAC->setText(det->getDetectorMAC());
	dispRxrHostname->setText(det->getReceiver());
	dispUDPIP->setText(det->getReceiverUDPIP());
	dispUDPMAC->setText(det->getReceiverUDPMAC());


	//check if its online and set it to red if offline
	if(det->setOnline()==slsDetectorDefs::ONLINE_FLAG)
		det->checkOnline();
	if(det->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG)
		det->checkReceiverOnline();
	comboOnline->setCurrentIndex(det->setOnline());
	comboRxrOnline->setCurrentIndex(det->setReceiverOnline());
	//highlight in red if detector or receiver is offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}
	if(comboRxrOnline->isEnabled()){
		if(!comboRxrOnline->currentIndex()){
			comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setPalette(red);
			lblRxrOnline->setText("Online:*");
		}else{
			comboRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setPalette(lblHostname->palette());
			lblRxrOnline->setText("Online:");
		}
	}

	qDefs::checkErrorMessage(det,"qTabAdvanced::SetDetector");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetAllTrimbits(){
#ifdef VERBOSE
	cout<<"Set all trimbits to " << spinSetAllTrimbits->value() << endl;
#endif
	myDet->setAllTrimbits(spinSetAllTrimbits->value());
	 qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetAllTrimbits");
	 updateAllTrimbitsFromServer();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::updateAllTrimbitsFromServer(){
#ifdef VERBOSE
	cout<<"Getting all trimbits value" << endl;
#endif
	disconnect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));

	int ret = myDet->setAllTrimbits(-1);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::updateAllTrimbitsFromServer");
	if(ret<0){
		qDefs::Message(qDefs::WARNING,"Inconsistent value from alltrimbits value.\n"
				"Setting it for all detectors involved to 0.","qTabAdvanced::updateAllTrimbitsFromServer");
		//set to default
		spinSetAllTrimbits->setValue(0);
		myDet->setAllTrimbits(0);
		qDefs::checkErrorMessage(myDet,"qTabAdvanced::updateAllTrimbitsFromServer");
	}else
		spinSetAllTrimbits->setValue(ret);

	connect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Refresh(){


#ifdef VERBOSE
		cout  << endl << "**Updating Advanced Tab" << endl;
#endif
	//setting color of tab
		//void 	setTabBar ( QTabBar * tb )
		//QTabBar * 	tabBar () const
		/*
		for(int i=0;i<NumberOfTabs;i++)
			tabAdvancedSettings->tabBar()->setTabTextColor(i,defaultTabColor);
		tabAdvancedSettings->tabBar()->setTabTextColor(index,QColor(0,0,200,255));
		*/

	if(isAngular){
#ifdef VERBOSE
		cout << "Angular Calibration Log set to " << chkAngularLog->isChecked() << endl;
#endif

		disconnect(chkAngularLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));

		chkAngularLog->setChecked(myDet->getActionMode(slsDetectorDefs::angCalLog));

		connect(chkAngularLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
	}


	if(isEnergy){
		//disconnect
		disconnect(chkEnergyLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
		disconnect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
		disconnect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));
		disconnect(spinThreshold,	SIGNAL(valueChanged(double)),		this,	SLOT(SetThreshold()));


		//energy/angular logs
		chkEnergyLog->setChecked(myDet->getActionMode(slsDetectorDefs::enCalLog));
#ifdef VERBOSE
		cout << "Energy Calibration Log set to " << chkEnergyLog->isChecked() << endl;
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
		double threshold = (double)myDet->setDAC(-1,slsDetectorDefs::THRESHOLD,0);
#ifdef VERBOSE
		cout << "Getting Threshold DACu : " << threshold << endl;
#endif
		spinThreshold->setValue(threshold);


		//connect
		connect(chkEnergyLog,	SIGNAL(toggled(bool)),				this,	SLOT(SetLogs()));
		connect(spinExpTime,	SIGNAL(valueChanged(double)),		this,	SLOT(SetExposureTime()));
		connect(comboExpUnit,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetExposureTime()));
		connect(spinThreshold,	SIGNAL(valueChanged(double)),		this,	SLOT(SetThreshold()));
	}


	//network
	det = myDet->getSlsDetector(comboDetector->currentIndex());

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::Refresh");



#ifdef VERBOSE
		cout << "Getting Detector Ports" << endl;
#endif
	//disconnect
	disconnect(spinControlPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetControlPort(int)));
	disconnect(spinStopPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetStopPort(int)));
	disconnect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));

	//so that updated status
	if(det->setOnline()==slsDetectorDefs::ONLINE_FLAG)
		det->checkOnline();
	comboOnline->setCurrentIndex(det->setOnline());
	spinControlPort->setValue(det->getControlPort());
	spinStopPort->setValue(det->getStopPort());

	//connect
	connect(spinControlPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetControlPort(int)));
	connect(spinStopPort,		SIGNAL(valueChanged(int)),			this,	SLOT(SetStopPort(int)));
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));


#ifdef VERBOSE
		cout << "Getting Receiver Network Information" << endl;
#endif
	if ((detType==slsDetectorDefs::GOTTHARD) ||
			(detType==slsDetectorDefs::MOENCH)||
			(detType==slsDetectorDefs::PROPIX)||
			(detType==slsDetectorDefs::JUNGFRAU)||
			(detType==slsDetectorDefs::EIGER)){
		//disconnect
		disconnect(spinTCPPort,			SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
		disconnect(spinUDPPort,			SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
		disconnect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
		disconnect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		disconnect(dispMAC,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		disconnect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		disconnect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		disconnect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));

		dispIP->setText(det->getDetectorIP());
		dispMAC->setText(det->getDetectorMAC());

		//so that updated status
		if(det->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG)
			det->checkReceiverOnline();
		comboRxrOnline->setCurrentIndex(det->setReceiverOnline());

		dispRxrHostname->setText(det->getReceiver());
		spinTCPPort->setValue(det->getReceiverPort());
		spinUDPPort->setValue(atoi(det->getReceiverUDPPort()));

		dispUDPIP->setText(det->getReceiverUDPIP());
		dispUDPMAC->setText(det->getReceiverUDPMAC());

		//connect
		connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
		connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
		connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
		connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
		connect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));

	}

	//highlight in red if detector or receiver is offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}
	if(comboRxrOnline->isEnabled()){
		if(!comboRxrOnline->currentIndex()){
			comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setPalette(red);
			lblRxrOnline->setText("Online:*");
		}else{
			comboRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setPalette(lblHostname->palette());
			lblRxrOnline->setText("Online:");
		}
	}

	//roi
#ifdef VERBOSE
		cout << "Getting ROI" << endl;
#endif
	updateROIList();

	//update alltirmbits from server
	if(boxSetAllTrimbits->isEnabled())
		updateAllTrimbitsFromServer();

#ifdef VERBOSE
		cout  << "**Updated Advanced Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(det,"qTabAdvanced::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


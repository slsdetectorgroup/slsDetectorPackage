/*
 * qTabDataOutput.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
/**********************************************************************
 * TO DO
 * 1. Rate correction auto: for eiger depends on settings, tdeadtime{vv,vv,vv} in postprocessing.h
 * ********************************************************************/

#include "qTabDataOutput.h"
#include "qDefs.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QFileDialog>
// C++ Include Headers
#include <iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDataOutput::qTabDataOutput(QWidget *parent,multiSlsDetector*& detector,int detID):
						QWidget(parent),myDet(detector),detID(detID){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
	Refresh();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDataOutput::~qTabDataOutput(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetupWidgetWindow(){
	// Detector Type
	detType=myDet->getDetectorsType();

	//rate correction  - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER))
		chkRate->setEnabled(true);

	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD))
		chkAngular->setEnabled(true);

	/** error message **/
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	flatFieldTip = dispFlatField->toolTip();
	errFlatFieldTip = QString("<nobr>Flat field corrections.</nobr><br>"
			"<nobr> #flatfield# filename</nobr><br><br>")+
			QString("<nobr><font color=\"red\">"
					"Enter a valid file to enable Flat Field.</font></nobr>");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Initialization(){
	//output dir
	connect(dispOutputDir,		SIGNAL(textChanged(const QString&)), 	this, 	SLOT(setOutputDir(const QString&)));
	connect(btnOutputBrowse,	SIGNAL(clicked()), 						this, 	SLOT(browseOutputDir()));
	//flat field correction
	connect(chkFlatField,		SIGNAL(toggled(bool)), 		this, 	SLOT(SetFlatField()));
	connect(btnFlatField,		SIGNAL(clicked()), 			this, 	SLOT(BrowseFlatFieldPath()));
	//rate correction
	connect(chkRate,			SIGNAL(toggled(bool)), 			this,	SLOT(SetRateCorrection()));
	connect(radioAuto,			SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,		SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));
	//angular correction
	connect(chkAngular,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetAngularCorrection()));
	//discard bad channels
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

}
//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::setOutputDir(const QString& path){
	myDet->setFilePath(string(path.toAscii().constData()));
#ifdef VERBOSE
	cout << "Output Directory changed to :"<<myDet->getFilePath() << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::browseOutputDir()
{
	QString directory = QFileDialog::getExistingDirectory(this,tr("Choose Output Directory "),dispOutputDir->text());
	if (!directory.isEmpty())
		dispOutputDir->setText(directory);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetFlatField(){
#ifdef VERYVERBOSE
	cout << "Entering Set Flat Field Correction Function" << endl;
#endif
	// so that it doesnt call it twice
	disconnect(dispFlatField,		SIGNAL(editingFinished()),	this, 			SLOT(SetFlatField()));

	//enable/disable
	dispFlatField->setEnabled(chkFlatField->isChecked());
	btnFlatField->setEnabled(chkFlatField->isChecked());

	if(chkFlatField->isChecked()){
		if(dispFlatField->text().isEmpty()){
			chkFlatField->setToolTip(errFlatFieldTip);
			dispFlatField->setToolTip(errFlatFieldTip);
			chkFlatField->setPalette(red);
			chkFlatField->setText("Flat Field File:*");
#ifdef VERBOSE
		cout << "Flat Field File is not set." << endl;
#endif
		}else{
			QString fName = dispFlatField->text();
			QString file = fName.section('/',-1);
			QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);

			chkFlatField->setToolTip(flatFieldTip);
			dispFlatField->setToolTip(flatFieldTip);
			chkFlatField->setPalette(chkRate->palette());
			chkFlatField->setText("Flat Field File:");
			//set ff dir
			myDet->setFlatFieldCorrectionDir(dir.toAscii().constData());
			//set ff file and catch error if -1
			if(myDet->setFlatFieldCorrectionFile(file.toAscii().constData())<0){
				string sDir = dir.toAscii().constData(),sFile = file.toAscii().constData();
				if(sDir.length()<1) {sDir = string(QDir::current().absolutePath().toAscii().constData()); /*"/home/";*/}
				qDefs::WarningMessage("Invalid Flat Field file: "+sDir+"/"+sFile+
						".\nUnsetting Flat Field.","Data Output");

				//Unsetting flat field
				myDet->setFlatFieldCorrectionFile("");
				dispFlatField->setText("");
				chkFlatField->setToolTip(errFlatFieldTip);
				dispFlatField->setToolTip(errFlatFieldTip);
				chkFlatField->setPalette(red);
				chkFlatField->setText("Flat Field File:*");
#ifdef VERBOSE
		cout << "Invalid Flat Field File - "<< sDir << sFile << ". Unsetting Flat Field." << endl;
#endif
			}
			else{
#ifdef VERBOSE
		cout << "Setting flat field file to "<< dispFlatField->text().toAscii().constData() << endl;
#endif
			}
		}
	}else{
		chkFlatField->setToolTip(flatFieldTip);
		dispFlatField->setToolTip(flatFieldTip);
		chkFlatField->setPalette(chkRate->palette());
		chkFlatField->setText("Flat Field File:");
		//Unsetting flat field
		myDet->setFlatFieldCorrectionFile("");
		dispFlatField->setText("");
#ifdef VERBOSE
		cout << "Unsetting flat field correction file" << endl;
#endif
	}

	connect(dispFlatField,SIGNAL(editingFinished()),this,SLOT(SetFlatField()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDataOutput::UpdateFlatFieldFromServer(){
	disconnect(dispFlatField,	SIGNAL(editingFinished()),	this, 	SLOT(SetFlatField()));

	dispFlatField->setText(QString(myDet->getFlatFieldCorrectionDir().c_str())+"/"+QString(myDet->getFlatFieldCorrectionFile().c_str()));
#ifdef VERBOSE
	cout << "Getting flat field correction file" << dispFlatField->text().toAscii().constData() << endl;
#endif
	//calls setflatfield to ensure the file still exists or disable it
	if(!QString(myDet->getFlatFieldCorrectionFile().c_str()).compare("none")){
		dispFlatField->setText("");
		chkFlatField->setChecked(false);
#ifdef VERBOSE
		cout << "Flat Field is not set." << endl;
#endif
	}
	else
		chkFlatField->setChecked(true);

	chkFlatField->setToolTip(flatFieldTip);
	dispFlatField->setToolTip(flatFieldTip);
	chkFlatField->setPalette(chkRate->palette());
	chkFlatField->setText("Flat Field File:");

	connect(dispFlatField,		SIGNAL(editingFinished()),	this,	SLOT(SetFlatField()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDataOutput::BrowseFlatFieldPath()
{
	QString fName = dispFlatField->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir =  QString(myDet->getFlatFieldCorrectionDir().c_str());/*"/home/";*/
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Flat Field Correction File"),dir,
			tr("Flat Field Correction Files(*.dat)"),0,QFileDialog::ShowDirsOnly);
	if (!fName.isEmpty()){
		dispFlatField->setText(fName);
		SetFlatField();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetRateCorrection(){
	disconnect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));

#ifdef VERBOSE
	cout << "Entering Set Rate Correction function" << endl;
#endif
	slsDetector *s = myDet->getSlsDetector(detID);
	if(chkRate->isChecked()){
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		//set auto as default if nothing selected
		if(!radioAuto->isChecked()&&!radioDeadTime->isChecked())
			radioAuto->setChecked(true);
		//auto
		if(radioAuto->isChecked()){
			spinDeadTime->setEnabled(false);
			s->setRateCorrection(-1);
#ifdef VERBOSE
			cout << "Setting rate corrections with default dead time"  << endl;
#endif
		}//custom dead time
		else{
			spinDeadTime->setEnabled(true);
			s->setRateCorrection((double)spinDeadTime->value());
#ifdef VERBOSE
			cout << "Setting rate corrections with dead time "<< spinDeadTime->value() << endl;
#endif
		}
	}//unsetting
	else{
		radioAuto->setEnabled(false);
		radioDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
		//Unsetting rate correction

		s->setRateCorrection(0);
#ifdef VERBOSE
		cout << "Unsetting rate correction" << endl;
#endif
	}
	connect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateRateCorrectionFromServer(){
	disconnect(chkRate,			SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));

	double rate;
	slsDetector *s = myDet->getSlsDetector(detID);
	rate = (double)s->getRateCorrectionTau();
#ifdef VERBOSE
	cout << "Getting rate correction from server:" << rate << " : ";
#endif
	if(rate==0){
#ifdef VERBOSE
	cout << "None" << endl;
#endif
		radioAuto->setEnabled(false);
		radioDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);

		chkRate->setChecked(false);
	}else if(rate<0){
#ifdef VERBOSE
	cout << "Auto" << endl;
#endif
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(false);

		chkRate->setChecked(true);
		radioAuto->setChecked(true);
	}else{
#ifdef VERBOSE
	cout << "Custom" << endl;
#endif
		radioAuto->setEnabled(true);
		radioDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(true);

		chkRate->setChecked(true);
		radioDeadTime->setChecked(true);
		spinDeadTime->setValue((double)rate);
	}

	connect(chkRate,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioAuto,		SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(radioDeadTime,	SIGNAL(toggled(bool)), 			this, 	SLOT(SetRateCorrection()));
	connect(spinDeadTime,	SIGNAL(valueChanged(double)), 	this, 	SLOT(SetRateCorrection()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetAngularCorrection(){
#ifdef VERYVERBOSE
	cout << "Entering Set Angular Correction function" << endl;
#endif
	if(chkAngular->isChecked()){
		if(myDet->setAngularConversionFile("default")){
#ifdef VERBOSE
		cout << "Setting angular conversion to default"  << endl;
#endif
		}else{
			qDefs::WarningMessage("Angular Conversion could not be set.","Data Output");
			chkAngular->setChecked(false);
		}
	}else{
		myDet->setAngularConversionFile("");
#ifdef VERBOSE
		cout << "Unsetting angular correction" << endl;
#endif
	}
	emit AngularConversionSignal(chkAngular->isChecked());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::DiscardBadChannels(){
#ifdef VERYVERBOSE
	cout << "Entering Discard bad channels function" << endl;
#endif
	if(chkDiscardBad->isChecked()){
		myDet->setBadChannelCorrection("default");
#ifdef VERBOSE
		cout << "Setting bad channel correction to default"  << endl;
#endif
	}else{
		myDet->setBadChannelCorrection("");
#ifdef VERBOSE
		cout << "Unsetting bad channel correction" << endl;
#endif
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Refresh(){
	// output dir
	dispOutputDir->setText(QString(myDet->getFilePath().c_str()));
	//flat field correction from server
	UpdateFlatFieldFromServer();
	//rate correction - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER))
		UpdateRateCorrectionFromServer();
	//update angular conversion from server
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD)){
		int ang;
		if(myDet->getAngularConversion(ang))
			chkAngular->setChecked(true);
		emit AngularConversionSignal(chkAngular->isChecked());
	}
	//discard bad channels from server
	if(myDet->getBadChannelCorrection()) chkDiscardBad->setChecked(true);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

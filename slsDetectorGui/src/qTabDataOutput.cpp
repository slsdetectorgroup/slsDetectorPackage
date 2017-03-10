/*
 * qTabDataOutput.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
/**********************************************************************
 * ********************************************************************/

#include "qTabDataOutput.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QStandardItemModel>
#include <QFileDialog>
// C++ Include Headers
#include <iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------



qTabDataOutput::qTabDataOutput(QWidget *parent,multiSlsDetector*& detector):
						QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
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
	widgetEiger->setVisible(false);

	//rate correction  - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER))
		chkRate->setEnabled(true);

	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD))
		chkAngular->setEnabled(true);

	if(detType == slsDetectorDefs::EIGER){
		chkRate->setEnabled(true);
		chkTenGiga->setEnabled(true);
		widgetEiger->setVisible(true);
	}

	/** error message **/
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	black = QPalette();
	black.setColor(QPalette::Active,QPalette::WindowText,Qt::black);

	red1 = new QPalette();
	red1->setColor(QPalette::Text,Qt::red);
	black1 = new QPalette();
	black1->setColor(QPalette::Text,Qt::black);

	flatFieldTip = dispFlatField->toolTip();
	errFlatFieldTip = QString("<nobr>Flat field corrections.</nobr><br>"
			"<nobr> #flatfield# filename</nobr><br><br>")+
			QString("<nobr><font color=\"red\">"
					"Enter a valid file to enable Flat Field.</font></nobr>");
	outDirTip = lblOutputDir->toolTip();


	//expert mode is not enabled initially
	chkCompression->setEnabled(false);

	//enabling file format depending on detector type
	SetupFileFormat();

	Initialization();

	disconnect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	PopulateDetectors();
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));

	//flat field correction from server
#ifdef VERBOSE
	cout  << "Getting flat field" << endl;
#endif
	UpdateFlatFieldFromServer();


	//rate correction - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER)){
#ifdef VERBOSE
		cout  << "Getting rate correction" << endl;
#endif
		UpdateRateCorrectionFromServer();
	}


	//update angular conversion from server
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD)){
#ifdef VERBOSE
		cout  << "Getting angular conversion" << endl;
#endif
		int ang;
		if(myDet->getAngularConversion(ang))
			chkAngular->setChecked(true);
		emit AngularConversionSignal(chkAngular->isChecked());
	}


	//discard bad channels from server
#ifdef VERBOSE
	cout  << "Getting bad channel correction:" << myDet->getBadChannelCorrection() << endl;
#endif
	disconnect(chkDiscardBad,		SIGNAL(toggled(bool)));
	if(myDet->getBadChannelCorrection())
		chkDiscardBad->setChecked(true);
	else
		chkDiscardBad->setChecked(false);
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

/*
	if(detType == slsDetectorDefs::MYTHEN){
		comboDetector->hide();
	}
*/
	qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetupWidgetWindow");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Initialization(){
	//output dir
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));

	//overwrite enable
	connect(chkOverwriteEnable,	SIGNAL(toggled(bool)), 		this, 	SLOT(SetOverwriteEnable(bool)));

	//file format
	connect(comboFileFormat,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFileFormat(int)));

	connect(btnOutputBrowse,	SIGNAL(clicked()), 			this, 	SLOT(BrowseOutputDir()));
	//flat field correction
	connect(chkFlatField,		SIGNAL(toggled(bool)), 		this, 	SLOT(SetFlatField()));
	connect(btnFlatField,		SIGNAL(clicked()), 			this, 	SLOT(BrowseFlatFieldPath()));
	//rate correction
	connect(chkRate,			SIGNAL(toggled(bool)), 		this,	SLOT(SetRateCorrection()));
	connect(btnDefaultRate,		SIGNAL(clicked()), 			this, 	SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime,		SIGNAL(editingFinished()), 	this, 	SLOT(SetRateCorrection()));

	//angular correction
	connect(chkAngular,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetAngularCorrection()));
	//discard bad channels
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));
	//compression
	connect(chkCompression,		SIGNAL(toggled(bool)), 	this, 	SLOT(SetCompression(bool)));
	//10GbE
	connect(chkTenGiga,			SIGNAL(toggled(bool)), 	this, 	SLOT(EnableTenGigabitEthernet(bool)));

	//eiger
	if(widgetEiger->isVisible()){
		//speed
		connect(comboEigerClkDivider,SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetSpeed()));
		//flags
		connect(comboEigerFlags1,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));
		connect(comboEigerFlags2,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetExpertMode(bool enable){
	if((detType == slsDetectorDefs::GOTTHARD) || (detType == slsDetectorDefs::MOENCH)){
		chkCompression->setEnabled(enable);
		GetCompression();
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::BrowseOutputDir()
{
	QString directory = QFileDialog::getExistingDirectory(this,tr("Choose Output Directory "),dispOutputDir->text());
	if (!directory.isEmpty())
		dispOutputDir->setText(directory);
	SetOutputDir();
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
			chkFlatField->setPalette(chkDiscardBad->palette());
			chkFlatField->setText("Flat Field File:");
			//set ff dir
			myDet->setFlatFieldCorrectionDir(dir.toAscii().constData());
			//set ff file and catch error if -1
			if(myDet->setFlatFieldCorrectionFile(file.toAscii().constData())<0){
				string sDir = dir.toAscii().constData(),sFile = file.toAscii().constData();
				if(sDir.length()<1) {sDir = string(QDir::current().absolutePath().toAscii().constData()); /*"/home/";*/}
				qDefs::Message(qDefs::WARNING,"Invalid Flat Field file: "+sDir+"/"+sFile+
						".\nUnsetting Flat Field.","qTabDataOutput::SetFlatField");

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
		chkFlatField->setPalette(chkDiscardBad->palette());
		chkFlatField->setText("Flat Field File:");
		//Unsetting flat field
		myDet->setFlatFieldCorrectionFile("");
		dispFlatField->setText("");
#ifdef VERBOSE
		cout << "Unsetting flat field correction file" << endl;
#endif
	}

	connect(dispFlatField,SIGNAL(editingFinished()),this,SLOT(SetFlatField()));

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetFlatField");
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
	chkFlatField->setPalette(chkDiscardBad->palette());
	chkFlatField->setText("Flat Field File:");

	connect(dispFlatField,		SIGNAL(editingFinished()),	this,	SLOT(SetFlatField()));

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::UpdateFlatFieldFromServer");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDataOutput::BrowseFlatFieldPath(){
	QString fName = dispFlatField->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir =  QString(myDet->getFlatFieldCorrectionDir().c_str());/*"/home/";*/
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Flat Field Correction File"),dir,
					     tr("Data Files(*.raw *.dat);; All Files (*.*)"),0,QFileDialog::ShowDirsOnly);
	if (!fName.isEmpty()){
		dispFlatField->setText(fName);
		SetFlatField();
	}

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::BrowseFlatFieldPath");

}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetRateCorrection(int deadtime){
	disconnect(btnDefaultRate,	SIGNAL(clicked()), 			this, 	SLOT(SetDefaultRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(editingFinished()), 	this, 	SLOT(SetRateCorrection()));

#ifdef VERBOSE
	cout << "Entering Set Rate Correction function" << endl;
#endif

	if(chkRate->isChecked()){
		if(!btnDefaultRate->isEnabled()){
			btnDefaultRate->setEnabled(true);
			lblDeadTime->setEnabled(true);
			spinDeadTime->setEnabled(true);
		}

		if(deadtime!=-1){
			deadtime = (double)spinDeadTime->value();
#ifdef VERBOSE
			cout << "Setting rate corrections with custom dead time: "  << deadtime << endl;
#endif
		}else{;
#ifdef VERBOSE
		cout << "Setting rate corrections with default dead time"  <<  endl;
#endif
		}
		myDet->setRateCorrection(deadtime);

	}//unsetting rate correction
	else{
		btnDefaultRate->setEnabled(false);
		lblDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
		myDet->setRateCorrection(0);
#ifdef VERBOSE
		cout << "Unsetting rate correction" << endl;
#endif
	}
	qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetRateCorrection");

	//update just the value
	double rate = (double)myDet->getRateCorrectionTau();
	spinDeadTime->setValue((double)rate);
	if(rate == -1){
		qDefs::Message(qDefs::WARNING,"Dead time is inconsistent for all detectors. Returned Value: -1.","qTabDataOutput::UpdateRateCorrectionFromServer");
		QString errorTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>")+
				QString("<nobr><font color=\"red\">"
						"Dead time is inconsistent for all detectors.</font></nobr>");
		chkRate->setToolTip(errorTip);
		spinDeadTime->setToolTip(errorTip);
		chkRate->setPalette(red);
		chkRate->setText("Rate:*");
	}else{
		QString normalTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>");
		chkRate->setToolTip(normalTip);
		spinDeadTime->setToolTip(normalTip);
		chkRate->setPalette(chkDiscardBad->palette());
		chkRate->setText("Rate:");
	}

	connect(btnDefaultRate,	SIGNAL(clicked()), 			this, 	SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime,	SIGNAL(editingFinished()), 	this, 	SLOT(SetRateCorrection()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetDefaultRateCorrection(){
	SetRateCorrection(-1);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateRateCorrectionFromServer(){
	disconnect(chkRate,			SIGNAL(toggled(bool)), 		this,	SLOT(SetRateCorrection()));
	disconnect(btnDefaultRate,	SIGNAL(clicked()), 			this, 	SLOT(SetDefaultRateCorrection()));
	disconnect(spinDeadTime,	SIGNAL(editingFinished()), 	this, 	SLOT(SetRateCorrection()));

	double rate;
	rate = (double)myDet->getRateCorrectionTau();
	qDefs::checkErrorMessage(myDet,"qTabDataOutput::UpdateRateCorrectionFromServer");
#ifdef VERBOSE
	cout << "Getting rate correction from server: " << rate << endl;
#endif
	if(rate==0){
		chkRate->setChecked(false);
		btnDefaultRate->setEnabled(false);
		lblDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
	}

	else{
		chkRate->setChecked(true);
		btnDefaultRate->setEnabled(true);
		lblDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(true);
		spinDeadTime->setValue((double)rate);
	}

	if(rate == -1){
		qDefs::Message(qDefs::WARNING,"Dead time is inconsistent for all detectors. Returned Value: -1.","qTabDataOutput::UpdateRateCorrectionFromServer");
		QString errorTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>")+
				QString("<nobr><font color=\"red\">"
						"Dead time is inconsistent for all detectors.</font></nobr>");
		chkRate->setToolTip(errorTip);
		spinDeadTime->setToolTip(errorTip);
		chkRate->setPalette(red);
		chkRate->setText("Rate:*");
	}else{
		QString normalTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>");
		chkRate->setToolTip(normalTip);
		spinDeadTime->setToolTip(normalTip);
		chkRate->setPalette(chkDiscardBad->palette());
		chkRate->setText("Rate:");
	}

	connect(chkRate,		SIGNAL(toggled(bool)), 		this,	SLOT(SetRateCorrection()));
	connect(btnDefaultRate,	SIGNAL(clicked()), 			this, 	SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime,	SIGNAL(editingFinished()), 	this, 	SLOT(SetRateCorrection()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetAngularCorrection(){
	disconnect(chkAngular,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetAngularCorrection()));
#ifdef VERYVERBOSE
	cout << "Entering Set Angular Correction function" << endl;
#endif
	bool enabled = chkAngular->isChecked();
	//set
	if(myDet->setAngularCorrectionMask(enabled) == enabled){
#ifdef VERBOSE
		cout << "Angular Conversion mask:"  << enabled << endl;
#endif
	}
	//error
	else{
#ifdef VERBOSE
		cout << "Could not set angular conversion to default"  << endl;
#endif
		qDefs::Message(qDefs::WARNING,"Angular Conversion could not be set/reset. Please set the default file name using the command line, if you want to set it.","qTabDataOutput::SetAngularCorrection");
		chkAngular->setChecked(!enabled);
	}

	emit AngularConversionSignal(chkAngular->isChecked());
	connect(chkAngular,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetAngularCorrection()));
	qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetAngularCorrection");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::DiscardBadChannels(){
#ifdef VERBOSE
	cout << "Entering Discard bad channels function" << endl;
#endif
	if(chkDiscardBad->isChecked()){
#ifdef VERBOSE
		cout << "Setting bad channel correction to default"  << endl;
#endif
		myDet->setBadChannelCorrection("default");
	}else{
#ifdef VERBOSE
		cout << "Unsetting bad channel correction" << endl;
#endif
		myDet->setBadChannelCorrection("");
	}

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::DiscardBadChannels");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::PopulateDetectors(){
#ifdef VERBOSE
	cout << "Populating detectors" << endl;
#endif
	comboDetector->clear();
	comboDetector->addItem("All");
	lblOutputDir->setText("Path:");
	//add specific detector options only if more than 1 detector
	if(myDet->getNumberOfDetectors()>1){
		for(int i=0;i<myDet->getNumberOfDetectors();i++)
			comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
	}
	GetOutputDir();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::GetOutputDir(){
#ifdef VERBOSE
	cout << "Getting output directory" << endl;
#endif

	disconnect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));
	//all
	if(!comboDetector->currentIndex()) {
		dispOutputDir->setText(QString(myDet->getFilePath().c_str()));
		//multi file path blank means sls file paths are different
		if (dispOutputDir->text().isEmpty()) {
			qDefs::Message(qDefs::INFORMATION,"The file path for individual units are different.\n"
					"Hence, leaving the common field blank.","qTabDataOutput::GetOutputDir");
#ifdef VERBOSE
						cout << "The file path for individual units are different.\n"
					"Hence, leaving the common field blank." << endl;
#endif
						QString errTip = QString("<br><nobr><font color=\"red\">"
												"<b>Output Directory</b> Information only: The file path for individual units are different.<br>"
												"Hence, leaving the common field blank.</font></nobr>");
						lblOutputDir->setText("Path*:");
						lblOutputDir->setPalette(red);
						lblOutputDir->setToolTip(errTip);
						btnOutputBrowse->setToolTip(errTip);
						dispOutputDir->setToolTip(errTip);
		} else {
			lblOutputDir->setText("Path:");
			lblOutputDir->setPalette(*black1);
			lblOutputDir->setToolTip(outDirTip);
			btnOutputBrowse->setToolTip(outDirTip);
			dispOutputDir->setToolTip(outDirTip);
		}
	}

	//specific
	else{
		slsDetector *det = 	myDet->getSlsDetector(comboDetector->currentIndex()-1);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::GetOutputDir");
		dispOutputDir->setText(QString(det->getFilePath().c_str()));
	}

	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qTabDataOutput::VerifyOutputDirectory(){
#ifdef VERBOSE
	cout << "Verifying output directory" << endl;
#endif

	GetOutputDir();

	bool error = false;
	string detName = "";
	string mess = "";

	//common
	myDet->setFilePath(myDet->getFilePath());
	if(!qDefs::checkErrorMessage(myDet,"qTabDataOutput::VerifyOutputDirectory").empty())
		error = true;

	//for each detector
	for(int i=0;i<myDet->getNumberOfDetectors();i++){
		slsDetector *det = 	myDet->getSlsDetector(i);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::VerifyOutputDirectory");
		detName = string("\n - ") + string(comboDetector->itemText(i+1).toAscii().constData());
		det->setFilePath(det->getFilePath());
		if(!qDefs::checkErrorMessage(det,"qTabDataOutput::VerifyOutputDirectory").empty()) {
			mess. append(detName);
			error = true;
		}
	}

	//invalid
	if(error){
		qDefs::Message(qDefs::WARNING,string("Invalid Output Directory ")+ mess ,"qTabDataOutput::VerifyOutputDirectory");
#ifdef VERBOSE
		cout << "The output path doesnt exist anymore" << endl;
#endif
		//replace all \n with <br>
		size_t pos = 0;
		while((pos = mess.find("\n", pos)) != string::npos){
			mess.replace(pos, 1, "<br>");
			pos += 1;
		}
		QString errTip = outDirTip +
				QString("<br><nobr><font color=\"red\">"
				"Invalid <b>Output Directory</b>") +
				QString(mess.c_str()) +
				QString( ".</font></nobr>");
		lblOutputDir->setText("Path*:");
		lblOutputDir->setPalette(red);
		lblOutputDir->setToolTip(errTip);
		btnOutputBrowse->setToolTip(errTip);
		dispOutputDir->setToolTip(errTip);

		return slsDetectorDefs::FAIL;
	}

	//valid
	else{
#ifdef VERBOSE
		cout << "The output pathid valid" << endl;
#endif
		lblOutputDir->setText("Path:");
		lblOutputDir->setPalette(*black1);
		lblOutputDir->setToolTip(outDirTip);
		btnOutputBrowse->setToolTip(outDirTip);
		dispOutputDir->setToolTip(outDirTip);
	}

	return slsDetectorDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetOutputDir(){

#ifdef VERBOSE
	cout << "Setting output directory" << endl;
#endif

	disconnect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));


	bool error = false;
	QString path = dispOutputDir->text();

	//empty
	if(path.isEmpty()) {
		qDefs::Message(qDefs::WARNING,"Invalid Output Path. Must not be empty.","qTabDataOutput::SetOutputDir");
#ifdef VERBOSE
			cout << "Invalid Output Path. Must not be empty." << endl;
#endif
		error = true;
	}
	//gets rid of the end '/'s
	else if (path.endsWith('/')){
		while(path.endsWith('/'))
			path.chop(1);
		dispOutputDir->setText(path);
	}

	//specific
	if(comboDetector->currentIndex()){
		slsDetector *det = 	myDet->getSlsDetector(comboDetector->currentIndex()-1);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetOutputDir");
		det->setFilePath(string(dispOutputDir->text().toAscii().constData()));
		if(!qDefs::checkErrorMessage(det,"qTabDataOutput::SetOutputDir").empty())
			error = true;
	}

	//multi
	else{
		myDet->setFilePath(string(path.toAscii().constData()));
		if(!qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetOutputDir").empty())
			error = true;
	}


	if(error){
#ifdef VERBOSE
		cout << "The output path could not be set" << endl;
#endif
		QString errTip = outDirTip + QString("<br><nobr><font color=\"red\">"
								"Invalid <b>File Path</b></font></nobr>");

		lblOutputDir->setText("Path*:");
		lblOutputDir->setPalette(red);
		lblOutputDir->setToolTip(errTip);
		btnOutputBrowse->setToolTip(errTip);
		dispOutputDir->setToolTip(errTip);
	}
	else{
#ifdef VERBOSE
		cout << "The output path has been modified" << endl;
#endif
		lblOutputDir->setText("Path:");
		lblOutputDir->setPalette(*black1);
		lblOutputDir->setToolTip(outDirTip);
		btnOutputBrowse->setToolTip(outDirTip);
		dispOutputDir->setToolTip(outDirTip);

	}


	connect(dispOutputDir,		SIGNAL(editingFinished()), 	this, 	SLOT(SetOutputDir()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::GetCompression(){
	disconnect(chkCompression,		SIGNAL(toggled(bool)), 	this, 	SLOT(SetCompression(bool)));
	int ret = myDet->enableReceiverCompression();
	if(ret > 0)	chkCompression->setChecked(true);
	else		chkCompression->setChecked(false);
	connect(chkCompression,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetCompression(bool)));

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::GetCompression");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetCompression(bool enable){
	disconnect(chkCompression,		SIGNAL(toggled(bool)), 	this, 	SLOT(SetCompression(bool)));
	int ret = myDet->enableReceiverCompression(enable);
	if(ret > 0)	chkCompression->setChecked(true);
	else		chkCompression->setChecked(false);
	connect(chkCompression,			SIGNAL(toggled(bool)), 	this, 	SLOT(SetCompression(bool)));

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetCompression");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::EnableTenGigabitEthernet(bool enable,int get){
#ifdef VERBOSE
	cout  << endl << "Enabling/Disabling 10GbE" << endl;
#endif
	disconnect(chkTenGiga,	SIGNAL(toggled(bool)), 	this, 	SLOT(EnableTenGigabitEthernet(bool)));
	int ret;
	if(get)
		ret = myDet->enableTenGigabitEthernet(-1);
	else
		ret = myDet->enableTenGigabitEthernet(enable);
	if(ret > 0)	chkTenGiga->setChecked(true);
	else		chkTenGiga->setChecked(false);
	connect(chkTenGiga,		SIGNAL(toggled(bool)), 	this, 	SLOT(EnableTenGigabitEthernet(bool)));

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::EnableTenGigabitEthernet");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetSpeed(){
#ifdef VERBOSE
	cout  << endl << "Setting Speed" << endl;
#endif
	if(widgetEiger->isVisible()){
		 myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,comboEigerClkDivider->currentIndex());
		 qDefs::checkErrorMessage(myDet,"qTabDataOutput::SetSpeed");
		 UpdateSpeedFromServer();
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetFlags(){
#ifdef VERBOSE
	cout  << endl << "Setting Readout Flags" << endl;
#endif
	slsDetectorDefs::readOutFlags val = slsDetectorDefs::GET_READOUT_FLAGS;
	if(widgetEiger->isVisible()){

		//set to continous or storeinram
		switch(comboEigerFlags1->currentIndex()){
		case Storeinram:	val = slsDetectorDefs::STORE_IN_RAM;	break;
		default:			val = slsDetectorDefs::CONTINOUS_RO;	break;
		}
		myDet->setReadOutFlags(val);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::setFlags");

		//set to parallel, nonparallel or safe
		switch(comboEigerFlags2->currentIndex()){
		case Parallel:		val = slsDetectorDefs::PARALLEL;		break;
		case Safe:			val = slsDetectorDefs::SAFE;			break;
		default:			val = slsDetectorDefs::NONPARALLEL;		break;
		}
		myDet->setReadOutFlags(val);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::setFlags");

		//update flags
		 UpdateFlagsFromServer();
	}

}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateSpeedFromServer(){
	int ret;
	if(widgetEiger->isVisible()){
		disconnect(comboEigerClkDivider,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetSpeed()));

		//get speed
		ret = myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, -1);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::updateSpeedFromServer");

		//valid speed
		if(ret  >= 0 && ret < NumberofSpeeds)
			comboEigerClkDivider->setCurrentIndex(ret);

		//invalid speed
		else{
			qDefs::Message(qDefs::WARNING,"Inconsistent value from clock divider.\n"
					"Setting it for all detectors involved to half speed.","qTabDataOutput::updateSpeedFromServer");
			//set to default
			comboEigerClkDivider->setCurrentIndex(HalfSpeed);
			 myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,HalfSpeed);
			 qDefs::checkErrorMessage(myDet,"qTabDataOutput::updateSpeedFromServer");

		}
		connect(comboEigerClkDivider,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetSpeed()));
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateFlagsFromServer(){
	int ret;
	if(widgetEiger->isVisible()){
		disconnect(comboEigerFlags1,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));
		disconnect(comboEigerFlags2,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));

		//get speed
		ret = myDet->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS);
		qDefs::checkErrorMessage(myDet,"qTabDataOutput::updateFlagsFromServer");

		//invalid flags
		if(ret==-1){
			qDefs::Message(qDefs::WARNING,"Inconsistent value for readout flags.\n"
								"Setting it for all detectors involved to continous nonparallel mode.",
								"qTabDataOutput::updateFlagsFromServer");
			//set to default
			comboEigerFlags1->setCurrentIndex(Continous);
			 myDet->setReadOutFlags(slsDetectorDefs::CONTINOUS_RO);
			 qDefs::checkErrorMessage(myDet,"qTabDataOutput::updateFlagsFromServer");
			comboEigerFlags2->setCurrentIndex(NonParallel);
			 myDet->setReadOutFlags(slsDetectorDefs::NONPARALLEL);
			 qDefs::checkErrorMessage(myDet,"qTabDataOutput::updateFlagsFromServer");
		}

		//valid flags
		else{
			if(ret & slsDetectorDefs::STORE_IN_RAM)
				comboEigerFlags1->setCurrentIndex(Storeinram);
			else if(ret & slsDetectorDefs::CONTINOUS_RO)
				comboEigerFlags1->setCurrentIndex(Continous);
			if(ret & slsDetectorDefs::PARALLEL)
				comboEigerFlags2->setCurrentIndex(Parallel);
			else if(ret & slsDetectorDefs::NONPARALLEL)
				comboEigerFlags2->setCurrentIndex(NonParallel);
			else if(ret & slsDetectorDefs::SAFE)
				comboEigerFlags2->setCurrentIndex(Safe);
		}

		connect(comboEigerFlags1,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));
		connect(comboEigerFlags2,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFlags()));
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetupFileFormat(){


	//To be able to index items on a combo box
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(comboFileFormat->model());
	QModelIndex index[slsReceiverDefs::NUM_FILE_FORMATS];
	QStandardItem* item[slsReceiverDefs::NUM_FILE_FORMATS];
	if (model) {
		for(int i=0;i<slsReceiverDefs::NUM_FILE_FORMATS;i++){
			index[i] = model->index(i,	comboFileFormat->modelColumn(), comboFileFormat->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}
		//Enabling/Disabling depending on the detector type
		switch(detType){
		case slsDetectorDefs::MYTHEN:
			item[(int)slsReceiverDefs::BINARY]->setEnabled(false);
			item[(int)slsReceiverDefs::ASCII]->setEnabled(true);
			item[(int)slsReceiverDefs::HDF5]->setEnabled(false);
			break;
		case slsDetectorDefs::EIGER:
		case slsDetectorDefs::MOENCH:
		case slsDetectorDefs::PROPIX:
		case slsDetectorDefs::GOTTHARD:
		case slsDetectorDefs::JUNGFRAU:
			item[(int)slsReceiverDefs::BINARY]->setEnabled(true);
			item[(int)slsReceiverDefs::ASCII]->setEnabled(false);
			item[(int)slsReceiverDefs::HDF5]->setEnabled(true);
			break;
		default:
			cout << "Unknown detector type " << endl;
			qDefs::Message(qDefs::CRITICAL,"Unknown detector type.","qTabDataOutput::SetupFileFormat");
			exit(-1);
			break;
		}
	}

	comboFileFormat->setCurrentIndex((int)myDet->getFileFormat());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateFileFormatFromServer(){
#ifdef VERBOSE
	cout  << endl << "Getting File Format" << endl;
#endif
	disconnect(comboFileFormat,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFileFormat(int)));

	comboFileFormat->setCurrentIndex((int)myDet->getFileFormat());

	connect(comboFileFormat,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFileFormat(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetFileFormat(int format){
#ifdef VERBOSE
	cout  << endl << "Setting File Format" << endl;
#endif
	disconnect(comboFileFormat,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFileFormat(int)));

	int ret = (int)myDet->setFileFormat((slsReceiverDefs::fileFormat)comboFileFormat->currentIndex());
	if (ret != comboFileFormat->currentIndex()) {
		qDefs::Message(qDefs::WARNING,"Could not set file format.","qTabDataOutput::SetFileFormat");
		comboFileFormat->setCurrentIndex((int)ret);
	}

	connect(comboFileFormat,	SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(SetFileFormat(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::UpdateFileOverwriteFromServer(){
#ifdef VERBOSE
	cout  << endl << "Getting File Over Write Enable" << endl;
#endif
	disconnect(chkOverwriteEnable,	SIGNAL(toggled(bool)), 		this, 	SLOT(SetOverwriteEnable(bool)));

	chkOverwriteEnable->setChecked(myDet->overwriteFile());

	connect(chkOverwriteEnable,	SIGNAL(toggled(bool)), 		this, 	SLOT(SetOverwriteEnable(bool)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetOverwriteEnable(bool enable){
#ifdef VERBOSE
	cout  << endl << "Setting File Over Write Enable" << endl;
#endif
	disconnect(chkOverwriteEnable,	SIGNAL(toggled(bool)), 		this, 	SLOT(SetOverwriteEnable(bool)));

	int valid = (enable?1:0);
	if (myDet->overwriteFile(enable) != valid)
		qDefs::Message(qDefs::WARNING,"Could not over write enable.","qTabDataOutput::SetOverwriteEnable");

	connect(chkOverwriteEnable,	SIGNAL(toggled(bool)), 		this, 	SLOT(SetOverwriteEnable(bool)));

	UpdateFileOverwriteFromServer();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating DataOutput Tab" << endl;
#endif

	if (!myDet->enableWriteToFile())
		boxFileWriteEnabled->setEnabled(false);
	else
		boxFileWriteEnabled->setEnabled(true);

	// output dir
#ifdef VERBOSE
	cout  << "Getting output directory" << endl;
#endif

	disconnect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));
	PopulateDetectors();
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)), 	this, 	SLOT(GetOutputDir()));

	//file format
	UpdateFileFormatFromServer();

	//overwrite
	UpdateFileOverwriteFromServer();

	//file name
	dispFileName->setText(QString(myDet->getFileName().c_str()));

	//flat field correction from server
#ifdef VERBOSE
	cout  << "Getting flat field" << endl;
#endif
	UpdateFlatFieldFromServer();


	//rate correction - not for charge integrating detectors
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::EIGER)){
#ifdef VERBOSE
		cout  << "Getting rate correction" << endl;
#endif
		UpdateRateCorrectionFromServer();
	}


	//update angular conversion from server
	if((detType == slsDetectorDefs::MYTHEN)||(detType == slsDetectorDefs::GOTTHARD)){
#ifdef VERBOSE
		cout  << "Getting angular conversion" << endl;
#endif
		int ang;
		if(myDet->getAngularConversion(ang))
			chkAngular->setChecked(true);
		emit AngularConversionSignal(chkAngular->isChecked());
	}


	//discard bad channels from server
#ifdef VERBOSE
	cout  << "Getting bad channel correction" << endl;//cout << "ff " << myDet->getBadChannelCorrection() << endl;
#endif


	disconnect(chkDiscardBad,		SIGNAL(toggled(bool)));
	if(myDet->getBadChannelCorrection())
		chkDiscardBad->setChecked(true);
	else
		chkDiscardBad->setChecked(false);
	connect(chkDiscardBad,		SIGNAL(toggled(bool)), 	this, 	SLOT(DiscardBadChannels()));

	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		btnOutputBrowse->setEnabled(false);
		btnOutputBrowse->setToolTip("<font color=\"red\">This button is disabled as receiver PC is different from "
				"client PC and hence different directory structures.</font><br><br>" + dispOutputDir->toolTip());
	}else{
		btnOutputBrowse->setEnabled(true);
		btnOutputBrowse->setToolTip(dispOutputDir->toolTip());
	}

	//getting compression
	if(chkCompression->isEnabled()){
#ifdef VERBOSE
		cout  << "Getting compression" << endl;
#endif
		GetCompression();
	}

	//getting 10GbE
	if(chkTenGiga->isEnabled()){
#ifdef VERBOSE
		cout  << "Getting 10GbE enable" << endl;
#endif
		EnableTenGigabitEthernet(-1,1);
	}

	//Eiger specific
	if(widgetEiger->isVisible()){
		//speed
#ifdef VERBOSE
		cout  << "Getting Speed" << endl;
#endif
		UpdateSpeedFromServer();
		//flags
#ifdef VERBOSE
		cout  << "Getting Readout Flags" << endl;
#endif
		UpdateFlagsFromServer();



	}


#ifdef VERBOSE
	cout  << "**Updated DataOutput Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(myDet,"qTabDataOutput::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



/*
 * qTabDebugging.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDebugging.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt  Include Headers
#include <QDesktopWidget>
#include <QGridLayout>
// C++ Include Headers
#include<iostream>
using namespace std;



//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDebugging::qTabDebugging(QWidget *parent,multiSlsDetector*& detector):
				  QWidget(parent),
				  myDet(detector),
				  det(0),
				  treeDet(0),
				  dispFrame(0),
				  lblDetectorId(0),
				  lblDetectorSerial(0),
				  lblDetectorFirmware(0),
				  lblDetectorSoftware(0),
				  lblModuleId(0),
				  lblModuleFirmware(0),
				  lblModuleSerial(0){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDebugging::~qTabDebugging(){
	delete myDet;
	if(det) delete det;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::SetupWidgetWindow(){

	blue = new QPalette();
	blue->setColor(QPalette::Active,QPalette::WindowText,Qt::darkBlue);


	// Detector Type
	detType=myDet->getDetectorsType();
	///change module label
	switch(detType){
	case slsDetectorDefs::EIGER:
		lblDetector->setText("Half Module:");
		chkDetectorFirmware->setText("Half Module Firmware:");
		chkDetectorSoftware->setText("Half Module Software:");
		chkDetectorMemory->setText("Half Module Memory:");
		chkDetectorBus->setText("Half Module Bus:");
		lblModule->hide();
		comboModule->hide();
		chkModuleFirmware->hide();
		chkChip->setEnabled(false);
		chkModuleFirmware->setEnabled(false);
		break;
	case slsDetectorDefs::PROPIX:
	case slsDetectorDefs::GOTTHARD:
		lblDetector->setText("Module:");
		chkDetectorFirmware->setText("Module Firmware:");
		chkDetectorSoftware->setText("Module Software:");
		chkDetectorMemory->setText("Module Memory:");
		chkDetectorBus->setText("Module Bus:");
		lblModule->hide();
		comboModule->hide();
		chkModuleFirmware->hide();
		chkChip->setEnabled(false);
		chkModuleFirmware->setEnabled(false);
		break;
	case slsDetectorDefs::MOENCH:
		lblDetector->setText("Module:");
		chkDetectorFirmware->setText("Module Firmware:");
		chkDetectorSoftware->setText("Module Software:");
		chkDetectorMemory->setText("Module Memory:");
		chkDetectorBus->setText("Module Bus:");
		lblModule->hide();
		comboModule->hide();
		chkModuleFirmware->hide();
		chkChip->setEnabled(false);
		chkModuleFirmware->setEnabled(false);
		break;
	case slsDetectorDefs::MYTHEN:
		break;
	default:
		//leave everything as it is(mythen is default)
		break;
	}


	//add detectors
	for(int i=0;i<myDet->getNumberOfDetectors();i++){
		comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
	}


	//add modules  and status for current detector
	if(detType==slsDetectorDefs::MYTHEN) UpdateModuleList();
	UpdateStatus();

	qDefs::checkErrorMessage(myDet,"qTabDebugging::SetupWidgetWindow");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::Initialization(){
	if(detType==slsDetectorDefs::MYTHEN)
		connect(comboDetector,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(UpdateModuleList()));

	connect(comboDetector,	SIGNAL(currentIndexChanged(int)),	this,	SLOT(UpdateStatus()));
	connect(btnGetInfo,		SIGNAL(clicked()),					this,	SLOT(GetInfo()));
	connect(btnTest,		SIGNAL(clicked()),					this,	SLOT(TestDetector()));
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::UpdateModuleList(){
#ifdef VERBOSE
		cout  << "Getting Module List" << endl;
#endif
	det = myDet->getSlsDetector(comboDetector->currentIndex());
	qDefs::checkErrorMessage(myDet,"qTabDebugging::UpdateModuleList");
	//deletes all modules except "all modules"
	for(int i=0;i<comboModule->count()-1;i++)
		comboModule->removeItem(i);
	for(int i=0;i<det->getNMods();i++){
		comboModule->addItem(QString("Module %1").arg(i));
	}

	qDefs::checkErrorMessage(det,"qTabDebugging::UpdateModuleList");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::UpdateStatus(){
#ifdef VERBOSE
		cout  << "Getting Status" << endl;
#endif
	det = myDet->getSlsDetector(comboDetector->currentIndex());
	qDefs::checkErrorMessage(myDet,"qTabDebugging::UpdateStatus");
	int detStatus = (int)det->getRunStatus();
	string status = slsDetectorBase::runStatusType(slsDetectorDefs::runStatus(detStatus));
	lblStatus->setText(QString(status.c_str()).toUpper());

	qDefs::checkErrorMessage(det,"qTabDebugging::UpdateStatus");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::GetInfo(){
#ifdef VERBOSE
		cout  << "Getting Info" << endl;
#endif
	//window
	QFrame* popup1 = new QFrame(this, Qt::Popup | Qt::SubWindow );
	QList<QTreeWidgetItem *> items;



	//layout
	QGridLayout *layout 	= new QGridLayout(popup1);
	//treewidget
	treeDet 				= new QTreeWidget(popup1);
	layout->addWidget(treeDet,0,0);
	//display the details
	dispFrame 				= new QFrame(popup1);
	QGridLayout *formLayout = new QGridLayout(dispFrame);
	lblDetectorId			= new QLabel("");	lblDetectorId->setPalette(*blue);
	lblDetectorSerial		= new QLabel("");	lblDetectorSerial->setPalette(*blue);
	lblDetectorFirmware		= new QLabel("");	lblDetectorFirmware->setPalette(*blue);
	lblDetectorSoftware		= new QLabel("");	lblDetectorSoftware->setPalette(*blue);
	lblModuleId				= new QLabel("");	lblModuleId->setPalette(*blue);
	lblModuleSerial			= new QLabel("");	lblModuleSerial->setPalette(*blue);
	lblModuleFirmware		= new QLabel("");	lblModuleFirmware->setPalette(*blue);
	//to make sure the size is constant
	lblDetectorFirmware->setFixedWidth(100);
	layout->addWidget(dispFrame,0,1);

	switch(detType){

	case slsDetectorDefs::MYTHEN:
		//display widget
		formLayout->addWidget(new QLabel("Readout:"),0,0);
		formLayout->addItem(new QSpacerItem(15,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		formLayout->addWidget(lblDetectorId,0,2);
		formLayout->addWidget(new QLabel("Readout MAC Address:"),1,0);
		formLayout->addWidget(lblDetectorSerial,1,2);
		formLayout->addWidget(new QLabel("Readout Firmware Version:"),2,0);
		formLayout->addWidget(lblDetectorFirmware,2,2);
		formLayout->addWidget(new QLabel("Readout Software Version:"),3,0);
		formLayout->addWidget(lblDetectorSoftware,3,2);
		formLayout->addWidget(new QLabel("Module:"),4,0);
		formLayout->addWidget(lblModuleId,4,2);
		formLayout->addWidget(new QLabel("Module Serial Number:"),5,0);
		formLayout->addWidget(lblModuleSerial,5,2);
		formLayout->addWidget(new QLabel("Module Firmware Version:"),6,0);
		formLayout->addWidget(lblModuleFirmware,6,2);


		//tree widget
		treeDet->setHeaderLabel("Mythen Detector");
		//gets det names
		for (int i=0;i<comboDetector->count();i++)
			items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Readout (%1)").arg(comboDetector->itemText(i)))));
		treeDet->insertTopLevelItems(0, items);
		//gets module names
		for (int i=0;i<comboDetector->count();i++){
			QList<QTreeWidgetItem *> childItems;
			det = myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet,"qTabDebugging::GetInfo");
			for(int j=0;j<det->getNMods();j++)
				childItems.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Module %1").arg(j))));
			treeDet->topLevelItem(i)->insertChildren(0,childItems);
			qDefs::checkErrorMessage(det,"qTabDebugging::GetInfo");
		}

		break;




	case slsDetectorDefs::EIGER:
		//display widget
		formLayout->addWidget(new QLabel("Half Module:"),0,0);
		formLayout->addItem(new QSpacerItem(15,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		formLayout->addWidget(lblDetectorId,0,2);
		formLayout->addWidget(new QLabel("Half Module MAC Address:"),1,0);
		formLayout->addWidget(lblDetectorSerial,1,2);
		formLayout->addWidget(new QLabel("Half Module Firmware Version:"),2,0);
		formLayout->addWidget(lblDetectorFirmware,2,2);
		formLayout->addWidget(new QLabel("Half Module Software Version:"),3,0);
		formLayout->addWidget(lblDetectorSoftware,3,2);

		//tree widget
		treeDet->setHeaderLabel("Eiger Detector");
		//get num modules
		for (int i=0;i<comboDetector->count()/2;i++)
			items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Module %1").arg(i))));
		treeDet->insertTopLevelItems(0, items);
		//gets det names
		for (int i=0;i<comboDetector->count();i++){
			QList<QTreeWidgetItem *> childItems;
			childItems.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Half Module (%1)").arg(comboDetector->itemText(i)))));
			treeDet->topLevelItem(i*2)->insertChildren(0,childItems);
		}
		break;



	case slsDetectorDefs::MOENCH:

		//display widget
		formLayout->addWidget(new QLabel("Module:"),0,0);
		formLayout->addItem(new QSpacerItem(15,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		formLayout->addWidget(lblDetectorId,0,2);
		formLayout->addWidget(new QLabel("Module MAC Address:"),1,0);
		formLayout->addWidget(lblDetectorSerial,1,2);
		formLayout->addWidget(new QLabel("Module Firmware Version:"),2,0);
		formLayout->addWidget(lblDetectorFirmware,2,2);
		formLayout->addWidget(new QLabel("Module Software Version:"),3,0);
		formLayout->addWidget(lblDetectorSoftware,3,2);
		//tree widget
		treeDet->setHeaderLabel("Moench Detector");
		//gets det names
		for (int i=0;i<comboDetector->count();i++)
			items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Module (%1)").arg(comboDetector->itemText(i)))));
		treeDet->insertTopLevelItems(0, items);

		break;



	case slsDetectorDefs::PROPIX:

		//display widget
		formLayout->addWidget(new QLabel("Module:"),0,0);
		formLayout->addItem(new QSpacerItem(15,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		formLayout->addWidget(lblDetectorId,0,2);
		formLayout->addWidget(new QLabel("Module MAC Address:"),1,0);
		formLayout->addWidget(lblDetectorSerial,1,2);
		formLayout->addWidget(new QLabel("Module Firmware Version:"),2,0);
		formLayout->addWidget(lblDetectorFirmware,2,2);
		formLayout->addWidget(new QLabel("Module Software Version:"),3,0);
		formLayout->addWidget(lblDetectorSoftware,3,2);
		//tree widget
		treeDet->setHeaderLabel("Propix Detector");
		//gets det names
		for (int i=0;i<comboDetector->count();i++)
			items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Module (%1)").arg(comboDetector->itemText(i)))));
		treeDet->insertTopLevelItems(0, items);

		break;



	case slsDetectorDefs::GOTTHARD:

		//display widget
		formLayout->addWidget(new QLabel("Module:"),0,0);
		formLayout->addItem(new QSpacerItem(15,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		formLayout->addWidget(lblDetectorId,0,2);
		formLayout->addWidget(new QLabel("Module MAC Address:"),1,0);
		formLayout->addWidget(lblDetectorSerial,1,2);
		formLayout->addWidget(new QLabel("Module Firmware Version:"),2,0);
		formLayout->addWidget(lblDetectorFirmware,2,2);
		formLayout->addWidget(new QLabel("Module Software Version:"),3,0);
		formLayout->addWidget(lblDetectorSoftware,3,2);
		//tree widget
		treeDet->setHeaderLabel("Gotthard Detector");
		//gets det names
		for (int i=0;i<comboDetector->count();i++)
			items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("Module (%1)").arg(comboDetector->itemText(i)))));
		treeDet->insertTopLevelItems(0, items);

		break;




	default:
		break;
	}

	//show and center widget

	int x = ((parentWidget()->width()) - (popup1->frameGeometry().width())) / 2;
	int y = ((parentWidget()->height()) - (popup1->frameGeometry().height())) / 2;
	QDesktopWidget *desktop = QApplication::desktop();
	int screen = desktop->screenNumber(this);
	popup1->setWindowModality(Qt::WindowModal);
	popup1->move( (desktop->screenGeometry(screen).x())+x, (desktop->screenGeometry(screen).y())+y );
	popup1->show();

	//put the first parameters
	SetParameters(treeDet->topLevelItem(0));

	//initializations
	connect(treeDet, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)) , this, SLOT(SetParameters(QTreeWidgetItem *)));

}
//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDebugging::SetParameters(QTreeWidgetItem *item){
	char value[200];
	int i;


	switch(detType){

	case slsDetectorDefs::MYTHEN:
		if(item->text(0).contains("Readout")){
			//find index
			for(i=0;i<comboDetector->count();i++)
				if(item== treeDet->topLevelItem(i))
					break;

			det = myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet,"qTabDebugging::SetParameters");
			lblDetectorId->setText(comboDetector->itemText(i));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER));
			lblDetectorSerial->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION));
			lblDetectorFirmware	->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION));
			lblDetectorSoftware->setText(QString(value));
			qDefs::checkErrorMessage(det,"qTabDebugging::SetParameters");

			lblModuleId->setText("");
			lblModuleSerial->setText("");
			lblModuleFirmware->setText("");
		}else{
			//find index
			for(i=0;i<comboDetector->count();i++)
				if(item->parent() == treeDet->topLevelItem(i))
					break;
			int im = item->parent()->indexOfChild(item);

			det = myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet,"qTabDebugging::SetParameters");
			lblDetectorId->setText(comboDetector->itemText(i));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER));
			lblDetectorSerial->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION));
			lblDetectorFirmware	->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION));
			lblDetectorSoftware->setText(QString(value));

			lblModuleId->setText(QString("%1").arg(im));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::MODULE_SERIAL_NUMBER,im));
			lblModuleSerial->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::MODULE_FIRMWARE_VERSION,im));
			lblModuleFirmware->setText(QString(value));

			qDefs::checkErrorMessage(det,"qTabDebugging::SetParameters");
		}
		break;





	case slsDetectorDefs::EIGER:
		//only if half module clicked
		if(item->text(0).contains("Half Module")){
			//find index
			for(i=0;i<comboDetector->count();i++)
				if(item== treeDet->topLevelItem(i))
					break;

			det = myDet->getSlsDetector(i);
			qDefs::checkErrorMessage(myDet,"qTabDebugging::SetParameters");
			lblDetectorId->setText(comboDetector->itemText(i));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER));
			lblDetectorSerial->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION));
			lblDetectorFirmware	->setText(QString(value));
			sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION));
			lblDetectorSoftware->setText(QString(value));

			qDefs::checkErrorMessage(det,"qTabDebugging::SetParameters");
		}
		break;



	case slsDetectorDefs::PROPIX:
	case slsDetectorDefs::MOENCH:
	case slsDetectorDefs::GOTTHARD:
		//find index
		for(i=0;i<comboDetector->count();i++)
			if(item== treeDet->topLevelItem(i))
				break;

		det = myDet->getSlsDetector(i);
		qDefs::checkErrorMessage(myDet,"qTabDebugging::SetParameters");
		lblDetectorId->setText(comboDetector->itemText(i));
		sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SERIAL_NUMBER));
		lblDetectorSerial->setText(QString(value));
		sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION));
		lblDetectorFirmware	->setText(QString(value));
		sprintf(value,"%llx",det->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION));
		lblDetectorSoftware->setText(QString(value));

		qDefs::checkErrorMessage(det,"qTabDebugging::SetParameters");
		break;




	default:
		break;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::TestDetector(){
#ifdef VERBOSE
	cout << "Testing" << endl;
#endif

	int retval=slsDetectorDefs::FAIL;
	QString message;
	QString Detector = "Detector";

	//main messagebox title
	switch(detType){
	case slsDetectorDefs::MYTHEN:
		message = QString("<nobr>Test Results for %1 and %2:</nobr><br><br>").arg(comboDetector->currentText(),comboModule->currentText());
		break;
	case slsDetectorDefs::EIGER:	 Detector =  "Half Module";	break;
	case slsDetectorDefs::MOENCH:
	case slsDetectorDefs::PROPIX:
	case slsDetectorDefs::GOTTHARD:	 Detector =  "Module";	break;
	default: break;
	}


	if(detType==slsDetectorDefs::MYTHEN)
		message = QString("<nobr>Test Results for %1 and %2:</nobr><br><br>").arg(comboDetector->currentText(),comboModule->currentText());
	else message = QString("<nobr>Test Results for %1:</nobr><br><br>").arg(comboDetector->currentText());

	//get sls det object
	det = myDet->getSlsDetector(comboDetector->currentIndex());
	qDefs::checkErrorMessage(myDet,"qTabDebugging::TestDetector");

	//detector firmware
	if(chkDetectorFirmware->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::DETECTOR_FIRMWARE_TEST);
		if(retval== slsDetectorDefs::FAIL)	message.append(QString("<nobr>%1 Firmware: FAIL</nobr><br>").arg(Detector));
		else								message.append(QString("<nobr>%1 Firmware: %2</nobr><br>").arg(Detector,QString::number(retval)));
#ifdef VERBOSE
		cout<<"Detector Firmware: "<<retval<<endl;
#endif
	}
	//detector software
	if(chkDetectorSoftware->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::DETECTOR_SOFTWARE_TEST);
		if(retval== slsDetectorDefs::FAIL)	message.append(QString("<nobr>%1 Software: FAIL</nobr><br>").arg(Detector));
		else								message.append(QString("<nobr>%1 Software: %2</nobr><br>").arg(Detector,QString::number(retval)));
#ifdef VERBOSE
		cout<<"Detector Software: "<<retval<<endl;
#endif
	}
	//detector CPU-FPGA bus
	if(chkDetectorBus->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::DETECTOR_BUS_TEST);
		if(retval== slsDetectorDefs::FAIL)	message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;FAIL</nobr><br>").arg(Detector));
		else								message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;%2</nobr><br>").arg(Detector,QString::number(retval)));
#ifdef VERBOSE
		cout<<"Detector Bus: "<<retval<<endl;
#endif
	}
	//detector Memory
	if(chkDetectorMemory->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::DETECTOR_MEMORY_TEST);
		if(retval== slsDetectorDefs::FAIL)	message.append(QString("<nobr>%1 Memory: &nbsp;FAIL</nobr><br>").arg(Detector));
		else								message.append(QString("<nobr>%1 Memory: &nbsp;%2</nobr><br>").arg(Detector,QString::number(retval)));
#ifdef VERBOSE
		cout<<"Detector Memory: "<<retval<<endl;
#endif
	}
	//chip
	if(chkChip->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::CHIP_TEST,comboModule->currentIndex());
		if(retval== slsDetectorDefs::FAIL)	message.append("<br><nobr>Chip: FAIL</nobr><br>");
		else								message.append(QString("<nobr>Chip: %1</nobr><br>").arg(retval));
#ifdef VERBOSE
		cout<<"Chip: "<<retval<<endl;
#endif
	}
	//module firmware
	if(chkModuleFirmware->isChecked()){
		retval = det->digitalTest(slsDetectorDefs::MODULE_FIRMWARE_TEST,comboModule->currentIndex());
		if(retval== slsDetectorDefs::FAIL)	message.append("<nobr>Module Firmware: FAIL</nobr><br>");
		else								message.append(QString("<nobr>Module Firmware: %1</nobr><br>").arg(retval));
#ifdef VERBOSE
		cout<<"Module Firmware: "<<retval<<endl;
#endif
	}
	//display message
	qDefs::Message(qDefs::INFORMATION,message.toAscii().constData(),"qTabDebugging::TestDetector");

	qDefs::checkErrorMessage(det,"qTabDebugging::TestDetector");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDebugging::Refresh(){
#ifdef VERBOSE
		cout  << endl << "**Updating Debugging Tab" << endl;
#endif
	UpdateStatus();
#ifdef VERBOSE
		cout  << "**Updated Debugging Tab" << endl << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


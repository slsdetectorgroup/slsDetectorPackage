/*
 * qScanWidget.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Project Class Headers
#include "qScanWidget.h"
#include "qDefs.h"
// Qt Include Headers
#include <QFileDialog>
#include <QStackedLayout>

// C++ Include Headers
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------
int qScanWidget::NUM_SCAN_WIDGETS(0);
//-------------------------------------------------------------------------------------------------------------------------------------------------

qScanWidget::qScanWidget(QWidget *parent,multiSlsDetector*& detector):
		QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qScanWidget::~qScanWidget(){
	delete myDet;
	delete stackedLayout;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetupWidgetWindow(){
	id = NUM_SCAN_WIDGETS;
	NUM_SCAN_WIDGETS++;

	setFixedHeight(125);

	//layout for the size widgets
	stackedLayout = new QStackedLayout;
	stackedLayout->setSpacing(0);

	//  Constant Size Layout
	QWidget *constantWidget = new QWidget;
	QHBoxLayout *layoutConstant = new QHBoxLayout(constantWidget);
	layoutConstant->setContentsMargins(0, 0, 0, 0);
	layoutConstant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	lblFrom	= new QLabel("from",constantWidget);
	lblFrom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layoutConstant->addWidget(lblFrom);
	spinFrom = new QSpinBox(constantWidget);
	layoutConstant->addWidget(spinFrom);
	lblTo = new QLabel("to",constantWidget);
	lblTo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layoutConstant->addWidget(lblTo);
	spinTo			= new QSpinBox(constantWidget);
	layoutConstant->addWidget(spinTo);
	layoutConstant->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	lblSize			= new QLabel("Size",constantWidget);
	lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layoutConstant->addWidget(lblSize);
	spinSize		= new QSpinBox(constantWidget);
	layoutConstant->addWidget(spinSize);
	layoutConstant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

	//  Specific Values Layout
	QWidget *specificWidget = new QWidget;
	QHBoxLayout *layoutSpecific = new QHBoxLayout(specificWidget);
	layoutSpecific->setContentsMargins(0, 0, 0, 0);
	layoutSpecific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	comboSpecific = new QComboBox(specificWidget);
	comboSpecific->setEditable(true);
	comboSpecific->setCompleter(false);
	layoutSpecific->addWidget(comboSpecific);
	layoutSpecific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

	//  Values From a File Layout
	QWidget *valuesWidget = new QWidget;
	QHBoxLayout *layoutValues = new QHBoxLayout(valuesWidget);
	layoutValues->setContentsMargins(0, 0, 0, 0);
	layoutValues->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	dispValues = new QLineEdit("steps.txt",valuesWidget);
	dispValues->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
	layoutValues->addWidget(dispValues);
	btnValues = new QPushButton("Browse",valuesWidget);
	btnValues->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layoutValues->addWidget(btnValues);
	layoutValues->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));


	stackedLayout->addWidget(constantWidget);
	stackedLayout->addWidget(specificWidget);
	stackedLayout->addWidget(valuesWidget);
	stackedWidget->setLayout(stackedLayout);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::Initialization(){
	//mode
	connect(comboScript,	SIGNAL(currentIndexChanged(int)),		this,SLOT(SetScript(int)));
	//file
	connect(dispScript,		SIGNAL(editingFinished()),				this, SLOT(SetScriptFile()));
	connect(btnBrowse,		SIGNAL(clicked()), 						this, SLOT(BrowsePath()));
	//parameter
	connect(dispParameter,	SIGNAL(textChanged(const QString&)), 	this, SLOT(SetParameter(const QString&)));
	//sizewidgets
	connect(radioConstant,	SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
	connect(radioSpecific,	SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
	connect(radioValue,		SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::EnableSizeWidgets(){
	if(radioConstant->isChecked())
		stackedLayout->setCurrentIndex(0);
	else if(radioSpecific->isChecked())
		stackedLayout->setCurrentIndex(1);
	else
		stackedLayout->setCurrentIndex(2);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetScript(int index){
#ifdef VERBOSE
	cout << "Setting mode of scan widget:" << id << " to " << index << endl;
#endif
	// defaults
	dispScript->setEnabled(false);
	btnBrowse->setEnabled(false);
	lblParameter->setEnabled(false);
	dispParameter->setEnabled(false);
	group->setEnabled(false);
	lblSteps->setEnabled(false);
	spinSteps->setEnabled(false);
	lblPrecision->setEnabled(false);
	spinPrecision->setEnabled(false);

	// If anything other than None is selected
	if(index){
		// Custom Script only enables the first layout with addnl parameters etc
		if(index==4){
			dispScript->setEnabled(true);
			btnBrowse->setEnabled(true);
			lblParameter->setEnabled(true);
			dispParameter->setEnabled(true);
		}
		group->setEnabled(true);
		lblPrecision->setEnabled(true);
		spinPrecision->setEnabled(true);
		// Steps are enabled only if constant step size is not checked
		lblSteps->setEnabled(!radioConstant->isChecked());
		spinSteps->setEnabled(!radioConstant->isChecked());

	}
	//emit signal to enable scanbox in plot tab
	emit EnableScanBox(index,id);


	QString fName = dispScript->text();
	//set the mode
	if(index)	myDet->setActionScript(id,fName.toAscii().constData());
	else myDet->setActionScript(id,"");
	//mode is not set when fname is blank
	if(!fName.isEmpty()){
		//check if mode didnt get set
		if(index!=myDet->getActionMode(id)){
			qDefs::WarningMessage("The mode could not be changed.","ScanWidget");
			comboScript->setCurrentIndex(myDet->getActionMode(id));
		}//if mode got set and its custom script
		else if(index){
			//when the file name did not get set correctly
			if(fName.compare(QString(myDet->getActionScript(id).c_str()))){
				qDefs::WarningMessage("The file path could not be set.","ScanWidget");
				dispScript->setText(QString(myDet->getActionScript(id).c_str()));
				SetScriptFile();
			}
		}
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qScanWidget::BrowsePath(){
#ifdef VERBOSE
	cout << "Browsing Script File Path" << endl;
#endif
	QString fName = dispScript->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir = "/home";
	//dialog
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Script File"),dir,
			tr("Script Files(*.awk);;All Files(*)"));
	//if empty, set the file name and it calls setscriptfile, else ignore
	if (!fName.isEmpty()){
		dispScript->setText(fName);
		SetScriptFile();
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qScanWidget::SetScriptFile(){
	QString fName = dispScript->text();bool set = false;
#ifdef VERBOSE
	cout << "Setting script file of scan widget:" << id << " to " << fName.toAscii().constData() << endl;
#endif
	disconnect(dispScript,	SIGNAL(editingFinished()),	this, 	SLOT(SetScriptFile()));

	//blank
	if(fName.isEmpty())		set = true;
	else if(!fName.compare("none"))		set = true;
	//not blank
	else{
		QString file = dispScript->text().section('/',-1);
		//is a file
		if(file.contains('.')){
			//check if it exists and set the script file
			if(QFile::exists(fName))
				set = true;
			//if the file doesnt exist, set it to what it was before
			else{
				qDefs::WarningMessage("The script file entered does not exist","ScanWidget");
				dispScript->setText(QString(myDet->getActionScript(id).c_str()));
			}
		}//not a file, set it to what it was before
		else {
			qDefs::WarningMessage("The script file path entered is not a file","ScanWidget");
			dispScript->setText(QString(myDet->getActionScript(id).c_str()));
		}
	}

	//if blank or valid file
	if(set){
		myDet->setActionScript(id,fName.toAscii().constData());
		if(fName.compare(QString(myDet->getActionScript(id).c_str()))){
			//did not get set, write what is was before
			if(!fName.isEmpty())
				qDefs::WarningMessage("The script file could not be set. Reverting to previous file.","ScanWidget");
			dispScript->setText(QString(myDet->getActionScript(id).c_str()));
		}

	}

	//dont display if theres a none
	if(!dispScript->text().compare("none")) dispScript->setText("");

	connect(dispScript,	SIGNAL(editingFinished()),	this, 	SLOT(SetScriptFile()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetParameter(const QString& parameter){
#ifdef VERBOSE
	cout << "Setting parameter of scan widget:" << id << " to " << parameter.toAscii().constData() << endl;
#endif

	myDet->setActionParameter(id,parameter.toAscii().constData());

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::Refresh(){
	int mode = (myDet->getScanMode(id)>0?1:0);
	string script = myDet->getScanScript(id);
	string parameter = myDet->getScanParameter(id);
	double precision = myDet->getScanPrecision(id);

	//defaults
	if(script == "none") script="";
	if(parameter == "none") parameter="";
	//settings values
	dispScript->setText(QString(script.c_str()));
	dispParameter->setText(QString(parameter.c_str()));
	spinPrecision->setValue(precision);
	//set mode which also checks everything
	comboScript->setCurrentIndex(mode);


#ifdef VERBOSE
	cout << "Updated scan widget " << id << "\t"
			"mode:"<<mode<<"\t"
			"script:" << script << "\t"
			"parameter:" << parameter << "\t"
			"precision:" << precision << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

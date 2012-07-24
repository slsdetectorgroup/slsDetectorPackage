/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Project Class Headers
#include "qActionsWidget.h"
#include "qDefs.h"
// Qt Include Headers
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QSpacerItem>
#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QFileDialog>
#include <QCheckBox>
// C++ Include Headers
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


ActionsWidget::ActionsWidget(QWidget *parent,multiSlsDetector*& detector, int scanType, int id):
		QFrame(parent),myDet(detector),scanType(scanType),id(id),expand(false){
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


ActionsWidget::~ActionsWidget(){
	delete myDet;
	delete layout;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::SetupWidgetWindow(){
	// Widget Settings
	//setFrameStyle(QFrame::Box);
	//setFrameShadow(QFrame::Raised);
	// Main Layout Settings
	setFixedHeight(25);
	if(scanType)	setFixedHeight(125);
	layout = new QGridLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0,0,0,0);
	if(scanType)	layout->setVerticalSpacing(5);

	if(id==NumPositions){
		setFixedHeight(75);
		QLabel *lblNumPos = new QLabel("Number of Positions:");
		lblNumPos->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		layout->addWidget(lblNumPos,0,0);
		spinNumPos = new QSpinBox(this);
		layout->addWidget(spinNumPos,0,1);
		layout->addItem(new QSpacerItem(80,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,2);
		QLabel *lblPosList = new QLabel("List of Positions:");
		lblPosList->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		layout->addWidget(lblPosList,0,3);
		comboPos = new QComboBox(this);
		comboPos->setEditable(true);
		comboPos->setCompleter(0);
		layout->addWidget(comboPos,0,4);
		btnDelete = new QPushButton("Delete");
		btnDelete->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		layout->addWidget(btnDelete,0,5);

		QGroupBox *w = new QGroupBox;
		layout->addWidget(w,1,0,1,6);
		QHBoxLayout *l1 = new QHBoxLayout(w);
		l1->setContentsMargins(0,0,0,0);
		l1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		chkInvert = new QCheckBox("Invert Angles");
		l1->addWidget(chkInvert);
		chkSeparate = new QCheckBox("Separate Two Halves");
		l1->addWidget(chkSeparate);
		chkReturn = new QCheckBox("Return to Start Position");
		chkReturn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		l1->addWidget(chkReturn);
		l1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));


	}else{
		// Main Widgets
		comboScript = new QComboBox(this);
		if(!scanType){
			comboScript->addItem("None");
			comboScript->addItem("Custom Script");
		}else{
			comboScript->addItem("None");
			comboScript->addItem("Energy Scan");
			comboScript->addItem("Threshold Scan");
			comboScript->addItem("Trimbits Scan");
			comboScript->addItem("Custom Script");
		}
		layout->addWidget(comboScript,0,0);
		layout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
		dispScript = new QLineEdit("");
		dispScript->setEnabled(false);
		layout->addWidget(dispScript,0,2);
		btnBrowse = new QPushButton("Browse");
		btnBrowse->setEnabled(false);
		layout->addWidget(btnBrowse,0,3);
		layout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,4);
		lblParameter = new QLabel("Additional Parameter:");
		lblParameter->setEnabled(false);
		layout->addWidget(lblParameter,0,5);
		dispParameter = new QLineEdit("");
		dispParameter->setEnabled(false);
		dispParameter->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		layout->addWidget(dispParameter,0,6);



		//	Scan Levels Widgets
		if(scanType){
			lblSteps = new QLabel("Number of Steps:");
			lblSteps->setEnabled(false);
			layout->addWidget(lblSteps,1,2);
			spinSteps = new QSpinBox(this);
			spinSteps->setEnabled(false);
			layout->addWidget(spinSteps,1,3);
			lblPrecision = new QLabel("Precision:");
			lblPrecision->setEnabled(false);
			layout->addWidget(lblPrecision,1,5);
			spinPrecision = new QSpinBox(this);
			spinPrecision->setEnabled(false);
			layout->addWidget(spinPrecision,1,6);
			group = new QGroupBox(this);
			group->setEnabled(false);
			// Fix the size of the groupbox
			group->setFixedSize(513,66);
			layout->addWidget(group,2,2,1,6);


			// Group Box for step size
			//  Radio Buttons Layout
			QWidget *h1Widget = new QWidget(group);
			h1Widget->setGeometry(QRect(10, 5, group->width()-20, 23));
			QHBoxLayout *h1 = new QHBoxLayout(h1Widget);
			h1->setContentsMargins(0, 0, 0, 0);
			radioConstant = new QRadioButton("Constant Step Size",h1Widget);
			radioConstant->setChecked(true);
			h1->addWidget(radioConstant);
			radioSpecific = new QRadioButton("Specific Values",h1Widget);
			h1->addWidget(radioSpecific);
			radioValue 	= new QRadioButton("Values from File",h1Widget);
			radioValue->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h1->addWidget(radioValue);

			//  Constant Size Layout
			QWidget *h2ConstantWidget = new QWidget(group);
			h2ConstantWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
			QHBoxLayout *h2Constant = new QHBoxLayout(h2ConstantWidget);
			h2Constant->setContentsMargins(0, 0, 0, 0);

			h2Constant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
			lblFrom	= new QLabel("from",h2ConstantWidget);
			lblFrom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblFrom);
			spinFrom = new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinFrom);
			lblTo = new QLabel("to",h2ConstantWidget);
			lblTo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblTo);
			spinTo			= new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinTo);
			h2Constant->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
			lblSize			= new QLabel("Size",h2ConstantWidget);
			lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblSize);
			spinSize		= new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinSize);
			h2Constant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

			//  Specific Values Layout
			QWidget *h2SpecificWidget = new QWidget(group);
			h2SpecificWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
			QHBoxLayout *h2Specific = new QHBoxLayout(h2SpecificWidget);
			h2Specific->setContentsMargins(0, 0, 0, 0);
			h2Specific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
			comboSpecific = new QComboBox(h2SpecificWidget);
			h2Specific->addWidget(comboSpecific);
			comboSpecific->hide();
			h2Specific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

			//  Values From a File Layout
			QWidget *h2ValuesWidget = new QWidget(group);
			h2ValuesWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
			QHBoxLayout *h2Values = new QHBoxLayout(h2ValuesWidget);
			h2Values->setContentsMargins(0, 0, 0, 0);
			h2Values->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
			dispValues = new QLineEdit("steps.txt",h2ValuesWidget);
			dispValues->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
			h2Values->addWidget(dispValues);
			dispValues->hide();
			btnValues = new QPushButton("Browse",h2ValuesWidget);
			btnValues->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Values->addWidget(btnValues);
			btnValues->hide();
			h2Values->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

		}
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::Initialization(){
	if(id==NumPositions){
		connect(spinNumPos,		SIGNAL(valueChanged(int)), 				this, SLOT(SetNumPositions(int)));
		connect(btnDelete,		SIGNAL(clicked()),						this, SLOT(DeletePosition()));
	}else{
		connect(comboScript,	SIGNAL(currentIndexChanged(int)),		this,SLOT(SetScript(int)));
		connect(dispScript,		SIGNAL(editingFinished()),				this, SLOT(SetScriptFile()));
		connect(btnBrowse,		SIGNAL(clicked()), 						this, SLOT(BrowsePath()));
		connect(dispParameter,	SIGNAL(textChanged(const QString&)), 	this, SLOT(SetParameter(const QString&)));
		if(scanType){
			connect(radioConstant,	SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
			connect(radioSpecific,	SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
			connect(radioValue,		SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::EnableSizeWidgets(){
	// defaults
	lblFrom->hide();
	spinFrom->hide();
	lblTo->hide();
	spinTo->hide();
	lblSize->hide();
	spinSize->hide();
	comboSpecific->hide();
	dispValues->hide();
	btnValues->hide();
	lblSteps->setEnabled(true);
	spinSteps->setEnabled(true);
	// Constant Step Size
	if(radioConstant->isChecked()){
		lblFrom->show();
		spinFrom->show();
		lblTo->show();
		spinTo->show();
		lblSize->show();
		spinSize->show();
		lblSteps->setEnabled(false);
		spinSteps->setEnabled(false);
	}// Specific Values
	else if(radioSpecific->isChecked())
		comboSpecific->show();
	// Values from a File
	else{
		dispValues->show();
		btnValues->show();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::SetScript(int index){
	// defaults
	dispScript->setEnabled(false);
	btnBrowse->setEnabled(false);
	lblParameter->setEnabled(false);
	dispParameter->setEnabled(false);
	if(scanType){
		group->setEnabled(false);
		lblSteps->setEnabled(false);
		spinSteps->setEnabled(false);
		lblPrecision->setEnabled(false);
		spinPrecision->setEnabled(false);
	}
	// If anything other than None is selected
	if(index){
		// Custom Script only enables the first layout with addnl parameters etc
		if(!comboScript->currentText().compare("Custom Script")){
			dispScript->setEnabled(true);
			btnBrowse->setEnabled(true);
			lblParameter->setEnabled(true);
			dispParameter->setEnabled(true);
		}
		// If this group includes Energy scan , threhold scan etc
		if(scanType){
			group->setEnabled(true);
			lblPrecision->setEnabled(true);
			spinPrecision->setEnabled(true);
			// Steps are enabled only if constant step size is not checked
			lblSteps->setEnabled(!radioConstant->isChecked());
			spinSteps->setEnabled(!radioConstant->isChecked());
		}
	}
	//emit signal to enable scanbox and the radiobuttons
	if(scanType) emit EnableScanBox(index,((id==2)?1:0));

#ifdef VERBOSE
	cout << "Setting mode of action widget:" << id << " to " << index << endl;
#endif
	QString fName = dispScript->text();
	//script
	if((id!=Scan0)&&(id!=Scan1))
		//scan and positions wouldnt get here
		if(index)	myDet->setActionScript(GetActionIndex(id),fName.toAscii().constData());
		else 	myDet->setActionScript(GetActionIndex(id),"");
	//scan
	else{

	}

	cout<<"mode:"<<myDet->getActionMode(GetActionIndex(id))<<" "
			"script:"<<myDet->getActionScript(GetActionIndex(id))<<" "
			"parameter:"<<myDet->getActionParameter(GetActionIndex(id))<<endl;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void ActionsWidget::BrowsePath(){
	QString fName = dispScript->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir = "/home";
	//dialog
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Script File"),dir,
			tr("Script Files(*.awk);;All Files(*)"));//,0,QFileDialog::ShowDirsOnly);
	//if empty, set the file name and it calls setscriptfile, else ignore
	if (!fName.isEmpty())
		dispScript->setText(fName);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void ActionsWidget::SetScriptFile(){
	QString fName = dispScript->text();bool set = false;
#ifdef VERBOSE
	cout << "Setting script file of action widget:" << id << " to " << fName.toAscii().constData() << endl;
#endif
	disconnect(dispScript,	SIGNAL(editingFinished()),	this, 	SLOT(SetScriptFile()));

	//blank
	if(fName.isEmpty())
		set = true;
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
				qDefs::WarningMessage("The script file entered does not exist","ActionsWidget");
				dispScript->setText(QString(myDet->getActionScript(GetActionIndex(id)).c_str()));
			}
		}//not a file, set it to what it was before
		else {
			qDefs::WarningMessage("The script file path entered is not a file","ActionsWidget");
			dispScript->setText(QString(myDet->getActionScript(GetActionIndex(id)).c_str()));
		}
	}

	//if blank or valid file
	if(set){
		//script
		if((id!=Scan0)&&(id!=Scan1)){
			//scan and positions wouldnt get here
			if(!myDet->setActionScript(GetActionIndex(id),fName.toAscii().constData())){
				//did not get set, write what is was before
				if(!fName.isEmpty())
					qDefs::WarningMessage("The script file could not be set. Reverting to previous file.","ActionsWidget");
				dispScript->setText(QString(myDet->getActionScript(GetActionIndex(id)).c_str()));
			}
		}
		//scan
		else{

		}
	}

	//dont display if theres a none
	if(!dispScript->text().compare("none")) dispScript->setText("");

	connect(dispScript,	SIGNAL(editingFinished()),	this, 	SLOT(SetScriptFile()));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::SetParameter(const QString& parameter){
#ifdef VERBOSE
	cout << "Setting parameter of action widget:" << id << " to " << parameter.toAscii().constData() << endl;
#endif
	//script
	if((id!=ActionsWidget::Scan0)&&(id!=ActionsWidget::Scan1))
		//scan and positions wouldnt get here
		myDet->setActionParameter(GetActionIndex(id),parameter.toAscii().constData());
	//scan
	else{

	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void ActionsWidget::SetNumPositions(int index){
	//comboPos->setEnabled(index);
	//if there arent enough positions
	if((index) && (comboPos->count()<index)){
		qDefs::WarningMessage("Insufficient number of positions in the list. "
				"\nAdd more positions to the list, then set Number of Positions.","ActionsWidget");
		spinNumPos->setValue(comboPos->count());
	}else{
		//emit SetPositionsSignal();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::DeletePosition(){
	QString pos = comboPos->currentText();
	bool found = false;
	//loops through to find the index and to make sure its in the list
	for(int i=0;i<comboPos->count();i++){
		if(!comboPos->itemText(i).compare(pos)){
			found = true;
			comboPos->removeItem(i);
			break;
		}
	}
	//give the warning only when you try to delete stuff that arent there
	if((!found)&&(comboPos->count()))	qDefs::WarningMessage("This position cannot be deleted as it doesn't exist in the list anyway","ActionsWidget");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void ActionsWidget::Refresh(){
	//disabling signals and slots
	if(id==NumPositions){

	}else{
		disconnect(dispScript,		SIGNAL(editingFinished()),			this, SLOT(SetScriptFile()));
		disconnect(dispParameter,	SIGNAL(textChanged(const QString&)),this, SLOT(SetParameter(const QString&)));
	}


	int mode;string script,parameter;
	if((id == Scan0)||(id == Scan1)){

	}else if(id == NumPositions){

	}else{
		mode = (myDet->getActionMode(GetActionIndex(id))>0?1:0);
		script = myDet->getActionScript(GetActionIndex(id));
		parameter = myDet->getActionParameter(GetActionIndex(id));
		//defaults
		if(script == "none") script="";
		if(parameter == "none") parameter="";
		//settings values
		dispScript->setText(QString(script.c_str()));
		dispParameter->setText(QString(parameter.c_str()));
		//set mode which also checks everything
		comboScript->setCurrentIndex(mode);
	}


	//enabling signals and slots
	if(id==NumPositions){

	}else{
		connect(dispScript,		SIGNAL(editingFinished()),			this, SLOT(SetScriptFile()));
		connect(dispParameter,	SIGNAL(textChanged(const QString&)),this, SLOT(SetParameter(const QString&)));
	}
#ifdef VERBOSE
	cout << "Updated action widget " << id << "\tscript:" << script << "\tparameter:" << parameter << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int ActionsWidget::GetActionIndex(int gIndex){
	switch(gIndex){
	case Start: 		return slsDetectorDefs::startScript;
	case ActionBefore: 	return slsDetectorDefs::scriptBefore;
	case HeaderBefore: 	return slsDetectorDefs::headerBefore;
	case HeaderAfter: 	return slsDetectorDefs::headerAfter;
	case ActionAfter: 	return slsDetectorDefs::scriptAfter;
	case Stop: 			return slsDetectorDefs::stopScript;
	default: 			return -1;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

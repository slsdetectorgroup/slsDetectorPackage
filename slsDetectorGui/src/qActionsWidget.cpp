/*
 * qActionsWidget.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Project Class Headers
#include "qActionsWidget.h"
// Qt Include Headers
#include <QFileDialog>
// C++ Include Headers
#include<iostream>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------
int qActionsWidget::NUM_ACTION_WIDGETS(0);
//-------------------------------------------------------------------------------------------------------------------------------------------------


qActionsWidget::qActionsWidget(QWidget *parent,multiSlsDetector*& detector):
		QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qActionsWidget::~qActionsWidget(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qActionsWidget::SetupWidgetWindow(){
	id = NUM_ACTION_WIDGETS;
	NUM_ACTION_WIDGETS++;

	setFixedHeight(25);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qActionsWidget::Initialization(){
	//mode
	connect(comboScript,	SIGNAL(currentIndexChanged(int)),		this,SLOT(SetMode(int)));
	//file
	connect(dispScript,		SIGNAL(editingFinished()),				this, SLOT(SetScriptFile()));
	connect(btnBrowse,		SIGNAL(clicked()), 						this, SLOT(BrowsePath()));
	//parameter
	connect(dispParameter,	SIGNAL(editingFinished()), 				this, SLOT(SetParameter()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qActionsWidget::SetMode(int mode){
#ifdef VERBOSE
	cout << "Setting\taction:" << id << "\tmode:" << mode << endl;
#endif
	//enabling/disabling
	dispScript->setEnabled(mode);
	btnBrowse->setEnabled(mode);
	lblParameter->setEnabled(mode);
	dispParameter->setEnabled(mode);

	QString fName = dispScript->text();
	//set the mode
	if(mode)	myDet->setActionScript(id,fName.toAscii().constData());
	else myDet->setActionScript(id,"");
	//mode is not set when fname is blank

	if(!fName.isEmpty()){
		//check if mode didnt get set
		if(mode!=myDet->getActionMode(id)){
			qDefs::Message(qDefs::WARNING,"The mode could not be changed.","qActionsWidget::SetMode");
			comboScript->setCurrentIndex(myDet->getActionMode(id));
		}//if mode got set and its custom script
		else if(mode){
			//when the file name did not get set correctly
			if(fName.compare(QString(myDet->getActionScript(id).c_str()))){
				qDefs::Message(qDefs::WARNING,"The file path could not be set.","qActionsWidget::SetMode");
				dispScript->setText(QString(myDet->getActionScript(id).c_str()));
				SetScriptFile();
			}
		}
	}
	qDefs::checkErrorMessage(myDet,"qActionsWidget::SetMode");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qActionsWidget::BrowsePath(){
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

void qActionsWidget::SetScriptFile(){
	QString fName = dispScript->text();
#ifdef VERBOSE
	cout << "Setting\taction:" << id << "\tscript:" << fName.toAscii().constData() << endl;
#endif
	bool set = false;
	struct stat st_buf;

	//blank
	if(fName.isEmpty())
		set = true;
	else if(!fName.compare("none"))
		set = true;
	//not blank
	else{
		//path doesnt exist
		if(stat(fName.toAscii().constData(),&st_buf)){
			qDefs::Message(qDefs::WARNING,"The script file entered does not exist","qActionsWidget::SetScriptFile");
			dispScript->setText(QString(myDet->getActionScript(id).c_str()));
		}
		//if its not a file
		else if (!S_ISREG (st_buf.st_mode)) {
			qDefs::Message(qDefs::WARNING,"The script file path entered is not a file","qActionsWidget::SetScriptFile");
			dispScript->setText(QString(myDet->getActionScript(id).c_str()));
		}
		else
			set=true;
	}


	//if blank or valid file
	if(set){
		//scan and positions wouldnt get here
		myDet->setActionScript(id,fName.toAscii().constData());
		if(fName.compare(QString(myDet->getActionScript(id).c_str()))){
			//did not get set, write what is was before
			if(!fName.isEmpty())
				qDefs::Message(qDefs::WARNING,"The script file could not be set. Reverting to previous file.","qActionsWidget::SetScriptFile");
			dispScript->setText(QString(myDet->getActionScript(id).c_str()));
		}
	}

	//dont display if theres a none
	if(!dispScript->text().compare("none")) dispScript->setText("");

	qDefs::checkErrorMessage(myDet,"qActionsWidget::SetScriptFile");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qActionsWidget::SetParameter(){
	QString parameter = dispParameter->text();
#ifdef VERBOSE
	cout << "Setting\taction:" << id << "\tparameter:" << parameter.toAscii().constData() << endl;
#endif
	myDet->setActionParameter(id,parameter.toAscii().constData());
	//dont display if theres a none
	if(!dispParameter->text().compare("none")) dispParameter->setText("");

	qDefs::checkErrorMessage(myDet,"qActionsWidget::SetParameter");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qActionsWidget::Refresh(){
	int mode = (myDet->getActionMode(id)>0?1:0);
	string script = myDet->getActionScript(id);
	string parameter = myDet->getActionParameter(id);


	//settings values and checking for none
	if(QString(script.c_str()).compare("none"))
		dispScript->setText(QString(script.c_str()));
	if(mode)SetScriptFile();
	dispParameter->setText(QString(parameter.c_str()));
	SetParameter();
	//set mode which also checks everything
	comboScript->setCurrentIndex(mode);
#ifdef VERBOSE
	cout << "Updated\taction:" << id << "\t"
			"mode:"<<mode<<"\t"
			"script:" << script << "\t"
			"parameter:" << parameter << "\t***" << endl;
#endif

	qDefs::checkErrorMessage(myDet,"qActionsWidget::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

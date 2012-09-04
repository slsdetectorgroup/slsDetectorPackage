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

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Initialization(){
	//energy/angular logs
	connect(chkEnergyLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));
	connect(chkAngularLog,	SIGNAL(toggled(bool)),		this,	SLOT(SetLogs()));


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
		qDefs::WarningMessage("Could not set/reset Log.","Advanced");
		checkedBox->setChecked(!enable);
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Refresh(){
	//energy/angular logs
	chkEnergyLog->setChecked(myDet->getActionMode(slsDetectorDefs::enCalLog));
	chkAngularLog->setChecked(myDet->getActionMode(slsDetectorDefs::angCalLog));
#ifdef VERBOSE
	cout << "Energy Calibration Log set to " << chkEnergyLog->isChecked() << endl;
	cout << "Angular Calibration Log set to " << chkAngularLog->isChecked() << endl;
#endif


}


//-------------------------------------------------------------------------------------------------------------------------------------------------


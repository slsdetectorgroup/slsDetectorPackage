/*
 * qTabMessages.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: l_maliakal_d
 */

#include "qTabMessages.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QDir>
#include <QProcess>
#include <QStringList>
/** C++ Include Headers */
#include <iostream>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabMessages::qTabMessages(QWidget *parent,slsDetectorUtils*& detector):
		QWidget(parent),myDet(detector){
	SetupWidgetWindow();
	Initialization();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabMessages::~qTabMessages(){
	delete myDet;
	delete dispLog;
	delete dispCommand;
	delete dispPath;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::SetupWidgetWindow(){
	/** Layout */
	QGridLayout *gridLayout = new QGridLayout(this);
	QLabel *lblCommand 	= new QLabel("System Command:",this);
	QLabel *lblPath 	= new QLabel("Working Directory:",this);
	QSpacerItem *hSpacer= new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
	dispLog 	= new QTextEdit(this);
	dispCommand	= new QLineEdit(this);
	dispPath	= new QLineEdit(this);
	dispLog->setReadOnly(true);
	dispPath->setReadOnly(true);
	dispLog->setFocusPolicy(Qt::NoFocus);
	dispPath->setFocusPolicy(Qt::NoFocus);
	gridLayout->addWidget(dispLog, 		0, 0, 1, 3);
	gridLayout->addWidget(lblCommand, 	1, 0, 1, 1);
	gridLayout->addItem(hSpacer,		1, 1, 1, 1);
	gridLayout->addWidget(dispCommand, 	1, 2, 1, 1);
	gridLayout->addWidget(lblPath, 		2, 0, 1, 1);
	gridLayout->addWidget(dispPath, 	2, 2, 1, 1);

	/** Command & Path*/
	dispCommand->setText("Insert your command here");
	dispPath->setText(QDir("./").absolutePath());
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::Initialization(){
	connect(dispCommand,SIGNAL(returnPressed()),this,SLOT(executeCommand()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::executeCommand(){
#ifdef VERBOSE
	cout<<"Calling: "<< dispCommand->text().toAscii().constData()<<endl;
#endif
	QProcess proc(this);
#ifdef VERBOSE
	//std::cout <<"working directory would be " << proc.workingDirectory().absPath() << std::endl;
	cout<<"Original Working Directory: "<< proc.workingDirectory().toAscii().constData()<<endl;
#endif
	proc.setWorkingDirectory(QDir(dispPath->text()).absolutePath());
#ifdef VERBOSE
	//std::cout <<"working directory is " << proc.workingDirectory().absPath() << std::endl;
	cout<<"Current Working Directory: "<<proc.workingDirectory().toAscii().constData()<<endl;
#endif
	proc.setArguments(QStringList::split(' ',dispCommand->text()));
#ifdef VERBOSE
/*	QStringList list = proc.arguments();
	QStringList::Iterator it = list.begin();
	while( it != list.end() ) {
		cout<<*it<<endl;
		++it;
	}*/
#endif
	//if (!proc.start()) {
	if(!(proc.state()==QProcess::Running)){
		// error handling
		cout<<"Could not launch process"<<endl;
	} else {
		//while(proc.isRunning()) {
		while(proc.state()==QProcess::Running){
			;
#ifdef VERBOSE
			cout<<"Process running "<< proc.exitStatus()<< endl;
#endif
		}
		//if (proc.normalExit()) {
		if(proc.exitStatus()==QProcess::NormalExit){
			;
#ifdef VERBOSE
			cout<<" process returned OK"<<endl;
#endif
		} else
			cout<<" process returned error"<<endl;
		while (proc.canReadLineStdout()) {//readAllStandardOutput ()
			cout<<proc.readLineStdout() <<endl;
		}
		while (proc.canReadLineStderr()) {
			cout<<"Error: "<<proc.readLineStderr() <<endl;
		}
	}


}

//-------------------------------------------------------------------------------------------------------------------------------------------------

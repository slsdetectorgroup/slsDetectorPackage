/*
 * qTabDataOutput.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDataOutput.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QFileDialog>
/** C++ Include Headers */
#include <iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDataOutput::qTabDataOutput(QWidget *parent,slsDetectorUtils*& detector):
		QWidget(parent),myDet(detector){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDataOutput::~qTabDataOutput(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::SetupWidgetWindow(){
	dispOutputDir->setText(QString(myDet->getFilePath().c_str()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Initialization(){
	connect(dispOutputDir,		SIGNAL(textChanged(const QString&)), 	this, 	SLOT(setOutputDir(const QString&)));
	connect(btnOutputBrowse,	SIGNAL(clicked()), 						this, 	SLOT(browseOutputDir()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::setOutputDir(const QString& path){
	myDet->setFilePath(string(path.toAscii().constData()));
#ifdef VERBOSE
	cout<<"Output Directory changed to :"<<myDet->getFilePath()<<endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::Refresh(){
	/** output dir*/
	dispOutputDir->setText(QString(myDet->getFilePath().c_str()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDataOutput::browseOutputDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,tr("Choose Output Directory "),dispOutputDir->text());
    if (!directory.isEmpty())
    	dispOutputDir->setText(directory);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

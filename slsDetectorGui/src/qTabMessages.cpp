/*
 * qTabMessages.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: l_maliakal_d
 */

/** Qt Project Class Headers */
#include "qTabMessages.h"
#include "qDefs.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QGridLayout>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>


/** C++ Include Headers */
#include <iostream>
#include <string>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabMessages::qTabMessages(QWidget *parent):QWidget(parent),myDet(0){
	SetupWidgetWindow();
	Initialization();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabMessages::~qTabMessages(){
	delete myDet;
	delete dispLog;
	delete qout;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::SetupWidgetWindow(){
	/** Layout */
	QGridLayout *gridLayout = new QGridLayout(this);

	dispLog 	= new QTextEdit(this);
	dispLog->setReadOnly(true);
	dispLog->setFocusPolicy(Qt::NoFocus);
	dispLog->setTextColor(Qt::darkBlue);

	btnSave = new QPushButton("Save Log  ",this);
	btnSave->setFocusPolicy(Qt::NoFocus);
	btnSave->setFixedWidth(100);
 	btnSave->setIcon(QIcon( ":/icons/images/save.png" ));

	btnClear = new QPushButton("Clear  ",this);
	btnClear->setFocusPolicy(Qt::NoFocus);
	btnClear->setFixedWidth(100);
	btnClear->setIcon(QIcon( ":/icons/images/erase.png" ));

	gridLayout->addItem(new QSpacerItem(15,10,QSizePolicy::Fixed,QSizePolicy::Fixed),0,0);
	gridLayout->addWidget(btnSave,1,0,1,1);
	gridLayout->addWidget(btnClear,1,4,1,1);
	gridLayout->addItem(new QSpacerItem(15,10,QSizePolicy::Fixed,QSizePolicy::Fixed),2,0);
	gridLayout->addWidget(dispLog,3,0,1,5);

	qout=new qDebugStream(cout,cerr, this);cout<<"working!"<<endl;
	//qerr=new qDebugStream(cerr,this);

	//qerr=NULL;
	//qerr=new qDebugStream(std::cerr,this);cout<<"worked!"<<endl;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::Initialization(){
	connect(btnSave,SIGNAL(clicked()),this,SLOT(SaveLog()));
	connect(btnClear,SIGNAL(clicked()),this,SLOT(ClearLog()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::customEvent(QEvent *e) {
  if (e->type() == STREAMEVENT)
	  dispLog->append(((qStreamEvent*)e)->getString());
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::SaveLog() {
	//cerr<<endl<<"ERRRORRRR"<<endl<<endl;
	QString fName = QString(myDet->getFilePath().c_str());
	fName = fName+"/LogFile.txt";
    fName = QFileDialog::getSaveFileName(this,tr("Save Snapshot "),
    		fName,tr("Text files (*.txt)"));
    if (!fName.isEmpty()){
    	QFile outfile;
    	outfile.setFileName(fName);
    	if(outfile.open(QIODevice::WriteOnly | QIODevice::Text)){//Append
    		QTextStream out(&outfile);
    		out<<dispLog->toPlainText() << endl;
    		qDefs::Message(qDefs::INFORMATION,string("The Log has been successfully saved to "
    				"")+fName.toAscii().constData(),"Messages");
    	}
    	else qDefs::Message(qDefs::WARNING,"Attempt to save log file failed.","Messages");
     }
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabMessages::ClearLog() {
	dispLog->clear();
#ifdef VERBOSE
		cout<<"Log Cleared"<<endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

/*
 * qTabMessages.h
 *
 *  Created on: Jun 26, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABMESSAGES_H_
#define QTABMESSAGES_H_


#include "qDefs.h"



/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QWidget>
#include <QTextEdit>
#include <QEvent>
#include <QPushButton>
#include "qDebugStream.h"

/**
 *@short sets up the Messages parameters
 */
class qTabMessages:public QWidget{
	Q_OBJECT

public:
	/** \short The constructor
	 *  @param parent is the parent tab widget
	 *  @param det the detector reference
	 */
	qTabMessages(QWidget *parent,multiSlsDetector* detector);

	/** Destructor
	 */
	~qTabMessages();


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** Log of executed commands */
	QTextEdit *dispLog;

	/** To save the log to file */
	QPushButton *btnSave;

	/** To clear the log to file */
	QPushButton *btnClear;

	/** This class creates the log */
	qDebugStream *qout;
	//qDebugStream *qerr;

/** methods */
	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

private slots:
/** Stream log to textedit in GUI */
void customEvent(QEvent *e);

/** Save Log to File*/
void SaveLog();

/** Clear Log to File*/
void ClearLog();



};





#endif /* QTABMESSAGES_H_ */

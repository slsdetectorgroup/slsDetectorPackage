/*
 * qTabMessages.h
 *
 *  Created on: Jun 26, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABMESSAGES_H_
#define QTABMESSAGES_H_



/** Project Class Headers */
class slsDetectorUtils;
/** Qt Include Headers */
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>

/**
 *@short sets up the Messages parameters
 */
class qTabMessages:public QWidget{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabMessages(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qTabMessages();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** Log of executed commands */
	QTextEdit *dispLog;

	/** Command display */
	QLineEdit *dispCommand;

	/** Path display */
	QLineEdit *dispPath;

/** methods */
	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();


private slots:
	void executeCommand();

};





#endif /* QTABMESSAGES_H_ */

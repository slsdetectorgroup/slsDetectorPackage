#pragma once

#include "qDefs.h"
#include "qDebugStream.h"
class qDetectorMain;

#include <QWidget>
#include <QTextEdit>
#include <QEvent>
#include <QPushButton>


/**
 *@short sets up the Messages parameters
 */
class qTabMessages:public QWidget{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent parent widget
	 */
	qTabMessages(QWidget* parent);

	/**
	 * Destructor
	 */
	~qTabMessages();

private slots:
	/**
	 * Stream log to textedit in GUI
	 */
	void customEvent(QEvent *e);

	/**
	 * Save Log to File
	 */
	void SaveLog();

	/**
	 * Clear Log to File
	 */
	void ClearLog();

private:

	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();


	/** The qDetectorMain object */
	qDetectorMain *myMainTab;

	/** Log of executed commands */
	QTextEdit *dispLog;

	/** To save the log to file */
	QPushButton *btnSave;

	/** To clear the log to file */
	QPushButton *btnClear;
};


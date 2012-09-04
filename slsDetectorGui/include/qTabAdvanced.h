/*
 * qTabAdvanced.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABADVANCED_H_
#define QTABADVANCED_H_

/** Form Header */
#include "ui_form_tab_advanced.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Header */
#include <QStackedLayout>

/**
 *@short sets up the advanced parameters
 */
class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabAdvanced(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabAdvanced();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();


	/** The sls detector object */
	multiSlsDetector *myDet;


private slots:
/** Enable/Disable Energy and Calibration Logs
 */
void SetLogs();

};



#endif /* QTABADVANCED_H_ */

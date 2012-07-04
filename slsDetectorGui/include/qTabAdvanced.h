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
class slsDetectorUtils;

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
	qTabAdvanced(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qTabAdvanced();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** The sls detector object type*/
	int myDetType;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);

private slots:
/** Get Temperature data
 */
void getTemperature();

};



#endif /* QTABADVANCED_H_ */

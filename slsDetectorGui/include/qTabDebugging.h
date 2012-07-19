/*
 * qTabDebugging.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEBUGGING_H_
#define QTABDEBUGGING_H_

/** Form Header */
#include "ui_form_tab_debugging.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */

/** C++ Include Headers */
#include "sls_detector_defs.h"

/**
 *@short sets up the Debugging parameters
 */
class qTabDebugging:public QWidget, private Ui::TabDebuggingObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDebugging(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDebugging();

	/** To refresh and update widgets
	 */
	void Refresh();

private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);

};



#endif /* QTABDEBUGGING_H_ */

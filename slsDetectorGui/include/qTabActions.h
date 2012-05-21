/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABACTIONS_H_
#define QTABACTIONS_H_

/** Form Header */
#include "ui_form_tab_actions.h"
/** Project Class Headers */
class slsDetectorUtils;

/**
 *@short sets up the acions parameters
 */
class qTabActions:public QWidget, private Ui::TabActionsObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabActions(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qTabActions();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

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

#endif /* QTABACTIONS_H_ */

/*
 * qTabDeveloper.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEVELOPER_H_
#define QTABDEVELOPER_H_

/** Form Header */
#include "ui_form_tab_developer.h"
/** Project Class Headers */
class slsDetectorUtils;

/**
 *@short sets up the Developer parameters
 */
class qTabDeveloper:public QWidget, private Ui::TabDeveloperObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDeveloper(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qTabDeveloper();


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



#endif /* QTABDEVELOPER_H_ */

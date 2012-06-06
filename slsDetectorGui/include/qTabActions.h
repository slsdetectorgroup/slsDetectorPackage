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
class ActionsWidget;


class QPushButton;
class QLabel;
class QScrollArea;
class QGridLayout;
class QPalette;


/**
 *@short sets up the acions parameters
 */
class qTabActions:public QWidget,private Ui::TabActionsObject{
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

	static const int NUM_ACTION_WIDGETS = 9;

	QGridLayout		*gridLayout;
	QButtonGroup 	*group;
	QPalette		*palette;

	/** action widget objects */
	ActionsWidget	*actionWidget[NUM_ACTION_WIDGETS];
	QPushButton 	*btnExpand[NUM_ACTION_WIDGETS];
	QLabel			*lblName[NUM_ACTION_WIDGETS];

	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/** Enables/Disables all the widgets */
	void Enable(bool enable);


private slots:
/** To Expand the Action Widget */
void Expand(QAbstractButton *button);

};

#endif /* QTABACTIONS_H_ */


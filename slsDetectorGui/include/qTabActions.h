/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABACTIONS_H_
#define QTABACTIONS_H_

/** Project Class Headers */
class multiSlsDetector;
class ActionsWidget;
/** Qt Include Headers */
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QPalette>


/**
 *@short sets up the acions parameters
 */
class qTabActions:public QWidget{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabActions(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabActions();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

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



private slots:
/** To Expand the Action Widget */
void Expand(QAbstractButton *button);


signals:
void EnableScanBox(bool,int);

};

#endif /* QTABACTIONS_H_ */


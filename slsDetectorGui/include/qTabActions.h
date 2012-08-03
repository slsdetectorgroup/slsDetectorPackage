/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABACTIONS_H_
#define QTABACTIONS_H_


/* Qt Project Class Headers */
#include "qActionsWidget.h"
#include "qScanWidget.h"
/** Project Class Headers */
class multiSlsDetector;
class qActionsWidget;
/** Qt Include Headers */
#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
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

	enum actionIndex{Scan0=1, Scan1, NumPositions=4, NumTotalActions=9};

	QGridLayout		*gridLayout;
	QButtonGroup 	*group;
	QPalette		*palette;

	/** all action widget objects */
	qActionsWidget	*actionWidget[6];
	qScanWidget		*scanWidget[2];
	QWidget			*positionWidget;
	QPushButton 	*btnExpand[NumTotalActions];
	QLabel			*lblName[NumTotalActions];


	/** NumPositions widget */
	QLabel 		*lblNumPos;
	QLabel 		*lblPosList;
	QSpinBox 	*spinNumPos;
	QComboBox	*comboPos;
	QPushButton *btnDelete;
	QCheckBox 	*chkInvert;
	QCheckBox 	*chkSeparate;
	QCheckBox 	*chkReturn;

	double		*positions;
	QPalette	normal;


	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/** creates the Num Positions object */
	void CreatePositionsWidget();

	/** Returns the index in each of the classes
	 * of actionwidget and scanwidget
	 * @param index the index in the list of all widgets
	 * returns actual index of the class
	 */
	int GetActualIndex(int index);

private slots:
/** To Expand the Action Widget
 * */
void Expand(QAbstractButton *button);

/** Sets the positions list and the number of positions
 * */
void SetPosition();

/** Deletes current position
 * */
void DeletePosition();

signals:
void EnableScanBox(bool,int);

};

#endif /* QTABACTIONS_H_ */


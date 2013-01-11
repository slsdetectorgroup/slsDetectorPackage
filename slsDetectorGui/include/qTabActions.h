/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABACTIONS_H_
#define QTABACTIONS_H_

#include "qDefs.h"


/* Qt Project Class Headers */
#include "qActionsWidget.h"
#include "qScanWidget.h"
/** C++ Project Class Headers */
class multiSlsDetector;
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


public slots:
	/** Disable Positions
	 * @param enable true if to disable
	 * */
	void EnablePositions(bool enable);


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	slsDetectorDefs::detectorType detType;


	enum actionIndex{
		ActionStart,
		Scan0,
		Scan1,
		ActionBefore,
		NumPositions,
		HeaderBefore,
		HeaderAfter,
		ActionAfter,
		ActionStop,
		NumTotalActions};

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
	QIcon		*iconPlus;
	QIcon		*iconMinus;


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

	/** Updates to green color if collapsed and mode not none
	 */
	void UpdateCollapseColors();




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
	void EnableScanBox();

};

#endif /* QTABACTIONS_H_ */


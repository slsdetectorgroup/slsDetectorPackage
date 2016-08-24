/*
 * qTabDeveloper.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEVELOPER_H_
#define QTABDEVELOPER_H_

#include "qDefs.h"


/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QTimer>
#include <QString>
#include <QPalette>
class qDetectorMain;

/** C++ Include Headers */
#include <string>
#include <vector>
using namespace std;


/**To override the spin box class to have an id and emit it*/
class MyDoubleSpinBox:public QDoubleSpinBox{
Q_OBJECT
private:
	int myId;
	private slots:
	void valueChangedWithID() {emit editingFinished(myId);};
	public:
	/** Overridden constructor from QDoubleSpinBox */
	MyDoubleSpinBox(int id,QWidget* parent = 0)
	:QDoubleSpinBox(parent),myId(id){
		//setParent(parent);
		connect(this,SIGNAL(editingFinished()),
				this,SLOT(valueChangedWithID()));
	}
	signals:
	void editingFinished(int myId);
};



/**
 *@short sets up the Developer parameters
 */
class qTabDeveloper:public QWidget{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDeveloper(qDetectorMain *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDeveloper();

	/** To stop ADC Timer when starting acquisition
	 */
	void StopADCTimer(){if(adcTimer) adcTimer->stop();};

private:
	/** parent widget */
	qDetectorMain *thisParent;
	/** The sls detector object */
	multiSlsDetector *myDet;
	/** The sls detector object */
	slsDetector *det;
	/** detector type */
	slsDetectorDefs::detectorType detType;
	/**number of dac widgets*/
	static int NUM_DAC_WIDGETS;
	/**number of adc widgets*/
	static int NUM_ADC_WIDGETS;

	static const int ADC_TIMEOUT = 5000;

	vector<string>dacNames;
	vector<string>adcNames;


	/**widgets needed*/
	QGridLayout 	*layout;
	QScrollArea 	*scroll;
	QGroupBox		*boxDacs;
	QGroupBox		*boxAdcs;
	QLabel 			*lblDacs[20];
	QLabel			*lblAdcs[20];
	MyDoubleSpinBox	*spinDacs[20];
	QLabel			*lblDacsmV[20];
	QDoubleSpinBox	*spinAdcs[20];
	QLabel			*lblHV;
	QComboBox		*comboHV;
	QTimer 			*adcTimer;
	QGridLayout		*dacLayout;
	QString 		tipHV;
	QPalette		red;
	QComboBox		*comboDetector;
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Sets up the DAC Widgets
	 */
	void CreateDACWidgets();

	/** Sets up the ADC Widgets
	 */
	void CreateADCWidgets();

	/** Gets the sls index to set/get dac/adc
	 * @param index is the gui dac/adc index
	 * returns the sls index
	 */
	slsDetectorDefs::dacIndex getSLSIndex(int index);

public slots:

	/** To refresh and update widgets
	 */
	void Refresh();

private slots:
	/** Refreshes the adcs
	 */
	void RefreshAdcs();

	/** Set Dac values
	 * @param id id of dac
	 */
	void SetDacValues(int id);

	/** Set High Voltage
	 */
	void SetHighVoltage();
};



#endif /* QTABDEVELOPER_H_ */

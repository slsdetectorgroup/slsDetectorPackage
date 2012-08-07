/*
 * qTabDeveloper.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEVELOPER_H_
#define QTABDEVELOPER_H_

/** Project Class Headers */
class multiSlsDetector;
#include "sls_detector_defs.h"
/** Qt Include Headers */
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QTimer>
/** C++ Include Headers */
#include <string>
#include <vector>
using namespace std;


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
	qTabDeveloper(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDeveloper();

	/** To refresh and update widgets
	 */
	void Refresh();

private:
	/** The sls detector object */
	multiSlsDetector *myDet;
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
	QDoubleSpinBox	*spinDacs[20];
	QDoubleSpinBox	*spinAdcs[20];
	QTimer 			*adcTimer;

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



private slots:
/** Refreshes the adcs
 */
void RefreshAdcs();

};



#endif /* QTABDEVELOPER_H_ */

/*
 * qEnergyCalibration.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QENERGY_CALIBRATION_H_
#define QENERGY_CALIBRATION_H_

/** Form Header */
#include "ui_form_energy_calibration.h"
/** Project Class Headers */
class multiSlsDetector;

/**
 *@short sets up the advanced parameters
 */
class qEnergyCalibration:public QWizard, private Ui::EnergyCalibrationObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qEnergyCalibration(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qEnergyCalibration();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();



private slots:


};



#endif /* QENERGY_CALIBRATION_H_ */

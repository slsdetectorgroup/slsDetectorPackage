/*
 * qTabMeasurement.h
 *
 *  Created on: May 2, 2012
 *      Author: l_maliakal_d
 */
#ifndef QTABMEASUREMENT
#define QTABMEASUREMENT

#include "qDefs.h"


/** Form Header */
#include "ui_form_tab_measurement.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Project Class Headers */
#include <QStandardItemModel>
#include "qDrawPlot.h"
class qDetectorMain;

/**
 *@short sets up the measurement parameters
 */
class qTabMeasurement:public QWidget, private Ui::TabMeasurementObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    This tab allows to change the detector settings, the threshold, the number of (non real time) measurements,
	 *    the acquisition time, the file name, the start run index and shows the current progress of the measurement
	 *    via a progress bar and labels inidicating the current position, scan variable, frame number etc.
	 *    Contains the start and stop acquisition button
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 *    @param plot plot object reference
	 */
	qTabMeasurement(qDetectorMain *parent,multiSlsDetector*& detector, qDrawPlot*& plot);

	/** Destructor
	 */
	~qTabMeasurement();

	/** To refresh and update widgets
	 */
	void Refresh();

	/** To enable expert mode
	 * @param enable to enable if true
	 */
	void SetExpertMode(bool enable);

	/** Returns the status of the Start/Stop Acquisition button
	 */
	bool GetStartStatus(){return btnStartStop->isChecked();};

	/** Click the Start/Stop Acquisition button
	 *  This is used if this command came from gui client
	 */
	void ClickStartStop(){btnStartStop->click();myPlot->SetClientInitiated();};

	/** Returns progress bar value */
	int GetProgress(){return progressBar->value();};


public slots:

	/** update plot is finished,
	 * changes start/stop text and enables/disables all widgets
	 */
	void UpdateFinished();

	/** updates the current measurement
	 * @param val the value to be updated
	 */
	void SetCurrentMeasurement(int val);




private:
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up the timing mode
	 */
	void SetupTimingMode();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);

	/** Validates before enabling or disabling probes */
	void EnableProbes();

	/** Get timing mode from detector
	 * @param startup is true when gui has just started up*/
	void GetModeFromDetector(bool startup = false);

	/** Checks if acquisition period is greater than exposure time
	 * and dsplays in red as a warning */
	void CheckAcqPeriodGreaterThanExp();


private slots:
	/** Sets the timing mode
	 * @ param mode cane be None, Auto, Gated, Trigger Exposure Series,
	 * Trigger Frame, Trigger Readout, External Trigger Window
	 */
	void SetTimingMode(int mode);

	/** Set number of measurements
	 *  @param num number of measurements to be set */
	void setNumMeasurements(int num);

	/** Set file name
	 */
	void setFileName();

	/** Set index of file name
	 * @param index index of selection
	 */
	void setRunIndex(int index);

	/** starts/stops Acquisition
	 */
	void startStopAcquisition();

	/** Set number of frames
	 *  @param val number of frames to be set
	 */
	void setNumFrames(int val);

	/** Set acquisition time
	 */
	void setExposureTime();

	/** Set frame period between exposures
	 */
	void setAcquisitionPeriod();

	/** Set number of triggers
	 *  @param val number of triggers to be set
	 */
	void setNumTriggers(int val);

	/** Set delay
	 */
	void setDelay();

	/** Set number of gates
	 *  @param val number of gates to be set
	 */
	void setNumGates(int val);

	/** Set number of probes
	 *  @param val number of probes to be set
	 */
	void setNumProbes(int val);

	/** Update progress*/
	void UpdateProgress();

	/** Enable write to file */
	void EnableFileWrite(bool enable);


private:
	/** parent widget */
	qDetectorMain *thisParent;
	/** The sls detector object */
	multiSlsDetector *myDet;
	/** The Plot widget	 */
	qDrawPlot *myPlot;
	/** detector type */
	slsDetectorDefs::detectorType detType;
	/** enum for the timing mode */
	enum{None, Auto, Trigger_Exp_Series, Trigger_Readout, Gated, Gated_Start, Burst_Trigger, NumTimingModes};
	/** timer to update the progress*/
	QTimer *progressTimer;
	/** tool tip variables*/
	QString 	acqPeriodTip;
	QString 	errPeriodTip;
	QPalette	red;
	/** expert mode */
	bool expertMode;
	QIcon	*iconStart;
	QIcon	*iconStop;
	/** to access items in settings combobox */
	QStandardItemModel* model;

signals:
	void StartSignal();
	void StopSignal();
	void CheckPlotIntervalSignal();
};



#endif /* QTABMEASUREMENT */

/*
 * qTabMeasurement.h
 *
 *  Created on: May 2, 2012
 *      Author: l_maliakal_d
 */
#ifndef QTABMEASUREMENT
#define QTABMEASUREMENT

/** Form Header */
#include "ui_form_tab_measurement.h"
/** Project Class Headers */
class slsDetectorUtils;
/** Qt Project Class Headers */
class qDrawPlot;

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
	qTabMeasurement(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot);

	/** Destructor
	 */
	~qTabMeasurement();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** The Plot widget	 */
	qDrawPlot *myPlot;

	enum{None, Auto, Gated, Trigger_Exp_Series, Trigger_Frame, Trigger_Readout, Gated_Start, Trigger_Window, NumTimingModes};


/** methods */
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 * @param timingChange only some of the signals are disconnected when timing mode is changed
	 * This method is to reconnect them again.
	 */
	void Initialization(int timingChange=0);

	/** Disconnects all the slots and signals (which depend on timing mode)
	 * to retrieve all the parameters from client.
	 * This is done only when Timing mode is changed
	 */
	void DeInitialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);


public slots:

/** update plot is finished,
 * changes start/stop text and enables/disables all widgets
 */
void UpdateFinished();



private slots:
/** Set file name
 * @param fName name of file
 */
void setFileName(const QString& fName);

/** Set index of file name
 * @param index index of selection
 */
void setRunIndex(int index);

/** starts/stops Acquisition
 */
void startStopAcquisition();

/** Sets the timing mode
 * @ param mode cane be None, Auto, Gated, Trigger Exposure Series,
 * Trigger Frame, Trigger Readout, External Trigger Window
 */
void setTimingMode(int mode);

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

signals:

void StartSignal();
void StopSignal();
};



#endif /* QTABMEASUREMENT */

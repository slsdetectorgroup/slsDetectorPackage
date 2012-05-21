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


/** methods */
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);


public slots:
/** update plot is finished,
 * changes start/stop text and enables/disables all widgets
 */
void UpdateFinished();


private slots:
/** Set settings according to selection
 *  @param index index of selection
 */
void setSettings(int index);
/** Set number of frames
 *  @param val number of frames to be set
 */
void setNumFrames(int val);

/** Set acquisition time
 */
void setAcquisitionTime();

/** Set frame period between exposures
 */
void setFramePeriod();

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

signals:


};



#endif /* QTABMEASUREMENT */

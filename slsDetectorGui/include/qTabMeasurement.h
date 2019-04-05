#pragma once

#include "qDefs.h"

#include "ui_form_tab_measurement.h"

class multiSlsDetector;

#include <QStandardItemModel>
#include "qDrawPlot.h"
class qDetectorMain;

/**
 *@short sets up the measurement parameters
 */
class qTabMeasurement:public QWidget, private Ui::TabMeasurementObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * This tab allows to change measurement parameters and to start/stop an acquisition
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 * @param plot plot object reference
	 */
	qTabMeasurement(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot);

	/**
	 * Destructor
	 */
	~qTabMeasurement();

	/**
	 * Returns the status of the acquisition in gui
	 */
	bool GetStartStatus();

	/**
	 * Click the Start/Stop Acquisition button
	 *  This is used if this command came from gui client
	 */
	void ClickStartStop();

	/**
	 * Returns progress bar value
	 */
	int GetProgress();

	/**
	 * Refresh and update widgets
	 */
	void Refresh();


public slots:

	/**
	 * Update plot is finished,
	 * changes start/stop text and enables/disables all widgets
	 */
	void UpdateFinished();

	/**
	 * Updates the current measurement
	 * @param val the value to be updated
	 */
	void SetCurrentMeasurement(int val);

private slots:

	/**
	 * Set number of measurements
	 * @param num number of measurements to be set
	 */
	void setNumMeasurements(int num);

	/**
	 * Set number of frames
	 *  @param val number of frames to be set
	 */
	void setNumFrames(int val);

	/**
	 * Set acquisition time
	 */
	void setExposureTime();

	/**
	 * Set frame period between exposures
	 */
	void setAcquisitionPeriod();

	/**
	 * Set number of triggers
	 *  @param val number of triggers to be set
	 */
	void setNumTriggers(int val);

	/**
	 * Set delay
	 */
	void setDelay();

	/**
	 * Set number of samples
	 *  @param val number of samples to be set
	 */
	void setNumSamples(int val);

	/**
	 * Set file name
	 */
	void setFileName();

	/**
	 * Enable write to file
	 */
	void EnableFileWrite(bool enable);

	/**
	 * Set index of file name
	 * @param index index of selection
	 */
	void setRunIndex(int index);

	/**
	 * Update progress
	 */
	void UpdateProgress();

	/**
	 * starts Acquisition
	 */
	void startAcquisition();

    /**
     * stops Acquisition
     */
    void stopAcquisition();

	/**
	 * Sets the timing mode
	 * @param mode timing mode
	 */
	void SetTimingMode(int mode);

private:
	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up the timing mode
	 */
	void SetupTimingMode();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();

	/**
	 * Get timing mode from detector
	 * @param startup is true when gui has just started up
	 */
	void GetTimingModeFromDetector(bool startup = false);

	/**
	 * Enables/Disables widgetframes to avoid setting measurement during an acquisition
	 */
	void Enable(bool enable);

	/**
	 * Checks if acquisition period is greater than exposure time
	 */
	void CheckAcqPeriodGreaterThanExp();

	/**
	 * Verify if output directory existing error is set
	 * @returns OK or FAIL
	 */
	int VerifyOutputDirectoryError();

	/** The sls detector object */
	multiSlsDetector *myDet;
	/** The Plot widget	 */
	qDrawPlot *myPlot;
	/** detector type */
	slsDetectorDefs::detectorType detType;
	/** enum for the timing mode */
	enum{AUTO, TRIGGER, GATED, BURST_TRIGGER, NUM_TIMING_MODES};
	/** timer to update the progress*/
	QTimer *progressTimer;
	/** tool tip variables*/
	QString 	acqPeriodTip;
	QString 	errPeriodTip;
	QPalette	red;
	/** to access items in settings combobox */
	QStandardItemModel* model;

signals:
	void StartSignal();
	void StopSignal();
	void CheckPlotIntervalSignal();
};

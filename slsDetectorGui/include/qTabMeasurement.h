#pragma once

#include "ui_form_tab_measurement.h"

class qDrawPlot;

class multiSlsDetector;

class QStandardItemModel;

class qTabMeasurement:public QWidget, private Ui::TabMeasurementObject{
	Q_OBJECT

public:
	qTabMeasurement(QWidget *parent, multiSlsDetector* detector, qDrawPlot* plot);
	~qTabMeasurement();

	bool GetStartStatus();
	void ClentStartAcquisition();
	int GetProgress();
	void Refresh();

public slots:
	void SetCurrentMeasurement(int val);
	void UpdateFinished();
    void StopAcquisition();

private slots:
	void SetTimingMode(int mode);
	void SetNumMeasurements(int num);
	void SetNumFrames(int val);
	void SetNumTriggers(int val);
	void SetNumSamples(int val);
	void SetExposureTime();
	void SetAcquisitionPeriod();
	void SetDelay();
	void SetFileWriteEnable(bool enable);
	void SetFileName();
	void SetRunIndex(int val);
	void UpdateProgress();
	void StartAcquisition();

private:
	void SetupWidgetWindow();
	void Initialization();
	void SetupTimingMode();
	void EnableWidgetsforTimingMode();

	void GetTimingMode();
	void GetNumMeasurements();
	void GetNumFrames();
	void GetNumTriggers();
	void GetNumSamples();
	void GetExposureTime();
	void GetAcquisitionPeriod();
	void CheckAcqPeriodGreaterThanExp();
	void GetDelay();
	void GetFileWriteEnable();
	void GetFileName();
	void GetRunIndex();

	void ResetProgress();

	void Enable(bool enable);
	int VerifyOutputDirectoryError();

	multiSlsDetector *myDet;
	qDrawPlot *myPlot;
	// enum for the timing mode
	enum{
		AUTO, 
		TRIGGER, 
		GATED, 
		BURST_TRIGGER, 
		NUMTIMINGMODES
	};
	QTimer *progressTimer;
	//tool tip variables
	QString 	acqPeriodTip;
	QString 	errPeriodTip;
	QPalette	red;
	bool delayImplemented;
	bool sampleImplemented;

signals:
	void StartSignal();
	void StopSignal();
	void CheckPlotIntervalSignal();
};

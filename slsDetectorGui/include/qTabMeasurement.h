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
    void StopAcquisition();
	void UpdateFinished();

private slots:
	void SetTimingMode(int val);
	void SetNumMeasurements(int val);
	void SetNumFrames(int val);
	void SetNumTriggers(int val);
	void SetNumSamples(int val);
	void SetExposureTime();
	void SetAcquisitionPeriod();
	void SetDelay();
	void SetFileWrite(bool val);
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
	void GetFileWrite();
	void GetFileName();
	void GetRunIndex();

	void ResetProgress();

	void Enable(bool enable);
	int VerifyOutputDirectoryError();

signals:
	void StartSignal();
	void StopSignal();

private:
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
};

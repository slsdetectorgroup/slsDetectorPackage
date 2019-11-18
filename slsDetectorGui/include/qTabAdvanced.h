#pragma once

#include "ui_form_tab_advanced.h"

class qDrawPlot;

#include "Detector.h"

class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	qTabAdvanced(QWidget *parent, sls::Detector* detector, qDrawPlot* p);
	~qTabAdvanced();

public slots:
	void Refresh();

private slots:
	void SetDetector();
	void SetControlPort(int port);
	void SetStopPort(int port);
	void SetDetectorUDPIP();
	void SetDetectorUDPMAC();
	void SetCltZMQPort(int port);
	void SetCltZMQIP();
	void SetRxrHostname();
	void SetRxrTCPPort(int port);
	void SetRxrUDPPort(int port);
	void SetRxrUDPIP();
	void SetRxrUDPMAC();
	void SetRxrZMQPort(int port);
	void SetRxrZMQIP();
	void GetROI();
	void ClearROI();
	void SetROI();
	void SetAllTrimbits();
	void SetNumStoragecells(int value);
	void SetSubExposureTime();
	void SetSubDeadTime();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PopulateDetectors();

	void GetControlPort();
	void GetStopPort();
	void GetDetectorUDPIP();
	void GetDetectorUDPMAC();
	void GetCltZMQPort();
	void GetCltZMQIP();
	void GetRxrHostname();
	void GetRxrTCPPort();
	void GetRxrUDPPort();
	void GetRxrUDPIP();
	void GetRxrUDPMAC();
	void GetRxrZMQPort();
	void GetRxrZMQIP();
	void GetAllTrimbits();
	void GetNumStoragecells();
	void GetSubExposureTime();
	void GetSubDeadTime();

	sls::Detector *det;
	qDrawPlot *plot;
};



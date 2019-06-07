#pragma once

#include "ui_form_tab_advanced.h"

class multiSlsDetector;

class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	qTabAdvanced(QWidget *parent, multiSlsDetector* detector);
	~qTabAdvanced();

public slots:
	void Refresh();

private slots:
	void SetDetector(int index);
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
	void AddROISlot();
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

	void GetOnline();
	void GetControlPort();
	void GetStopPort();
	void GetDetectorUDPIP();
	void GetDetectorUDPMAC();
	void GetCltZMQPort();
	void GetCltZMQIP();
	void GetRxrHostname();
	void GetReceiverOnline();
	void GetRxrTCPPort();
	void GetRxrUDPPort();
	void GetRxrUDPIP();
	void GetRxrUDPMAC();
	void GetRxrZMQPort();
	void GetRxrZMQIP();
	void ClearROIWidgets();
	void GetAllTrimbits();
	void GetNumStoragecells();
	void GetSubExposureTime();
	void GetSubDeadTime();

	multiSlsDetector *myDet;
	/** Tool Tip */
	QString		errOnlineTip;
	QString		detOnlineTip;
	QString 	rxrOnlineTip;
	QPalette	red;
	/** ROI */
	std::vector <QLabel*> lblFromX;
	std::vector <QSpinBox*> spinFromX;
	std::vector <QLabel*> lblFromY;
	std::vector <QSpinBox*> spinFromY;
	std::vector <QLabel*> lblToX;
	std::vector <QSpinBox*> spinToX;
	std::vector <QLabel*> lblToY;
	std::vector <QSpinBox*> spinToY;
};



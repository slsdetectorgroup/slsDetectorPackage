#pragma once

#include "qDefs.h"
#include "sls_detector_defs.h"

#include "ui_form_tab_advanced.h"

class multiSlsDetector;
class slsDetector;

class qDrawPlot;

#include <QStackedLayout>
#include <QSpacerItem>
/**
 *@short sets up the advanced parameters
 */
class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabAdvanced(QWidget *parent, multiSlsDetector* detector);

	/**
	 * Destructor
	 */
	~qTabAdvanced();

public slots:
	/**
	 * To refresh and update widgets
	 */
	void Refresh();

private slots:

	/**
	 * Select Readout
	 * @param index position index of readout
	 */
	void SetDetector(int index);

	/**
	 * Sets control port
	 * @param port control port
	 */
	void SetControlPort(int port);

	/**
	 * Sets stop port
	 * @param port stop port
	 */
	void SetStopPort(int port);

	/**
	 * Sets detector udp ip
	 */
	void SetDetectorUDPIP();

	/**
	 * Sets detector udp mac
	 */
	void SetDetectorUDPMAC();
	
	/**
	 * Sets client zmq port
	 * @param port client zmq port
	 */
	void SetCltZMQPort(int port);
	
	/**
	 * Sets client zmq ip to listen to
	 */
	void SetCltZMQIP();

	/**
	 * Sets the receiver hostname
	 */
	void SetRxrHostname();

	/**
	 * Sets receiver tcp port
	 * @param port receiver tcp port
	 */
	void SetRxrTCPPort(int port);

	/**
	 * Sets receiver udp port
	 * @param port receiver udp port
	 */
	void SetRxrUDPPort(int port);

	/**
	 * Sets receiver ip
	 */
	void SetRxrUDPIP();

	/**
	 * Sets reciever mac
	 */
	void SetRxrUDPMAC();

	/**
	 * Sets receiver zmq port
	 * @param port receiver zmq port
	 */
	void SetRxrZMQPort(int port);

	/**
	 * Sets receiver zmq ip to stream from
	 */
	void SetRxrZMQIP();

	/**
	 * Add ROI 
	 */
	void AddROISlot();

	/**
	 * Gets ROIs from detector and updates it
	 */
	void GetROI();

	/**
	 * Clears ROI in detector
	 */
	void ClearROI();

	/**
	 * Sets ROI in detector
	 */
	void SetROI();

	/**
	 * Set all trimbits to a value
	 */
	void SetAllTrimbits();


	/**
	 * Set number of additional storage cells
	 * @param value value to set to
	 */
	void SetNumStoragecells(int value);

	/**
	 * Set sub frame exposure time
	 */
	void SetSubExposureTime();

	/**
	 * Set sub frame dead time
	 */
	void SetSubDeadTime();


private:

	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();

	/** 
	 * Populate detectors
	 */
	void PopulateDetectors();

	/**
	 * Gets detector online
	 */
	void GetOnline();

	/**
	 * Gets control port
	 */
	void GetControlPort();

	/**
	 * Gets stop port
	 */
	void GetStopPort();

	/**
	 * Gets detector udp ip
	 */
	void GetDetectorUDPIP();

	/**
	 * Gets detector udp mac
	 */
	void GetDetectorUDPMAC();

	/**
	 * Gets client zmq receiver port
	 */
	void GetCltZMQPort();

	/**
	 * Gets client zmq ip to listen to
	 */
	void GetCltZMQIP();

	/**
	 * Gets receiver hostname
	 */
	void GetRxrHostname();

	/**
	 * Sets receiver online
	 */
	void GetReceiverOnline();

	/**
	 * Gets receiver tcp port
	 */
	void GetRxrTCPPort();

	/**
	 * Gets receiver udp port
	 */
	void GetRxrUDPPort();

	/**
	 * Gets receiver udp ip
	 */
	void GetRxrUDPIP();

	/**
	 * Gets receiver udp mac
	 */
	void GetRxrUDPMAC();

	/**
	 * Gets receiver zmq transmitting port
	 */
	void GetRxrZMQPort();

	/**
	 * Gets receiver zmq transmitting ip
	 */
	void GetRxrZMQIP();

	/**
	 * Clears all the ROI widgets
	 */
	void ClearROIWidgets();

	/**
	 * Get all trimbits
	 */
	void GetAllTrimbits();

	/**
	 * Get number of additional storage cells
	 */
	void GetNumStoragecells();

	/**
	 * Get sub frame exposure time
	 */
	void GetSubExposureTime();

	/**
	 * Get sub frame dead time
	 */
	void GetSubDeadTime();


	/** The multi detector object */
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



/*
 * qTabAdvanced.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABADVANCED_H_
#define QTABADVANCED_H_

#include "qDefs.h"
#include "sls_detector_defs.h"

/** Form Header */
#include "ui_form_tab_advanced.h"
/** Project Class Headers */
class multiSlsDetector;
class slsDetector;
/** Qt Project Class Headers */
class qDrawPlot;
/** Qt Include Header */
#include <QStackedLayout>
#include <QSpacerItem>
/**
 *@short sets up the advanced parameters
 */
class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 *    @param plot plot object reference
	 */
	qTabAdvanced(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot);

	/** Destructor
	 */
	~qTabAdvanced();

public slots:
	/** To refresh and update widgets
	 */
	void Refresh();

private:
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Add ROI Input
	 * @param num number of inputs to add
	 */
	void AddROIInput(int num);

	/** Checks for a few conditions before trimming
	 /returns OK or FAIL
	 */
	int validateBeforeTrimming();

	/** update the setalltrimbits value from server
	 */
	void updateAllTrimbitsFromServer();


private slots:

	/** Enable/Disable Energy and Calibration Logs
	 */
	void SetLogs();

	/** Set acquisition time
	 */
	void SetExposureTime();

	/** Set the Threshold dac value
	 */
	void SetThreshold();

	/** Set output directory for trimming
	 */
	void SetOutputFile();

	/** Browse output directory for trimming
	 */
	void BrowseOutputFile();

	/** Enables trimming method and calls SetTrimmingMethod if enabled
	 * @param enable to enable trimming
	 */
	void EnableTrimming(bool enable);

	/** Enabling resolution and Counts if this is enabled
	 * @param enable to enable
	 */
	void SetOptimize(bool enable);

	/** Sets the trimming method
	 * @param mode trimming method
	 */
	void SetTrimmingMethod(int mode);

	/** Ensures the right trimming mode and Executes Trimming
	 */
	void StartTrimming();

	/** Updates the plot with trimbits from detector/shared memory
	 */
	void UpdateTrimbitPlot(int id);

	/** Sets control port
	 * @param port control port
	 */
	void SetControlPort(int port);

	/** Sets stop port
	 * @param port stop port
	 */
	void SetStopPort(int port);

	/** Sets receiver tcp port
	 * @param port receiver tcp port
	 */
	void SetRxrTCPPort(int port);

	/** Sets receiver udp port
	 * @param port receiver udp port
	 */
	void SetRxrUDPPort(int port);

	/** Sets receiver online
	 * @param index 1 for online and 0 for offline
	 */
	void SetReceiverOnline(int index);

	/** Sets detector online
	 * @param index 1 for online and 0 for offline
	 */
	void SetOnline(int index);

	/** Sets network parameters like receiver udp ip,
	 * receiver udp mac, detector ip and detector mac
	 */
	void SetNetworkParameters();

	/** Sets the receiver. which also sets the receiver parameters
	 */
	void SetReceiver();

	/** Add ROI Input if the value changed in the last slot
	 */
	void AddROIInputSlot(){AddROIInput(1);};

	/** Clears all the ROI inputs
	 */
	void clearROI();

	/** Gets ROIs from detector and updates it
	 */
	void updateROIList();

	/** Sets ROI in detector
	 */
	void setROI();

	/** Clears ROI in detector
	 */
	void clearROIinDetector();

	/** Clears ROI in detector
	 */
	void SetDetector(int index);

	/** Set all trimbits to a value
	 */
	void SetAllTrimbits();

private:
	/** The multi detector object */
	multiSlsDetector *myDet;

	/** The sls detector object */
	slsDetector *det;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** The Plot widget	 */
	qDrawPlot *myPlot;

	QButtonGroup 	*btnGroup;

	/** Tool Tip for the output dir */
	QString 	outputDirTip;
	QString 	errOutputTip;
	QString		errOnlineTip;
	QString		detOnlineTip;
	QString 	rxrOnlineTip;
	QPalette	red;

	/** Trimming mode */
	slsDetectorDefs::trimMode trimmingMode;
	static const int TRIMMING_DYNAMIC_RANGE 	= 32;
	static const int TRIMMING_FRAME_NUMBER 		= 1;
	static const int TRIMMING_TRIGGER_NUMBER 	= 1;
	static const int TRIMMING_PROBE_NUMBER 		= 0;

	bool isEnergy;
	bool isAngular;

	/** ROI */
	vector <QLabel*> 	lblFromX;
	vector <QSpinBox*> 	spinFromX;
	vector <QLabel*> 	lblFromY;
	vector <QSpinBox*> 	spinFromY;
	vector <QLabel*> 	lblToX;
	vector <QSpinBox*> 	spinToX;
	vector <QLabel*> 	lblToY;
	vector <QSpinBox*> 	spinToY;
	int numRois;




};



#endif /* QTABADVANCED_H_ */

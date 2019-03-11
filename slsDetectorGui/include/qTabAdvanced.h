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

	/** update the setalltrimbits value from server
	 */
	void updateAllTrimbitsFromServer();


private slots:

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

	/** Sets client zmq receiver port
	 * @param port client zmq receiver port
	 */
	void SetCltZmqPort(int port);

	/** Sets receiver zmq transmitting port
	 * @param port receiver zmq transmitting port
	 */
	void SetRxrZmqPort(int port);

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

	/** Sets client zmq ip to listen to
	 */
	void SetClientZMQIP();

	/** Sets receiver zmq ip to stream from
	 */
	void SetReceiverZMQIP();

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

	/** Set storage cells */
	void SetNumStoragecells(int value);

	/** Set sub frame exposure time */
	void SetSubExposureTime();

	/** Set sub frame period */
	void SetSubPeriod();


private:
	/** The multi detector object */
	multiSlsDetector *myDet;

	/** The sls detector object */
	// slsDetector *det;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** The Plot widget	 */
	qDrawPlot *myPlot;

	QButtonGroup 	*btnGroup;

	/** Tool Tip */
	QString		errOnlineTip;
	QString		detOnlineTip;
	QString 	rxrOnlineTip;
	QPalette	red;

	bool isEnergy;

	/** ROI */
	std::vector <QLabel*> 	lblFromX;
	std::vector <QSpinBox*> 	spinFromX;
	std::vector <QLabel*> 	lblFromY;
	std::vector <QSpinBox*> 	spinFromY;
	std::vector <QLabel*> 	lblToX;
	std::vector <QSpinBox*> 	spinToX;
	std::vector <QLabel*> 	lblToY;
	std::vector <QSpinBox*> 	spinToY;
	int numRois;

	/** sub period tool tip variables*/
	QString 	acqSubPeriodTip;
	QString 	errSubPeriodTip;

	void CheckAcqPeriodGreaterThanExp();


};



#endif /* QTABADVANCED_H_ */

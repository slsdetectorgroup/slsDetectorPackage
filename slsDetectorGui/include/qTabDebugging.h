/*
 * qTabDebugging.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDEBUGGING_H_
#define QTABDEBUGGING_H_

#include "qDefs.h"


/** Form Header */
#include "ui_form_tab_debugging.h"
/** Project Class Headers */
class multiSlsDetector;
class slsDetector;
/** Qt Include Headers */
#include <QTreeWidget>



/**
 *@short sets up the Debugging parameters
 */
class qTabDebugging:public QWidget, private Ui::TabDebuggingObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDebugging(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDebugging();

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



private slots:
	/** Updates the module list depending on current detector
 	*/
	void UpdateModuleList();

	/** Updates the status depending on current detector
 	*/
	void UpdateStatus();

	/** Gets id and versions etc
 	*/
	void GetInfo();

	/** Sets id and versions on the display widget
 	*/
	void SetParameters(QTreeWidgetItem *item);

	/** Test detector and module
 	*/
	void TestDetector();

private:
	/** The multi sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/**sls detecctor object */
	slsDetector *det;

	/** Tree Widget displaying the detectors, modules */
	QTreeWidget *treeDet;
	/** Widget displaying the serial numbers, mac addresses etc */
	QFrame *dispFrame;
	QLabel *lblDetectorId;
	QLabel *lblDetectorSerial;
	QLabel *lblDetectorFirmware;
	QLabel *lblDetectorSoftware;
	QLabel *lblModuleId;
	QLabel *lblModuleFirmware;
	QLabel *lblModuleSerial;
	QPalette *blue;
};



#endif /* QTABDEBUGGING_H_ */

#pragma once

#include "qDefs.h"

#include "ui_form_tab_debugging.h"

class multiSlsDetector;
class slsDetector;

#include <QTreeWidget>


/**
 *@short sets up the Debugging parameters
 */
class qTabDebugging:public QWidget, private Ui::TabDebuggingObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabDebugging(QWidget *parent, multiSlsDetector*& detector);

	/**
	 * Destructor
	 */
	~qTabDebugging();

	/**
	 * To refresh and update widgets
	 */
	void Refresh();


private slots:

		/**
		 * Updates the status depending on current detector
	 	*/
		void UpdateStatus();

		/**
		 * Gets id and versions etc
	 	*/
		void
		GetInfo();

		/**
		 * Sets id and versions on the display widget
	 	*/
		void SetParameters(QTreeWidgetItem *item);

		/**
		 * Test detector
	 	*/
		void TestDetector();

private:
	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();


	/** The multi sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** Tree Widget displaying the detectors, modules */
	QTreeWidget *treeDet;
	/** Widget displaying the serial numbers, mac addresses etc */
	QLabel *lblDetectorId;
	QLabel *lblDetectorFirmware;
	QLabel *lblDetectorSoftware;
	QPalette *blue;
};


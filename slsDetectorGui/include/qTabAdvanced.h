/*
 * qTabAdvanced.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABADVANCED_H_
#define QTABADVANCED_H_

/** Form Header */
#include "ui_form_tab_advanced.h"
/** Project Class Headers */
class multiSlsDetector;
#include "sls_detector_defs.h"
/** Qt Include Header */
#include <QStackedLayout>

/**
 *@short sets up the advanced parameters
 */
class qTabAdvanced:public QWidget, private Ui::TabAdvancedObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabAdvanced(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabAdvanced();

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

private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** Tool Tip for the output dir */
	QString 	outputDirTip;
	QString 	errOutputTip;
	QPalette	red;

	/** Trimming mode */
	slsDetectorDefs::trimMode trimmingMode;


};



#endif /* QTABADVANCED_H_ */

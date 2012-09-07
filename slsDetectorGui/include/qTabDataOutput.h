/*
 * qTabDataOutput.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDATAOUTPUT_H_
#define QTABDATAOUTPUT_H_

/** Form Header */
#include "ui_form_tab_dataoutput.h"
/** Project Class Headers */
class multiSlsDetector;
#include "sls_detector_defs.h"
/** Qt Include Headers */
#include <QString>


/**
 *@short sets up the DataOutput parameters
 */
class qTabDataOutput:public QWidget, private Ui::TabDataOutputObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabDataOutput(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabDataOutput();

	/** To refresh and update widgets
	 */
	void Refresh();



private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	QString 	flatFieldTip;
	QString 	errFlatFieldTip;
	QPalette	red;

/** methods */
	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();


private slots:
/** Sets the output directory
 * @param path output path to be set
 */
void setOutputDir(const QString& path);

/** Open dialog to choose the output directory */
void browseOutputDir();

/**set flat field file*/
void SetFlatField();

/** update flat field correction from server */
void UpdateFlatFieldFromServer();

/**browse flat field*/
void BrowseFlatFieldPath();

/**rate correction*/
void SetRateCorrection();

/** update rate correction from server */
void UpdateRateCorrectionFromServer();

/**angular correction*/
void SetAngularCorrection();

/**discard bad channels*/
void DiscardBadChannels();

signals:
/**signal to enable/disable positions in Actions*/
void AngularConversionSignal(bool);
};



#endif /* QTABDATAOUTPUT_H_ */

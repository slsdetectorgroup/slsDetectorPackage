/*
 * qTabDataOutput.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABDATAOUTPUT_H_
#define QTABDATAOUTPUT_H_

#include "qDefs.h"


/** Form Header */
#include "ui_form_tab_dataoutput.h"
/** Project Class Headers */
class multiSlsDetector;
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

	/** verify output directories
	 *  /returns success or fail
	 */
	int VerifyOutputDirectory();

	/** To enable expert mode
	 * @param enable to enable if true
	 */
	void SetExpertMode(bool enable);


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;


	QString 	flatFieldTip;
	QString 	errFlatFieldTip;
	QString 	outDirTip;
	QPalette	red;
	QPalette	black;
	QPalette	*red1;
	QPalette	*black1;

	/** enum for the Eiger clock divider */
	enum {FullSpeed, HalfSpeed, QuarterSpeed, SuperSlowSpeed, NumberofSpeeds};
	/** enum for the Eiger readout flags1 */
	enum {Continous, Storeinram};
	/** enum for the Eiger readout flags2 */
	enum {Parallel, NonParallel, Safe};


/** methods */
	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/** Populate the readouts
	 */
	void PopulateDetectors();

	/** Get Compression */
	void GetCompression();

	/** update speed */
	void updateSpeedFromServer();

	/** update flags */
	void updateFlagsFromServer();

private slots:

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

/** set output directory*/
void SetOutputDir();

/** set output directory*/
void GetOutputDir();

/** set compression */
void SetCompression(bool enable);

/** enable 10GbE */
void EnableTenGigabitEthernet(bool enable, int get=0);

/** set speed */
void setSpeed();

/** set flags */
void setFlags();

signals:
/**signal to enable/disable positions in Actions*/
void AngularConversionSignal(bool);
};



#endif /* QTABDATAOUTPUT_H_ */

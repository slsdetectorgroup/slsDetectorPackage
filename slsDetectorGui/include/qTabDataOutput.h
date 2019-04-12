#pragma once

#include "qDefs.h"

#include "ui_form_tab_dataoutput.h"

class multiSlsDetector;

#include <QString>


/**
 *@short sets up the DataOutput parameters
 */
class qTabDataOutput:public QWidget, private Ui::TabDataOutputObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabDataOutput(QWidget *parent,multiSlsDetector* detector);

	/**
	 * Destructor
	 */
	~qTabDataOutput();

	/**
	 * To refresh and update widgets
	 */
	void Refresh();

	/**
	 * Verify output directories
	 * @returns success or fail
	 */
	int VerifyOutputDirectory();


	private slots:

	/**
	 * Open dialog to choose the output directory
	 */
	void BrowseOutputDir();

	/**
	 * Set output directory
	 */
	void SetOutputDir();

	/**
	 * Get output directory
	 */
	void GetOutputDir();

	/**
	 * Set rate correction
	 */
	void SetRateCorrection(int deadtime=0);

	/**
	 * Set default rate correction
	 */
	void SetDefaultRateCorrection();

	/**
	 * Set update rate correction from server
	 */
	void UpdateRateCorrectionFromServer();

	/**
	 * Enable/Disable 10GbE
	 */
	void EnableTenGigabitEthernet(bool enable, int get=0);

	/**
	 * Set speed
	 */
	void SetSpeed();

	/**
	 * Set flags
	 */
	void SetFlags();

	/**
	 * Set file format
	 */
	void SetFileFormat(int format);

	/**
	 * Set overwrite enable
	 */
	void SetOverwriteEnable(bool enable);


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
	 * Populate the readouts
	 */
	void PopulateDetectors();

	/**
	 * Update speed
	 */
	void UpdateSpeedFromServer();

	/**
	 * Update flags
	 */
	void UpdateFlagsFromServer();

	/**
	 * Update file format
	 */
	void UpdateFileFormatFromServer();

	/**
	 * Update overwrite enable
	 */
	void UpdateFileOverwriteFromServer();


	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	QString 	outDirTip;
	QPalette	red;
	QPalette	black;
	QPalette	*red1;
	QPalette	*black1;

	/** enum for the Eiger clock divider */
	enum {FULLSPEED, HALFSPEED, QUARTERSPEED, SUPERSLOWSPEED, NUMBEROFSPEEDS};
	/** enum for the Eiger readout flags1 */
	enum {CONTINUOUS, STOREINRAM};
	/** enum for the Eiger readout flags2 */
	enum {PARALLEL, NONPARALLEL, SAFE};

};


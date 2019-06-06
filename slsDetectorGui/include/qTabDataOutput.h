#pragma once

#include "qDefs.h"

#include "ui_form_tab_dataoutput.h"

class multiSlsDetector;

#include <QString>
#include <QButtonGroup>

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

	private slots:

	/**
	 * Get output directory
	 */
	void GetOutputDir();

	/**
	 * Open dialog to choose the output directory
	 */
	void BrowseOutputDir();

	/**
	 * Set output directory
	 */
	void SetOutputDir();

	/**
	 * Set file format
	 * @param format file format
	 */
	void SetFileFormat(int format);

	/**
	 * Set overwrite enable
	 * @param enable enable
	 */
	void SetOverwriteEnable(bool enable);

	/**
	 * Enable/Disable 10GbE
	 * @param enable enable
	 */
	void SetTenGigaEnable(bool enable);

	/**
	 * Enable rate correction
	 */
	void EnableRateCorrection();

	/**
	 * Set rate correction
	 */
	void SetRateCorrection();

	/**
	 * Set speed
	 * @param speed speed chosen
	 */
	void SetSpeed(int speed);

	/**
	 * Set flags
	 */
	void SetFlags();

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
	 * Enable browse
	 */
	void EnableBrowse();

	/**
	 * Get file format
	 */
	void GetFileFormat();

	/**
	 * Get overwrite enable
	 */
	void GetFileOverwrite();

	/**
	 * Get Ten Giga Enable
	 */
	void GetTenGigaEnable();

	/**
	 * Set Get rate correction
	 */
	void GetRateCorrection();

	/**
	 * Get speed
	 */
	void GetSpeed();

	/**
	 * Get flags
	 */
	void GetFlags();

	/** The sls detector object */
	multiSlsDetector *myDet;

	/** Button group for radiobuttons for rate*/
	QButtonGroup *btnGroupRate;

	/** enum for the Eiger clock divider */
	enum {
		FULLSPEED,
		HALFSPEED,
		QUARTERSPEED,
		NUMBEROFSPEEDS
	};

	/** enum for the Eiger readout flags1 */
	enum { 
		CONTINUOUS, 
		STOREINRAM 
	};

	/** enum for the Eiger readout flags2 */
	enum { 
		PARALLEL, 
		NONPARALLEL
	};
};


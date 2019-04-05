#pragma once

#include "qDefs.h"

#include "ui_form_tab_settings.h"

class multiSlsDetector;

/**
 *@short sets up the Settings parameters
 */
class qTabSettings:public QWidget, private Ui::TabSettingsObject{
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabSettings(QWidget *parent,multiSlsDetector*& detector);

	/**
	 * Destructor
	 */
	~qTabSettings();

	/**
	 * Refresh and update widgets
	 */
	void Refresh();


private slots:
	/**
	 * Set settings according to selection
	 *  @param index index of selection
	 */
	void SetSettings(int index);

	/**
	 * Set dynamic range if possible
	 *  @param index selection
	 */
	void SetDynamicRange(int index);

	/**
	 * Set threshold energy
	 */
	void SetEnergy();


private:

	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up the detector settings
	 */
	void SetupDetectorSettings();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();

	/**
	 * Get Settings
	 */
	void GetSettings();

	/**
	 * Gets the dynamic range and sets it on the gui
	 */
	void GetDynamicRange();

	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	enum{STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, LOWNOISE,
		DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2, VERLOWGAIN,
		UNDEFINED, UNINITIALIZED, NUMSETTINGS};
};

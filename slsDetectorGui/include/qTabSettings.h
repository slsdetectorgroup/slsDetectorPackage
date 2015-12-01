/*
 * qTabSettings.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABSETTINGS_H_
#define QTABSETTINGS_H_

#include "qDefs.h"


/** Form Header */
#include "ui_form_tab_settings.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QStandardItemModel>

/**
 *@short sets up the Settings parameters
 */
class qTabSettings:public QWidget, private Ui::TabSettingsObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qTabSettings(QWidget *parent,multiSlsDetector*& detector);

	/** Destructor
	 */
	~qTabSettings();

	/** To refresh and update widgets
	 */
	void Refresh();

	/** To enable expert mode
	 * @param enable to enable if true
	 */
	void SetExpertMode(bool enable){expertMode =  enable;};




private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;

	/** expert mode */
	bool expertMode;

	enum{Standard,Fast,HighGain,DynamicGain,LowGain,MediumGain,VeryHighGain,LowNoise,
		DynamicHG0,FixGain1,FixGain2,ForceSwitchG1,ForceSwitchG2,
		Undefined,Uninitialized,NumSettings};

	/** To be able to index items on a combo box */
	QStandardItemModel* model;
	QModelIndex index[NumSettings];
	QStandardItem* item[NumSettings];

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up the detector settings
	 */
	void SetupDetectorSettings();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Gets the dynamic range and sets it on the gui
	 * @param setvalue the value set by the gui when used as a check
	 */
	void GetDynamicRange(int setvalue = -1);



private slots:
/** Set settings according to selection
 *  @param index index of selection
 */
void setSettings(int index);

/** Set number of modules if possible
 *  @param index number of modules
 */
void SetNumberOfModules(int index);

/** Set dynamic range if possible
 *  @param index selection
 */
void SetDynamicRange(int index);

/** Set threshold energy
 */
void SetEnergy();


signals:
/**Update Trimbits after Set Settings */
void UpdateTrimbitSignal(int);
};



#endif /* QTABSETTINGS_H_ */

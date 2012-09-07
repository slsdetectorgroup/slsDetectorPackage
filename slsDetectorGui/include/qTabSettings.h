/*
 * qTabSettings.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABSETTINGS_H_
#define QTABSETTINGS_H_

/** Form Header */
#include "ui_form_tab_settings.h"
/** Project Class Headers */
class multiSlsDetector;
#include "sls_detector_defs.h"
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


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** detector type */
	slsDetectorDefs::detectorType detType;



	enum{Standard,Fast,HighGain,DynamicGain,LowGain,MediumGain,VeryHighGain,Undefined,Uninitialized,NumSettings};

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
};



#endif /* QTABSETTINGS_H_ */

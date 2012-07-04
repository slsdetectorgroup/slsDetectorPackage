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
class slsDetectorUtils;
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
	 *    @param detID is the id of the detector
	 */
	qTabSettings(QWidget *parent,slsDetectorUtils*& detector,int detID);

	/** Destructor
	 */
	~qTabSettings();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;
	/**etector id */
	int detID;
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

};



#endif /* QTABSETTINGS_H_ */

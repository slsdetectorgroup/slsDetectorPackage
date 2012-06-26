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


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** sls detector id */
	int detID;


	/** Sets up the widget
	 */
	void SetupWidgetWindow();

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

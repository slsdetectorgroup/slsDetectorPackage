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
class slsDetectorUtils;
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
	qTabDataOutput(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qTabDataOutput();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** Output Directory */
	QString outputDir;

/** methods */
	/** Sets up the widget */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/** Enables/Disables all the widgets */
	void Enable(bool enable);


private slots:
/** Sets the output directory
 * @param path output path to be set
 */
void setOutputDir(const QString& path);

/** Open dialog to choose the output directory */
void browseOutputDir();
};



#endif /* QTABDATAOUTPUT_H_ */

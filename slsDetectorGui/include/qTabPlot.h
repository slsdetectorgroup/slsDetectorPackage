/*
 * qTabPlot.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABPLOT_H_
#define QTABPLOT_H_

/** Form Header */
#include "ui_form_tab_plot.h"
/** Project Class Headers */
class slsDetectorUtils;
/** Qt Project Class Headers */
class qDrawPlot;

/**
 *@short sets up the Plot parameters
 */
class qTabPlot:public QWidget, private Ui::TabPlotObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 *    @param plot plot object reference
	 */
	qTabPlot(QWidget *parent,slsDetectorUtils*& detector, qDrawPlot*& plot);

	/** Destructor
	 */
	~qTabPlot();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** The Plot widget	 */
	qDrawPlot *myPlot;


/** methods */
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);



public slots:





private slots:
signals:


};



#endif /* QTABPLOT_H_ */

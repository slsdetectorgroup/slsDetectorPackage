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
/** Qt Include Headers */
#include <QStackedLayout>

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

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** The Plot widget	 */
	qDrawPlot *myPlot;

	/** 1d/2d plot	 */
	bool isOneD;

	QStackedLayout* stackedLayout;
	QSpinBox *spinNthFrame;
	QDoubleSpinBox *spinTimeGap;
	QComboBox *comboTimeGapUnit;

	/** some Default Values */
	static QString defaultPlotTitle;
	static QString defaultHistXAxisTitle;
	static QString defaultHistYAxisTitle;
	static QString defaultImageXAxisTitle;
	static QString defaultImageYAxisTitle;
	static QString defaultImageZAxisTitle;

/** methods */
	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();




public slots:
/** Set frequency between plots
 * returns 0 if there were no errors(important
 * while editing acquisition period in measurement tab) */
int SetFrequency();




private slots:
/** Selects the plot to display, enables/disables widgets
 * @param b true to select plot dimension 1, else false to select 2D
 */
void Select1DPlot(bool b);

/**Enables Persistency depending on Superimpose checkbox */
void EnablePersistency(bool enable);

/**Sets the titles in plot axis */
void SetTitles();
/** Enables/Sets default Titles to default */
void EnableTitles();
/** Enables range of the axes */
void EnableRange();
/** Sets the range of the x and y axes */
void SetAxesRange();
/** Sets the range of the z axis */
void SetZRange();
/** Enables the range of the z axis */
void EnableZRange();
/** Set Plot to none, data graph, histogram*/
void SetPlot();


signals:
void DisableZoomSignal(bool);
void SetZRangeSignal(double,double);
void EnableZRangeSignal(bool);
};



#endif /* QTABPLOT_H_ */

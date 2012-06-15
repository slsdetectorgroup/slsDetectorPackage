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

	/** Enables/Disables all the widgets
	 */
	void Enable(bool enable);





public slots:





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
/** Sets the range of the axes */
void SetAxesRange();

/** Save Plot */
void SavePlot();


signals:
void DisableZoomSignal(bool);

};



#endif /* QTABPLOT_H_ */

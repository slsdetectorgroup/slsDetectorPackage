/*
 * qTabPlot.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QTABPLOT_H_
#define QTABPLOT_H_

#include "qDefs.h"


/** Form Header */
#include "ui_form_tab_plot.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Project Class Headers */
class qDrawPlot;
/** Qt Include Headers */
#include <QStackedLayout>
#include <QButtonGroup>
#include <QAbstractButton>


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
	qTabPlot(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot);

	/** Destructor
	 */
	~qTabPlot();

	/** Sets the scan argument of the plot
	 */
	void SetScanArgument();

	/** To refresh and update widgets
	 */
	void Refresh();


private:
	/** The sls detector object */
	multiSlsDetector *myDet;

	/** The Plot widget	 */
	qDrawPlot *myPlot;


	/** 1d/2d plot	 */
	bool isOneD;
	bool isOriginallyOneD;

	/**is set if its a possible wrong interval between plots*/
	bool wrongInterval;

	QStackedLayout	*stackedLayout;
	QSpinBox 		*spinNthFrame;
	QDoubleSpinBox 	*spinTimeGap;
	QComboBox 		*comboTimeGapUnit;
	QButtonGroup 	*btnGroupScan;
	QButtonGroup	*btnGroupPlotType;
	QButtonGroup	*btnGroupHistogram;

	/** some Default Values */
	static QString defaultPlotTitle;
	static QString defaultHistXAxisTitle;
	static QString defaultHistYAxisTitle;
	static QString defaultImageXAxisTitle;
	static QString defaultImageYAxisTitle;
	static QString defaultImageZAxisTitle;

	/** scans */
	static const QString modeNames[5];

	/** error palette*/
	QPalette *red;
	QString intervalTip;




	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 */
	void Initialization();


public slots:
	/** Set frequency between plots*/
	void SetFrequency();

	/** Enable Scan box
	 */
	void EnableScanBox();


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

	/** Change pages in plot options box to the right*/
	void SetPlotOptionsRightPage();

	/** Change pages in plot options box to the left*/
	void SetPlotOptionsLeftPage();

	/** Plot binary plot */
	void SetBinary();

	/** Set histogram options */
	void SetHistogramOptions();

signals:
	void DisableZoomSignal(bool);
	void SetZRangeSignal(double,double);
	void ResetZMinZMaxSignal(bool,bool,double,double);

};



#endif /* QTABPLOT_H_ */

#pragma once

#include "ui_form_tab_plot.h"

class qDrawPlot;

class multiSlsDetector;

class QStackedLayout;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QButtonGroup;
class QPalette;

class qTabPlot:public QWidget, private Ui::TabPlotObject{
	Q_OBJECT

public:
	qTabPlot(QWidget *parent,multiSlsDetector* detector, qDrawPlot* plot);
	~qTabPlot();
	void SetScanArgument();
	void Refresh();

public slots:
	void SetFrequency();
	 void EnableScanBox();

private slots:
	void Select1DPlot(bool b);

	/**Enables Persistency depending on Superimpose checkbox */
	void EnablePersistency(bool enable);
	void SetTitles();
	void EnableTitles();
	void checkAspectRatio();

	/** maintain aspect ratio
	 * @param axis axis to be changed: 0 for x(y axis values changed), 1 for y (xaxis values changes), -1 for the larger one (aspect ratio checked)
	 */
	void maintainAspectRatio(int axis);
	void EnableXRange();
	void EnableYRange();
	void EnableRange();
	void SetXAxisRange();
	void SetYAxisRange();
	void SetAxesRange();
	void SetZRange();
	void EnableZRange();
	bool CheckZRange(QString value);
	void SetPlot();
	void SetPlotOptionsRightPage();
	void SetPlotOptionsLeftPage();
	void SetBinary();
	void SetHistogramOptions();
	void EnableGapPixels(bool enable);

signals:
	void DisableZoomSignal(bool);
	void ResetZMinZMaxSignal(bool,bool,double,double);

private:
	void SetupWidgetWindow();
	void Initialization();

	multiSlsDetector *myDet;
	qDrawPlot *myPlot;
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
	QPalette 		*red;
	QString 		intervalTip;

	/** some Default Values */
	static QString defaultPlotTitle;
	static QString defaultHistXAxisTitle;
	static QString defaultHistYAxisTitle;
	static QString defaultImageXAxisTitle;
	static QString defaultImageYAxisTitle;
	static QString defaultImageZAxisTitle;

	/** scans */
	static const QString modeNames[5];

};



#pragma once

#include "ui_form_tab_plot.h"

class qDrawPlot;

#include "Detector.h"

class QButtonGroup;

class qTabPlot:public QWidget, private Ui::TabPlotObject{
	Q_OBJECT

public:
	qTabPlot(QWidget *parent, sls::Detector* detector, qDrawPlot* p);
	~qTabPlot();
	void SetScanArgument();
	void Refresh();

private slots:
	void SetPlot();
	void Set1DPlotOptionsRight();
	void Set1DPlotOptionsLeft();
	void Set2DPlotOptionsRight();
	void Set2DPlotOptionsLeft();
	void EnablePersistency(bool enable);
	void SetBinary();
	void SetGapPixels(bool enable);
	void SetTitles();
	void SetXRange();
	void SetYRange();
	void CheckAspectRatio();
	void SetZRange();
	void SetStreamingFrequency();

signals:
	void DisableZoomSignal(bool);

private:
	void SetupWidgetWindow();
	void Initialization();
	void Select1DPlot(bool enable);
	void GetGapPixels();
	void GetStreamingFrequency();
	void SetXYRange();
	void MaintainAspectRatio(int dimension);

	sls::Detector *det;
	qDrawPlot *plot;
	bool is1d;

	QButtonGroup	*btnGroupPlotType{nullptr};

	/** default plot and axis titles */
	static QString defaultPlotTitle;
	static QString defaultHistXAxisTitle;
	static QString defaultHistYAxisTitle;
	static QString defaultImageXAxisTitle;
	static QString defaultImageYAxisTitle;
	static QString defaultImageZAxisTitle;
};



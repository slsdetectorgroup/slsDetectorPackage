#include <iostream>

#include <qtoolbutton.h>
#include <qgroupbox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <QString>

#include "SlsQt2DPlotLayout.h"



SlsQt2DPlotLayout::SlsQt2DPlotLayout(QWidget *parent):QGroupBox(parent){
	the_layout=0;
	the_plot   = new SlsQt2DPlot(this);
	isLog = false;
	zmin = 0;
  zmax = 0;
  isZmin = false;
  isZmax = false;
	Layout();
	ConnectSignalsAndSlots();
}

SlsQt2DPlotLayout::~SlsQt2DPlotLayout(){
	if(the_layout) delete the_layout;
	delete the_plot;
}

void SlsQt2DPlotLayout::Layout(){
	if(the_layout) delete the_layout;
	the_layout =  new QGridLayout(this);
	the_layout->addWidget(the_plot,2,0,3,3);
}

void SlsQt2DPlotLayout::ConnectSignalsAndSlots(){
	connect(this, SIGNAL(InterpolateSignal(bool)), the_plot,	SLOT(InterpolatedPlot(bool)));
	connect(this,	SIGNAL(ContourSignal(bool)), the_plot,	SLOT(showContour(bool)));
	connect(this, SIGNAL(LogzSignal(bool)), this, SLOT(SetZScaleToLog(bool)));
}

void SlsQt2DPlotLayout::SetZRange(bool isMin, bool isMax, double min, double max){
	isZMin = isMin;
	isZmax = isMax;

	// reset zmin and zmax first (recalculate from plot)
	the_plot->SetZMinMax();

	UpdateZRange(min, max);
}

void SlsQt2DPlotLayout::UpdateZRange(double min, double max) {
	if(isLog) {
		the_plot->SetZMinimumToFirstGreaterThanZero();
	}

	// set zmin and zmax
	if (isZmin || isZmax) {
		zmin = (isZmin ? min : the_plot->GetZMinimum());
		zmax = (isZmax ? max : the_plot->GetZMaximum());
		// if it is the same values, we should reset it to plots min and max (not doing this now: not foolproof now)
		// setting the range of values possible in the dispZMin and dispZMax (not doin this now: not foolproof)
		the_plot->SetZMinMax(zmin, zmax);
	} else {
		zmin = 0;
		zmax = -1;
	}
	
	the_plot->Update();
}

void SlsQt2DPlotLayout::SetZScaleToLog(bool enable) {
	FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling")  << " Z Scale to log";
	isLog = enable;
	the_plot->LogZ(enable);
	SetZRange(isZMin, isZmax, zmin, zmax);
}


void SlsQt2DPlotLayout::SetXTitle(QString st){
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::xBottom)->setTitle(title);
}

void SlsQt2DPlotLayout::SetYTitle(QString st){
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::yLeft)->setTitle(title);
}

void SlsQt2DPlotLayout::SetZTitle(QString st){
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::yRight)->setTitle(title);
}

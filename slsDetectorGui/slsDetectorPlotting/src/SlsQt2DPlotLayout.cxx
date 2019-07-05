#include "SlsQt2DPlotLayout.h"
#include "logger.h"

#include <iostream>
#include "ansi.h"
#include <qtoolbutton.h>
#include <qgroupbox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <QString>

SlsQt2DPlotLayout::SlsQt2DPlotLayout(QWidget *parent):QGroupBox(parent){
	the_layout=0;
	the_plot   = new SlsQt2DPlot(this);
	isLog = false;
	Layout();
}

SlsQt2DPlotLayout::~SlsQt2DPlotLayout(){
	if(the_layout) delete the_layout;
	delete the_plot;
}

SlsQt2DPlot* SlsQt2DPlotLayout::GetPlot(){
	return the_plot;
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

void SlsQt2DPlotLayout::SetInterpolate(bool enable) {
	the_plot->InterpolatedPlot(enable);
}

void SlsQt2DPlotLayout::SetContour(bool enable) {
	the_plot->showContour(enable);
}

void SlsQt2DPlotLayout::SetLogz(bool enable, bool isMin, bool isMax, double min, double max) {
	isLog = enable;
	the_plot->LogZ(enable);
	SetZRange(isMin, isMax, min, max);
}

void SlsQt2DPlotLayout::Layout(){
	if(the_layout) delete the_layout;
	the_layout =  new QGridLayout(this);
	the_layout->addWidget(the_plot,2,0,3,3);
}

void SlsQt2DPlotLayout::SetZRange(bool isMin, bool isMax, double min, double max){
	if(isLog) {
		the_plot->SetZMinimumToFirstGreaterThanZero();
	}

	// set zmin and zmax
	if (isMin || isMax) {
		double zmin = (isMin ? min : the_plot->GetZMinimum());
		double zmax = (isMax ? max : the_plot->GetZMaximum());
		the_plot->SetZMinMax(zmin, zmax);
	} 

	the_plot->Update();

}


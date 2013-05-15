/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>

#include <qtoolbutton.h>
#include <qgroupbox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <QString>

#include "SlsQt2DPlotLayout.h"

using namespace std;

SlsQt2DPlotLayout::SlsQt2DPlotLayout(QWidget *parent):QGroupBox(parent){
	the_layout=0;
	the_plot   = new SlsQt2DPlot(this);

	z_range_ne = new SlsQtNumberEntry(this,1,"Set the z axis range from",2,"to",2);
	z_range_ne->setFixedWidth(402);
#ifndef IAN
	zRangeChecked = false;
	z_range_ne->hide();
#endif

	CreateTheButtons();
	Layout();
	ConnectSignalsAndSlots();
}

SlsQt2DPlotLayout::~SlsQt2DPlotLayout(){

	if(the_layout) delete the_layout;


	delete the_plot;
	delete z_range_ne;
}


void SlsQt2DPlotLayout::CreateTheButtons(){
	/** Dhanya: All these buttons are already in another class, logz is used and a wrapper around it*/
#ifdef IAN
  btnInterpolate = new QToolButton(this);
    btnInterpolate->setText("Interpolate");
    btnInterpolate->setCheckable(true);
    btnInterpolate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  btnContour = new QToolButton(this);
    btnContour->setText("Contour");
    btnContour->setCheckable(true);
    btnContour->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif
  btnLogz = new QToolButton(this);
    btnLogz->setText("Log Scale (Z)");
    btnLogz->setCheckable(true);
    btnLogz->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#ifndef IAN
    btnLogz->hide();
#endif
}


void SlsQt2DPlotLayout::Layout(){
	if(the_layout) delete the_layout;
	the_layout =  new QGridLayout(this);
#ifdef IAN
    the_layout->addWidget(btnInterpolate,1,1);
    the_layout->addWidget(btnContour,1,2);
    the_layout->addWidget(btnLogz,1,3);
	the_layout->addWidget(the_plot,2,1,3,3);
	the_layout->addWidget(z_range_ne,5,1,1,3);
	the_layout->setMargin(12);
#else
	the_layout->addWidget(the_plot,2,0,3,3);
	the_layout->addWidget(z_range_ne,5,0,1,3);
#endif
}

void SlsQt2DPlotLayout::ConnectSignalsAndSlots(){
#ifndef IAN
	connect(this,		SIGNAL(InterpolateSignal(bool)),			the_plot,	SLOT(InterpolatedPlot(bool)));
	connect(this,		SIGNAL(ContourSignal(bool)),				the_plot,	SLOT(showContour(bool)));
#else
    connect(btnInterpolate, SIGNAL(toggled(bool)),the_plot, SLOT(InterpolatedPlot(bool)));
    connect(btnContour, SIGNAL(toggled(bool)),the_plot, SLOT(showContour(bool)));
    connect(btnLogz, SIGNAL(toggled(bool)),this,SLOT(SetZScaleToLog(bool)));
    btnInterpolate->setChecked(false);
    btnContour->setChecked(false);
#endif
	connect(z_range_ne,	SIGNAL(CheckBoxChanged(bool)),				this,		SLOT(ResetRange()));
	connect(z_range_ne,	SIGNAL(AValueChanged(SlsQtNumberEntry*)),	this,		SLOT(ResetRange()));
	btnLogz->setChecked(false);
}

void SlsQt2DPlotLayout::UpdateNKeepSetRangeIfSet(){
#ifdef IAN
	if(z_range_ne->CheckBoxState()){
#endif
		//just reset histogram range before update
		the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));
#ifdef IAN
	}
#endif
	the_plot->Update();
}

void SlsQt2DPlotLayout::ResetRange(){
	//refind z limits
	the_plot->SetZMinMax();
	if(btnLogz->isChecked()) the_plot->SetZMinimumToFirstGreaterThanZero();
#ifdef IAN
	if(z_range_ne->CheckBoxState()){
#else
	if(zRangeChecked){
#endif
		//first time check validity
		bool same = (z_range_ne->GetValue(0)==z_range_ne->GetValue(1)) ? 1:0;
		if(!z_range_ne->IsValueOk(0)||same) z_range_ne->SetValue(the_plot->GetZMinimum(),0);
		if(!z_range_ne->IsValueOk(1)||same) z_range_ne->SetValue(the_plot->GetZMaximum(),1);
		z_range_ne->SetRange(the_plot->GetZMinimum(),z_range_ne->GetValue(1),0);
		z_range_ne->SetRange(z_range_ne->GetValue(0),the_plot->GetZMaximum(),1);

		//set histogram range
		the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));
	}
	the_plot->Update();
}


void SlsQt2DPlotLayout::ResetZMinZMax(bool zmin, bool zmax, double min, double max){
	z_range_ne->SetNumber(min,0);
	z_range_ne->SetNumber(max,1);

	//refind z limits
	the_plot->SetZMinMax();
	if(btnLogz->isChecked()) the_plot->SetZMinimumToFirstGreaterThanZero();

	//first time check validity
	if(zmax)			z_range_ne->SetValue(max,0);
	else				z_range_ne->SetValue(the_plot->GetZMaximum(),1);

	if(zmin)			z_range_ne->SetValue(min,0);
	else				z_range_ne->SetValue(the_plot->GetZMinimum(),0);

	if(zmin && zmax){
		bool same = (z_range_ne->GetValue(0)==z_range_ne->GetValue(1)) ? 1:0;
		if(!z_range_ne->IsValueOk(0)||same) z_range_ne->SetValue(the_plot->GetZMinimum(),0);
		if(!z_range_ne->IsValueOk(1)||same) z_range_ne->SetValue(the_plot->GetZMaximum(),1);
	}

	z_range_ne->SetRange(the_plot->GetZMinimum(),z_range_ne->GetValue(1),0);
	z_range_ne->SetRange(z_range_ne->GetValue(0),the_plot->GetZMaximum(),1);

	//set histogram range
	the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));

	the_plot->Update();
}


void SlsQt2DPlotLayout::SetZScaleToLog(bool yes){
#ifndef IAN
#ifdef VERBOSE
	cout<<"Setting ZScale to log:"<<yes<<endl;
#endif
	btnLogz->setChecked(yes);
#endif
	the_plot->LogZ(yes);
	ResetRange();
}


void SlsQt2DPlotLayout::SetXTitle(QString st){
#ifndef IAN
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::xBottom)->setTitle(title);
#else
	GetPlot()->axisWidget(QwtPlot::xBottom)->setTitle(st);
#endif
}

void SlsQt2DPlotLayout::SetYTitle(QString st){
#ifndef IAN
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::yLeft)->setTitle(title);
#else
	GetPlot()->axisWidget(QwtPlot::yLeft)->setTitle(st);
#endif
}

void SlsQt2DPlotLayout::SetZTitle(QString st){
#ifndef IAN
	QwtText title(st);
	title.setFont(QFont("Sans Serif",11,QFont::Normal));
	GetPlot()->axisWidget(QwtPlot::yRight)->setTitle(title);
#else
	GetPlot()->axisWidget(QwtPlot::yRight)->setTitle(st);
#endif
}


#ifndef IAN
void SlsQt2DPlotLayout::SetZRange(double zmin, double zmax){
#ifdef VERBOSE
	cout<<"zmin:"<<zmin<<"\tzmax:"<<zmax<<endl;
#endif
	z_range_ne->SetNumber(zmin,0);
	z_range_ne->SetNumber(zmax,1);
	ResetRange();
}

void SlsQt2DPlotLayout::EnableZRange(bool enable){
#ifdef VERBOSE
	cout<<"Setting Z Range Enable to "<<enable<<endl;
#endif
	zRangeChecked  = enable;
	ResetRange();

}
#endif

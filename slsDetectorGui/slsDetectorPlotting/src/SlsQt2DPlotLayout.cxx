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



SlsQt2DPlotLayout::SlsQt2DPlotLayout(QWidget *parent):QGroupBox(parent){
	the_layout=0;
	the_plot   = new SlsQt2DPlot(this);

	z_range_ne = new SlsQtNumberEntry(this,1,(char*)"Set the z axis range from",2,(char*)"to",2);
	z_range_ne->setFixedWidth(402);
	zRangeChecked = false;
	z_range_ne->hide();

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
  btnLogz = new QToolButton(this);
    btnLogz->setText("Log Scale (Z)");
    btnLogz->setCheckable(true);
    btnLogz->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnLogz->hide();
}


void SlsQt2DPlotLayout::Layout(){
	if(the_layout) delete the_layout;
	the_layout =  new QGridLayout(this);
	the_layout->addWidget(the_plot,2,0,3,3);
	the_layout->addWidget(z_range_ne,5,0,1,3);
}

void SlsQt2DPlotLayout::ConnectSignalsAndSlots(){
	connect(this,		SIGNAL(InterpolateSignal(bool)),			the_plot,	SLOT(InterpolatedPlot(bool)));
	connect(this,		SIGNAL(ContourSignal(bool)),				the_plot,	SLOT(showContour(bool)));
	connect(z_range_ne,	SIGNAL(CheckBoxChanged(bool)),				this,		SLOT(ResetRange()));
	connect(z_range_ne,	SIGNAL(AValueChanged(SlsQtNumberEntry*)),	this,		SLOT(ResetRange()));
	btnLogz->setChecked(false);
}

void SlsQt2DPlotLayout::UpdateNKeepSetRangeIfSet(){
		//just reset histogram range before update
		the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));
	the_plot->Update();
}

void SlsQt2DPlotLayout::ResetRange(){
	//refind z limits
	the_plot->SetZMinMax();
	if(btnLogz->isChecked()) the_plot->SetZMinimumToFirstGreaterThanZero();
	if(zRangeChecked){
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

	if(zmin || zmax)		zRangeChecked = true;
	else					zRangeChecked = false;

	if(zmin)		z_range_ne->SetNumber(min,0);
	if(zmax)		z_range_ne->SetNumber(max,1);


	//refind z limits
	the_plot->SetZMinMax();
	//finds zmin value from hist
	if(btnLogz->isChecked())
		the_plot->SetZMinimumToFirstGreaterThanZero();

	if(zRangeChecked){

		//if value not given, take max or min of plot
		if(zmax)			z_range_ne->SetValue(max,0);
		else				z_range_ne->SetValue(the_plot->GetZMaximum(),1);

		if(zmin)			z_range_ne->SetValue(min,0);
		else				z_range_ne->SetValue(the_plot->GetZMinimum(),0);

		//check if zmin and zmax is same or not a proper double value
		//if(zmin && zmax){
			bool same = (z_range_ne->GetValue(0)==z_range_ne->GetValue(1)) ? 1:0;
			if(!z_range_ne->IsValueOk(0)||same) z_range_ne->SetValue(the_plot->GetZMinimum(),0);
			if(!z_range_ne->IsValueOk(1)||same) z_range_ne->SetValue(the_plot->GetZMaximum(),1);
		//}

		z_range_ne->SetRange(the_plot->GetZMinimum(),z_range_ne->GetValue(1),0);
		z_range_ne->SetRange(z_range_ne->GetValue(0),the_plot->GetZMaximum(),1);

		//set histogram range
		the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));

	}
	the_plot->Update();
}


void SlsQt2DPlotLayout::SetZScaleToLog(bool yes){
#ifdef VERBOSE
	cout<<"Setting ZScale to log:"<<yes<<endl;
#endif
	btnLogz->setChecked(yes);
	the_plot->LogZ(yes);
	ResetRange();
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

/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>


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
  // z_range_ne->SetNDecimalsOfDoubleValidators(2);
    z_range_ne->setFixedWidth(402);
    z_range_ne->hide();

    logsChecked = false;

  Layout();
  ConnectSignalsAndSlots();
}

SlsQt2DPlotLayout::~SlsQt2DPlotLayout(){

  if(the_layout) delete the_layout;


  delete the_plot;
  delete z_range_ne;
}


void SlsQt2DPlotLayout::Layout(){
  if(the_layout) delete the_layout;
  the_layout =  new QGridLayout(this);
    the_layout->addWidget(the_plot,2,1,3,3);
    the_layout->addWidget(z_range_ne,5,1,1,3);
}

void SlsQt2DPlotLayout::ConnectSignalsAndSlots(){
	connect(this,		SIGNAL(InterpolateSignal(bool)),			the_plot,	SLOT(InterpolatedPlot(bool)));
	connect(this,		SIGNAL(ContourSignal(bool)),				the_plot,	SLOT(showContour(bool)));
    connect(z_range_ne,	SIGNAL(CheckBoxChanged(bool)),				this,		SLOT(ResetRange()));
    connect(z_range_ne,	SIGNAL(AValueChanged(SlsQtNumberEntry*)),	this,		SLOT(ResetRange()));

}

void SlsQt2DPlotLayout::UpdateNKeepSetRangeIfSet(){
  if(z_range_ne->CheckBoxState()){
    //just reset histogram range before update
    the_plot->SetZMinMax(z_range_ne->GetValue(0),z_range_ne->GetValue(1));
  }

  the_plot->Update();
}

void SlsQt2DPlotLayout::ResetRange(){
  //refind z limits
  the_plot->SetZMinMax();
  if(logsChecked) the_plot->SetZMinimumToFirstGreaterThanZero();

  if(z_range_ne->CheckBoxState()){
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



void SlsQt2DPlotLayout::SetZScaleToLog(bool yes){
	logsChecked=yes;
  the_plot->LogZ(yes);
  ResetRange();
}


void SlsQt2DPlotLayout::SetXTitle(QString st){
  GetPlot()->axisWidget(QwtPlot::xBottom)->setTitle(st);
}

void SlsQt2DPlotLayout::SetYTitle(QString st){
  GetPlot()->axisWidget(QwtPlot::yLeft)->setTitle(st);
}

void SlsQt2DPlotLayout::SetZTitle(QString st){
  GetPlot()->axisWidget(QwtPlot::yRight)->setTitle(st);
}

void SlsQt2DPlotLayout::SetTitle(QString st){
	setTitle(st);
}


/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>
#include <qprinter.h>
#include <qtoolbutton.h>

#if QT_VERSION >= 0x040000
#include <qprintdialog.h>
#endif
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>

#include "SlsQt2DPlot.h"

using namespace std;

SlsQt2DPlot::SlsQt2DPlot(QWidget *parent):QwtPlot(parent){

  axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating);
  axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);

  d_spectrogram = new QwtPlotSpectrogram();

  hist = new SlsQt2DHist();

  SetupZoom();
  SetupColorMap();

  d_spectrogram->setData(*hist);
  d_spectrogram->attach(this);

  plotLayout()->setAlignCanvasToScales(true);

  FillTestPlot();
  Update();

}


void SlsQt2DPlot::SetupColorMap(){

  colorMapLinearScale = new   QwtLinearColorMap(Qt::darkCyan, Qt::red);
    colorMapLinearScale->addColorStop(0.1, Qt::cyan);
    colorMapLinearScale->addColorStop(0.4, Qt::blue);
    colorMapLinearScale->addColorStop(0.6, Qt::green);
    colorMapLinearScale->addColorStop(0.95, Qt::yellow);
  d_spectrogram->setColorMap(*colorMapLinearScale);

  
  colorMapLogScale = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
    colorMapLogScale->addColorStop((pow(10,2*0.10)-1)/99.0, Qt::cyan);  //linear scale goes from 0 to 2 and log scale goes from 1 to 100
    colorMapLogScale->addColorStop((pow(10,2*0.40)-1)/99.0,Qt::blue);
    colorMapLogScale->addColorStop((pow(10,2*0.60)-1)/99.0,Qt::green);
    colorMapLogScale->addColorStop((pow(10,2*0.95)-1)/99.0,Qt::yellow);

  contourLevelsLinear = new QwtValueList();
  for(double level=0.5;level<10.0;level+=1.0 ) (*contourLevelsLinear) += level;
    d_spectrogram->setContourLevels(*contourLevelsLinear);
  contourLevelsLog = new QwtValueList();
  for(double level=0.5;level<10.0;level+=1.0 ) (*contourLevelsLog) += (pow(10,2*level/10.0)-1)/99.0 * 10;


  // A color bar on the right axis
  rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);
    enableAxis(QwtPlot::yRight);
}

void SlsQt2DPlot::FillTestPlot(int mode){
  static int nx = 50;
  static int ny = 50;
  static double *the_data=0;
  if(the_data==0) the_data = new double [nx*ny];

  double dmax = sqrt(pow(nx/2.0-0.5,2) + pow(ny/2.0-0.5,2));
  for(int i=0;i<nx;i++){
    for(int j=0;j<ny;j++){
      double d = sqrt(pow(nx/2.0-(i+0.5),2) + pow(ny/2.0-(j+0.5),2));

      if(mode%3) the_data[i+j*nx] = 10*d/dmax;
      else       the_data[i+j*nx] = 10*(1-d/dmax);
    }
  }

  hist->SetData(nx,200,822,ny,-0.5,ny-0.5,the_data);
}

void SlsQt2DPlot::SetupZoom(){
  // LeftButton for the zooming
  // MidButton for the panning
  // RightButton: zoom out by 1
  // Ctrl+RighButton: zoom out to full size

  zoomer = new SlsQt2DZoomer(canvas());
    zoomer->SetHist(hist);
  
#if QT_VERSION < 0x040000
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlButton);
#else
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlModifier);
#endif
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::RightButton);

  panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::MidButton);

  // Avoid jumping when labels with more/less digits
  // appear/disappear when scrolling vertically

  const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
  QwtScaleDraw *sd = axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent( fm.width("100.00") );

    const QColor c(Qt::darkBlue);
    zoomer->setRubberBandPen(c);
    zoomer->setTrackerPen(c);
}

/*void SlsQt2DPlot::CompletelyUnZoom(){
	  setAxisScale(QwtPlot::xBottom,hist->GetXMin(),hist->GetXMin()+(hist->GetXMax()-hist->GetXMin()));
	  setAxisScale(QwtPlot::yLeft,hist->GetYMin(),hist->GetYMin()+(hist->GetYMax()-hist->GetYMin()));
	  zoomer->setZoomBase();
	  //replot();
}*/

void SlsQt2DPlot::UnZoom(){
  zoomer->setZoomBase(QwtDoubleRect(hist->GetXMin(),hist->GetYMin(),hist->GetXMax()-hist->GetXMin(),hist->GetYMax()-hist->GetYMin()));

  zoomer->setZoomBase();//Call replot for the attached plot before initializing the zoomer with its scales.
  // zoomer->zoom(0);
}

void SlsQt2DPlot::SetZoom(double xmin,double ymin,double x_width,double y_width){
  zoomer->setZoomBase(QwtDoubleRect(xmin,ymin,x_width,y_width));
}

void SlsQt2DPlot::SetZMinMax(double zmin,double zmax){
  hist->SetMinMax(zmin,zmax);
}


void SlsQt2DPlot::Update(){
  rightAxis->setColorMap(d_spectrogram->data().range(),d_spectrogram->colorMap());
  if(!zoomer->zoomRectIndex()) UnZoom();
  setAxisScale(QwtPlot::yRight,d_spectrogram->data().range().minValue(),
	                       d_spectrogram->data().range().maxValue());
  replot();
}



void SlsQt2DPlot::showContour(bool on){
  d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode,on);
  Update();
}

void SlsQt2DPlot::showSpectrogram(bool on){
  //  static int io=0;
  //  FillTestPlot(io++);
  d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
  d_spectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
  Update();
}

void SlsQt2DPlot::InterpolatedPlot(bool on){
  hist->Interpolate(on);
  Update();
}


void SlsQt2DPlot::LogZ(bool on){
  if(on){
    //if(hist->GetMinimum()<=0) hist->SetMinimumToFirstGreaterThanZero();
    d_spectrogram->setColorMap(*colorMapLogScale);
    setAxisScaleEngine(QwtPlot::yRight,new QwtLog10ScaleEngine);
    d_spectrogram->setContourLevels(*contourLevelsLog);
  }else{
    d_spectrogram->setColorMap(*colorMapLinearScale);
    setAxisScaleEngine(QwtPlot::yRight,new QwtLinearScaleEngine);
    d_spectrogram->setContourLevels(*contourLevelsLinear);
  }
  Update();

}

//Added by Dhanya on 19.06.2012 to disable zooming when any of the axes range has been set
void SlsQt2DPlot::DisableZoom(bool disableZoom){
#ifdef VERBOSE
	if(disableZoom) cout<<"Disabling zoom"<<endl;
	else cout<<"Enabling zoom"<<endl;
#endif
	if(disableZoom){
		if(zoomer){
			 zoomer->setMousePattern(QwtEventPattern::MouseSelect1,Qt::NoButton);
#if QT_VERSION < 0x040000
			zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::NoButton, Qt::ControlButton);
#else
			zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::NoButton, Qt::ControlModifier);
#endif
			zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::NoButton);
		}
		if(panner)	panner->setMouseButton(Qt::NoButton);
	}else {
		if(zoomer){
			zoomer->setMousePattern(QwtEventPattern::MouseSelect1,Qt::LeftButton);
#if QT_VERSION < 0x040000
			zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlButton);
#else
			zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlModifier);
#endif
			zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::RightButton);
		}
		if(panner)	panner->setMouseButton(Qt::MidButton);
	}
}


/*
void SlsQt2DPlot::printPlot(){
  QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
#if QT_VERSION < 0x040000
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName("spectrogram.ps");
    if (printer.setup())
#else
    printer.setOutputFileName("spectrogram.pdf");
    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
#endif
      {
        print(printer);
      }
}

*/

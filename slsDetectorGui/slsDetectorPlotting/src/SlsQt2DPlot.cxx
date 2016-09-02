
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <cmath>
#include <iostream>
#include <qprinter.h>
#include <qtoolbutton.h>
#include <qlist.h>

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

#if QWT_VERSION >= 0x060100
#define QwtLog10ScaleEngine QwtLogScaleEngine
#endif

using namespace std;

SlsQt2DPlot::SlsQt2DPlot(QWidget *parent):QwtPlot(parent){
  isLog=0;

  axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating);
  axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);

  d_spectrogram = new QwtPlotSpectrogram();

  hist = new SlsQt2DHist();
  SetupZoom();
  SetupColorMap();


#if QWT_VERSION<0x060000 
  d_spectrogram->setData(*hist);
#else
  d_spectrogram->setData(hist);
#endif


  d_spectrogram->attach(this);

  plotLayout()->setAlignCanvasToScales(true);

  FillTestPlot();
  Update();

}


void SlsQt2DPlot::SetupColorMap(){




  colorMapLinearScale = myColourMap(0);
#if QWT_VERSION<0x060000 
	d_spectrogram->setColorMap(*colorMapLinearScale );
#else
  d_spectrogram->setColorMap(colorMapLinearScale );
#endif

  colorMapLogScale = myColourMap(1);
#if QWT_VERSION<0x060000 
    contourLevelsLinear = new QwtValueList();
    for(double level=0.5;level<10.0;level+=1.0 ) (*contourLevelsLinear) += level;
    d_spectrogram->setContourLevels(*contourLevelsLinear);
#else
    ;
   
    for(double level=0.5;level<10.0;level+=1.0 ) (contourLevelsLinear) += level;
  d_spectrogram->setContourLevels(contourLevelsLinear);
#endif


    // 
#if QWT_VERSION<0x060000 
    contourLevelsLog = new QwtValueList();
    for(double level=0.5;level<10.0;level+=1.0 ) (*contourLevelsLog) += (pow(10,2*level/10.0)-1)/99.0 * 10;
  
#else
    ;
   
    for(double level=0.5;level<10.0;level+=1.0 ) (contourLevelsLog) += (pow(10,2*level/10.0)-1)/99.0 * 10;
#endif


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
#if QWT_VERSION<0x060000
  zoomer->setZoomBase(QwtDoubleRect(hist->GetXMin(),hist->GetYMin(),hist->GetXMax()-hist->GetXMin(),hist->GetYMax()-hist->GetYMin()));
#else
  zoomer->setZoomBase(QRectF(hist->GetXMin(),hist->GetYMin(),hist->GetXMax()-hist->GetXMin(),hist->GetYMax()-hist->GetYMin()));
#endif 
  zoomer->setZoomBase();//Call replot for the attached plot before initializing the zoomer with its scales.
  // zoomer->zoom(0);
}

void SlsQt2DPlot::SetZoom(double xmin,double ymin,double x_width,double y_width){

#if QWT_VERSION<0x060000
  zoomer->setZoomBase(QwtDoubleRect(xmin,ymin,x_width,y_width));

#else
  zoomer->setZoomBase(QRectF(xmin,ymin,x_width,y_width));
#endif
}

void SlsQt2DPlot::SetZMinMax(double zmin,double zmax){
  hist->SetMinMax(zmin,zmax);
}


QwtLinearColorMap* SlsQt2DPlot::myColourMap(QVector<double> colourStops) {

int ns=5;

double r[]={0.00, 0.00, 0.87, 1.00, 0.51};
double g[]={0.00, 0.81, 1.00, 0.20, 0.00};
 double b[]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };

 QColor c1,c2,c;
	c1.setRgbF(r[0],g[0],b[0],0);
	c2.setRgbF(r[ns-1],g[ns-1],b[ns-1]);
	QwtLinearColorMap* copyMap = new QwtLinearColorMap(Qt::lightGray, c2);

	for (int is=0; is<ns-1; is++) {
	c.setRgbF(r[is],g[is],b[is]);
  	copyMap->addColorStop(colourStops.value(is),c );
	}

  return copyMap;

    
}
QwtLinearColorMap* SlsQt2DPlot::myColourMap(int log) {

int ns=5;


	 QVector<double> cs1(0);
	QVector<double> lcs1(0);
		 
  
	  cs1.append(0.);
	  cs1.append(0.34);
	  cs1.append(0.61);
	  cs1.append(0.84);
	  cs1.append(1.);
if (log) {
	for (int is=0; is<ns; is++) {
	  lcs1.append((pow(10,2*cs1.value(is))-1)/99.0);
	}
	return myColourMap(lcs1);
}
	
		return myColourMap(cs1);
}


void SlsQt2DPlot::Update(){
#if QWT_VERSION<0x060000
  rightAxis->setColorMap(d_spectrogram->data().range(),d_spectrogram->colorMap());
#else
  if (isLog)
    hist->SetMinimumToFirstGreaterThanZero();

  const QwtInterval zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
 
  rightAxis->setColorMap(zInterval,myColourMap(isLog));

#endif

  if(!zoomer->zoomRectIndex()) UnZoom();
  
#if QWT_VERSION<0x060000


  setAxisScale(QwtPlot::yRight,d_spectrogram->data().range().minValue(),
	       d_spectrogram->data().range().maxValue());
#else  


  setAxisScale(QwtPlot::yRight,zInterval.minValue(), zInterval.maxValue());
#ifdef VERYVERBOSE
   cout << "axis scale set" << endl;
#endif
  plotLayout()->setAlignCanvasToScales(true);
#ifdef VERYVERBOSE
   cout << "layout" << endl;
#endif
#endif
#ifdef VERYVERBOSE
   cout << "going to replot" << endl;
#endif
  replot();
#ifdef VERYVERBOSE
   cout << "done" << endl;
#endif
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
isLog=1;
    //if(hist->GetMinimum()<=0) hist->SetMinimumToFirstGreaterThanZero();
#if QWT_VERSION<0x060000
    d_spectrogram->setColorMap(*colorMapLogScale);
#else
    d_spectrogram->setColorMap(myColourMap(isLog));
#endif
    setAxisScaleEngine(QwtPlot::yRight,new QwtLog10ScaleEngine);
#if QWT_VERSION<0x060000
    d_spectrogram->setContourLevels(*contourLevelsLog);
#else
    d_spectrogram->setContourLevels(contourLevelsLog);
#endif
  }else{
isLog=0;

#if QWT_VERSION<0x060000
    d_spectrogram->setColorMap(*colorMapLinearScale);
#else
   d_spectrogram->setColorMap(myColourMap(isLog));
#endif

	

    setAxisScaleEngine(QwtPlot::yRight,new QwtLinearScaleEngine);
	
#if QWT_VERSION<0x060000
    d_spectrogram->setContourLevels(*contourLevelsLinear);
#else
    d_spectrogram->setContourLevels(contourLevelsLinear);
#endif
	
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

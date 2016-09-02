
/**
 * @author Ian Johnson
 * @version 1.0
 */

#include <iostream>
#include <stdlib.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_math.h>
#include "SlsQt1DPlot.h"

#if QWT_VERSION >= 0x060100
#define QwtLog10ScaleEngine QwtLogScaleEngine
#endif

using namespace std;

SlsQtH1D::SlsQtH1D(QString title,int n, double min, double max, double* data):QwtPlotCurve(title){
  Initailize();
  SetData(n,min,max,data);
}

SlsQtH1D::SlsQtH1D(QString title,int n, double* data_x, double* data_y):QwtPlotCurve(title){
  Initailize();
  SetData(n,data_x,data_y);
}

void SlsQtH1D::Initailize(){
  ndata=n_array=0;
  x=y=0;
  pen_ptr = new QPen();
  SetLineColor();
}

SlsQtH1D::~SlsQtH1D(){delete x;delete y;delete pen_ptr;}

void SlsQtH1D::Attach(SlsQt1DPlot* p){
  attach((QwtPlot*) p);
  p->NewHistogramAttached(this);
}

void SlsQtH1D::Detach(SlsQt1DPlot* p){
  detach();
  p->HistogramDetached(this);
}


int SlsQtH1D::SetLineColor(int c){
  static int last_color = 1;
  if(c<0) c=(last_color+1)%3;
     
  switch(c){
  case 0:	pen_ptr->setColor(Qt::black);	break;
  case 1:	pen_ptr->setColor(Qt::red);		break;
  case 2:	pen_ptr->setColor(Qt::blue);	break;
  case 3:	pen_ptr->setColor(Qt::green);	break;
  case 4:	pen_ptr->setColor(Qt::magenta);	break;
  case 5:	pen_ptr->setColor(Qt::cyan);	break;
  case 6:	pen_ptr->setColor(Qt::darkYellow);	break;
  case 7:	pen_ptr->setColor(Qt::gray);	break;
  case 8:	pen_ptr->setColor(Qt::darkBlue);	break;
  case 9:	pen_ptr->setColor(Qt::darkGreen);	break;
  case 10:	pen_ptr->setColor(Qt::darkMagenta);	break;
  }
/*  if(c==0)      pen_ptr->setColor(Qt::black);
  else if(c==1) pen_ptr->setColor(Qt::red);
  else          pen_ptr->setColor(Qt::blue);*/

  setPen(*pen_ptr);

  return last_color=c;
}

int SlsQtH1D::SetLineWidth(int w){
  pen_ptr->setWidth(w);
  setPen(*pen_ptr);
  return w;
}

void SlsQtH1D::SetLineStyle(int s){
  if(s==1)       pen_ptr->setStyle(Qt::DashLine);
  else if(s==2)  pen_ptr->setStyle(Qt::DotLine);
  else if(s==3)  pen_ptr->setStyle(Qt::DashDotLine);
  else if(s==4)  pen_ptr->setStyle(Qt::DashDotDotLine);
  else if(s==5)  pen_ptr->setStyle(Qt::CustomDashLine);
  else           pen_ptr->setStyle(Qt::SolidLine);
  setPen(*pen_ptr);
}


void SlsQtH1D::SetData(int n, double xmin, double xmax, double *data){
  n = SetUpArrays(n);
  
  ndata=n;
  if(xmin>xmax){
    double t=xmin;
    xmin=xmax;
    xmax=t;
  }

  dx = (xmax-xmin)/n;
  ymin=ymax= data ? data[0]:0;
  firstXgt0=-1;
  firstYgt0=-1;

  for(int i=0;i<ndata;i++){
    x[i] = i ? x[i-1]+dx:xmin;
    y[i] = data ? data[i]:0;
    if(data&&ymin>y[i])  ymin = y[i];
    if(data&&ymax<y[i])  ymax = y[i]; 
    if(x[i]>0&&(firstXgt0<0||firstXgt0>x[i])) firstXgt0=x[i];
    if(y[i]>0&&(firstYgt0<0||firstYgt0>y[i])) firstYgt0=y[i];
  }

  //  cout<<endl<<endl<<"one can think about correcting negative values in the data and recalling the setRawData function in setRawData when plotting on log scales"<<endl;

  //  if(firstXgt0<0)firstXgt0=0.001;
  //  if(firstYgt0<0)firstYgt0=0.001;
#if QWT_VERSION<0x060000
 setRawData(x,y,ndata);
#else
  setRawSamples(x,y,ndata);
#endif
}

void SlsQtH1D::SetData(int n, double* data_x, double *data_y){

  int reverse = (data_x&&n>0&&data_x[0]>data_x[n-1]) ? 1:0;
  n = SetUpArrays(n);

  ndata=n;
  dx=-1; //signifies not regular intervals

  ymin=ymax= data_y ? data_y[0]:0;

  firstXgt0=-1;
  firstYgt0=-1;

  for(int i=0;i<ndata;i++){
    int b = reverse ? n-i-1:i;
    x[b] = data_x ? data_x[i]:0;
    y[b] = data_y ? data_y[i]:0;
    if(data_y&&ymin>y[b])  ymin = y[b];
    if(data_y&&ymax<y[b])  ymax = y[b]; 
    if(x[b]>0&&(firstXgt0<0||firstXgt0>x[b])) firstXgt0=x[b];
    if(y[b]>0&&(firstYgt0<0||firstYgt0>y[b])) firstYgt0=y[b];
  }

#if QWT_VERSION<0x060000
  setRawData(x,y,ndata);
#else
  setRawSamples(x,y,ndata);
#endif
}

int SlsQtH1D::SetUpArrays(int n){
  n = n<1 ? 1 : n; //overflow bin
  
  if(n+1>n_array){
    n_array = n+1;
    if(x) delete x;
    if(y) delete y;
    x = new double [n_array];
    y = new double [n_array];
  }

  return n;
}

double SlsQtH1D::FillBin(int bx, double v)       {
  bx = CheckIndex(bx);
  return SetBinContent(bx,y[bx]+v);
}
double SlsQtH1D::Fill(double x, double v){return FillBin(FindBinIndex(x),v);}

double SlsQtH1D::SetBinContent(int bx,double v){
  bx = CheckIndex(bx);
  y[bx]=v;
  if(bx<ndata){
    if(y[bx]<ymin) ymin = y[bx];
    if(y[bx]>0&&(firstYgt0<=0||y[bx]<firstYgt0)) firstYgt0 = y[bx];
    if(y[bx]>ymax) ymax = y[bx];
  }
  return y[bx];
}

double SlsQtH1D::SetContent(double x,double v)   {return SetBinContent(FindBinIndex(x),v); }

int SlsQtH1D::FindBinIndex(double px){
  if(dx>0) CheckIndex(int((px-x[0])/dx));
  
  //find closest bin
  int b=0; for(;b<ndata;b++) if(x[b]>px) break;

  if(b==0) return 0;
  else if(fabs(px-x[b-1])<fabs(px-x[b])) return b-1;

  return b;
}

int SlsQtH1D::CheckIndex(int bx){return (bx<0||bx>ndata) ? ndata : bx;}//ndata is the overflow bin

SlsQtH1D* SlsQtH1D::Add(double v){ for(int bx=0;bx<ndata;bx++) FillBin(bx,v); return this;}


//1d hist list stuff
SlsQtH1DList::SlsQtH1DList(SlsQtH1D* hist){
  the_hist = hist;
  the_next = 0;
}

SlsQtH1DList::~SlsQtH1DList(){
  if(the_next) delete the_next;
}

SlsQtH1D* SlsQtH1DList::Add(SlsQtH1D* hist){
  //  cout<<"Adding: "<<hist<<endl;
  SlsQtH1DList* hl=this;

  while(hl){
    if(hist==hl->the_hist) return hist; //already added
    if(!hl->the_next) break;
    hl=hl->the_next;
  }
  if(hl->the_hist) hl->the_next = new SlsQtH1DList(hist);
  else             hl->the_hist = hist;
  
  //  Print();

  return hist;
}

void SlsQtH1DList::Print(){
  //  cout<<"Printing List"<<endl;
  SlsQtH1DList* hl=this;
  int i=0;
  while(hl){
    cout<<"    "<<i++<<") "<<hl<<"  "<<hl->the_hist<<"  "<<hl->the_next<<endl;
    hl=hl->the_next;
    if(i>10) break;
  }
}

void SlsQtH1DList::Remove(SlsQtH1D* hist){
  //  cout<<"Removing: "<<hist<<endl;
  SlsQtH1DList* hl=this;
  while(hl){  //every match will be removed
    if(hl->the_hist!=hist)  hl = hl->the_next;
    else{ //match
      if(!hl->the_next) hl->the_hist=0; // first the_hist is zero when there's no next
      else{
	SlsQtH1DList* t = hl->the_next;
	hl->the_hist = t->the_hist;
	hl->the_next = t->the_next;
	t->the_next = 0;
	delete t;
      }
    }
  }
  //  Print();
}



//1d plot stuff
SlsQt1DPlot::SlsQt1DPlot(QWidget *parent):QwtPlot(parent){  
  //  n_histograms_attached=0;
  hline=vline=0;
   hist_list = new SlsQtH1DList();

  UnknownStuff();
  alignScales();
  SetupZoom();

  // Assign a title
#ifndef IAN
  insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
#else
  insertLegend(new QwtLegend(), QwtPlot::RightLegend);
#endif

  axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating);
  axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);
}

SlsQt1DPlot::~SlsQt1DPlot(){
  delete hist_list;

  if(hline) delete hline;
  if(vline) delete vline;
}

void SlsQt1DPlot::CalculateNResetZoomBase(){
  if(hist_list->Hist()) zoomer->SetZoomBase(hist_list->Hist());
  SlsQtH1DList* hl = hist_list->Next();
  while(hl){
    if(hl->Hist()) zoomer->ExtendZoomBase(hl->Hist());
    hl=hl->Next();
  }
}

void SlsQt1DPlot::NewHistogramAttached(SlsQtH1D* h){
  hist_list->Add(h);
  CalculateNResetZoomBase();
  //commented out by dhanya to take off zooming every hist in 1d plots
  //if(!hist_list->Next()) UnZoom();
  Update();
}

void SlsQt1DPlot::HistogramDetached(SlsQtH1D* h){
  hist_list->Remove(h);
  CalculateNResetZoomBase();
  Update();
}

void SlsQt1DPlot::Update(){
    replot();
}
 
void SlsQt1DPlot::SetTitle(const char* title){
  setTitle(title);
}

void SlsQt1DPlot::SetXTitle(const char* title){
	QwtText t(title);
	t.setFont(QFont("Sans Serif",11,QFont::Normal));
	setAxisTitle(QwtPlot::xBottom,t);
}
void SlsQt1DPlot::SetYTitle(const char* title){
	QwtText t(title);
	t.setFont(QFont("Sans Serif",11,QFont::Normal));
	setAxisTitle(QwtPlot::yLeft,t);
}

void SlsQt1DPlot::SetLogX(bool yes){ SetLog(QwtPlot::xBottom,yes);}
void SlsQt1DPlot::SetLogY(bool yes){ SetLog(QwtPlot::yLeft,yes);}
void SlsQt1DPlot::SetLog(int axisId, bool yes){
  if(axisId==QwtPlot::xBottom) zoomer->SetLogX(yes);
  if(axisId==QwtPlot::yLeft)   zoomer->SetLogY(yes);

  zoomer->ResetZoomBase();  //needs to be done before setting Engine

  //the old ones are deleted by in the setAxisScaleFunction() function see: 128 of file qwt_plot_axis.cpp
  if(yes) setAxisScaleEngine(axisId,new QwtLog10ScaleEngine());
  else    setAxisScaleEngine(axisId,new QwtLinearScaleEngine());

  axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating);
  axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);

  Update();
}

void SlsQt1DPlot::UnZoom(){
  setAxisScale(QwtPlot::xBottom,zoomer->x(),zoomer->x()+zoomer->w());
  setAxisScale(QwtPlot::yLeft,zoomer->y(),zoomer->y()+zoomer->h());

  zoomer->setZoomBase();//Call replot for the attached plot before initializing the zoomer with its scales.
  Update();
}

void SlsQt1DPlot::SetZoom(double xmin,double ymin,double x_width,double y_width){
  setAxisScale(QwtPlot::xBottom,xmin,xmin+x_width);
  setAxisScale(QwtPlot::yLeft  ,ymin,ymin+y_width);
  Update();
}

void SlsQt1DPlot::RemoveHLine(){
  if(hline) hline->detach();
  delete hline;
  hline=0;
}

void SlsQt1DPlot::InsertHLine(double y){
  if(!hline){
    hline = new QwtPlotMarker();
    hline->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    hline->setLineStyle(QwtPlotMarker::HLine);
    hline->attach(this);
  }
  hline->setYValue(y);
}

void SlsQt1DPlot::RemoveVLine(){
  if(vline) vline->detach();
  delete vline;
  vline=0;
}

void SlsQt1DPlot::InsertVLine(double x){
  if(!vline){
    vline = new QwtPlotMarker();
    vline->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
    vline->setLineStyle(QwtPlotMarker::VLine);
    vline->attach(this);
  }
  vline->setXValue(x);
}


    

void SlsQt1DPlot::SetupZoom(){
  // LeftButton for the zooming
  // MidButton for the panning
  // RightButton: zoom out by 1
  // Ctrl+RighButton: zoom out to full size

  zoomer = new SlsQt1DZoomer(canvas());
  
#if QT_VERSION < 0x040000
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlButton);
#else
  zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlModifier);
#endif
  zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::RightButton);

  panner = new QwtPlotPanner((QwtPlotCanvas*)canvas());
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


//  Set a plain canvas frame and align the scales to it
void SlsQt1DPlot::alignScales(){
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    ((QwtPlotCanvas*)canvas())->setFrameStyle(QFrame::Box | QFrame::Plain );
    ((QwtPlotCanvas*)canvas())->setLineWidth(1);

    for(int i = 0; i < QwtPlot::axisCnt; i++ ){
      QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(i);
      if(scaleWidget) scaleWidget->setMargin(0);
      QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw(i);
      if(scaleDraw) scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
}

void SlsQt1DPlot::UnknownStuff(){
#if QWT_VERSION<0x060000
  // Disable polygon clipping
  //not supported for version 6
  QwtPainter::setDeviceClipping(false);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);    
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);
#else
  // We don't need the cache here
   ((QwtPlotCanvas*)canvas())->setPaintAttribute(QwtPlotCanvas::BackingStore, false);

#endif

  

#if QT_VERSION >= 0x040000
#ifdef Q_WS_X11
     //  Qt::WA_PaintOnScreen is only supported for X11, but leads
     //  to substantial bugs with Qt 4.2.x/Windows
     canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
#endif
#endif
}



//Added by Dhanya on 19.06.2012 to disable zooming when any of the axes range has been set
void SlsQt1DPlot::DisableZoom(bool disableZoom){
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






/**
 * @author Ian Johnson
 * @version 1.0
 * Modifications:
 * 19.06.2012: {Some functions have been added by Dhanya to enable zooming in and out
 * without using mouse control:
 * DisableZoom,
 * SetXMinMax,SetYMinMax,
 * GetXMinimum,GetXMaximum,GetYMinimum,GetYMaximum}
 *  */

#ifndef SLSQT1DPLOT_H
#define SLSQT1DPLOT_H

typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#include "ansi.h"


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_div.h>
#include  "SlsQt1DZoomer.h"
#include <iostream>
using namespace std;

class QPen;
class SlsQt1DPlot;

class SlsQtH1D:public QwtPlotCurve{

 public:
  SlsQtH1D(QString title, int n, double xmin, double xmax, double* data=0);
  SlsQtH1D(QString title, int n, double* data_x, double* data_y);
  ~SlsQtH1D();

  void Attach(SlsQt1DPlot* p);
  void Detach(SlsQt1DPlot* p);

  int   SetLineColor(int c=-1);
  int   SetLineWidth(int w=1);
  void  SetLineStyle(int s=0);

  void SetData(int n, double xmin, double xmax, double* d=0);
  void SetData(int n, double* dx, double* dy);

  double*  GetX()      {return x;}
  double*  GetY()      {return y;}
  int      GetNBinsX() {return ndata;}

  double   FillBin(int bx, double v=1);
  double   Fill(double x, double v=1);
  double   SetBinContent(int bx,double v);
  double   SetContent(double x,double v);
  int      FindBinIndex(double px);

  double   GetXMin()         {return x[0];}
  double   GetFirstXgtZero() {return firstXgt0;}
  double   GetXMax()         {return x[ndata-1];}
  double   GetYMin()         {return ymin;}
  double   GetFirstYgtZero() {return firstYgt0;}
  double   GetYMax()         {return ymax;}

  SlsQtH1D*   Add(double v);



 private:
  int    ndata;
  int    n_array;
  double dx;
  double *x,*y;
  double ymin,ymax;
  double firstXgt0,firstYgt0;

  void Initailize();
  int  SetUpArrays(int n);
  int  CheckIndex(int bx);

  QPen* pen_ptr;
};


class SlsQtH1DList{
 public:
  SlsQtH1DList(SlsQtH1D* hist=0);
  ~SlsQtH1DList();

  SlsQtH1D*     Add(SlsQtH1D* h); 
  void       Remove(SlsQtH1D* h); 
  void       Print();

  SlsQtH1D*     Hist() {return the_hist;} //if no hist returns 0
  SlsQtH1DList* Next() {return the_next;}


private:
   SlsQtH1DList*  the_next;
   SlsQtH1D*      the_hist;
};
 

class SlsQt1DPlot:public QwtPlot{
  Q_OBJECT

 public:
  SlsQt1DPlot(QWidget* = NULL);
  ~SlsQt1DPlot();
    
  void SetTitle(const char *t);
  void SetXTitle(const char* title);
  void SetYTitle(const char* title);
  
  void InsertHLine(double y);
  void RemoveHLine();
  void InsertVLine(double v);
  void RemoveVLine();


  /**	This group of functions have been added by Dhanya on 19.06.2012 to be able to
  	use zooming functionality without mouse control*/
  void DisableZoom(bool disableZoom);
  void SetXMinMax(double min,double max){setAxisScale(QwtPlot::xBottom,min,max);};
  void SetYMinMax(double min,double max){setAxisScale(QwtPlot::yLeft,min,max);};
  double GetXMinimum(){return hist_list->Hist()->GetXMin();};
  double GetXMaximum(){return hist_list->Hist()->GetXMax();};
  double GetYMinimum(){return hist_list->Hist()->GetYMin();};
  double GetYMaximum(){return hist_list->Hist()->GetYMax();};
  /**---*/


  void SetZoom(double xmin,double ymin,double x_width,double y_width);
  void SetZoomBase(double xmin,double ymin,double x_width, double y_width){ zoomer->SetZoomBase(xmin,ymin,x_width,y_width);}

  void alignScales();
 private:
  
  SlsQtH1DList*     hist_list;
  SlsQt1DZoomer*    zoomer;
  QwtPlotPanner* panner;

  QwtPlotMarker *hline;
  QwtPlotMarker *vline;
  
  void SetupZoom();
  void UnknownStuff();
  //void alignScales();
  
  void CalculateNResetZoomBase();
  void NewHistogramAttached(SlsQtH1D* h);
  void HistogramDetached(SlsQtH1D* h);

  void SetLog(int axisId, bool yes);
  
  friend void SlsQtH1D::Attach(SlsQt1DPlot* p);
  friend void SlsQtH1D::Detach(SlsQt1DPlot* p);
  
  
 public slots:
  void UnZoom();
  void Update();

  void SetLogX(bool yes=1);  
  void SetLogY(bool yes=1);

 protected:

};

#endif
